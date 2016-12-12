#include "aboutbox.h"
#include "ui_aboutbox.h"
#include <QDesktopServices>

AboutBox::AboutBox(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutBox)
{
    ui->setupUi(this);
    connect(ui->labelText, SIGNAL(linkActivated(QString)), this, SLOT(openURL(QUrl)));
    ui->labVersion->setText(tr("%1-bits").arg(sizeof(void*)*8));
}

AboutBox::~AboutBox()
{
    delete ui;
}

void AboutBox::openURL(const QUrl &url)
{
    QDesktopServices::openUrl(url);
}
