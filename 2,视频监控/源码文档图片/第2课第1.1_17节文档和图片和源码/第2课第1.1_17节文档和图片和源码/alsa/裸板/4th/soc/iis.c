
#define IISCON              (*(volatile unsigned long *)0x55000000)
#define IISMOD              (*(volatile unsigned long *)0x55000004)
#define IISPSR              (*(volatile unsigned long *)0x55000008)
#define IISFCON             (*(volatile unsigned long *)0x5500000C)
#define IISFIFO             (*(volatile unsigned long *)0x55000010)

#define PCLK 50000000

#define ABS(a, b) ((a>b)?(a-b):(b-a))

void iis_init(int bits_per_sample, int fs)
{
    int tmp_fs;
    int i;
    int min = 0xffff;
    int pre = 0;

    /* 配置GPIO用于IIS */
    GPECON &= ~((3<<0) | (3<<2) | (3<<4) | (3<<6) | (3<<8));
    GPECON |= ((2<<0) | (2<<2) | (2<<4) | (2<<6) | (2<<8));
    
    
    /* bit[9] : Master clock select, 0-PCLK
     * bit[8] : 0 = Master mode
     * bit[7:6] : 10 = Transmit mode
     * bit[4] : 1-MSB (Left)-justified format
     * bit[2] : 384fs, 确定了MASTER CLOCK之后, fs = MASTER CLOCK/384
     * bit[1:0] : Serial bit clock frequency select, 32fs
     */
     
    if (bits_per_sample == 16)
        IISMOD = (2<<6) | (1<<4) | (1<<3) | (1<<2) | (1);
    else
        IISMOD = (2<<6) | (1<<4) | (0<<3) | (1<<2) | (1);

    /* Master clock = PCLK/(n+1)
     * fs = Master clock / 384
     * fs = PCLK / (n+1) / 384
     */
    for (i = 0; i <= 31; i++)
    {
        tmp_fs = PCLK/384/(i+1);
        if (ABS(tmp_fs, fs) < min)
        {
            min = ABS(tmp_fs, fs);
            pre = i;
        }
    }
    IISPSR = (pre << 5) | (pre);

    /*
     * bit15 : Transmit FIFO access mode select, 1-DMA
     * bit13 : Transmit FIFO, 1-enable
     */
    IISFCON = (1<<15) | (1<<13);
    
    /*
     * bit[5] : Transmit DMA service request, 1-enable
     * bit[3] : IIS prescaler, 1-enable
     */
    IISCON = (1<<5) | (1<<3) ;
}

