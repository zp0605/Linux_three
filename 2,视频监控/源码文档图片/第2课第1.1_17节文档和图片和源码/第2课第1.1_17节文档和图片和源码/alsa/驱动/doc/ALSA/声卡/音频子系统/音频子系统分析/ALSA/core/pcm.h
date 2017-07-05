#ifndef __SOUND_PCM_H
#define __SOUND_PCM_H

/*
 *  Digital Audio (PCM) abstract layer
 *  Copyright (c) by Jaroslav Kysela <perex@perex.cz>
 *                   Abramo Bagnara <abramo@alsa-project.org>
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

#include <sound/asound.h>
#include <sound/memalloc.h>
#include <sound/minors.h>
#include <linux/poll.h>
#include <linux/mm.h>
#include <linux/bitops.h>

#define snd_pcm_substream_chip(substream) ((substream)->private_data)  /* 获取保存到substream->private_data的数据 */
#define snd_pcm_chip(pcm) ((pcm)->private_data) /* 获取保存到pcm->private_data的 数据*/

#if defined(CONFIG_SND_PCM_OSS) || defined(CONFIG_SND_PCM_OSS_MODULE)
#include "pcm_oss.h"
#endif

/*
 *  Hardware (lowlevel) section
 */

struct snd_pcm_hardware {  /* PCM硬件 */
	unsigned int info;		/* SNDRV_PCM_INFO_* */        /* 该字段标识PCM设备的类型和能力，至少需要定义释放支持mmap，
	                                                                                       查看宏SNDRV_PCM_INFO_* */
	u64 formats;			/* SNDRV_PCM_FMTBIT_* */     /* 该字段包含PCM设备支持的格式，查看宏SNDRV_PCM_FMTBIT_*  */
	unsigned int rates;		/* SNDRV_PCM_RATE_* */ /* 该字段包含了PCM设备支持的采样率，查看宏SNDRV_PCM_RATE_*  */
	unsigned int rate_min;		/* min rate */  /* 最小采样率 */
	unsigned int rate_max;		/* max rate */ /* 最大采样率 */
	unsigned int channels_min;	/* min channels */  /* 最小通道数 */
	unsigned int channels_max;	/* max channels */  /* 最大通道数 */
	size_t buffer_bytes_max;	/* max buffer size */  /* 缓冲区最大尺寸 */

	/* 与OSS的fragment对应，定义了PCM中断产生的周期 */
	size_t period_bytes_min;	/* min period size */  /* 最小周期*/
	size_t period_bytes_max;	/* max period size */ /* 最大周期*/
	unsigned int periods_min;	/* min # of periods */ /* 最小周期数 */
	unsigned int periods_max;	/* max # of periods */ /* 最大周期数 */
	size_t fifo_size;		/* fifo size in bytes */ /* FIFO大小 */
};

struct snd_pcm_substream;

struct snd_pcm_ops {  /* PCM操作函数 */
	int (*open)(struct snd_pcm_substream *substream); /* 打开子流 (在这个函数中至少需要初始化runtime->hw)*/
	int (*close)(struct snd_pcm_substream *substream); /* 关闭子流 */
	int (*ioctl)(struct snd_pcm_substream * substream,
		     unsigned int cmd, void *arg); /* 控制子流 ----通常设置为通用的snd_pcm_lib_ioctl*/
	int (*hw_params)(struct snd_pcm_substream *substream,
			 struct snd_pcm_hw_params *params); /* 设置硬件参数，该函数将在应用程序设置硬件参数的时候被调用 */
	int (*hw_free)(struct snd_pcm_substream *substream); /* 资源释放 */
	int (*prepare)(struct snd_pcm_substream *substream); /* 为子流做准备 */
	int (*trigger)(struct snd_pcm_substream *substream, int cmd); /* 在PCM被开始、停止或暂停时调用 ，该函数是原子的，中途不能睡眠，
	                                                                                                   cmd可取SNDRV_PCM_TRIGGER_XXX*/
	snd_pcm_uframes_t (*pointer)(struct snd_pcm_substream *substream); /*用于PCM中间层查询目前缓冲区的硬件位置*/
	int (*copy)(struct snd_pcm_substream *substream, int channel,
		    snd_pcm_uframes_t pos,
		    void __user *buf, snd_pcm_uframes_t count);  /* 复制缓冲区----一般可以省略 */
	int (*silence)(struct snd_pcm_substream *substream, int channel, 
		       snd_pcm_uframes_t pos, snd_pcm_uframes_t count);  /*一般可以省略 */
	struct page *(*page)(struct snd_pcm_substream *substream,
			     unsigned long offset);
	int (*mmap)(struct snd_pcm_substream *substream, struct vm_area_struct *vma); /* 映射 */
	int (*ack)(struct snd_pcm_substream *substream); /* 应答 */
};

/*
 *
 */

#if defined(CONFIG_SND_DYNAMIC_MINORS)
#define SNDRV_PCM_DEVICES	(SNDRV_OS_MINORS-2)
#else
#define SNDRV_PCM_DEVICES	8
#endif

#define SNDRV_PCM_IOCTL1_FALSE		((void *)0)
#define SNDRV_PCM_IOCTL1_TRUE		((void *)1)

#define SNDRV_PCM_IOCTL1_RESET		0
#define SNDRV_PCM_IOCTL1_INFO		1
#define SNDRV_PCM_IOCTL1_CHANNEL_INFO	2
#define SNDRV_PCM_IOCTL1_GSTATE		3

/* trigger函数的命令 */
#define SNDRV_PCM_TRIGGER_STOP		0
#define SNDRV_PCM_TRIGGER_START		1
#define SNDRV_PCM_TRIGGER_PAUSE_PUSH	3
#define SNDRV_PCM_TRIGGER_PAUSE_RELEASE	4
#define SNDRV_PCM_TRIGGER_SUSPEND	5
#define SNDRV_PCM_TRIGGER_RESUME	6

#define SNDRV_PCM_POS_XRUN		((snd_pcm_uframes_t)-1)

/* If you change this don't forget to change rates[] table in pcm_native.c */
#define SNDRV_PCM_RATE_5512		(1<<0)		/* 5512Hz */
#define SNDRV_PCM_RATE_8000		(1<<1)		/* 8000Hz */
#define SNDRV_PCM_RATE_11025		(1<<2)		/* 11025Hz */
#define SNDRV_PCM_RATE_16000		(1<<3)		/* 16000Hz */
#define SNDRV_PCM_RATE_22050		(1<<4)		/* 22050Hz */
#define SNDRV_PCM_RATE_32000		(1<<5)		/* 32000Hz */
#define SNDRV_PCM_RATE_44100		(1<<6)		/* 44100Hz */
#define SNDRV_PCM_RATE_48000		(1<<7)		/* 48000Hz */
#define SNDRV_PCM_RATE_64000		(1<<8)		/* 64000Hz */
#define SNDRV_PCM_RATE_88200		(1<<9)		/* 88200Hz */
#define SNDRV_PCM_RATE_96000		(1<<10)		/* 96000Hz */
#define SNDRV_PCM_RATE_176400		(1<<11)		/* 176400Hz */
#define SNDRV_PCM_RATE_192000		(1<<12)		/* 192000Hz */

#define SNDRV_PCM_RATE_CONTINUOUS	(1<<30)		/* continuous range */
#define SNDRV_PCM_RATE_KNOT		(1<<31)		/* supports more non-continuos rates */

#define SNDRV_PCM_RATE_8000_44100	(SNDRV_PCM_RATE_8000|SNDRV_PCM_RATE_11025|\
					 SNDRV_PCM_RATE_16000|SNDRV_PCM_RATE_22050|\
					 SNDRV_PCM_RATE_32000|SNDRV_PCM_RATE_44100)
#define SNDRV_PCM_RATE_8000_48000	(SNDRV_PCM_RATE_8000_44100|SNDRV_PCM_RATE_48000)
#define SNDRV_PCM_RATE_8000_96000	(SNDRV_PCM_RATE_8000_48000|SNDRV_PCM_RATE_64000|\
					 SNDRV_PCM_RATE_88200|SNDRV_PCM_RATE_96000)
#define SNDRV_PCM_RATE_8000_192000	(SNDRV_PCM_RATE_8000_96000|SNDRV_PCM_RATE_176400|\
					 SNDRV_PCM_RATE_192000)
#define SNDRV_PCM_FMTBIT_S8		(1ULL << SNDRV_PCM_FORMAT_S8)
#define SNDRV_PCM_FMTBIT_U8		(1ULL << SNDRV_PCM_FORMAT_U8)
#define SNDRV_PCM_FMTBIT_S16_LE		(1ULL << SNDRV_PCM_FORMAT_S16_LE)
#define SNDRV_PCM_FMTBIT_S16_BE		(1ULL << SNDRV_PCM_FORMAT_S16_BE)
#define SNDRV_PCM_FMTBIT_U16_LE		(1ULL << SNDRV_PCM_FORMAT_U16_LE)
#define SNDRV_PCM_FMTBIT_U16_BE		(1ULL << SNDRV_PCM_FORMAT_U16_BE)
#define SNDRV_PCM_FMTBIT_S24_LE		(1ULL << SNDRV_PCM_FORMAT_S24_LE)
#define SNDRV_PCM_FMTBIT_S24_BE		(1ULL << SNDRV_PCM_FORMAT_S24_BE)
#define SNDRV_PCM_FMTBIT_U24_LE		(1ULL << SNDRV_PCM_FORMAT_U24_LE)
#define SNDRV_PCM_FMTBIT_U24_BE		(1ULL << SNDRV_PCM_FORMAT_U24_BE)
#define SNDRV_PCM_FMTBIT_S32_LE		(1ULL << SNDRV_PCM_FORMAT_S32_LE)
#define SNDRV_PCM_FMTBIT_S32_BE		(1ULL << SNDRV_PCM_FORMAT_S32_BE)
#define SNDRV_PCM_FMTBIT_U32_LE		(1ULL << SNDRV_PCM_FORMAT_U32_LE)
#define SNDRV_PCM_FMTBIT_U32_BE		(1ULL << SNDRV_PCM_FORMAT_U32_BE)
#define SNDRV_PCM_FMTBIT_FLOAT_LE	(1ULL << SNDRV_PCM_FORMAT_FLOAT_LE)
#define SNDRV_PCM_FMTBIT_FLOAT_BE	(1ULL << SNDRV_PCM_FORMAT_FLOAT_BE)
#define SNDRV_PCM_FMTBIT_FLOAT64_LE	(1ULL << SNDRV_PCM_FORMAT_FLOAT64_LE)
#define SNDRV_PCM_FMTBIT_FLOAT64_BE	(1ULL << SNDRV_PCM_FORMAT_FLOAT64_BE)
#define SNDRV_PCM_FMTBIT_IEC958_SUBFRAME_LE (1ULL << SNDRV_PCM_FORMAT_IEC958_SUBFRAME_LE)
#define SNDRV_PCM_FMTBIT_IEC958_SUBFRAME_BE (1ULL << SNDRV_PCM_FORMAT_IEC958_SUBFRAME_BE)
#define SNDRV_PCM_FMTBIT_MU_LAW		(1ULL << SNDRV_PCM_FORMAT_MU_LAW)
#define SNDRV_PCM_FMTBIT_A_LAW		(1ULL << SNDRV_PCM_FORMAT_A_LAW)
#define SNDRV_PCM_FMTBIT_IMA_ADPCM	(1ULL << SNDRV_PCM_FORMAT_IMA_ADPCM)
#define SNDRV_PCM_FMTBIT_MPEG		(1ULL << SNDRV_PCM_FORMAT_MPEG)
#define SNDRV_PCM_FMTBIT_GSM		(1ULL << SNDRV_PCM_FORMAT_GSM)
#define SNDRV_PCM_FMTBIT_SPECIAL	(1ULL << SNDRV_PCM_FORMAT_SPECIAL)
#define SNDRV_PCM_FMTBIT_S24_3LE	(1ULL << SNDRV_PCM_FORMAT_S24_3LE)
#define SNDRV_PCM_FMTBIT_U24_3LE	(1ULL << SNDRV_PCM_FORMAT_U24_3LE)
#define SNDRV_PCM_FMTBIT_S24_3BE	(1ULL << SNDRV_PCM_FORMAT_S24_3BE)
#define SNDRV_PCM_FMTBIT_U24_3BE	(1ULL << SNDRV_PCM_FORMAT_U24_3BE)
#define SNDRV_PCM_FMTBIT_S20_3LE	(1ULL << SNDRV_PCM_FORMAT_S20_3LE)
#define SNDRV_PCM_FMTBIT_U20_3LE	(1ULL << SNDRV_PCM_FORMAT_U20_3LE)
#define SNDRV_PCM_FMTBIT_S20_3BE	(1ULL << SNDRV_PCM_FORMAT_S20_3BE)
#define SNDRV_PCM_FMTBIT_U20_3BE	(1ULL << SNDRV_PCM_FORMAT_U20_3BE)
#define SNDRV_PCM_FMTBIT_S18_3LE	(1ULL << SNDRV_PCM_FORMAT_S18_3LE)
#define SNDRV_PCM_FMTBIT_U18_3LE	(1ULL << SNDRV_PCM_FORMAT_U18_3LE)
#define SNDRV_PCM_FMTBIT_S18_3BE	(1ULL << SNDRV_PCM_FORMAT_S18_3BE)
#define SNDRV_PCM_FMTBIT_U18_3BE	(1ULL << SNDRV_PCM_FORMAT_U18_3BE)

#ifdef SNDRV_LITTLE_ENDIAN
#define SNDRV_PCM_FMTBIT_S16		SNDRV_PCM_FMTBIT_S16_LE
#define SNDRV_PCM_FMTBIT_U16		SNDRV_PCM_FMTBIT_U16_LE
#define SNDRV_PCM_FMTBIT_S24		SNDRV_PCM_FMTBIT_S24_LE
#define SNDRV_PCM_FMTBIT_U24		SNDRV_PCM_FMTBIT_U24_LE
#define SNDRV_PCM_FMTBIT_S32		SNDRV_PCM_FMTBIT_S32_LE
#define SNDRV_PCM_FMTBIT_U32		SNDRV_PCM_FMTBIT_U32_LE
#define SNDRV_PCM_FMTBIT_FLOAT		SNDRV_PCM_FMTBIT_FLOAT_LE
#define SNDRV_PCM_FMTBIT_FLOAT64	SNDRV_PCM_FMTBIT_FLOAT64_LE
#define SNDRV_PCM_FMTBIT_IEC958_SUBFRAME SNDRV_PCM_FMTBIT_IEC958_SUBFRAME_LE
#endif
#ifdef SNDRV_BIG_ENDIAN
#define SNDRV_PCM_FMTBIT_S16		SNDRV_PCM_FMTBIT_S16_BE
#define SNDRV_PCM_FMTBIT_U16		SNDRV_PCM_FMTBIT_U16_BE
#define SNDRV_PCM_FMTBIT_S24		SNDRV_PCM_FMTBIT_S24_BE
#define SNDRV_PCM_FMTBIT_U24		SNDRV_PCM_FMTBIT_U24_BE
#define SNDRV_PCM_FMTBIT_S32		SNDRV_PCM_FMTBIT_S32_BE
#define SNDRV_PCM_FMTBIT_U32		SNDRV_PCM_FMTBIT_U32_BE
#define SNDRV_PCM_FMTBIT_FLOAT		SNDRV_PCM_FMTBIT_FLOAT_BE
#define SNDRV_PCM_FMTBIT_FLOAT64	SNDRV_PCM_FMTBIT_FLOAT64_BE
#define SNDRV_PCM_FMTBIT_IEC958_SUBFRAME SNDRV_PCM_FMTBIT_IEC958_SUBFRAME_BE
#endif

struct snd_pcm_file {
	struct snd_pcm_substream *substream;
	int no_compat_mmap;
};

struct snd_pcm_hw_rule;
typedef int (*snd_pcm_hw_rule_func_t)(struct snd_pcm_hw_params *params,
				      struct snd_pcm_hw_rule *rule);

struct snd_pcm_hw_rule {
	unsigned int cond;
	snd_pcm_hw_rule_func_t func;
	int var;
	int deps[4];
	void *private;
};

struct snd_pcm_hw_constraints {
	struct snd_mask masks[SNDRV_PCM_HW_PARAM_LAST_MASK - 
			 SNDRV_PCM_HW_PARAM_FIRST_MASK + 1];
	struct snd_interval intervals[SNDRV_PCM_HW_PARAM_LAST_INTERVAL -
			     SNDRV_PCM_HW_PARAM_FIRST_INTERVAL + 1];
	unsigned int rules_num;
	unsigned int rules_all;
	struct snd_pcm_hw_rule *rules;
};

static inline struct snd_mask *constrs_mask(struct snd_pcm_hw_constraints *constrs,
					    snd_pcm_hw_param_t var)
{
	return &constrs->masks[var - SNDRV_PCM_HW_PARAM_FIRST_MASK];
}

static inline struct snd_interval *constrs_interval(struct snd_pcm_hw_constraints *constrs,
						    snd_pcm_hw_param_t var)
{
	return &constrs->intervals[var - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL];
}

struct snd_ratnum {
	unsigned int num;
	unsigned int den_min, den_max, den_step;
};

struct snd_ratden {
	unsigned int num_min, num_max, num_step;
	unsigned int den;
};

struct snd_pcm_hw_constraint_ratnums {
	int nrats;
	struct snd_ratnum *rats;
};

struct snd_pcm_hw_constraint_ratdens {
	int nrats;
	struct snd_ratden *rats;
};

struct snd_pcm_hw_constraint_list { /* PCM硬件约束列表 */
	unsigned int count;
	unsigned int *list;
	unsigned int mask;
};

struct snd_pcm_runtime {  /* PCM运行时 ----当PCM子流被打开后，PCM运行时实例将被分配给这个子流，
	                                              该结构体中大多数记录对被声卡驱动操作集中的函数是只读的*/
	/* -- Status -- */
	struct snd_pcm_substream *trigger_master;  /* 指向触发主要的PCM子流 */
	struct timespec trigger_tstamp;	/* trigger timestamp */ /* 触发时间戳 */
	int overrange;
	snd_pcm_uframes_t avail_max;
	snd_pcm_uframes_t hw_ptr_base;	/* Position at buffer restart */ /* 缓冲区复位时的位置 */
	snd_pcm_uframes_t hw_ptr_interrupt; /* Position at interrupt time */ /* 中断时的位置 */
	unsigned long hw_ptr_jiffies;	/* Time when hw_ptr is updated */

	/* -- HW params -- */
	/* 硬件参数 */
	snd_pcm_access_t access;	/* access mode */ /*存取模式  */
	snd_pcm_format_t format;	/* SNDRV_PCM_FORMAT_* */
	snd_pcm_subformat_t subformat;	/* subformat */ /* 子格式 */
	unsigned int rate;		/* rate in Hz */
	unsigned int channels;		/* channels */ /* 通道 */
	snd_pcm_uframes_t period_size;	/* period size */ /* 周期大小 */
	unsigned int periods;		/* periods */ /* 周期数 */
	snd_pcm_uframes_t buffer_size;	/* buffer size */ /* 缓冲区大小 */
	snd_pcm_uframes_t min_align;	/* Min alignment for the format */ /* 格式对应的最小对齐 */
	size_t byte_align;
	unsigned int frame_bits;
	unsigned int sample_bits;
	unsigned int info;
	unsigned int rate_num;
	unsigned int rate_den;

	/* -- SW params -- */
	int tstamp_mode;		/* mmap timestamp is updated */ /* mmap时间戳被更新 */
  	unsigned int period_step;
	snd_pcm_uframes_t start_threshold;
	snd_pcm_uframes_t stop_threshold;
	snd_pcm_uframes_t silence_threshold; /* Silence filling happens when
						noise is nearest than this */ /* Silence填充阈值 */
	snd_pcm_uframes_t silence_size;	/* Silence filling size */ /* Silence填充大小*/
	snd_pcm_uframes_t boundary;	/* pointers wrap point */

	snd_pcm_uframes_t silence_start; /* starting pointer to silence area */
	snd_pcm_uframes_t silence_filled; /* size filled with silence */

	union snd_pcm_sync_id sync;	/* hardware synchronization ID */ /* 硬件同步ID */

	/* -- mmap -- */
	struct snd_pcm_mmap_status *status;
	struct snd_pcm_mmap_control *control;

	/* -- locking / scheduling -- */
	wait_queue_head_t sleep;
	struct fasync_struct *fasync;

	/* -- private section -- */
	void *private_data;
	void (*private_free)(struct snd_pcm_runtime *runtime);

	/* -- hardware description -- */
	/* 硬件描述 */
	struct snd_pcm_hardware hw;
	struct snd_pcm_hw_constraints hw_constraints;

	/* -- interrupt callbacks -- */
	/* 中断回调函数 */
	void (*transfer_ack_begin)(struct snd_pcm_substream *substream);  
	void (*transfer_ack_end)(struct snd_pcm_substream *substream);

	/* -- timer -- */
	unsigned int timer_resolution;	/* timer resolution */ /* 定时器分辨率 */
	int tstamp_type;		/* timestamp type */

	/* -- DMA -- */           
	unsigned char *dma_area;	/* DMA area */ /* 指向DMA逻辑地址 */
	dma_addr_t dma_addr;		/* physical bus address (not accessible from main CPU) */ /* 指向总线物理地址 */
	size_t dma_bytes;		/* size of DMA area */  /* DMA缓冲区的大小----可以使用snd_pcm_lib_malloc_pages分配缓冲区 */
 
	struct snd_dma_buffer *dma_buffer_p;	/* allocated buffer */  /* DMA缓冲区 */

#if defined(CONFIG_SND_PCM_OSS) || defined(CONFIG_SND_PCM_OSS_MODULE)
	/* -- OSS things -- */
	struct snd_pcm_oss_runtime oss;
#endif
};

struct snd_pcm_group {/* keep linked substreams */ /* PCM组 ，保持和子流的连接 */
	spinlock_t lock;  /*  */
	struct list_head substreams;  /* 子流形成的链表 */
	int count;
};

struct snd_pcm_substream {  /* PCM子流 */
	struct snd_pcm *pcm;  /* 指向所属的PCM */
	struct snd_pcm_str *pstr; /* 所属PCM流=&pcm->streams[i]  */
	void *private_data;		/* copied from pcm->private_data */  /* 私有数据  =pcm->private_data */
	int number;   /* 当前操作是哪个子流 */
	char name[32];			/* substream name */  /* 子流的名字 */
	int stream;			/* stream (direction) */  /* 流传输的方向SNDRV_PCM_STREAM_XXX */
	char latency_id[20];		/* latency identifier */  /* 潜在的标识 */
	size_t buffer_bytes_max;	/* limit ring buffer size */  /* 最大的缓冲区大小 =UINT_MAX */
	struct snd_dma_buffer dma_buffer;   /* DMA缓冲区 */
	unsigned int dma_buf_id;  /* DMA缓冲区ID */
	size_t dma_max;  /* 最大DMA */
	/* -- hardware operations -- */
	struct snd_pcm_ops *ops;  /* 指向PCM操作函数 */
	/* -- runtime information -- */
	struct snd_pcm_runtime *runtime;  /* 指向PCM运行时 */
        /* -- timer section -- */
	struct snd_timer *timer;		/* timer */  /* 声卡定时器 */
	unsigned timer_running: 1;	/* time is running */  /* 定时器是否运行 */
	/* -- next substream -- */
	struct snd_pcm_substream *next;  /* 指向下一个PCM子流 */
	/* -- linked substreams -- */
	struct list_head link_list;	/* linked list member */  /* 用于形成链表 */
	struct snd_pcm_group self_group;	/* fake group for non linked substream (with substream lock inside) */ /* 没有连接的子流的假装的PCM组 */
	struct snd_pcm_group *group;		/* pointer to current group */  /* 指向当前组 */
	/* -- assigned files -- */
	void *file;
	int ref_count;  /* 计数器 */
	atomic_t mmap_count;  /* 映射计数 */
	unsigned int f_flags;
	void (*pcm_release)(struct snd_pcm_substream *);  /* 是否PCM */
#if defined(CONFIG_SND_PCM_OSS) || defined(CONFIG_SND_PCM_OSS_MODULE)
	/* -- OSS things -- */
	struct snd_pcm_oss_substream oss;
#endif
#ifdef CONFIG_SND_VERBOSE_PROCFS
	struct snd_info_entry *proc_root;
	struct snd_info_entry *proc_info_entry;
	struct snd_info_entry *proc_hw_params_entry;
	struct snd_info_entry *proc_sw_params_entry;
	struct snd_info_entry *proc_status_entry;
	struct snd_info_entry *proc_prealloc_entry;
	struct snd_info_entry *proc_prealloc_max_entry;
#endif
	/* misc flags */
	unsigned int hw_opened: 1; /* 标记硬件是否是打开的 */
};

#define SUBSTREAM_BUSY(substream) ((substream)->ref_count > 0)


struct snd_pcm_str {  /* PCM流 */
	int stream;				/* stream (direction) */ /* 流的传输方向，查看宏SNDRV_PCM_STREAM_XXX */
	struct snd_pcm *pcm;  /* 指向所属的PCM卡 */
	/* -- substreams -- */
	unsigned int substream_count;  /* 子流计数 */
	unsigned int substream_opened;  /* 打开的子流 */
	struct snd_pcm_substream *substream;  /* 指向PCM子流 */
#if defined(CONFIG_SND_PCM_OSS) || defined(CONFIG_SND_PCM_OSS_MODULE)
	/* -- OSS things -- */
	struct snd_pcm_oss_stream oss;  /* OSS */
#endif
#ifdef CONFIG_SND_VERBOSE_PROCFS
	struct snd_info_entry *proc_root;  
	struct snd_info_entry *proc_info_entry;
#ifdef CONFIG_SND_PCM_XRUN_DEBUG
	unsigned int xrun_debug;	/* 0 = disabled, 1 = verbose, 2 = stacktrace */ 
	struct snd_info_entry *proc_xrun_debug_entry;
#endif
#endif
};

struct snd_pcm {  /* PCM实例 */
	struct snd_card *card;  /* 指向所属的声卡 */
	struct list_head list;  /* 用于形成PCM设备链表 */
	int device; /* device number */  /* 设备号---第几个pcm设备 */
	unsigned int info_flags;  /* PCM信息标志，如SNDRV_PCM_INFO_HALF_DUPLEX(半双工) */
	unsigned short dev_class;
	unsigned short dev_subclass;
	char id[64];  /* ID */
	char name[80];   /* 名字 */
	struct snd_pcm_str streams[2];  /* PCM流，有两个流，streams[0]为播放流、streams[1]是录音流 */
	struct mutex open_mutex;
	wait_queue_head_t open_wait;  /* 等待队列头 */
	void *private_data;  /* 私有数据 */
	void (*private_free) (struct snd_pcm *pcm); /* 释放私有数据 */
	struct device *dev; /* actual hw device this belongs to */
#if defined(CONFIG_SND_PCM_OSS) || defined(CONFIG_SND_PCM_OSS_MODULE)
	struct snd_pcm_oss oss;
#endif
};

struct snd_pcm_notify {  /* PCM通知 */
	int (*n_register) (struct snd_pcm * pcm);  /* 注册pcm设备 */
	int (*n_disconnect) (struct snd_pcm * pcm);  /* 取消连接 */
	int (*n_unregister) (struct snd_pcm * pcm);  /* 注销pcm设备 */
	struct list_head list;   /* 用于形成链表 */
};

/*
 *  Registering
 */

extern const struct file_operations snd_pcm_f_ops[2];

int snd_pcm_new(struct snd_card *card, const char *id, int device,
		int playback_count, int capture_count,
		struct snd_pcm **rpcm);
int snd_pcm_new_stream(struct snd_pcm *pcm, int stream, int substream_count);

int snd_pcm_notify(struct snd_pcm_notify *notify, int nfree);

/*
 *  Native I/O
 */

extern rwlock_t snd_pcm_link_rwlock;

int snd_pcm_info(struct snd_pcm_substream *substream, struct snd_pcm_info *info);
int snd_pcm_info_user(struct snd_pcm_substream *substream,
		      struct snd_pcm_info __user *info);
int snd_pcm_status(struct snd_pcm_substream *substream,
		   struct snd_pcm_status *status);
int snd_pcm_start(struct snd_pcm_substream *substream);
int snd_pcm_stop(struct snd_pcm_substream *substream, int status);
int snd_pcm_drain_done(struct snd_pcm_substream *substream);
#ifdef CONFIG_PM
int snd_pcm_suspend(struct snd_pcm_substream *substream);
int snd_pcm_suspend_all(struct snd_pcm *pcm);
#endif
int snd_pcm_kernel_ioctl(struct snd_pcm_substream *substream, unsigned int cmd, void *arg);
int snd_pcm_open_substream(struct snd_pcm *pcm, int stream, struct file *file,
			   struct snd_pcm_substream **rsubstream);
void snd_pcm_release_substream(struct snd_pcm_substream *substream);
int snd_pcm_attach_substream(struct snd_pcm *pcm, int stream, struct file *file,
			     struct snd_pcm_substream **rsubstream);
void snd_pcm_detach_substream(struct snd_pcm_substream *substream);
void snd_pcm_vma_notify_data(void *client, void *data);
int snd_pcm_mmap_data(struct snd_pcm_substream *substream, struct file *file, struct vm_area_struct *area);

#if BITS_PER_LONG >= 64

static inline void div64_32(u_int64_t *n, u_int32_t div, u_int32_t *rem)
{
	*rem = *n % div;
	*n /= div;
}

#elif defined(i386)

static inline void div64_32(u_int64_t *n, u_int32_t div, u_int32_t *rem)
{
	u_int32_t low, high;
	low = *n & 0xffffffff;
	high = *n >> 32;
	if (high) {
		u_int32_t high1 = high % div;
		high /= div;
		asm("divl %2":"=a" (low), "=d" (*rem):"rm" (div), "a" (low), "d" (high1));
		*n = (u_int64_t)high << 32 | low;
	} else {
		*n = low / div;
		*rem = low % div;
	}
}
#else

static inline void divl(u_int32_t high, u_int32_t low,
			u_int32_t div,
			u_int32_t *q, u_int32_t *r)
{
	u_int64_t n = (u_int64_t)high << 32 | low;
	u_int64_t d = (u_int64_t)div << 31;
	u_int32_t q1 = 0;
	int c = 32;
	while (n > 0xffffffffU) {
		q1 <<= 1;
		if (n >= d) {
			n -= d;
			q1 |= 1;
		}
		d >>= 1;
		c--;
	}
	q1 <<= c;
	if (n) {
		low = n;
		*q = q1 | (low / div);
		*r = low % div;
	} else {
		*r = 0;
		*q = q1;
	}
	return;
}

static inline void div64_32(u_int64_t *n, u_int32_t div, u_int32_t *rem)
{
	u_int32_t low, high;
	low = *n & 0xffffffff;
	high = *n >> 32;
	if (high) {
		u_int32_t high1 = high % div;
		u_int32_t low1 = low;
		high /= div;
		divl(high1, low1, div, &low, rem);
		*n = (u_int64_t)high << 32 | low;
	} else {
		*n = low / div;
		*rem = low % div;
	}
}
#endif

/*
 *  PCM library
 */

static inline int snd_pcm_stream_linked(struct snd_pcm_substream *substream)
{
	return substream->group != &substream->self_group;
}

static inline void snd_pcm_stream_lock(struct snd_pcm_substream *substream)
{
	read_lock(&snd_pcm_link_rwlock);
	spin_lock(&substream->self_group.lock);
}

static inline void snd_pcm_stream_unlock(struct snd_pcm_substream *substream)
{
	spin_unlock(&substream->self_group.lock);
	read_unlock(&snd_pcm_link_rwlock);
}

static inline void snd_pcm_stream_lock_irq(struct snd_pcm_substream *substream)
{
	read_lock_irq(&snd_pcm_link_rwlock);
	spin_lock(&substream->self_group.lock);
}

static inline void snd_pcm_stream_unlock_irq(struct snd_pcm_substream *substream)
{
	spin_unlock(&substream->self_group.lock);
	read_unlock_irq(&snd_pcm_link_rwlock);
}

#define snd_pcm_stream_lock_irqsave(substream, flags) \
do { \
	read_lock_irqsave(&snd_pcm_link_rwlock, (flags)); \
	spin_lock(&substream->self_group.lock); \
} while (0)

#define snd_pcm_stream_unlock_irqrestore(substream, flags) \
do { \
	spin_unlock(&substream->self_group.lock); \
	read_unlock_irqrestore(&snd_pcm_link_rwlock, (flags)); \
} while (0)

#define snd_pcm_group_for_each_entry(s, substream) \
	list_for_each_entry(s, &substream->group->substreams, link_list)

static inline int snd_pcm_running(struct snd_pcm_substream *substream)
{
	return (substream->runtime->status->state == SNDRV_PCM_STATE_RUNNING ||
		(substream->runtime->status->state == SNDRV_PCM_STATE_DRAINING &&
		 substream->stream == SNDRV_PCM_STREAM_PLAYBACK));
}

static inline ssize_t bytes_to_samples(struct snd_pcm_runtime *runtime, ssize_t size)
{
	return size * 8 / runtime->sample_bits;
}

/* 帧到字节的转换 */
static inline snd_pcm_sframes_t bytes_to_frames(struct snd_pcm_runtime *runtime, ssize_t size)
{
	return size * 8 / runtime->frame_bits;
}

static inline ssize_t samples_to_bytes(struct snd_pcm_runtime *runtime, ssize_t size)
{
	return size * runtime->sample_bits / 8;
}

/* 帧到字节的转换 */
static inline ssize_t frames_to_bytes(struct snd_pcm_runtime *runtime, snd_pcm_sframes_t size)
{
	return size * runtime->frame_bits / 8;
}

static inline int frame_aligned(struct snd_pcm_runtime *runtime, ssize_t bytes)
{
	return bytes % runtime->byte_align == 0;
}

static inline size_t snd_pcm_lib_buffer_bytes(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	return frames_to_bytes(runtime, runtime->buffer_size);
}

static inline size_t snd_pcm_lib_period_bytes(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	return frames_to_bytes(runtime, runtime->period_size);
}

/*
 *  result is: 0 ... (boundary - 1)
 */
static inline snd_pcm_uframes_t snd_pcm_playback_avail(struct snd_pcm_runtime *runtime)
{
	snd_pcm_sframes_t avail = runtime->status->hw_ptr + runtime->buffer_size - runtime->control->appl_ptr;
	if (avail < 0)
		avail += runtime->boundary;
	else if ((snd_pcm_uframes_t) avail >= runtime->boundary)
		avail -= runtime->boundary;
	return avail;
}

/*
 *  result is: 0 ... (boundary - 1)
 */
static inline snd_pcm_uframes_t snd_pcm_capture_avail(struct snd_pcm_runtime *runtime)
{
	snd_pcm_sframes_t avail = runtime->status->hw_ptr - runtime->control->appl_ptr;
	if (avail < 0)
		avail += runtime->boundary;
	return avail;
}

static inline snd_pcm_sframes_t snd_pcm_playback_hw_avail(struct snd_pcm_runtime *runtime)
{
	return runtime->buffer_size - snd_pcm_playback_avail(runtime);
}

static inline snd_pcm_sframes_t snd_pcm_capture_hw_avail(struct snd_pcm_runtime *runtime)
{
	return runtime->buffer_size - snd_pcm_capture_avail(runtime);
}

/**
 * snd_pcm_playback_ready - check whether the playback buffer is available
 * @substream: the pcm substream instance
 *
 * Checks whether enough free space is available on the playback buffer.
 *
 * Returns non-zero if available, or zero if not.
 */
static inline int snd_pcm_playback_ready(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	return snd_pcm_playback_avail(runtime) >= runtime->control->avail_min;
}

/**
 * snd_pcm_capture_ready - check whether the capture buffer is available
 * @substream: the pcm substream instance
 *
 * Checks whether enough capture data is available on the capture buffer.
 *
 * Returns non-zero if available, or zero if not.
 */
static inline int snd_pcm_capture_ready(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	return snd_pcm_capture_avail(runtime) >= runtime->control->avail_min;
}

/**
 * snd_pcm_playback_data - check whether any data exists on the playback buffer
 * @substream: the pcm substream instance
 *
 * Checks whether any data exists on the playback buffer. If stop_threshold
 * is bigger or equal to boundary, then this function returns always non-zero.
 *
 * Returns non-zero if exists, or zero if not.
 */
static inline int snd_pcm_playback_data(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	
	if (runtime->stop_threshold >= runtime->boundary)
		return 1;
	return snd_pcm_playback_avail(runtime) < runtime->buffer_size;
}

/**
 * snd_pcm_playback_empty - check whether the playback buffer is empty
 * @substream: the pcm substream instance
 *
 * Checks whether the playback buffer is empty.
 *
 * Returns non-zero if empty, or zero if not.
 */
static inline int snd_pcm_playback_empty(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	return snd_pcm_playback_avail(runtime) >= runtime->buffer_size;
}

/**
 * snd_pcm_capture_empty - check whether the capture buffer is empty
 * @substream: the pcm substream instance
 *
 * Checks whether the capture buffer is empty.
 *
 * Returns non-zero if empty, or zero if not.
 */
static inline int snd_pcm_capture_empty(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	return snd_pcm_capture_avail(runtime) == 0;
}

static inline void snd_pcm_trigger_done(struct snd_pcm_substream *substream, 
					struct snd_pcm_substream *master)
{
	substream->runtime->trigger_master = master;
}

static inline int hw_is_mask(int var)
{
	return var >= SNDRV_PCM_HW_PARAM_FIRST_MASK &&
		var <= SNDRV_PCM_HW_PARAM_LAST_MASK;
}

static inline int hw_is_interval(int var)
{
	return var >= SNDRV_PCM_HW_PARAM_FIRST_INTERVAL &&
		var <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL;
}

static inline struct snd_mask *hw_param_mask(struct snd_pcm_hw_params *params,
				     snd_pcm_hw_param_t var)
{
	return &params->masks[var - SNDRV_PCM_HW_PARAM_FIRST_MASK];
}

static inline struct snd_interval *hw_param_interval(struct snd_pcm_hw_params *params,
					     snd_pcm_hw_param_t var)
{
	return &params->intervals[var - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL];
}

static inline const struct snd_mask *hw_param_mask_c(const struct snd_pcm_hw_params *params,
					     snd_pcm_hw_param_t var)
{
	return &params->masks[var - SNDRV_PCM_HW_PARAM_FIRST_MASK];
}

static inline const struct snd_interval *hw_param_interval_c(const struct snd_pcm_hw_params *params,
						     snd_pcm_hw_param_t var)
{
	return &params->intervals[var - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL];
}

#define params_access(p) snd_mask_min(hw_param_mask((p), SNDRV_PCM_HW_PARAM_ACCESS))
#define params_format(p) snd_mask_min(hw_param_mask((p), SNDRV_PCM_HW_PARAM_FORMAT))
#define params_subformat(p) snd_mask_min(hw_param_mask((p), SNDRV_PCM_HW_PARAM_SUBFORMAT))
#define params_channels(p) hw_param_interval((p), SNDRV_PCM_HW_PARAM_CHANNELS)->min
#define params_rate(p) hw_param_interval((p), SNDRV_PCM_HW_PARAM_RATE)->min
#define params_period_size(p) hw_param_interval((p), SNDRV_PCM_HW_PARAM_PERIOD_SIZE)->min
#define params_period_bytes(p) ((params_period_size(p)*snd_pcm_format_physical_width(params_format(p))*params_channels(p))/8)
#define params_periods(p) hw_param_interval((p), SNDRV_PCM_HW_PARAM_PERIODS)->min
#define params_buffer_size(p) hw_param_interval((p), SNDRV_PCM_HW_PARAM_BUFFER_SIZE)->min
#define params_buffer_bytes(p) hw_param_interval((p), SNDRV_PCM_HW_PARAM_BUFFER_BYTES)->min


int snd_interval_refine(struct snd_interval *i, const struct snd_interval *v);
void snd_interval_mul(const struct snd_interval *a, const struct snd_interval *b, struct snd_interval *c);
void snd_interval_div(const struct snd_interval *a, const struct snd_interval *b, struct snd_interval *c);
void snd_interval_muldivk(const struct snd_interval *a, const struct snd_interval *b, 
			  unsigned int k, struct snd_interval *c);
void snd_interval_mulkdiv(const struct snd_interval *a, unsigned int k,
			  const struct snd_interval *b, struct snd_interval *c);
int snd_interval_list(struct snd_interval *i, unsigned int count, unsigned int *list, unsigned int mask);
int snd_interval_ratnum(struct snd_interval *i,
			unsigned int rats_count, struct snd_ratnum *rats,
			unsigned int *nump, unsigned int *denp);

void _snd_pcm_hw_params_any(struct snd_pcm_hw_params *params);
void _snd_pcm_hw_param_setempty(struct snd_pcm_hw_params *params, snd_pcm_hw_param_t var);
int snd_pcm_hw_params_choose(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params);

int snd_pcm_hw_refine(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params);

int snd_pcm_hw_constraints_init(struct snd_pcm_substream *substream);
int snd_pcm_hw_constraints_complete(struct snd_pcm_substream *substream);

int snd_pcm_hw_constraint_mask(struct snd_pcm_runtime *runtime, snd_pcm_hw_param_t var,
			       u_int32_t mask);
int snd_pcm_hw_constraint_mask64(struct snd_pcm_runtime *runtime, snd_pcm_hw_param_t var,
				 u_int64_t mask);
int snd_pcm_hw_constraint_minmax(struct snd_pcm_runtime *runtime, snd_pcm_hw_param_t var,
				 unsigned int min, unsigned int max);
int snd_pcm_hw_constraint_integer(struct snd_pcm_runtime *runtime, snd_pcm_hw_param_t var);
int snd_pcm_hw_constraint_list(struct snd_pcm_runtime *runtime, 
			       unsigned int cond,
			       snd_pcm_hw_param_t var,
			       struct snd_pcm_hw_constraint_list *l);
int snd_pcm_hw_constraint_ratnums(struct snd_pcm_runtime *runtime, 
				  unsigned int cond,
				  snd_pcm_hw_param_t var,
				  struct snd_pcm_hw_constraint_ratnums *r);
int snd_pcm_hw_constraint_ratdens(struct snd_pcm_runtime *runtime, 
				  unsigned int cond,
				  snd_pcm_hw_param_t var,
				  struct snd_pcm_hw_constraint_ratdens *r);
int snd_pcm_hw_constraint_msbits(struct snd_pcm_runtime *runtime, 
				 unsigned int cond,
				 unsigned int width,
				 unsigned int msbits);
int snd_pcm_hw_constraint_step(struct snd_pcm_runtime *runtime,
			       unsigned int cond,
			       snd_pcm_hw_param_t var,
			       unsigned long step);
int snd_pcm_hw_constraint_pow2(struct snd_pcm_runtime *runtime,
			       unsigned int cond,
			       snd_pcm_hw_param_t var);
int snd_pcm_hw_rule_add(struct snd_pcm_runtime *runtime,
			unsigned int cond,
			int var,
			snd_pcm_hw_rule_func_t func, void *private,
			int dep, ...);

int snd_pcm_format_signed(snd_pcm_format_t format);
int snd_pcm_format_unsigned(snd_pcm_format_t format);
int snd_pcm_format_linear(snd_pcm_format_t format);
int snd_pcm_format_little_endian(snd_pcm_format_t format);
int snd_pcm_format_big_endian(snd_pcm_format_t format);
#if 0 /* just for DocBook */
/**
 * snd_pcm_format_cpu_endian - Check the PCM format is CPU-endian
 * @format: the format to check
 *
 * Returns 1 if the given PCM format is CPU-endian, 0 if
 * opposite, or a negative error code if endian not specified.
 */
int snd_pcm_format_cpu_endian(snd_pcm_format_t format);
#endif /* DocBook */
#ifdef SNDRV_LITTLE_ENDIAN
#define snd_pcm_format_cpu_endian(format) snd_pcm_format_little_endian(format)
#else
#define snd_pcm_format_cpu_endian(format) snd_pcm_format_big_endian(format)
#endif
int snd_pcm_format_width(snd_pcm_format_t format);			/* in bits */
int snd_pcm_format_physical_width(snd_pcm_format_t format);		/* in bits */
ssize_t snd_pcm_format_size(snd_pcm_format_t format, size_t samples);
const unsigned char *snd_pcm_format_silence_64(snd_pcm_format_t format);
int snd_pcm_format_set_silence(snd_pcm_format_t format, void *buf, unsigned int frames);
snd_pcm_format_t snd_pcm_build_linear_format(int width, int unsignd, int big_endian);

void snd_pcm_set_ops(struct snd_pcm * pcm, int direction, struct snd_pcm_ops *ops);
void snd_pcm_set_sync(struct snd_pcm_substream *substream);
int snd_pcm_lib_interleave_len(struct snd_pcm_substream *substream);
int snd_pcm_lib_ioctl(struct snd_pcm_substream *substream,
		      unsigned int cmd, void *arg);                      
int snd_pcm_update_hw_ptr(struct snd_pcm_substream *substream);
int snd_pcm_playback_xrun_check(struct snd_pcm_substream *substream);
int snd_pcm_capture_xrun_check(struct snd_pcm_substream *substream);
int snd_pcm_playback_xrun_asap(struct snd_pcm_substream *substream);
int snd_pcm_capture_xrun_asap(struct snd_pcm_substream *substream);
void snd_pcm_playback_silence(struct snd_pcm_substream *substream, snd_pcm_uframes_t new_hw_ptr);
void snd_pcm_period_elapsed(struct snd_pcm_substream *substream);
snd_pcm_sframes_t snd_pcm_lib_write(struct snd_pcm_substream *substream,
				    const void __user *buf,
				    snd_pcm_uframes_t frames);
snd_pcm_sframes_t snd_pcm_lib_read(struct snd_pcm_substream *substream,
				   void __user *buf, snd_pcm_uframes_t frames);
snd_pcm_sframes_t snd_pcm_lib_writev(struct snd_pcm_substream *substream,
				     void __user **bufs, snd_pcm_uframes_t frames);
snd_pcm_sframes_t snd_pcm_lib_readv(struct snd_pcm_substream *substream,
				    void __user **bufs, snd_pcm_uframes_t frames);

extern const struct snd_pcm_hw_constraint_list snd_pcm_known_rates;

int snd_pcm_limit_hw_rates(struct snd_pcm_runtime *runtime);
unsigned int snd_pcm_rate_to_rate_bit(unsigned int rate);

static inline void snd_pcm_set_runtime_buffer(struct snd_pcm_substream *substream,
					      struct snd_dma_buffer *bufp)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	if (bufp) {
		runtime->dma_buffer_p = bufp;
		runtime->dma_area = bufp->area;
		runtime->dma_addr = bufp->addr;
		runtime->dma_bytes = bufp->bytes;
	} else {
		runtime->dma_buffer_p = NULL;
		runtime->dma_area = NULL;
		runtime->dma_addr = 0;
		runtime->dma_bytes = 0;
	}
}

/*
 *  Timer interface
 */

void snd_pcm_timer_resolution_change(struct snd_pcm_substream *substream);
void snd_pcm_timer_init(struct snd_pcm_substream *substream);
void snd_pcm_timer_done(struct snd_pcm_substream *substream);

static inline void snd_pcm_gettime(struct snd_pcm_runtime *runtime,
				   struct timespec *tv)
{
	if (runtime->tstamp_type == SNDRV_PCM_TSTAMP_TYPE_MONOTONIC)
		do_posix_clock_monotonic_gettime(tv);
	else
		getnstimeofday(tv);
}

/*
 *  Memory
 */

int snd_pcm_lib_preallocate_free(struct snd_pcm_substream *substream);
int snd_pcm_lib_preallocate_free_for_all(struct snd_pcm *pcm);
int snd_pcm_lib_preallocate_pages(struct snd_pcm_substream *substream,
				  int type, struct device *data,
				  size_t size, size_t max);
int snd_pcm_lib_preallocate_pages_for_all(struct snd_pcm *pcm,
					  int type, void *data,
					  size_t size, size_t max);
int snd_pcm_lib_malloc_pages(struct snd_pcm_substream *substream, size_t size);
int snd_pcm_lib_free_pages(struct snd_pcm_substream *substream);

/*
 * SG-buffer handling
 */
#define snd_pcm_substream_sgbuf(substream) \
	((substream)->runtime->dma_buffer_p->private_data)

static inline dma_addr_t
snd_pcm_sgbuf_get_addr(struct snd_pcm_substream *substream, unsigned int ofs)
{
	struct snd_sg_buf *sg = snd_pcm_substream_sgbuf(substream);
	return snd_sgbuf_get_addr(sg, ofs);
}

static inline void *
snd_pcm_sgbuf_get_ptr(struct snd_pcm_substream *substream, unsigned int ofs)
{
	struct snd_sg_buf *sg = snd_pcm_substream_sgbuf(substream);
	return snd_sgbuf_get_ptr(sg, ofs);
}

struct page *snd_pcm_sgbuf_ops_page(struct snd_pcm_substream *substream,
				    unsigned long offset);
unsigned int snd_pcm_sgbuf_get_chunk_size(struct snd_pcm_substream *substream,
					  unsigned int ofs, unsigned int size);

/* handle mmap counter - PCM mmap callback should handle this counter properly */
static inline void snd_pcm_mmap_data_open(struct vm_area_struct *area)
{
	struct snd_pcm_substream *substream = (struct snd_pcm_substream *)area->vm_private_data;
	atomic_inc(&substream->mmap_count);
}

static inline void snd_pcm_mmap_data_close(struct vm_area_struct *area)
{
	struct snd_pcm_substream *substream = (struct snd_pcm_substream *)area->vm_private_data;
	atomic_dec(&substream->mmap_count);
}

/* mmap for io-memory area */
#if defined(CONFIG_X86) || defined(CONFIG_PPC) || defined(CONFIG_ALPHA)
#define SNDRV_PCM_INFO_MMAP_IOMEM	SNDRV_PCM_INFO_MMAP
int snd_pcm_lib_mmap_iomem(struct snd_pcm_substream *substream, struct vm_area_struct *area);
#else
#define SNDRV_PCM_INFO_MMAP_IOMEM	0
#define snd_pcm_lib_mmap_iomem	NULL
#endif

static inline void snd_pcm_limit_isa_dma_size(int dma, size_t *max)
{
	*max = dma < 4 ? 64 * 1024 : 128 * 1024;
}

/*
 *  Misc
 */

#define SNDRV_PCM_DEFAULT_CON_SPDIF	(IEC958_AES0_CON_EMPHASIS_NONE|\
					 (IEC958_AES1_CON_ORIGINAL<<8)|\
					 (IEC958_AES1_CON_PCM_CODER<<8)|\
					 (IEC958_AES3_CON_FS_48000<<24))

#define PCM_RUNTIME_CHECK(sub) snd_BUG_ON(!(sub) || !(sub)->runtime)

#endif /* __SOUND_PCM_H */
