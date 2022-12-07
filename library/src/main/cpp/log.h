//
// Created by chan on 2022/5/19.
//

#ifndef NKV_LOG_H
#define NKV_LOG_H

#if !defined(NKV_UNIT_TEST) && !defined(NKV_CLI)

#include <android/log.h>

#define LOG_TAG "NoKV"

#define LOGI(...)  ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, \
                   __VA_ARGS__))
#ifdef BAY_DEBUG
#define LOGD(...)  ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, \
                   __VA_ARGS__))
#else
#define LOGD(...) (void*) 0
#endif
#else
#include <stdio.h>
#define LOGI(...)  do {printf(__VA_ARGS__);printf("\n");} while(0);
#define LOGD(...)  do {printf(__VA_ARGS__);printf("\n");} while(0);
#endif

#endif //NKV_LOG_H
