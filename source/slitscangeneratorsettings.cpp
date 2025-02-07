#include "slitscangeneratorsettings.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QStyleFactory>

SlitScanGeneratorSettings::SlitScanGeneratorSettings():
    QSettings(QSettings::UserScope, "jkrieger.de", "SlitScanGenerator")
{

}

void SlitScanGeneratorSettings::loadCurrentStylesheet()
{
    loadCurrentStyle();
    SlitScanGeneratorSettings settings;
    QFile f(settings.value("Stylesheet", ":/stylesheets/default.qss").toString());
    if (f.open(QFile::ReadOnly)) {
        QTextStream stream(&f);
        qApp->setStyleSheet(stream.readAll());
    }
}

void SlitScanGeneratorSettings::loadCurrentStyle()
{
    SlitScanGeneratorSettings settings;
    qApp->setStyle(QStyleFactory::create(settings.value("Style", "Fusion").toString()));
}
