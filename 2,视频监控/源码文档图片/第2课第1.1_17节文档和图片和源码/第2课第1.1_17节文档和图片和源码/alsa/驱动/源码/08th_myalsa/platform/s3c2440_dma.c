
/* 参考 sound\soc\samsung\dma.c
 */

/* 1. 分配DMA BUFFER
 * 2. 从BUFFER取出period
 * 3. 启动DMA传输
 * 4. 传输完毕,更新状态(hw_ptr)
 */

struct s3c2440_dma_info {
    unsigned int buf_max_size;
    unsigned int phy_addr;
    unsigned int virt_addr;
    
};

static int struct s3c2440_dma_info playback_dma_info;

static const struct snd_pcm_hardware s3c2440_dma_hardware = {
	.info			= SNDRV_PCM_INFO_INTERLEAVED |
				    SNDRV_PCM_INFO_BLOCK_TRANSFER |
				    SNDRV_PCM_INFO_MMAP |
				    SNDRV_PCM_INFO_MMAP_VALID |
				    SNDRV_PCM_INFO_PAUSE |
				    SNDRV_PCM_INFO_RESUME,
	.formats		= SNDRV_PCM_FMTBIT_S16_LE |
				    SNDRV_PCM_FMTBIT_U16_LE |
				    SNDRV_PCM_FMTBIT_U8 |
				    SNDRV_PCM_FMTBIT_S8,
	.channels_min		= 2,
	.channels_max		= 2,
	.buffer_bytes_max	= 128*1024,
	.period_bytes_min	= PAGE_SIZE,
	.period_bytes_max	= PAGE_SIZE*2,
	.periods_min		= 2,
	.periods_max		= 128,
	.fifo_size		= 32,
};

static int s3c2440_dma_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;

    /* 设置属性 */
	snd_pcm_hw_constraint_integer(runtime, SNDRV_PCM_HW_PARAM_PERIODS);
	snd_soc_set_runtime_hwparams(substream, &s3c2440_dma_hardware);

	return 0;
}


static int s3c2440_dma_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
    /* 根据params设置DMA */
    return 0;
}

static struct snd_pcm_ops s3c2440_dma_ops = {
	.open		= s3c2440_dma_open,
	.hw_params	= s3c2440_dma_hw_params,
};


static int s3c2440_dma_new(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_card *card = rtd->card->snd_card;
	struct snd_pcm *pcm = rtd->pcm;
	int ret = 0;

    /* 1. 分配DMA BUFFER */

	if (pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream) {
        dma_alloc_writecombine();
	    playback_dma_info.virt_addr = dma_alloc_writecombine(pcm->card->dev, s3c2440_dma_hardware.buffer_bytes_max,
					   &playback_dma_info.phy_addr, GFP_KERNEL);
        if (!playback_dma_info.virt_addr)
        {
            return -ENOMEM;
        }
        playback_dma_info.buf_max_size = s3c2440_dma_hardware.buffer_bytes_max;
	}

	return ret;
}


static void s3c2440_dma_free(struct snd_pcm *pcm)
{
	struct snd_pcm_substream *substream;
	struct snd_dma_buffer *buf;

	dma_free_writecombine(pcm->card->dev, playback_dma_info.buf_max_size,
			      playback_dma_info.virt_addr, playback_dma_info.phy_addr);
}

static struct snd_soc_platform_driver s3c2440_dma_platform = {
	.ops		= &s3c2440_dma_ops,
	.pcm_new	= s3c2440_dma_new,
	.pcm_free	= s3c2440_dma_free,
};

static int s3c2440_dma_probe(struct platform_device *pdev)
{
	return snd_soc_register_platform(&pdev->dev, &s3c2440_dma_platform);
}
static int s3c2440_dma_remove(struct platform_device *pdev)
{
	return snd_soc_unregister_platform(&pdev->dev);
}

static void s3c2440_dma_release(struct device * dev)
{
}

static struct platform_device s3c2440_dma_dev = {
    .name         = "s3c2440-dma",
    .id       = -1,
    .dev = { 
    	.release = s3c2440_dma_release, 
	},
};
struct platform_driver s3c2440_dma_drv = {
	.probe		= s3c2440_dma_probe,
	.remove		= s3c2440_dma_remove,
	.driver		= {
		.name	= "s3c2440-dma",
	}
};

static int s3c2440_dma_init(void)
{
    platform_device_register(&s3c2440_dma_dev);
    platform_driver_register(&s3c2440_dma_drv);
    return 0;
}

static void s3c2440_dma_exit(void)
{
    platform_device_unregister(&s3c2440_dma_dev);
    platform_driver_unregister(&s3c2440_dma_drv);
}

module_init(s3c2440_dma_init);
module_exit(s3c2440_dma_exit);


