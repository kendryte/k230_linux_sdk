/*
 * Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
 * Description: oal sdio liteos scatter.
 */
#ifndef __OAL_SDIO_LITEOS_SCATTER_H__
#define __OAL_SDIO_LITEOS_SCATTER_H__

#ifdef CONFIG_HCC_SUPPORT_SDIO
typedef td_u32 gfp_t;

struct sg_table {
    struct scatterlist *sgl;    /* the list */
    td_u32 nents;         /* number of mapped entries */
    td_u32 orig_nents;    /* original size of list */
};

#define sg_is_chain(sg)         ((sg)->page_link & 0x01)
#define sg_is_last(sg)          ((sg)->page_link & 0x02)
#define sg_chain_ptr(sg)        ((struct scatterlist *) ((uintptr_t)((sg)->page_link) & ~0x03))
#define for_each_sg(sglist, sg, nr, i) \
    for ((i) = 0, (sg)= (sglist); (i) < (nr); (i)++, (sg) = sg_next((sg)))

#define SG_MAX_SINGLE_ALLOC             (PAGE_SIZE / sizeof(struct scatterlist))

typedef struct scatterlist *(sg_alloc_fn)(td_u32, gfp_t);
typedef td_void (sg_free_fn)(struct scatterlist *, td_u32);

static struct scatterlist *sg_next(struct scatterlist *sg)
{
    if (sg_is_last(sg)) {
        return TD_NULL;
    }

    sg++;
    if (unlikely(sg_is_chain(sg))) {
        sg = sg_chain_ptr(sg);
    }

    return sg;
}

static inline td_void *sg_virt(struct scatterlist *sg)
{
    return ((td_void *)(uintptr_t)(sg->dma_address + sg->offset));
}

static td_void sg_kfree(struct scatterlist *sg, td_u32 nents)
{
    osal_kfree(sg);
}

static td_void sg_free_table_func(struct sg_table *table, td_u32 max_ents, sg_free_fn *free_fn)
{
    struct scatterlist *sgl = TD_NULL;
    struct scatterlist *next = TD_NULL;

    if (unlikely(!table->sgl)) {
        return;
    }

    sgl = table->sgl;
    while (table->orig_nents) {
        td_u32 sg_alloc_size = table->orig_nents;
        td_u32 sg_size;

        if (sgl == TD_NULL) {
            break;
        }
        if (sg_alloc_size > max_ents) {
            next = sg_chain_ptr(&sgl[max_ents - 1]);
            sg_alloc_size = max_ents;
            sg_size = sg_alloc_size - 1;
        } else {
            sg_size = sg_alloc_size;
            next = TD_NULL;
        }

        table->orig_nents -= sg_size;
        free_fn(sgl, sg_alloc_size);
        sgl = next;
    }

    table->sgl = TD_NULL;
}


static inline td_void sg_free_table(struct sg_table *table)
{
    sg_free_table_func(table, SG_MAX_SINGLE_ALLOC, sg_kfree);
}

static struct scatterlist *sg_kmalloc(td_u32 nents, gfp_t gfp_mask)
{
    return osal_kmalloc(nents * sizeof(struct scatterlist), gfp_mask);
}

static inline td_void sg_chain(struct scatterlist *prv, td_u32 prv_nents, struct scatterlist *sgl)
{
    prv[prv_nents - 1].offset = 0;
    prv[prv_nents - 1].length = 0;

    prv[prv_nents - 1].page_link = ((unsigned long)(uintptr_t)sgl | 0x01) & ~0x02;
}

static td_s32 sg_alloc_table_func(struct sg_table *table, td_u32 nents, td_u32 max_ents, gfp_t gfp_mask,
    sg_alloc_fn *alloc_fn)
{
    struct scatterlist *sg = TD_NULL;
    struct scatterlist *prv = TD_NULL;
    td_u32 left;

    left = nents;
    prv = TD_NULL;
    do {
        td_u32 sg_size;
        td_u32 sg_alloc_size = left;

        if (sg_alloc_size > max_ents) {
            sg_alloc_size = max_ents;
            sg_size = sg_alloc_size - 1;
        } else {
            sg_size = sg_alloc_size;
        }

        left -= sg_size;

        sg = alloc_fn(sg_alloc_size, gfp_mask);
        if (sg == TD_NULL) {
            if (prv != TD_NULL) {
                table->nents = ++table->orig_nents;
            }

            return -ENOMEM;
        }
        sg_init_table(sg, sg_alloc_size);
        table->nents = table->orig_nents += sg_size;

        if (prv != TD_NULL) {
            sg_chain(prv, max_ents, sg);
        } else {
            table->sgl = sg;
        }

        if (!left) {
            sg_mark_end(&sg[sg_size - 1]);
        }

        prv = sg;
    } while (left);

    return 0;
}


static inline td_s32 sg_alloc_table(struct sg_table *table, td_u32 nents, gfp_t gfp_mask)
{
    td_s32 ret;

    ret = sg_alloc_table_func(table, nents, SG_MAX_SINGLE_ALLOC, gfp_mask, sg_kmalloc);
    if (unlikely(ret)) {
        sg_free_table_func(table, SG_MAX_SINGLE_ALLOC, sg_kfree);
    }

    return ret;
}
#endif
#endif
