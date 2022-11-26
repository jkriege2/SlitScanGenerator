#include "aboutbox.h"
#include "ui_aboutbox.h"
#include <QDesktopServices>
#include "defines.h"
#include <libavcodec/version.h>
#include <libavformat/version.h>
#include <libswscale/version.h>
#include <libavutil/version.h>
#include <CImg.h>

QString CImgVersion() {
    QString s;
    s+=QString::number(cimg_version/100);
    s+=".";
    s+=QString::number((cimg_version/10)%10);
    s+=".";
    s+=QString::number(cimg_version%10);
    return s;
}

AboutBox::AboutBox(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutBox)
{
    ui->setupUi(this);
    connect(ui->labelText, SIGNAL(linkActivated(QString)), this, SLOT(openURL(QUrl)));
    ui->labVersion->setText(tr("%2, %1-bits, %4\nbuild timestamp: %3").arg(sizeof(void*)*8).arg(PROJECT_VERSION).arg(PROJECT_BUILDATE).arg(PROJECT_BUILDTYPE));
    ui->labCopyright->setText(tr("%1").arg(PROJECT_COPYRIGHT));
    ui->labLibVersions->setText(tr("<center>Qt %1 (<a href=\"https://www.qt.io/\">https://www.qt.io/</a>) <br> CImg %2 (<a href=\"http://cimg.eu/\">http://cimg.eu/</a>) <br> FFMPEG (%3, %4, %5, %6) (<a href=\"https://ffmpeg.org/\">https://ffmpeg.org/</a>)</center>")
                                .arg(qVersion()).arg(CImgVersion()).arg(LIBAVCODEC_IDENT).arg(LIBAVFORMAT_IDENT).arg(LIBAVUTIL_IDENT).arg(LIBSWSCALE_IDENT));
}

AboutBox::~AboutBox()
{
    delete ui;
}

void AboutBox::openURL(const QUrl &url)
{
    QDesktopServices::openUrl(url);
}
