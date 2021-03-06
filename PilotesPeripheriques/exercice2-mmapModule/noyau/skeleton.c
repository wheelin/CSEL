#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include "linux/fs.h"
#include "linux/kdev_t.h"
#include "linux/cdev.h"
#include "linux/uaccess.h"
#include <linux/mm.h>

static int major = 0; /* Major number */

void vma_open(struct vm_area_struct *vma);
void vma_close(struct vm_area_struct *vma);

static struct vm_operations_struct remap_vma_ops = {
	.open =  vma_open,
	.close = vma_close,
};

static int mod_open(struct inode *inode, struct file *file)
{
	pr_info("mod: open\n");
	return 0;
}

static int mod_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long offset = vma->vm_pgoff;

        unsigned long size = vma->vm_end - vma->vm_start;
        
        if (remap_pfn_range(vma, vma->vm_start, offset,
                size,
                vma->vm_page_prot))

        	return -EFAULT;
	pr_info("mod: mmap\n");

	vma->vm_ops = &remap_vma_ops;
	vma_open(vma);

	return 0;
}

void vma_open(struct vm_area_struct *vma)
{
    printk(KERN_NOTICE "VMA open, virt %lx, phys %lx\n",
            vma->vm_start, vma->vm_pgoff << PAGE_SHIFT);
}

void vma_close(struct vm_area_struct *vma)
{
    printk(KERN_NOTICE "VMA close.\n");
}

static int mod_release(struct inode *inode, struct file *file)
{
	pr_info("mod: release\n");
	return 0;
}

static struct file_operations mod_fops = {
	.owner = THIS_MODULE,
	.open = mod_open,
	.release = mod_release,
	.mmap = mod_mmap,
};

static int __init mod_init(void)
{
	int ret;
	ret = register_chrdev(major, "mod", &mod_fops);
	if (ret < 0) {
		pr_info("mod: unable to get a major\n");
		return ret;
	}
	if (major == 0)
		major = ret; /* dynamic value */
	pr_info("mod: successfully loaded with major %d\n",major);
	return 0;
}

static void __exit mod_exit(void)
{
	unregister_chrdev(major, "mod");
	pr_info("mod: successfully unloaded\n");
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_AUTHOR("Emilie Gsponer");
MODULE_DESCRIPTION("driver oriented character");
MODULE_LICENSE("GPL");

