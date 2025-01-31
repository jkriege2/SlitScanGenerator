#include "videopreviewreaderthread.h"
#include "slitscangeneratorsettings.h"
#include <QApplication>

VideoPreviewReaderThread::VideoPreviewReaderThread(cimg_library::CImg<uint8_t> &video, const std::string &filename, int everyNthFrame, double xyscale, std::string *error, int maxFrame, QProgressDialog *progress, QObject *parent):
    QThread(parent),
    m_video(video), m_filename(filename), m_everyNthFrame(everyNthFrame), m_xyscale(xyscale), m_error(error), m_maxFrame(maxFrame), m_wasCanceled(false), m_result(false)
{
    connect(this, &VideoPreviewReaderThread::setProgressValue, progress, &QProgressDialog::setValue);
    connect(this, &VideoPreviewReaderThread::setProgressMaximum, progress, &QProgressDialog::setMaximum);
    connect(this, &VideoPreviewReaderThread::setProgressLabelText, progress, &QProgressDialog::setLabelText);
    connect(progress, &QProgressDialog::canceled, this, &VideoPreviewReaderThread::setCanceled);
}

bool VideoPreviewReaderThread::exec()
{
    start();
    while (isRunning()) {
        QApplication::processEvents();
    }
    return m_result.load();
}

void VideoPreviewReaderThread::run()
{
    SlitScanGeneratorSettings settings;
    auto progCB=std::bind(std::mem_fn(&VideoPreviewReaderThread::updateProgress), this, std::placeholders::_1, std::placeholders::_2);
    m_result=readFFMPEGAsImageStack(m_video, m_filename, m_everyNthFrame, m_xyscale, m_error, progCB, m_maxFrame, settings.value("ffmpeg_accel", "auto").toString(), settings.value("ffmpeg_threads", QThread::idealThreadCount()/2).toInt());
}

void VideoPreviewReaderThread::setCanceled()
{
    m_wasCanceled=true;
}

bool VideoPreviewReaderThread::updateProgress(int frame, int maxi)
{
    if (frame%5==0 || frame%7==0 || frame%3==0 || frame%4==0) {
        if (maxi>1) {
            emit setProgressValue(frame);
            emit setProgressMaximum(maxi+1);
            emit setProgressLabelText(QObject::tr("Reading frame %1/%2...").arg(frame).arg(maxi));
        } else {
            emit setProgressValue(1);
            emit setProgressMaximum(2);
            emit setProgressLabelText(QObject::tr("Reading frame %1...").arg(frame));
        }
    }
    return m_wasCanceled.load();
}
