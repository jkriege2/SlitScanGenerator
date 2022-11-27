#include "ffmpeg_tools.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/mem.h>
}

#include <QObject>



bool readFFMPEGAsImageStack(cimg_library::CImg<uint8_t> &video, const std::string& filename, int everyNthFrame, double xyscale, std::string* error, std::function<bool(int, int)> frameCallback, int maxFrame)
{

    // see  http://dranger.com/ffmpeg/tutorial01.html
    AVFormatContext *pFormatCtx = NULL;
    int i;
    AVCodecContext *pCodecContext = NULL;
    AVCodecParameters *pCodecParameters=NULL;
    const AVCodec *pCodec = NULL;
    AVFrame *pFrame = NULL;
    AVFrame *pFrameRGB = NULL;
    uint8_t *buffer = NULL;
    int numBytes;
    struct SwsContext *sws_ctx = NULL;
    int frameFinished;
    AVPacket* pPacket;
    int videoStream;
    AVDictionary *optionsDict = NULL;

    video.clear();
    if (error) error->clear();

    pFormatCtx = avformat_alloc_context();

    // Open video file
    if(avformat_open_input(&pFormatCtx, filename.c_str(), NULL, NULL)!=0){
        if (error) *error="Could not open file";
        return false; // Couldn't open file
    }

    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0) {
        if (error) *error="Could not find stream information";
        return false; // Couldn't find stream information
    }

    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, filename.c_str(), 0);


    // Find the first video stream
    videoStream=-1;
    for(i=0; i<pFormatCtx->nb_streams; i++)
      if(pFormatCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO) {
        videoStream=i;
        pCodecParameters=pFormatCtx->streams[i]->codecpar;
        pCodec=avcodec_find_decoder(pCodecParameters->codec_id);
        break;
      }
    if(videoStream==-1) {
        if (error) *error="Didn't find a video stream";
        return false; // Didn't find a video stream
    }
    if(pCodec==NULL ||pCodecParameters==NULL) {
        if (error) *error="Didn't find a codec for the video stream";
        return false; // Didn't find a codec for the video stream
    }

    // Get a pointer to the codec context for the video stream
    pCodecContext=avcodec_alloc_context3(pCodec);

    // Fill the codec context based on the values from the supplied codec parameters
    // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
    if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0) {
      if (error) *error="failed to copy codec params to codec context";
      return false;
    }

    // get number of frames
    int nb_frames = pFormatCtx->streams[videoStream]->nb_frames;



    // Open codec
    if(avcodec_open2(pCodecContext, pCodec, &optionsDict)<0) {
    if (error) *error="Couldn't open codec";
      return false; // Could not open codec
    }


    // Allocate video frame
    pFrame=av_frame_alloc();

    // Allocate an AVFrame structure
    pFrameRGB=av_frame_alloc();
    if(pFrameRGB==NULL || pFrame==NULL){
        if (error) *error="Couldn't allocate frames";
        return false; // Could not open codec
    }

    pPacket = av_packet_alloc();
    if (!pPacket)
    {
        if (error) *error="failed to allocate memory for AVPacket";
        return false; // Could not open codec
    }
    // Determine required buffer size and allocate buffer
    numBytes=av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecContext->width,
                                pCodecContext->height, 1);
    buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    //avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24, pCodecContext->width, pCodecContext->height);
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24, pCodecContext->width, pCodecContext->height, 1);


    // initialize SWS context for software scaling
    sws_ctx = sws_getContext(pCodecContext->width,pCodecContext->height,pCodecContext->pix_fmt,pCodecContext->width,pCodecContext->height,AV_PIX_FMT_RGB24,SWS_BILINEAR,NULL,NULL,NULL);

    i=0;
    bool canceled=false;
    if (frameCallback) {
        canceled=frameCallback(0, nb_frames);
    }
    int ifc=0;

    while(!canceled && av_read_frame(pFormatCtx, pPacket)>=0) {
        // Is this a packet from the video stream?
        if(pPacket->stream_index==videoStream) {
            // Decode video frame
            //avcodec_decode_video2(pCodecContext, pFrame, &frameFinished, pPacket);
            avcodec_send_packet(pCodecContext,pPacket);
            frameFinished = avcodec_receive_frame(pCodecContext,pFrame) == 0? TRUE : FALSE;

            // Did we get a video frame?
            if(frameFinished) {
                ifc++;
                // add frame to CImg
                if(i%everyNthFrame==0) {
                    // Convert the image from its native format to RGB
                    sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data, pFrame->linesize, 0, pCodecContext->height, pFrameRGB->data, pFrameRGB->linesize);

                    cimg_library::CImg<uint8_t> frame(pCodecContext->width, pCodecContext->height, 1,3);
                    for(int y=0; y<pCodecContext->height; y++) {
                        const uint8_t* l=pFrameRGB->data[0]+y*pFrameRGB->linesize[0];
                        for (int x=0; x<pCodecContext->width*3; x+=3) {
                            frame(x/3,y,0,0)=l[x+0];
                            frame(x/3,y,0,1)=l[x+1];
                            frame(x/3,y,0,2)=l[x+2];
                        }
                    }
                    frame.resize(pCodecContext->width/xyscale, pCodecContext->height/xyscale,1,3);
                    video.append(frame, 'z');
                    if (frameCallback) {
                        if (frameCallback((nb_frames>0)?ifc:video.depth(), nb_frames)) {
                            canceled=true;
                        }
                    }

                }
            }
        }


        i++;
        if (maxFrame>0 && ifc>=maxFrame) {
            break;
        }
    }

    // Free the packet that was allocated by av_read_frame
    av_packet_free(&pPacket);

    // Free the RGB image
    av_free(buffer);
    av_frame_free(&pFrameRGB);

    // Free the YUV frame
    av_frame_free(&pFrame);

    // Close the codecs
    avcodec_close(pCodecContext);

    // Close the video file
    avformat_close_input(&pFormatCtx);

    return true;
}

void initFFMPEG()
{
    //av_register_all();
}


struct FFMPEGVideo {
    AVFormatContext *pFormatCtx ;
    int i;
    AVCodecContext *pCodecCtx ;
    AVCodecParameters *pCodecParameters=NULL;
    const AVCodec *pCodec ;
    AVFrame *pFrame ;
    AVFrame *pFrameRGB ;
    uint8_t *buffer ;
    int numBytes;
    struct SwsContext *sws_ctx ;
    int nb_frames;
    int frameFinished;
    AVPacket* pPacket;
    int videoStream;
    AVDictionary *optionsDict;
};

FFMPEGVideo *openFFMPEGVideo(const std::string &filename, std::string *error)
{
    FFMPEGVideo* res=(FFMPEGVideo*)malloc(sizeof(FFMPEGVideo));

    res->pFormatCtx = NULL;
    res->i=0;
    res->pCodecCtx = NULL;
    res->pCodecParameters=NULL;
    res->pCodec = NULL;
    res->pFrame = NULL;
    res->pFrameRGB = NULL;
    res->buffer = NULL;
    res->numBytes=0;
    res->sws_ctx = NULL;
    res->frameFinished=0;
    res->pPacket=NULL;
    res->videoStream=0;
    res->optionsDict=NULL;

    if (error) error->clear();

    res->pFormatCtx = avformat_alloc_context();

    // Open video file
    if(avformat_open_input(&res->pFormatCtx, filename.c_str(), NULL, NULL)!=0){
        if (error) *error="Could not open file";
        free(res);
        return nullptr;
    }

    // Retrieve stream information
    if(avformat_find_stream_info(res->pFormatCtx, NULL)<0) {
        if (error) *error="Could not find stream information";
        free(res);
        return nullptr;
    }

    // Dump information about file onto standard error
    av_dump_format(res->pFormatCtx, 0, filename.c_str(), 0);


    // Find the first video stream
    res->videoStream=-1;
    for(int i=0; i<res->pFormatCtx->nb_streams; i++)
      if(res->pFormatCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO) {
        res->videoStream=i;
        res->pCodecParameters=res->pFormatCtx->streams[i]->codecpar;
        res->pCodec=avcodec_find_decoder(res->pCodecParameters->codec_id);
        break;
      }
    if(res->videoStream==-1) {
        if (error) *error="Didn't find a video stream";
        free(res);
        return nullptr;
    }
    if(res->pCodec==NULL ||res->pCodecParameters==NULL) {
        if (error) *error="Didn't find a codec for the video stream";
        return nullptr; // Didn't find a codec for the video stream
    }
    // Get a pointer to the codec context for the video stream
    res->pCodecCtx=avcodec_alloc_context3(res->pCodec);


    // Fill the codec context based on the values from the supplied codec parameters
    // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
    if (avcodec_parameters_to_context(res->pCodecCtx, res->pCodecParameters) < 0) {
      if (error) *error="failed to copy codec params to codec context";
      return nullptr;
    }


    // Open codec
    if(avcodec_open2(res->pCodecCtx, res->pCodec, &res->optionsDict)<0) {
        if (error) *error="Couldn't open codec";
        free(res);
        return nullptr;
    }


    // Allocate video frame
    res->pFrame=av_frame_alloc();

    // Allocate an AVFrame structure
    res->pFrameRGB=av_frame_alloc();
    if(res->pFrameRGB==NULL || res->pFrame==NULL){
        if (error) *error="Couldn't allocate frames";
        free(res);
        return nullptr;
    }

    res->pPacket = av_packet_alloc();
    if (!res->pPacket)
    {
        if (error) *error="failed to allocate memory for AVPacket";
        return nullptr; // Could not open codec
    }
    // Determine required buffer size and allocate buffer
    res->numBytes=av_image_get_buffer_size(AV_PIX_FMT_RGB24, res->pCodecCtx->width,
                                res->pCodecCtx->height, 1);
    res->buffer=(uint8_t *)av_malloc(res->numBytes*sizeof(uint8_t));

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    //avpicture_fill((AVPicture *)res->pFrameRGB, res->buffer, AV_PIX_FMT_RGB24, res->pCodecCtx->width, res->pCodecCtx->height);
    av_image_fill_arrays(res->pFrameRGB->data, res->pFrameRGB->linesize, res->buffer, AV_PIX_FMT_RGB24, res->pCodecCtx->width, res->pCodecCtx->height, 1);


    // initialize SWS context for software scaling
    res->sws_ctx = sws_getContext(res->pCodecCtx->width,res->pCodecCtx->height,res->pCodecCtx->pix_fmt,res->pCodecCtx->width,res->pCodecCtx->height,AV_PIX_FMT_RGB24,SWS_BILINEAR,NULL,NULL,NULL);

    res->i=0;

    res->nb_frames =res-> pFormatCtx->streams[res->videoStream]->nb_frames;

    return res;
}

bool readFFMPEGFrame(cimg_library::CImg<uint8_t>& frame, FFMPEGVideo *video)
{
    if (!video) return false;
    bool done=false;
    if (!video->pPacket) video->pPacket = av_packet_alloc();
    while (!done && (av_read_frame(video->pFormatCtx, video->pPacket)>=0)) {
        // Is this a packet from the video stream?
        if(video->pPacket->stream_index==video->videoStream) {
            // Decode video frame
            //avcodec_decode_video2(video->pCodecCtx, video->pFrame, &video->frameFinished, &video->packet);
            avcodec_send_packet(video->pCodecCtx,video->pPacket);
            video->frameFinished = avcodec_receive_frame(video->pCodecCtx,video->pFrame) == 0? TRUE : FALSE;

            // Did we get a video frame?
            if(video->frameFinished) {
                done=(video->pCodecCtx->width*video->pCodecCtx->height)>0;
                // add frame to CImg
                // Convert the image from its native format to RGB
                sws_scale(video->sws_ctx, (uint8_t const * const *)video->pFrame->data, video->pFrame->linesize, 0, video->pCodecCtx->height, video->pFrameRGB->data, video->pFrameRGB->linesize);

                frame.assign(video->pCodecCtx->width, video->pCodecCtx->height, 1,3);
                for(int y=0; y<video->pCodecCtx->height; y++) {
                    const uint8_t* l=video->pFrameRGB->data[0]+y*video->pFrameRGB->linesize[0];
                    for (int x=0; x<video->pCodecCtx->width*3; x+=3) {
                        frame(x/3,y,0,0)=l[x+0];
                        frame(x/3,y,0,1)=l[x+1];
                        frame(x/3,y,0,2)=l[x+2];
                    }
                }
            }
        }
        if (done) break;
    }

    // Free the packet that was allocated by av_read_frame
    av_packet_free(&(video->pPacket));

    video->i++;

    return done;
}

void closeFFMPEGVideo(FFMPEGVideo *video)
{
    if (video) {

        // Free the RGB image
        av_free(video->buffer);
        av_frame_free(&(video->pFrameRGB));

        // Free the YUV frame
        av_frame_free(&(video->pFrame));

        // Close the codecs
        avcodec_close(video->pCodecCtx);

        // Close the video file
        avformat_close_input(&(video->pFormatCtx));

        free(video);
    }
}

int getFrameCount(const FFMPEGVideo *video)
{
    return video->nb_frames;
}
