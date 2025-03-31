/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: singly list
 * This file should be changed only infrequently and with great care.
 */
#ifndef __DFX_SINGLY_LIST_H__
#define __DFX_SINGLY_LIST_H__
#include "td_base.h"
#include "soc_errno.h"
#include "zdiag_config.h"

typedef struct dfx_singly_list_node {
    struct dfx_singly_list_node *next;
} dfx_singly_list_node;

typedef struct {
    dfx_singly_list_node *head;
    dfx_singly_list_node *tail;
} dfx_singly_list;

td_void dfx_singly_list_init(dfx_singly_list *list);
td_void dfx_singly_list_push_node(dfx_singly_list *list, dfx_singly_list_node *node);
dfx_singly_list_node *dfx_singly_list_pop_node(dfx_singly_list *list);
dfx_singly_list_node *dfx_singly_list_get_first_node(dfx_singly_list *list);

#endif
