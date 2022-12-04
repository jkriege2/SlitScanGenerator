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
#include "optionsdialog.h"
#include "defines.h"
#include "videoreader_ffmpeg.h"
#include "videoreader_cimg.h"
#include "videopreviewreaderthread.h"
#include "cpp_tools.h"
#include "qt_tools.h"
#include "imagewriter_cimg.h"
#include "imagewriter_png.h"
#include "configio_dummy.h"
#include "configio_ini.h"

#define USE_FILTERING


void switchTranslator(QTranslator& translator, const QString& filename)
{
    // remove the old translator
    qApp->removeTranslator(&translator);

    // load the new translator
    bool result = translator.load(filename);
    qDebug("translator.load(%s) %s", filename.toLatin1().data(), result ? "true" : "false" );

    if(!result) {
        qWarning("*** Failed translator.load(\"%s\")", filename.toLatin1().data());
        return;
    }
    qApp->installTranslator(&translator);
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    lastX_reducedCoords(-1),
    lastY_reducedCoords(-1),
    m_mode(DisplayModes::unloaded),
    m_settings(),
    m_langGroup(nullptr)
{
    QDir appdir=QFileInfo(QApplication::instance()->applicationFilePath()).absoluteDir();
    appdir.cd("translations");
    m_langPath=appdir.absolutePath();

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
    connect(ui->actSettings, SIGNAL(triggered()), this, SLOT(showSettings()));
    connect(ui->actQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    connect(ui->actProcessAll, SIGNAL(triggered()), this, SLOT(processAll()));
    connect(ui->actionProcess_INI_File, SIGNAL(triggered()), this, SLOT(processINIFile()));
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
    connect(ui->cmbInterpolation, SIGNAL(currentIndexChanged(int)), this, SLOT(recalcAndRedisplaySamples()));
    connect(ui->spinFilterDelta, SIGNAL(valueChanged(double)), this, SLOT(recalcAndRedisplaySamples()));
    connect(ui->chkWavelength, SIGNAL(toggled(bool)), this, SLOT(recalcAndRedisplaySamples()));
    connect(ui->chkModifyWhitepoint, SIGNAL(toggled(bool)), this, SLOT(recalcAndRedisplaySamples()));
    connect(ui->spinWhitepointR, SIGNAL(valueChanged(int)), this, SLOT(recalcAndRedisplaySamples()));
    connect(ui->spinWhitepointG, SIGNAL(valueChanged(int)), this, SLOT(recalcAndRedisplaySamples()));
    connect(ui->spinWhitepointB, SIGNAL(valueChanged(int)), this, SLOT(recalcAndRedisplaySamples()));
    connect(ui->edtOutputBasename, SIGNAL(editingFinished()), this, SLOT(recalcAndRedisplaySamples()));
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
    ui->cmbInterpolation->setCurrentIndex(m_settings.value("lastInterpolation", 2).toInt());
    ui->chkStillStrip->setChecked(m_settings.value("lastStillStrip", true).toBool());
    ui->chkStillDeparateFile->setChecked(m_settings.value("lastStillSeparateFiles", false).toBool());
    ui->edtOutputBasename->setText("");
#ifndef USE_FILTERING
    ui->chkWavelength->setChecked(false);
    ui->tabWidget->setTabEnabled(3, false);
#endif

    setWindowTitle(tr("%1 %2 [%3bit]").arg(PROJECT_LONGNAME).arg(PROJECT_VERSION).arg(sizeof(void*)*8));

    loadLanguages();
    loadLanguage(m_settings.value("lastLanguage", "en").toString());
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
    m_settings.setValue("lastInterpolation", ui->cmbInterpolation->currentIndex());
    delete ui;
}

void MainWindow::saveINI()
{
    QString fn=QFileDialog::getSaveFileName(this, tr("Save Configuration File ..."), m_settings.value("lastIniDir", "").toString(), tr("INI-File (*.ini)"));
    if (fn.size()>0) {
        ProcessingTask task(std::shared_ptr<VideoReader>(nullptr), std::shared_ptr<ImageWriter>(nullptr), std::make_shared<ConfigIO_INI>());
        saveToTask(task);
        task.save(fn);

        m_settings.setValue("lastIniDir", QFileInfo(fn).absolutePath());
    }
}

void MainWindow::loadINI(const QString &fn, QString* vfn)
{
    if (fn.size()>0) {
        ProcessingTask task(std::shared_ptr<VideoReader>(nullptr), std::shared_ptr<ImageWriter>(nullptr), std::make_shared<ConfigIO_INI>());
        task.load(fn);
        loadFromTask(task);
        if (vfn) *vfn=task.filename;
    }
}

void MainWindow::loadFromTask(const ProcessingTask &task)
{
    ui->spinStillCount->setValue(task.stillCnt);
    ui->spinStillDelta->setValue(task.stillGap);
    ui->chkStillStrip->setChecked(task.stillStrip);
    ui->chkStillDeparateFile->setChecked(task.stillSeparateFiles);
    ui->spinStillGap->setValue(task.stillGap);
    ui->spinStillBorder->setValue(task.stillBorder);
    ui->spinStillLineWidth->setValue(task.stillLineWidth);
    ui->chkNormalize->setChecked(task.normalize);
    ui->spinNormalizeX->setValue(task.normalizeX);
    ui->spinNormalizeY->setValue(task.normalizeY);
    ui->chkWavelength->setChecked(task.filterNotch);
    ui->spinWavelength->setValue(task.fiterNotchWavelength);
    ui->spinFilterDelta->setValue(task.fiterNotchWidth);
    ui->chkModifyWhitepoint->setChecked(task.modifyWhite);
    ui->spinWhitepointR->setValue(task.whitepointR);
    ui->spinWhitepointG->setValue(task.whitepointG);
    ui->spinWhitepointB->setValue(task.whitepointB);
    ui->cmbInterpolation->setCurrentIndex(static_cast<int>(task.interpolationMethod));
    ui->edtOutputBasename->setText(task.outputBasename);
    m_procModel->load(task);
}

void MainWindow::saveToTask(ProcessingTask &task, double xyScaling, double tScaling) const
{
    task.filename=m_filename;
    task.outputFrames=m_video_xytscaled.depth()*video_everyNthFrame;
    task.outputBasename=ui->edtOutputBasename->text();
    task.stillCnt=ui->spinStillCount->value();
    task.stillGap=ui->spinStillDelta->value();
    task.stillStrip=ui->chkStillStrip->isChecked();
    task.stillSeparateFiles=ui->chkStillDeparateFile->isChecked();
    task.stillGap=ui->spinStillGap->value();
    task.stillBorder=ui->spinStillBorder->value();
    task.stillLineWidth=ui->spinStillLineWidth->value();
    task.normalize=ui->chkNormalize->isChecked();
    task.normalizeX=ui->spinNormalizeX->value()*xyScaling;
    task.normalizeY=ui->spinNormalizeY->value()*xyScaling;
    task.filterNotch=ui->chkWavelength->isChecked();
    task.fiterNotchWavelength=ui->spinWavelength->value();
    task.fiterNotchWidth=ui->spinFilterDelta->value();
    task.modifyWhite=ui->chkModifyWhitepoint->isChecked();
    task.whitepointR=ui->spinWhitepointR->value();
    task.whitepointG=ui->spinWhitepointG->value();
    task.whitepointB=ui->spinWhitepointB->value();
    task.interpolationMethod=static_cast<ProcessingTask::InterpolationMethod>(ui->cmbInterpolation->currentIndex());
    m_procModel->save(task, xyScaling, tScaling);
}

void MainWindow::loadLanguages()
{
    // format systems language
    QString defaultLocale = QLocale::system().name(); // e.g. "de_DE"
    defaultLocale.truncate(defaultLocale.lastIndexOf('_')); // e.g. "de"
    QDir appdir(m_langPath);
    // Create Language Menu to match qm translation files found
    m_langGroup = new QActionGroup(ui->menu_Language);
    m_langGroup->setExclusive(true);
    connect(m_langGroup, SIGNAL (triggered(QAction *)), this, SLOT (slotLanguageChanged(QAction *)));
    qDebug()<<appdir;

    QStringList fileNames = appdir.entryList(QStringList("SlitScanGenerator*.qm"));
    for (int i = 0; i < fileNames.size(); ++i) {
        // get locale extracted by filename
        QString locale;
        locale = fileNames[i]; // "SlitScanGenerator_de.qm"

        locale.truncate(locale.lastIndexOf('.')); // "SlitScanGenerator_de"
        locale.remove(0, locale.indexOf('_') + 1); // "de"

        QString lang = QLocale::languageToString(QLocale(locale).language());
        QIcon ico(QString(":/flags/%1.png").arg(locale));

        QAction *action = new QAction(ico, lang, this);
        action->setCheckable(true);
        // action->setData(resourceFileName);
        action->setData(locale);

        ui->menu_Language->addAction(action);
        m_langGroup->addAction(action);

        // set default translators and language checked
        if (defaultLocale == locale)
        {
            action->setChecked(true);
        }
    } // for: end
}

// Called every time, when a menu entry of the language menu is called
void MainWindow::slotLanguageChanged(QAction* action)
{
    if(0 == action) {
        return;
    }

    // load the language dependant on the action content
    loadLanguage(action->data().toString());
}

void MainWindow::loadLanguage(const QString& rLanguage)
{
    if(m_currLang == rLanguage) {
        return;
    }
    m_currLang = rLanguage;
    qDebug("loadLanguage %s", rLanguage.toLatin1().data());

    QLocale locale = QLocale(m_currLang);
    QLocale::setDefault(locale);
    QString languageName = QLocale::languageToString(locale.language());

    // m_translator contains the app's translations
    QString resourceFileName = QString("%1/SlitScanGenerator_%2.qm").arg(m_langPath).arg(rLanguage);
    switchTranslator(m_translator, resourceFileName);
    switchTranslator(m_translatorQt, QString("qt_%1.qm").arg(rLanguage));

    m_settings.setValue("lastLanguage", rLanguage);

    for (auto act: m_langGroup->actions()) {
        if (act->data().toString().toLatin1().toLower()==m_currLang.toLower()) {
            act->setChecked(true);
            break;
        }
    }
}

void MainWindow::changeEvent(QEvent* event)
{
    if(0 != event) {
        switch(event->type()) {
        // this event is send if a translator is loaded
        case QEvent::LanguageChange:
            // UI will not update unless you call retranslateUi
            ui->retranslateUi(this);
            break;

            // this event is send, if the system, language changes
        case QEvent::LocaleChange:
        {
            QString locale = QLocale::system().name();
            locale.truncate(locale.lastIndexOf('_'));
            loadLanguage(locale);
        }
        break;
        default: break;
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::loadINI()
{
    QString fn=QFileDialog::getOpenFileName(this, tr("Open Configuration File ..."), m_settings.value("lastIniDir", "").toString(), tr("INI-File (*.ini)"));
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

void MainWindow::showSettings()
{
  OptionsDialog* dlg=new OptionsDialog(this);
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

void MainWindow::openVideo(const QString& filename, const QString &ini_in) {
    auto finallySetWidgets=finally(std::bind([](MainWindow* t) {t->setWidgetsEnabledForCurrentMode();}, this));
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


        QSharedPointer<ImportDialog> dlg(new ImportDialog(this, &m_settings));
        if (dlg->openVideo(fn, ini)) {
           m_procModel->clear();
           if (QFile::exists(ini) && (ini_in.isEmpty() || !QFile::exists(ini_in))) {
               loadINI(ini, NULL);
           }
           if (dlg->exec()==QDialog::Accepted) {
               ui->labFilename->setText(fn);
               ui->labProps->setText(tr("%1 frames, %2x%3 Pixels^2").arg(dlg->getFrames()).arg(dlg->getWidth()).arg(dlg->getHeight()));
               ui->labPreviewSettings->setText(tr("every %1-th frame, 1/%2x-scaling").arg(dlg->getEveryNthFrame()).arg(dlg->getXYScaleFactor()));
               QOverrideCursorGuard cursorGuard(Qt::WaitCursor);
               QProgressDialog progress(tr("Opening Video"), tr("Cancel"), 0, 2, this);
               progress.setLabelText(tr("opening file '%1'...").arg(fn));
               progress.setMinimumDuration(0);
               progress.setWindowModality(Qt::WindowModal);
               progress.show();
               QApplication::processEvents();
               VideoPreviewReaderThread frameReader(m_video_xytscaled, fn.toStdString(), video_everyNthFrame=dlg->getEveryNthFrame(), video_xyFactor=dlg->getXYScaleFactor(), &error, -1, &progress, nullptr);


               if (!frameReader.exec()) {

                   progress.close();
                   QMessageBox::critical(this, tr("Error opening video"), QString(error.c_str()));
                   m_filename="";
                   m_mode=DisplayModes::unloaded;
               } else {
                   VideoPreviewReaderThread frameReaderDetail(m_video_some_frames, fn.toStdString(), 1, 1, &error, dlg->getFramesHR(), &progress, nullptr);
                   if (dlg->getFramesHR()<=0 || frameReaderDetail.exec()) {
                       progress.close();
                       recalcAndRedisplaySamples(m_video_xytscaled.width()/2, m_video_xytscaled.height()/2, 0, 0);
                       cursorGuard.resetCursor();
                       QMessageBox::information(this, tr("Video opened"), tr("Video: %1\nframe size: %2x%3\n frames: %4\n color channels: %5").arg(fn).arg(m_video_xytscaled.width()).arg(m_video_xytscaled.height()).arg(m_video_xytscaled.depth()).arg(m_video_xytscaled.spectrum()));
                       m_filename=fn;
                       m_mode=DisplayModes::loaded;
                   } else {
                       progress.close();
                       cursorGuard.resetCursor();
                       QMessageBox::critical(this, tr("Error opening video"), QString(error.c_str()));
                       m_filename="";
                       m_mode=DisplayModes::unloaded;
                   }
               }
           }
        }

    }

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
    if (ui->tabWidget->currentWidget()==ui->tabFiltering) {
        video_input=&m_video_some_frames;
    }
    //qDebug()<<"recalcCuts("<<x<<", "<<y<<")  "<<m_video_scaled.width()<<","<<m_video_scaled.height();
    if (x>=0 && x<video_input->width() && y>=0 && y<video_input->height()) {
        if (ui->tabWidget->currentWidget()==ui->tabFiltering) {
            lastX_reducedCoords=x/video_xyFactor;
            lastY_reducedCoords=y/video_xyFactor;
        } else {
            lastX_reducedCoords=x;
            lastY_reducedCoords=y;
        }

    }

    if (ui->spinAngle->value()!=angle) ui->spinAngle->setValue(angle);
    if (ui->cmbAngle->currentIndex()!=angleMode) ui->cmbAngle->setCurrentIndex(angleMode);
    recalcAndRedisplaySamples();
}

void MainWindow::recalcAndRedisplaySamples()
{
    TIME_BLOCK_SW(timer,"recalcAndRedisplaySamples()");
    QOverrideCursorGuard cursorGuard(Qt::WaitCursor);
    cimg_library::CImg<uint8_t>* video_input=&m_video_xytscaled;
    const bool isFilteringPreview=ui->tabWidget->currentWidget()==ui->tabFiltering;
    const double angle=ui->spinAngle->value();
    ProcessingTask::AngleMode angleMode=ProcessingTask::AngleMode::AngleNone;
    if (angle!=0) {
        if (ui->cmbAngle->currentIndex()==0) {
            angleMode=ProcessingTask::AngleMode::AngleRoll;
        } else if (ui->cmbAngle->currentIndex()==1) {
            angleMode=ProcessingTask::AngleMode::AnglePitch;
        }
    }
    double angleCorrected=angle;

    double xyFactor=1;
    double invxyFactor=video_xyFactor;
    double tFactor=video_everyNthFrame;
    if (isFilteringPreview) {
        video_input=&m_video_some_frames;
        xyFactor=video_xyFactor;
        invxyFactor=1;
        tFactor=1;
    } else {
        switch (angleMode) {
        case ProcessingTask::AngleMode::AnglePitch:
            // pitch angle is corrected, so the (in x/y and t differently reduced) preview-dataset yields the same results as when processing the full dataset.
            // this compensates for invxyFactor/tFactor!=1
            angleCorrected=atan(tan(angle/180.0*M_PI)*tFactor/invxyFactor)/M_PI*180.0;
            break;
        case ProcessingTask::AngleMode::AngleNone:
        case ProcessingTask::AngleMode::AngleRoll:
            // for simple roll-angles we do not have to correct, because the angle relates x and y, which are modified with the same factor!
            break;
        }
    }


    qDebug()<< "isFilteringPreview="<<isFilteringPreview<<":   lastX="<<lastX_reducedCoords<<", lastY="<<lastY_reducedCoords<<", xyFactor="<<xyFactor<<", invxyFactor="<<invxyFactor<<", tFactor="<<tFactor<<", angle="<<angle<<", angleCorrected="<<angleCorrected;
    qDebug()<< "video_input->width()="<<video_input->width()<<", video_input->height()="<<video_input->height()<<", video_input->depth()="<<video_input->depth()<<", video_input->spectrum()="<<video_input->spectrum();

    if (lastX_reducedCoords*xyFactor>=0 && lastX_reducedCoords*xyFactor<video_input->width() && lastY_reducedCoords*xyFactor>=0 && lastY_reducedCoords*xyFactor<video_input->height()) {
        TIME_BLOCK_SW(timer, "recalcAndRedisplaySamples:doRecalc()");

        QImage img=CImgToQImage(*video_input, video_input->depth()/2);
        cimg_library::CImg<uint8_t> cxz;
        cimg_library::CImg<uint8_t> cyz;

        ProcessingTask taskXZ(std::make_shared<VideoReader_CImg>(*video_input), std::make_shared<ImageWriter_CImg>(cxz, ImageWriter::FinalImage), std::make_shared<ConfigIO_Dummy>());
        { // store current settings and modify the ProcessingItem to only cover the currently selected item
            saveToTask(taskXZ, 1.0/invxyFactor, 1.0/tFactor);
            taskXZ.pis.clear();
            ProcessingTask::ProcessingItem pi;
            pi.mode=ProcessingTask::Mode::XZ;
            pi.location_x=lastX_reducedCoords*xyFactor;
            pi.location_y=lastY_reducedCoords*xyFactor;
            pi.angle=angleCorrected;
            pi.angleMode=angleMode;
            taskXZ.pis.push_back(pi);
            if (!isFilteringPreview) taskXZ.filterNotch=false;
            if (isFilteringPreview) taskXZ.filterNotch=ui->chkWavelength->isChecked();
            taskXZ.do_not_save_anyting=true;
        }

        ProcessingTask taskYZ(std::make_shared<VideoReader_CImg>(*video_input), std::make_shared<ImageWriter_CImg>(cyz, ImageWriter::FinalImage), std::make_shared<ConfigIO_Dummy>());
        { // store current settings and modify the ProcessingItem to only cover the currently selected item
            saveToTask(taskYZ, 1.0/invxyFactor, 1.0/tFactor);
            taskYZ.pis.clear();
            ProcessingTask::ProcessingItem pi;
            pi.mode=ProcessingTask::Mode::ZY;
            pi.location_x=lastX_reducedCoords*xyFactor;
            pi.location_y=lastY_reducedCoords*xyFactor;
            pi.angle=angleCorrected;
            pi.angleMode=angleMode;
            taskYZ.pis.push_back(pi);
            if (!isFilteringPreview) taskYZ.filterNotch=false;
            if (isFilteringPreview) taskXZ.filterNotch=ui->chkWavelength->isChecked();
            taskYZ.do_not_save_anyting=true;
        }

        {
            TIME_BLOCK_SW(timer, "taskXZ.process()");
            taskXZ.process();
        }
        {
            TIME_BLOCK_SW(timer, "taskYZ.process()");
            taskYZ.process();
        }



        QImage imgxz;
        {
            TIME_BLOCK_SW(timer, "CImgToQImage(cxz)");
            imgxz=CImgToQImage(cxz);
        }

        QImage imgyz;
        {
            TIME_BLOCK_SW(timer, "CImgToQImage(cyz)");
            imgyz=CImgToQImage(cyz);
        }
        {
            TIME_BLOCK_SW(timer3, "paint lines")
            {

                QPainter pnt(&img);
                pnt.setPen(QPen(QColor("red")));
                if (angleMode==ProcessingTask::AngleMode::AngleNone || angleMode==ProcessingTask::AngleMode::AngleRoll) {
                    pnt.save();
                    pnt.translate(lastX_reducedCoords,lastY_reducedCoords);
                    pnt.rotate(angle);
                    const int l=2*std::max(img.width(), img.height());
                    pnt.drawLine(-l,0,l,0);
                    pnt.drawLine(0,-l,0,l);
                    pnt.restore();
                } else {
                    pnt.drawLine(0, lastY_reducedCoords*xyFactor,img.width(),lastY_reducedCoords*xyFactor);
                    pnt.drawLine(lastX_reducedCoords*xyFactor, 0,lastX_reducedCoords*xyFactor, img.height());
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
        }

        {
            TIME_BLOCK_SW(timer, "update GUI")
            labXY->setPixmap(QPixmap::fromImage(img));
            labXZ->setPixmap(QPixmap::fromImage(imgxz));
            labYZ->setPixmap(QPixmap::fromImage(imgyz));
        }
    }
}

void MainWindow::ImageClicked(int x, int y)
{
    if (ui->tabWidget->currentWidget()==ui->tabNormalize) {
        if (ui->chkNormalize->isChecked()) {
            ui->spinNormalizeX->setValue(x*video_xyFactor);
            ui->spinNormalizeY->setValue(y*video_xyFactor);
            recalcAndRedisplaySamples();
        }
    } else if (ui->tabWidget->currentWidget()==ui->tabColor) {
        if (ui->chkModifyWhitepoint->isChecked()) {
            //qDebug()<<m_video_xytscaled.width()<<m_video_xytscaled.height()<<m_video_xytscaled.depth()<<m_video_xytscaled.spectrum();
            ui->spinWhitepointR->setValue(m_video_xytscaled.atXYZC(x,y,0,0,255));
            ui->spinWhitepointG->setValue(m_video_xytscaled.atXYZC(x,y,0,1,255));
            ui->spinWhitepointB->setValue(m_video_xytscaled.atXYZC(x,y,0,2,255));
            recalcAndRedisplaySamples();
        }
    } else /*if (ui->tabWidget->currentWidget()==ui->tabCuts)*/ {
        recalcAndRedisplaySamples(x,y,ui->spinAngle->value(), ui->cmbAngle->currentIndex());
    }
}

void MainWindow::on_btnAddXZ_clicked()
{
    if (lastY_reducedCoords<0) return;
    ProcessingTask::ProcessingItem item;
    item.mode=ProcessingTask::Mode::XZ;
    item.location_x=lastX_reducedCoords*video_xyFactor;
    item.location_y=lastY_reducedCoords*video_xyFactor;
    item.angle=ui->spinAngle->value();
    item.angleMode=ProcessingTask::AngleMode::AngleNone;
    if (ui->cmbAngle->currentIndex()==0) item.angleMode=ProcessingTask::AngleMode::AngleRoll;
    if (ui->cmbAngle->currentIndex()==1) item.angleMode=ProcessingTask::AngleMode::AnglePitch;
    m_procModel->addItem(item);
}

void MainWindow::on_btnAddZY_clicked()
{
    if (lastX_reducedCoords<0) return;
    ProcessingTask::ProcessingItem item;
    item.mode=ProcessingTask::Mode::ZY;
    item.location_x=lastX_reducedCoords*video_xyFactor;
    item.location_y=lastY_reducedCoords*video_xyFactor;
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


void MainWindow::processINIFile()
{
    QString fn=QFileDialog::getOpenFileName(this, tr("Save Configuration File ..."), m_settings.value("lastIniDir", "").toString(), tr("INI-File (*.ini)"));
    if (fn.size()>0) {
        m_settings.setValue("lastIniDir", QFileInfo(fn).absolutePath());
        ProcessingTask* task=new ProcessingTask(std::make_shared<VideoReader_FFMPEG>(), std::make_shared<ImageWriter_PNG>(), std::make_shared<ConfigIO_INI>());
        task->load(fn);

        ProcessingThread* thr=new ProcessingThread(task, this);
        ui->widProcessing->push_back(thr);
    }
}

void MainWindow::processAll()
{
    if (m_filename.size()<=0) return;
    if (m_procModel->rowCount()<=0) return;

    ProcessingTask* task=new ProcessingTask(std::make_shared<VideoReader_FFMPEG>(), std::make_shared<ImageWriter_PNG>(), std::make_shared<ConfigIO_INI>());
    saveToTask(*task);

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

