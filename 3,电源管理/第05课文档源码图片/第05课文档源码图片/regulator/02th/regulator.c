
/* 参考: drivers/regulator/tps6105x-regulator.c */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/mfd/core.h>
#include <asm/io.h>

static volatile unsigned long *gpbcon;
static volatile unsigned long *gpbdat;

static int myregulator_enable(struct regulator_dev *rdev)
{
	*gpbdat |= 1;     /* 输出高电平 */
	return 0;
}

static int myregulator_disable(struct regulator_dev *rdev)
{
	*gpbdat &= ~1;     /* 输出低电平 */
	return 0;
}

static int myregulator_is_enabled(struct regulator_dev *rdev)
{
	if (*gpbdat & 1)
		return 1;
	else
		return 0;
}


static struct regulator_ops myregulator_ops = {
	.enable		= myregulator_enable,
	.disable	= myregulator_disable,
	.is_enabled	= myregulator_is_enabled,
};


static struct regulator_desc myregulator_desc = {
	.name		= "myregulator",
	.ops		= &myregulator_ops,
	.type		= REGULATOR_VOLTAGE,
	.id		= 0,
	.owner		= THIS_MODULE,
	.n_voltages	= 1,
};

static struct regulator_dev *myregulator_dev;
static int myregulator_probe(struct platform_device *pdev)
{
	struct regulator_init_data *init_data = dev_get_platdata(&pdev->dev);

	gpbcon = ioremap(0x56000010, 8);
	gpbdat = gpbcon+1;

	*gpbcon &= ~(3);  /* GPB0设置为输出引脚 */
	*gpbcon |= 1;
	
	/* 分配/设置/注册 regulator */
	myregulator_dev = regulator_register(&myregulator_desc,
					     &pdev->dev,
					     init_data, NULL,
					     NULL);

	if (IS_ERR(myregulator_dev)) {
		printk("regulator_register error!\n");
		return -EIO;
	}

	return 0;
}

static int myregulator_remove(struct platform_device *pdev)
{
	regulator_unregister(myregulator_dev);
	return 0;
}

struct platform_driver myregulator_drv = {
	.probe		= myregulator_probe,
	.remove		= myregulator_remove,
	.driver		= {
		.name	= "myregulator",
	}
};

static int myregulator_init(void)
{
	platform_driver_register(&myregulator_drv);
	return 0;
}

static void myregulator_exit(void)
{
	platform_driver_unregister(&myregulator_drv);
}

module_init(myregulator_init);
module_exit(myregulator_exit);

MODULE_LICENSE("GPL v2");


