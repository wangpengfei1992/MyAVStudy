// FfmpegFirst.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>
//
//int main()
//{
//    std::cout << "Hello World!\n";
//}


// libavcodec libavdevice  libavfilter libavformat libavutil libpostproc libswresample  libswscale
extern "C"
{
#include<libavcodec/avcodec.h>
#include<libavformat/avformat.h>
#include<libavutil/avutil.h>
#include<libavdevice/avdevice.h>
#include<libavfilter/avfilter.h>
#include<libswresample/swresample.h>
#include<libswscale/swscale.h>
}

int main()
{
    //当前Ffmpeg的版本5.0
    //av_register_all(); //过期
    //avcodec_register_all();//过期
    
   // printf("%s\n", avcodec_configuration()); //验证是否正常引用
   // avformat_network_init();

    //整体流程：创建编码器->编码器上下文->输出上下文->输出流->文件流->重采样->输出AAC->关闭清理对象

    int ret = 0;
    char inputfile[] = "audio.pcm";
    char outputfile[] = "audio.aac";

    //找到AAC编码器
    AVCodec* codec = const_cast<AVCodec*>(avcodec_find_encoder(AV_CODEC_ID_AAC));
    if (!codec) {
        std::cout << "avcodec_find_encoder failed" << std::endl;
        return -1;
    }
    //给AAC编码器配置上下文
    AVCodecContext* ac = avcodec_alloc_context3(codec);
    if (!ac) {
        std::cout << "avcodec_alloc_context3 failed" << std::endl;
        return -1;
    }
    //给编码器设置参数
    ac->sample_rate = 44100; //采样率
    ac->channels = 2; //通道数
    ac->sample_fmt = AV_SAMPLE_FMT_FLTP;//采样格式
    ac->bit_rate = 64000; //码率
    ac->channel_layout = AV_CH_LAYOUT_STEREO; //立体声
    //头部
    ac->flags = AV_CODEC_FLAG_GLOBAL_HEADER;

    //打开编码器
    ret = avcodec_open2(ac, codec, NULL);
    if (ret<0) {
        std::cout << "avcodec_open2 failed" << std::endl;
        return -1;
    }
    //创建一个输出上下文
    AVFormatContext* oc = NULL;
    ret = avformat_alloc_output_context2(&oc,NULL,NULL,outputfile);
    if (ret < 0) {
        std::cout << "avformat_alloc_output_context2 failed" << std::endl;
        return -1;
    }
    //创建一个输出流
    AVStream* st = avformat_new_stream(oc,NULL);
    st->codecpar->codec_tag = 0;
    avcodec_parameters_from_context(st->codecpar,ac);
    av_dump_format(oc,0,outputfile,1);

    //打开输出文件流
    ret = avio_open(&oc->pb,outputfile,AVIO_FLAG_WRITE);
    if (ret < 0) {
        std::cout << "avio_open failed" << std::endl;
        return -1;
    }
    avformat_write_header(oc, NULL);
    //重采样
    SwrContext* ctx = NULL;
    ctx = swr_alloc_set_opts(ctx, ac->channel_layout, ac->sample_fmt, ac->sample_rate,
        AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, 44100, 0, 0);
    if (!ctx) {
        std::cout << "swr_alloc_set_opts failed" << std::endl;
        return -1;
    }
    ret = swr_init(ctx); //重采样初始化
    if (ret < 0) {
        std::cout << "swr_init failed" << std::endl;
        return -1;
    }
    //读取数据重新编码
    //设置每一帧参数
    AVFrame* frame = av_frame_alloc();
    frame->format = AV_SAMPLE_FMT_FLTP;
    frame->channels = 2;
    frame->channel_layout = AV_CH_LAYOUT_STEREO;
    frame->nb_samples = 1024;//一帧音频的样本数量
    ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        std::cout << "av_frame_get_buffer" << std::endl;
        return -1;
    }
    int readSize = frame->nb_samples * 2 * 2; //每帧数量*声道数*位数
    char* pcms = new char[readSize];
    FILE* fp = fopen(inputfile, "rb");
    for (;;) {
        //循环读取
        int len = fread(pcms,1,readSize,fp);
        if (len <= 0) { //读完
            break;
        }
        //重采样
        const uint8_t* data[1];
        data[0] = (uint8_t*)pcms;
        len = swr_convert(ctx, frame->data,frame->nb_samples,data,frame->nb_samples);
        if (len <= 0) { //采样完成
            break;
        }

        //输出aac

        AVPacket pkt;  //封装数据
        av_init_packet(&pkt);

        //将重采样的数据发送到编码线程
        ret = avcodec_send_frame(ac,frame);
        if (ret != 0) {
            continue;
        }
        ret = avcodec_receive_packet(ac, &pkt);
        if (ret != 0) {
            continue;
        }
        //写入文件
        //0:音频流 1:视频流
        pkt.stream_index = 0;
        pkt.dts = 0;
        pkt.pts = 0;
        ret = av_interleaved_write_frame(oc,&pkt);
        if (ret != 0) {
            continue;
        }
        std::cout << len << std::endl;

    }

    //清除指针
    delete pcms;
    pcms = NULL;

    //写入视频索引，为了切换指定时间播放
    av_write_trailer(oc);


    avio_close(oc->pb);//关闭流
    avcodec_close(ac);//关闭编码器
    avcodec_free_context(&ac);//清理编码器参数
    avformat_free_context(oc);//清理输出上下文
    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
