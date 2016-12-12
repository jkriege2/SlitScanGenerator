#include "importdialog.h"
#include "ui_importdialog.h"
#include "imageviewer.h"
#include "ffmpeg_tools.h"
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QFileInfo>
#include <QDir>

ImportDialog::ImportDialog(QWidget *parent, QSettings* settings) :
    QDialog(parent),
    m_settings(settings),
    ui(new Ui::ImportDialog)
{
    ui->setupUi(this);
    ui->scrollArea->setWidget(preview=new ImageViewer(this));
    if (m_settings) {
        ui->spinEveryNThFrame->setValue(m_settings->value("lastNthFrame", 10).toInt());
        ui->spinXYFactor->setValue(m_settings->value("lastXYFactor", 4).toDouble());
        ui->spinHRFrames->setValue(m_settings->value("lastFramesHR", 50).toDouble());
    }
}

ImportDialog::~ImportDialog()
{
    if (m_settings) {
        m_settings->setValue("lastNthFrame",ui->spinEveryNThFrame->value());
        m_settings->setValue("lastXYFactor",ui->spinXYFactor->value());
        m_settings->setValue("lastFramesHR",ui->spinHRFrames->value());
    }
    delete ui;
}

bool ImportDialog::openVideo(const QString& video, QString& ini) {
    m_width=0,
    m_height=0;
    m_frames=0;
    ui->labName->setText(video);
    QFileInfo fi(video);
    ini=fi.absoluteDir().absoluteFilePath(fi.baseName()+".ini");
    if (QFile::exists(ini)) {
        ui->labINI->setText(tr("yes"));
    } else {
        ui->labINI->setText(tr("no"));
    }
    std::string error;
    FFMPEGVideo* vid=openFFMPEGVideo(video.toStdString(), &error);
    int z=0;

    if (vid && readFFMPEGFrame(frame, vid)) {
        ui->labFrameSize->setText(tr("%1x%2").arg(m_width=frame.width()).arg(m_height=frame.height()));
        ui->labFrameCount->setText(QString::number(m_frames=getFrameCount(vid)));
        closeFFMPEGVideo(vid);
        on_spinXYFactor_valueChanged(ui->spinXYFactor->value());
        on_spinHRFrames_valueChanged(ui->spinHRFrames->value());
        return true;
    } else {
        QMessageBox::critical(this, tr("Error opening video"), QString(error.c_str()));
        return false;
    }
}

int ImportDialog::getEveryNthFrame() const
{
    return ui->spinEveryNThFrame->value();
}

double ImportDialog::getXYScaleFactor() const
{
    return ui->spinXYFactor->value();
}

int ImportDialog::getWidth() const
{
    return m_width;
}

int ImportDialog::getHeight() const
{
    return m_height;
}

int ImportDialog::getFrames() const
{
    return m_frames;
}

int ImportDialog::getFramesHR() const
{
    return 0;
    //return m_framesHR;
}



void ImportDialog::on_spinXYFactor_valueChanged(double scale)
{
    cimg_library::CImg<uint8_t> f=frame;
    f.resize(frame.width()/scale, frame.height()/scale);
    preview->setPixmap(QPixmap::fromImage(CImgToQImage(f)));
    on_spinEveryNThFrame_valueChanged(ui->spinEveryNThFrame->value());
}

void ImportDialog::on_spinEveryNThFrame_valueChanged(int nth)
{
    ui->labMEMSize->setText(tr("%1 MBytes").arg(m_width/ui->spinXYFactor->value()*m_height/ui->spinXYFactor->value()*m_frames/ui->spinEveryNThFrame->value()*3/1024.0/1024.0, 0, 'g', 3));
}

void ImportDialog::on_spinHRFrames_valueChanged(int nth)
{
    m_framesHR=nth;
    ui->labMEMSizeHR->setText(tr("%1 MBytes").arg(m_width*m_height*m_framesHR*3/1024.0/1024.0, 0, 'g', 3));
}
