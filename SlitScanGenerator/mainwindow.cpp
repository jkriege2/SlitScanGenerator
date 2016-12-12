#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutbox.h"
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
#include "importdialog.h"
#include "processingthread.h"

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


    connect(ui->actQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    connect(ui->actProcessAll, SIGNAL(triggered()), this, SLOT(processAll()));
    connect(ui->actOpenVideo, SIGNAL(triggered()), this, SLOT(openVideo()));
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
    connect(ui->spinFilterDelta, SIGNAL(valueChanged(double)), this, SLOT(recalcAndRedisplaySamples()));
    connect(ui->chkWavelength, SIGNAL(toggled(bool)), this, SLOT(recalcAndRedisplaySamples()));
    setWidgetsEnabledForCurrentMode();

    ui->spinWavelength->setValue(m_settings.value("lastFilterWavelength", 5).toDouble());
    ui->spinFilterDelta->setValue(m_settings.value("lastFilterDelta", 0.5).toDouble());
    ui->spinStillCount->setValue(m_settings.value("lastStillCount", 5).toInt());
    ui->spinStillDelta->setValue(m_settings.value("lastStillDelta", 60).toInt());
    ui->chkStillStrip->setChecked(m_settings.value("lastStillStrip", true).toBool());
    ui->chkStillDeparateFile->setChecked(m_settings.value("lastStillSeparateFiles", false).toBool());

    ui->chkWavelength->setChecked(false);
    ui->tabWidget->setTabEnabled(3, false);
}

MainWindow::~MainWindow()
{
    m_settings.setValue("lastStillCount", ui->spinStillCount->value());
    m_settings.setValue("lastStillDelta", ui->spinStillDelta->value());
    m_settings.setValue("lastStillStrip", ui->chkStillStrip->isChecked());
    m_settings.setValue("lastStillSeparateFiles", ui->chkStillDeparateFile->isChecked());
    m_settings.setValue("lastFilterWavelength", ui->spinWavelength->value());
    m_settings.setValue("lastFilterDelta", ui->spinFilterDelta->value());

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
        ui->chkNormalize->setChecked(setall.value("normalize/enabled", ui->chkNormalize->isChecked()).toBool());
        ui->spinNormalizeX->setValue(setall.value("normalize/x", ui->spinNormalizeX->value()).toInt());
        ui->spinNormalizeY->setValue(setall.value("normalize/y", ui->spinNormalizeY->value()).toInt());
        ui->chkWavelength->setChecked(setall.value("filter/notch/enabled", ui->chkWavelength->isChecked()).toBool());
        ui->spinWavelength->setValue(setall.value("filter/notch/wavelength", ui->spinWavelength->value()).toDouble());
        ui->spinWavelength->setValue(setall.value("filter/notch/delta", ui->spinFilterDelta->value()).toDouble());
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

    ui->btnDelete->setEnabled(m_procModel->rowCount()>0);
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
                       recalcAndRedisplaySamples(m_video_xytscaled.width()/2, m_video_xytscaled.height()/2);
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

void MainWindow::recalcAndRedisplaySamples(int x, int y)
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
    recalcAndRedisplaySamples();
}

void MainWindow::recalcAndRedisplaySamples()
{
    cimg_library::CImg<uint8_t>* video_input=&m_video_xytscaled;
    double xyFactor=1;
    double invxyFactor=video_xyFactor;
    if (ui->tabWidget->currentIndex()==3) {
        video_input=&m_video_some_frames;
        xyFactor=video_xyFactor;
        invxyFactor=1;
    }

    if (lastX*xyFactor>=0 && lastX*xyFactor<video_input->width() && lastX*xyFactor>=0 && lastX*xyFactor<video_input->height()) {


        QImage img=CImgToQImage(*video_input, video_input->depth()/2);
        cimg_library::CImg<uint8_t> cxz=extractXZ(*video_input, lastY*xyFactor);
        cimg_library::CImg<uint8_t> cyz=extractZY(*video_input, lastX*xyFactor);

        if (ui->chkNormalize->isChecked()) {
            ProcessingTask::normalizeZY(cyz, ui->spinNormalizeY->value()/invxyFactor);
            ProcessingTask::normalizeXZ(cxz, ui->spinNormalizeX->value()/invxyFactor);
        }

        if (ui->tabWidget->currentIndex()==3 && ui->chkWavelength->isChecked()) {
            ProcessingTask::applyFilterNotch(cyz, ui->spinWavelength->value(), ui->spinFilterDelta->value());
            ProcessingTask::applyFilterNotch(cxz, ui->spinNormalizeX->value(), ui->spinFilterDelta->value());
        }

        QImage imgxz=CImgToQImage(cxz);
        QImage imgyz=CImgToQImage(cyz);
        {
            QPainter pnt(&img);
            pnt.setPen(QPen(QColor("red")));
            pnt.drawLine(0, lastY*xyFactor,img.width(),lastY*xyFactor);
            pnt.drawLine(lastX*xyFactor, 0,lastX*xyFactor, img.height());
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
        recalcAndRedisplaySamples(x,y);
    }
}

void MainWindow::on_btnAddXZ_clicked()
{
    if (lastY<0) return;
    ProcessingTask::ProcessingItem item;
    item.mode=ProcessingTask::Mode::XZ;
    item.location=lastY*video_xyFactor;
    m_procModel->addItem(item);
}

void MainWindow::on_btnAddZY_clicked()
{
    if (lastX<0) return;
    ProcessingTask::ProcessingItem item;
    item.mode=ProcessingTask::Mode::ZY;
    item.location=lastX*video_xyFactor;
    m_procModel->addItem(item);
}

void MainWindow::on_btnDelete_clicked()
{
    //if (ui->table->currentIndex().row()>=0 && ui->table->currentIndex().row()<m_procModel->rowCount()) {
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
    //}
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
            recalcAndRedisplaySamples(m_procModel->getItem(index).location/video_xyFactor, lastY);
        } else if (m_procModel->getItem(index).mode==ProcessingTask::Mode::XZ) {
            recalcAndRedisplaySamples(lastX, m_procModel->getItem(index).location/video_xyFactor);
        }
    //}
}

