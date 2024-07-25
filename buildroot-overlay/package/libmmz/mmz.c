#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>
#include<stdlib.h>
#include<assert.h>
#include <stdint.h>
#include <pthread.h>
#include"mmz.h"

#define MMZ_ALLOC_MEM                         _IOWR('g', 1, unsigned long)
#define MMZ_FREE_MEM                          _IOWR('g', 2, unsigned long)

#define MMZ_DEV         "/dev/mmz"

typedef struct {
    void *user_virt_addr;
    void *kernel_virt_addr;
    unsigned long mmz_phys;
    unsigned long length;
} mmz_data_type;

typedef struct s_mmz_node
{
	mmz_data_type data;
	struct s_mmz_node* next;
}mmz_node;

static pthread_mutex_t mmz_mutex = PTHREAD_MUTEX_INITIALIZER;
int fd_mmz = -1;
mmz_node* plist = NULL;
int mmz_init_flag = 0;

static inline void thead_csi_dcache_clean_invalid_range(void* addr, uint64_t size)
{
    uint64_t op_addr = (uint64_t)addr;
    int64_t op_size = size + op_addr % 64;
    const int64_t linesize = 64;

    __asm volatile("fence iorw, iorw");

    while (op_size > 0) {
        __asm volatile("dcache.civa %0" : : "r"(op_addr));
        op_addr += linesize;
        op_size -= linesize;
    }

    __asm volatile("fence iorw, iorw");
    __asm volatile("fence.i");
    __asm volatile("fence r, r");
}

static mmz_node* mmz_create_node(void)
{
	mmz_node* newnode = (mmz_node*)malloc(sizeof(mmz_node));
	if (newnode == NULL)
	{
		perror("malloc");
		exit(1);
	}
	memset(&(newnode->data), 0, sizeof(newnode->data));
	newnode->next = NULL;

	return newnode;
}

static void mmz_push_back(mmz_node** pphead, mmz_node* node)
{
	assert(pphead);
	mmz_node* newnode = node;
	if (*pphead == NULL)
	{
		*pphead = newnode;
	}
	else
	{
		mmz_node* ptail = *pphead;
		while (ptail->next != NULL)
		{
			ptail = ptail->next;
		}
		ptail->next = newnode;
	}
}

static void mmz_pop_front(mmz_node** pphead)
{
	assert(pphead);
	assert(*pphead);
	mmz_node* next = (*pphead)->next;
	free(*pphead);
	*pphead = next;
}

static mmz_node* mmz_find(mmz_node* phead, void *ptr)
{
	mmz_node* pcur = phead;
	while (pcur)
	{
		if (pcur->data.user_virt_addr == ptr)
		{
			return pcur;
		}
		pcur = pcur->next;
	}
	return NULL;
}

static void mmz_erase(mmz_node** pphead, mmz_node* pos)
{
	assert(pphead);
	assert(*pphead);
	assert(pos);

	if (pos == *pphead)
	{
		mmz_pop_front(pphead);
	}
	else
	{
		mmz_node* prev = *pphead;
		while (prev->next != pos)
		{
			prev = prev->next;
		}
		prev->next = pos->next;
		free(pos);
		pos = NULL;
	}
}

static int mmz_free(unsigned long phy_addr, void *virt_addr)
{
    mmz_node* ret = mmz_find(plist, virt_addr);
	if(ret == NULL)
	{
		printf("Find Err\n");
        return -1;
	}
	else
	{
		// printf("Find pass\n");
		// printf("PHY 0x%llX, user addr %p\n\n", ret->data.mmz_phys, ret->data.user_virt_addr);
	}

    if (munmap(virt_addr, ret->data.length) == -1) {
        printf("failed to munmap /dev/mmz!");
    }

    ioctl(fd_mmz, MMZ_FREE_MEM, &ret->data);

    mmz_erase(&plist, ret);

    return 0;
}

static int mmz_destroy(mmz_node** pphead)
{
    mmz_node* pcur = *pphead;
    while (pcur)
	{
		mmz_node* tmp = pcur->next;

        if(mmz_free(pcur->data.mmz_phys, pcur->data.user_virt_addr))
        {
            return -1;
        }
		pcur = tmp;
	}

    return 0;
}

static int mmz_init(void)
{
    fd_mmz = open(MMZ_DEV, O_RDWR | O_SYNC);
    if(fd_mmz < 0)
    {
        printf("open k230-mmz error\n");
        return -1;
    }
    return 0;
}

int kd_mpi_sys_mmz_alloc(unsigned long *phy_addr, void **virt_addr, const char *mmb, const char *zone, unsigned int len)
{
    mmz_node *tmp;
    pthread_mutex_lock(&mmz_mutex);

    if(fd_mmz < 0) {
        if(mmz_init()) {
            pthread_mutex_unlock(&mmz_mutex);
            return -1;
        }
    }

    tmp = mmz_create_node();
    tmp->data.length = len;
    ioctl(fd_mmz, MMZ_ALLOC_MEM, &(tmp->data));
    // printf("Alloc phy 0x%llX\n", tmp->data.mmz_phys);
    tmp->data.user_virt_addr = mmap(NULL, tmp->data.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mmz, tmp->data.mmz_phys);
    if (tmp->data.user_virt_addr == MAP_FAILED) {
        pthread_mutex_unlock(&mmz_mutex);
        printf("failed to mmap /dev/mmz!");
        return -1;
    }

    *phy_addr = tmp->data.mmz_phys;
    *virt_addr = tmp->data.user_virt_addr;
    mmz_push_back(&plist, tmp);
    pthread_mutex_unlock(&mmz_mutex);

    return 0;
}

int kd_mpi_sys_mmz_alloc_cached(unsigned long *phy_addr, void **virt_addr, const char *mmb, const char *zone, unsigned int len)
{
    return kd_mpi_sys_mmz_alloc(phy_addr, virt_addr, mmb, zone, len);
}

int kd_mpi_sys_mmz_free(unsigned long phy_addr, void *virt_addr)
{
    pthread_mutex_lock(&mmz_mutex);
    mmz_free(phy_addr, virt_addr);
    pthread_mutex_unlock(&mmz_mutex);

    return 0;
}

int kd_mpi_sys_mmz_flush_cache(unsigned long phy_addr, void *virt_addr, unsigned int size)
{
    thead_csi_dcache_clean_invalid_range(virt_addr, size);
    return 0;
}

int kd_mpi_mmz_init(void)
{
    int ret = 0;

    pthread_mutex_lock(&mmz_mutex);
    ret = mmz_init();
    pthread_mutex_unlock(&mmz_mutex);

    return ret;
}

int kd_mpi_mmz_deinit(void)
{
    pthread_mutex_lock(&mmz_mutex);
    mmz_destroy(&plist);
    if(fd_mmz >= 0)
        close(fd_mmz);
    fd_mmz = -1;
    plist = NULL;
    pthread_mutex_unlock(&mmz_mutex);
    return 0;
}
