#include <stdio.h>

#define DMA0_BASE_ADDR  0x4B000000
#define DMA1_BASE_ADDR  0x4B000040
#define DMA2_BASE_ADDR  0x4B000080
#define DMA3_BASE_ADDR  0x4B0000C0

struct s3c_dma_regs {
	unsigned long disrc;
	unsigned long disrcc;
	unsigned long didst;
	unsigned long didstc;
	unsigned long dcon;
	unsigned long dstat;
	unsigned long dcsrc;
	unsigned long dcdst;
	unsigned long dmasktrig;
};

struct s3c_dma_regs *dma_regs;

static int dma_src_rep;
static int dma_len_rep;
    
/* 数据传输: 源,目的,长度 */
void dma_init(unsigned int src, unsigned int len)
{
    dma_src_rep = src;
    dma_len_rep = len;
    
    dma_regs = (struct s3c_dma_regs *)DMA2_BASE_ADDR;
    
	/* 把源,目的,长度告诉DMA */
	dma_regs->disrc      = src;        /* 源的物理地址 */
	dma_regs->disrcc     = (0<<1) | (0<<0); /* 源位于AHB总线, 源地址递增 */
	dma_regs->didst      = 0x55000010;        /* 目的的物理地址 */
	dma_regs->didstc     = (0<<2) | (1<<1) | (1<<0); /* 目的位于APB总线, 目的地址不变 */
	dma_regs->dcon       = (1<<31)|(0<<30)|(1<<29)|(0<<28)|(0<<27)|(0<<24)|(1<<23)|(1<<20)|(len/2);  /* 使能中断,单个传输,硬件触发 */
}

void dma_start(void)
{
	/* 启动DMA */
	dma_regs->dmasktrig  = (1<<1);
}

void dma_stop(void)
{
	/* 启动DMA */
	dma_regs->dmasktrig  &= ~(1<<1);
}

void dma_repeat(void)
{
    dma_init(dma_src_rep, dma_len_rep);
}

void DMA2IntHandle(void)
{
    printf("DMA2 done\n\r");
    dma_repeat();
}
    
