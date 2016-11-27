#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <cstdint>
#include <QLabel>

#define cimg_display 0
#include "CImg.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/mem.h>
}

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

    protected slots:
        void openVideo(const QString &filename=QString());
    private:
        Ui::MainWindow *ui;

        QLabel* labXY;
        QLabel* labXZ;
        QLabel* labYZ;
    protected:
        /** \brief internal video, scaled version */
        cimg_library::CImg<uint8_t> m_video_scaled;

        bool openPreview(cimg_library::CImg<uint8_t>& video, const char* filename, int everyNthFrame, int xyscale, QString *error=nullptr);

        /** \brief path to the ffmpeg utility */
        QString m_ffmpegPath;

        static QImage CImgToQImage(const cimg_library::CImg<uint8_t>& img, int z=0);

};

#endif // MAINWINDOW_H
