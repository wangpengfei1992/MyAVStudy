#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include <zconf.h>
//混合C代码编译
extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_wpf_ffmpegdemo_FfmpegTools_stringFromJNI(JNIEnv *env, jobject thiz) {

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
Java_com_wpf_ffmpegdemo_FfmpegTools_native_1start_1play(JNIEnv *env, jobject thiz, jstring path, jobject surface) {
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
    AVCodec *av_codec = avcodec_find_decoder(av_codec_parameters->codec_id);

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
        av_free_packet(av_packet);
    }

    env->ReleaseStringUTFChars(path,video_path);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_wpf_ffmpegdemo_FfmpegTools_native_1start_1play2(JNIEnv *env, jobject thiz, jstring path_, jobject surface) {
    const char *path = env->GetStringUTFChars(path_, 0);
    // 初始化ffmpeg
    av_register_all();

    // 获取AVFormatContext ,这个类存储这文件的音频和视频流信息
    AVFormatContext *avFormatContext = avformat_alloc_context();

    // 打开一个输入流并读取到AVFormatContext中，如果路径不对或者其他情况会返回-1 .返回-1则表示读文件出错
    int code = avformat_open_input(&avFormatContext, path, NULL, NULL);
    if (code < 0) {
        return;
    }

    // 读取一个媒体文件的数据包以获取流信息  如果返回0则获取成功，-1则获取失败
    int findCode = avformat_find_stream_info(avFormatContext, NULL);
    if (findCode < 0) {
        return;
    }

    int video_stream_id = -1;
    // 遍历文件流，找到里面的视频流位置(因为文件可能有多个流（视频流，音频流）等等 )
    // nb_streams 代表流的个数
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        // AVMEDIA_TYPE_VIDEO 来自 AVMediaType ，里面定义了多个类型
        if (avFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_id = i;
            break;
        }
    }

    // Codec context associated with this stream. Allocated and freed by  libavformat.
    // 与此流相关的编解码上下文。由libavformat分配和释放。
    // 获取到编解码上下文。根据视频流id获取
    AVCodecContext *avCodecContext = avFormatContext->streams[video_stream_id]->codec;

    // Find a registered decoder with a matching codec ID.
    // 找到一个带有匹配的编解码器ID的注册解码器。
    // 获取解码器
    AVCodec *avCodec = avcodec_find_decoder(avCodecContext->codec_id);

    // 打开解码器
    // Initialize the AVCodecContext to use the given AVCodec. Prior to using this
    // function the context has to be allocated with avcodec_alloc_context3().
    // 初始化AVCodecContext以使用给定的AVCodec。之前使用这个
    // 必须使用avcodecalloc context3（）分配上下文。
    int codecOpenCode = avcodec_open2(avCodecContext, avCodec, NULL);
    if (codecOpenCode < 0) {
        return;
    }
    // 格式转换关键类
    // 首先这是mp4 如果需要解析成 yuv 需要用到 SwsContext
    // 构造函数传入的参数为 原视频的宽高、像素格式、目标的宽高这里也取原视频的宽高（可以修改参数）
    SwsContext *swsContext = sws_getContext(avCodecContext->width, avCodecContext->height,
                                            avCodecContext->pix_fmt,
                                            avCodecContext->width, avCodecContext->height,
                                            AV_PIX_FMT_RGBA, SWS_BICUBIC, NULL, NULL, NULL);
    // 解析的每一帧都是 AVPacket
    // 初始化AVPacket对象
    AVPacket *avPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    // （和c不一样的是，ffmpeg初始化参数的时候都有特定的方法去初始化）
    av_init_packet(avPacket);

    // 初始化AvFrame
    // This structure describes decoded (raw) audio or video data.
    // 这个结构描述了解码（原始的）音频或视频数据。
    // 这里需要将AvPacket(帧)里面的数据解析到AvFrame中去。（这里会在avcodec_decode_video2方法讲AvPacket的方法解析进去）
    AVFrame *srcFrame = av_frame_alloc();

    // 初始化 目标 Frame
    AVFrame *dstFrame = av_frame_alloc();
    // dstFrame分配内存
    u_int8_t *out_buffer = (u_int8_t *) av_malloc(
            avpicture_get_size(AV_PIX_FMT_RGBA, avCodecContext->width, avCodecContext->height));
    // 基于指定的图像参数设置图像字段
    // 以及所提供的图像数据缓冲区。
    avpicture_fill((AVPicture *) (dstFrame), out_buffer, AV_PIX_FMT_RGBA, avCodecContext->width,
                   avCodecContext->height);

    // 提供对本机窗口的访问
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    // 视频缓冲区
    ANativeWindow_Buffer outBuffer;
    // ANativeWindow

    int got_frame;
    // 将视频数据读取到avPacket（avPacket代表一帧，这里的while循环也是一帧一帧读）
    while (av_read_frame(avFormatContext, avPacket) >= 0) {

        //  Decode the video frame of size avpkt->size from avpkt->data into picture.
        //  Some decoders may support multiple frames in a single AVPacket,
        //  such decoders would then just decode the first frame.
        // 将avPacket中的数据解析到srcFrame中去
        avcodec_decode_video2(avCodecContext, srcFrame, &got_frame, avPacket);
        // got_frame got_picture_ptr Zero if no frame could be decompressed, otherwise, it is nonzero.
        // 如果没有帧可以解压，则为0，否则为非0。
        if (got_frame) {
            // 设置缓存区
            ANativeWindow_setBuffersGeometry(nativeWindow, avCodecContext->width,
                                             avCodecContext->height, WINDOW_FORMAT_RGBA_8888);
            ANativeWindow_lock(nativeWindow, &outBuffer, NULL);

            // 将h264的格式转化成rgb
            // 从srcFrame中的数据（h264）解析成rgb存放到dstFrame中去
            sws_scale(swsContext, (const uint8_t *const *) srcFrame->data, srcFrame->linesize, 0,
                      srcFrame->height, dstFrame->data,
                      dstFrame->linesize
            );

            // 一帧的具体字节大小
            uint8_t *dst = static_cast<uint8_t *>(outBuffer.bits);
            // 每一个像素的字节  ARGB 一共是四个字节
            int dstStride = outBuffer.stride * 4;

            // 像素数据的首地址
            uint8_t *src = dstFrame->data[0];
            int srcStride = dstFrame->linesize[0];

            // 将 dstFrame的数据 一行行复制到屏幕上去
            for (int i = 0; i < avCodecContext->height; ++i) {
                memcpy(dst + i * dstStride, src + i * srcStride, srcStride);
            }
            ANativeWindow_unlockAndPost(nativeWindow);

            // 16微秒之后解析下一帧，因为人的眼睛无法观测到这个变化
            usleep(1000 * 16);
        }
        // 绘制完成之后，回收一帧资源Packet
        av_free_packet(avPacket);
    }
    // 回收窗体
    ANativeWindow_release(nativeWindow);

    // 回收Frame
    av_frame_free(&srcFrame);
    av_frame_free(&dstFrame);
    // 关闭解码器资
    avcodec_close(avCodecContext);
    avformat_free_context(avFormatContext);

    env->ReleaseStringUTFChars(path_, path);
}

