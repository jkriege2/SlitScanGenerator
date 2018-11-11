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
    ui->labVersion->setText(tr("%2, %1-bits").arg(sizeof(void*)*8).arg(PROJECT_VERSION));
}

AboutBox::~AboutBox()
{
    delete ui;
}

void AboutBox::openURL(const QUrl &url)
{
    QDesktopServices::openUrl(url);
}
