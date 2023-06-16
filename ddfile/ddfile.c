// clang-format off
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <asm/current.h>
#include <asm/uaccess.h>
// clang-format on

MODULE_LICENSE("Dual BSD/GPL");

/* /proc/devices等で表示されるデバイス名 */
#define DRIVER_NAME "DDFile"
/*** 各ファイル(open毎に作られるファイルディスクリプタ)に紐づく情報 ***/
#define NUM_BUFFER 256
struct _ddfile_data {
  unsigned char buffer[NUM_BUFFER];
};

/* このデバイスドライバで使うマイナー番号の開始番号と個数(=デバイス数) */
/* マイナー番号は 0 ~ 1 */
static const unsigned int MINOR_BASE = 0;
static const unsigned int MINOR_NUM = 2;

/* このデバイスドライバのメジャー番号(動的に決める) */
static unsigned int ddfile_major;

/* キャラクタデバイスのオブジェクト */
static struct cdev ddfile_cdev;

/* デバイスドライバのクラスオブジェクト */
static struct class *ddfile_class = NULL;

static int ddfile_open(struct inode *inode, struct file *file) {
  printk("ddfile_open\n");

  /* 各ファイル固有のデータを格納する領域を確保する */
  struct _ddfile_data *p = kmalloc(sizeof(struct _ddfile_data), GFP_KERNEL);
  if (p == NULL) {
    printk(KERN_ERR "kmalloc\n");
    return -ENOMEM;
  }

  /* ファイル固有データを初期化する */
  strlcat(p->buffer, "dummy", 5);

  /* 確保したポインタはユーザ側のfdで保持してもらう */
  file->private_data = p;

  return 0;
}

static int ddfile_close(struct inode *inode, struct file *file) {
  printk("ddfile_close\n");

  if (file->private_data) {
    /* open時に確保した、各ファイル固有のデータ領域を解放する */
    kfree(file->private_data);
    file->private_data = NULL;
  }

  return 0;
}

static ssize_t ddfile_read(struct file *filp, char __user *buf, size_t count,
                           loff_t *f_pos) {
  printk("ddfile_read\n");
  if (count > NUM_BUFFER) count = NUM_BUFFER;

  struct _ddfile_data *p = filp->private_data;
  if (copy_to_user(buf, p->buffer, count)) {
    return -EFAULT;
  }
  return count;
}

static ssize_t ddfile_write(struct file *filp, const char __user *buf,
                            size_t count, loff_t *f_pos) {
  printk("ddfile_write : ");

  struct _ddfile_data *p = filp->private_data;
  if (raw_copy_from_user(p->buffer, buf, count)) {
    return -EFAULT;
  }
  printk("%s\n", p->buffer);
  return count;
}

struct file_operations s_ddfile_fops = {
    .open = ddfile_open,
    .release = ddfile_close,
    .read = ddfile_read,
    .write = ddfile_write,
};

static int ddfile_init(void) {
  printk("ddfile_init\n");

  int alloc_ret = 0;
  int cdev_err = 0;
  dev_t dev;

  /* 1. 空いているメジャー番号を確保する */
  alloc_ret = alloc_chrdev_region(&dev, MINOR_BASE, MINOR_NUM, DRIVER_NAME);
  if (alloc_ret != 0) {
    printk(KERN_ERR "alloc_chrdev_region = %d\n", alloc_ret);
    return -1;
  }

  /* 2. 取得したdev( = メジャー番号 +
   * マイナー番号)からメジャー番号を取得して保持しておく */
  ddfile_major = MAJOR(dev);
  dev = MKDEV(ddfile_major, MINOR_BASE); /* 不要? */

  /* 3. cdev構造体の初期化とシステムコールハンドラテーブルの登録 */
  cdev_init(&ddfile_cdev, &s_ddfile_fops);
  ddfile_cdev.owner = THIS_MODULE;

  /* 4. このデバイスドライバ(cdev)をカーネルに登録する */
  cdev_err = cdev_add(&ddfile_cdev, dev, MINOR_NUM);
  if (cdev_err != 0) {
    printk(KERN_ERR "cdev_add = %d\n", cdev_err);
    unregister_chrdev_region(dev, MINOR_NUM);
    return -1;
  }

  /* 5. このデバイスのクラス登録をする(/sys/class/ddfile/ を作る) */
  ddfile_class = class_create(THIS_MODULE, "ddfile");
  if (IS_ERR(ddfile_class)) {
    printk(KERN_ERR "class_create\n");
    cdev_del(&ddfile_cdev);
    unregister_chrdev_region(dev, MINOR_NUM);
    return -1;
  }

  /* 6. /sys/class/ddfile/ddfile* を作る
  これにより、udevdがクラス登録を検出して自動的にデバイスファイルを作ってくれる　*/
  for (int minor = MINOR_BASE; minor < MINOR_BASE + MINOR_NUM; minor++) {
    device_create(ddfile_class, NULL, MKDEV(ddfile_major, minor), NULL,
                  "ddfile%d", minor);
  }

  return 0;
}

static void ddfile_exit(void) {
  printk("ddfile_exit\n");

  dev_t dev = MKDEV(ddfile_major, MINOR_BASE);

  /* 7. /sys/class/mydevice/mydevice* を削除する */
  for (int minor = MINOR_BASE; minor < MINOR_BASE + MINOR_NUM; minor++) {
    device_destroy(ddfile_class, MKDEV(ddfile_major, minor));
  }

  /* 8. このデバイスのクラス登録を取り除く(/sys/class/mydevice/を削除する) */
  class_destroy(ddfile_class);

  /* 9. このデバイスドライバ(cdev)をカーネルから取り除く */
  cdev_del(&ddfile_cdev);

  /* 10. このデバイスドライバで使用していたメジャー番号の登録を取り除く */
  unregister_chrdev_region(dev, MINOR_NUM);
}

module_init(ddfile_init);
module_exit(ddfile_exit);
