/*
 *	Sound core.  This file is composed of two parts.  sound_class
 *	which is common to both OSS and ALSA and OSS sound core which
 *	is used OSS or emulation of it.
 */

/*
 * First, the common part.
 */
#include <linux/module.h>
#include <linux/device.h>
#include <linux/err.h>
#include <sound/core.h>

#ifdef CONFIG_SOUND_OSS_CORE
static int __init init_oss_soundcore(void);
static void cleanup_oss_soundcore(void);
#else
static inline int init_oss_soundcore(void)	{ return 0; }
static inline void cleanup_oss_soundcore(void)	{ }
#endif

struct class *sound_class;
EXPORT_SYMBOL(sound_class);

MODULE_DESCRIPTION("Core sound module");
MODULE_AUTHOR("Alan Cox");
MODULE_LICENSE("GPL");

static int __init init_soundcore(void)
{
	int rc;

	rc = init_oss_soundcore();  /* 初始化OSS核心-----就是注册了字符设备 */
	if (rc)
		return rc;

	sound_class = class_create(THIS_MODULE, "sound");  /* 创建一个类，即在/sys/class目录下创建sound目录 */
	if (IS_ERR(sound_class)) {
		cleanup_oss_soundcore();
		return PTR_ERR(sound_class);
	}

	return 0;
}

static void __exit cleanup_soundcore(void)
{
	cleanup_oss_soundcore();
	class_destroy(sound_class);
}

module_init(init_soundcore);
module_exit(cleanup_soundcore);


#ifdef CONFIG_SOUND_OSS_CORE
/*
 *	OSS sound core handling. Breaks out sound functions to submodules
 *	
 *	Author:		Alan Cox <alan@lxorguk.ukuu.org.uk>
 *
 *	Fixes:
 *
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *                         --------------------
 * 
 *	Top level handler for the sound subsystem. Various devices can
 *	plug into this. The fact they don't all go via OSS doesn't mean 
 *	they don't have to implement the OSS API. There is a lot of logic
 *	to keeping much of the OSS weight out of the code in a compatibility
 *	module, but it's up to the driver to rember to load it...
 *
 *	The code provides a set of functions for registration of devices
 *	by type. This is done rather than providing a single call so that
 *	we can hide any future changes in the internals (eg when we go to
 *	32bit dev_t) from the modules and their interface.
 *
 *	Secondly we need to allocate the dsp, dsp16 and audio devices as
 *	one. Thus we misuse the chains a bit to simplify this.
 *
 *	Thirdly to make it more fun and for 2.3.x and above we do all
 *	of this using fine grained locking.
 *
 *	FIXME: we have to resolve modules and fine grained load/unload
 *	locking at some point in 2.3.x.
 */

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sound.h>
#include <linux/major.h>
#include <linux/kmod.h>

#define SOUND_STEP 16

struct sound_unit  /* 声音单元 */
{
	int unit_minor;  /* 次设备号 */
	const struct file_operations *unit_fops;  /* 操作函数 */
	struct sound_unit *next;  /* 用于形成链表 */
	char name[32];  /* 名字 */
};

#ifdef CONFIG_SOUND_MSNDCLAS
extern int msnd_classic_init(void);
#endif
#ifdef CONFIG_SOUND_MSNDPIN
extern int msnd_pinnacle_init(void);
#endif

/*
 *	Low level list operator. Scan the ordered list, find a hole and
 *	join into it. Called with the lock asserted
 */
/* 插入声音单元 
    参数list为要插入的节点
    参数list为存放节点的变量，一般为指针数组*chains[i]  (chains[0]---> mixer  chains[2]--->sequencer  chains[2]--->midi   chains[3]--->dsp
                                                                                                                             chains[4]--->audio  chains[8]---> sequencer2   chains[9]--->dmmidi
                                                                                                                               chains[10]--->dmfm   chains[12]--->adsp   chains[13]--->amidi  chains[14]--->admmidi)
                                                                                                                             
    参数index为次设备号
    参数low为最小次设备号
    参数top为最大次设备号*/
static int __sound_insert_unit(struct sound_unit * s, struct sound_unit **list, const struct file_operations *fops, int index, int low, int top)
{
	int n=low;

	if (index < 0) {	/* first free */ /* 如果设备编号<0----->就分配一个次设备号 */

		while (*list && (*list)->unit_minor<n)  /* 遍历以数组chains[i]为头的链表，查找 次设备号<low的节点*/
			list=&((*list)->next); 

		while(n<top)
		{
			/* Found a hole ? */
			if(*list==NULL || (*list)->unit_minor>n)  /* 遍历以数组chains[i]为头的链表，查找 次设备号>low的节点 */
				break;
			list=&((*list)->next);
			n+=SOUND_STEP;  /* n=n+16 */
		}

		if(n>=top)
			return -ENOENT;
	} else {/* 否则 */
		n = low+(index*16);
		while (*list) {
			if ((*list)->unit_minor==n)  /* 查找与计算的设备号相等节点，如果有则说明重复了则返回错误 */
				return -EBUSY;
			if ((*list)->unit_minor>n)
				break;
			list=&((*list)->next);
		}
	}	
		
	/*
	 *	Fill it in
	 */
	 /* 初始化 */
	s->unit_minor=n;
	s->unit_fops=fops;
	
	/*
	 *	Link it
	 */
	 
	s->next=*list;  /* 插入到链表尾部 */
	*list=s;
	
	
	return n;
}

/*
 *	Remove a node from the chain. Called with the lock asserted
 */
 /* 返回次设备号等于unit的节点 */
static struct sound_unit *__sound_remove_unit(struct sound_unit **list, int unit)
{
	while(*list)
	{
		struct sound_unit *p=*list;
		if(p->unit_minor==unit)
		{
			*list=p->next;
			return p;
		}
		list=&(p->next);
	}
	printk(KERN_ERR "Sound device %d went missing!\n", unit);
	return NULL;
}

/*
 *	This lock guards the sound loader list.
 */

static DEFINE_SPINLOCK(sound_loader_lock);

/*
 *	Allocate the controlling structure and add it to the sound driver
 *	list. Acquires locks as needed
 */
/* 插入声音单元设备 
    参数list指向全局数组chains[i]
    参数index为设备编号
    参数low为最小设备编号
    参数top为最大设备编号*/
static int sound_insert_unit(struct sound_unit **list, const struct file_operations *fops, int index, int low, int top, const char *name, umode_t mode, struct device *dev)
{
	struct sound_unit *s = kmalloc(sizeof(*s), GFP_KERNEL);  /* 分配一个struct sound_unit */
	int r;

	if (!s)
		return -ENOMEM;
		
	spin_lock(&sound_loader_lock);
	r = __sound_insert_unit(s, list, fops, index, low, top);  /* 将s插入链表并且分配设备号，并将此设备号返回 */
	spin_unlock(&sound_loader_lock);
	
	if (r < 0)
		goto fail;
	else if (r < SOUND_STEP)
		sprintf(s->name, "sound/%s", name);
	else
		sprintf(s->name, "sound/%s%d", name, r / SOUND_STEP);

	device_create(sound_class, dev, MKDEV(SOUND_MAJOR, s->unit_minor),  /* 创建设备节点 */
		      NULL, s->name+6);
	return r;

 fail:
	kfree(s);
	return r;
}

/*
 *	Remove a unit. Acquires locks as needed. The drivers MUST have
 *	completed the removal before their file operations become
 *	invalid.
 */
 /* 删除一个音频单元 */	
static void sound_remove_unit(struct sound_unit **list, int unit)
{
	struct sound_unit *p;

	spin_lock(&sound_loader_lock);
	p = __sound_remove_unit(list, unit);   /* 获取与次设备号与unit相等的节点 */
	spin_unlock(&sound_loader_lock);
	if (p) {
		device_destroy(sound_class, MKDEV(SOUND_MAJOR, p->unit_minor));  /* 销毁设备文件 */
		kfree(p);   /* 释放节点 */
	}
}

/*
 *	Allocations
 *
 *	0	*16		Mixers
 *	1	*8		Sequencers
 *	2	*16		Midi
 *	3	*16		DSP
 *	4	*16		SunDSP
 *	5	*16		DSP16
 *	6	--		sndstat (obsolete)
 *	7	*16		unused
 *	8	--		alternate sequencer (see above)
 *	9	*16		raw synthesizer access
 *	10	*16		unused
 *	11	*16		unused
 *	12	*16		unused
 *	13	*16		unused
 *	14	*16		unused
 *	15	*16		unused
 */

static struct sound_unit *chains[SOUND_STEP];  /* 全局的变量，注册的设备将存放以这个数组项为链表头的链表中 */

/**
 *	register_sound_special_device - register a special sound node
 *	@fops: File operations for the driver
 *	@unit: Unit number to allocate
 *      @dev: device pointer
 *
 *	Allocate a special sound device by minor number from the sound
 *	subsystem. The allocated number is returned on succes. On failure
 *	a negative error code is returned.
 */
 /* 注册一个特殊的音频设备 */
int register_sound_special_device(const struct file_operations *fops, int unit,
				  struct device *dev)
{
	const int chain = unit % SOUND_STEP;  /* unit % 16 取余 */
	int max_unit = 128 + chain;
	const char *name;
	char _name[16];

	switch (chain) {
	    case 0:
		name = "mixer";  /* 音频设备 */
		break;
	    case 1:
		name = "sequencer";  /* 音序设备 */
		if (unit >= SOUND_STEP)
			goto __unknown;
		max_unit = unit + 1;
		break;
	    case 2:
		name = "midi";  /* 迷笛设备 */
		break;
	    case 3:
		name = "dsp";  /* 音频编解码器 */
		break;
	    case 4:
		name = "audio";  /* 音频设备 */
		break;
	    case 8:
		name = "sequencer2";
		if (unit >= SOUND_STEP)
			goto __unknown;
		max_unit = unit + 1;
		break;
	    case 9:
		name = "dmmidi";
		break;
	    case 10:
		name = "dmfm";
		break;
	    case 12:
		name = "adsp";
		break;
	    case 13:
		name = "amidi";
		break;
	    case 14:
		name = "admmidi";
		break;
	    default:
	    	{
		    __unknown:
			sprintf(_name, "unknown%d", chain);
		    	if (unit >= SOUND_STEP)
		    		strcat(_name, "-");
		    	name = _name;
		}
		break;
	}
	return sound_insert_unit(&chains[chain], fops, -1, unit, max_unit,
				 name, S_IRUSR | S_IWUSR, dev);  /* 插入设备 */
}
 
EXPORT_SYMBOL(register_sound_special_device);

/* 注册一个特殊的音频设备 */
int register_sound_special(const struct file_operations *fops, int unit)
{
	return register_sound_special_device(fops, unit, NULL);
}

EXPORT_SYMBOL(register_sound_special);

/**
 *	register_sound_mixer - register a mixer device
 *	@fops: File operations for the driver
 *	@dev: Unit number to allocate
 *
 *	Allocate a mixer device. Unit is the number of the mixer requested.
 *	Pass -1 to request the next free mixer unit. On success the allocated
 *	number is returned, on failure a negative error code is returned.
 */

/* 注册一个混音器，dev为设备编号，如果dev=-1，则系统自动分配一个设备编号。
    mixer是一个典型的字符设备，因此编码的主要工作时实现 file_operations 中的open、ioctl等函数；
    mixer接口中的最重要函数是ioctl，它实现混音器的不同控制命令*/
int register_sound_mixer(const struct file_operations *fops, int dev)
{
	return sound_insert_unit(&chains[0], fops, dev, 0, 128,
				 "mixer", S_IRUSR | S_IWUSR, NULL);  /*0<=dev<=128 */
}

EXPORT_SYMBOL(register_sound_mixer);

/**
 *	register_sound_midi - register a midi device
 *	@fops: File operations for the driver
 *	@dev: Unit number to allocate
 *
 *	Allocate a midi device. Unit is the number of the midi device requested.
 *	Pass -1 to request the next free midi unit. On success the allocated
 *	number is returned, on failure a negative error code is returned.
 */
/* 注册一个迷笛设备 */
int register_sound_midi(const struct file_operations *fops, int dev)
{
	return sound_insert_unit(&chains[2], fops, dev, 2, 130,
				 "midi", S_IRUSR | S_IWUSR, NULL);
}

EXPORT_SYMBOL(register_sound_midi);

/*
 *	DSP's are registered as a triple. Register only one and cheat
 *	in open - see below.
 */
 
/**
 *	register_sound_dsp - register a DSP device
 *	@fops: File operations for the driver
 *	@dev: Unit number to allocate
 *
 *	Allocate a DSP device. Unit is the number of the DSP requested.
 *	Pass -1 to request the next free DSP unit. On success the allocated
 *	number is returned, on failure a negative error code is returned.
 *
 *	This function allocates both the audio and dsp device entries together
 *	and will always allocate them as a matching pair - eg dsp3/audio3
 */
/* 注册一个音频编解码设备 ，dev为设备编号，如果dev=-1，则系统自动分配一个设备编号。
    dsp是一个典型的字符设备，因此编码的主要工作时实现 file_operations 中的open、read、write、ioctl等函数；
    dsp接口file_operations中read和write函数非常重要:
    read函数从音频控制器中获取录音数据到缓冲区并复制到用户空间
    write函数从用户空间复制音频数据到内核空间缓冲区并最终发送到音频控制器
    ioctl函数用于处理对采样率、量化精度、DMA缓冲区块大小等参数设置I/O控制命令的处理*/
int register_sound_dsp(const struct file_operations *fops, int dev)
{
	return sound_insert_unit(&chains[3], fops, dev, 3, 131,
				 "dsp", S_IWUSR | S_IRUSR, NULL);
}

EXPORT_SYMBOL(register_sound_dsp);

/**
 *	unregister_sound_special - unregister a special sound device
 *	@unit: unit number to allocate
 *
 *	Release a sound device that was allocated with
 *	register_sound_special(). The unit passed is the return value from
 *	the register function.
 */

/* 注销一个特殊的声音设备 */
void unregister_sound_special(int unit)
{
	sound_remove_unit(&chains[unit % SOUND_STEP], unit);
}
 
EXPORT_SYMBOL(unregister_sound_special);

/**
 *	unregister_sound_mixer - unregister a mixer
 *	@unit: unit number to allocate
 *
 *	Release a sound device that was allocated with register_sound_mixer().
 *	The unit passed is the return value from the register function.
 */
/* 注销混音器 */
void unregister_sound_mixer(int unit)
{
	sound_remove_unit(&chains[0], unit);
}

EXPORT_SYMBOL(unregister_sound_mixer);

/**
 *	unregister_sound_midi - unregister a midi device
 *	@unit: unit number to allocate
 *
 *	Release a sound device that was allocated with register_sound_midi().
 *	The unit passed is the return value from the register function.
 */
/* 注销迷笛设备 */
void unregister_sound_midi(int unit)
{
	sound_remove_unit(&chains[2], unit);
}

EXPORT_SYMBOL(unregister_sound_midi);

/**
 *	unregister_sound_dsp - unregister a DSP device
 *	@unit: unit number to allocate
 *
 *	Release a sound device that was allocated with register_sound_dsp().
 *	The unit passed is the return value from the register function.
 *
 *	Both of the allocated units are released together automatically.
 */

/* 注销 编解码设备*/
void unregister_sound_dsp(int unit)
{
	sound_remove_unit(&chains[3], unit);
}


EXPORT_SYMBOL(unregister_sound_dsp);

/*
 *	Now our file operations
 */

static int soundcore_open(struct inode *, struct file *);

static const struct file_operations soundcore_fops=
{
	/* We must have an owner or the module locking fails */
	.owner	= THIS_MODULE,
	.open	= soundcore_open,
};

/* 寻找音频单元 */
static struct sound_unit *__look_for_unit(int chain, int unit)
{
	struct sound_unit *s;
	
	s=chains[chain];
	while(s && s->unit_minor <= unit)
	{
		if(s->unit_minor==unit)
			return s;
		s=s->next;
	}
	return NULL;
}

/* 打开函数 */
static int soundcore_open(struct inode *inode, struct file *file)
{
	int chain;
	int unit = iminor(inode);   /* 获取次设备号 */
	struct sound_unit *s;
	const struct file_operations *new_fops = NULL;

	lock_kernel ();

	chain=unit&0x0F;  /* 取出次设备号的低4位 */
	if(chain==4 || chain==5)	/* dsp/audio/dsp16 */
	{
		unit&=0xF0;
		unit|=3;
		chain=3;
	}
	
	spin_lock(&sound_loader_lock);
	s = __look_for_unit(chain, unit);  /* 寻找音频单元 */
	if (s)
		new_fops = fops_get(s->unit_fops);  /* 获取寻找找的音频单元的操作函数集 */
	if (!new_fops) {
		spin_unlock(&sound_loader_lock);
		/*
		 *  Please, don't change this order or code.
		 *  For ALSA slot means soundcard and OSS emulation code
		 *  comes as add-on modules which aren't depend on
		 *  ALSA toplevel modules for soundcards, thus we need
		 *  load them at first.	  [Jaroslav Kysela <perex@jcu.cz>]
		 */
		request_module("sound-slot-%i", unit>>4);
		request_module("sound-service-%i-%i", unit>>4, chain);
		spin_lock(&sound_loader_lock);
		s = __look_for_unit(chain, unit);  /* 尝试重新获取一次 */
		if (s)
			new_fops = fops_get(s->unit_fops);
	}
	if (new_fops) {
		/*
		 * We rely upon the fact that we can't be unloaded while the
		 * subdriver is there, so if ->open() is successful we can
		 * safely drop the reference counter and if it is not we can
		 * revert to old ->f_op. Ugly, indeed, but that's the cost of
		 * switching ->f_op in the first place.
		 */
		int err = 0;
		const struct file_operations *old_fops = file->f_op;
		file->f_op = new_fops;  /* 指向获取到的新的操作函数集 */
		spin_unlock(&sound_loader_lock);
		if(file->f_op->open)
			err = file->f_op->open(inode,file);  /* 调用它的open函数 */
		if (err) {
			fops_put(file->f_op);
			file->f_op = fops_get(old_fops);
		}
		fops_put(old_fops);
		unlock_kernel();
		return err;
	}
	spin_unlock(&sound_loader_lock);
	unlock_kernel();
	return -ENODEV;
}

MODULE_ALIAS_CHARDEV_MAJOR(SOUND_MAJOR);

/* 清除oss核心 */
static void cleanup_oss_soundcore(void)
{
	/* We have nothing to really do here - we know the lists must be
	   empty */
	unregister_chrdev(SOUND_MAJOR, "sound");
}

/* 初始化oss 核心 */
static int __init init_oss_soundcore(void)
{
	if (register_chrdev(SOUND_MAJOR, "sound", &soundcore_fops)==-1) {  /* 注册一个字符设备，主设备为14 */
		printk(KERN_ERR "soundcore: sound device already in use.\n");
		return -EBUSY;
	}

	return 0;
}

#endif /* CONFIG_SOUND_OSS_CORE */
