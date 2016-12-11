#include "aboutbox.h"
#include "ui_aboutbox.h"
#include <QDesktopServices>

AboutBox::AboutBox(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutBox)
{
    ui->setupUi(this);
    connect(ui->labelText, SIGNAL(openURL(QUrl)), this, SLOT(openURL(QUrl)));
}

AboutBox::~AboutBox()
{
    delete ui;
}

void AboutBox::openURL(const QUrl &url)
{
    QDesktopServices::openUrl(url);
}
