#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <QDialog>
#include "cimg_tools.h"

class ImageViewer; // forward
class QSettings;

namespace Ui {
    class ImportDialog;
}

class ImportDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit ImportDialog(QWidget *parent = 0, QSettings* settings=0);
        ~ImportDialog();

        bool openVideo(const QString &video, QString& ini);
        int getEveryNthFrame() const;
        double getXYScaleFactor() const;
        int getWidth() const;
        int getHeight() const;
        int getFrames() const;
        int getFramesHR() const;
    public slots:
        void on_spinHRFrames_valueChanged(int nth);
    protected slots:
        void on_spinXYFactor_valueChanged(double scale);
        void on_spinEveryNThFrame_valueChanged(int nth);
    private:
        Ui::ImportDialog *ui;
        ImageViewer* preview;
        QSettings* m_settings;
        int m_width;
        int m_height;
        int m_frames;
        int m_framesHR;
        cimg_library::CImg<uint8_t> frame;
};

#endif // IMPORTDIALOG_H
