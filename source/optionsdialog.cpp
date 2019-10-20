#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include "slitscangeneratorsettings.h"
#include <QThread>

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);

    ui->cmbStylesheets->clear();
    ui->cmbStylesheets->addItem(tr("Default"), ":/stylesheets/default.qss");
    ui->cmbStylesheets->addItem(tr("Dark Orange"), ":/stylesheets/darkorange.qss");

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
}

void OptionsDialog::loadSettings()
{
    SlitScanGeneratorSettings settings;
    ui->spinParalleltasks->setRange(1, QThread::idealThreadCount()*2);
    ui->spinParalleltasks->setValue(settings.value("numberOfParallelTasks", QThread::idealThreadCount()-1).toInt());
    ui->cmbStylesheets->setCurrentIndex(ui->cmbStylesheets->findData(settings.value("Stylesheet", ":/stylesheets/default.qss").toString()));
    if (ui->cmbStylesheets->currentIndex()<0) ui->cmbStylesheets->setCurrentIndex(0);

}
