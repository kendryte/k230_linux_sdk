/*
 * Dongle BUS interface for USB, OS independent
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
 * <<Broadcom-WL-IPTag/Dual:>>
 */

/**
 * @file @brief
 * This file contains DBUS code that is USB, but not OS specific. DBUS is a Broadcom proprietary
 * host specific abstraction layer.
 */

#include <osl.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <dbus.h>
#include <usbrdl.h>
#include <bcmdevs_legacy.h>
#include <bcmdevs.h>
#include <bcmendian.h>

#if defined(BCM_DNGL_EMBEDIMAGE)
#ifdef EMBED_IMAGE_43526a
#include "rtecdc_43526a.h"
#endif /* EMBED_IMAGE_43526a */
#ifdef EMBED_IMAGE_43526b
#include "rtecdc_43526b.h"
#endif /* EMBED_IMAGE_43526b */
#ifdef EMBED_IMAGE_GENERIC
#include "rtecdc.h"
#endif
#endif /* BCM_DNGL_EMBEDIMAGE */

uint dbus_msglevel = DBUS_ERROR_VAL;
module_param(dbus_msglevel, int, 0);

#define USB_DLIMAGE_RETRY_TIMEOUT    3000    /* retry Timeout */
#define USB_SFLASH_DLIMAGE_SPINWAIT  150     /* in unit of ms */
#define USB_SFLASH_DLIMAGE_LIMIT     2000    /* spinwait limit (ms) */
#define POSTBOOT_ID                  0xA123  /* ID to detect if dongle has boot up */
#define USB_RESETCFG_SPINWAIT        1       /* wait after resetcfg (ms) */
#define USB_DEV_ISBAD(u)             (u->pub->attrib.devid == 0xDEAD)
#define USB_DLGO_SPINWAIT            100     /* wait after DL_GO (ms) */
#define TEST_CHIP                    0x4328

/* driver info, initialized when bcmsdh_register is called */
static dbus_driver_t drvinfo = {NULL, NULL, NULL, NULL};

typedef struct {
	dbus_pub_t  *pub;

	void        *cbarg;
	dbus_intf_callbacks_t *cbs;  /** callbacks into higher DBUS level (dbus.c) */
	dbus_intf_t *drvintf;
	void        *usbosl_info;
	uint32      rdlram_base_addr;
	uint32      rdlram_size;
} usb_info_t;

/*
 * Callbacks common to all USB
 */
static void dbus_usb_disconnect(void *handle);
static void dbus_usb_send_irb_timeout(void *handle, dbus_irb_tx_t *txirb);
static void dbus_usb_send_irb_complete(void *handle, dbus_irb_tx_t *txirb, int status);
static void dbus_usb_recv_irb_complete(void *handle, dbus_irb_rx_t *rxirb, int status);
static void dbus_usb_errhandler(void *handle, int err);
static void dbus_usb_ctl_complete(void *handle, int type, int status);
static void dbus_usb_state_change(void *handle, int state);
static struct dbus_irb* dbus_usb_getirb(void *handle, bool send);
static void dbus_usb_rxerr_indicate(void *handle, bool on);
#if !defined(BCM_REQUEST_FW)
static int dbus_usb_resetcfg(usb_info_t *usbinfo);
#endif
static int dbus_usb_iovar_op(void *bus, const char *name,
	void *params, uint plen, void *arg, uint len, bool set);
static int dbus_iovar_process(usb_info_t* usbinfo, const char *name,
                 void *params, int plen, void *arg, uint len, bool set);
static int dbus_usb_doiovar(usb_info_t *bus, const bcm_iovar_t *vi, uint32 actionid,
	const char *name, void *params, int plen, void *arg, uint len, int val_size);
static int dhdusb_downloadvars(usb_info_t *bus, void *arg, int len);

static int dbus_usb_dl_writeimage(usb_info_t *usbinfo, uint8 *fw, int fwlen);
static int dbus_usb_dlstart(void *bus, uint8 *fw, int len);
static int dbus_usb_dlneeded(void *bus);
static int dbus_usb_dlrun(void *bus);
static int dbus_usb_rdl_dwnld_state(usb_info_t *usbinfo);

#ifdef BCM_DNGL_EMBEDIMAGE
static bool dbus_usb_device_exists(void *bus);
#endif

/* OS specific */
extern bool dbus_usbos_dl_cmd(void *info, uint8 cmd, void *buffer, int buflen);
extern int dbus_usbos_wait(void *info, uint16 ms);
extern int dbus_write_membytes(usb_info_t *usbinfo, bool set, uint32 address,
	uint8 *data, uint size);
extern bool dbus_usbos_dl_send_bulk(void *info, void *buffer, int len);
extern int dbus_usbos_loopback_tx(void *usbos_info_ptr, int cnt, int size);

/**
 * These functions are called by the lower DBUS level (dbus_usb_os.c) to notify this DBUS level
 * (dbus_usb.c) of an event.
 */
static dbus_intf_callbacks_t dbus_usb_intf_cbs = {
	dbus_usb_send_irb_timeout,
	dbus_usb_send_irb_complete,
	dbus_usb_recv_irb_complete,
	dbus_usb_errhandler,
	dbus_usb_ctl_complete,
	dbus_usb_state_change,
	NULL,			/* isr */
	NULL,			/* dpc */
	NULL,			/* watchdog */
	NULL,			/* dbus_if_pktget */
	NULL, 			/* dbus_if_pktfree */
	dbus_usb_getirb,
	dbus_usb_rxerr_indicate
};

/* IOVar table */
enum {
	IOV_SET_DOWNLOAD_STATE = 1,
	IOV_DBUS_MSGLEVEL,
	IOV_MEMBYTES,
	IOV_VARS,
	IOV_LOOPBACK_TX
};

const bcm_iovar_t dhdusb_iovars[] = {
	{"vars",	IOV_VARS,	0,	0, IOVT_BUFFER,	0 },
	{"dbus_msglevel",	IOV_DBUS_MSGLEVEL,	0,	0, IOVT_UINT32,	0 },
	{"dwnldstate",	IOV_SET_DOWNLOAD_STATE,	0,	0, IOVT_BOOL,	0 },
	{"membytes",	IOV_MEMBYTES,	0,	0, IOVT_BUFFER,	2 * sizeof(int) },
	{"usb_lb_txfer", IOV_LOOPBACK_TX, 0,    0, IOVT_BUFFER,    2 * sizeof(int) },
	{NULL, 0, 0, 0, 0, 0 }
};

/*
 * Need global for probe() and disconnect() since
 * attach() is not called at probe and detach()
 * can be called inside disconnect()
 */
static dbus_intf_t	*g_dbusintf = NULL;
static dbus_intf_t	dbus_usb_intf; /** functions called by higher layer DBUS into lower layer */

/*
 * dbus_intf_t common to all USB
 * These functions override dbus_usb_<os>.c.
 */
static void *dbus_usb_attach(dbus_pub_t *pub, void *cbarg, dbus_intf_callbacks_t *cbs);
static void dbus_usb_detach(dbus_pub_t *pub, void *info);
/* : g_usb_info needed for over-ridden functions
 * since the bus argument is actually from dbus_usb_<os>.c.
 */
static void * dbus_usb_probe(uint16 bus_no, uint16 slot, uint32 hdrlen);

/* functions */

/**
 * As part of DBUS initialization/registration, the higher level DBUS (dbus.c) needs to know what
 * lower level DBUS functions to call (in both dbus_usb.c and dbus_usb_os.c).
 */
static void *
dbus_usb_probe(uint16 bus_no, uint16 slot, uint32 hdrlen)
{
	DBUSTRACE(("%s(): \n", __FUNCTION__));
	if (drvinfo.probe) {
		if (g_dbusintf != NULL) {
			/* First, initialize all lower-level functions as default
			 * so that dbus.c simply calls directly to dbus_usb_os.c.
			 */
			bcopy(g_dbusintf, &dbus_usb_intf, sizeof(dbus_intf_t));

			/* Second, selectively override functions we need, if any. */
			dbus_usb_intf.attach = dbus_usb_attach;
			dbus_usb_intf.detach = dbus_usb_detach;
			dbus_usb_intf.iovar_op = dbus_usb_iovar_op;
			dbus_usb_intf.dlstart = dbus_usb_dlstart;
			dbus_usb_intf.dlneeded = dbus_usb_dlneeded;
			dbus_usb_intf.dlrun = dbus_usb_dlrun;
#ifdef BCM_DNGL_EMBEDIMAGE
			dbus_usb_intf.device_exists = dbus_usb_device_exists;
#endif
		}

		return drvinfo.probe(bus_no, slot, hdrlen);
	}

	return NULL;
}

static int
dbus_usb_suspend(void *handle)
{
	DBUSTRACE(("%s(): \n", __FUNCTION__));

	if (drvinfo.suspend)
		return drvinfo.suspend(handle);

	return BCME_OK;
}

static int
dbus_usb_resume(void *handle)
{
	DBUSTRACE(("%s(): \n", __FUNCTION__));

	if (drvinfo.resume)
		drvinfo.resume(handle);

	return 0;
}

static dbus_driver_t dhd_usb_dbus = {
	dbus_usb_probe,
	dbus_usb_disconnect,
	dbus_usb_suspend,
	dbus_usb_resume
};

/**
 * On return, *intf contains this or lower-level DBUS functions to be called by higher
 * level (dbus.c)
 */
int
dbus_bus_register(dbus_driver_t *driver, dbus_intf_t **intf)
{
	int err;

	DBUSTRACE(("%s(): \n", __FUNCTION__));
	drvinfo = *driver;
	*intf = &dbus_usb_intf;

	err = dbus_bus_osl_register(&dhd_usb_dbus, &g_dbusintf);

	ASSERT(g_dbusintf);
	return err;
}

int
dbus_bus_deregister()
{
	DBUSTRACE(("%s(): \n", __FUNCTION__));
	return dbus_bus_osl_deregister();
}

/** initialization consists of registration followed by 'attach'. */
void *
dbus_usb_attach(dbus_pub_t *pub, void *cbarg, dbus_intf_callbacks_t *cbs)
{
	usb_info_t *usb_info;

	DBUSTRACE(("%s(): \n", __FUNCTION__));

	if ((g_dbusintf == NULL) || (g_dbusintf->attach == NULL))
		return NULL;

	/* Sanity check for BUS_INFO() */
	ASSERT(OFFSETOF(usb_info_t, pub) == 0);

	usb_info = MALLOC(pub->osh, sizeof(usb_info_t));
	if (usb_info == NULL)
		return NULL;

	bzero(usb_info, sizeof(usb_info_t));

	usb_info->pub = pub;
	usb_info->cbarg = cbarg;
	usb_info->cbs = cbs;

	usb_info->usbosl_info = (dbus_pub_t *)g_dbusintf->attach(pub,
		usb_info, &dbus_usb_intf_cbs);
	if (usb_info->usbosl_info == NULL) {
		MFREE(pub->osh, usb_info, sizeof(usb_info_t));
		return NULL;
	}

	/* Save USB OS-specific driver entry points */
	usb_info->drvintf = g_dbusintf;

	pub->bus = usb_info;
#if !defined(BCM_REQUEST_FW)
	if (!dbus_usb_resetcfg(usb_info)) {
		usb_info->pub->busstate = DBUS_STATE_DL_DONE;
	}
#endif
	/* Return Lower layer info */
	return (void *) usb_info->usbosl_info;
}

void
dbus_usb_detach(dbus_pub_t *pub, void *info)
{
	usb_info_t *usb_info = (usb_info_t *) pub->bus;
	osl_t *osh = pub->osh;

	if (usb_info == NULL)
		return;

	if (usb_info->drvintf && usb_info->drvintf->detach)
		usb_info->drvintf->detach(pub, usb_info->usbosl_info);

	MFREE(osh, usb_info, sizeof(usb_info_t));
}

void
dbus_usb_disconnect(void *handle)
{
	DBUSTRACE(("%s(): \n", __FUNCTION__));
	if (drvinfo.remove)
		drvinfo.remove(handle);
}

/**
 * When the lower DBUS level (dbus_usb_os.c) signals this event, the higher DBUS level has to be
 * notified.
 */
static void
dbus_usb_send_irb_timeout(void *handle, dbus_irb_tx_t *txirb)
{
	usb_info_t *usb_info = (usb_info_t *) handle;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usb_info == NULL)
		return;

	if (usb_info->cbs && usb_info->cbs->send_irb_timeout)
		usb_info->cbs->send_irb_timeout(usb_info->cbarg, txirb);
}

/**
 * When the lower DBUS level (dbus_usb_os.c) signals this event, the higher DBUS level has to be
 * notified.
 */
static void
dbus_usb_send_irb_complete(void *handle, dbus_irb_tx_t *txirb, int status)
{
	usb_info_t *usb_info = (usb_info_t *) handle;

	if (usb_info == NULL)
		return;

	if (usb_info->cbs && usb_info->cbs->send_irb_complete)
		usb_info->cbs->send_irb_complete(usb_info->cbarg, txirb, status);
}

/**
 * When the lower DBUS level (dbus_usb_os.c) signals this event, the higher DBUS level has to be
 * notified.
 */
static void
dbus_usb_recv_irb_complete(void *handle, dbus_irb_rx_t *rxirb, int status)
{
	usb_info_t *usb_info = (usb_info_t *) handle;

	if (usb_info == NULL)
		return;

	if (usb_info->cbs && usb_info->cbs->recv_irb_complete)
		usb_info->cbs->recv_irb_complete(usb_info->cbarg, rxirb, status);
}

/** Lower DBUS level (dbus_usb_os.c) requests a free IRB. Pass this on to the higher DBUS level. */
static struct dbus_irb*
dbus_usb_getirb(void *handle, bool send)
{
	usb_info_t *usb_info = (usb_info_t *) handle;

	if (usb_info == NULL)
		return NULL;

	if (usb_info->cbs && usb_info->cbs->getirb)
		return usb_info->cbs->getirb(usb_info->cbarg, send);

	return NULL;
}

/**
 * When the lower DBUS level (dbus_usb_os.c) signals this event, the higher DBUS level has to be
 * notified.
 */
static void
dbus_usb_rxerr_indicate(void *handle, bool on)
{
	usb_info_t *usb_info = (usb_info_t *) handle;

	if (usb_info == NULL)
		return;

	if (usb_info->cbs && usb_info->cbs->rxerr_indicate)
		usb_info->cbs->rxerr_indicate(usb_info->cbarg, on);
}

/**
 * When the lower DBUS level (dbus_usb_os.c) signals this event, the higher DBUS level has to be
 * notified.
 */
static void
dbus_usb_errhandler(void *handle, int err)
{
	usb_info_t *usb_info = (usb_info_t *) handle;

	if (usb_info == NULL)
		return;

	if (usb_info->cbs && usb_info->cbs->errhandler)
		usb_info->cbs->errhandler(usb_info->cbarg, err);
}

/**
 * When the lower DBUS level (dbus_usb_os.c) signals this event, the higher DBUS level has to be
 * notified.
 */
static void
dbus_usb_ctl_complete(void *handle, int type, int status)
{
	usb_info_t *usb_info = (usb_info_t *) handle;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usb_info == NULL) {
		DBUSERR(("%s: usb_info is NULL\n", __FUNCTION__));
		return;
	}

	if (usb_info->cbs && usb_info->cbs->ctl_complete)
		usb_info->cbs->ctl_complete(usb_info->cbarg, type, status);
}

/**
 * When the lower DBUS level (dbus_usb_os.c) signals this event, the higher DBUS level has to be
 * notified.
 */
static void
dbus_usb_state_change(void *handle, int state)
{
	usb_info_t *usb_info = (usb_info_t *) handle;

	if (usb_info == NULL)
		return;

	if (usb_info->cbs && usb_info->cbs->state_change)
		usb_info->cbs->state_change(usb_info->cbarg, state);
}

/** called by higher DBUS level (dbus.c) */
static int
dbus_usb_iovar_op(void *bus, const char *name,
	void *params, uint plen, void *arg, uint len, bool set)
{
	int err = DBUS_OK;

	err = dbus_iovar_process((usb_info_t*)bus, name, params, plen, arg, len, set);
	return err;
}

/** process iovar request from higher DBUS level */
static int
dbus_iovar_process(usb_info_t* usbinfo, const char *name,
                 void *params, int plen, void *arg, uint len, bool set)
{
	const bcm_iovar_t *vi = NULL;
	int bcmerror = 0;
	int val_size;
	uint32 actionid;

	DBUSTRACE(("%s: Enter\n", __FUNCTION__));

	ASSERT(name);

	/* Get MUST have return space */
	ASSERT(set || (arg && len));

	/* Set does NOT take qualifiers */
	ASSERT(!set || (!params && !plen));

	/* Look up var locally; if not found pass to host driver */
	if ((vi = bcm_iovar_lookup(dhdusb_iovars, name)) == NULL) {
		/* Not Supported */
		bcmerror = BCME_UNSUPPORTED;
		DBUSTRACE(("%s: IOVAR %s is not supported\n", name, __FUNCTION__));
		goto exit;

	}

	DBUSTRACE(("%s: %s %s, len %d plen %d\n", __FUNCTION__,
	         name, (set ? "set" : "get"), len, plen));

	/* set up 'params' pointer in case this is a set command so that
	 * the convenience int and bool code can be common to set and get
	 */
	if (params == NULL) {
		params = arg;
		plen = len;
	}

	if (vi->type == IOVT_VOID)
		val_size = 0;
	else if (vi->type == IOVT_BUFFER)
		val_size = len;
	else
		/* all other types are integer sized */
		val_size = sizeof(int);

	actionid = set ? IOV_SVAL(vi->varid) : IOV_GVAL(vi->varid);
	bcmerror = dbus_usb_doiovar(usbinfo, vi, actionid,
		name, params, plen, arg, len, val_size);

exit:
	return bcmerror;
} /* dbus_iovar_process */

static int
dbus_usb_doiovar(usb_info_t *bus, const bcm_iovar_t *vi, uint32 actionid, const char *name,
                void *params, int plen, void *arg, uint len, int val_size)
{
	int bcmerror = 0;
	int32 int_val = 0;
	int32 int_val2 = 0;
	bool bool_val = 0;

	DBUSTRACE(("%s: Enter, action %d name %s params %p plen %d arg %p len %d val_size %d\n",
	           __FUNCTION__, actionid, name, params, plen, arg, len, val_size));

	if ((bcmerror = bcm_iovar_lencheck(vi, arg, len, IOV_ISSET(actionid))) != 0)
		goto exit;

	if (plen >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	if (plen >= (int)sizeof(int_val) * 2)
		bcopy((void*)((uintptr)params + sizeof(int_val)), &int_val2, sizeof(int_val2));

	bool_val = (int_val != 0) ? TRUE : FALSE;

	switch (actionid) {

	case IOV_SVAL(IOV_MEMBYTES):
	case IOV_GVAL(IOV_MEMBYTES):
	{
		uint32 address;
		uint size, dsize;
		uint8 *data;

		bool set = (actionid == IOV_SVAL(IOV_MEMBYTES));

		ASSERT(plen >= 2*sizeof(int));

		address = (uint32)int_val;
		BCM_REFERENCE(address);
		bcopy((char *)params + sizeof(int_val), &int_val, sizeof(int_val));
		size = (uint)int_val;

		/* Do some validation */
		dsize = set ? plen - (2 * sizeof(int)) : len;
		if (dsize < size) {
			DBUSTRACE(("%s: error on %s membytes, addr 0x%08x size %d dsize %d\n",
			           __FUNCTION__, (set ? "set" : "get"), address, size, dsize));
			bcmerror = BCME_BADARG;
			break;
		}
		DBUSTRACE(("%s: Request to %s %d bytes at address 0x%08x\n", __FUNCTION__,
		          (set ? "write" : "read"), size, address));

		/* Generate the actual data pointer */
		data = set ? (uint8*)params + 2 * sizeof(int): (uint8*)arg;

		/* Call to do the transfer */
		bcmerror = dbus_usb_dl_writeimage(BUS_INFO(bus, usb_info_t), data, size);
	}
		break;

	case IOV_SVAL(IOV_SET_DOWNLOAD_STATE):

		if (bool_val == TRUE) {
			bcmerror = dbus_usb_dlneeded(bus);
			dbus_usb_rdl_dwnld_state(BUS_INFO(bus, usb_info_t));
		} else {
			usb_info_t *usbinfo = BUS_INFO(bus, usb_info_t);
			bcmerror = dbus_usb_dlrun(bus);
			usbinfo->pub->busstate = DBUS_STATE_DL_DONE;
		}
		break;

	case IOV_SVAL(IOV_VARS):
		bcmerror = dhdusb_downloadvars(BUS_INFO(bus, usb_info_t), arg, len);
		break;

	case IOV_GVAL(IOV_DBUS_MSGLEVEL):
		int_val = (int32)dbus_msglevel;
		bcopy(&int_val, arg, val_size);
		break;

	case IOV_SVAL(IOV_DBUS_MSGLEVEL):
		dbus_msglevel = int_val;
		break;

#ifdef DBUS_USB_LOOPBACK
	case IOV_SVAL(IOV_LOOPBACK_TX):
			bcmerror = dbus_usbos_loopback_tx(BUS_INFO(bus, usb_info_t), int_val,
			  int_val2);
			break;
#endif
	default:
		bcmerror = BCME_UNSUPPORTED;
		break;
	}

exit:
	return bcmerror;
} /* dbus_usb_doiovar */

/** higher DBUS level (dbus.c) wants to set NVRAM variables in dongle */
static int
dhdusb_downloadvars(usb_info_t *bus, void *arg, int len)
{
	int bcmerror = 0;
	uint32 varsize;
	uint32 varaddr;
	uint32 varsizew;

	if (!len) {
		bcmerror = BCME_BUFTOOSHORT;
		goto err;
	}

	/* RAM size is not set. Set it at dbus_usb_dlneeded */
	if (!bus->rdlram_size)
		bcmerror = BCME_ERROR;

	/* Even if there are no vars are to be written, we still need to set the ramsize. */
	varsize = len ? ROUNDUP(len, 4) : 0;
	varaddr = (bus->rdlram_size - 4) - varsize;

	/* Write the vars list */
	DBUSTRACE(("WriteVars: @%x varsize=%d\n", varaddr, varsize));
	bcmerror = dbus_write_membytes(bus->usbosl_info, TRUE, (varaddr + bus->rdlram_base_addr),
		arg, varsize);

	/* adjust to the user specified RAM */
	DBUSTRACE(("Usable memory size: %d\n", bus->rdlram_size));
	DBUSTRACE(("Vars are at %d, orig varsize is %d\n", varaddr, varsize));

	varsize = ((bus->rdlram_size - 4) - varaddr);

	/*
	 * Determine the length token:
	 * Varsize, converted to words, in lower 16-bits, checksum in upper 16-bits.
	 */
	if (bcmerror) {
		varsizew = 0;
	} else {
		varsizew = varsize / 4;
		varsizew = (~varsizew << 16) | (varsizew & 0x0000FFFF);
		varsizew = htol32(varsizew);
	}

	DBUSTRACE(("New varsize is %d, length token=0x%08x\n", varsize, varsizew));

	/* Write the length token to the last word */
	bcmerror = dbus_write_membytes(bus->usbosl_info, TRUE, ((bus->rdlram_size - 4) +
		bus->rdlram_base_addr), (uint8*)&varsizew, 4);
err:
	return bcmerror;
} /* dbus_usb_doiovar */

#if !defined(BCM_REQUEST_FW)
/**
 * After downloading firmware into dongle and starting it, we need to know if the firmware is
 * indeed up and running.
 */
static int
dbus_usb_resetcfg(usb_info_t *usbinfo)
{
	void *osinfo;
	bootrom_id_t id;
	uint16 waittime = 0;

	uint32 starttime = 0;
	uint32 endtime = 0;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbinfo == NULL)
		return DBUS_ERR;

	osinfo = usbinfo->usbosl_info;
	ASSERT(osinfo);

	/* Give dongle chance to boot */
	dbus_usbos_wait(osinfo, USB_SFLASH_DLIMAGE_SPINWAIT);
	waittime = USB_SFLASH_DLIMAGE_SPINWAIT;
	while (waittime < USB_DLIMAGE_RETRY_TIMEOUT) {

		starttime = OSL_SYSUPTIME();

		id.chip = 0xDEAD;       /* Get the ID */
		dbus_usbos_dl_cmd(osinfo, DL_GETVER, &id, sizeof(bootrom_id_t));
		id.chip = ltoh32(id.chip);

		endtime = OSL_SYSUPTIME();
		waittime += (endtime - starttime);

		if (id.chip == POSTBOOT_ID)
			break;
	}

	if (id.chip == POSTBOOT_ID) {
		DBUSERR(("%s: download done. Bootup time = %d ms postboot chip 0x%x/rev 0x%x\n",
			__FUNCTION__, waittime, id.chip, id.chiprev));

		dbus_usbos_dl_cmd(osinfo, DL_RESETCFG, &id, sizeof(bootrom_id_t));

		/* this wait may not be necessary */
		dbus_usbos_wait(osinfo, USB_RESETCFG_SPINWAIT);
		return DBUS_OK;
	} else {
		DBUSERR(("%s: Cannot talk to Dongle. Wait time = %d ms. Firmware is not UP \n",
			__FUNCTION__, waittime));
		return DBUS_ERR;
	}

	return DBUS_OK;
}
#endif

/** before firmware download, the dongle has to be prepared to receive the fw image */
static int
dbus_usb_rdl_dwnld_state(usb_info_t *usbinfo)
{
	void *osinfo = usbinfo->usbosl_info;
	rdl_state_t state;
	int err = DBUS_OK;

	/* 1) Prepare USB boot loader for runtime image */
	dbus_usbos_dl_cmd(osinfo, DL_START, &state, sizeof(rdl_state_t));

	state.state = ltoh32(state.state);
	state.bytes = ltoh32(state.bytes);

	/* 2) Check we are in the Waiting state */
	if (state.state != DL_WAITING) {
		DBUSERR(("%s: Failed to DL_START\n", __FUNCTION__));
		err = DBUS_ERR;
		goto fail;
	}

fail:
	return err;
}

/**
 * Dongle contains bootcode in ROM but firmware is (partially) contained in dongle RAM. Therefore,
 * firmware has to be downloaded into dongle RAM.
 */
static int
dbus_usb_dl_writeimage(usb_info_t *usbinfo, uint8 *fw, int fwlen)
{
	osl_t *osh = usbinfo->pub->osh;
	void *osinfo = usbinfo->usbosl_info;
	unsigned int sendlen, sent, dllen;
	char *bulkchunk = NULL, *dlpos;
	rdl_state_t state;
	int err = DBUS_OK;
	uint32 dl_trunk_size = RDL_CHUNK;

	while (!bulkchunk) {
		bulkchunk = MALLOC(osh, dl_trunk_size);
		if (dl_trunk_size == RDL_CHUNK)
			break;
		if (!bulkchunk) {
			dl_trunk_size /= 2;
			if (dl_trunk_size < RDL_CHUNK)
				dl_trunk_size = RDL_CHUNK;
		}
	}

	if (bulkchunk == NULL) {
		err = DBUS_ERR;
		goto fail;
	}

	sent = 0;
	dlpos = fw;
	dllen = fwlen;

	DBUSTRACE(("enter %s: fwlen=%d\n", __FUNCTION__, fwlen));

	dbus_usbos_dl_cmd(osinfo, DL_GETSTATE, &state, sizeof(rdl_state_t));

	/* 3) Load the image */
	while ((sent < dllen)) {
		/* Wait until the usb device reports it received all the bytes we sent */

		/* add a timeout so we dont wait indefinetly */
		if (sent < dllen) {
			if ((dllen-sent) < dl_trunk_size)
				sendlen = dllen-sent;
			else
				sendlen = dl_trunk_size;

			/* simply avoid having to send a ZLP by ensuring we never have an even
			 * multiple of 64
			 */
			if (!(sendlen % 64))
				sendlen -= 4;

			/* send data */
			memcpy(bulkchunk, dlpos, sendlen);
			if (!dbus_usbos_dl_send_bulk(osinfo, bulkchunk, sendlen)) {
				err = DBUS_ERR;
				goto fail;
			}

			dlpos += sendlen;
			sent += sendlen;
			DBUSTRACE(("%s: sendlen %d\n", __FUNCTION__, sendlen));
		}

		/* Wait for downloaded image crc check to complete in the dongle */
		while (!dbus_usbos_dl_cmd(osinfo, DL_GETSTATE, &state, sizeof(rdl_state_t))) {
			DBUSERR(("%s: DL_GETSTATE Failed xxxx\n", __FUNCTION__));
			err = DBUS_ERR;
			goto fail;
		}

		state.state = ltoh32(state.state);
		state.bytes = ltoh32(state.bytes);

		/* restart if an error is reported */
		if ((state.state == DL_BAD_HDR) || (state.state == DL_BAD_CRC)) {
			DBUSERR(("%s: Bad Hdr or Bad CRC\n", __FUNCTION__));
			err = DBUS_ERR;
			goto fail;
		}

	}
fail:
	if (bulkchunk)
		MFREE(osh, bulkchunk, dl_trunk_size);

	return err;
} /* dbus_usb_dl_writeimage */

/** Higher level DBUS layer (dbus.c) requests this layer to download image into dongle */
static int
dbus_usb_dlstart(void *bus, uint8 *fw, int len)
{
	usb_info_t *usbinfo = BUS_INFO(bus, usb_info_t);
	int err;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbinfo == NULL)
		return DBUS_ERR;

	if (USB_DEV_ISBAD(usbinfo))
		return DBUS_ERR;

	err = dbus_usb_rdl_dwnld_state(usbinfo);

	if (DBUS_OK == err) {
		err = dbus_usb_dl_writeimage(usbinfo, fw, len);
		if (err == DBUS_OK)
			usbinfo->pub->busstate = DBUS_STATE_DL_DONE;
		else
			usbinfo->pub->busstate = DBUS_STATE_DL_PENDING;
	} else
		usbinfo->pub->busstate = DBUS_STATE_DL_PENDING;

	return err;
}

static bool
dbus_usb_update_chipinfo(usb_info_t *usbinfo, uint32 chip)
{
	bool retval = TRUE;
	/* based on the CHIP Id, store the ram size which is needed for NVRAM download. */
	switch (chip) {
		case 0x4360:
		case 0xAA06:
			usbinfo->rdlram_size = RDL_RAM_SIZE_4360;
			usbinfo->rdlram_base_addr = RDL_RAM_BASE_4360;
			break;
		case 43569:
			usbinfo->rdlram_size = RDL_RAM_SIZE_4350;
			usbinfo->rdlram_base_addr = RDL_RAM_BASE_4350;
			break;
		case BCM4381_CHIP_ID:
			usbinfo->rdlram_size = RDL_RAM_SIZE_4381;
			usbinfo->rdlram_base_addr = RDL_RAM_BASE_4381;
			break;

		case BCM4382_CHIP_ID:
			usbinfo->rdlram_size = RDL_RAM_SIZE_4382;
			usbinfo->rdlram_base_addr = RDL_RAM_BASE_4382;
			break;

		case POSTBOOT_ID:
			break;

		default:
			DBUSERR(("%s: Chip 0x%x Ram size is not known\n", __FUNCTION__, chip));
			retval = FALSE;
			break;

	}

	return retval;
} /* dbus_usb_update_chipinfo */

/** higher DBUS level (dbus.c) wants to know if firmware download is required. */
static int
dbus_usb_dlneeded(void *bus)
{
	usb_info_t *usbinfo = BUS_INFO(bus, usb_info_t);
	void *osinfo;
	bootrom_id_t id;
	int dl_needed = 1;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbinfo == NULL)
		return DBUS_ERR;

	osinfo = usbinfo->usbosl_info;
	ASSERT(osinfo);

	/* Check if firmware downloaded already by querying runtime ID */
	id.chip = 0xDEAD;
	dbus_usbos_dl_cmd(osinfo, DL_GETVER, &id, sizeof(bootrom_id_t));

	id.chip = ltoh32(id.chip);
	id.chiprev = ltoh32(id.chiprev);

	if (FALSE == dbus_usb_update_chipinfo(usbinfo, id.chip)) {
		dl_needed = DBUS_ERR;
		goto exit;
	}

	DBUSERR(("%s: chip 0x%x rev 0x%x\n", __FUNCTION__, id.chip, id.chiprev));
	if (id.chip == POSTBOOT_ID) {
		/* This code is  needed to support two enumerations on USB1.1 scenario */
		DBUSERR(("%s: Firmware already downloaded\n", __FUNCTION__));

		dbus_usbos_dl_cmd(osinfo, DL_RESETCFG, &id, sizeof(bootrom_id_t));
		dl_needed = DBUS_OK;
		if (usbinfo->pub->busstate == DBUS_STATE_DL_PENDING)
			usbinfo->pub->busstate = DBUS_STATE_DL_DONE;
	} else {
		usbinfo->pub->attrib.devid = id.chip;
		usbinfo->pub->attrib.chiprev = id.chiprev;
	}

exit:
	return dl_needed;
}

/** After issuing firmware download, higher DBUS level (dbus.c) wants to start the firmware. */
static int
dbus_usb_dlrun(void *bus)
{
	usb_info_t *usbinfo = BUS_INFO(bus, usb_info_t);
	void *osinfo;
	rdl_state_t state;
	int err = DBUS_OK;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbinfo == NULL)
		return DBUS_ERR;

	if (USB_DEV_ISBAD(usbinfo))
		return DBUS_ERR;

	osinfo = usbinfo->usbosl_info;
	ASSERT(osinfo);

	/* Check we are runnable */
	dbus_usbos_dl_cmd(osinfo, DL_GETSTATE, &state, sizeof(rdl_state_t));

	state.state = ltoh32(state.state);
	state.bytes = ltoh32(state.bytes);

	/* Start the image */
	if (state.state == DL_RUNNABLE) {
		DBUSTRACE(("%s: Issue DL_GO\n", __FUNCTION__));
		dbus_usbos_dl_cmd(osinfo, DL_GO, &state, sizeof(rdl_state_t));

		/* : Need this for 4326 for some reason
		 * Same issue under both Linux/Windows
		 */
		if (usbinfo->pub->attrib.devid == TEST_CHIP)
			dbus_usbos_wait(osinfo, USB_DLGO_SPINWAIT);

//		dbus_usb_resetcfg(usbinfo);
		/* The Donlge may go for re-enumeration. */
	} else {
		DBUSERR(("%s: Dongle not runnable\n", __FUNCTION__));
		err = DBUS_ERR;
	}

	return err;
}

/**
 * As preparation for firmware download, higher DBUS level (dbus.c) requests the firmware image
 * to be used for the type of dongle detected. Directly called by dbus.c (so not via a callback
 * construction)
 */
void
dbus_bus_fw_get(void *bus, uint8 **fw, int *fwlen, int *decomp)
{
	usb_info_t *usbinfo = BUS_INFO(bus, usb_info_t);
	unsigned int devid;
	unsigned int crev;

	devid = usbinfo->pub->attrib.devid;
	crev = usbinfo->pub->attrib.chiprev;
	BCM_REFERENCE(crev);

	*fw = NULL;
	*fwlen = 0;

	switch (devid) {
	default:
#ifdef EMBED_IMAGE_GENERIC
		*fw = (uint8 *)dlarray;
		*fwlen = sizeof(dlarray);
#endif
		break;
	}
} /* dbus_bus_fw_get */

#ifdef BCM_DNGL_EMBEDIMAGE
static bool
dbus_usb_device_exists(void *bus)
{
	usb_info_t *usbinfo = BUS_INFO(bus, usb_info_t);
	void *osinfo;
	bootrom_id_t id;

	DBUSTRACE(("%s\n", __FUNCTION__));

	if (usbinfo == NULL)
		return FALSE;

	osinfo = usbinfo->usbosl_info;
	ASSERT(osinfo);

	id.chip = 0xDEAD;
	/* Query device to see if we get a response */
	dbus_usbos_dl_cmd(osinfo, DL_GETVER, &id, sizeof(bootrom_id_t));

	usbinfo->pub->attrib.devid = id.chip;
	if (id.chip == 0xDEAD)
		return FALSE;
	else
		return TRUE;
}
#endif /* BCM_DNGL_EMBEDIMAGE */
