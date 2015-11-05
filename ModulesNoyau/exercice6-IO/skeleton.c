#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/ioport.h>

const unsigned long addr = 0x10000000;
const unsigned long mem_len = 0x100;

static int __init skeleton_init(void)
{
	void* regs;
	int memId;

	pr_info("Linux module skeleton loaded\n");
	
	// Allocate memory region
	request_mem_region(addr,mem_len,"uP register");

	pr_info("Memory allocated\n");

	// Mapp memory with linux noyau
	regs = ioremap(addr, mem_len);
	if (regs == NULL) {
		pr_info ("Error while trying to map processor chipid register...\n");
		return -EFAULT;
	}

	// Read memory id;
	memId = ioread32(regs+0x00);
	pr_info("uP register: Bit 31..12 : product id=0x%05x\n",(memId>>12)&0x7ffff);
	pr_info("uP register: Bit 11..8  : package id=0x%01x\n",(memId>>8)&0xf);
	pr_info("uP register: Bit 7..4   : major revision=0x%01x\n",(memId>>4)&0xf);
	pr_info("uP register: Bit 3..0   : minor revision=0x%01x\n",(memId)&0xf);

	iounmap(regs);

	return 0;
}

static void __exit skeleton_exit(void)
{
	pr_info("Linux module skeleton unloaded\n");

	// Free memory region
	release_mem_region(addr,mem_len);

	pr_info("Memory released\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Emilie Gsponer");
MODULE_DESCRIPTION("Memory map");
MODULE_LICENSE("GPL");
