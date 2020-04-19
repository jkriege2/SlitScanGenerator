#ifndef VIDEOPREVIEWREADERASYNC_H
#define VIDEOPREVIEWREADERASYNC_H

#include <QThread>
#include <QProgressDialog>
#include "ffmpeg_tools.h"
#include <string>
#include <CImg.h>

/** \brief wrapper around readFFMPEGAsImageStack() from ffmpeg_tools.h, which
 *         executes the function inside a QThread and reports to a QProgressDialog \a progress */
class VideoPreviewReaderThread : public QThread
{
    Q_OBJECT
public:
    VideoPreviewReaderThread(cimg_library::CImg<uint8_t> &video, const std::string& filename, int everyNthFrame, double xyscale, std::string* error, int maxFrame, QProgressDialog* progress, QObject *parent = nullptr);

    bool exec();

signals:
    void setProgressValue(int value);
    void setProgressMaximum(int value);
    void setProgressLabelText(QString value);
protected:
    virtual void run() override;
    bool updateProgress(int value, int max);
protected slots:
    void setCanceled();
private:
    cimg_library::CImg<uint8_t> &m_video;
    std::string m_filename;
    int m_everyNthFrame;
    double m_xyscale;
    std::string *m_error;
    int m_maxFrame;
    std::atomic<bool> m_wasCanceled;
    std::atomic<bool> m_result;
};

#endif // VIDEOPREVIEWREADERASYNC_H
