#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <functional>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QPainter>
#include <QScrollBar>
#include <QProgressDialog>

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
        } else {
            progress.close();
            QApplication::restoreOverrideCursor();
            recalcCuts(m_video_scaled.width()/2, m_video_scaled.height()/2);
            QMessageBox::information(this, tr("Video opened"), tr("Video: %1\nframe size: %2x%3\n frames: %4\n color channels: %5").arg(fn).arg(m_video_scaled.width()).arg(m_video_scaled.height()).arg(m_video_scaled.depth()).arg(m_video_scaled.spectrum()));
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
    QProgressDialog progress(tr("Opening Video"), "", 0, 2, this);
    progress.setWindowModality(Qt::WindowModal);
}

