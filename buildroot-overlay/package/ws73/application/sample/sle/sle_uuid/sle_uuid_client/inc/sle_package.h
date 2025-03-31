#ifndef __SLE_PACKAGE_H
#define __SLE_PACKAGE_H

#define PACKAGE_LENGTH (230U)
#define PACKAGE_DATA_LENGTH (PACKAGE_LENGTH - sizeof(sle_package_t))

typedef struct {
    long        mtype;
    size_t      index;
    void        *peri;
    size_t      data_len;
    uint8_t     data[0];
} sle_package_t;

typedef struct {
    size_t index_start;
    size_t index_stop;
    struct timespec *time_start;
    struct timespec *time_stop;
    size_t total_byte;
} sle_transfer_stat_t;
#endif
