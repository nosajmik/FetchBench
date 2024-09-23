#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include "tester.h"

MODULE_LICENSE("GPL");

static int device_open(struct inode *inode, struct file *file) {
	try_module_get(THIS_MODULE);
	return 0;
}

static int device_release(struct inode *inode, struct file *file) {
	module_put(THIS_MODULE);
	return 0;
}

static ssize_t device_read(struct file *file, char __user * buffer, size_t length, loff_t * offset) {
	return 0;
}

static ssize_t device_write(struct file *file, const char __user * buffer, size_t length, loff_t * offset) {
	return 0;
}

extern void __asm_dcache_level(uint64_t level, uint64_t invalidate);

static void clear_l2(void* info) {
	__asm_dcache_level(1, 1);
}

static void clear_l1(void* info) {
	__asm_dcache_level(0, 1);
}

static void clear_l3(void* info) {
	__asm_dcache_level(2, 1);
}

long int device_ioctl(
		struct file *file,
		unsigned int ioctl_num,
		unsigned long ioctl_param)
{
	uint64_t* param = (uint64_t*)ioctl_param;
	uint64_t reg = 0;
	switch (ioctl_num) {
		case IOCTL_CurrentEL:
			reg = read_sysreg(CurrentEL);
			if(copy_to_user(param, &reg, sizeof(reg)) != 0) return -1;
			break;
		case IOCTL_CPUECTLR_EL1:
			reg = read_sysreg(s3_0_c15_c1_4);
			if(copy_to_user(param, &reg, sizeof(reg)) != 0) return -1;
			break;
		case IOCTL_PMUSERENR_EL0:
			reg = read_sysreg(PMUSERENR_EL0);
			if(copy_to_user(param, &reg, sizeof(reg)) != 0) return -1;
			break;
		case IOCTL_PMCR_EL0:
			reg = read_sysreg(PMCR_EL0);
			if(copy_to_user(param, &reg, sizeof(reg)) != 0) return -1;
			break;
		
		case IOCTL_PMUSERENR_EL0_WRITE:
			if(copy_from_user(&reg, param, sizeof(reg)) != 0) return -1;
			write_sysreg(reg, PMUSERENR_EL0);
			break;
		case IOCTL_PMCR_EL0_WRITE:
			if(copy_from_user(&reg, param, sizeof(reg)) != 0) return -1;
			write_sysreg(reg, PMCR_EL0);
			break;
		case IOCTL_CPUECTLR_EL1_WRITE:
			if(copy_from_user(&reg, param, sizeof(reg)) != 0) return -1;
			write_sysreg(reg, s3_0_c15_c1_4);
			break;

		case IOCTL_CLEAR_CACHE_L1:
			on_each_cpu(clear_l1, NULL, 1);
			break;
		case IOCTL_CLEAR_CACHE_L2:
			on_each_cpu(clear_l2, NULL, 1);
			break;
		case IOCTL_CLEAR_CACHE_L3:
			on_each_cpu(clear_l3, NULL, 1);
			break;
	
	}
	return 0;
}

struct file_operations Fops = {
	.read = device_read,
	.write = device_write,
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release,
};

int init_module() {
	int ret_val;
	ret_val = register_chrdev(MAJOR_NUM, "primer", &Fops);

	if (ret_val < 0) {
		return ret_val;
	}

	return 0;
}

void cleanup_module() {
	unregister_chrdev(MAJOR_NUM, "primer");
}
