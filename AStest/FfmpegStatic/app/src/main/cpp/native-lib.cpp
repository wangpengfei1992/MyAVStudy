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

    ANativeWindow* aNativeWindow = ANativeWindow_fromSurface(env,surface);//创建native window
    const char* cPath = env->GetStringUTFChars(path, 0);
    //ffmpeg 处理
    avformat_network_init();
    AVFormatContext* avFormatContext = avformat_alloc_context();

    //打开流对象
/*    AVDictionary* opertion = NULL;
    av_dict_set(&opertion,"timeout","3000000",0);*/
    int ret = 0;
    ret = avformat_open_input(&avFormatContext,cPath, NULL,NULL);
    if (ret){
        return;
    }
    //获取流信息
    avformat_find_stream_info(avFormatContext,NULL);
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
    avcodec_open2(avCodecContext,avCodec,NULL);

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
    sws_freeContext(swsContext);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wpf_ffmpegtest_FfmpegTools_native_1start_1play2(JNIEnv *env, jobject thiz, jstring path, jobject surface) {
    //获取用于绘制的NativeWindow
    ANativeWindow *a_native_window = ANativeWindow_fromSurface(env,surface);

    //转换视频路径字符串为C中可用的
    const char *video_path = env->GetStringUTFChars(path,0);

    //网络模块初始化（可以播放Url）
    avformat_network_init();

    //获取用于获取视频文件中各种流（视频流、音频流、字幕流等）的上下文：AVFormatContext
    AVFormatContext *av_format_context = avformat_alloc_context();

    //配置信息
    AVDictionary *options = NULL;
    av_dict_set(&options,"timeout","3000000",0);

    //打开视频文件
    //第一个参数：AVFormatContext的二级指针
    //第二个参数：视频路径
    //第三个参数：非NULL的话就是设置输入格式，NULL就是自动
    //第四个参数：配置项
    //返回值是是否打开成功，0是成功其他为失败
    int open_result = avformat_open_input(&av_format_context, video_path, NULL, &options);

    //如果打开失败就返回
    if(open_result){
        return;
    }

    //让FFmpeg将流解析出来,并找到视频流对应的索引
    avformat_find_stream_info(av_format_context, NULL);
    int video_stream_index = 0;
    for(int i = 0; i < av_format_context->nb_streams ; i++){
        //如果当前流是视频流的话保存索引
        if(av_format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_index = i;
            break;
        }
    }

    //获取视频流的解码参数（宽高等信息）
    AVCodecParameters * av_codec_parameters = av_format_context->streams[video_stream_index]->codecpar;

    //获取视频流的解码器
    AVCodec *av_codec = const_cast<AVCodec *>(avcodec_find_decoder(av_codec_parameters->codec_id));

    //获取解码上下文
    AVCodecContext * av_codec_context = avcodec_alloc_context3(av_codec);

    //将解码器参数复制到解码上下文(因为解码上下文目前还没有解码器参数)
    avcodec_parameters_to_context(av_codec_context,av_codec_parameters);

    //进行解码
    avcodec_open2(av_codec_context,av_codec,NULL);

    //因为YUV数据被封装在了AVPacket中，因此我们需要用AVPacket去获取数据
    AVPacket *av_packet = av_packet_alloc();

    //获取转换上下文（把解码后的YUV数据转换为RGB数据才能在屏幕上显示）
    SwsContext *sws_context = sws_getContext(av_codec_context->width,av_codec_context->height,av_codec_context->pix_fmt,
                                             av_codec_context->width,av_codec_context->height,AV_PIX_FMT_RGBA,SWS_BILINEAR,
                                             0,0,0);

    //设置NativeWindow绘制的缓冲区
    ANativeWindow_setBuffersGeometry(a_native_window,av_codec_context->width,av_codec_context->height,
                                     WINDOW_FORMAT_RGBA_8888);
    //绘制时，用于接收的缓冲区
    ANativeWindow_Buffer a_native_window_buffer;

    //计算出转换为RGB所需要的容器的大小
    //接收的容器
    uint8_t *dst_data[4];
    //每一行的首地址（R、G、B、A四行）
    int dst_line_size[4];
    //进行计算
    av_image_alloc(dst_data,dst_line_size,av_codec_context->width,av_codec_context->height,
                   AV_PIX_FMT_RGBA,1);

    //从视频流中读数据包，返回值小于0的时候表示读取完毕
    while (av_read_frame(av_format_context,av_packet) >= 0){
        //将取出的数据发送出来
        avcodec_send_packet(av_codec_context,av_packet);

        //接收发送出来的数据
        AVFrame *av_frame = av_frame_alloc();
        int av_receive_result = avcodec_receive_frame(av_codec_context,av_frame);

        //如果读取失败就重新读
        if(av_receive_result == AVERROR(EAGAIN)){
            continue;
        } else if(av_receive_result < 0){
            //如果到末尾了就结束循环读取
            break;
        }

        //将取出的数据放到之前定义的RGB目标容器中
        sws_scale(sws_context,av_frame->data,av_frame->linesize,0,av_frame->height,
                  dst_data,dst_line_size);

        //加锁然后进行渲染
        ANativeWindow_lock(a_native_window,&a_native_window_buffer,0);

        uint8_t *first_window = static_cast<uint8_t *>(a_native_window_buffer.bits);
        uint8_t *src_data = dst_data[0];

        //拿到每行有多少个RGBA字节
        int dst_stride = a_native_window_buffer.stride * 4;
        int src_line_size = dst_line_size[0];
        //循环遍历所得到的缓冲区数据
        for(int i = 0; i < a_native_window_buffer.height;i++){
            //内存拷贝进行渲染
            memcpy(first_window+i*dst_stride,src_data+i*src_line_size,dst_stride);
        }

        //绘制完解锁
        ANativeWindow_unlockAndPost(a_native_window);

        //40000微秒之后解析下一帧(这个是根据视频的帧率来设置的，我这播放的视频帧率是25帧/秒)
        usleep(1000 * 40);
        //释放资源
        av_frame_free(&av_frame);
        av_packet_free(&av_packet);
    }

    env->ReleaseStringUTFChars(path,video_path);
}
