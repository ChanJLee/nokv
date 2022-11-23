//
// Created by chan on 2022/5/31.
//

#ifndef NKV_BCTL_H
#define NKV_BCTL_H

#include <inttypes.h>

typedef unsigned char byte_t;

/*
 * Get a 2-byte value, in big-endian order, from memory.
 */
static inline uint16_t get2BE(const byte_t *buf) {
    uint16_t val;

    val = (buf[0] << 8) | buf[1];
    return val;
}

/*
 * Set a 4-byte value, in big-endian order.
 */
static inline void set2BE(byte_t *buf, uint16_t val) {
    buf[0] = val >> 8;
    buf[1] = val;
}

/*
 * Get a 4-byte value, in big-endian order, from memory.
 */
static inline uint32_t get4BE(const byte_t *buf) {
    uint32_t val;

    val = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    return val;
}

/*
 * Set a 4-byte value, in big-endian order.
 */
static inline void set4BE(byte_t *buf, uint32_t val) {
    buf[0] = val >> 24;
    buf[1] = val >> 16;
    buf[2] = val >> 8;
    buf[3] = val;
}

static inline void set4LE(byte_t *buf, uint32_t val)
{
    buf[3] = val >> 24;
    buf[2] = val >> 16;
    buf[1] = val >> 8;
    buf[0] = val;
}

/*
 * Get a 8-byte value, in big-endian order, from memory.
 */
static inline uint64_t get8BE(const byte_t *buf) {
    uint64_t val = get4BE(buf);
    return val << 32 | get4BE(buf + sizeof(uint32_t));
}

/*
 * Set a 8-byte value, in big-endian order.
 */
static inline void set8BE(byte_t *buf, uint64_t val) {
    set4BE(buf, val >> 32);
    set4BE(buf + sizeof(uint32_t), val);
}

#define __read_size(is, name, type, buf, size) \
    if (is->read(buf, size) != size) { \
       return false;                    \
    } \
    type name = get##size##BE(buf)

#define __write_size(os, name, buf, size) \
    set##size##BE(buf, name);    \
    os->write(buf, size);

#endif //NKV_BCTL_H