#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>
#include <mach/fb.h>
#include <mach/hardware.h>
#include <asm/irq.h>

#include <plat/regs-serial.h>
#include <plat/udc.h>

#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/regs-spi.h>

void	s3c_device_ac97_release(struct device *dev)
{

}

static struct resource s3c_ac97_resource[] = {  /* AC97设备资源 */
	[0] = {
		.start = S3C2440_PA_AC97,      /* 0x55000000 */
		.end   = S3C2440_PA_AC97+ S3C2440_SZ_AC97 -1,
		.flags = IORESOURCE_MEM,
	}
};

static u64 s3c_device_ac97_dmamask = 0xffffffffUL;
 
struct platform_device s3c_device_ac97 = {  /* IIS设备*/
	.name		  = "s3c24xx-uda1341",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_iis_resource),
	.resource	  = s3c_iis_resource,
	.dev              = {
		.dma_mask = &s3c_device_ac97_dmamask,
		.coherent_dma_mask = 0xffffffffUL,
		.release=s3c_device_ac97_release
	}
};


static int __init s3c_device_ac97_init(void)
{
     return platform_device_register(&s3c_device_ac97);
}


static void __exit s3c_device_ac97_exit(void)
{
     platform_device_unregister(&s3c_device_ac97);
}
module_init(s3c_device_ac97_init);
module_exit(s3c_device_ac97_exit);

MODULE_AUTHOR("shenchaoping"); 
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("v1.0");       
MODULE_DESCRIPTION("A audio device"); 




