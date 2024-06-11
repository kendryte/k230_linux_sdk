#include <linux/cdev.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include <linux/irqflags.h>
#include <linux/module.h>
#include <linux/cpu.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/cacheinfo.h>
#include <linux/sizes.h>
#include <asm/csr.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/irq.h>    //含有IRQ_HANDLED\IRQ_TYPE_EDGE_RISING
// #include <asm-arm/irq.h>   //含有IRQT_BOTHEDGE触发类型
#include <linux/interrupt.h> //含有request_irq、free_irq函数
#include <linux/poll.h>

#define gnne_log(format, ...)       do{if (debug) {printk("GNNE: " format, ##__VA_ARGS__);}} while(0);

#define GNNE_ACLK_CFG       (0x97001020U)
#define GNNE_SYSCLK_CFG     (0x97001028U)

#define GNNE_ENABLE_ACLK    (0x08000200U)
#define GNNE_DISABLE_ACLK   (0x08000000U)
#define GNNE_ENABLE_SYSCLK  (0x04000100U)
#define GNNE_DISABLE_SYSCLK (0x04000000U)

struct gnne_plat {
    struct resource *res;
    void __iomem    *regs;
    char      *gnne_reg;
    int             major;
    int             minor;
    int             irq;
    struct class    *class;
    struct device   *device;
    struct cdev     cdev;
};

static int debug = 0;
static struct gnne_plat *plat;
static uint64_t gnne_membase[4];
static unsigned int gnne_int_flag;
static int gnne_fasync_flag = 0;
DECLARE_WAIT_QUEUE_HEAD(gnne_waitq);//注册一个等待队列gnne_waitq
struct fasync_struct *gnne_fasync;

static DEFINE_MUTEX(gnne_mutex);

static void gnne_disable(void)
{

}

static int gnne_enable(void)
{
    return 0;
}

static void gnne_interrupt_clear(void)
{
    iowrite64(0x400000004, (plat->gnne_reg + 0x128));
}

static irqreturn_t gnne_irq(int irq, void *dev_id)
{
    gnne_interrupt_clear();
    gnne_log("Gnne interrupt\n");
    gnne_int_flag = 1;
    wake_up_interruptible(&gnne_waitq);   /* 唤醒休眠的进程，即调用read函数的进程 */

    if (gnne_fasync_flag)
        kill_fasync(&gnne_fasync, SIGIO, POLL_IN); // 发送信号
    return IRQ_HANDLED;
}

static int gnne_drv_fasync(int fd, struct file *file, int on)
{
    int err;
    gnne_log("fansync_helper %08X\n", on);
    err = fasync_helper(fd, file, on, &gnne_fasync);
    if(err < 0) {
        return err;
    }
    gnne_fasync_flag = on;

    return 0;
}

static unsigned int gnne_poll(struct file *file, poll_table *wait)
{
    unsigned int ret = 0;
    poll_wait(file, &gnne_waitq, wait);//将当前进程放到gnne_waitq列表
    if(gnne_int_flag) {
        ret |= POLLIN;//说明有数据被取到了
        gnne_int_flag = 0;
    }

    return ret;
}

static long gnne_ioctl(struct file *filp, unsigned int cmd,
                             unsigned long arg)
{
    return 0;
}

static long gnne_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    long ret = 0;
    return ret;
}

static int gnne_power_on(void)
{
    void __iomem *sysctl_reg;

    sysctl_reg = ioremap(GNNE_ACLK_CFG, 4);
    if (!sysctl_reg) {
        gnne_log("can't remap gnne sysctl 0x%08X\n", GNNE_ACLK_CFG);
        return -1;
    }
    iowrite32(GNNE_ENABLE_ACLK, sysctl_reg);
    iounmap(sysctl_reg);

    sysctl_reg = ioremap(GNNE_SYSCLK_CFG, 4);
    if (!sysctl_reg) {
        gnne_log("can't remap gnne sysctl 0x%08X\n", GNNE_SYSCLK_CFG);
        return -1;
    }
    iowrite32(GNNE_ENABLE_SYSCLK, sysctl_reg);
    iounmap(sysctl_reg);

    return 0;
}

static int gnne_power_off(void)
{
    void __iomem *sysctl_reg;

    sysctl_reg = ioremap(GNNE_ACLK_CFG, 4);
    if (!sysctl_reg) {
        gnne_log("can't remap gnne sysctl 0x%08X\n", GNNE_ACLK_CFG);
        return -1;
    }
    iowrite32(GNNE_DISABLE_ACLK, sysctl_reg);
    iounmap(sysctl_reg);

    sysctl_reg = ioremap(GNNE_SYSCLK_CFG, 4);
    if (!sysctl_reg) {
        gnne_log("can't remap gnne sysctl 0x%08X\n", GNNE_SYSCLK_CFG);
        return -1;
    }
    iowrite32(GNNE_DISABLE_SYSCLK, sysctl_reg);
    iounmap(sysctl_reg);

    return 0;
}

static int gnne_open(struct inode *inode, struct file *filp)
{
    gnne_int_flag = 0;
    gnne_power_on();
    gnne_disable();
    gnne_log("GNNE: open gnne success\n");
    return 0;
}

static int gnne_release(struct inode *inode, struct file *filp)
{
    gnne_int_flag = 0;
    gnne_disable();
    gnne_power_off();

    return 0;
}

const struct file_operations gnne_fops = {
    .owner          = THIS_MODULE,
    .open           = gnne_open,
    .release        = gnne_release,
    .unlocked_ioctl = gnne_unlocked_ioctl,
    .poll           = gnne_poll,
    .fasync         = gnne_drv_fasync,
};

static int gnne_probe(struct platform_device *pdev)
{
    struct resource *res;
    int err = 0;
    dev_t dev = 0;
    int devno;
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res) {
        gnne_log("gnne_probe: get resource failed \n");
        err = -ENODEV;
        goto error;
    }

    plat = kzalloc(sizeof(struct gnne_plat), GFP_KERNEL);
    if (!plat) {
        gnne_log("gnne_probe: kzalloc failed \n");
        err = -ENOMEM;
        goto error;
    }

    plat->res = res;

    plat->irq = platform_get_irq(pdev, 0);
    gnne_log("Gnne irq number is %d\n", plat->irq);
    if(plat->irq < 0) {
        gnne_log("Gnne get irq err\n");
        err = -ENODEV;
        goto cleanup_kmem;
    }

    if((err = request_irq(plat->irq, gnne_irq, 0, "t0", NULL))) {
        gnne_log("Gnne request irq err %d\n", err);
        goto cleanup_kmem;
    }

    plat->regs = ioremap(res->start, resource_size(res));
    if(!plat->regs) {
        dev_dbg(&pdev->dev, "could not remap register memory\n");
        err = -ENOMEM;
        goto cleanup_irq;
    }
    plat->gnne_reg = plat->regs;

    plat->major = 0;
    plat->minor = 0;

    err = alloc_chrdev_region(&dev, 0, 1, "k230-gnne");
    if (err) {
        gnne_log("k230-gnne: can't get major %d\n", plat->major);
        goto cleanup_ioremap;
    }
    plat->major = MAJOR(dev);

    // setup_chrdev_region();
    plat->class = class_create("k230_gnne_class");
    if (IS_ERR(plat->class)) {
        err = PTR_ERR(plat->class);
        goto cleanup_ioremap;
    }
    // create_module_class();
    devno = MKDEV(plat->major, plat->minor);

    cdev_init(&plat->cdev, &gnne_fops);
    plat->cdev.owner = THIS_MODULE;
    err = cdev_add(&plat->cdev, devno, 1);
    if (err) {
        gnne_log("Error %d adding gnne device number %d \n", err, plat->minor);
        goto cleanup_class;
    }

    plat->device = device_create(plat->class, NULL, devno, NULL, "k230-gnne");
    if (IS_ERR(plat->device)) {
        gnne_log("device not created\n");
        err = PTR_ERR(plat->device);
        goto cleanup_cdev;
    }

    gnne_log("k230 gnne driver loaded\n");

	return 0;

cleanup_device:
    device_destroy(plat->class, dev);
cleanup_cdev:
    cdev_del(&plat->cdev);
cleanup_class:
    class_destroy(plat->class);
cleanup_ioremap:
    iounmap(plat->regs);
cleanup_irq:
    free_irq(plat->irq, NULL);
cleanup_kmem:
    kfree(plat);
error:
	return err;
}

static int gnne_remove(struct platform_device *pdev)
{
    dev_t dev = MKDEV(plat->major, plat->minor);

    device_destroy(plat->class, dev);
    // clean_up_gnne_cdev();
    cdev_del(&plat->cdev);
    class_destroy(plat->class);
    iounmap(plat->regs);
    free_irq(plat->irq, NULL);
    kfree(plat);

    return 0;
}

static const struct of_device_id k230_gnne_ids[] = {
	{ .compatible = "k230-gnne" },
	{}
};

static struct platform_driver k230_gnne_driver = {
    .probe          = gnne_probe,
    .remove         = gnne_remove,
    .driver         = {
        .name           = "k230-gnne",
        .of_match_table = of_match_ptr(k230_gnne_ids),
    },
};

int gnne_module_init(void)
{
    int ret;
    ret = platform_driver_register(&k230_gnne_driver);
    return ret;
}

void gnne_module_deinit(void)
{
    platform_driver_unregister(&k230_gnne_driver);
}

module_init(gnne_module_init);
module_exit(gnne_module_deinit);
module_param(debug, int, S_IRUGO);
MODULE_LICENSE("GPL v2");
