#include <jni.h>
#include <string>
#include "util/LogUtil.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>

}

extern "C" JNIEXPORT jstring JNICALL
Java_com_wpf_ffmpegtest_FfmpegTools_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {

/*    std::string hello = "Hello from C++";
    LOGE("my log hello word");
    return env->NewStringUTF(hello.c_str());*/

    //获取ffmpeg版本信息
    char data[1024 * 4] = {0};
    strcat(data, "libavcodec: ");
    strcat(data, AV_STRINGIFY(LIBAVCODEC_VERSION));
    strcat(data, "\nlibavfilter: ");
    strcat(data, AV_STRINGIFY(LIBAVFILTER_VERSION));
    strcat(data, "\nlibavformat: ");
    strcat(data, AV_STRINGIFY(LIBAVFORMAT_VERSION));
    strcat(data, "\nlibavutil: ");
    strcat(data, AV_STRINGIFY(LIBAVUTIL_VERSION));
    strcat(data, "\nlibswresample: ");
    strcat(data, AV_STRINGIFY(LIBSWRESAMPLE_VERSION));
    strcat(data, "\nlibswscale: ");
    strcat(data, AV_STRINGIFY(LIBSWSCALE_VERSION));


    return env->NewStringUTF(data);
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_wpf_ffmpegtest_FfmpegTools_native_1start_1play(JNIEnv *env, jobject thiz, jstring path, jobject surface) {
    // TODO: implement native_start_play()


}