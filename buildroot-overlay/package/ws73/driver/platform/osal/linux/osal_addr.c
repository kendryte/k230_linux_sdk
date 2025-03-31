/*
 * Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
 */

#include <asm/io.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/mm.h>
#include "soc_osal.h"
#include "osal_inner.h"

#define OSAL_ADDR_RESERVED_SIZE_MAX  0xF000000

static unsigned int osal_convert_gfp_flag(unsigned int osal_gfp_flag)
{
    unsigned int gfp_flag;

    switch (osal_gfp_flag) {
        case OSAL_GFP_KERNEL:
            gfp_flag = GFP_KERNEL;
            break;

        case OSAL_GFP_ATOMIC:
            gfp_flag = GFP_ATOMIC;
            break;

        case OSAL_GFP_DMA:
            gfp_flag = GFP_DMA;
            break;

        case OSAL_GFP_ZERO:
            gfp_flag = __GFP_ZERO;
            break;

        default:
            gfp_flag = GFP_KERNEL;
            break;
    }

    return gfp_flag;
}

void *osal_kmalloc(unsigned long size, unsigned int osal_gfp_flag)
{
    unsigned int  gfp_flag = osal_convert_gfp_flag(osal_gfp_flag);
    return kmalloc(size, gfp_flag);
}
EXPORT_SYMBOL(osal_kmalloc);

void osal_kfree(void *addr)
{
    if (addr != NULL) {
        kfree(addr);
    }
    return;
}
EXPORT_SYMBOL(osal_kfree);

void *osal_vmalloc(unsigned long size)
{
    return vmalloc(size);
}
EXPORT_SYMBOL(osal_vmalloc);

void osal_vfree(void *addr)
{
    if (addr != NULL) {
        vfree(addr);
    }
    return;
}
EXPORT_SYMBOL(osal_vfree);

#ifdef OSAL_API_SUPPORT_MEMMAP
void *osal_ioremap(unsigned long phys_addr, unsigned long size)
{
    return ioremap(phys_addr, size);
}
EXPORT_SYMBOL(osal_ioremap);

void *osal_ioremap_nocache(unsigned long phys_addr, unsigned long size)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    return ioremap(phys_addr, size);
#else
    return ioremap_nocache(phys_addr, size);
#endif
}
EXPORT_SYMBOL(osal_ioremap_nocache);

void *osal_ioremap_wc(unsigned long phys_addr, unsigned long size)
{
    return ioremap_wc(phys_addr, size);
}
EXPORT_SYMBOL(osal_ioremap_wc);

void *osal_ioremap_cached(unsigned long phys_addr, unsigned long size)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
    return ioremap_cached(phys_addr, size);
#else
    return ioremap_cache(phys_addr, size);
#endif
}
EXPORT_SYMBOL(osal_ioremap_cached);

void osal_iounmap(void *addr, unsigned long size)
{
    iounmap(addr);
}
EXPORT_SYMBOL(osal_iounmap);
#endif

unsigned long osal_copy_from_user(void *to, const void *from, unsigned long n)
{
    return copy_from_user(to, from, n);
}
EXPORT_SYMBOL(osal_copy_from_user);

unsigned long osal_copy_to_user(void *to, const void *from, unsigned long n)
{
    return copy_to_user(to, from, n);
}
EXPORT_SYMBOL(osal_copy_to_user);

int osal_access_ok(int type, const void *addr, unsigned long size)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    return access_ok(addr, size);
#else
    return access_ok(type, addr, size);
#endif
}
EXPORT_SYMBOL(osal_access_ok);

void *osal_phys_to_virt(unsigned long addr)
{
    return phys_to_virt(addr);
}
EXPORT_SYMBOL(osal_phys_to_virt);

unsigned long osal_virt_to_phys(const void *virt_addr)
{
    return virt_to_phys(virt_addr);
}
EXPORT_SYMBOL(osal_virt_to_phys);

#ifdef OSAL_API_SUPPORT_MEMMAP
/*
 * Maps @size from @phys_addr into contiguous kernel virtual space
 * Note:this function only support VM_MAP with PAGE_KERNEL flag
 * */
void *osal_blockmem_vmap(unsigned long phys_addr, unsigned long size)
{
    unsigned int i;
    unsigned int page_count;
    struct page **pages = NULL;
    void *vaddr = NULL;

    if ((phys_addr == 0) || (size == 0) || size > OSAL_ADDR_RESERVED_SIZE_MAX) {
        osal_log("invalid vmap phys_addr is null or size:%lu!\n", size);
        return NULL;
    }

    page_count = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    pages = vmalloc(page_count * sizeof(struct page *));
    if (pages == NULL) {
        osal_log("vmap malloc pages failed!\n");
        return NULL;
    }

    for (i = 0; i < page_count; i++) {
        pages[i] = phys_to_page(phys_addr + i * PAGE_SIZE);
    }

    vaddr = vmap(pages, page_count, VM_MAP, PAGE_KERNEL);
    if (vaddr == NULL) {
        osal_log("vmap failed phys_addr:0x%lX, size:%lu!\n", phys_addr, size);
    }

    vfree(pages);
    pages = NULL;

    return vaddr;
}
EXPORT_SYMBOL(osal_blockmem_vmap);

/*
 * Free the virtually contiguous memory area starting at @virt_addr
 * which was created from the phys_addr passed to osal_vunmap()
 * Must not be called in interrupt context.
 * */
void osal_blockmem_vunmap(const void *virt_addr)
{
    if (virt_addr == NULL) {
        osal_log("vumap failed: virt_addr is NULL!\n");
        return;
    }

    vunmap(virt_addr);
}
EXPORT_SYMBOL(osal_blockmem_vunmap);

osal_blockmem_status osal_blockmem_get_status(unsigned long phyaddr, unsigned int size)
{
    unsigned int pfn_start;
    unsigned int pfn_end;
    unsigned int valid_pages = 0;
    osal_blockmem_status  status;

    pfn_start = __phys_to_pfn(phyaddr);
    pfn_end = __phys_to_pfn(phyaddr + size);

    for (; pfn_start < pfn_end; pfn_start++) {
        struct page *page = pfn_to_page(pfn_start);
        if (!PageReserved(page)) {
            break;
        }
        valid_pages++;
    }

    if (valid_pages == (size >> PAGE_SHIFT)) {
        status = OSAL_BLOCKMEM_VALID;
    } else if (valid_pages == 0) {
        status = OSAL_BLOCKMEM_INVALID_PHYADDR;
    } else {
        status = OSAL_BLOCKMEM_INVALID_SIZE;
    }

    return status;
}
EXPORT_SYMBOL(osal_blockmem_get_status);
#endif

