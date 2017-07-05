                                                                                             linux音频子系统

------------------------------------------------------------------------------------------------------------------------------------------------
/*
 *基础知识：
 *
 *采样频率：采样频率是每秒的采样次数，我常说的44.1KHz采样频率就是每秒采样44100次，理论上采样频率越高，转换精度越高，目前主流的采样频率是48KHz。
 *
 *量化精度：对采样数据的分析精度，比如24bit量化精度就是将标准电平信号按照2的24次方进行分析。量化精度越高，声音越逼真。
 *
 *音频设备硬件接口：
 *
 *1.PCM接口
 *   最简单的音频接口是PCM（脉冲编码调制）接口，该接口由时钟脉冲（BCLK）、帧同步信号（FS）及接收数据（DR）和发送数据（DX）组成。在FS信号的上升沿
 *数据传输从MSB开始，FS频率等于采样频率。FS信号之后开始数据字的传输，单个数据位按顺序进行传输，一个时钟周期传输一个数据字。PCM接口很容易实现，
 *原则上能够支持任何数据方案和任何采样频率，但需要每个音频通道获得一个独立的数据队列。
 *
 *2.IIS接口
 *   IIS接口在20世纪80年代首先被PHILIPS用于消费音频产品，并在一个称为LRCLK（Left/Right CLOCK）的信号机制中经过转换，将两路音频变成单一的数据队列。
 *当LRCLK为高时，左声道数据被传输；LRCLK为低时，右声道数据被传输。与CPM相比，IIS更适合于立体声系统，当然，IIS的变体也支持多通道的时分复用，因此
 *可以支持多通道。
 *
 *AC'97接口
 *与PCM和IIS不同，AC'97不只是一种数据格式，用于音频编码的内部架构规格，它还具有控制功能。
 *AC'97采用AC-Link与外部的编解码器连接，AC-Link接口包括位时钟（BITCLK）、同步信号校正(SYNC)和从编码到处理器及从处理器中解码（SDATDIN与SDATAOUT）
 *的数据队列。AC'97数据帧以SYNC脉冲开始，包括12个20位时隙以及1个16位“tag”段，共计256个数据序列。例如，时序“1”和“2”用于编码的控制器，而时隙
 *“3”和“4”分别负载左右两个音频通道。“tag”段表示其他时隙中哪一个包含有效数据，把帧分成时隙是传输控制信号和音频数据仅通过4根线达到9个音频通道
 *或转换成其他数据流称为可能。
 *
 *应用：
 *CD、MD、MP3多采用IIS接口；移动电话多采用PCM接口；智能手机、PDA则多使用和PC一样的AC'97编码格式。
 *
 */

/*
 *linux OSS音频设备驱动
 *OSS标准中有两个最基本的音频设备：mixer（混音器）和dsp（编解码器）。
 *在声卡的硬件电路中，mixer的作用是将多个信号组合或者叠加在一起，对于不同的声卡来说，其混音器的作用可能各不相同，OSS驱动中，/dev/mixer设备文件是
 *应用程序对mixer进行操作的软件接口。
 *混音器电路通常由两个部分组成：输入混音器和输出混音器。对混音器的编程包括如何设置增益控制器的级别，以及怎样在不同的音源间进行切换，这些操作通常
 *来将是不连续的，而且不会像录音或者播放那样需要占用大量的计算机资源，由于混音器的操作不符合典型的读/写操作模式，因此除了open()和close()这两个系
 *统调用之外，大部分的操作都是通过ioctl()系统调用来完成，与/dev/dsp不同，/dev/mixer允许多个应用程序同时访问，并且混音器的设置值会保持到对应的设备
 *文件被关闭为止。
 *
 *dsp也称为编解码器，实现录音与放音，其对应的设备文件是/dev/dsp或/dev/sound/dsp。OSS声卡驱动提供的/dev/dsp是用于数字采集和数字录音的设备文件，向
 *设备写数据即意味着激活声卡上的D/A转换器进行播放，而向该设备读数据则意味着激活声卡上的A/D转换器进行录音。如果应用程序读取数据速度过慢，以致低于
 *声卡的采样频率，那么多余的数据将会被丢弃；如果读取数据的速度过快，以致高于声卡的采样频率，那么声卡驱动程序将会阻塞那些请求数据的应用程序，直到
 *新的数据到来为止；应用程序写入数据的速度应该至少等于声卡的采样频率，过慢会产生声音暂停或停顿的现象，如果用户写入过快的话，它会被内核的声卡驱动
 *阻塞，直到硬件有能力处理新的数据为止。
 *
 *
 *linux ALSA音频设备驱动
 *ALSA的主要特点如下：
 *（1）支持多种声卡设备
 *（2）模块化的内核驱动程序
 *（3）支持SMP和多线程
 *（4）提供应用开发函数库以简化应用程序开发
 *（5）支持OSS API，兼容OSS应用程序
 *
 *ALSA系统包括包括驱动包alsa-driver、开发包alsa-libs、开发包插件alsa-libplugins、设置管理工具包alsa-utils、其他声音相关处理小程序包alsa-tools、
 *特殊音频固件支持包alsa-firmware、OSS接口兼容模拟层工具alsa-oss供7个子项目，其中只有驱动包是必需的。
 *
 *目前ALSA内核提供给用户空间的接口有：
 *
 *（1）设备信息接口（/proc/asound）
 *（2）设备控制接口（/dev/snd/controlCX）
 *（3）混音器设备接口（/dev/snd/mixerCXDX）
 *（4）PCM设备接口（/dev/snd/pcmCXDX）
 *（5）原始MIDI(迷笛)设备接口（/dev/snd/midiCXDX）
 *（6）声音合成(synthesizer)设备接口（/dev/snd/seq）
 *（7）定时器接口（/dev/snd/timer）
 *这些接口被提供给alsa-lib使用，而不是给应用程序使用，应用程序最好使用alsa-lib，或者更高级的接口比如jack提供的接口。
 *
 *
 *linux ASoC音频设备驱动
 *ASoC是ALSA在SoC方面的发展和演变，它的本质仍然属于ALSA，但是在ALSA架构基础上对CPU相关的代码和Codec相关的嗲吗进行了分离，其原因是采用传统ALSA
 *架构情况下，同一型号的Codec工作于不同的CPU时，需要不同的驱动，这是不符合代码重用的要求的。
 *
 *ASoC主要由3部分组成：
 *（1）Codec驱动，这一部分只关系Codec本身，与CPU相关的特性不由此部分操作
 *（2）平台驱动，这一部分只关心CPU本身，不关系Codec，它主要处理了两个问题：DMA引擎和SoC解除的PCM、IIS或AC'97数字接口控制。
 *（3）板驱动，这一部分将平台驱动和Codec驱动绑定在一起，描述了板一级的硬件特征
 *
 *以上3部分中，1和2基本都可以仍然是通用的驱动了，即Codec驱动认为自己可以连接任意CPU，而CPU的IIS、PCM、或AC'97接口对应的平台驱动则认为自己可以连接
 *符号其接口类型的Codec，只有3是不通用的，由特定的电路板上具体的CPU和Codec确定，因此它很像一个插座，上面插着Codec和平台这两个插头。ASoC的用户空间
 *编程方法与ALSA完全一致。
 *
 */

/***********************************************************card和组件管理******************************************************************/

/* 对于每个声卡，必须创建一个card实例，该函数用于创建card
    参数idx为索引号
    参数id为标识的字符串
    参数module一般指向THIS_MODULE
    参数extra_size是要分配的额外的数据的大小，分配的extra_size大小的内存将作为card->private_data*/
 struct snd_card *snd_card_new(int idx, const char *id,struct module *module, int extra_size)

  /* 注册声卡 */
int snd_card_register(struct snd_card *card)

/* 释放(注销)声卡 */
int snd_card_free(struct snd_card *card)

 /* 创建一个ALSA设备部件
     参数type为设备类型，查看宏SNDRV_DEV_XXX*/
int snd_device_new(struct snd_card *card, snd_device_type_t type,void *device_data, struct snd_device_ops *ops)

  /* 
   释放声卡的设备
   参数device_data指向要设备的私有数据
   */
int snd_device_free(struct snd_card *card, void *device_data)

 
/************************************************************************************************************************************************/

/****************************************************************PCM 设备**********************************************************************/
 /* 创建PCM实例
     参数card指向声卡
     参数id是标识字符串
     参数device为PCM设备引索(0表示第1个PCM设备)
     参数playback_count为播放设备的子流数
     参数capture_count为录音设备的子流数
     参数指向构造的PCM实例*/
int snd_pcm_new(struct snd_card *card, const char *id, int device,int playback_count, int capture_count,struct snd_pcm ** rpcm)

 /* 设置PCM操作函数 
     参数direction，查看宏SNDRV_PCM_STREAM_XXX*/
void snd_pcm_set_ops(struct snd_pcm *pcm, int direction, struct snd_pcm_ops *ops)

 /* 分配DMA缓冲区，仅当DMA缓冲区已预分配的情况下才可调用该函数 */
int snd_pcm_lib_malloc_pages(struct snd_pcm_substream *substream, size_t size)

 /* 释放由snd_pcm_lib_malloc_pages函数分配的一致性DMA缓冲区 */
int snd_pcm_lib_free_pages(struct snd_pcm_substream *substream)

 /* 分配缓冲区的最简单的方法是调用该函数
     type的取值可查看宏SNDRV_DMA_TYPE_* */
int snd_pcm_lib_preallocate_pages_for_all(struct snd_pcm *pcm,int type, void *data,size_t size, size_t max)

/************************************************************************************************************************************************/

/**************************************************************控制接口*********************************************************************/
 /* 创建一个control实例----struct snd_kcontrol结构体
     参数ncontrol为初始化记录
     private_data为设置的私有数据 */
struct snd_kcontrol *snd_ctl_new1(const struct snd_kcontrol_new *ncontrol, void *private_data)

 /* 为声卡添加一个控制实例 */
int snd_ctl_add(struct snd_card *card, struct snd_kcontrol *kcontrol)

/* 驱动程序可中断服务程序中调用该函数来改变或更新一个control*/
void snd_ctl_notify(struct snd_card *card, unsigned int mask,struct snd_ctl_elem_id *id)

/************************************************************************************************************************************************/


/******************************************************************AC97**************************************************************************/
 /* 构造AC97总线及其操作 */
int snd_ac97_bus(struct snd_card *card, int num, struct snd_ac97_bus_ops *ops,void *private_data, struct snd_ac97_bus **rbus)

 /* 根据模板创建一个AC97 混音组件 */
int snd_ac97_mixer(struct snd_ac97_bus *bus, struct snd_ac97_template *template, struct snd_ac97 **rac97)

 /* 写AC97寄存器 */
void snd_ac97_write(struct snd_ac97 *ac97, unsigned short reg, unsigned short value)

 /* 更新AC97寄存器 */
int snd_ac97_update(struct snd_ac97 *ac97, unsigned short reg, unsigned short value)

 /* 更新寄存器某个bit位----有锁版本 */
int snd_ac97_update_bits(struct snd_ac97 *ac97, unsigned short reg, unsigned short mask, unsigned short value)

 /* 读AC97寄存器 */
unsigned short snd_ac97_read(struct snd_ac97 *ac97, unsigned short reg)

 /* 设置采样率，reg可以是AC97_PMC_MIC_ADC_RATE  ,  AC97_PCM_FRONT_DAC_RATE  ,  AC97_PCM_LR_ADC_RATE , AC97_PCM_SURR_DAC_RATE 
     and   AC97_PCM_LFE_DAC_RATE*/
int snd_ac97_set_rate(struct snd_ac97 *ac97, int reg, unsigned int rate)
/************************************************************************************************************************************************/

/**************************************************************驱动程序设计***************************************************************/
/* 基于ALSA音频框架的驱动程序设计:    */

1:struct snd_card *snd_card_new(int idx, const char *id,struct module *module, int extra_size);/* 创建一个声卡 */

2:static struct snd_device_ops ops = {
		.dev_free =     xxx_free,
	};

3:struct snd_kcontrol *snd_ctl_new1(const struct snd_kcontrol_new *ncontrol,void *private_data); /*  创建control实例 */

4:int snd_ctl_add(struct snd_card *card, struct snd_kcontrol *kcontrol);    /* 为声卡添加控制实例 */

5:int snd_device_new(struct snd_card *card, SNDRV_DEV_CODEC,void *device_data, struct snd_device_ops *ops);  /* 创建一个ALSA设备部件-----编解码设备 */

6:int snd_pcm_new(struct snd_card *card, const char *id, int device,int playback_count, int capture_count,struct snd_pcm ** rpcm) /*  创建PCM实例 */

8:int snd_pcm_lib_preallocate_pages_for_all(struct snd_pcm *pcm,int type, void *data,size_t size, size_t max)  /* 分配缓冲区 */

9:void snd_pcm_set_ops(struct snd_pcm *pcm, SNDRV_PCM_STREAM_PLAYBACK, struct snd_pcm_ops *ops)  /* 设置PCM操作函数----播放  */

10:void snd_pcm_set_ops(struct snd_pcm *pcm, SNDRV_PCM_STREAM_CAPTURE, struct snd_pcm_ops *ops)  /* 设置PCM操作函数 ----录音 */

11:/*音频相关的初始化:音频流相关、引脚等相关的初始化*/

12:DMA相关的设置

13:int snd_card_register(struct snd_card *card);  /* 注册声卡 */IRQ_AC97
/************************************************************************************************************************************************/

/*************************************************************ASoC音频驱动******************************************************************/

/*
  *1:ASoC Codec驱动:
  */

struct snd_soc_dai { /* 数字音频接口，描述了Codec DAI(数字音频接口---Digital Audio Interface)和PCM配置 */
	/* DAI description */
	char *name;  /* 名字 */
	unsigned int id;  /* ID */
	int ac97_control;

	struct device *dev;

	/* DAI callbacks */
	int (*probe)(struct platform_device *pdev,
		     struct snd_soc_dai *dai);
	void (*remove)(struct platform_device *pdev,
		       struct snd_soc_dai *dai);
	int (*suspend)(struct snd_soc_dai *dai);
	int (*resume)(struct snd_soc_dai *dai);

	/* ops */
	struct snd_soc_dai_ops *ops;  /* DAI操作函数 */

	/* DAI capabilities */
	struct snd_soc_pcm_stream capture;  /* 录音流 */
	struct snd_soc_pcm_stream playback; /* 播放流 */

	/* DAI runtime info */
	struct snd_pcm_runtime *runtime;  /* PCM运行时 */
	struct snd_soc_codec *codec;  /* 编解码器 */
	unsigned int active;
	unsigned char pop_wait:1;
	void *dma_data;

	/* DAI private data */
	void *private_data;

	/* parent codec/platform */
	union {
		struct snd_soc_codec *codec;  /* 编解码器 */
		struct snd_soc_platform *platform;  /* 平台驱动----CPU */
	};

	struct list_head list; /* 用于形成链表 */
};

struct snd_soc_codec { /* SoC音频编解码器---IO操作、动态音频电源管理以及时钟、PLL等控制 */
	char *name;  /* 名字 */
	struct module *owner; /* THIS_MODULE */
	struct mutex mutex;
	struct device *dev;

	struct list_head list; /* 用于形成链表 */

	/* callbacks */
	int (*set_bias_level)(struct snd_soc_codec *,
			      enum snd_soc_bias_level level); /* 回调函数 */

	/* runtime */
	struct snd_card *card;  /* 声卡 */
	struct snd_ac97 *ac97;  /* for ad-hoc ac97 devices */ /* AC97设备 */
	unsigned int active;
	unsigned int pcm_devs;
	void *private_data;

	/* codec IO */
	void *control_data; /* codec control (i2c/3wire) data */
	unsigned int (*read)(struct snd_soc_codec *, unsigned int);
	int (*write)(struct snd_soc_codec *, unsigned int, unsigned int);
	int (*display_register)(struct snd_soc_codec *, char *,
				size_t, unsigned int);
	hw_write_t hw_write;
	hw_read_t hw_read;
	void *reg_cache;
	short reg_cache_size;
	short reg_cache_step;

	/* dapm */
	u32 pop_time;
	struct list_head dapm_widgets;
	struct list_head dapm_paths;
	enum snd_soc_bias_level bias_level;
	enum snd_soc_bias_level suspend_bias_level;
	struct delayed_work delayed_work;

	/* codec DAI's */
	struct snd_soc_dai *dai; /* SoC层接口 */
	unsigned int num_dai;

#ifdef CONFIG_DEBUG_FS
	struct dentry *debugfs_reg;
	struct dentry *debugfs_pop_time;
#endif
};

struct snd_soc_dai_ops { /* 数字音频接口DAI操作函数集 */
	/*
	 * DAI clocking configuration, all optional.
	 * Called by soc_card drivers, normally in their hw_params.
	 */
	int (*set_sysclk)(struct snd_soc_dai *dai,
		int clk_id, unsigned int freq, int dir); /* 设置系统时钟 */
	int (*set_pll)(struct snd_soc_dai *dai,
		int pll_id, unsigned int freq_in, unsigned int freq_out); /* 设置PLL */
	int (*set_clkdiv)(struct snd_soc_dai *dai, int div_id, int div);  /* 设置时钟分频 */

	/*
	 * DAI format configuration
	 * Called by soc_card drivers, normally in their hw_params.
	 */
	 /* DAI格式配置 */
	int (*set_fmt)(struct snd_soc_dai *dai, unsigned int fmt); /* 设置格式 */
	int (*set_tdm_slot)(struct snd_soc_dai *dai,
		unsigned int mask, int slots);
	int (*set_tristate)(struct snd_soc_dai *dai, int tristate);

	/*
	 * DAI digital mute - optional.
	 * Called by soc-core to minimise any pops.
	 */
	int (*digital_mute)(struct snd_soc_dai *dai, int mute); /* 数字静音 */

	/*
	 * ALSA PCM audio operations - all optional.
	 * Called by soc-core during audio PCM operations.
	 */
	 /* ALSA PCM音频操作 */
	int (*startup)(struct snd_pcm_substream *,
		struct snd_soc_dai *);
	void (*shutdown)(struct snd_pcm_substream *,
		struct snd_soc_dai *);
	int (*hw_params)(struct snd_pcm_substream *,
		struct snd_pcm_hw_params *, struct snd_soc_dai *);
	int (*hw_free)(struct snd_pcm_substream *,
		struct snd_soc_dai *);
	int (*prepare)(struct snd_pcm_substream *,
		struct snd_soc_dai *);
	int (*trigger)(struct snd_pcm_substream *, int,
		struct snd_soc_dai *);
};

struct snd_soc_ops { /* SoC操作函数----Codec音频操作 */
	int (*startup)(struct snd_pcm_substream *);
	void (*shutdown)(struct snd_pcm_substream *);
	int (*hw_params)(struct snd_pcm_substream *, struct snd_pcm_hw_params *);
	int (*hw_free)(struct snd_pcm_substream *);
	int (*prepare)(struct snd_pcm_substream *);
	int (*trigger)(struct snd_pcm_substream *, int);
};

/*
  *2:ASoC平台驱动:
  *
  *在ASoC平台驱动部分，同样存在着Codec驱动中的snd_soc_dai、snd_soc_dai_ops、snd_soc_ops这三个结构体的实例用于描述
  *DAI和DAI的操作，不过不同的是，在平台驱动中，它们只描述CPU相关的部分而不描述Codec。除此之外，在ASoC
  *平台驱动中，必须实现完整的DMA驱动，即传统ALSA的and_pcm_ops结构体成员函数trigger()、pointer()等，因此ASoC平台
  *驱动通常由DAI和DMA两部分组成。
  */

/*
  *ASoC板驱动:
  *在板驱动的模块初始化函数中，会通过platform_device_add()注册一个名为"soc-audio"的platform设备，这是因为soc-core.c
  *注册了一个名为"soc-audio"的platform驱动
  */

struct snd_soc_device { /* SoC设备 */
	struct device *dev; /* 内嵌的设备模型的设备 */
	struct snd_soc_card *card; /* SoC卡 */
	struct snd_soc_codec_device *codec_dev;  /* SoC编解码设备 */
	void *codec_data;  /* 编解码设备使用的数据 */
};

struct snd_soc_dai_link  { /* 绑定ASoC Codec驱动和CPU端的平台驱动数据结构 */
	char *name;			/* Codec name */ /* 编解码器的名字 */
	char *stream_name;		/* Stream name */ /* 流的名字 */

	/* DAI */
	struct snd_soc_dai *codec_dai; /* SoC层的接口----编解码器端 */
	struct snd_soc_dai *cpu_dai;     /* SoC层的接口----CPU端*/

	/* machine stream operations */
	struct snd_soc_ops *ops; /* SoC操作函数----流操作函数 */

	/* codec/machine specific init - e.g. add machine controls */
	int (*init)(struct snd_soc_codec *codec);  /* 初始化 */

	/* DAI pcm */
	struct snd_pcm *pcm; /* 指向pcm */
};


/************************************************************************************************************************************************/





























































 
