#include <stdio.h>                  // standard lib
#include <libavcodec/avcodec.h>     // av codecs
#include <libavutil/imgutils.h>     // image codecs
#include <libavformat/avformat.h>   // av formats
#include <libswscale/swscale.h>     // sw scaling
#include <libavutil/log.h>          // logging
#include "save_frame.c"

void help_menu();


int main(int argc, char* argv[]){
    
    if(argc!=3){
        help_menu();
        return -1;
    }
    av_log_set_level(AV_LOG_QUIET); // Only show errors

    AVFormatContext *v_format_ctx;      // AV file context contains file's codec info

    if(avformat_open_input(&v_format_ctx,argv[1],NULL,NULL)!=0){           // dumps the header info to the ctx
        fprintf(stderr,"couldn't open file\n");
        return -1;
    }

    if(avformat_find_stream_info(v_format_ctx,NULL)){                      // reads the stream information
        fprintf(stderr,"error in getting codec info for %s\n",argv[1]);
        return -1;
    }

    av_dump_format(v_format_ctx,0,argv[1],0);                              // dump file info onto stderr[o/p]

    AVCodecContext *v_codec_ctx;

    int video_stream=-1;
    for(int i=0;i<v_format_ctx->nb_streams;i++){
        if(v_format_ctx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO){     // find the video stream
            video_stream=i;
        }
    }
    
    if(video_stream==-1){
        fprintf(stderr,"No video streams found\n");
        return -1;
    }

    AVCodec *v_codec;                                                       // actual codec data used 
    v_codec=avcodec_find_decoder(v_format_ctx->streams[video_stream]->codecpar->codec_id);
    
    if(v_codec==NULL){
        fprintf(stderr,"Codec not found !\n");
        return -1;
    }

    v_codec_ctx=avcodec_alloc_context3(v_codec);                            // populates the codec ctx with def values acc to the codec
    if(avcodec_parameters_to_context(v_codec_ctx,v_format_ctx->streams[video_stream]->codecpar)<0){     // fill the needed codec parameters
        fprintf(stderr,"codec copying error\n");
        return -1;
    }

    if(avcodec_open2(v_codec_ctx,v_codec,NULL)<0){                          // initialize the AVCodecContext to use the given AVCodec
        fprintf(stderr,"couldnt open codec\n");
        return -1;
    }

    AVFrame *v_frame;                                                       // allocate pict frame
    v_frame=av_frame_alloc();
    if(v_frame==NULL){
        fprintf(stderr,"couldnt allocate frame");
        return -1;
    }

    AVFrame *v_frame_jpeg;                                                  // pict frame [jpeg]
    v_frame_jpeg=av_frame_alloc();
    if(v_frame_jpeg==NULL){
        fprintf(stderr,"couldnt allocate frame");
        return -1;
    }

    // buffersize calcs
    uint8_t *buffer;
    int num_bytes;

    num_bytes=av_image_get_buffer_size(AV_PIX_FMT_RGB24,v_codec_ctx->width,v_codec_ctx->height,32);
    buffer=(uint8_t *)av_malloc(num_bytes*sizeof(uint8_t));

    // fill the data pts in an image
    av_image_fill_arrays(v_frame_jpeg->data,v_frame_jpeg->linesize,buffer,AV_PIX_FMT_RGB24,v_codec_ctx->width,v_codec_ctx->height,32);                                                 
    
    // read from stream now ---

    // SW scaling 

    struct SwsContext * sws_ctx = NULL;
    sws_ctx=sws_getContext(v_codec_ctx->width,v_codec_ctx->height,v_codec_ctx->pix_fmt,v_codec_ctx->width,v_codec_ctx->height,AV_PIX_FMT_RGB24,SWS_BILINEAR,NULL,NULL,NULL);
    AVPacket *v_packet=av_packet_alloc();                                   // allocate an AVPacket and set its fields to default values
    if(v_packet==NULL){
        fprintf(stderr,"packet alloc error\n");
        return -1;
    }

    double frame_rate=(double)av_q2d(v_format_ctx->streams[video_stream]->avg_frame_rate);
    int total_frames = frame_rate*av_q2d(v_format_ctx->streams[video_stream]->time_base)*v_format_ctx->duration;
    int max_frames;
    // if(memcmp(argv[1],"all")==0){
    //     max_frames=total_frames;
    // }
    // else{
        
    // }

    strcmp(argv[2],"all")==0? max_frames= total_frames:sscanf(argv[2],"%d",&max_frames);;

    int i=0;
    while(av_read_frame(v_format_ctx,v_packet)>=0){                                                  // returns next frame of scene
        if(v_packet->stream_index==video_stream){                                                    // check if from vid stream
            if(avcodec_send_packet(v_codec_ctx,v_packet)<0){                                         // supply raw packet data as input to a decoder
                fprintf(stderr,"packet error\n");
                return -1;
            }

            int ret=0;
            while(ret>=0){
                ret=avcodec_receive_frame(v_codec_ctx,v_frame);
                // convert the image from native fmt to rgb
                sws_scale(sws_ctx,(uint8_t const * const *)v_frame->data,v_frame->linesize,0,v_codec_ctx->height,v_frame_jpeg->data,v_frame_jpeg->linesize);
                
                if(++i<max_frames){
                    save_frame(v_frame_jpeg,v_codec_ctx->width,v_codec_ctx->height,i);
                    // log info
                    printf("Frame index: %d, average frame rate: %.2f, resolution[%dx%d]\n",i,frame_rate,v_codec_ctx->width,v_codec_ctx->height);
                }
                else{
                    return 0;
                    break;
                }
            }
        }
        av_packet_unref(v_packet);
        
        


    }
    // cleanup
        
    av_free(buffer);
    av_frame_free(&v_frame_jpeg);
    av_free(v_frame_jpeg);
    
    av_frame_free(&v_frame);
    av_free(v_frame);
    
    // close codecs
    avcodec_close(v_codec_ctx);
    avformat_close_input(&v_format_ctx);

        return 0;
}

void help_menu(){
    printf("Enter the filename along with the num of frames to be saved, 'all' to save all frames\n");
}


