/* 
 * dvbdev.c
 *
 * Copyright (C) 2000 Ralph  Metzler <ralph@convergence.de>
 *                  & Marcus Metzler <marcus@convergence.de>
 *                    for convergence integrated media GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

/*#define CONFIG_DVB_DEVFS_ONLY 1*/

#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <linux/kmod.h>
#include <linux/slab.h>
#include <linux/videodev.h>

#include "compat.h"
#include "dvbdev.h"

static int dvbdev_debug = 0;
#define dprintk if (dvbdev_debug) printk

static devfs_handle_t dvb_devfs_handle;
static LIST_HEAD(dvb_adapter_list);
static DECLARE_MUTEX(dvbdev_register_lock);


static char *dnames[] = { 
        "video", "audio", "sec", "frontend", "demux", "dvr", "ca",
	"net", "osd"
};


#ifdef CONFIG_DVB_DEVFS_ONLY

	#define DVB_MAX_IDS              ~0
	#define nums2minor(num,type,id)  0
	#define DVB_DEVFS_FLAGS          (DEVFS_FL_DEFAULT|DEVFS_FL_AUTO_DEVNUM)

#else

	#define DVB_MAX_IDS              4
	#define nums2minor(num,type,id)  ((num << 6) | (id << 4) | type)
	#define DVB_DEVFS_FLAGS          (DEVFS_FL_DEFAULT)


static
struct dvb_device* dvbdev_find_device (int minor)
{
	struct list_head *entry;

	list_for_each (entry, &dvb_adapter_list) {
		struct list_head *entry0;
		struct dvb_adapter *adap;
		adap = list_entry (entry, struct dvb_adapter, list_head);
		list_for_each (entry0, &adap->device_list) {
			struct dvb_device *dev;
			dev = list_entry (entry0, struct dvb_device, list_head);
			if (nums2minor(adap->num, dev->type, dev->id) == minor)
				return dev;
		}
	}

	return NULL;
}


static
int dvb_device_open(struct inode *inode, struct file *file)
{
	struct dvb_device *dvbdev;
	
	dvbdev = dvbdev_find_device (minor(inode->i_rdev));

	if (dvbdev && dvbdev->fops) {
		int err = 0;
		struct file_operations *old_fops;

		file->private_data = dvbdev;
		old_fops = file->f_op;
                file->f_op = fops_get(dvbdev->fops);
                if(file->f_op->open)
                        err = file->f_op->open(inode,file);
                if (err) {
                        fops_put(file->f_op);
                        file->f_op = fops_get(old_fops);
                }
                fops_put(old_fops);
                return err;
	}
	return -ENODEV;
}


static struct file_operations dvb_device_fops =
{
	.owner =	THIS_MODULE,
	.open =		dvb_device_open,
};
#endif /* CONFIG_DVB_DEVFS_ONLY */



int dvb_generic_open(struct inode *inode, struct file *file)
{
        struct dvb_device *dvbdev = file->private_data;

        if (!dvbdev)
                return -ENODEV;

	if (!dvbdev->users)
                return -EBUSY;

	if ((file->f_flags & O_ACCMODE) != O_RDONLY) {
                if (!dvbdev->writers)
		        return -EBUSY;
		dvbdev->writers--;
	}

	dvbdev->users--;
	return 0;
}


int dvb_generic_release(struct inode *inode, struct file *file)
{
        struct dvb_device *dvbdev = file->private_data;

	if (!dvbdev)
                return -ENODEV;

	if ((file->f_flags & O_ACCMODE) != O_RDONLY)
		dvbdev->writers++;

	dvbdev->users++;
	return 0;
}


int dvb_generic_ioctl(struct inode *inode, struct file *file,
		      unsigned int cmd, unsigned long arg)
{
        struct dvb_device *dvbdev = file->private_data;
	
        if (!dvbdev)
	        return -ENODEV;

	if (!dvbdev->kernel_ioctl)
		return -EINVAL;

	return video_usercopy (inode, file, cmd, arg, dvbdev->kernel_ioctl);
}


static
int dvbdev_get_free_id (struct dvb_adapter *adap, int type)
{
	u32 id = 0;

	while (id < DVB_MAX_IDS) {
		struct list_head *entry;
		list_for_each (entry, &adap->device_list) {
			struct dvb_device *dev;
			dev = list_entry (entry, struct dvb_device, list_head);
			if (dev->type == type && dev->id == id)
				goto skip;
		}
		return id;
skip:
		id++;
	}
	return -ENFILE;
}


int dvb_register_device(struct dvb_adapter *adap, struct dvb_device **pdvbdev, 
			const struct dvb_device *template, void *priv, int type)
{
	u32 id;
	char name [20];
	struct dvb_device *dvbdev;

	if (down_interruptible (&dvbdev_register_lock))
		return -ERESTARTSYS;

	if ((id = dvbdev_get_free_id (adap, type)) < 0) {
		up (&dvbdev_register_lock);
		*pdvbdev = 0;
		printk ("%s: could get find free device id...\n", __FUNCTION__);
		return -ENFILE;
	}

	*pdvbdev = dvbdev = kmalloc(sizeof(struct dvb_device), GFP_KERNEL);

	if (!dvbdev) {
		up(&dvbdev_register_lock);
		return -ENOMEM;
	}

	up (&dvbdev_register_lock);
	
	memcpy(dvbdev, template, sizeof(struct dvb_device));
	dvbdev->type = type;
	dvbdev->id = id;
	dvbdev->adapter = adap;
	dvbdev->priv = priv;

	list_add_tail (&dvbdev->list_head, &adap->device_list);

	sprintf(name, "%s%d", dnames[type], id);
	dvbdev->devfs_handle = devfs_register(adap->devfs_handle, name,
					      DVB_DEVFS_FLAGS,
					      DVB_MAJOR,
					      nums2minor(adap->num, type, id),
					      S_IFCHR | S_IRUSR | S_IWUSR,
					      dvbdev->fops, dvbdev);

	dprintk("DVB: register adapter%d/%s @ minor: %i (0x%02x)\n",
		adap->num, name, nums2minor(adap->num, type, id),
		nums2minor(adap->num, type, id));

	return 0;
}


void dvb_unregister_device(struct dvb_device *dvbdev)
{
	if (!dvbdev)
		return;

	devfs_unregister(dvbdev->devfs_handle);
	list_del (&dvbdev->list_head);
	kfree (dvbdev);
}


static
int dvbdev_get_free_adapter_num (void)
{
	int num = 0;

	while (1) {
		struct list_head *entry;
		list_for_each (entry, &dvb_adapter_list) {
			struct dvb_adapter *adap;
			adap = list_entry (entry, struct dvb_adapter, list_head);
			if (adap->num == num)
				goto skip;
		}
		return num;
skip:
		num++;
	}

	return -ENFILE;
}


int dvb_register_adapter(struct dvb_adapter **padap, const char *name)
{
	char dirname[10];
	struct dvb_adapter *adap;
	int num;

	if (down_interruptible (&dvbdev_register_lock))
		return -ERESTARTSYS;

	if ((num = dvbdev_get_free_adapter_num ()) < 0) {
		up (&dvbdev_register_lock);
		return -ENFILE;
	}

	if (!(*padap = adap = kmalloc(sizeof(struct dvb_adapter), GFP_KERNEL))) {
		up(&dvbdev_register_lock);
		return -ENOMEM;
	}

	memset (adap, 0, sizeof(struct dvb_adapter));
	INIT_LIST_HEAD (&adap->device_list);

	MOD_INC_USE_COUNT;

	printk ("DVB: registering new adapter (%s).\n", name);
	
	sprintf(dirname, "adapter%d", num);
	adap->devfs_handle = devfs_mk_dir(dvb_devfs_handle, dirname, NULL);
	adap->num = num;
	adap->name = name;

	list_add_tail (&adap->list_head, &dvb_adapter_list);

	up (&dvbdev_register_lock);

	return num;
}


int dvb_unregister_adapter(struct dvb_adapter *adap)
{
        devfs_unregister (adap->devfs_handle);
	if (down_interruptible (&dvbdev_register_lock))
		return -ERESTARTSYS;
	list_del (&adap->list_head);
	up (&dvbdev_register_lock);
	kfree (adap);
	MOD_DEC_USE_COUNT;
	return 0;
}


static
int __init init_dvbdev(void)
{
	dvb_devfs_handle = devfs_mk_dir (NULL, "dvb", NULL);
#ifndef CONFIG_DVB_DEVFS_ONLY
	if(register_chrdev(DVB_MAJOR,"DVB", &dvb_device_fops)) {
		printk("video_dev: unable to get major %d\n", DVB_MAJOR);
		return -EIO;
	}
#endif
	return 0;
}


static 
void __exit exit_dvbdev(void)
{
#ifndef CONFIG_DVB_DEVFS_ONLY
	unregister_chrdev(DVB_MAJOR, "DVB");
#endif
        devfs_unregister(dvb_devfs_handle);
}

module_init(init_dvbdev);
module_exit(exit_dvbdev);

MODULE_DESCRIPTION("DVB Core Driver");
MODULE_AUTHOR("Marcus Metzler, Ralph Metzler, Holger Waechtler");
MODULE_LICENSE("GPL");

MODULE_PARM(dvbdev_debug,"i");
MODULE_PARM_DESC(dvbdev_debug, "enable verbose debug messages");

