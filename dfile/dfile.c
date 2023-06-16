// clang-format off
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <asm/current.h>
#include <asm/uaccess.h>
// clang-format on

MODULE_LICENSE("Dual BSD/GPL");

#define DRIVER_NAME "DFile"
#define DRIVER_MAJOR 63

static int dfile_open(struct inode *inode, struct file *file) {
  printk("dfile_open\n");
  return 0;
}

static int dfile_close(struct inode *inode, struct file *file) {
  printk("dfile_close\n");
  return 0;
}

static ssize_t dfile_read(struct file *filp, char __user *buf, size_t count,
                          loff_t *f_pos) {
  char data = 'A';
  printk("dfile_read\n");
  if (copy_to_user(buf, &data, sizeof(char))) {
    return -EFAULT;
  }
  return sizeof(char);
}

static ssize_t dfile_write(struct file *filp, const char __user *buf,
                           size_t count, loff_t *f_pos) {
  printk("dfile_write\n");
  return 1;
}

struct file_operations s_dfile_fops = {
    .open = dfile_open,
    .release = dfile_close,
    .read = dfile_read,
    .write = dfile_write,
};

static int dfile_init(void) {
  printk("dfile_init\n");
  register_chrdev(DRIVER_MAJOR, DRIVER_NAME, &s_dfile_fops);
  return 0;
}

static void dfile_exit(void) {
  printk("dfile_exit\n");
  unregister_chrdev(DRIVER_MAJOR, DRIVER_NAME);
}

module_init(dfile_init);
module_exit(dfile_exit);