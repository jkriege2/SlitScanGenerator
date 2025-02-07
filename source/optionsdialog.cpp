#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include "slitscangeneratorsettings.h"
#include "ffmpeg_tools.h"
#include <QThread>
#include <QStyleFactory>
#include "slitscangeneratorsettings.h"

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);

    ui->cmbStyle->clear();
    bool first=true;
    for (const auto& style: QStyleFactory::keys()) {
        ui->cmbStyle->addItem(style);
        if (style.toLower().trimmed()=="fusion") ui->cmbStyle->setCurrentIndex(ui->cmbStyle->count()-1);
        else if (first) ui->cmbStyle->setCurrentIndex(0);
        first=false;
    }


    ui->cmbStylesheets->clear();
    ui->cmbStylesheets->addItem(tr("Default"), ":/stylesheets/default.qss");
    ui->cmbStylesheets->addItem(tr("Dark Orange"), ":/stylesheets/darkorange.qss");
    ui->cmbStylesheets->setCurrentIndex(0);

    ui->cmbFFMPEGHWAccell->clear();
    ui->cmbFFMPEGHWAccell->addItems(listFFMPEGHWAccelOptions());
    ui->cmbFFMPEGHWAccell->setCurrentIndex(0);
    loadSettings();

    connect(this, SIGNAL(accepted()), this, SLOT(storeSettings()));
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}



void OptionsDialog::storeSettings()
{
    SlitScanGeneratorSettings settings;
    settings.setValue("numberOfParallelTasks", ui->spinParalleltasks->value());
    settings.setValue("Stylesheet", ui->cmbStylesheets->currentData().toString());
    settings.setValue("Style", ui->cmbStyle->currentText());
    settings.setValue("ffmpeg_threads", ui->spinFFMPEGThreads->value());
    settings.setValue("ffmpeg_accel", ui->cmbFFMPEGHWAccell->currentText());
}

void OptionsDialog::loadSettings()
{
    SlitScanGeneratorSettings settings;
    ui->spinParalleltasks->setRange(1, QThread::idealThreadCount()*2);
    ui->spinParalleltasks->setValue(settings.value("numberOfParallelTasks", QThread::idealThreadCount()/2).toInt());
    ui->cmbStylesheets->setCurrentIndex(ui->cmbStylesheets->findData(settings.value("Stylesheet", ":/stylesheets/default.qss").toString()));
    if (ui->cmbStylesheets->currentIndex()<0) ui->cmbStylesheets->setCurrentIndex(0);
    ui->cmbStyle->setCurrentIndex(ui->cmbStyle->findText(settings.value("Style", "Fusion").toString(), Qt::MatchExactly));
    if (ui->cmbStyle->currentIndex()<0) ui->cmbStyle->setCurrentIndex(0);
    ui->cmbFFMPEGHWAccell->setCurrentIndex(ui->cmbFFMPEGHWAccell->findText(settings.value("ffmpeg_accel", "auto").toString()));
    if (ui->cmbFFMPEGHWAccell->currentIndex()<0) ui->cmbFFMPEGHWAccell->setCurrentIndex(0);
    ui->spinFFMPEGThreads->setValue(settings.value("ffmpeg_threads", 0).toInt());

}
