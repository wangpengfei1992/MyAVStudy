//
// Created by anker on 2022/5/31.
// 定义打印方法
//

#ifndef FFMPEGTEST_LOGUTIL_H
#define FFMPEGTEST_LOGUTIL_H

#include <android/log.h>

#define LOG_TAG "WPF"

#define LOGI(...)   __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);
#define LOGD(...)   __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__);
#define LOGE(...)   __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__);
#define LOGE(...)   __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__);


#endif //FFMPEGTEST_LOGUTIL_H
