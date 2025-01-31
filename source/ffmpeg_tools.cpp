#include "ffmpeg_tools.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/mem.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/hwcontext.h>
}

#include <QObject>
#include <memory>


// Custom deleter for AVFrame
struct AVFrameDeleter {
    void operator()(AVFrame* frame) const {
        if (frame) {
            av_frame_free(&frame);
        }
    }
};

// Custom deleter for a frame
struct AVFreeDeleter {
    void operator()(void* frame) const {
        if (frame) {
            av_free(frame);
        }
    }
};

// Custom deleter for AVCodecContext
struct AVCodecContextDeleter {
    void operator()(AVCodecContext* frame) const {
        if (frame) {
            avcodec_free_context(&frame);
        }
    }
};

// Custom deleter for AVCodecContext
struct AVFormatContextDeleter {
    void operator()(AVFormatContext* frame) const {
        if (frame) {
            avformat_close_input(&frame);
        }
    }
};

// Custom deleter for AVPacket
struct AVPacketDeleter {
    void operator()(AVPacket* frame) const {
        if (frame) {
            av_packet_free(&frame);
        }
    }
};

// Custom deleter for AVPacket
struct AVCodecParameterDeleter {
    void operator()(AVCodecParameters* frame) const {
        if (frame) {
            avcodec_parameters_free(&frame);
        }
    }
};
QStringList listFFMPEGHWAccelOptions() {
    QStringList res;
    res<<"auto";
    AVHWDeviceType type=AV_HWDEVICE_TYPE_NONE;
    for (;;) {
        res << av_hwdevice_get_type_name(type);
        type = av_hwdevice_iterate_types(type);
        if (type == AV_HWDEVICE_TYPE_NONE) {
            break;
        }
    }
    return res;
}

void printFFMPEGDictionary(AVDictionary* dict) {
    AVDictionaryEntry *entry = nullptr;
    while ((entry = av_dict_get(dict, "", entry, AV_DICT_IGNORE_SUFFIX))) {
        qDebug() << "FFMPEG option: "<<entry->key << "= '" << entry->value<<"'";
    }
}

bool readFFMPEGAsImageStack(cimg_library::CImg<uint8_t> &video, const std::string& filename, int everyNthFrame, double xyscale, std::string* error, std::function<bool(int, int)> frameCallback, int maxFrame, QString hw_accel_opt, int numFFMPEGThreads)
{
    AVFormatContext* pFormatCtx = nullptr;
    int i;
    std::unique_ptr<AVCodecContext, AVCodecContextDeleter> pCodecContext = nullptr;
    AVCodecParameters *pCodecParameters = nullptr;
    const AVCodec *pCodec = NULL;
    std::unique_ptr<AVFrame, AVFrameDeleter> pFrame = nullptr;
    std::unique_ptr<AVFrame, AVFrameDeleter> pFrameRGB = nullptr;
    std::unique_ptr<uint8_t, AVFreeDeleter> buffer = nullptr;
    int numBytes;
    struct SwsContext *sws_ctx = NULL;

    std::unique_ptr<AVPacket, AVPacketDeleter> pPacket=nullptr;
    int videoStream;
    AVDictionary *optionsDict = NULL;

    video.clear();

    if (error) error->clear();

    pFormatCtx = avformat_alloc_context();

    // Open video file
    if (avformat_open_input(&pFormatCtx, filename.c_str(), NULL, NULL) != 0) {
        if (error) *error = "Could not open file";
        return false; // Couldn't open file
    }

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        if (error) *error = "Could not find stream information";
        return false; // Couldn't find stream information
    }

    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, filename.c_str(), 0);

    // Find the first video stream
    videoStream = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            pCodecParameters = pFormatCtx->streams[i]->codecpar;
            pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
            break;
        }
    if (videoStream == -1) {
        if (error) *error = "Didn't find a video stream";
        return false; // Didn't find a video stream
    }
    if (pCodec == NULL || pCodecParameters == NULL) {
        if (error) *error = "Didn't find a codec for the video stream";
        return false; // Didn't find a codec for the video stream
    }

    // Get a pointer to the codec context for the video stream
    pCodecContext.reset(avcodec_alloc_context3(pCodec));

    // Fill the codec context based on the values from the supplied codec parameters
    if (avcodec_parameters_to_context(pCodecContext.get(), pCodecParameters) < 0) {
        if (error) *error = "Failed to copy codec params to codec context";
        return false;
    }

    // Get number of frames
    const int nb_frames = pFormatCtx->streams[videoStream]->nb_frames;
    const int finalFrameCnt=(maxFrame>0)?maxFrame:(nb_frames/everyNthFrame);

    // set options
    if (numFFMPEGThreads<=0) {
        if (av_dict_set(&optionsDict, "threads", "auto", 0)<0) {
            qDebug()<<"error setting option 'threads=auto'";
        }
    } else {
        if (av_dict_set(&optionsDict, "threads", std::to_string(numFFMPEGThreads).c_str(), 0)<0) {
            qDebug()<<"error setting option 'threads="<<numFFMPEGThreads<<"'";
        }
    }
    if (av_dict_set(&optionsDict, "hwaccel", hw_accel_opt.toLatin1().data(), 0)<0) {
        qDebug()<<"error setting option 'hwaccel="<<hw_accel_opt<<"'";
    }
    if (av_dict_set(&optionsDict, "hwaccel_output_format", hw_accel_opt.toLatin1().data(), 0)<0) {
        qDebug()<<"error setting option 'hwaccel_output_format="<<hw_accel_opt<<"'";
    }

    printFFMPEGDictionary(optionsDict);

    // Open codec
    if (avcodec_open2(pCodecContext.get(), pCodec, &optionsDict) < 0) {
        if (error) *error = "Couldn't open codec";
        return false; // Could not open codec
    }

    // Allocate video frame
    pFrame.reset(av_frame_alloc());

    // Allocate an AVFrame structure
    pFrameRGB.reset(av_frame_alloc());
    if (!pFrameRGB  || !pFrame) {
        if (error) *error = "Couldn't allocate frames";
        return false; // Could not open codec
    }

    pPacket.reset(av_packet_alloc());
    if (!pPacket) {
        if (error) *error = "Failed to allocate memory for AVPacket";
        return false; // Could not open codec
    }

    // Determine required buffer size and allocate buffer
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecContext->width, pCodecContext->height, 1);
    buffer.reset((uint8_t *)av_malloc(numBytes * sizeof(uint8_t)));

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer.get(), AV_PIX_FMT_RGB24, pCodecContext->width, pCodecContext->height, 1);

    // Initialize SWS context for software scaling
    sws_ctx = sws_getContext(pCodecContext->width, pCodecContext->height, pCodecContext->pix_fmt, pCodecContext->width, pCodecContext->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);

    i = 0;
    bool canceled = false;
    if (frameCallback) {
        canceled = frameCallback(0, nb_frames);
    }
    int ifc = 0;
    int decoded=0;
    int disposable=0;
    int keyFrames=0;
    int discardFrames=0;
    int corruptFrames=0;
    cimg_library::CImg<uint8_t> frame, frameResized;
    while (!canceled && av_read_frame(pFormatCtx, pPacket.get()) >= 0) {
        // Is this a packet from the video stream?
        if (pPacket->stream_index == videoStream) {
            // check whether we have to actually decode this frame, weither because it is an everyNthFrame
            // and we want to process it, or it is an I-frame (i.e. not disposable), so subsequent frames may deüend on this frame
            const bool frameToRead=i % everyNthFrame == 0;
            const bool isDisposablePacket=((pPacket->flags & AV_PKT_FLAG_DISPOSABLE) == AV_PKT_FLAG_DISPOSABLE);
            const bool isKeyPacket=((pPacket->flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY);
            const bool isDiscardPacket=((pPacket->flags & AV_PKT_FLAG_DISCARD) == AV_PKT_FLAG_DISCARD);
            const bool isCorruptPacket=((pPacket->flags & AV_PKT_FLAG_CORRUPT) == AV_PKT_FLAG_CORRUPT);
            if (isDisposablePacket) disposable++;
            if (isKeyPacket) keyFrames++;
            if (isDiscardPacket) discardFrames++;
            if (isCorruptPacket) corruptFrames++;
            if (frameToRead || (!isDisposablePacket)) {
                // Decode video frame
                if (avcodec_send_packet(pCodecContext.get(), pPacket.get())==0) {
                    const bool frameFinished = (avcodec_receive_frame(pCodecContext.get(), pFrame.get()) == 0);
                    decoded++;
                    // Did we get a video frame?
                    if (frameFinished && frameToRead) {
                        // Add frame to CImg
                        // Convert the image from its native format to RGB
                        sws_scale(sws_ctx, (uint8_t const *const *)pFrame->data, pFrame->linesize, 0, pCodecContext->height, pFrameRGB->data, pFrameRGB->linesize);

                        frame.resize(pCodecContext->width, pCodecContext->height, 1, 3);
                        for (int y = 0; y < pCodecContext->height; y++) {
                            const uint8_t *l = pFrameRGB->data[0] + y * pFrameRGB->linesize[0];
                            for (int x = 0; x < pCodecContext->width * 3; x += 3) {
                                frame(x / 3, y, 0, 0) = l[x + 0];
                                frame(x / 3, y, 0, 1) = l[x + 1];
                                frame(x / 3, y, 0, 2) = l[x + 2];
                            }
                        }
                        if (video.is_empty()) {
                            video.resize(pCodecContext->width / xyscale, pCodecContext->height / xyscale, finalFrameCnt, 3);
                            qDebug()<<"allocated " << (double(video.size())/1024.0/1024.0)<<"MBytes memory for video "<<int(pCodecContext->width / xyscale)<<"x"<< int(pCodecContext->height / xyscale)<<"x"<< finalFrameCnt<<"x"<< 3;
                        }
                        frameResized=frame.get_resize(pCodecContext->width / xyscale, pCodecContext->height / xyscale, 1, 3);

                        if (ifc<finalFrameCnt) {
                            auto s0=video.get_shared_slice(ifc,0);
                            s0.assign(frameResized.get_shared_channel(0));
                            auto s1=video.get_shared_slice(ifc,1);
                            s1.assign(frameResized.get_shared_channel(1));
                            auto s2=video.get_shared_slice(ifc,2);
                            s2.assign(frameResized.get_shared_channel(2));
                        }

                        //video.append(frame.get_resize(pCodecContext->width / xyscale, pCodecContext->height / xyscale, 1, 3), 'z');
                        if (frameCallback) {
                            if (frameCallback((nb_frames > 0) ? i : video.depth(), nb_frames)) {
                                canceled = true;
                            }
                        }
                        ifc++;
                    }
                }
            }
            i++;
        }

        av_packet_unref(pPacket.get());

        if (maxFrame > 0 && ifc >= maxFrame) {
            break;
        }
    }

    if (ifc<video.depth()) {
        video.crop(0,0,0,0,video.width()-1,video.height()-1,ifc-1,video.spectrum()-1);
    }

    qDebug()<<"i="<<i<<", decoded="<<decoded<<", ifc="<<ifc<<", nb_frames="<<nb_frames<<", disposable="<<disposable<<", keyFrames="<<keyFrames<<", discardFrames="<<discardFrames<<", corruptFrames="<<corruptFrames;

    // Free the packet that was allocated by av_read_frame
    pPacket.reset();
	// Free the RGB image
    buffer.reset();

    pFrameRGB.reset();

    // Free the YUV frame
    pFrame.reset();

    // Close the codecs
    pCodecContext.reset();

    // Close the video file
    avformat_close_input(&pFormatCtx);

    return true;
}

void initFFMPEG()
{
    // No need to call av_register_all() as it is deprecated and no longer required
}

struct FFMPEGVideo {
    AVFormatContext *pFormatCtx;
    int i;
    AVCodecContext *pCodecCtx;
    AVCodecParameters *pCodecParameters = NULL;
    const AVCodec *pCodec;
    AVFrame *pFrame;
    AVFrame *pFrameRGB;
    uint8_t *buffer;
    int numBytes;
    struct SwsContext *sws_ctx;
    int nb_frames;
    int frameFinished;
    AVPacket *pPacket;
    int videoStream;
    AVDictionary *optionsDict;
};

FFMPEGVideo *openFFMPEGVideo(const std::string &filename, std::string *error)
{
    FFMPEGVideo *res = (FFMPEGVideo *)malloc(sizeof(FFMPEGVideo));

    res->pFormatCtx = NULL;
    res->i = 0;
    res->pCodecCtx = NULL;
    res->pCodecParameters = NULL;
    res->pCodec = NULL;
    res->pFrame = NULL;
    res->pFrameRGB = NULL;
    res->buffer = NULL;
    res->numBytes = 0;
    res->sws_ctx = NULL;
    res->frameFinished = 0;
    res->pPacket = NULL;
    res->videoStream = 0;
    res->optionsDict = NULL;

    if (error) error->clear();

    res->pFormatCtx = avformat_alloc_context();

    // Open video file
    if (avformat_open_input(&res->pFormatCtx, filename.c_str(), NULL, NULL) != 0) {
        if (error) *error = "Could not open file";
        free(res);
        return nullptr;
    }

    // Retrieve stream information
    if (avformat_find_stream_info(res->pFormatCtx, NULL) < 0) {
        if (error) *error = "Could not find stream information";
        free(res);
        return nullptr;
    }

    // Dump information about file onto standard error
    av_dump_format(res->pFormatCtx, 0, filename.c_str(), 0);

    // Find the first video stream
    res->videoStream = -1;
    for (int i = 0; i < res->pFormatCtx->nb_streams; i++)
        if (res->pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            res->videoStream = i;
            res->pCodecParameters = res->pFormatCtx->streams[i]->codecpar;
            res->pCodec = avcodec_find_decoder(res->pCodecParameters->codec_id);
            break;
        }
    if (res->videoStream == -1) {
        if (error) *error = "Didn't find a video stream";
        free(res);
        return nullptr;
    }
    if (res->pCodec == NULL || res->pCodecParameters == NULL) {
        if (error) *error = "Didn't find a codec for the video stream";
        return nullptr; // Didn't find a codec for the video stream
    }
    // Get a pointer to the codec context for the video stream
    res->pCodecCtx = avcodec_alloc_context3(res->pCodec);

    // Fill the codec context based on the values from the supplied codec parameters
    if (avcodec_parameters_to_context(res->pCodecCtx, res->pCodecParameters) < 0) {
        if (error) *error = "Failed to copy codec params to codec context";
        return nullptr;
    }

    // Open codec
    if (avcodec_open2(res->pCodecCtx, res->pCodec, &res->optionsDict) < 0) {
        if (error) *error = "Couldn't open codec";
        free(res);
        return nullptr;
    }

    // Allocate video frame
    res->pFrame = av_frame_alloc();

    // Allocate an AVFrame structure
    res->pFrameRGB = av_frame_alloc();
    if (res->pFrameRGB == NULL || res->pFrame == NULL) {
        if (error) *error = "Couldn't allocate frames";
        free(res);
        return nullptr;
    }

    res->pPacket = av_packet_alloc();
    if (!res->pPacket) {
        if (error) *error = "Failed to allocate memory for AVPacket";
        return nullptr; // Could not open codec
    }
    // Determine required buffer size and allocate buffer
    res->numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, res->pCodecCtx->width, res->pCodecCtx->height, 1);
    res->buffer = (uint8_t *)av_malloc(res->numBytes * sizeof(uint8_t));

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    av_image_fill_arrays(res->pFrameRGB->data, res->pFrameRGB->linesize, res->buffer, AV_PIX_FMT_RGB24, res->pCodecCtx->width, res->pCodecCtx->height, 1);

    // Initialize SWS context for software scaling
    res->sws_ctx = sws_getContext(res->pCodecCtx->width, res->pCodecCtx->height, res->pCodecCtx->pix_fmt, res->pCodecCtx->width, res->pCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);

    res->i = 0;

    res->nb_frames = res->pFormatCtx->streams[res->videoStream]->nb_frames;

    return res;
}

bool readFFMPEGFrame(cimg_library::CImg<uint8_t>& frame, FFMPEGVideo *video)
{
    if (!video) return false;
    bool done = false;
    if (!video->pPacket) video->pPacket = av_packet_alloc();
    while (!done && (av_read_frame(video->pFormatCtx, video->pPacket) >= 0)) {
        // Is this a packet from the video stream?
        if (video->pPacket->stream_index == video->videoStream) {
            // Decode video frame
            avcodec_send_packet(video->pCodecCtx, video->pPacket);
            video->frameFinished = avcodec_receive_frame(video->pCodecCtx, video->pFrame) == 0 ? TRUE : FALSE;

            // Did we get a video frame?
            if (video->frameFinished) {
                done = (video->pCodecCtx->width * video->pCodecCtx->height) > 0;
                // Add frame to CImg
                // Convert the image from its native format to RGB
                sws_scale(video->sws_ctx, (uint8_t const *const *)video->pFrame->data, video->pFrame->linesize, 0, video->pCodecCtx->height, video->pFrameRGB->data, video->pFrameRGB->linesize);

                frame.assign(video->pCodecCtx->width, video->pCodecCtx->height, 1, 3);
                for (int y = 0; y < video->pCodecCtx->height; y++) {
                    const uint8_t *l = video->pFrameRGB->data[0] + y * video->pFrameRGB->linesize[0];
                    for (int x = 0; x < video->pCodecCtx->width * 3; x += 3) {
                        frame(x / 3, y, 0, 0) = l[x + 0];
                        frame(x / 3, y, 0, 1) = l[x + 1];
                        frame(x / 3, y, 0, 2) = l[x + 2];
                    }
                }
            }
        }
        av_packet_unref(video->pPacket);
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
        avcodec_free_context(&(video->pCodecCtx));

        // Close the video file
        avformat_close_input(&(video->pFormatCtx));

        free(video);
    }
}

int getFrameCount(const FFMPEGVideo *video)
{
    return video->nb_frames;
}
