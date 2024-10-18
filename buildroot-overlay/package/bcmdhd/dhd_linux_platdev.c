/*
 * Linux platform device for DHD WLAN adapter
 *
 * Copyright (C) 2024 Synaptics Incorporated. All rights reserved.
 *
 * This software is licensed to you under the terms of the
 * GNU General Public License version 2 (the "GPL") with Broadcom special exception.
 *
 * INFORMATION CONTAINED IN THIS DOCUMENT IS PROVIDED "AS-IS," AND SYNAPTICS
 * EXPRESSLY DISCLAIMS ALL EXPRESS AND IMPLIED WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * AND ANY WARRANTIES OF NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS.
 * IN NO EVENT SHALL SYNAPTICS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, PUNITIVE, OR CONSEQUENTIAL DAMAGES ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OF THE INFORMATION CONTAINED IN THIS DOCUMENT, HOWEVER CAUSED
 * AND BASED ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, AND EVEN IF SYNAPTICS WAS ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE. IF A TRIBUNAL OF COMPETENT JURISDICTION
 * DOES NOT PERMIT THE DISCLAIMER OF DIRECT DAMAGES OR ANY OTHER DAMAGES,
 * SYNAPTICS' TOTAL CUMULATIVE LIABILITY TO ANY PARTY SHALL NOT
 * EXCEED ONE HUNDRED U.S. DOLLARS
 *
 * Copyright (C) 2024, Broadcom.
 *
 *      Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 *
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions of
 * the license of that module.  An independent module is a module which is not
 * derived from this software.  The special exception does not apply to any
 * modifications of the software.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id$
 */
#include <typedefs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <bcmutils.h>
#include <linux_osl.h>
#include <dhd_dbg.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_bus.h>
#include <dhd_linux.h>
#if defined(OEM_ANDROID)
#include <wl_android.h>
#endif
#include <dhd_plat.h>
#if defined(CONFIG_WIFI_CONTROL_FUNC)
#include <linux/wlan_plat.h>
#endif /* CONFIG_WIFI_CONTROL_FUNC */
#ifdef CONFIG_DTS
#include<linux/regulator/consumer.h>
#include<linux/of_gpio.h>
#endif /* CONFIG_DTS */

#if defined(CUSTOMER_HW) || defined(BCMDHD_PLATDEV)
extern int dhd_wlan_init_plat_data(wifi_adapter_info_t *adapter);
extern void dhd_wlan_deinit_plat_data(wifi_adapter_info_t *adapter);
#endif /* CUSTOMER_HW */

#define WIFI_PLAT_NAME		"bcmdhd_wlan"
#define WIFI_PLAT_NAME2		"bcm4329_wlan"
#define WIFI_PLAT_EXT		"bcmdhd_wifi_platform"

#if defined(SUPPORT_MULTIPLE_BOARD_REVISION)
#include <linux/of.h>
extern char* dhd_get_device_dt_name(void);
#endif /* SUPPORT_MULTIPLE_BOARD_REVISION */

#ifdef DHD_WIFI_SHUTDOWN
extern void wifi_plat_dev_drv_shutdown(struct platform_device *pdev);
#endif

#ifdef CONFIG_DTS
struct regulator *wifi_regulator = NULL;
#endif /* CONFIG_DTS */

bool cfg_multichip = FALSE;
bcmdhd_wifi_platdata_t *dhd_wifi_platdata = NULL;
static int wifi_plat_dev_probe_ret = 0;
static bool is_power_on = FALSE;
/* XXX Some Qualcomm based CUSTOMER_HW4 platforms are using platform
 * device structure even if the Kernel uses device tree structure.
 * Therefore, the CONFIG_ARCH_MSM condition is temporarly remained
 * to support in this case.
 */
#if !defined(CONFIG_DTS)
#if defined(DHD_OF_SUPPORT)
static bool dts_enabled = TRUE;
extern struct resource dhd_wlan_resources;
extern struct wifi_platform_data dhd_wlan_control;
#else
static bool dts_enabled = FALSE;
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
struct resource dhd_wlan_resources = {0};
extern struct wifi_platform_data dhd_wlan_control;
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif /* CONFIG_OF && !defined(CONFIG_ARCH_MSM) */
#endif /* !defind(CONFIG_DTS) */

static int dhd_wifi_platform_load(void);

extern void* wl_cfg80211_get_dhdp(struct net_device *dev);

// modify for compatibility
#if defined(BCMDHD_MODULAR) && defined(BOARD_MODULAR_INIT)
//extern int dhd_wlan_init(void);
//extern int dhd_wlan_deinit(void);
#ifdef WBRC
extern int wbrc_init(void);
extern void wbrc_exit(void);
#endif /* WBRC */
#endif /* defined(BCMDHD_MODULAR) && defined(BOARD_MODULAR_INIT) */

#ifdef ENABLE_4335BT_WAR
extern int bcm_bt_lock(int cookie);
extern void bcm_bt_unlock(int cookie);
static int lock_cookie_wifi = 'W' | 'i'<<8 | 'F'<<16 | 'i'<<24;	/* cookie is "WiFi" */
#endif /* ENABLE_4335BT_WAR */

#ifdef BCM4335_XTAL_WAR
extern bool check_bcm4335_rev(void);
#endif /* BCM4335_XTAL_WAR */

#if defined(CONFIG_X86)
#define PCIE_RC_VENDOR_ID 0x8086
#define PCIE_RC_DEVICE_ID 0x9c1a
#elif defined(CONFIG_ARCH_TEGRA)
#define PCIE_RC_VENDOR_ID 0x14e4
#define PCIE_RC_DEVICE_ID 0x4347
#else /* CONFIG_ARCH_TEGRA */
/* Dummy defn */
#define PCIE_RC_VENDOR_ID 0xffff
#define PCIE_RC_DEVICE_ID 0xffff
#endif /* CONFIG_X86 */

wifi_adapter_info_t* dhd_wifi_platform_attach_adapter(uint32 bus_type,
	uint32 bus_num, uint32 slot_num, unsigned long status)
{
	int i;

	if (dhd_wifi_platdata == NULL)
		return NULL;

	for (i = 0; i < dhd_wifi_platdata->num_adapters; i++) {
		wifi_adapter_info_t *adapter = &dhd_wifi_platdata->adapters[i];
		if ((adapter->bus_type == -1 || adapter->bus_type == bus_type) &&
			(adapter->bus_num == -1 || adapter->bus_num == bus_num) &&
			(adapter->slot_num == -1 || adapter->slot_num == slot_num)
#if defined(ENABLE_INSMOD_NO_FW_LOAD) && !defined(ENABLE_INSMOD_NO_POWER_OFF)
			&& (wifi_chk_adapter_status(adapter, status))
#endif
		) {
			DHD_ERROR(("attach adapter info '%s'\n", adapter->name));
			return adapter;
		}
	}
	return NULL;
}

wifi_adapter_info_t* dhd_wifi_platform_get_adapter(uint32 bus_type, uint32 bus_num, uint32 slot_num)
{
	int i;

	if (dhd_wifi_platdata == NULL)
		return NULL;

	for (i = 0; i < dhd_wifi_platdata->num_adapters; i++) {
		wifi_adapter_info_t *adapter = &dhd_wifi_platdata->adapters[i];
		if ((adapter->bus_type == -1 || adapter->bus_type == bus_type) &&
			(adapter->bus_num == -1 || adapter->bus_num == bus_num) &&
			(adapter->slot_num == -1 || adapter->slot_num == slot_num)) {
			DHD_TRACE(("found adapter info '%s'\n", adapter->name));
			return adapter;
		}
	}
	return NULL;
}

#if defined(CONFIG_WIFI_CONTROL_FUNC) && defined(CONFIG_DHD_USE_STATIC_BUF)
extern void *dhd_wlan_mem_prealloc(int section, unsigned long size);
#endif /* CONFIG_WIFI_CONTROL_FUNC && CONFIG_DHD_USE_STATIC_BUF */

void* wifi_platform_prealloc(wifi_adapter_info_t *adapter, int section, unsigned long size)
{
	void *alloc_ptr = NULL;
	struct wifi_platform_data *plat_data;

	if (!adapter || !adapter->wifi_plat_data)
		return NULL;
	plat_data = adapter->wifi_plat_data;
	if (plat_data->mem_prealloc) {
#if defined(BCMDHD_MDRIVER) && !defined(DHD_STATIC_IN_DRIVER)
		alloc_ptr = plat_data->mem_prealloc(adapter->bus_type, adapter->index, section, size);
#else
#if defined(CONFIG_WIFI_CONTROL_FUNC) && defined(CONFIG_DHD_USE_STATIC_BUF)
		alloc_ptr = dhd_wlan_mem_prealloc(section, size);
#else
		alloc_ptr = plat_data->mem_prealloc(section, size);
#endif
#endif
		if (alloc_ptr) {
			DHD_INFO(("success alloc section %d\n", section));
			if (size != 0L)
				bzero(alloc_ptr, size);
			return alloc_ptr;
		}
	} else
		return NULL;

	DHD_ERROR(("%s: failed to alloc static mem section %d\n", __FUNCTION__, section));
	return NULL;
}

void* wifi_platform_get_prealloc_func_ptr(wifi_adapter_info_t *adapter)
{
	struct wifi_platform_data *plat_data;

	if (!adapter || !adapter->wifi_plat_data)
		return NULL;
	plat_data = adapter->wifi_plat_data;
	return plat_data->mem_prealloc;
}

int wifi_platform_get_irq_number(wifi_adapter_info_t *adapter, unsigned long *irq_flags_ptr)
{
	if (adapter == NULL)
		return -1;
	if (irq_flags_ptr)
		*irq_flags_ptr = adapter->intr_flags;
	return adapter->irq_num;
}

int wifi_platform_get_irq_level(wifi_adapter_info_t *adapter)
{
	struct wifi_platform_data  *plat_data = NULL;

	if (adapter == NULL) {
		return BCME_BADARG;
	} else if ((plat_data = adapter->wifi_plat_data) == NULL) {
		return BCME_BADARG;
	}
#ifdef DHD_USE_HOST_WAKE
	else if (plat_data->get_oob_gpio_level == NULL) {
		/* use legacy way */
		extern int dhd_get_wlan_oob_gpio(void);
		return dhd_get_wlan_oob_gpio();
	} else {
		return plat_data->get_oob_gpio_level();
	}
#else // DHD_USE_HOST_WAKE
	else {
		return BCME_UNSUPPORTED;
	}
#endif /* DHD_USE_HOST_WAKE */
}

int wifi_platform_set_power(wifi_adapter_info_t *adapter, bool on, unsigned long msec)
{
	int err = 0;
	struct wifi_platform_data *plat_data;

	if (on) {
		wifi_set_adapter_status(adapter, WIFI_STATUS_POWER_ON);
	} else {
		wifi_clr_adapter_status(adapter, WIFI_STATUS_POWER_ON);
	}
	if (!adapter->wifi_plat_data) {
		err = -EINVAL;
		goto fail;
	}
	plat_data = adapter->wifi_plat_data;

	DHD_ERROR(("%s = %d, delay: %lu msec\n", __FUNCTION__, on, msec));
	if (plat_data->set_power) {
#ifdef ENABLE_4335BT_WAR
		if (on) {
			printk("WiFi: trying to acquire BT lock\n");
			if (bcm_bt_lock(lock_cookie_wifi) != 0)
				printk("** WiFi: timeout in acquiring bt lock**\n");
			printk("%s: btlock acquired\n", __FUNCTION__);
		}
		else {
			/* For a exceptional case, release btlock */
			bcm_bt_unlock(lock_cookie_wifi);
		}
#endif /* ENABLE_4335BT_WAR */

#ifdef CONFIG_WIFI_CONTROL_FUNC
		err = plat_data->set_power(on);
#else
		err = plat_data->set_power(on, adapter);
#endif
	}

	if (msec && !err) {
		OSL_SLEEP(msec);
		DHD_ERROR(("%s = %d, sleep done: %lu msec\n", __FUNCTION__, on, msec));
	}

	if (on && !err)
		is_power_on = TRUE;
	else
		is_power_on = FALSE;

	return err;
fail:
	if (on) {
		wifi_clr_adapter_status(adapter, WIFI_STATUS_POWER_ON);
	} else {
		wifi_set_adapter_status(adapter, WIFI_STATUS_POWER_ON);
	}
	return err;
}

int wifi_platform_bus_enumerate(wifi_adapter_info_t *adapter, bool device_present)
{
	int err = 0;
	struct wifi_platform_data *plat_data;

	if (!adapter || !adapter->wifi_plat_data)
		return -EINVAL;
	plat_data = adapter->wifi_plat_data;

	DHD_ERROR(("%s device present %d\n", __FUNCTION__, device_present));
	if (plat_data->set_carddetect) {
		err = plat_data->set_carddetect(device_present);
	}
	return err;

}

int wifi_platform_get_mac_addr(wifi_adapter_info_t *adapter, unsigned char *buf,
	int ifidx)
{
	struct wifi_platform_data *plat_data;

	DHD_ERROR(("%s\n", __FUNCTION__));
	if (!buf || !adapter || !adapter->wifi_plat_data)
		return -EINVAL;
	plat_data = adapter->wifi_plat_data;
	if (plat_data->get_mac_addr) {
#ifdef CONFIG_WIFI_CONTROL_FUNC
		return plat_data->get_mac_addr(buf);
#else
		return plat_data->get_mac_addr(buf, ifidx);
#endif
	}
	return -EOPNOTSUPP;
}

#ifdef DHD_COREDUMP
int wifi_platform_set_coredump(wifi_adapter_info_t *adapter, const char *buf,
	int buf_len, const char *info)
{
	struct wifi_platform_data *plat_data;

	DHD_ERROR(("%s\n", __FUNCTION__));
	if (!buf || !adapter || !adapter->wifi_plat_data)
		return -EINVAL;
	plat_data = adapter->wifi_plat_data;
	if (plat_data->set_coredump) {
		return plat_data->set_coredump(buf, buf_len, info);
	}
	return -EOPNOTSUPP;
}
#endif /* DHD_COREDUMP */

void *
#ifdef CUSTOM_COUNTRY_CODE
wifi_platform_get_country_code(wifi_adapter_info_t *adapter, char *ccode, u32 flags)
#else
wifi_platform_get_country_code(wifi_adapter_info_t *adapter, char *ccode)
#endif /* CUSTOM_COUNTRY_CODE */
{
	/* get_country_code was added after 2.6.39 */
#if	(LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39))
	struct wifi_platform_data *plat_data;

	if (!ccode || !adapter || !adapter->wifi_plat_data)
		return NULL;
	plat_data = adapter->wifi_plat_data;

	DHD_TRACE(("%s\n", __FUNCTION__));
	if (plat_data->get_country_code) {
#ifdef CUSTOM_FORCE_NODFS_FLAG
		return plat_data->get_country_code(ccode, flags);
#else
		return plat_data->get_country_code(ccode);
#endif /* CUSTOM_COUNTRY_CODE */
	}
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)) */

	return NULL;
}

#ifndef CUSTOMER_HW
static int wifi_plat_dev_drv_probe(struct platform_device *pdev)
{
	struct resource *resource;
	wifi_adapter_info_t *adapter;

	/* Android style wifi platform data device ("bcmdhd_wlan" or "bcm4329_wlan")
	 * is kept for backward compatibility and supports only 1 adapter
	 */
	ASSERT(dhd_wifi_platdata != NULL);
	ASSERT(dhd_wifi_platdata->num_adapters == 1);
	adapter = &dhd_wifi_platdata->adapters[0];
#if defined(CONFIG_WIFI_CONTROL_FUNC)
	adapter->wifi_plat_data = (struct wifi_platform_data *)(pdev->dev.platform_data);
#else
	adapter->wifi_plat_data = (void *)&dhd_wlan_control;
#endif

#ifdef BCMDHD_PLATDEV
	adapter->pdev = pdev;
	wifi_plat_dev_probe_ret = dhd_wlan_init_plat_data(adapter);
	if (!wifi_plat_dev_probe_ret)
		wifi_plat_dev_probe_ret = dhd_wifi_platform_load();
	return wifi_plat_dev_probe_ret;
#endif

	resource = platform_get_resource_byname(pdev, IORESOURCE_IRQ, "bcmdhd_wlan_irq");
	if (resource == NULL)
		resource = platform_get_resource_byname(pdev, IORESOURCE_IRQ, "bcm4329_wlan_irq");
	if (resource) {
		adapter->irq_num = resource->start;
		adapter->intr_flags = resource->flags & IRQF_TRIGGER_MASK;
#ifdef DHD_ISR_NO_SUSPEND
		adapter->intr_flags |= IRQF_NO_SUSPEND;
#endif
	}

	wifi_plat_dev_probe_ret = dhd_wifi_platform_load();
	return wifi_plat_dev_probe_ret;
}

static int wifi_plat_dev_drv_remove(struct platform_device *pdev)
{
	wifi_adapter_info_t *adapter;

	/* Android style wifi platform data device ("bcmdhd_wlan" or "bcm4329_wlan")
	 * is kept for backward compatibility and supports only 1 adapter
	 */
	ASSERT(dhd_wifi_platdata != NULL);
	ASSERT(dhd_wifi_platdata->num_adapters == 1);
	adapter = &dhd_wifi_platdata->adapters[0];
	if (is_power_on) {
#ifdef BCMPCIE
		wifi_platform_bus_enumerate(adapter, FALSE);
		wifi_platform_set_power(adapter, FALSE, WIFI_TURNOFF_DELAY);
#else
		wifi_platform_set_power(adapter, FALSE, WIFI_TURNOFF_DELAY);
		wifi_platform_bus_enumerate(adapter, FALSE);
#endif /* BCMPCIE */
	}

#ifdef BCMDHD_PLATDEV
	dhd_wlan_deinit_plat_data(adapter);
#endif
	return 0;
}

static int wifi_plat_dev_drv_suspend(struct platform_device *pdev, pm_message_t state)
{
	DHD_TRACE(("##> %s\n", __FUNCTION__));
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 39)) && defined(OOB_INTR_ONLY) && \
	defined(BCMSDIO)
	bcmsdh_oob_intr_set(0);
#endif /* (OOB_INTR_ONLY) */
	return 0;
}

static int wifi_plat_dev_drv_resume(struct platform_device *pdev)
{
	DHD_TRACE(("##> %s\n", __FUNCTION__));
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 39)) && defined(OOB_INTR_ONLY) && \
	defined(BCMSDIO)
	if (dhd_os_check_if_up(wl_cfg80211_get_dhdp()))
		bcmsdh_oob_intr_set(1);
#endif /* (OOB_INTR_ONLY) */
	return 0;
}

static const struct of_device_id wifi_device_dt_match[] = {
	{ .compatible = "android,bcmdhd_wlan", .name = "", },
	{ .name = "" },
};
static struct platform_driver wifi_platform_dev_driver = {
	.probe          = wifi_plat_dev_drv_probe,
	.remove         = wifi_plat_dev_drv_remove,
	.suspend        = wifi_plat_dev_drv_suspend,
	.resume         = wifi_plat_dev_drv_resume,
#ifdef DHD_WIFI_SHUTDOWN
	.shutdown       = wifi_plat_dev_drv_shutdown,
#endif /* DHD_WIFI_SHUTDOWN */
	.driver         = {
	.name   = WIFI_PLAT_NAME,
	.of_match_table = wifi_device_dt_match,
	}
};

static struct platform_driver wifi_platform_dev_driver_legacy = {
	.probe          = wifi_plat_dev_drv_probe,
	.remove         = wifi_plat_dev_drv_remove,
	.suspend        = wifi_plat_dev_drv_suspend,
	.resume         = wifi_plat_dev_drv_resume,
#ifdef DHD_WIFI_SHUTDOWN
	.shutdown       = wifi_plat_dev_drv_shutdown,
#endif /* DHD_WIFI_SHUTDOWN */
	.driver         = {
	.name	= WIFI_PLAT_NAME2,
	}
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
static int wifi_platdev_match(struct device *dev, const void *data)
#else
static int wifi_platdev_match(struct device *dev, void *data)
#endif /* LINUX_VER >= 5.3.0 */
{
	char *name = NULL;
	const struct platform_device *pdev;
	GCC_DIAGNOSTIC_PUSH_SUPPRESS_CAST();
	name = (char*)data;
	pdev = to_platform_device(dev);
	GCC_DIAGNOSTIC_POP();

	if (strcmp(pdev->name, name) == 0) {
		DHD_ERROR(("found wifi platform device %s\n", name));
		return TRUE;
	}

	return FALSE;
}
#endif

static int wifi_ctrlfunc_register_drv(void)
{
	wifi_adapter_info_t *adapter;

#ifndef CUSTOMER_HW
	int err = 0;
	struct device *dev1, *dev2;
	dev1 = bus_find_device(&platform_bus_type, NULL, WIFI_PLAT_NAME, wifi_platdev_match);
	dev2 = bus_find_device(&platform_bus_type, NULL, WIFI_PLAT_NAME2, wifi_platdev_match);
#endif

// modify for compaibility
#if defined(BCMDHD_MODULAR) && defined(BOARD_MODULAR_INIT)
//	if ((err = dhd_wlan_init())) {
//		DHD_ERROR(("%s: dhd_wlan_init() failed(%d)\n", __FUNCTION__, err));
//		return err;
//	}
#ifdef WBRC
	wbrc_init();
#endif /* WBRC */
#endif /* defined(BCMDHD_MODULAR) && defined(BOARD_MODULAR_INIT) */

#if !defined(CONFIG_DTS) && !defined(CUSTOMER_HW)
	if (!dts_enabled) {
		if (dev1 == NULL && dev2 == NULL) {
			DHD_ERROR(("no wifi platform data, skip\n"));
			return -ENXIO;
		}
	}
#endif /* !defined(CONFIG_DTS) */

	/* multi-chip support not enabled, build one adapter information for
	 * DHD (either SDIO, USB or PCIe)
	 */
	adapter = kzalloc(sizeof(wifi_adapter_info_t), GFP_KERNEL);
	if (adapter == NULL) {
		DHD_ERROR(("%s:adapter alloc failed", __FUNCTION__));
		return -ENOMEM;
	}
	adapter->name = "DHD generic adapter";
	adapter->index = 0;
	adapter->bus_type = -1;
	adapter->bus_num = -1;
	adapter->slot_num = -1;
	adapter->irq_num = -1;
	is_power_on = FALSE;
	wifi_plat_dev_probe_ret = 0;
	dhd_wifi_platdata = kzalloc(sizeof(bcmdhd_wifi_platdata_t), GFP_KERNEL);
	if (dhd_wifi_platdata == NULL) {
		DHD_ERROR(("%s:dhd_wifi_platdata alloc failed", __FUNCTION__));
		kfree(adapter);
		return -ENOMEM;
	}
	dhd_wifi_platdata->num_adapters = 1;
	dhd_wifi_platdata->adapters = adapter;
	init_waitqueue_head(&adapter->status_event);

#ifndef CUSTOMER_HW
	if (dev1) {
		err = platform_driver_register(&wifi_platform_dev_driver);
		if (err) {
			DHD_ERROR(("%s: failed to register wifi ctrl func driver\n",
				__FUNCTION__));
			return err;
		}
	}
	if (dev2) {
		err = platform_driver_register(&wifi_platform_dev_driver_legacy);
		if (err) {
			DHD_ERROR(("%s: failed to register wifi ctrl func legacy driver\n",
				__FUNCTION__));
			return err;
		}
	}
#endif

#if !defined(CONFIG_DTS)
	if (dts_enabled) {
		adapter->wifi_plat_data = (void *)&dhd_wlan_control;
#ifdef CUSTOMER_HW
		wifi_plat_dev_probe_ret = dhd_wlan_init_plat_data(adapter);
		if (wifi_plat_dev_probe_ret)
			return wifi_plat_dev_probe_ret;
#endif
#ifdef DHD_ISR_NO_SUSPEND
		adapter->intr_flags |= IRQF_NO_SUSPEND;
#endif
		wifi_plat_dev_probe_ret = dhd_wifi_platform_load();
	}
#endif /* !defined(CONFIG_DTS) */

#if defined(CONFIG_DTS) && !defined(CUSTOMER_HW)
	wifi_plat_dev_probe_ret = platform_driver_register(&wifi_platform_dev_driver);
#endif /* CONFIG_DTS */

	/* return probe function's return value if registeration succeeded */
	return wifi_plat_dev_probe_ret;
}

void wifi_ctrlfunc_unregister_drv(void)
{
	wifi_adapter_info_t *adapter = NULL;

#ifndef CUSTOMER_HW
	struct device *dev1, *dev2;
	dev1 = bus_find_device(&platform_bus_type, NULL, WIFI_PLAT_NAME, wifi_platdev_match);
	dev2 = bus_find_device(&platform_bus_type, NULL, WIFI_PLAT_NAME2, wifi_platdev_match);
	if (!dts_enabled)
		if (dev1 == NULL && dev2 == NULL)
			return;
#endif

	DHD_ERROR(("unregister wifi platform drivers\n"));

	if (!dhd_wifi_platdata) {
		goto done;
	}

	if (dts_enabled) {
		adapter = &dhd_wifi_platdata->adapters[0];
		if (is_power_on) {
			wifi_platform_set_power(adapter, FALSE, WIFI_TURNOFF_DELAY);
		}
		wifi_platform_bus_enumerate(adapter, FALSE);
	} else {
#ifndef CUSTOMER_HW
		if (dev1)
			platform_driver_unregister(&wifi_platform_dev_driver);
		if (dev2)
			platform_driver_unregister(&wifi_platform_dev_driver_legacy);
#endif
	}

// modify for compaibility
#if defined(BCMDHD_MODULAR) && defined(BOARD_MODULAR_INIT)
//	dhd_wlan_deinit();
	osl_static_mem_deinit(NULL, NULL);
#ifdef WBRC
	wbrc_exit();
#endif /* WBRC */
#endif /* defined(BCMDHD_MODULAR) && defined(BOARD_MODULAR_INIT) */

#if defined(CUSTOMER_HW)
	if (adapter)
		dhd_wlan_deinit_plat_data(adapter);
#endif

done:
	if (dhd_wifi_platdata && dhd_wifi_platdata->adapters) {
		kfree(dhd_wifi_platdata->adapters);
		dhd_wifi_platdata->adapters = NULL;
		dhd_wifi_platdata->num_adapters = 0;
	}
	if (dhd_wifi_platdata) {
		kfree(dhd_wifi_platdata);
		dhd_wifi_platdata = NULL;
	}
}

#ifndef CUSTOMER_HW
static int bcmdhd_wifi_plat_dev_drv_probe(struct platform_device *pdev)
{
	dhd_wifi_platdata = (bcmdhd_wifi_platdata_t *)(pdev->dev.platform_data);

	return dhd_wifi_platform_load();
}

static int bcmdhd_wifi_plat_dev_drv_remove(struct platform_device *pdev)
{
	int i;
	wifi_adapter_info_t *adapter;
	ASSERT(dhd_wifi_platdata != NULL);

	/* power down all adapters */
	for (i = 0; i < dhd_wifi_platdata->num_adapters; i++) {
		adapter = &dhd_wifi_platdata->adapters[i];
		wifi_platform_set_power(adapter, FALSE, WIFI_TURNOFF_DELAY);
		wifi_platform_bus_enumerate(adapter, FALSE);
	}
	return 0;
}

static struct platform_driver dhd_wifi_platform_dev_driver = {
	.probe          = bcmdhd_wifi_plat_dev_drv_probe,
	.remove         = bcmdhd_wifi_plat_dev_drv_remove,
	.driver         = {
	.name   = WIFI_PLAT_EXT,
	}
};
#endif

int dhd_wifi_platform_register_drv(void)
{
	int err = 0;
#ifndef CUSTOMER_HW
	struct device *dev;

	/* register Broadcom wifi platform data driver if multi-chip is enabled,
	 * otherwise use Android style wifi platform data (aka wifi control function)
	 * if it exists
	 *
	 * to support multi-chip DHD, Broadcom wifi platform data device must
	 * be added in kernel early boot (e.g. board config file).
	 */
	if (cfg_multichip) {
		dev = bus_find_device(&platform_bus_type, NULL, WIFI_PLAT_EXT, wifi_platdev_match);
		if (dev == NULL) {
			DHD_ERROR(("bcmdhd wifi platform data device not found!!\n"));
			return -ENXIO;
		}
		err = platform_driver_register(&dhd_wifi_platform_dev_driver);
	} else
#endif
	{
		err = wifi_ctrlfunc_register_drv();

		/* no wifi ctrl func either, load bus directly and ignore this error */
		if (err) {
			if (err == -ENXIO) {
				/* wifi ctrl function does not exist */
				err = dhd_wifi_platform_load();
			} else {
				/* unregister driver due to initialization failure */
				wifi_ctrlfunc_unregister_drv();
			}
		}
	}

	return err;
}

#ifdef BCMPCIE
static int dhd_wifi_platform_load_pcie(void)
{
	int i;
	int err;
	int retry;
	wifi_adapter_info_t *adapter;

	if (dhd_wifi_platdata) {
		/* enumerate PCIe RC */
		for (i = 0; i < dhd_wifi_platdata->num_adapters; i++) {
			adapter = &dhd_wifi_platdata->adapters[i];
			err = wifi_platform_bus_enumerate(adapter, TRUE);
			if (err) {
				DHD_ERROR(("failed to enumerate bus %s err=%d",
					adapter->name, err));
				return err;
			}
		}
#ifdef DHD_SUPPORT_HDM
		if (dhd_download_fw_on_driverload || hdm_trigger_init)
#else
		if (dhd_download_fw_on_driverload)
#endif /* DHD_SUPPORT_HDM */
		{
			/* power up all adapters */
			for (i = 0; i < dhd_wifi_platdata->num_adapters; i++) {
				retry = POWERUP_MAX_RETRY;
				adapter = &dhd_wifi_platdata->adapters[i];

				DHD_ERROR(("Power-up adapter '%s'\n", adapter->name));
				DHD_INFO((" - irq %d [flags %d], firmware: %s, nvram: %s\n",
					adapter->irq_num, adapter->intr_flags, adapter->fw_path,
					adapter->nv_path));
				DHD_INFO((" - bus type %d, bus num %d, slot num %d\n\n",
					adapter->bus_type, adapter->bus_num, adapter->slot_num));

				do {
					err = wifi_platform_set_power(adapter,
						TRUE, WIFI_TURNON_DELAY);
					if (err) {
						DHD_ERROR(("failed to power up %s,"
							" %d retry left\n",
							adapter->name, retry));
						/* WL_REG_ON state unknown, Power off forcely */
						wifi_platform_set_power(adapter,
							FALSE, WIFI_TURNOFF_DELAY);
						continue;
					}

					err = wifi_platform_bus_enumerate(adapter, TRUE);
					if (err) {
						DHD_ERROR(("failed to enumerate bus %s, "
							"%d retry left\n",
							adapter->name, retry));
						wifi_platform_set_power(adapter, FALSE,
							WIFI_TURNOFF_DELAY);
					} else {
						break;
					}
				} while (retry--);

				if (retry < 0) {
					DHD_ERROR(("failed to power up %s, max retry reached**\n",
						adapter->name));
					return -ENODEV;
				}
			}
		}
	}

	err = dhd_bus_register();
	if (err) {
		DHD_ERROR(("%s: dhd_bus_register failed err=%d\n", __FUNCTION__, err));
		if (dhd_wifi_platdata && dhd_download_fw_on_driverload) {
			/* power down all adapters */
			for (i = 0; i < dhd_wifi_platdata->num_adapters; i++) {
				adapter = &dhd_wifi_platdata->adapters[i];
				wifi_platform_bus_enumerate(adapter, FALSE);
				wifi_platform_set_power(adapter,
					FALSE, WIFI_TURNOFF_DELAY);
			}
		}
	}

	return err;
}
#else
static int dhd_wifi_platform_load_pcie(void)
{
	return 0;
}
#endif /* BCMPCIE  */

void dhd_wifi_platform_unregister_drv(void)
{
#ifndef CUSTOMER_HW
	if (cfg_multichip)
		platform_driver_unregister(&dhd_wifi_platform_dev_driver);
	else
#endif
		wifi_ctrlfunc_unregister_drv();
}

extern int dhd_watchdog_prio;
extern int dhd_dpc_prio;
extern uint dhd_deferred_tx;
#if defined(OEM_ANDROID) && (defined(BCMLXSDMMC) || defined(BCMDBUS))
extern struct semaphore dhd_registration_sem;
#endif /* defined(OEM_ANDROID) && defined(BCMLXSDMMC) */

#ifdef BCMSDIO
static int dhd_wifi_platform_load_sdio(void)
{
	int i;
	int err = 0;
	wifi_adapter_info_t *adapter;

	BCM_REFERENCE(i);
	BCM_REFERENCE(adapter);
	/* Sanity check on the module parameters
	 * - Both watchdog and DPC as tasklets are ok
	 * - If both watchdog and DPC are threads, TX must be deferred
	 */
	if (!(dhd_watchdog_prio < 0 && dhd_dpc_prio < 0) &&
		!(dhd_watchdog_prio >= 0 && dhd_dpc_prio >= 0 && dhd_deferred_tx))
		return -EINVAL;

#if defined(OEM_ANDROID) && defined(BCMLXSDMMC) && !defined(DHD_PRELOAD)
	sema_init(&dhd_registration_sem, 0);
#endif

	if (dhd_wifi_platdata == NULL) {
		DHD_ERROR(("DHD wifi platform data is required for Android build\n"));
		DHD_ERROR(("DHD registering bus directly\n"));
		/* x86 bring-up PC needs no power-up operations */
		err = dhd_bus_register();
		return err;
	}

#if defined(OEM_ANDROID) && defined(BCMLXSDMMC) && !defined(DHD_PRELOAD)
	/* power up all adapters */
	for (i = 0; i < dhd_wifi_platdata->num_adapters; i++) {
		bool chip_up = FALSE;
		int retry = POWERUP_MAX_RETRY;
#ifndef DHD_INSMOD_NOWAIT
		struct semaphore dhd_chipup_sem;
#endif

		adapter = &dhd_wifi_platdata->adapters[i];

		DHD_ERROR(("Power-up adapter '%s'\n", adapter->name));
		DHD_INFO((" - irq %d [flags %d], firmware: %s, nvram: %s\n",
			adapter->irq_num, adapter->intr_flags, adapter->fw_path, adapter->nv_path));
		DHD_INFO((" - bus type %d, bus num %d, slot num %d\n\n",
			adapter->bus_type, adapter->bus_num, adapter->slot_num));

		do {
#ifdef DHD_INSMOD_NOWAIT
			err = wifi_platform_set_power(adapter, TRUE, WIFI_TURNON_DELAY);
			if (err) {
				DHD_ERROR(("%s: wifi pwr on error ! \n", __FUNCTION__));
				wifi_platform_set_power(adapter, FALSE, WIFI_TURNOFF_DELAY);
				continue;
			} else {
				wifi_platform_bus_enumerate(adapter, TRUE);
				chip_up = TRUE;
				break;
			}
#else
			sema_init(&dhd_chipup_sem, 0);
			err = dhd_bus_reg_sdio_notify(&dhd_chipup_sem);
			if (err) {
				DHD_ERROR(("%s dhd_bus_reg_sdio_notify fail(%d)\n\n",
					__FUNCTION__, err));
				return err;
			}
			err = wifi_platform_set_power(adapter, TRUE, WIFI_TURNON_DELAY);
			if (err) {
				DHD_ERROR(("%s: wifi pwr on error ! \n", __FUNCTION__));
				dhd_bus_unreg_sdio_notify();
				/* WL_REG_ON state unknown, Power off forcely */
				wifi_platform_set_power(adapter, FALSE, WIFI_TURNOFF_DELAY);
				continue;
			} else {
				wifi_platform_bus_enumerate(adapter, TRUE);
			}

			if (down_timeout(&dhd_chipup_sem, msecs_to_jiffies(POWERUP_WAIT_MS)) == 0) {
				dhd_bus_unreg_sdio_notify();
				chip_up = TRUE;
				break;
			}

			DHD_ERROR(("failed to power up %s, %d retry left\n", adapter->name, retry));
			dhd_bus_unreg_sdio_notify();
			wifi_platform_set_power(adapter, FALSE, WIFI_TURNOFF_DELAY);
			wifi_platform_bus_enumerate(adapter, FALSE);
#endif
		} while (retry--);

		if (!chip_up) {
			DHD_ERROR(("failed to power up %s, max retry reached**\n", adapter->name));
			return -ENODEV;
		}

	}

	err = dhd_bus_register();

	if (err) {
		DHD_ERROR(("%s: sdio_register_driver failed\n", __FUNCTION__));
		goto fail;
	}

#ifndef DHD_INSMOD_NOWAIT
	/*
	 * Wait till MMC sdio_register_driver callback called and made driver attach.
	 * It's needed to make sync up exit from dhd insmod  and
	 * Kernel MMC sdio device callback registration
	 */
	err = down_timeout(&dhd_registration_sem, msecs_to_jiffies(DHD_REGISTRATION_TIMEOUT));
	if (err) {
		DHD_ERROR(("%s: sdio_register_driver timeout or error \n", __FUNCTION__));
		dhd_bus_unregister();
		goto fail;
	}
#endif

	return err;

fail:
	/* power down all adapters */
	for (i = 0; i < dhd_wifi_platdata->num_adapters; i++) {
		adapter = &dhd_wifi_platdata->adapters[i];
		wifi_platform_set_power(adapter, FALSE, WIFI_TURNOFF_DELAY);
		wifi_platform_bus_enumerate(adapter, FALSE);
	}
#else
	/* x86 bring-up PC needs no power-up operations */
	err = dhd_bus_register();
#endif /* defined(OEM_ANDROID) && defined(BCMLXSDMMC) */

	return err;
}
#else /* BCMSDIO */
static int dhd_wifi_platform_load_sdio(void)
{
	return 0;
}
#endif /* BCMSDIO */

#ifdef BCMDBUS
static int dhd_wifi_platform_load_usb(void)
{
	int err = 0;
#if !defined(DHD_PRELOAD)
	wifi_adapter_info_t *adapter;
	s32 timeout = -1;
	int i;
#endif

#if !defined(DHD_PRELOAD) && !defined(ENABLE_INSMOD_NO_POWER_OFF)
	/* power down all adapters */
	for (i = 0; i < dhd_wifi_platdata->num_adapters; i++) {
		adapter = &dhd_wifi_platdata->adapters[i];
		wifi_platform_set_power(adapter, FALSE, 0);
		if (err) {
			DHD_ERROR(("failed to wifi_platform_set_power off %s\n", adapter->name));
			goto exit;
		}
	}
	OSL_SLEEP(200);
#endif

	err = dhd_bus_register();
	if (err) {
		DHD_ERROR(("%s: usb_register failed\n", __FUNCTION__));
		goto exit;
	}

#if !defined(DHD_PRELOAD)
	/* power up all adapters */
	for (i = 0; i < dhd_wifi_platdata->num_adapters; i++) {
		adapter = &dhd_wifi_platdata->adapters[i];
		DHD_ERROR(("Power-up adapter '%s'\n", adapter->name));
		DHD_INFO((" - irq %d [flags %d], firmware: %s, nvram: %s\n",
			adapter->irq_num, adapter->intr_flags, adapter->fw_path, adapter->nv_path));
		DHD_INFO((" - bus type %d, bus num %d, slot num %d\n\n",
			adapter->bus_type, adapter->bus_num, adapter->slot_num));
		err = wifi_platform_set_power(adapter, TRUE, WIFI_TURNON_DELAY);
		if (err) {
			DHD_ERROR(("failed to wifi_platform_set_power on %s\n", adapter->name));
			goto fail;
		}
		timeout = wait_event_interruptible_timeout(adapter->status_event,
			wifi_get_adapter_status(adapter, WIFI_STATUS_NET_ATTACHED),
			msecs_to_jiffies(DHD_REGISTRATION_TIMEOUT));
		if (timeout <= 0) {
			err = -1;
			DHD_ERROR(("%s: usb_register_driver timeout\n", __FUNCTION__));
			goto fail;
		}
	}
#endif

exit:
	return err;

#if !defined(DHD_PRELOAD)
fail:
	dhd_bus_unregister();
	/* power down all adapters */
	for (i = 0; i < dhd_wifi_platdata->num_adapters; i++) {
		adapter = &dhd_wifi_platdata->adapters[i];
		wifi_platform_set_power(adapter, FALSE, WIFI_TURNOFF_DELAY);
	}

	return err;
#endif
}
#else /* BCMDBUS */
static int dhd_wifi_platform_load_usb(void)
{
	return 0;
}
#endif /* BCMDBUS */

static int dhd_wifi_platform_load(void)
{
	int err = 0;
	printf("%s: Enter\n", __FUNCTION__);

#if defined(OEM_ANDROID)
	wl_android_init();
#endif /* OEM_ANDROID */

	if ((err = dhd_wifi_platform_load_usb())) {
		goto end;
	} else if ((err = dhd_wifi_platform_load_sdio())) {
		goto end;
	} else {
		err = dhd_wifi_platform_load_pcie();
	}

end:
#if defined(OEM_ANDROID)
	if (err)
		wl_android_exit();
#if !defined(MULTIPLE_SUPPLICANT)
	else
		wl_android_post_init();
#endif
#endif /* OEM_ANDROID */

	return err;
}

#if defined(SUPPORT_MULTIPLE_BOARD_REVISION)
void
concate_custom_board_revision(char *nv_path)
{
	uint32 board_revision = 0;
	struct device_node *root_node = NULL;
	char* wlan_node = NULL;

	if (!nv_path) {
		DHD_ERROR(("nv_path is null\n"));
		return;
	}

	wlan_node = dhd_get_device_dt_name();
	if (!wlan_node) {
		DHD_ERROR(("Failed to dt name\n"));
		return;
	}

	root_node = of_find_compatible_node(NULL, NULL, wlan_node);
	if (!root_node) {
		DHD_ERROR(("Failed to get device node\n"));
		return;
	}

	if (of_property_read_u32(root_node, "nvram-ES", &board_revision)) {
		DHD_ERROR(("No board revision property in dtsi\n"));
		return;
	}

	DHD_INFO(("Board revision:%d\n", board_revision));

	if (board_revision == 1) {
		strcat(nv_path, "_ES10");
		DHD_INFO(("Mached Board revision ES10: nvram name:%s\n", nv_path));
	}

}
#endif /* SUPPORT_MULTIPLE_BOARD_REVISION */

/* Weak functions that can be overridden in Platform specific implementation */
char* __attribute__ ((weak)) dhd_get_device_dt_name(void)
{
	return NULL;
}

uint32 __attribute__ ((weak)) dhd_plat_get_info_size(void)
{
	return 0;
}

int __attribute__ ((weak)) dhd_plat_pcie_register_event(void *plat_info,
		struct pci_dev *pdev, dhd_pcie_event_cb_t pfn)
{
	return 0;
}

void __attribute__ ((weak)) dhd_plat_pcie_deregister_event(void *plat_info)
{
	return;
}

void __attribute__ ((weak)) dhd_plat_l1ss_ctrl(bool ctrl)
{
	return;
}

void __attribute__ ((weak)) dhd_plat_l1_exit_io(void)
{
	return;
}

void __attribute__ ((weak)) dhd_plat_l1_exit(void)
{
	return;
}

void __attribute__ ((weak)) dhd_plat_report_bh_sched(void *plat_info, int resched)
{
	return;
}

int __attribute__ ((weak)) dhd_plat_pcie_suspend(void *plat_info)
{
	return 0;
}

int __attribute__ ((weak)) dhd_plat_pcie_resume(void *plat_info)
{
	return 0;
}

void __attribute__ ((weak)) dhd_plat_pcie_register_dump(void *plat_info)
{
	return;
}

void __attribute__ ((weak)) dhd_plat_pin_dbg_show(void *plat_info)
{
	return;
}

uint32 __attribute__ ((weak)) dhd_plat_get_rc_vendor_id(void)
{
	return PCIE_RC_VENDOR_ID;
}

uint32 __attribute__ ((weak)) dhd_plat_get_rc_device_id(void)
{
	return PCIE_RC_DEVICE_ID;
}

uint16 __attribute__ ((weak)) dhd_plat_align_rxbuf_size(uint16 rxbufpost_sz)
{
	return rxbufpost_sz;
}

int
__attribute__ ((weak)) dhd_get_platform_naming_for_nvram_clmblob_file(download_type_t component,
	char *file_name)
{
	return BCME_ERROR;
}
