#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->scrollXY->setWidget(labXY=new QLabel(this));
    ui->scrollXZ->setWidget(labXZ=new QLabel(this));
    ui->scrollYZ->setWidget(labYZ=new QLabel(this));

    connect(ui->actQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actOpenVideo, SIGNAL(triggered()), this, SLOT(openVideo()));
    connect(ui->scrollXY->horizontalScrollBar(), SIGNAL(sliderMoved(int)), ui->scrollXZ->horizontalScrollBar(), SLOT(setValue(int)));
    connect(ui->scrollXZ->horizontalScrollBar(), SIGNAL(sliderMoved(int)), ui->scrollXY->horizontalScrollBar(), SLOT(setValue(int)));
    connect(ui->scrollXY->verticalScrollBar(), SIGNAL(sliderMoved(int)), ui->scrollYZ->verticalScrollBar(), SLOT(setValue(int)));
    connect(ui->scrollYZ->verticalScrollBar(), SIGNAL(sliderMoved(int)), ui->scrollXY->verticalScrollBar(), SLOT(setValue(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openVideo(const QString& filename) {
    QString fn=filename;
    if (fn.size()<=0) {
        fn=QFileDialog::getOpenFileName(this, tr("Open Video File ..."));
    }
    if (fn.size()>0) {
        //m_video.load_ffmpeg_external(filename.toLatin1().data(), 'z');
        QString error;
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        if (!openPreview(m_video_scaled, fn.toLatin1().data(), 10, 4, &error)) {
            QApplication::restoreOverrideCursor();
            QMessageBox::critical(this, tr("Error opening video"), error);
        } else {
            QApplication::restoreOverrideCursor();
            labXY->setPixmap(QPixmap::fromImage(CImgToQImage(m_video_scaled, 0)));

            cimg_library::CImg<uint8_t> img;
            img=cimg_library::CImg<uint8_t>(m_video_scaled.width(), m_video_scaled.depth(),1,3);
            cimg_forXZ( m_video_scaled, x, z )
            {
                img(x,z,0,0)=m_video_scaled(x,m_video_scaled.height()/2,z,0);
                img(x,z,0,1)=m_video_scaled(x,m_video_scaled.height()/2,z,1);
                img(x,z,0,2)=m_video_scaled(x,m_video_scaled.height()/2,z,2);
            }
            labXZ->setPixmap(QPixmap::fromImage(CImgToQImage(img, 0)));
            QMessageBox::information(this, tr("Video opened"), tr("Video: %1\nframe size: %2x%3\n frames: %4\n color channels: %5").arg(fn).arg(m_video_scaled.width()).arg(m_video_scaled.height()).arg(m_video_scaled.depth()).arg(m_video_scaled.spectrum()));
        }
    }
}

bool MainWindow::openPreview(cimg_library::CImg<uint8_t> &video, const char *filename, int everyNthFrame, int xyscale, QString* error)
{


    // see: https://github.com/mpenkov/ffmpeg-tutorial/blob/master/tutorial01.c
    AVFormatContext *pFormatCtx = NULL;
    int             i, videoStream;
    AVCodecContext  *pCodecCtx = NULL;
    AVCodec         *pCodec = NULL;
    AVFrame         *pFrame = NULL;
    AVFrame         *pFrameRGB = NULL;
    AVPacket        packet;
    int             frameFinished;
    int             numBytes;
    uint8_t         *buffer = NULL;

    AVDictionary    *optionsDict = NULL;
    struct SwsContext      *sws_ctx = NULL;
    video.clear();


    // Register all formats and codecs
    av_register_all();

    // Open video file
    if(avformat_open_input(&pFormatCtx, filename, NULL, NULL)!=0) {
        if (error) *error=tr("Could not open file %1").arg(filename);
        return false; // Couldn't open file
    }

    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0) {
        if (error) *error=tr("Could not find stream information, file %1").arg(filename);
        return false; // Couldn't find stream information
    }

    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, filename, 0);

    // Find the first video stream
    videoStream=-1;
    for(i=0; i<pFormatCtx->nb_streams; i++)
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
            videoStream=i;
            break;
        }
    if(videoStream==-1) {
        if (error) *error=tr("Didn't find a video stream, file %1").arg(filename);
        return false; // Didn't find a video stream
    }

    // Get a pointer to the codec context for the video stream
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;

    // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL) {
        fprintf(stderr, "Unsupported codec!\n");
        if (error) *error=tr("Codec not found, file %1").arg(filename);
        return false; // Codec not found
    }
    // Open codec
    if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0) {
        if (error) *error=tr("Could not open codec, file %1").arg(filename);
        return false; // Could not open codec
    }

    // Allocate video frame
    pFrame=av_frame_alloc();

    // Allocate an AVFrame structure
    pFrameRGB=av_frame_alloc();
    if(pFrameRGB==NULL) {
        if (error) *error=tr("Could not allocate frame, file %1").arg(filename);
        return false;
    }

    // Determine required buffer size and allocate buffer
    numBytes=avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
    buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

    sws_ctx = sws_getContext(
                pCodecCtx->width,
                pCodecCtx->height,
                pCodecCtx->pix_fmt,
                pCodecCtx->width,
                pCodecCtx->height,
                AV_PIX_FMT_RGB24,
                SWS_BILINEAR,
                NULL,
                NULL,
                NULL
                );

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24,pCodecCtx->width, pCodecCtx->height);

    // Read frames and save first five frames to disk
    i=0;
    while(av_read_frame(pFormatCtx, &packet)>=0) {

        // Is this a packet from the video stream?
        if(packet.stream_index==videoStream) {
            // Decode video frame
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,
                                  &packet);

            // Did we get a video frame?
            if(frameFinished) {
                // add frame to CImg
                if(i%everyNthFrame==0) {
                    // Convert the image from its native format to RGB
                    sws_scale(sws_ctx,
                              (uint8_t const * const *)pFrame->data,
                              pFrame->linesize,
                              0,
                              pCodecCtx->height,
                              pFrameRGB->data,
                              pFrameRGB->linesize
                              );

                    cimg_library::CImg<uint8_t> frame(pCodecCtx->width, pCodecCtx->height, 1,3);
                    for(int y=0; y<pCodecCtx->height; y++) {
                        const uint8_t* l=pFrameRGB->data[0]+y*pFrameRGB->linesize[0];
                        for (int x=0; x<pCodecCtx->width*3; x+=3) {
                            frame(x/3,y,0,0)=l[x+0];
                            frame(x/3,y,0,1)=l[x+1];
                            frame(x/3,y,0,2)=l[x+2];
                        }
                    }
                    frame.resize(pCodecCtx->width/xyscale, pCodecCtx->height/xyscale,1,3);
                    video.append(frame, 'z');
                }
            }
        }

        // Free the packet that was allocated by av_read_frame
        av_free_packet(&packet);
        i++;
    }

    // Free the RGB image
    av_free(buffer);
    av_free(pFrameRGB);

    // Free the YUV frame
    av_free(pFrame);

    // Close the codec
    avcodec_close(pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);

    return true;
}

QImage MainWindow::CImgToQImage(const cimg_library::CImg<uint8_t> &img, int z)
{
    QImage imgQt(img.width(), img.height(), QImage::Format_ARGB32);

    cimg_forXY( img, x, y )
    {
        const uint8_t R = img( x, y, z, 0 );
        const uint8_t G = img( x, y, z, 1 );
        const uint8_t B = img( x, y, z, 2 );
        imgQt.setPixel( x, y, qRgb(R,G,B) );
    }

    return imgQt;
}

