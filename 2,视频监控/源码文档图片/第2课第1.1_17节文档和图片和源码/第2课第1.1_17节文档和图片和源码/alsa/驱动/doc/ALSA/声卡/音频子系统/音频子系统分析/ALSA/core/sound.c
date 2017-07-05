/*
 *  Advanced Linux Sound Architecture
 *  Copyright (c) by Jaroslav Kysela <perex@perex.cz>
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/time.h>
#include <linux/device.h>
#include <linux/moduleparam.h>
#include <sound/core.h>
#include <sound/minors.h>
#include <sound/info.h>
#include <sound/version.h>
#include <sound/control.h>
#include <sound/initval.h>
#include <linux/kmod.h>
#include <linux/mutex.h>

static int major = CONFIG_SND_MAJOR;  /* 声卡的主设备号 ---116*/
int snd_major; 
EXPORT_SYMBOL(snd_major);

static int cards_limit = 1;

MODULE_AUTHOR("Jaroslav Kysela <perex@perex.cz>");
MODULE_DESCRIPTION("Advanced Linux Sound Architecture driver for soundcards.");
MODULE_LICENSE("GPL");
module_param(major, int, 0444);
MODULE_PARM_DESC(major, "Major # for sound driver.");
module_param(cards_limit, int, 0444);
MODULE_PARM_DESC(cards_limit, "Count of auto-loadable soundcards.");
MODULE_ALIAS_CHARDEV_MAJOR(CONFIG_SND_MAJOR);

/* this one holds the actual max. card number currently available.
 * as default, it's identical with cards_limit option.  when more
 * modules are loaded manually, this limit number increases, too.
 */
int snd_ecards_limit;  /* 卡的最大数量的限制 */
EXPORT_SYMBOL(snd_ecards_limit);

static struct snd_minor *snd_minors[SNDRV_OS_MINORS];  /* 存放所有的次设备号相关信息 */
static DEFINE_MUTEX(sound_mutex);

#ifdef CONFIG_MODULES

/**
 * snd_request_card - try to load the card module
 * @card: the card number
 *
 * Tries to load the module "snd-card-X" for the given card number
 * via request_module.  Returns immediately if already loaded.
 */
 /* 尝试加载一个声卡模块
    参数card为卡的次设备号*/
void snd_request_card(int card)
{
	if (snd_card_locked(card)) /* 如果卡被上锁了则返回 */
		return;
	if (card < 0 || card >= cards_limit) /*检查次设备号是否合法 */
		return;
	request_module("snd-card-%i", card);  /*  */
}

EXPORT_SYMBOL(snd_request_card);

/* 尝试加载其他模块 */
static void snd_request_other(int minor)
{
	char *str;

	switch (minor) {
	case SNDRV_MINOR_SEQUENCER:	str = "snd-seq";	break;
	case SNDRV_MINOR_TIMER:		str = "snd-timer";	break;
	default:			return;
	}
	request_module(str);
}

#endif	/* modular kernel */

/**
 * snd_lookup_minor_data - get user data of a registered device
 * @minor: the minor number
 * @type: device type (SNDRV_DEVICE_TYPE_XXX)
 *
 * Checks that a minor device with the specified type is registered, and returns
 * its user data pointer.
 */
 /* 获取用户数据从一个注册的设备中
     minor为次设备号
     type为设备时间，查看SNDRV_DEVICE_TYPE_XXX*/
void *snd_lookup_minor_data(unsigned int minor, int type)
{
	struct snd_minor *mreg;
	void *private_data;

	if (minor >= ARRAY_SIZE(snd_minors)) /* 检查次设备号是否合法 */
		return NULL;
	mutex_lock(&sound_mutex);
	mreg = snd_minors[minor]; /* 获取以次设备号为下标的数据中项 */
	if (mreg && mreg->type == type)  /* 如果类型匹配匹配则说明匹配 */
		private_data = mreg->private_data;  /* 获取它的私有数据 */
	else
		private_data = NULL;
	mutex_unlock(&sound_mutex);
	return private_data; /* 返回私有数据 */
}

EXPORT_SYMBOL(snd_lookup_minor_data);

/* 打开声卡设备 */
static int __snd_open(struct inode *inode, struct file *file)
{
	unsigned int minor = iminor(inode); /*获取次设备号 */
	struct snd_minor *mptr = NULL;
	const struct file_operations *old_fops;
	int err = 0;

	if (minor >= ARRAY_SIZE(snd_minors)) /* 检查次设备号是否合法 */
		return -ENODEV;
	mptr = snd_minors[minor]; /* 以次设备号为下标获取对应的struct snd_minor指针 */
	if (mptr == NULL) {
#ifdef CONFIG_MODULES
		int dev = SNDRV_MINOR_DEVICE(minor);
		if (dev == SNDRV_MINOR_CONTROL) {
			/* /dev/aloadC? */
			int card = SNDRV_MINOR_CARD(minor);
			if (snd_cards[card] == NULL)
				snd_request_card(card); /* 尝试加载一个模块 */
		} else if (dev == SNDRV_MINOR_GLOBAL) {
			/* /dev/aloadSEQ */
			snd_request_other(minor); /*  尝试加载其他模块 */
		}
#ifndef CONFIG_SND_DYNAMIC_MINORS
		/* /dev/snd/{controlC?,seq} */
		mptr = snd_minors[minor];
		if (mptr == NULL)
#endif
#endif
			return -ENODEV;
	}
	old_fops = file->f_op; /* 保存原来的操作函数集指针 */
	file->f_op = fops_get(mptr->f_ops);  /* 获取设备的新的操作函数集并将它赋值给file->f_op ，这样系统调用就可以到具体的设备*/
	if (file->f_op == NULL) {
		file->f_op = old_fops;
		return -ENODEV;
	}
	if (file->f_op->open) 
		err = file->f_op->open(inode, file); /* 调用它的open函数 */
	if (err) {
		fops_put(file->f_op);
		file->f_op = fops_get(old_fops);
	}
	fops_put(old_fops);
	return err;
}


/* BKL pushdown: nasty #ifdef avoidance wrapper */
/* 打开声卡设备 */
static int snd_open(struct inode *inode, struct file *file)
{
	int ret;

	lock_kernel();
	ret = __snd_open(inode, file); /* 调用该函数打开设备 */
	unlock_kernel();
	return ret;
}

static const struct file_operations snd_fops =  /* 声卡的操作函数集 */
{
	.owner =	THIS_MODULE,
	.open =		snd_open
};

#ifdef CONFIG_SND_DYNAMIC_MINORS
/* 寻找空闲的次设备号 */
static int snd_find_free_minor(void)
{
	int minor;

	for (minor = 0; minor < ARRAY_SIZE(snd_minors); ++minor) {
		/* skip minors still used statically for autoloading devices */
		if (SNDRV_MINOR_DEVICE(minor) == SNDRV_MINOR_CONTROL ||
		    minor == SNDRV_MINOR_SEQUENCER)
			continue;
		if (!snd_minors[minor])
			return minor;
	}
	return -EBUSY;
}
#else
/* 获取次设备号 */
static int snd_kernel_minor(int type, struct snd_card *card, int dev)
{
	int minor;

	switch (type) {
	case SNDRV_DEVICE_TYPE_SEQUENCER:
	case SNDRV_DEVICE_TYPE_TIMER:
		minor = type;
		break;
	case SNDRV_DEVICE_TYPE_CONTROL:
		if (snd_BUG_ON(!card))
			return -EINVAL;
		minor = SNDRV_MINOR(card->number, type);
		break;
	case SNDRV_DEVICE_TYPE_HWDEP:
	case SNDRV_DEVICE_TYPE_RAWMIDI:
	case SNDRV_DEVICE_TYPE_PCM_PLAYBACK:
	case SNDRV_DEVICE_TYPE_PCM_CAPTURE:
		if (snd_BUG_ON(!card))
			return -EINVAL;
		minor = SNDRV_MINOR(card->number, type + dev);
		break;
	default:
		return -EINVAL;
	}
	if (snd_BUG_ON(minor < 0 || minor >= SNDRV_OS_MINORS))
		return -EINVAL;
	return minor;
}
#endif

/**
 * snd_register_device_for_dev - Register the ALSA device file for the card
 * @type: the device type, SNDRV_DEVICE_TYPE_XXX
 * @card: the card instance
 * @dev: the device index
 * @f_ops: the file operations
 * @private_data: user pointer for f_ops->open()
 * @name: the device file name
 * @device: the &struct device to link this new device to
 *
 * Registers an ALSA device file for the given card.
 * The operators have to be set in reg parameter.
 *
 * Returns zero if successful, or a negative error code on failure.
 */
 /* 为声卡注册ALSA设备(组件)文件 
     参数type为设备类型，查看宏SNDRV_DEVICE_TYPE_XXX
     参数card为设备要依附的声卡
     参数dev为设备引索
     参数f_ops为设备的操作函数集指针
     参数private_data为私有数组，用于f_ops->open()
     参数name为设备文件的名字
     参数device为参数card内嵌的设备*/
int snd_register_device_for_dev(int type, struct snd_card *card, int dev,
				const struct file_operations *f_ops,
				void *private_data,
				const char *name, struct device *device)
{
	int minor;
	struct snd_minor *preg;

	if (snd_BUG_ON(!name))
		return -EINVAL;
	preg = kmalloc(sizeof *preg, GFP_KERNEL);  /* 分配一个struct snd_minor结构体 */
	if (preg == NULL)
		return -ENOMEM;
	preg->type = type;
	preg->card = card ? card->number : -1;
	preg->device = dev;
	preg->f_ops = f_ops;
	preg->private_data = private_data;
	mutex_lock(&sound_mutex);
#ifdef CONFIG_SND_DYNAMIC_MINORS
	minor = snd_find_free_minor();  /* 寻找空闲的次设备号 */
#else
	minor = snd_kernel_minor(type, card, dev);/* 寻找空闲的次设备号 */
#else
	if (minor >= 0 && snd_minors[minor]) /* 说明次设备号已经被占用 */
		minor = -EBUSY;
#endif
	if (minor < 0) {
		mutex_unlock(&sound_mutex);
		kfree(preg);
		return minor;
	}
	snd_minors[minor] = preg; /* 以次设备号为引索，将创建的struct snd_minor 放入数组 */
	preg->dev = device_create(sound_class, device, MKDEV(major, minor),
				  private_data, "%s", name); /* 创建设备文件 */
	if (IS_ERR(preg->dev)) {
		snd_minors[minor] = NULL;
		mutex_unlock(&sound_mutex);
		minor = PTR_ERR(preg->dev);
		kfree(preg);
		return minor;
	}

	mutex_unlock(&sound_mutex);
	return 0;
}

EXPORT_SYMBOL(snd_register_device_for_dev);

/* find the matching minor record
 * return the index of snd_minor, or -1 if not found
 */
 /* 寻找匹配的次设备号 */
static int find_snd_minor(int type, struct snd_card *card, int dev)
{
	int cardnum, minor;
	struct snd_minor *mptr;

	cardnum = card ? card->number : -1; /* 获取卡的数量 */
	for (minor = 0; minor < ARRAY_SIZE(snd_minors); ++minor)  /* 遍历数组snd_minors，寻找对应的项，如果找到就将该项的下标返回，
	                                                                                                    由此可见，数组snd_minors是以次设备号为引索的*/
		if ((mptr = snd_minors[minor]) != NULL &&
		    mptr->type == type &&
		    mptr->card == cardnum &&
		    mptr->device == dev)
			return minor;
	return -1;
}

/**
 * snd_unregister_device - unregister the device on the given card
 * @type: the device type, SNDRV_DEVICE_TYPE_XXX
 * @card: the card instance
 * @dev: the device index
 *
 * Unregisters the device file already registered via
 * snd_register_device().
 *
 * Returns zero if sucecessful, or a negative error code on failure
 */
 /* 注销给定声卡上的设备(组件)
     参数type为设备类型，查看宏SNDRV_DEVICE_TYPE_XXX
     参数card为声卡
     参数dev为设备引索*/
int snd_unregister_device(int type, struct snd_card *card, int dev)
{
	int minor;

	mutex_lock(&sound_mutex);
	minor = find_snd_minor(type, card, dev);  /* 寻找次设备号 */
	if (minor < 0) {
		mutex_unlock(&sound_mutex);
		return -EINVAL;
	}

	device_destroy(sound_class, MKDEV(major, minor));  /* 销毁设备节点 */

	kfree(snd_minors[minor]);  /* 释放对于的数组项指向的结构的占用内存 */
	snd_minors[minor] = NULL;
	mutex_unlock(&sound_mutex);
	return 0;
}

EXPORT_SYMBOL(snd_unregister_device);

/* 在sysfs文件系统中添加设备(组件)文件
    参数type为设备类型
    参数card为设备依附的声卡
    参数dev为引索
    参数attr为设备属性*/
int snd_add_device_sysfs_file(int type, struct snd_card *card, int dev,
			      struct device_attribute *attr)
{
	int minor, ret = -EINVAL;
	struct device *d;

	mutex_lock(&sound_mutex);
	minor = find_snd_minor(type, card, dev); /* 寻找设备的次设备 */
	if (minor >= 0 && (d = snd_minors[minor]->dev) != NULL)
		ret = device_create_file(d, attr); /* 创建设备属性文件 */
	mutex_unlock(&sound_mutex);
	return ret;

}

EXPORT_SYMBOL(snd_add_device_sysfs_file);

#ifdef CONFIG_PROC_FS
/*
 *  INFO PART
 */

static struct snd_info_entry *snd_minor_info_entry;

/* 在函数给定设备(组件)类型type，返回给类型对应的设备名字 */
static const char *snd_device_type_name(int type)
{
	switch (type) {
	case SNDRV_DEVICE_TYPE_CONTROL:
		return "control";
	case SNDRV_DEVICE_TYPE_HWDEP:
		return "hardware dependent";
	case SNDRV_DEVICE_TYPE_RAWMIDI:
		return "raw midi";
	case SNDRV_DEVICE_TYPE_PCM_PLAYBACK:
		return "digital audio playback";
	case SNDRV_DEVICE_TYPE_PCM_CAPTURE:
		return "digital audio capture";
	case SNDRV_DEVICE_TYPE_SEQUENCER:
		return "sequencer";
	case SNDRV_DEVICE_TYPE_TIMER:
		return "timer";
	default:
		return "?";
	}
}

static void snd_minor_info_read(struct snd_info_entry *entry, struct snd_info_buffer *buffer)
{
	int minor;
	struct snd_minor *mptr;

	mutex_lock(&sound_mutex);
	for (minor = 0; minor < SNDRV_OS_MINORS; ++minor) {
		if (!(mptr = snd_minors[minor]))
			continue;
		if (mptr->card >= 0) {
			if (mptr->device >= 0)
				snd_iprintf(buffer, "%3i: [%2i-%2i]: %s\n",
					    minor, mptr->card, mptr->device,
					    snd_device_type_name(mptr->type));
			else
				snd_iprintf(buffer, "%3i: [%2i]   : %s\n",
					    minor, mptr->card,
					    snd_device_type_name(mptr->type));
		} else
			snd_iprintf(buffer, "%3i:        : %s\n", minor,
				    snd_device_type_name(mptr->type));
	}
	mutex_unlock(&sound_mutex);
}

int __init snd_minor_info_init(void)
{
	struct snd_info_entry *entry;

	entry = snd_info_create_module_entry(THIS_MODULE, "devices", NULL);
	if (entry) {
		entry->c.text.read = snd_minor_info_read;
		if (snd_info_register(entry) < 0) {
			snd_info_free_entry(entry);
			entry = NULL;
		}
	}
	snd_minor_info_entry = entry;
	return 0;
}

int __exit snd_minor_info_done(void)
{
	snd_info_free_entry(snd_minor_info_entry);
	return 0;
}
#endif /* CONFIG_PROC_FS */

/*
 *  INIT PART
 */

/* 入口函数 */
static int __init alsa_sound_init(void)
{
	snd_major = major; /* snd_major=16 */
	snd_ecards_limit = cards_limit; /* 初始化卡的数量限制，snd_ecards_limit= 1*/
	if (register_chrdev(major, "alsa", &snd_fops)) {  /* 注册字符设备 */
		snd_printk(KERN_ERR "unable to register native major device number %d\n", major);
		return -EIO;
	}
	if (snd_info_init() < 0) {  /* 声卡信息初始化，在info.c中定义 */
		unregister_chrdev(major, "alsa");
		return -ENOMEM;
	}
	snd_info_minor_register();
#ifndef MODULE
	printk(KERN_INFO "Advanced Linux Sound Architecture Driver Version " CONFIG_SND_VERSION CONFIG_SND_DATE ".\n");
#endif
	return 0;
}

static void __exit alsa_sound_exit(void)
{
	snd_info_minor_unregister();
	snd_info_done();
	unregister_chrdev(major, "alsa");
}

module_init(alsa_sound_init)
module_exit(alsa_sound_exit)
