//
// Created by chan on 2025/8/27.
//

#include "boot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __linux__

#include <errno.h>

int get_boot_id(char *buf, size_t len) {
    if (!buf || len < 37) return -1;

    FILE *fp = fopen("/proc/sys/kernel/random/boot_id", "r");
    if (!fp) return -1;

    if (!fgets(buf, (int) len, fp)) {
        fclose(fp);
        return -1;
    }
    fclose(fp);

    // 去掉换行符
    buf[strcspn(buf, "\n")] = '\0';
    return 0;
}

#elif __APPLE__
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <uuid/uuid.h>

int get_boot_id(char *buf, size_t len) {
    if (!buf || len < 37) return -1;

    struct timeval boottime;
    size_t size = sizeof(boottime);
    int mib[2] = {CTL_KERN, KERN_BOOTTIME};

    if (sysctl(mib, 2, &boottime, &size, NULL, 0) != 0) {
        return -1;
    }

    // 伪随机种子基于开机时间
    unsigned long long seed =
        ((unsigned long long)boottime.tv_sec << 20) ^
        (unsigned long long)boottime.tv_usec;

    // 构造一个 128bit 的 buffer
    uuid_t uuid;
    for (int i = 0; i < 16; i++) {
        uuid[i] = (unsigned char)((seed >> ((i % 8) * 8)) & 0xFF);
    }

    // 设置 UUID 版本（v4随机）和变种位，符合标准格式
    uuid[6] = (uuid[6] & 0x0F) | 0x40; // version 4
    uuid[8] = (uuid[8] & 0x3F) | 0x80; // variant

    char uuid_str[37];
    uuid_unparse_lower(uuid, uuid_str);

    snprintf(buf, len, "%s", uuid_str);
    return 0;
}

#else
int get_boot_id(char *buf, size_t len) {
    (void)buf; (void)len;
    return -1; // 不支持的平台
}
#endif