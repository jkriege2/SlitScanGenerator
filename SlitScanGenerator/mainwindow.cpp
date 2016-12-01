#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <functional>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QPainter>
#include <QScrollBar>
#include <QProgressDialog>
#include <QFileInfo>
#include <QSettings>
#include <vector>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    lastX(-1),
    lastY(-1)
{
    initFFMPEG();
    ui->setupUi(this);
    ui->scrollXY->setWidget(labXY=new ImageViewer(this));
    ui->scrollXZ->setWidget(labXZ=new QLabel(this));
    ui->scrollYZ->setWidget(labYZ=new QLabel(this));
    ui->table->setModel(m_procModel=new ProcessingParameterTable(ui->table));


    connect(ui->actQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actOpenVideo, SIGNAL(triggered()), this, SLOT(openVideo()));
    connect(ui->scrollXY->horizontalScrollBar(), SIGNAL(sliderMoved(int)), ui->scrollXZ->horizontalScrollBar(), SLOT(setValue(int)));
    connect(ui->scrollXZ->horizontalScrollBar(), SIGNAL(sliderMoved(int)), ui->scrollXY->horizontalScrollBar(), SLOT(setValue(int)));
    connect(ui->scrollXY->verticalScrollBar(), SIGNAL(sliderMoved(int)), ui->scrollYZ->verticalScrollBar(), SLOT(setValue(int)));
    connect(ui->scrollYZ->verticalScrollBar(), SIGNAL(sliderMoved(int)), ui->scrollXY->verticalScrollBar(), SLOT(setValue(int)));
    connect(labXY, SIGNAL(mouseClicked(int,int)), this, SLOT(recalcCuts(int,int)));
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
        std::string error;
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        QProgressDialog progress(tr("Opening Video"), "", 0, 2, this);
        progress.setLabelText(tr("opening file '%1'...").arg(fn));
        progress.setMinimumDuration(0);
        QApplication::processEvents();
        progress.setWindowModality(Qt::WindowModal);
        progress.show();
        QApplication::processEvents();
        QApplication::processEvents();
        auto progCB=std::bind([](QProgressDialog* progress, int frame) {
                    if (frame%5==0) {
                        progress->setValue(1);
                        progress->setLabelText(tr("Processing frame %1...").arg(frame));
                        QApplication::processEvents();
                    }
            }, &progress, std::placeholders::_1);
        if (!readFFMPEGAsImageStack(m_video_scaled, fn.toStdString(), video_everyNthFrame=ui->spinEveryNThFrame->value(), video_xyFactor=ui->spinXYFactor->value(), &error, progCB)) {
            progress.close();
            QApplication::restoreOverrideCursor();
            QMessageBox::critical(this, tr("Error opening video"), QString(error.c_str()));
            m_filename="";
        } else {
            progress.close();
            QApplication::restoreOverrideCursor();
            recalcCuts(m_video_scaled.width()/2, m_video_scaled.height()/2);
            QMessageBox::information(this, tr("Video opened"), tr("Video: %1\nframe size: %2x%3\n frames: %4\n color channels: %5").arg(fn).arg(m_video_scaled.width()).arg(m_video_scaled.height()).arg(m_video_scaled.depth()).arg(m_video_scaled.spectrum()));
            m_filename=fn;
        }
    }
}

void MainWindow::recalcCuts(int x, int y)
{
    if (x>=0 && x<m_video_scaled.width() && y>=0 && y<m_video_scaled.height()) {
        lastX=x;
        lastY=y;
        QImage img=CImgToQImage(m_video_scaled, m_video_scaled.depth()/2);
        {
            QPainter pnt(&img);
            pnt.setPen(QPen(QColor("red")));
            pnt.drawLine(0, y,img.width(),y);
            pnt.drawLine(x, 0,x, img.height());
        }

        labXY->setPixmap(QPixmap::fromImage(img));
        labXZ->setPixmap(QPixmap::fromImage(CImgToQImage(extractXZ(m_video_scaled, y))));
        labYZ->setPixmap(QPixmap::fromImage(CImgToQImage(extractZY(m_video_scaled, x))));
    }
}

void MainWindow::on_btnAddXZ_clicked()
{
    if (lastY<0) return;
    ProcessingParameterTable::ProcessingItem item;
    item.mode=ProcessingParameterTable::Mode::XZ;
    item.location=lastY*video_xyFactor;
    m_procModel->addItem(item);
}

void MainWindow::on_btnAddZY_clicked()
{
    if (lastX<0) return;
    ProcessingParameterTable::ProcessingItem item;
    item.mode=ProcessingParameterTable::Mode::ZY;
    item.location=lastX*video_xyFactor;
    m_procModel->addItem(item);
}

void MainWindow::on_btnDelete_clicked()
{
    if (ui->table->currentIndex().row()>=0 && ui->table->currentIndex().row()<m_procModel->rowCount()) {
        m_procModel->removeRow(ui->table->currentIndex().row());
    }
}

void MainWindow::on_btnProcessAll_clicked()
{
    if (m_filename.size()<=0) return;
    if (m_procModel->rowCount()<=0) return;
    QProgressDialog progress(tr("Processing Video"), tr("Cancel"), 0, m_video_scaled.depth()*video_everyNthFrame, this);
    progress.setLabelText(tr("Opening file '%1' ...").arg(m_filename));
    progress.setMinimumDuration(0);
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    progress.setWindowModality(Qt::WindowModal);
    QApplication::processEvents();
    QApplication::processEvents();
    std::string error;
    FFMPEGVideo* vid=openFFMPEGVideo(m_filename.toStdString(), &error);
    int z=0;
    cimg_library::CImg<uint8_t> frame;
    if (vid && readFFMPEGFrame(frame, vid)) {
        std::vector<cimg_library::CImg<uint8_t> > results;
        QVector<ProcessingParameterTable::ProcessingItem> pis=m_procModel->data();
        int j=0;
        qDebug()<<"frame: "<<frame.width()<<"x"<<frame.height()<<"x"<<frame.depth()<<" - "<<frame.spectrum();
        qDebug()<<"m_video_scaled: "<<m_video_scaled.width()<<"x"<<m_video_scaled.height()<<"x"<<m_video_scaled.depth()<<" - "<<m_video_scaled.spectrum();
        qDebug()<<"video_everyNthFrame="<<video_everyNthFrame;
        for (ProcessingParameterTable::ProcessingItem pi: pis) {
            if (pi.mode==ProcessingParameterTable::Mode::ZY) {
                results.push_back(cimg_library::CImg<uint8_t>());
                results[j].assign(m_video_scaled.depth()*video_everyNthFrame, frame.height(), 1, 3);
                qDebug()<<j<<": ZY "<<results[j].width()<<"x"<<results[j].height();
            } else if (pi.mode==ProcessingParameterTable::Mode::XZ) {
                results.push_back(cimg_library::CImg<uint8_t>());
                results[j].assign(frame.width(), m_video_scaled.depth()*video_everyNthFrame, 1, 3);
                qDebug()<<j<<": XZ "<<results[j].width()<<"x"<<results[j].height();
            }
            j++;
        }
        progress.setLabelText(tr("Processing video '%1' ...").arg(m_filename));
        QApplication::processEvents();
        do {

            for (int j=0; j<pis.size(); j++) {
                ProcessingParameterTable::ProcessingItem pi=pis[j];
                if (pi.mode==ProcessingParameterTable::Mode::ZY && z<results[j].width()) {
                    cimg_forY( frame, y )
                    {
                        results[j](z, y, 0,0)=frame(pi.location, y, 0, 0);
                        results[j](z, y, 0,1)=frame(pi.location, y, 0, 1);
                        results[j](z, y, 0,2)=frame(pi.location, y, 0, 2);
                    }
                } else if (pi.mode==ProcessingParameterTable::Mode::XZ && z<results[j].height()) {
                    cimg_forX( frame, x )
                    {
                        results[j](x, z, 0,0)=frame(x, pi.location,  0, 0);
                        results[j](x, z, 0,1)=frame(x, pi.location,  0, 1);
                        results[j](x, z, 0,2)=frame(x, pi.location,  0, 2);
                    }
                }
            }

            if (z%5==0) {
                progress.setValue(z);
                QApplication::processEvents();
            }
            z++;
        } while (readFFMPEGFrame(frame, vid));
        closeFFMPEGVideo(vid);

        QFileInfo fi(m_filename);
        QString allini=fi.absoluteDir().absoluteFilePath(QString("%1.ini").arg(fi.baseName()));
        QSettings setall(allini, QSettings::IniFormat);
        setall.setValue("count", results.size());
        for (size_t j=0; j<results.size(); j++) {

            QString fn=fi.absoluteDir().absoluteFilePath(QString("%1_stack%2.png").arg(fi.baseName()).arg(j+1, 3, 10,QChar('0')));
            QString fnini=fi.absoluteDir().absoluteFilePath(QString("%1_stack%2.ini").arg(fi.baseName()).arg(j+1, 3, 10,QChar('0')));
            progress.setLabelText(tr("Saving result %2: '%1' ...").arg(fn).arg(j+1));
            QApplication::processEvents();
            QApplication::processEvents();
            QImage img=CImgToQImage(results[j]);
            img.save(fn);
            QSettings set(fnini, QSettings::IniFormat);
            ProcessingParameterTable::ProcessingItem pi=pis[j];
            if (pi.mode==ProcessingParameterTable::Mode::ZY) {
                set.setValue("mode", "ZY");
                setall.setValue(QString("item%1/mode").arg(j,3,10,QChar('0')), "ZY");
            }
            if (pi.mode==ProcessingParameterTable::Mode::XZ) {
                set.setValue("mode", "XZ");
                setall.setValue(QString("item%1/mode").arg(j,3,10,QChar('0')), "XZ");
            }
            set.setValue("location", pi.location);
            setall.setValue(QString("item%1/location").arg(j,3,10,QChar('0')), pi.location);
            setall.setValue(QString("item%1/file").arg(j,3,10,QChar('0')), QFileInfo(fn).fileName());
        }
    } else {
        progress.close();
        QApplication::restoreOverrideCursor();
        QMessageBox::critical(this, tr("Error opening video"), QString(error.c_str()));
    }
}

