#ifndef __MMZ__
#define __MMZ__

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

int kd_mpi_mmz_init(void);
int kd_mpi_mmz_deinit(void);
int kd_mpi_sys_mmz_alloc(unsigned long *phy_addr, void **virt_addr, const char *mmb, const char *zone, unsigned int len);
int kd_mpi_sys_mmz_alloc_cached(unsigned long *phy_addr, void **virt_addr, const char *mmb, const char *zone, unsigned int len);
int kd_mpi_sys_mmz_flush_cache(unsigned long phy_addr, void *virt_addr, unsigned int size);
int kd_mpi_sys_mmz_free(unsigned long phy_addr, void *virt_addr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
