#include <stdio.h>                  // standard lib
#include <sys/stat.h>               // for mkdir
#include <sys/types.h>              // for mkdir
#include <libavcodec/avcodec.h>     // av codecs
#include <libavutil/imgutils.h>     // image codecs
#include <libavformat/avformat.h>   // av formats
#include <libswscale/swscale.h>     // sw scaling
#include <libavutil/log.h>          // logging
#include<time.h>                    // time
#include "save_frame.c"

/*
TODO
  X  add the timeframe option
  X  help option
  X  custom output directory
  X  verbose mode
  X  less mode with process indication
  X  get file info as well(verbose only?)
  X  time taken

  ** change the arguement parsing to getopt!!

*/
clock_t start_time,end_time;
char locn[512];
int verbose=0;
int timeframe[] = {0,0};
int frames_to_process[] = {0,INT_MAX};

void options(char *s, char *s1, char*s2);
void help_option();
void custom_directory(char *s);
void custom_frames();

int main(int argc, char* argv[]){
    av_log_set_level(AV_LOG_QUIET); // Only show errors
    
    if (argc > 5) {
        options(argv[3], argv[4], argv[5]);
    } else if (argc > 4) {
        options(argv[3], argv[4], "NULL");
    } else if (argc > 3) {
        options(argv[3], "NULL", "NULL");
    }


    if(argc>1 && strcmp(argv[1],"--h")==0){
        help_option();
    }

    if(argc<3){
        help_option();
        return -1;
    }
    
    //printf("%d\n\n,size = %ld\n\n",(locn[0]=='\0'),sizeof(locn));
    


    AVFormatContext *v_format_ctx;      // AV file context contains file's codec info
    
    if(avformat_open_input(&v_format_ctx,argv[1],NULL,NULL)<0){           // dumps the header info to the ctx
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
    if(timeframe[1]!=0){
        frames_to_process[0]=timeframe[0]*frame_rate;
        frames_to_process[1]=timeframe[1]*frame_rate;
    }
    //printf("frame rate :%f\n",frame_rate);
    int total_frames = frame_rate * (double) v_format_ctx->duration/ AV_TIME_BASE;
    //printf("total frames : %d\n",total_frames);
    int max_frames;
    // if(memcmp(argv[1],"all")==0){
    //     max_frames=total_frames;
    // }
    // else{
        
    // }

    strcmp(argv[2],"all")==0? max_frames= total_frames:sscanf(argv[2],"%d",&max_frames);;
    float percent_completed;
    int i=0;
    int k=0;
    start_time=clock();
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
                
                if(++i<=max_frames){
                    if(i<frames_to_process[0]){
                        
                        if(k==1){
                            printf(".");
                        }
                        if(k==0){
                            printf("buffering");
                            k=1;
                        }
                        continue;
                    }
                    
                    save_frame(locn,v_frame_jpeg,v_codec_ctx->width,v_codec_ctx->height,i);
                    // log info
                    printf("Frame index: %d, average frame rate: %.2f, resolution[%dx%d]\n",i,frame_rate,v_codec_ctx->width,v_codec_ctx->height);
                }
                else{
                    end_time = clock();
                    // cleanup
                    printf("\r\n");
                    printf("Completed in %fsec\n",(double)(end_time-start_time)/CLOCKS_PER_SEC);
                    fflush(stdout);
                    exit(0);
                }
                
                if(i>frames_to_process[1]){
                    end_time = clock();
                    // cleanup
                    printf("\r\n");
                    printf("Completed in %fsec\n",(double)(end_time-start_time)/CLOCKS_PER_SEC);
                    fflush(stdout);
                    exit(0);
                }
            }
        }
        av_packet_unref(v_packet);
        


    }
    end_time = clock();
    // cleanup
    printf("\r\n");
    printf("Completed in %fsec\n",(double)(end_time-start_time)/CLOCKS_PER_SEC);
    fflush(stdout);
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


void help_option() {
    printf("\033[2J"); // Clear the screen
    printf("\033[H");  // Move cursor to top left
    printf(">>    VFE v1 ~ Video Frame Extractor\n");
    printf("---------------------------------------------------------\n");
    printf("Usage:\n");
    printf("  vfe <input_file> <max_frames|all> [options]\n");
    printf("\n");
    printf("Arguments:\n");
    printf("  <input_file>        Path to the input video file.\n");
    printf("  <num_frames|all>    Specify the number of frames to extract or use 'all' to extract all frames.\n");
    printf("\n");
    printf("Options:\n");
    printf("  --frames                     Extract frames between the specified start and end time\n");
    printf("  --dir <dir>                  Specify a directory to save the extracted frames. Will create the directory if it doesn't exist.\n");
    printf("  --verbose                    Enable verbose output for detailed information during the extraction process.\n");
    printf("  --h                          Show this help message.\n");
    printf("  --frames                     Enter the start and end duration of frames to be shown.\n                               << keep max_frames as all >>\n");
    printf("\n");
    printf("Example Usage:\n");
    printf("  vfe input.mp4 all                           # Extract all frames from a video file.\n");
    printf("  vfe input.mp4 10                            # Extract the first 10 frames from a video file.\n");
    printf("  vfe input.mp4 all --timeframe 10 20         # Extract frames from 10 to 20 seconds of the video.\n");
    printf("  vfe input.mp4 all --dir ./frames            # Specify a directory to save the extracted frames.\n");
    printf("---------------------------------------------------------\n");
    printf("This program utilizes FFmpeg libraries for efficient video processing.\n");
}


void options(char *s, char *s1,char *s2){
    if(strcmp(s,"--verbose")==0 || strcmp(s1,"--verbose")==0 || strcmp(s2,"--verbose")==0){
        av_log_set_level(AV_LOG_TRACE);
        verbose=1;
    }
    
    if(strcmp(s,"--dir")==0 || strcmp(s1,"--dir")==0 || strcmp(s2,"--dir")==0){
        char dir_name[1024];
        printf("Enter a directory name \n");
        scanf("%s",dir_name);
        custom_directory(dir_name);
    }

    if(strcmp(s,"--frames")==0 || strcmp(s1,"--frames")==0 || strcmp(s2,"--frames")==0){
        custom_frames();
    }
    return;
    
}

void custom_directory(char *s){
    strcat(locn,s);
    struct stat statbuf;
    if (stat(locn, &statbuf) == 0) {
        printf("Folder already exists\n");
        return; 
    }
    if(mkdir(locn,0777)!=0){
        fprintf(stderr,"Unable to create directory\n");
        return;
    }
}

void custom_frames(){
    int hr0, m0, s0, hr1, m1, s1;
    printf("Enter start duration (HH:MM:SS) : ");
    scanf("%d:%d:%d", &hr0, &m0, &s0);  
    timeframe[0] = hr0 * 3600 + m0 * 60 + s0;              


    printf("Enter end duration (HH:MM:SS) : ");
    scanf("%d:%d:%d", &hr1, &m1, &s1);  
    timeframe[1] = hr1 * 3600 + m1 * 60 + s1;
    
    if(timeframe[1]-timeframe[0]<0){
        fprintf(stderr,"End time must be greater than start time !\n");
        custom_frames();
    }
}