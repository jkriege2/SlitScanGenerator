#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <cstdint>
#include <QLabel>
#include <QSettings>
#include "imageviewer.h"
#include "processingparametertable.h"
#include "cimg_tools.h"
#include "ffmpeg_tools.h"


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
        void saveINI();
        void loadINI();
        void openVideo(const QString &filename=QString());
        void recalcCuts(int x, int y);

        void on_btnAddXZ_clicked();
        void on_btnAddZY_clicked();
        void on_btnDelete_clicked();
        void processAll();
        void tableRowClicked(const QModelIndex &index);
        void setButtonsEnabled();
        void showAbout();
    private:
        Ui::MainWindow *ui;

        ImageViewer* labXY;
        QLabel* labXZ;
        QLabel* labYZ;
    protected:
        /** \brief internal video, scaled version */
        cimg_library::CImg<uint8_t> m_video_scaled;
        QString m_filename;


        /** \brief path to the ffmpeg utility */
        QString m_ffmpegPath;

        ProcessingParameterTable* m_procModel;

        int lastX;
        int lastY;

        int video_everyNthFrame;
        double video_xyFactor;
        QSettings m_settings;

};

#endif // MAINWINDOW_H
