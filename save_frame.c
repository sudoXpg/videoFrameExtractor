#include <stdio.h>                  // standard lib
#include <libavcodec/avcodec.h>     // av codecs
#include <libavutil/imgutils.h>     // image codecs
#include <libavformat/avformat.h>   // av formats
#include <libswscale/swscale.h>     // sw scaling

void save_frame(AVFrame * fr, int width, int height, int index){


    AVCodec *jpeg_codec=avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    if(jpeg_codec==NULL){
        fprintf(stderr,"Jpeg codec not found\n");
        return ;
    }
    
    AVCodecContext *jpeg_codec_ctx=avcodec_alloc_context3(jpeg_codec);
    if(jpeg_codec_ctx==NULL){
        fprintf(stderr,"Jpeg codec error\n");
        return ;
    }

    jpeg_codec_ctx->width=width;
    jpeg_codec_ctx->height=height;
    jpeg_codec_ctx->pix_fmt=AV_PIX_FMT_YUVJ420P;
    jpeg_codec_ctx->time_base = (AVRational){1, 25};

    if(avcodec_open2(jpeg_codec_ctx,jpeg_codec,NULL)<0){
        fprintf(stderr,"jpeg codec error\n");
        return ;
    }

    AVFrame *jpeg_frame=av_frame_alloc();
    if(jpeg_frame==NULL){
        fprintf(stderr,"jpeg codec error\n");
        return;
    }

    jpeg_frame->width=width;
    jpeg_frame->height=height;
    jpeg_frame->format=jpeg_codec_ctx->pix_fmt;

    AVPacket *jpeg_packet=av_packet_alloc();
    if(jpeg_packet==NULL){
        fprintf(stderr,"jpeg packet error\n");
        return;
    }

    if(av_frame_get_buffer(jpeg_frame, 32)<0){
        fprintf(stderr, "Could not allocate JPEG image\n");
        av_packet_free(&jpeg_packet);
        av_frame_free(&jpeg_frame);
        avcodec_free_context(&jpeg_codec_ctx);
        return;
    }

    struct SwsContext * sws_ctx=NULL;
    sws_ctx=sws_getContext(width,height,AV_PIX_FMT_RGB24,width,height,AV_PIX_FMT_YUVJ420P,SWS_BILINEAR,NULL,NULL,NULL);    
    if(sws_ctx==NULL){
        fprintf(stderr, "Could not create SWScale context\n");
        av_packet_free(&jpeg_packet);
        av_frame_free(&jpeg_frame);
        avcodec_free_context(&jpeg_codec_ctx);
        return;
    }

    if (fr->data[0] != NULL) {
        sws_scale(sws_ctx, (const uint8_t * const *)fr->data, fr->linesize, 0, height, jpeg_frame->data, jpeg_frame->linesize);
    } else {
        fprintf(stderr, "Source frame data is not populated\n");
    }


    sws_scale(sws_ctx,(uint8_t const * const *)fr->data,fr->linesize,0,height,jpeg_frame->data,jpeg_frame->linesize);

    if(avcodec_send_frame(jpeg_codec_ctx,jpeg_frame)<0){
        fprintf(stderr, "Error sending JPEG frame\n");
        av_packet_free(&jpeg_packet);
        av_frame_free(&jpeg_frame);
        avcodec_free_context(&jpeg_codec_ctx);
        return;
    }

    int ret = avcodec_receive_packet(jpeg_codec_ctx, jpeg_packet);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        av_packet_unref(jpeg_packet);
        av_packet_free(&jpeg_packet);
        av_frame_free(&jpeg_frame);
        avcodec_free_context(&jpeg_codec_ctx);
        return;
    } else if (ret < 0) {
        fprintf(stderr, "Error receiving JPEG packet\n");
        av_packet_free(&jpeg_packet);
        av_frame_free(&jpeg_frame);
        avcodec_free_context(&jpeg_codec_ctx);
        return;
    }


    
    FILE *fp;
    char file_name[30];
    int i;
    snprintf(file_name,sizeof(file_name),"frame%d.jpeg",index);
    fp=fopen(file_name,"wb");
    if(fp==NULL){
        fprintf(stderr,"unable to create file\n");
        return;
    }
    fwrite(jpeg_packet->data,1,jpeg_packet->size,fp);
    fclose(fp);

    av_packet_unref(jpeg_packet);
    av_packet_free(&jpeg_packet);
    av_frame_free(&jpeg_frame);
    avcodec_free_context(&jpeg_codec_ctx);
    sws_freeContext(sws_ctx);

}
