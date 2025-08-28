//
// Created by chan on 2025/8/27.
//

#ifndef NKV_BOOT_H
#define NKV_BOOT_H

#include <stddef.h>

#define BOOT_ID_SIZE 64

#ifdef __cplusplus
extern "C" {
#endif

// 获取 boot_id（跨平台）
// 返回值：0 表示成功，非0表示失败
// buf: 存放boot_id的缓冲区
// len: 缓冲区长度（建议 >= 64）
int get_boot_id(char *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif //NKV_BOOT_H
