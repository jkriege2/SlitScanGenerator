#include "slitscangeneratorsettings.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>

SlitScanGeneratorSettings::SlitScanGeneratorSettings():
    QSettings(QSettings::UserScope, "jkrieger.de", "SlitScanGenerator")
{

}

void SlitScanGeneratorSettings::loadCurrentStylesheet()
{
    SlitScanGeneratorSettings settings;
    QFile f(settings.value("Stylesheet", ":/stylesheets/default.qss").toString());
    if (f.open(QFile::ReadOnly)) {
        QTextStream stream(&f);
        qApp->setStyleSheet(stream.readAll());
    }
}
