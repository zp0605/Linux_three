/*
 *  Device management routines
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

#include <linux/slab.h>
#include <linux/time.h>
#include <linux/errno.h>
#include <sound/core.h>

/**
 * snd_device_new - create an ALSA device component
 * @card: the card instance
 * @type: the device type, SNDRV_DEV_XXX
 * @device_data: the data pointer of this device
 * @ops: the operator table
 *
 * Creates a new device component for the given data pointer.
 * The device will be assigned to the card and managed together
 * by the card.
 *
 * The data pointer plays a role as the identifier, too, so the
 * pointer address must be unique and unchanged.
 *
 * Returns zero if successful, or a negative error code on failure.
 */
 /* 创建一个ALSA设备部件
     参数type为设备类型，查看宏SNDRV_DEV_XXX*/SNDRV_DEV_
int snd_device_new(struct snd_card *card, snd_device_type_t type,
		   void *device_data, struct snd_device_ops *ops)
{
	struct snd_device *dev;

	if (snd_BUG_ON(!card || !device_data || !ops))
		return -ENXIO;
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);  /*分配一个设备的设备snd_device */
	if (dev == NULL) {
		snd_printk(KERN_ERR "Cannot allocate device\n");
		return -ENOMEM;
	}
	/* 初始化它 */
	dev->card = card;
	dev->type = type;
	dev->state = SNDRV_DEV_BUILD;
	dev->device_data = device_data;
	dev->ops = ops;
	list_add(&dev->list, &card->devices);	/* add to the head of list */  /* 将dev->list插入card->devices之后 */
	return 0;
}

EXPORT_SYMBOL(snd_device_new);

/**
 * snd_device_free - release the device from the card
 * @card: the card instance
 * @device_data: the data pointer to release
 *
 * Removes the device from the list on the card and invokes the
 * callbacks, dev_disconnect and dev_free, corresponding to the state.
 * Then release the device.
 *
 * Returns zero if successful, or a negative error code on failure or if the
 * device not found.
 */
 /* 
   释放声卡的设备
   参数device_data指向要设备的私有数据
   */
int snd_device_free(struct snd_card *card, void *device_data)
{
	struct snd_device *dev;
	
	if (snd_BUG_ON(!card || !device_data))
		return -ENXIO;
	list_for_each_entry(dev, &card->devices, list) {
		if (dev->device_data != device_data)  /* 遍历链表card->devices寻找dev->device_data = device_data的项 */
			continue;
		/* unlink */
		list_del(&dev->list); /* 将节点dev->list从链表中删除 */
		if (dev->state == SNDRV_DEV_REGISTERED &&
		    dev->ops->dev_disconnect)  /* 如果设备已经注册并且 dev->ops->dev_disconnect函数有定义则调用它将它从声卡脱离*/
			if (dev->ops->dev_disconnect(dev))
				snd_printk(KERN_ERR
					   "device disconnect failure\n");
		if (dev->ops->dev_free) {  /* 如果dev->ops->dev_free有定义则调用设备的dev_free函数来设备设备 */
			if (dev->ops->dev_free(dev))
				snd_printk(KERN_ERR "device free failure\n");
		}
		kfree(dev); /* 直接释放设备占用的内存 */
		return 0;
	}
	snd_printd("device free %p (from %pF), not found\n", device_data,
		   __builtin_return_address(0));
	return -ENXIO;
}

EXPORT_SYMBOL(snd_device_free);

/**
 * snd_device_disconnect - disconnect the device
 * @card: the card instance
 * @device_data: the data pointer to disconnect
 *
 * Turns the device into the disconnection state, invoking
 * dev_disconnect callback, if the device was already registered.
 *
 * Usually called from snd_card_disconnect().
 *
 * Returns zero if successful, or a negative error code on failure or if the
 * device not found.
 */
 /* 取消连接函数---取消同声卡的连接
      参数device_data指向要设备的私有数据*/
int snd_device_disconnect(struct snd_card *card, void *device_data)
{
	struct snd_device *dev;

	if (snd_BUG_ON(!card || !device_data))
		return -ENXIO;
	list_for_each_entry(dev, &card->devices, list) {
		if (dev->device_data != device_data) /* 遍历链表card->devices寻找dev->device_data = device_data的项 */
			continue;
		if (dev->state == SNDRV_DEV_REGISTERED &&
		    dev->ops->dev_disconnect) {
			if (dev->ops->dev_disconnect(dev)) /* 如果设备已经注册并且 dev->ops->dev_disconnect函数有定义则调用它将它从声卡脱离*/
				snd_printk(KERN_ERR "device disconnect failure\n");
			dev->state = SNDRV_DEV_DISCONNECTED; /* 设置设备已经取消了连接 */
		}
		return 0;
	}
	snd_printd("device disconnect %p (from %pF), not found\n", device_data,
		   __builtin_return_address(0));
	return -ENXIO;
}

/**
 * snd_device_register - register the device
 * @card: the card instance
 * @device_data: the data pointer to register
 *
 * Registers the device which was already created via
 * snd_device_new().  Usually this is called from snd_card_register(),
 * but it can be called later if any new devices are created after
 * invocation of snd_card_register().
 *
 * Returns zero if successful, or a negative error code on failure or if the
 * device not found.
 */
 /* 注册设备
    参数device_data指向要设备的私有数据
     */
int snd_device_register(struct snd_card *card, void *device_data)
{
	struct snd_device *dev;
	int err;

	if (snd_BUG_ON(!card || !device_data))
		return -ENXIO;
	list_for_each_entry(dev, &card->devices, list) {
		if (dev->device_data != device_data) /* 遍历链表card->devices寻找dev->device_data = device_data的项 */
			continue;
		if (dev->state == SNDRV_DEV_BUILD && dev->ops->dev_register) { /* 如果设备已经建立并且 dev->ops->dev_register有定义则调用它来注册设备*/
			if ((err = dev->ops->dev_register(dev)) < 0)
				return err;
			dev->state = SNDRV_DEV_REGISTERED; /* 标记设备已经注册 */
			return 0;
		}
		snd_printd("snd_device_register busy\n");
		return -EBUSY;
	}
	snd_BUG();
	return -ENXIO;
}

EXPORT_SYMBOL(snd_device_register);

/*
 * register all the devices on the card.
 * called from init.c
 */
 /* 注册所有的设备到声卡 */
int snd_device_register_all(struct snd_card *card)
{
	struct snd_device *dev;
	int err;
	
	if (snd_BUG_ON(!card))
		return -ENXIO;
	list_for_each_entry(dev, &card->devices, list) { /* 遍历设备链表card->devices */
		if (dev->state == SNDRV_DEV_BUILD && dev->ops->dev_register) { /*  如果设备已经建立并且 dev->ops->dev_register有定义则调用它来注册设备 */
			if ((err = dev->ops->dev_register(dev)) < 0)
				return err;
			dev->state = SNDRV_DEV_REGISTERED; /* 标记设备已经注册 */
		}
	}
	return 0;
}

/*
 * disconnect all the devices on the card.
 * called from init.c
 */
 /* 取消声卡上所有设备同声卡的连接 */
int snd_device_disconnect_all(struct snd_card *card)
{
	struct snd_device *dev;
	int err = 0;

	if (snd_BUG_ON(!card))
		return -ENXIO;
	list_for_each_entry(dev, &card->devices, list) {  /* 遍历声卡的设备链表，对每一项调用 snd_device_disconnect来依次取消连接*/
		if (snd_device_disconnect(card, dev->device_data) < 0)
			err = -ENXIO;
	}
	return err;
}

/*
 * release all the devices on the card.
 * called from init.c
 */
 /* 释放声卡上所有的设备 */
int snd_device_free_all(struct snd_card *card, snd_device_cmd_t cmd)
{
	struct snd_device *dev;
	int err;
	unsigned int range_low, range_high;

	if (snd_BUG_ON(!card))
		return -ENXIO;
	range_low = cmd * SNDRV_DEV_TYPE_RANGE_SIZE;
	range_high = range_low + SNDRV_DEV_TYPE_RANGE_SIZE - 1;
      __again:
	list_for_each_entry(dev, &card->devices, list) {  /* 遍历声卡的设备链表 */
		if (dev->type >= range_low && dev->type <= range_high) {  
			if ((err = snd_device_free(card, dev->device_data)) < 0)  /* 调用 snd_device_free函数来释放设备*/
				return err;
			goto __again;
		}
	}
	return 0;
}
