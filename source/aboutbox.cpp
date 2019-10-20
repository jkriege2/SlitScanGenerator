#include "aboutbox.h"
#include "ui_aboutbox.h"
#include <QDesktopServices>
#include "defines.h"

AboutBox::AboutBox(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutBox)
{
    ui->setupUi(this);
    connect(ui->labelText, SIGNAL(linkActivated(QString)), this, SLOT(openURL(QUrl)));
    ui->labVersion->setText(tr("%2, %1-bits, %4\nbuild timestamp: %3").arg(sizeof(void*)*8).arg(PROJECT_VERSION).arg(PROJECT_BUILDATE).arg(PROJECT_BUILDTYPE));
    ui->labCopyright->setText(tr("%1").arg(PROJECT_COPYRIGHT));
}

AboutBox::~AboutBox()
{
    delete ui;
}

void AboutBox::openURL(const QUrl &url)
{
    QDesktopServices::openUrl(url);
}
