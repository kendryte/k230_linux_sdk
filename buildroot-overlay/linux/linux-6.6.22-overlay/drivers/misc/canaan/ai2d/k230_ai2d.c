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

#define ai2d_log(format, ...)       do{if (debug) {printk("AI2D: " format, ##__VA_ARGS__);}} while(0);

#define AI2D_ACLK_CFG       (0x97001020U)
#define AI2D_SYSCLK_CFG     (0x97001028U)

#define AI2D_ENABLE_ACLK    (0x08000200U)
#define AI2D_DISABLE_ACLK   (0x08000000U)
#define AI2D_ENABLE_SYSCLK  (0x04000100U)
#define AI2D_DISABLE_SYSCLK (0x04000000U)

struct ai2d_plat {
    struct resource *res;
    void __iomem    *regs;
    char      *ai2d_reg;
    int             major;
    int             minor;
    int             irq;
    struct class    *class;
    struct device   *device;
    struct cdev     cdev;
};

static int debug = 0;
static struct ai2d_plat *plat;
static uint64_t ai2d_membase[4];
static unsigned int ai2d_int_flag;
static int ai2d_fasync_flag = 0;
DECLARE_WAIT_QUEUE_HEAD(ai2d_waitq);//注册一个等待队列ai2d_waitq
struct fasync_struct *ai2d_fasync;

static DEFINE_MUTEX(ai2d_mutex);

static void ai2d_disable(void)
{

}

static int ai2d_enable(void)
{
    return 0;
}

static void ai2d_interrupt_clear(void)
{
    iowrite64(0x400000004, (plat->ai2d_reg + 0x128));
}

static irqreturn_t ai2d_irq(int irq, void *dev_id)
{
    ai2d_interrupt_clear();
    ai2d_log("Gnne interrupt\n");
    ai2d_int_flag = 1;
    wake_up_interruptible(&ai2d_waitq);   /* 唤醒休眠的进程，即调用read函数的进程 */

    if (ai2d_fasync_flag)
        kill_fasync(&ai2d_fasync, SIGIO, POLL_IN); // 发送信号
    return IRQ_HANDLED;
}

static int ai2d_drv_fasync(int fd, struct file *file, int on)
{
    int err;
    ai2d_log("fansync_helper %08X\n", on);
    err = fasync_helper(fd, file, on, &ai2d_fasync);
    if(err < 0) {
        return err;
    }
    ai2d_fasync_flag = on;

    return 0;
}

static unsigned int ai2d_poll(struct file *file, poll_table *wait)
{
    unsigned int ret = 0;
    poll_wait(file, &ai2d_waitq, wait);//将当前进程放到ai2d_waitq列表
    if(ai2d_int_flag) {
        ret |= POLLIN;//说明有数据被取到了
        ai2d_int_flag = 0;
    }

    return ret;
}

static long ai2d_ioctl(struct file *filp, unsigned int cmd,
                             unsigned long arg)
{
    return 0;
}

static long ai2d_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    long ret = 0;
    return ret;
}

static int ai2d_power_on(void)
{
    void __iomem *sysctl_reg;

    sysctl_reg = ioremap(AI2D_ACLK_CFG, 4);
    if (!sysctl_reg) {
        ai2d_log("can't remap ai2d sysctl 0x%08X\n", AI2D_ACLK_CFG);
        return -1;
    }
    iowrite32(AI2D_ENABLE_ACLK, sysctl_reg);
    iounmap(sysctl_reg);

    sysctl_reg = ioremap(AI2D_SYSCLK_CFG, 4);
    if (!sysctl_reg) {
        ai2d_log("can't remap ai2d sysctl 0x%08X\n", AI2D_SYSCLK_CFG);
        return -1;
    }
    iowrite32(AI2D_ENABLE_SYSCLK, sysctl_reg);
    iounmap(sysctl_reg);

    return 0;
}

static int ai2d_power_off(void)
{
    void __iomem *sysctl_reg;

    sysctl_reg = ioremap(AI2D_ACLK_CFG, 4);
    if (!sysctl_reg) {
        ai2d_log("can't remap ai2d sysctl 0x%08X\n", AI2D_ACLK_CFG);
        return -1;
    }
    iowrite32(AI2D_DISABLE_ACLK, sysctl_reg);
    iounmap(sysctl_reg);

    sysctl_reg = ioremap(AI2D_SYSCLK_CFG, 4);
    if (!sysctl_reg) {
        ai2d_log("can't remap ai2d sysctl 0x%08X\n", AI2D_SYSCLK_CFG);
        return -1;
    }
    iowrite32(AI2D_DISABLE_SYSCLK, sysctl_reg);
    iounmap(sysctl_reg);

    return 0;
}

static int ai2d_open(struct inode *inode, struct file *filp)
{
    ai2d_int_flag = 0;
    ai2d_power_on();
    ai2d_disable();
    ai2d_log("AI2D: open ai2d success\n");
    return 0;
}

static int ai2d_release(struct inode *inode, struct file *filp)
{
    ai2d_int_flag = 0;
    ai2d_disable();
    ai2d_power_off();

    return 0;
}

const struct file_operations ai2d_fops = {
    .owner          = THIS_MODULE,
    .open           = ai2d_open,
    .release        = ai2d_release,
    .unlocked_ioctl = ai2d_unlocked_ioctl,
    .poll           = ai2d_poll,
    .fasync         = ai2d_drv_fasync,
};

static int ai2d_probe(struct platform_device *pdev)
{
    struct resource *res;
    int err = 0;
    dev_t dev = 0;
    int devno;
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res) {
        ai2d_log("ai2d_probe: get resource failed \n");
        err = -ENODEV;
        goto error;
    }

    plat = kzalloc(sizeof(struct ai2d_plat), GFP_KERNEL);
    if (!plat) {
        ai2d_log("ai2d_probe: kzalloc failed \n");
        err = -ENOMEM;
        goto error;
    }

    plat->res = res;

    plat->irq = platform_get_irq(pdev, 0);
    ai2d_log("Gnne irq number is %d\n", plat->irq);
    if(plat->irq < 0) {
        ai2d_log("Gnne get irq err\n");
        err = -ENODEV;
        goto cleanup_kmem;
    }

    if((err = request_irq(plat->irq, ai2d_irq, 0, "t0", NULL))) {
        ai2d_log("Gnne request irq err %d\n", err);
        goto cleanup_kmem;
    }

    plat->regs = ioremap(res->start, resource_size(res));
    if(!plat->regs) {
        dev_dbg(&pdev->dev, "could not remap register memory\n");
        err = -ENOMEM;
        goto cleanup_irq;
    }
    plat->ai2d_reg = plat->regs;

    plat->major = 0;
    plat->minor = 0;

    err = alloc_chrdev_region(&dev, 0, 1, "k230-ai2d");
    if (err) {
        ai2d_log("k230-ai2d: can't get major %d\n", plat->major);
        goto cleanup_ioremap;
    }
    plat->major = MAJOR(dev);

    // setup_chrdev_region();
    plat->class = class_create("k230_ai2d_class");
    if (IS_ERR(plat->class)) {
        err = PTR_ERR(plat->class);
        goto cleanup_ioremap;
    }
    // create_module_class();
    devno = MKDEV(plat->major, plat->minor);

    cdev_init(&plat->cdev, &ai2d_fops);
    plat->cdev.owner = THIS_MODULE;
    err = cdev_add(&plat->cdev, devno, 1);
    if (err) {
        ai2d_log("Error %d adding ai2d device number %d \n", err, plat->minor);
        goto cleanup_class;
    }

    plat->device = device_create(plat->class, NULL, devno, NULL, "k230-ai2d");
    if (IS_ERR(plat->device)) {
        ai2d_log("device not created\n");
        err = PTR_ERR(plat->device);
        goto cleanup_cdev;
    }

    ai2d_log("k230 ai2d driver loaded\n");

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

static int ai2d_remove(struct platform_device *pdev)
{
    dev_t dev = MKDEV(plat->major, plat->minor);

    device_destroy(plat->class, dev);
    // clean_up_ai2d_cdev();
    cdev_del(&plat->cdev);
    class_destroy(plat->class);
    iounmap(plat->regs);
    free_irq(plat->irq, NULL);
    kfree(plat);

    return 0;
}

static const struct of_device_id k230_ai2d_ids[] = {
	{ .compatible = "k230-ai2d" },
	{}
};

static struct platform_driver k230_ai2d_driver = {
    .probe          = ai2d_probe,
    .remove         = ai2d_remove,
    .driver         = {
        .name           = "k230-ai2d",
        .of_match_table = of_match_ptr(k230_ai2d_ids),
    },
};

int ai2d_module_init(void)
{
    int ret;
    ret = platform_driver_register(&k230_ai2d_driver);
    return ret;
}

void ai2d_module_deinit(void)
{
    platform_driver_unregister(&k230_ai2d_driver);
}

module_init(ai2d_module_init);
module_exit(ai2d_module_deinit);
module_param(debug, int, S_IRUGO);
MODULE_LICENSE("GPL v2");
