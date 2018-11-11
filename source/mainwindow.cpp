#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutbox.h"
#include <functional>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include <QPainter>
#include <QScrollBar>
#include <QProgressDialog>
#include <QFileInfo>
#include <QSettings>
#include <vector>
#include "importdialog.h"
#include "processingthread.h"

#define USE_FILTERING

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    lastX(-1),
    lastY(-1),
    m_mode(DisplayModes::unloaded),
    m_settings(QSettings::UserScope, "jkrieger.de", "SlitScanGenerator")
{
    initFFMPEG();
    ui->setupUi(this);
    ui->scrollXY->setWidget(labXY=new ImageViewer(this));
    ui->scrollXZ->setWidget(labXZ=new QLabel(this));
    ui->scrollYZ->setWidget(labYZ=new QLabel(this));
    ui->table->setModel(m_procModel=new ProcessingParameterTable(ui->table));
    ui->toolBar->addAction(ui->actQuit);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actOpenVideo);
    ui->toolBar->addAction(ui->actProcessAll);
    ui->btnProcessAll->setDefaultAction(ui->actProcessAll);
    ui->tabWidget->setCurrentIndex(0);

#ifdef DEBUG_FLAG
    ui->actTest->setVisible(true);
#else
    ui->actTest->setVisible(false);
#endif


    connect(ui->actTest, SIGNAL(triggered()), this, SLOT(test()));
    connect(ui->actQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    connect(ui->actProcessAll, SIGNAL(triggered()), this, SLOT(processAll()));
    connect(ui->actOpenVideo, SIGNAL(triggered()), this, SLOT(openVideo()));
    connect(ui->actOpenExampleVideo, SIGNAL(triggered()), this, SLOT(openExampleVideo()));
    connect(ui->actOpenINI, SIGNAL(triggered()), this, SLOT(loadINI()));
    connect(ui->actSaveINI, SIGNAL(triggered()), this, SLOT(saveINI()));
    connect(ui->scrollXY->horizontalScrollBar(), SIGNAL(sliderMoved(int)), ui->scrollXZ->horizontalScrollBar(), SLOT(setValue(int)));
    connect(ui->scrollXZ->horizontalScrollBar(), SIGNAL(sliderMoved(int)), ui->scrollXY->horizontalScrollBar(), SLOT(setValue(int)));
    connect(ui->scrollXY->verticalScrollBar(), SIGNAL(sliderMoved(int)), ui->scrollYZ->verticalScrollBar(), SLOT(setValue(int)));
    connect(ui->scrollYZ->verticalScrollBar(), SIGNAL(sliderMoved(int)), ui->scrollXY->verticalScrollBar(), SLOT(setValue(int)));
    connect(ui->table, SIGNAL(clicked(QModelIndex)), this, SLOT(tableRowClicked(QModelIndex)));
    connect(labXY, SIGNAL(mouseClicked(int,int)), this, SLOT(ImageClicked(int,int)));
    connect(m_procModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(setWidgetsEnabledForCurrentMode()));
    connect(m_procModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(setWidgetsEnabledForCurrentMode()));
    connect(m_procModel, SIGNAL(modelReset()), this, SLOT(setWidgetsEnabledForCurrentMode()));
    connect(ui->chkNormalize, SIGNAL(toggled(bool)),this, SLOT(recalcAndRedisplaySamples()));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(recalcAndRedisplaySamples()));
    connect(ui->spinWavelength, SIGNAL(valueChanged(double)), this, SLOT(recalcAndRedisplaySamples()));
    connect(ui->spinAngle, SIGNAL(valueChanged(double)), this, SLOT(recalcAndRedisplaySamples()));
    connect(ui->cmbAngle, SIGNAL(currentIndexChanged(int)), this, SLOT(recalcAndRedisplaySamples()));
    connect(ui->spinFilterDelta, SIGNAL(valueChanged(double)), this, SLOT(recalcAndRedisplaySamples()));
    connect(ui->chkWavelength, SIGNAL(toggled(bool)), this, SLOT(recalcAndRedisplaySamples()));
    setWidgetsEnabledForCurrentMode();

    ui->spinWavelength->setValue(m_settings.value("lastFilterWavelength", 5).toDouble());
    ui->spinFilterDelta->setValue(m_settings.value("lastFilterDelta", 0.5).toDouble());
    ui->spinStillGap->setValue(m_settings.value("lastStillGap", 5).toDouble());
    ui->spinStillBorder->setValue(m_settings.value("lastStillBorder", 5).toDouble());
    ui->spinStillLineWidth->setValue(m_settings.value("lastStillLineWidth", 0.2).toDouble());
    ui->spinStillCount->setValue(m_settings.value("lastStillCount", 5).toInt());
    ui->spinStillDelta->setValue(m_settings.value("lastStillDelta", 60).toInt());
    ui->spinAngle->setValue(m_settings.value("lastAngle", 0).toDouble());
    ui->cmbAngle->setCurrentIndex(m_settings.value("lastAngleMode", 0).toInt());
    ui->chkStillStrip->setChecked(m_settings.value("lastStillStrip", true).toBool());
    ui->chkStillDeparateFile->setChecked(m_settings.value("lastStillSeparateFiles", false).toBool());
#ifndef USE_FILTERING
    ui->chkWavelength->setChecked(false);
    ui->tabWidget->setTabEnabled(3, false);
#endif
}

MainWindow::~MainWindow()
{
    m_settings.setValue("lastStillCount", ui->spinStillCount->value());
    m_settings.setValue("lastStillDelta", ui->spinStillDelta->value());
    m_settings.setValue("lastStillStrip", ui->chkStillStrip->isChecked());
    m_settings.setValue("lastStillSeparateFiles", ui->chkStillDeparateFile->isChecked());
    m_settings.setValue("lastFilterWavelength", ui->spinWavelength->value());
    m_settings.setValue("lastFilterDelta", ui->spinFilterDelta->value());
    m_settings.setValue("lastStillGap", ui->spinStillGap->value());
    m_settings.setValue("lastStillBorder", ui->spinStillBorder->value());
    m_settings.setValue("lastStillLineWidth", ui->spinStillLineWidth->value());
    m_settings.setValue("lastAngle", ui->spinAngle->value());
    m_settings.setValue("lastAngleMode", ui->cmbAngle->currentIndex());
    delete ui;
}

void MainWindow::saveINI()
{
    QString fn=QFileDialog::getSaveFileName(this, tr("Save Configuration File ..."), m_settings.value("lastIniDir", "").toString(), tr("INI-File (*.ini)"));
    if (fn.size()>0) {
        m_procModel->save(fn, m_filename);
        QSettings setall(fn, QSettings::IniFormat);
        setall.setValue("stills/count", ui->spinStillCount->value());
        setall.setValue("stills/delta", ui->spinStillDelta->value());
        setall.setValue("stills/strip", ui->chkStillStrip->isChecked());
        setall.setValue("stills/separate_files", ui->chkStillDeparateFile->isChecked());
        setall.setValue("stills/gap", ui->spinStillGap->value());
        setall.setValue("stills/border", ui->spinStillBorder->value());
        setall.setValue("stills/line_width", ui->spinStillLineWidth->value());
        setall.setValue("normalize/enabled", ui->chkNormalize->isChecked());
        setall.setValue("normalize/x", ui->spinNormalizeX->value());
        setall.setValue("normalize/y", ui->spinNormalizeY->value());
        setall.setValue("filter/notch/enabled", ui->chkWavelength->isChecked());
        setall.setValue("filter/notch/wavelength", ui->spinWavelength->value());
        setall.setValue("filter/notch/delta", ui->spinFilterDelta->value());
        m_settings.setValue("lastIniDir", QFileInfo(fn).absolutePath());
    }
}

void MainWindow::loadINI(const QString &fn, QString* vfn)
{
    if (fn.size()>0) {
        m_procModel->load(fn, vfn);

        QSettings setall(fn, QSettings::IniFormat);
        ui->spinStillCount->setValue(setall.value("stills/count", ui->spinStillCount->value()).toInt());
        ui->spinStillDelta->setValue(setall.value("stills/delta", ui->spinStillDelta->value()).toInt());
        ui->chkStillStrip->setChecked(setall.value("stills/strip", ui->chkStillStrip->isChecked()).toBool());
        ui->chkStillDeparateFile->setChecked(setall.value("stills/separate_files", ui->chkStillDeparateFile->isChecked()).toBool());
        ui->spinStillGap->setValue(setall.value("stills/gap", ui->spinStillGap->value()).toInt());
        ui->spinStillBorder->setValue(setall.value("stills/border", ui->spinStillBorder->value()).toInt());
        ui->spinStillLineWidth->setValue(setall.value("stills/line_width", ui->spinStillLineWidth->value()).toInt());
        ui->chkNormalize->setChecked(setall.value("normalize/enabled", ui->chkNormalize->isChecked()).toBool());
        ui->spinNormalizeX->setValue(setall.value("normalize/x", ui->spinNormalizeX->value()).toInt());
        ui->spinNormalizeY->setValue(setall.value("normalize/y", ui->spinNormalizeY->value()).toInt());
        ui->chkWavelength->setChecked(setall.value("filter/notch/enabled", ui->chkWavelength->isChecked()).toBool());
        ui->spinWavelength->setValue(setall.value("filter/notch/wavelength", ui->spinWavelength->value()).toDouble());
        ui->spinFilterDelta->setValue(setall.value("filter/notch/delta", ui->spinFilterDelta->value()).toDouble());
    }
}

void MainWindow::loadINI()
{
    QString fn=QFileDialog::getOpenFileName(this, tr("Save Configuration File ..."), m_settings.value("lastIniDir", "").toString(), tr("INI-File (*.ini)"));
    if (fn.size()>0) {
        m_settings.setValue("lastIniDir", QFileInfo(fn).absolutePath());
        QString vfn;
        loadINI(fn, &vfn);
        if (vfn!=m_filename && vfn.size()>0 && QFile::exists(vfn)) {
            if (QMessageBox::question(this, tr("Load Video File?"), tr("The INI-file you loaded mentioned a video. Should this video be loaded?"), QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes)==QMessageBox::Yes) {
                openVideo(vfn);
            }
        }
    }
}


void MainWindow::setWidgetsEnabledForCurrentMode() {
    const bool isloaded =(m_mode==DisplayModes::loaded);

    ui->tabWidget->setEnabled(isloaded);
    ui->grpParams->setEnabled(isloaded);
    ui->scrollXY->setEnabled(isloaded);
    ui->scrollYZ->setEnabled(isloaded);
    ui->scrollXZ->setEnabled(isloaded);
    ui->widAngle->setEnabled(isloaded);

    ui->btnDelete->setEnabled(m_procModel->rowCount()>0);
    ui->btnDeleteAll->setEnabled(m_procModel->rowCount()>0);
    ui->actProcessAll->setEnabled(m_procModel->rowCount()>0);
    ui->btnAddXZ->setEnabled(m_video_xytscaled.depth()>0);
    ui->btnAddZY->setEnabled(m_video_xytscaled.depth()>0);
}

void MainWindow::showAbout()
{
  AboutBox* dlg=new AboutBox(this);
  dlg->exec();
  delete dlg;
}

void MainWindow::test()
{
    QString fn=QFileDialog::getOpenFileName(this, tr("Open Test Image ..."), m_settings.value("lastTestDir", "").toString());
    if (fn.size()>0) {
        m_settings.setValue("lastTestDir", QFileInfo(fn).absolutePath());
        cimg_library::CImg<uint8_t> img;
        img.load_bmp(fn.toLocal8Bit().data());
        labXY->setPixmap(QPixmap::fromImage(CImgToQImage(img)));
        ProcessingTask::applyFilterNotch(img, QInputDialog::getInt(this, "TEST", "wavelength=", 10, 0,1000,2), 2, true);
        labYZ->setPixmap(QPixmap::fromImage(CImgToQImage(img)));
    }
}

void MainWindow::openVideo(const QString& filename) {
    QString fn=filename;
    if (fn.size()<=0) {
        fn=QFileDialog::getOpenFileName(this, tr("Open Video File ..."), m_settings.value("lastVideoDir", "").toString());
    }
    QFileInfo fi(fn);
    QString ini;
    if (fn.size()>0) {
        m_settings.setValue("lastVideoDir", QFileInfo(fn).absolutePath());
        //m_video.load_ffmpeg_external(filename.toLatin1().data(), 'z');
        std::string error;


        ImportDialog* dlg=new ImportDialog(this, &m_settings);
        if (dlg->openVideo(fn, ini)) {
           m_procModel->clear();
           if (QFile::exists(ini)) {
               loadINI(ini, NULL);
           }
           if (dlg->exec()==QDialog::Accepted) {
               ui->labFilename->setText(fn);
               ui->labProps->setText(tr("%1 frames, %2x%3 Pixels^2").arg(dlg->getFrames()).arg(dlg->getWidth()).arg(dlg->getHeight()));
               ui->labPreviewSettings->setText(tr("every %1-th frame, 1/%2x-scaling").arg(dlg->getEveryNthFrame()).arg(dlg->getXYScaleFactor()));
               QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
               QProgressDialog progress(tr("Opening Video"), tr("Cancel"), 0, 2, this);
               progress.setLabelText(tr("opening file '%1'...").arg(fn));
               progress.setMinimumDuration(0);
               QApplication::processEvents();
               progress.setWindowModality(Qt::WindowModal);
               progress.show();
               QApplication::processEvents();
               QApplication::processEvents();
               auto progCB=std::bind([](QProgressDialog* progress, int frame, int maxi) -> bool {
                           if (frame%5==0) {
                               if (maxi>1) {
                                   progress->setValue(frame);
                                   progress->setMaximum(maxi+1);
                                   progress->setLabelText(tr("Reading frame %1/%2...").arg(frame).arg(maxi));
                               } else {
                                   progress->setValue(1);
                                   progress->setMaximum(2);
                                   progress->setLabelText(tr("Reading frame %1...").arg(frame));
                               }

                               QApplication::processEvents();

                           }
                           return progress->wasCanceled();
                   }, &progress, std::placeholders::_1, std::placeholders::_2);
               if ((!readFFMPEGAsImageStack(m_video_xytscaled, fn.toStdString(), video_everyNthFrame=dlg->getEveryNthFrame(), video_xyFactor=dlg->getXYScaleFactor(), &error, progCB))) {

                   progress.close();
                   QApplication::restoreOverrideCursor();
                   QMessageBox::critical(this, tr("Error opening video"), QString(error.c_str()));
                   m_filename="";
                   m_mode=DisplayModes::unloaded;
               } else {
                   if (dlg->getFramesHR()<=0 || readFFMPEGAsImageStack(m_video_some_frames, fn.toStdString(), 1, 1, &error, progCB, dlg->getFramesHR())) {
                       progress.close();
                       QApplication::restoreOverrideCursor();
                       recalcAndRedisplaySamples(m_video_xytscaled.width()/2, m_video_xytscaled.height()/2, 0, 0);
                       QMessageBox::information(this, tr("Video opened"), tr("Video: %1\nframe size: %2x%3\n frames: %4\n color channels: %5").arg(fn).arg(m_video_xytscaled.width()).arg(m_video_xytscaled.height()).arg(m_video_xytscaled.depth()).arg(m_video_xytscaled.spectrum()));
                       m_filename=fn;
                       m_mode=DisplayModes::loaded;
                   } else {
                       progress.close();
                       QApplication::restoreOverrideCursor();
                       QMessageBox::critical(this, tr("Error opening video"), QString(error.c_str()));
                       m_filename="";
                       m_mode=DisplayModes::unloaded;
                   }
               }
           }
        }

        delete dlg;
    }
    setWidgetsEnabledForCurrentMode();
}

void MainWindow::openExampleVideo()
{
    QDir d(QApplication::instance()->applicationDirPath());
    d.cd("testmovie");
    QString fn=QFileDialog::getOpenFileName(this, tr("Open Example Video File ..."), d.absolutePath());
    if (fn.size()>0) {
        openVideo(fn);
    }
}

void MainWindow::recalcAndRedisplaySamples(int x, int y, double angle, int angleMode)
{
    cimg_library::CImg<uint8_t>* video_input=&m_video_xytscaled;
    if (ui->tabWidget->currentIndex()==3) {
        video_input=&m_video_some_frames;
    }
    //qDebug()<<"recalcCuts("<<x<<", "<<y<<")  "<<m_video_scaled.width()<<","<<m_video_scaled.height();
    if (x>=0 && x<video_input->width() && y>=0 && y<video_input->height()) {
        if (ui->tabWidget->currentIndex()==3) {
            lastX=x/video_xyFactor;
            lastY=y/video_xyFactor;
        } else {
            lastX=x;
            lastY=y;
        }

    }

    if (ui->spinAngle->value()!=angle) ui->spinAngle->setValue(angle);
    if (ui->cmbAngle->currentIndex()!=angleMode) ui->cmbAngle->setCurrentIndex(angleMode);
    recalcAndRedisplaySamples();
}

void MainWindow::recalcAndRedisplaySamples()
{
    cimg_library::CImg<uint8_t>* video_input=&m_video_xytscaled;
    double xyFactor=1;
    double invxyFactor=video_xyFactor;
    double angle=ui->spinAngle->value();
    if (ui->tabWidget->currentIndex()==3) {
        video_input=&m_video_some_frames;
        xyFactor=video_xyFactor;
        invxyFactor=1;        
    }

    //qDebug()<<"lastX="<<lastX<<", lastY="<<lastY<<", xyFactor="<<xyFactor<<", video_xyFactor="<<video_xyFactor<<", video_everyNthFrame="<<video_everyNthFrame<<", angle="<<angle;

    if (lastX*xyFactor>=0 && lastX*xyFactor<video_input->width() && lastY*xyFactor>=0 && lastY*xyFactor<video_input->height()) {


        QImage img=CImgToQImage(*video_input, video_input->depth()/2);
        cimg_library::CImg<uint8_t> cxz;
        cimg_library::CImg<uint8_t> cyz;
        if (angle==0) {
            cxz=extractXZ(*video_input, lastY*xyFactor);
            cyz=extractZY(*video_input, lastX*xyFactor);
        } else if (ui->cmbAngle->currentIndex()==0) {
            cxz=extractXZ_roll(*video_input, lastX*xyFactor, lastY*xyFactor, angle);
            cyz=extractZY_roll(*video_input, lastX*xyFactor, lastY*xyFactor, angle);
        } else {
            cxz=extractXZ_pitch(*video_input, lastY, angle, video_xyFactor, video_everyNthFrame);
            cyz=extractZY_pitch(*video_input, lastX, angle, video_xyFactor, video_everyNthFrame);
        }

        if (ui->chkNormalize->isChecked()) {
            ProcessingTask::normalizeZY(cyz, ui->spinNormalizeY->value()/invxyFactor);
            ProcessingTask::normalizeXZ(cxz, ui->spinNormalizeX->value()/invxyFactor);
        }

        if (ui->tabWidget->currentIndex()==3 && ui->chkWavelength->isChecked()) {
            ProcessingTask::applyFilterNotch(cyz, ui->spinWavelength->value(), ui->spinFilterDelta->value());
            ProcessingTask::applyFilterNotch(cxz, ui->spinWavelength->value(), ui->spinFilterDelta->value());
        }

        QImage imgxz=CImgToQImage(cxz);
        QImage imgyz=CImgToQImage(cyz);
        {
            QPainter pnt(&img);
            pnt.setPen(QPen(QColor("red")));
            if (ui->cmbAngle->currentIndex()==0) {
                pnt.save();
                pnt.translate(lastX,lastY);
                pnt.rotate(angle);
                const int l=2*std::max(img.width(), img.height());
                pnt.drawLine(-l,0,l,0);
                pnt.drawLine(0,-l,0,l);
                pnt.restore();
            } else {
                pnt.drawLine(0, lastY*xyFactor,img.width(),lastY*xyFactor);
                pnt.drawLine(lastX*xyFactor, 0,lastX*xyFactor, img.height());
            }

            if (ui->chkNormalize->isChecked()) {
                pnt.setPen(QPen(QColor("blue")));
                pnt.drawRect(QRect(ui->spinNormalizeX->value()/invxyFactor-1, ui->spinNormalizeY->value()/invxyFactor-1,3,3));
            }
        }
        if (ui->chkNormalize->isChecked()) {
            QPainter pnt(&imgxz);
            pnt.setPen(QPen(QColor("blue")));
            pnt.drawLine(ui->spinNormalizeX->value()/invxyFactor, 0,ui->spinNormalizeX->value()/invxyFactor, imgxz.height());
        }
        if (ui->chkNormalize->isChecked()) {
            QPainter pnt(&imgyz);
            pnt.setPen(QPen(QColor("blue")));
            pnt.drawLine(0,ui->spinNormalizeY->value()/invxyFactor, imgyz.width(),ui->spinNormalizeY->value()/invxyFactor);
        }


        labXY->setPixmap(QPixmap::fromImage(img));
        labXZ->setPixmap(QPixmap::fromImage(imgxz));
        labYZ->setPixmap(QPixmap::fromImage(imgyz));
    }
}

void MainWindow::ImageClicked(int x, int y)
{
    if (ui->tabWidget->currentIndex()==2) {
        ui->spinNormalizeX->setValue(x*video_xyFactor);
        ui->spinNormalizeY->setValue(y*video_xyFactor);
        recalcAndRedisplaySamples();
    } else {
        recalcAndRedisplaySamples(x,y,ui->spinAngle->value(), ui->cmbAngle->currentIndex());
    }
}

void MainWindow::on_btnAddXZ_clicked()
{
    if (lastY<0) return;
    ProcessingTask::ProcessingItem item;
    item.mode=ProcessingTask::Mode::XZ;
    item.location_x=lastX*video_xyFactor;
    item.location_y=lastY*video_xyFactor;
    item.angle=ui->spinAngle->value();
    item.angleMode=ProcessingTask::AngleMode::AngleNone;
    if (ui->cmbAngle->currentIndex()==0) item.angleMode=ProcessingTask::AngleMode::AngleRoll;
    if (ui->cmbAngle->currentIndex()==1) item.angleMode=ProcessingTask::AngleMode::AnglePitch;
    m_procModel->addItem(item);
}

void MainWindow::on_btnAddZY_clicked()
{
    if (lastX<0) return;
    ProcessingTask::ProcessingItem item;
    item.mode=ProcessingTask::Mode::ZY;
    item.location_x=lastX*video_xyFactor;
    item.location_y=lastY*video_xyFactor;
    item.angle=ui->spinAngle->value();
    item.angleMode=ProcessingTask::AngleMode::AngleNone;
    if (ui->cmbAngle->currentIndex()==0) item.angleMode=ProcessingTask::AngleMode::AngleRoll;
    if (ui->cmbAngle->currentIndex()==1) item.angleMode=ProcessingTask::AngleMode::AnglePitch;
    m_procModel->addItem(item);
}

void MainWindow::on_btnDelete_clicked()
{
    auto rows=ui->table->selectionModel()->selectedRows();
    //qDebug()<<"delete "<<rows.size();
    QVector<int> r;
    for (auto idx: rows) {
        r.push_back(idx.row());
    }
    std::sort(r.begin(), r.end(), [](int a, int b) { return a>=b;});
    for (auto idx: r) {
        m_procModel->removeRow(idx);
    }
}

void MainWindow::on_btnDeleteAll_clicked()
{
    m_procModel->clear();
}

void MainWindow::processAll()
{
    if (m_filename.size()<=0) return;
    if (m_procModel->rowCount()<=0) return;

    ProcessingTask* task=new ProcessingTask();
    task->filename=m_filename;
    task->outputFrames=m_video_xytscaled.depth()*video_everyNthFrame;
    task->pis=m_procModel->dataVector();

    task->stillCnt=ui->spinStillCount->value();
    task->stillDelta=ui->spinStillDelta->value();
    task->stillStrip=ui->chkStillStrip->isChecked();
    task->stillSeparateFiles=ui->chkStillDeparateFile->isChecked();
    task->stillGap=ui->spinStillGap->value();
    task->stillBorder=ui->spinStillBorder->value();
    task->stillLineWidth=ui->spinStillLineWidth->value();

    task->normalize=ui->chkNormalize->isChecked();
    task->normalizeX=ui->spinNormalizeX->value();
    task->normalizeY=ui->spinNormalizeY->value();

    task->filterNotch=ui->chkWavelength->isChecked();
    task->fiterNotchWavelength=ui->spinWavelength->value();
    task->fiterNotchWidth=ui->spinFilterDelta->value();

    ProcessingThread* thr=new ProcessingThread(task, this);
    ui->widProcessing->push_back(thr);

}

void MainWindow::tableRowClicked(const QModelIndex &index)
{
    //qDebug()<<"clicked "<<index<<"  "<<m_procModel->getItem(index).location;
    //if (index.isValid()) {
        if (m_procModel->getItem(index).mode==ProcessingTask::Mode::ZY) {
            recalcAndRedisplaySamples(m_procModel->getItem(index).location_x/video_xyFactor, m_procModel->getItem(index).location_y/video_xyFactor, m_procModel->getItem(index).angle, m_procModel->getItem(index).angleModeForCombo());
        } else if (m_procModel->getItem(index).mode==ProcessingTask::Mode::XZ) {
            recalcAndRedisplaySamples(m_procModel->getItem(index).location_x/video_xyFactor, m_procModel->getItem(index).location_y/video_xyFactor, m_procModel->getItem(index).angle, m_procModel->getItem(index).angleModeForCombo());
        }
    //}
}
