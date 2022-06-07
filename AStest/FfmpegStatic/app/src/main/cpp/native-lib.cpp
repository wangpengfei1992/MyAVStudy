#include <jni.h>
#include <string>
#include "util/LogUtil.h"
#include <android/native_window_jni.h>
#include <zconf.h>


extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
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
JNIEXPORT void JNICALL
Java_com_wpf_ffmpegtest_FfmpegTools_native_1start_1play(JNIEnv *env, jobject thiz, jstring path, jobject surface) {
    // TODO: implement native_start_play()

/*    ANativeWindow* aNativeWindow = ANativeWindow_fromSurface(env,surface);//创建native window
    const char* cPath = env->GetStringUTFChars(path, 0);
    //ffmpeg 处理
    avformat_network_init();
    AVFormatContext* avFormatContext = avformat_alloc_context();

    //打开流对象
    AVDictionary* opertion = NULL;
    av_dict_set(&opertion,"timeout","3000000",0);
    int ret = 0;
    ret = avformat_open_input(&avFormatContext,cPath, NULL,&opertion);
    if (ret){
        return;
    }
    //获取流信息
    avformat_find_stream_info(avFormatContext,&opertion);
    int steam_index = 0;
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            steam_index = i;
            break;
        }
    }
    //获取解码参数
    AVCodecParameters * avCodecParameters = avFormatContext->streams[steam_index]->codecpar;
    AVCodec* avCodec = const_cast<AVCodec *>(avcodec_find_decoder(avCodecParameters->codec_id));
    AVCodecContext * avCodecContext = avcodec_alloc_context3(avCodec);
    avcodec_open2(avCodecContext,avCodec,&opertion);

    AVPacket * avPacket = av_packet_alloc();

    //获取转换上下文（把解码后的YUV数据转换为RGB数据才能在屏幕上显示）
    SwsContext* swsContext = sws_getContext(avCodecContext->width,avCodecContext->height,avCodecContext->pix_fmt,
                                            avCodecContext->width,avCodecContext->height,AV_PIX_FMT_ABGR,
                                            SWS_BILINEAR,0,0,0);

    //设置window接收缓存区
    ANativeWindow_setBuffersGeometry(aNativeWindow,avCodecContext->width,avCodecContext->height,WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer  aNativeWindowBuffer;
    //计算出转换为RGB所需要的容器的大小
    //接收的容器
    uint8_t *dst_data[4];
    //每一行的首地址（R、G、B、A四行）
    int dst_line_size[4];
    //进行计算
    av_image_alloc(dst_data,dst_line_size,avCodecContext->width,avCodecContext->height,
                   AV_PIX_FMT_RGBA,1);

    //从视频流中读数据包，返回值小于0的时候表示读取完毕
    while (av_read_frame(avFormatContext,avPacket) >= 0){
        //将取出的数据发送出来
        avcodec_send_packet(avCodecContext,avPacket);

        //接收发送出来的数据
        AVFrame *av_frame = av_frame_alloc();
        int av_receive_result = avcodec_receive_frame(avCodecContext,av_frame);

        //如果读取失败就重新读
        if(av_receive_result == AVERROR(EAGAIN)){
            continue;
        } else if(av_receive_result < 0){
            //如果到末尾了就结束循环读取
            break;
        }

        //将取出的数据放到之前定义的RGB目标容器中
        sws_scale(swsContext,av_frame->data,av_frame->linesize,0,av_frame->height,
                  dst_data,dst_line_size);

        //加锁然后进行渲染
        ANativeWindow_lock(aNativeWindow,&aNativeWindowBuffer,0);

        uint8_t *first_window = static_cast<uint8_t *>(aNativeWindowBuffer.bits);
        uint8_t *src_data = dst_data[0];

        //拿到每行有多少个RGBA字节
        int dst_stride = aNativeWindowBuffer.stride * 4;
        int src_line_size = dst_line_size[0];
        //循环遍历所得到的缓冲区数据
        for(int i = 0; i < aNativeWindowBuffer.height;i++){
            //内存拷贝进行渲染
            memcpy(first_window+i*dst_stride,src_data+i*src_line_size,dst_stride);
        }

        //绘制完解锁
        ANativeWindow_unlockAndPost(aNativeWindow);

        //40000微秒之后解析下一帧(这个是根据视频的帧率来设置的，我这播放的视频帧率是25帧/秒)
        usleep(1000 * 40);
        //释放资源
        av_frame_free(&av_frame);
        av_packet_free(&avPacket);
    }


    env->ReleaseStringUTFChars(path,cPath);
    avformat_free_context(avFormatContext);
    avcodec_parameters_free(&avCodecParameters);
    avcodec_free_context(&avCodecContext);
    av_packet_free(&avPacket);
    sws_freeContext(swsContext);*/
}