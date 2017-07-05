#include <linux/err.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include " l3.h"

static LIST_HEAD(l3_drv_list);
static DEFINE_SPINLOCK(l3_dev_list_lock);
l
static int l3_attach_client(struct l3_client *clnt , const char *adap, const char *name)
{
     struct l3_driver *drv;
     spin_lock(&l3_dev_list_lock);
     list_for_each_entry(drv, &l3_drv_list, list) 	
     	{
         if(drv->name==name)  /* 遍历链表检查合法性 */
		 break;
	}
     if(drv->name==name)  /* 说明没有这个设备 */
     	{
         clnt->driver=drv;
         return 0;
     	}
     spin_unlock(&l3_dev_list_lock);
     return -ENXIO;
}


static void l3_detach_client(struct l3_client *clnt)
{
     spin_lock(&l3_dev_list_lock);
     clnt->driver=NULL;
     spin_unlock(&l3_dev_list_lock);
}
static int  l3_add_driver(struct l3_driver *l3_drv)
{
     struct l3_driver *drv;
     if(!l3_drv->name&&!l3_drv->ops&&!l3_drv->attach_client&&!l3_drv->detach_client)
	 	return -EPERM;
    if(!l3_drv->owner)
		l3_drv->owner=THIS_MODULE;
     spin_lock(&l3_dev_list_lock);
     list_for_each_entry(drv, &l3_drv_list, list) 	
     	{
         if(drv->name==l3_drv->name)  /* 遍历链表检查合法性 */
		 return -EBUSY;
	}
     list_add(&l3_drv->list, &drv->list);  /* 添加到链表*/
     spin_unlock(&l3_dev_list_lock);
}
static void l3_del_driver(struct l3_driver *l3_drv)
{
     struct l3_driver *drv;
     spin_lock(&l3_dev_list_lock);
          list_for_each_entry(drv, &l3_drv_list, list) 	
     	{
         if(drv->name==l3_drv->name)  /* 遍历链表检查合法性 */
		 break;
	}
      if(drv->name==l3_drv->name)  /* 说明没有这个设备 */
              list_del(&l3_drv->list);  /* 从链表中删除 */
     kfree(l3_drv);
     spin_unlock(&l3_dev_list_lock);
}


