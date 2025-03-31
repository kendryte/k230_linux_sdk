/*
 * Copyright (c) CompanyNameMagicTag 2021-2021. All rights reserved.
 * Description: ipc private definition for host
 * Create: 2021-11-11
 */

#ifndef HCC_ADAPT_IPC_PRIVATE_H
#define HCC_ADAPT_IPC_PRIVATE_H
#ifdef CONFIG_HCC_SUPPORT_IPC
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define IPC_CREDIT_READ_ADDR IPC_CREDIT_START_ADDR
#define IPC_CREDIT_WRITE_ADDR (IPC_CREDIT_START_ADDR + 4)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* CONFIG_HCC_SUPPORT_IPC */
#endif /* HCC_ADAPT_IPC_PRIVATE_H */