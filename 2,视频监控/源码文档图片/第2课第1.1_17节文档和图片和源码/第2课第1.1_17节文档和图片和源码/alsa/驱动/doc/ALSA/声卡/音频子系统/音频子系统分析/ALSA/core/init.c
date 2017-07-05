/*
 *  Initialization routines
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
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/ctype.h>
#include <linux/pm.h>

#include <sound/core.h>
#include <sound/control.h>
#include <sound/info.h>

static DEFINE_SPINLOCK(shutdown_lock);
static LIST_HEAD(shutdown_files);

static const struct file_operations snd_shutdown_f_ops;

static unsigned int snd_cards_lock;	/* locked for registering/using */ /* 锁定注册或者使用的卡 ----如果卡被锁定则将snd_cards_lock的minor位
                                                                                                              设置为1，minor为声卡的次设备号*/
struct snd_card *snd_cards[SNDRV_CARDS];  /* 所有的声卡均被放到该数组中 */
EXPORT_SYMBOL(snd_cards);

static DEFINE_MUTEX(snd_card_mutex);

static char *slots[SNDRV_CARDS];   /* 用于存放模块的名字的全局指针数组 */
module_param_array(slots, charp, NULL, 0444);
MODULE_PARM_DESC(slots, "Module names assigned to the slots.");

/* return non-zero if the given index is reserved for the given
 * module via slots option
 */
 /* 模块位置匹配，如果给定的idx是预留的返回非0 */
static int module_slot_match(struct module *module, int idx)
{
	int match = 1;
#ifdef MODULE
	const char *s1, *s2;

	if (!module || !module->name || !slots[idx])
		return 0;

	s1 = module->name;
	s2 = slots[idx];
	if (*s2 == '!') {
		match = 0; /* negative match */
		s2++;
	}
	/* compare module name strings
	 * hyphens are handled as equivalent with underscore
	 */
	for (;;) {
		char c1 = *s1++;
		char c2 = *s2++;
		if (c1 == '-')
			c1 = '_';
		if (c2 == '-')
			c2 = '_';
		if (c1 != c2)
			return !match;
		if (!c1)
			break;
	}
#endif /* MODULE */
	return match;
}

#if defined(CONFIG_SND_MIXER_OSS) || defined(CONFIG_SND_MIXER_OSS_MODULE)
int (*snd_mixer_oss_notify_callback)(struct snd_card *card, int free_flag);
EXPORT_SYMBOL(snd_mixer_oss_notify_callback);
#endif

#ifdef CONFIG_PROC_FS
static void snd_card_id_read(struct snd_info_entry *entry,
			     struct snd_info_buffer *buffer)
{
	snd_iprintf(buffer, "%s\n", entry->card->id);
}

/* 为声卡初始化信息 */
static inline int init_info_for_card(struct snd_card *card)
{
	int err;
	struct snd_info_entry *entry;

	if ((err = snd_info_card_register(card)) < 0) {
		snd_printd("unable to create card info\n");
		return err;
	}
	if ((entry = snd_info_create_card_entry(card, "id", card->proc_root)) == NULL) {
		snd_printd("unable to create card entry\n");
		return err;
	}
	entry->c.text.read = snd_card_id_read;
	if (snd_info_register(entry) < 0) {
		snd_info_free_entry(entry);
		entry = NULL;
	}
	card->proc_id = entry;
	return 0;
}
#else /* !CONFIG_PROC_FS */
#define init_info_for_card(card)
#endif

/**
 *  snd_card_create - create and initialize a soundcard structure
 *  @idx: card index (address) [0 ... (SNDRV_CARDS-1)]
 *  @xid: card identification (ASCII string)
 *  @module: top level module for locking
 *  @extra_size: allocate this extra size after the main soundcard structure
 *  @card_ret: the pointer to store the created card instance
 *
 *  Creates and initializes a soundcard structure.
 *
 *  The function allocates snd_card instance via kzalloc with the given
 *  space for the driver to use freely.  The allocated struct is stored
 *  in the given card_ret pointer.
 *
 *  Returns zero if successful or a negative error code.
 */
 /* 创建并初始化一个声卡结构
     参数idx为索引号，小于0则会自动分配，取值范围为0 ... (SNDRV_CARDS-1)
     参数id为标识的字符串----card->id= xid
     参数module一般指向THIS_MODULE
     参数extra_size是要分配的额外的数据的大小，分配的extra_size大小的内存将作为card->private_data
     参数card_ret用于存储创建和初始化的声卡结构*/
int snd_card_create(int idx, const char *xid,
		    struct module *module, int extra_size,
		    struct snd_card **card_ret)
{
	struct snd_card *card;
	int err, idx2;

	if (snd_BUG_ON(!card_ret))
		return -EINVAL;
	*card_ret = NULL;

	if (extra_size < 0)
		extra_size = 0;
	card = kzalloc(sizeof(*card) + extra_size, GFP_KERNEL);  /* 分配一个 struct snd_card结构*/
	if (!card)
		return -ENOMEM;
	if (xid) {
		if (!snd_info_check_reserved_words(xid)) {  /*将参数xid与 "version" ,"meminfo","memdebug","detect","devices","oss","cards","timers","synth","pcm","seq","card"
		                                                                             这些字符比较，如果符合这些字符串中的一个，则返回0，否则1*/
			snd_printk(KERN_ERR
				   "given id string '%s' is reserved.\n", xid);
			err = -EBUSY;
			goto __error;
		}
		strlcpy(card->id, xid, sizeof(card->id));  /* card->id= xid*/
	}
	err = 0;
	mutex_lock(&snd_card_mutex);
	if (idx < 0) {  /* 如果索引号idx 小于0*/
		for (idx2 = 0; idx2 < SNDRV_CARDS; idx2++)
			/* idx == -1 == 0xffff means: take any free slot */
			if (~snd_cards_lock & idx & 1<<idx2) {
				if (module_slot_match(module, idx2)) {  /* 如果 module->name=slots[idx]则匹配，否则不匹配*/
					idx = idx2;
					break;
				}
			}
	}
	if (idx < 0) {
		for (idx2 = 0; idx2 < SNDRV_CARDS; idx2++)
			/* idx == -1 == 0xffff means: take any free slot */
			if (~snd_cards_lock & idx & 1<<idx2) {
				if (!slots[idx2] || !*slots[idx2]) {
					idx = idx2;
					break;
				}
			}
	}
	if (idx < 0)
		err = -ENODEV;
	else if (idx < snd_ecards_limit) {
		if (snd_cards_lock & (1 << idx))
			err = -EBUSY;	/* invalid */
	} else if (idx >= SNDRV_CARDS)
		err = -ENODEV;
	if (err < 0) {
		mutex_unlock(&snd_card_mutex);
		snd_printk(KERN_ERR "cannot find the slot for index %d (range 0-%i), error: %d\n",
			 idx, snd_ecards_limit - 1, err);
		goto __error;
	}
	snd_cards_lock |= 1 << idx;		/* lock it */  /*锁定 */
	if (idx >= snd_ecards_limit)
		snd_ecards_limit = idx + 1; /* increase the limit */
	mutex_unlock(&snd_card_mutex);

	/* 初始化struct snd_card成员 */
	card->number = idx;
	card->module = module;
	INIT_LIST_HEAD(&card->devices);
	init_rwsem(&card->controls_rwsem);
	rwlock_init(&card->ctl_files_rwlock);
	INIT_LIST_HEAD(&card->controls);
	INIT_LIST_HEAD(&card->ctl_files);
	spin_lock_init(&card->files_lock);
	INIT_LIST_HEAD(&card->files_list);
	init_waitqueue_head(&card->shutdown_sleep);
#ifdef CONFIG_PM
	mutex_init(&card->power_lock);
	init_waitqueue_head(&card->power_sleep);
#endif
	/* the control interface cannot be accessed from the user space until */
	/* snd_cards_bitmask and snd_cards are set with snd_card_register */
	err = snd_ctl_create(card);  /* 创建声卡的控制核心 */
	if (err < 0) {
		snd_printk(KERN_ERR "unable to register control minors\n");
		goto __error;
	}
	err = snd_info_card_create(card);  /* 在proc文件系统里创建声卡相关的文件 */
	if (err < 0) {
		snd_printk(KERN_ERR "unable to create card info\n");
		goto __error_ctl;
	}
	if (extra_size > 0)
		card->private_data = (char *)card + sizeof(struct snd_card);
	*card_ret = card;
	return 0;

      __error_ctl:
	snd_device_free_all(card, SNDRV_DEV_CMD_PRE);
      __error:
	kfree(card);
  	return err;
}
EXPORT_SYMBOL(snd_card_create);

/* return non-zero if a card is already locked */
/* 获取声卡的锁 */
int snd_card_locked(int card)
{
	int locked;

	mutex_lock(&snd_card_mutex);
	locked = snd_cards_lock & (1 << card);  
	mutex_unlock(&snd_card_mutex);
	return locked;
}

static loff_t snd_disconnect_llseek(struct file *file, loff_t offset, int orig)
{
	return -ENODEV;
}

static ssize_t snd_disconnect_read(struct file *file, char __user *buf,
				   size_t count, loff_t *offset)
{
	return -ENODEV;
}

static ssize_t snd_disconnect_write(struct file *file, const char __user *buf,
				    size_t count, loff_t *offset)
{
	return -ENODEV;
}

static int snd_disconnect_release(struct inode *inode, struct file *file)
{
	struct snd_monitor_file *df = NULL, *_df;

	spin_lock(&shutdown_lock);
	list_for_each_entry(_df, &shutdown_files, shutdown_list) {
		if (_df->file == file) {
			df = _df;
			list_del_init(&df->shutdown_list);
			break;
		}
	}
	spin_unlock(&shutdown_lock);

	if (likely(df)) {
		if ((file->f_flags & FASYNC) && df->disconnected_f_op->fasync)
			df->disconnected_f_op->fasync(-1, file, 0);
		return df->disconnected_f_op->release(inode, file);
	}

	panic("%s(%p, %p) failed!", __func__, inode, file);
}

static unsigned int snd_disconnect_poll(struct file * file, poll_table * wait)
{
	return POLLERR | POLLNVAL;
}

static long snd_disconnect_ioctl(struct file *file,
				 unsigned int cmd, unsigned long arg)
{
	return -ENODEV;
}

static int snd_disconnect_mmap(struct file *file, struct vm_area_struct *vma)
{
	return -ENODEV;
}

static int snd_disconnect_fasync(int fd, struct file *file, int on)
{
	return -ENODEV;
}

static const struct file_operations snd_shutdown_f_ops =  /* 关闭声卡操作函数 */
{
	.owner = 	THIS_MODULE,
	.llseek =	snd_disconnect_llseek,
	.read = 	snd_disconnect_read,
	.write =	snd_disconnect_write,
	.release =	snd_disconnect_release,
	.poll =		snd_disconnect_poll,
	.unlocked_ioctl = snd_disconnect_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = snd_disconnect_ioctl,
#endif
	.mmap =		snd_disconnect_mmap,
	.fasync =	snd_disconnect_fasync
};

/**
 *  snd_card_disconnect - disconnect all APIs from the file-operations (user space)
 *  @card: soundcard structure
 *
 *  Disconnects all APIs from the file-operations (user space).
 *
 *  Returns zero, otherwise a negative error code.
 *
 *  Note: The current implementation replaces all active file->f_op with special
 *        dummy file operations (they do nothing except release).
 */
 /* 取消连接 */
int snd_card_disconnect(struct snd_card *card)
{
	struct snd_monitor_file *mfile;
	struct file *file;
	int err;

	if (!card)
		return -EINVAL;

	spin_lock(&card->files_lock);
	if (card->shutdown) {  /* 如果声卡已经关闭则直接返回 */
		spin_unlock(&card->files_lock);
		return 0;
	}
	card->shutdown = 1;  /* 标识声卡已经关闭 */
	spin_unlock(&card->files_lock);

	/* phase 1: disable fops (user space) operations for ALSA API */
	mutex_lock(&snd_card_mutex);
	snd_cards[card->number] = NULL;
	snd_cards_lock &= ~(1 << card->number);  /* 取消声卡的锁 */
	mutex_unlock(&snd_card_mutex);
	
	/* phase 2: replace file->f_op with special dummy operations */
	
	spin_lock(&card->files_lock);
	list_for_each_entry(mfile, &card->files_list, list) {
		file = mfile->file;

		/* it's critical part, use endless loop */
		/* we have no room to fail */
		mfile->disconnected_f_op = mfile->file->f_op;

		spin_lock(&shutdown_lock);
		list_add(&mfile->shutdown_list, &shutdown_files);
		spin_unlock(&shutdown_lock);

		mfile->file->f_op = &snd_shutdown_f_ops;
		fops_get(mfile->file->f_op);
	}
	spin_unlock(&card->files_lock);	

	/* phase 3: notify all connected devices about disconnection */
	/* at this point, they cannot respond to any calls except release() */

#if defined(CONFIG_SND_MIXER_OSS) || defined(CONFIG_SND_MIXER_OSS_MODULE)
	if (snd_mixer_oss_notify_callback)
		snd_mixer_oss_notify_callback(card, SND_MIXER_OSS_NOTIFY_DISCONNECT);
#endif

	/* notify all devices that we are disconnected */
	err = snd_device_disconnect_all(card); /* 取消声卡上所有设备同声卡的连接 */
	if (err < 0)
		snd_printk(KERN_ERR "not all devices for card %i can be disconnected\n", card->number);

	snd_info_card_disconnect(card);
#ifndef CONFIG_SYSFS_DEPRECATED
	if (card->card_dev) {
		device_unregister(card->card_dev);  /* 注销设备 */
		card->card_dev = NULL;
	}
#endif
#ifdef CONFIG_PM
	wake_up(&card->power_sleep);  
#endif
	return 0;	
}

EXPORT_SYMBOL(snd_card_disconnect);

/**
 *  snd_card_free - frees given soundcard structure
 *  @card: soundcard structure
 *
 *  This function releases the soundcard structure and the all assigned
 *  devices automatically.  That is, you don't have to release the devices
 *  by yourself.
 *
 *  Returns zero. Frees all associated devices and frees the control
 *  interface associated to given soundcard.
 */
 /* 释放给定的card结构 */
static int snd_card_do_free(struct snd_card *card)
{
#if defined(CONFIG_SND_MIXER_OSS) || defined(CONFIG_SND_MIXER_OSS_MODULE)
	if (snd_mixer_oss_notify_callback)
		snd_mixer_oss_notify_callback(card, SND_MIXER_OSS_NOTIFY_FREE);
#endif 
       /* 释放声卡上的所有设备 */
	if (snd_device_free_all(card, SNDRV_DEV_CMD_PRE) < 0) { 
		snd_printk(KERN_ERR "unable to free all devices (pre)\n");
		/* Fatal, but this situation should never occur */
	}
	if (snd_device_free_all(card, SNDRV_DEV_CMD_NORMAL) < 0) {
		snd_printk(KERN_ERR "unable to free all devices (normal)\n");
		/* Fatal, but this situation should never occur */
	}
	if (snd_device_free_all(card, SNDRV_DEV_CMD_POST) < 0) {
		snd_printk(KERN_ERR "unable to free all devices (post)\n");
		/* Fatal, but this situation should never occur */
	}
	if (card->private_free)
		card->private_free(card);  /* 释放私有数据 */
	snd_info_free_entry(card->proc_id);
	if (snd_info_card_free(card) < 0) {  /* 释放声卡在proc文件系统中占有的资源 */
		snd_printk(KERN_WARNING "unable to free card info\n");
		/* Not fatal error */
	}
	kfree(card);  /* 释放卡占有的内存 */
	return 0;
}

/* 当声卡关闭时释放声卡结构 */
int snd_card_free_when_closed(struct snd_card *card)
{
	int free_now = 0;
	int ret = snd_card_disconnect(card);  /* 取消连接 */
	if (ret)
		return ret;

	spin_lock(&card->files_lock);
	if (list_empty(&card->files_list))
		free_now = 1;
	else
		card->free_on_last_close = 1;
	spin_unlock(&card->files_lock);

	if (free_now)
		snd_card_do_free(card);  /* 释放给定的card结构 */
	return 0;
}

EXPORT_SYMBOL(snd_card_free_when_closed);

/* 释放声卡 */
int snd_card_free(struct snd_card *card)
{
	int ret = snd_card_disconnect(card);
	if (ret)
		return ret;

	/* wait, until all devices are ready for the free operation */
	wait_event(card->shutdown_sleep, list_empty(&card->files_list));
	snd_card_do_free(card);
	return 0;
}

EXPORT_SYMBOL(snd_card_free);

/* 选择默认的ID */
static void choose_default_id(struct snd_card *card)
{
	int i, len, idx_flag = 0, loops = SNDRV_CARDS;
	char *id, *spos;
	
	id = spos = card->shortname;	
	while (*id != '\0') {
		if (*id == ' ')
			spos = id + 1;
		id++;
	}
	id = card->id;
	while (*spos != '\0' && !isalnum(*spos))
		spos++;
	if (isdigit(*spos))
		*id++ = isalpha(card->shortname[0]) ? card->shortname[0] : 'D';
	while (*spos != '\0' && (size_t)(id - card->id) < sizeof(card->id) - 1) {
		if (isalnum(*spos))
			*id++ = *spos;
		spos++;
	}
	*id = '\0';

	id = card->id;
	
	if (*id == '\0')
		strcpy(id, "default");

	while (1) {
	      	if (loops-- == 0) {
      			snd_printk(KERN_ERR "unable to choose default card id (%s)\n", id);
      			strcpy(card->id, card->proc_root->name);
      			return;
      		}
	      	if (!snd_info_check_reserved_words(id))
      			goto __change;
		for (i = 0; i < snd_ecards_limit; i++) {
			if (snd_cards[i] && !strcmp(snd_cards[i]->id, id))
				goto __change;
		}
		break;

	      __change:
		len = strlen(id);
		if (idx_flag) {
			if (id[len-1] != '9')
				id[len-1]++;
			else
				id[len-1] = 'A';
		} else if ((size_t)len <= sizeof(card->id) - 3) {
			strcat(id, "_1");
			idx_flag++;
		} else {
			spos = id + len - 2;
			if ((size_t)len <= sizeof(card->id) - 2)
				spos++;
			*spos++ = '_';
			*spos++ = '1';
			*spos++ = '\0';
			idx_flag++;
		}
	}
}

#ifndef CONFIG_SYSFS_DEPRECATED
/* 声卡的设备属性文件ID的操作函数----显示ID */
static ssize_t
card_id_show_attr(struct device *dev,
		  struct device_attribute *attr, char *buf)
{
	struct snd_card *card = dev_get_drvdata(dev);
	return snprintf(buf, PAGE_SIZE, "%s\n", card ? card->id : "(null)");
}

/* 声卡的设备属性文件ID的操作函数----存储ID */
static ssize_t
card_id_store_attr(struct device *dev, struct device_attribute *attr,
		   const char *buf, size_t count)
{
	struct snd_card *card = dev_get_drvdata(dev);
	char buf1[sizeof(card->id)];
	size_t copy = count > sizeof(card->id) - 1 ?
					sizeof(card->id) - 1 : count;
	size_t idx;
	int c;

	for (idx = 0; idx < copy; idx++) {
		c = buf[idx];
		if (!isalnum(c) && c != '_' && c != '-')
			return -EINVAL;
	}
	memcpy(buf1, buf, copy);
	buf1[copy] = '\0';
	mutex_lock(&snd_card_mutex);
	if (!snd_info_check_reserved_words(buf1)) {
	     __exist:
		mutex_unlock(&snd_card_mutex);
		return -EEXIST;
	}
	for (idx = 0; idx < snd_ecards_limit; idx++) {
		if (snd_cards[idx] && !strcmp(snd_cards[idx]->id, buf1))
			goto __exist;
	}
	strcpy(card->id, buf1);
	snd_info_card_id_change(card);
	mutex_unlock(&snd_card_mutex);

	return count;
}

static struct device_attribute card_id_attrs =
	__ATTR(id, S_IRUGO | S_IWUSR, card_id_show_attr, card_id_store_attr);

/* 声卡设备属性文件number的操作函数----显示number */
static ssize_t
card_number_show_attr(struct device *dev,
		     struct device_attribute *attr, char *buf)
{
	struct snd_card *card = dev_get_drvdata(dev);
	return snprintf(buf, PAGE_SIZE, "%i\n", card ? card->number : -1);
}

static struct device_attribute card_number_attrs =
	__ATTR(number, S_IRUGO, card_number_show_attr, NULL);
#endif /* CONFIG_SYSFS_DEPRECATED */

/**
 *  snd_card_register - register the soundcard
 *  @card: soundcard structure
 *
 *  This function registers all the devices assigned to the soundcard.
 *  Until calling this, the ALSA control interface is blocked from the
 *  external accesses.  Thus, you should call this function at the end
 *  of the initialization of the card.
 *
 *  Returns zero otherwise a negative error code if the registrain failed.
 */
 /* 注册声卡 */
int snd_card_register(struct snd_card *card)
{
	int err;

	if (snd_BUG_ON(!card))
		return -EINVAL;
#ifndef CONFIG_SYSFS_DEPRECATED
	if (!card->card_dev) {
		card->card_dev = device_create(sound_class, card->dev,
					       MKDEV(0, 0), card,
					       "card%i", card->number);  /* 创建设备节点card%card->number*/
		if (IS_ERR(card->card_dev))
			card->card_dev = NULL;
	}
#endif
	if ((err = snd_device_register_all(card)) < 0)  /* 注册所有的设备到声卡 */
		return err;
	mutex_lock(&snd_card_mutex);
	if (snd_cards[card->number]) {  /* 如果声卡已经注册则返回 */
		/* already registered */
		mutex_unlock(&snd_card_mutex);
		return 0;
	}
	if (card->id[0] == '\0')
		choose_default_id(card);  /* 选择默认的ID */
	snd_cards[card->number] = card;  /* 将它放入全局数组中 */
	mutex_unlock(&snd_card_mutex);
	init_info_for_card(card); /* 为声卡初始化信息 */
#if defined(CONFIG_SND_MIXER_OSS) || defined(CONFIG_SND_MIXER_OSS_MODULE)
	if (snd_mixer_oss_notify_callback)
		snd_mixer_oss_notify_callback(card, SND_MIXER_OSS_NOTIFY_REGISTER);
#endif
#ifndef CONFIG_SYSFS_DEPRECATED
	if (card->card_dev) {
		err = device_create_file(card->card_dev, &card_id_attrs); /* 创建设备属性文件id */
		if (err < 0)
			return err;
		err = device_create_file(card->card_dev, &card_number_attrs);  /* 创建设备属性文件number */
		if (err < 0)
			return err;
	}
#endif
	return 0;
}

EXPORT_SYMBOL(snd_card_register);

#ifdef CONFIG_PROC_FS
static struct snd_info_entry *snd_card_info_entry;

static void snd_card_info_read(struct snd_info_entry *entry,
			       struct snd_info_buffer *buffer)
{
	int idx, count;
	struct snd_card *card;

	for (idx = count = 0; idx < SNDRV_CARDS; idx++) {
		mutex_lock(&snd_card_mutex);
		if ((card = snd_cards[idx]) != NULL) {
			count++;
			snd_iprintf(buffer, "%2i [%-15s]: %s - %s\n",
					idx,
					card->id,
					card->driver,
					card->shortname);
			snd_iprintf(buffer, "                      %s\n",
					card->longname);
		}
		mutex_unlock(&snd_card_mutex);
	}
	if (!count)
		snd_iprintf(buffer, "--- no soundcards ---\n");
}

#ifdef CONFIG_SND_OSSEMUL

void snd_card_info_read_oss(struct snd_info_buffer *buffer)
{
	int idx, count;
	struct snd_card *card;

	for (idx = count = 0; idx < SNDRV_CARDS; idx++) {
		mutex_lock(&snd_card_mutex);
		if ((card = snd_cards[idx]) != NULL) {
			count++;
			snd_iprintf(buffer, "%s\n", card->longname);
		}
		mutex_unlock(&snd_card_mutex);
	}
	if (!count) {
		snd_iprintf(buffer, "--- no soundcards ---\n");
	}
}

#endif

#ifdef MODULE
static struct snd_info_entry *snd_card_module_info_entry;
static void snd_card_module_info_read(struct snd_info_entry *entry,
				      struct snd_info_buffer *buffer)
{
	int idx;
	struct snd_card *card;

	for (idx = 0; idx < SNDRV_CARDS; idx++) {
		mutex_lock(&snd_card_mutex);
		if ((card = snd_cards[idx]) != NULL)
			snd_iprintf(buffer, "%2i %s\n",
				    idx, card->module->name);
		mutex_unlock(&snd_card_mutex);
	}
}
#endif

int __init snd_card_info_init(void)  /* 被info.c的入口函数调用 */
{
	struct snd_info_entry *entry;

	entry = snd_info_create_module_entry(THIS_MODULE, "cards", NULL);
	if (! entry)
		return -ENOMEM;
	entry->c.text.read = snd_card_info_read;
	if (snd_info_register(entry) < 0) {
		snd_info_free_entry(entry);
		return -ENOMEM;
	}
	snd_card_info_entry = entry;

#ifdef MODULE
	entry = snd_info_create_module_entry(THIS_MODULE, "modules", NULL);
	if (entry) {
		entry->c.text.read = snd_card_module_info_read;
		if (snd_info_register(entry) < 0)
			snd_info_free_entry(entry);
		else
			snd_card_module_info_entry = entry;
	}
#endif

	return 0;
}

int __exit snd_card_info_done(void)
{
	snd_info_free_entry(snd_card_info_entry);
#ifdef MODULE
	snd_info_free_entry(snd_card_module_info_entry);
#endif
	return 0;
}

#endif /* CONFIG_PROC_FS */

/**
 *  snd_component_add - add a component string
 *  @card: soundcard structure
 *  @component: the component id string
 *
 *  This function adds the component id string to the supported list.
 *  The component can be referred from the alsa-lib.
 *
 *  Returns zero otherwise a negative error code.
 */
  /* 添加组件ID */
int snd_component_add(struct snd_card *card, const char *component)
{
	char *ptr;
	int len = strlen(component);

	ptr = strstr(card->components, component);
	if (ptr != NULL) {
		if (ptr[len] == '\0' || ptr[len] == ' ')	/* already there */
			return 1;
	}
	if (strlen(card->components) + 1 + len + 1 > sizeof(card->components)) {
		snd_BUG();
		return -ENOMEM;
	}
	if (card->components[0] != '\0')
		strcat(card->components, " ");
	strcat(card->components, component);
	return 0;
}

EXPORT_SYMBOL(snd_component_add);

/**
 *  snd_card_file_add - add the file to the file list of the card
 *  @card: soundcard structure
 *  @file: file pointer
 *
 *  This function adds the file to the file linked-list of the card.
 *  This linked-list is used to keep tracking the connection state,
 *  and to avoid the release of busy resources by hotplug.
 *
 *  Returns zero or a negative error code.
 */
int snd_card_file_add(struct snd_card *card, struct file *file)
{
	struct snd_monitor_file *mfile;

	mfile = kmalloc(sizeof(*mfile), GFP_KERNEL);
	if (mfile == NULL)
		return -ENOMEM;
	mfile->file = file;
	mfile->disconnected_f_op = NULL;
	spin_lock(&card->files_lock);
	if (card->shutdown) {
		spin_unlock(&card->files_lock);
		kfree(mfile);
		return -ENODEV;
	}
	list_add(&mfile->list, &card->files_list);
	spin_unlock(&card->files_lock);
	return 0;
}

EXPORT_SYMBOL(snd_card_file_add);

/**
 *  snd_card_file_remove - remove the file from the file list
 *  @card: soundcard structure
 *  @file: file pointer
 *
 *  This function removes the file formerly added to the card via
 *  snd_card_file_add() function.
 *  If all files are removed and snd_card_free_when_closed() was
 *  called beforehand, it processes the pending release of
 *  resources.
 *
 *  Returns zero or a negative error code.
 */
 /* 从file链表中删除file */
int snd_card_file_remove(struct snd_card *card, struct file *file)
{
	struct snd_monitor_file *mfile, *found = NULL;
	int last_close = 0;

	spin_lock(&card->files_lock);
	list_for_each_entry(mfile, &card->files_list, list) {  /* 遍历card->files_list链表 */
		if (mfile->file == file) {
			list_del(&mfile->list); /* 如果mfile->file == file则将它从链表中删除 */
			if (mfile->disconnected_f_op)
				fops_put(mfile->disconnected_f_op);
			found = mfile;
			break;
		}
	}
	if (list_empty(&card->files_list))  /* 如果 card->files_list为空*/
		last_close = 1;
	spin_unlock(&card->files_lock);
	if (last_close) {
		wake_up(&card->shutdown_sleep);  /* 缓存 card->shutdown_sleep队列上的进程*/
		if (card->free_on_last_close)
			snd_card_do_free(card);
	}
	if (!found) {
		snd_printk(KERN_ERR "ALSA card file remove problem (%p)\n", file);
		return -ENOENT;
	}
	kfree(found);
	return 0;
}

EXPORT_SYMBOL(snd_card_file_remove);

#ifdef CONFIG_PM
/**
 *  snd_power_wait - wait until the power-state is changed.
 *  @card: soundcard structure
 *  @power_state: expected power state
 *
 *  Waits until the power-state is changed.
 *
 *  Note: the power lock must be active before call.
 */
 /* 等待直到电源状态改变
     参数power_state为期望的电源状态*/
int snd_power_wait(struct snd_card *card, unsigned int power_state)
{
	wait_queue_t wait;
	int result = 0;

	/* fastpath */
	if (snd_power_get_state(card) == power_state)  /* card->power_state == power_state? */
		return 0;
	init_waitqueue_entry(&wait, current);  /* 初始化等待队列 */
	add_wait_queue(&card->power_sleep, &wait);  /* 将等待队列添加到card->power_sleep等待队列头之后 */
	while (1) {
		if (card->shutdown) {
			result = -ENODEV;
			break;
		}
		if (snd_power_get_state(card) == power_state) /* card->power_state == power_state? */
			break;
		set_current_state(TASK_UNINTERRUPTIBLE); 
		snd_power_unlock(card);
		schedule_timeout(30 * HZ);/*睡眠30s  */
		snd_power_lock(card);
	}
	remove_wait_queue(&card->power_sleep, &wait);  /* 将wait从等待队列中移除 */
	return result;
}

EXPORT_SYMBOL(snd_power_wait);
#endif /* CONFIG_PM */
