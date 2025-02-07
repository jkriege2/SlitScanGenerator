#ifndef SLITSCANGENERATORSETTINGS_H
#define SLITSCANGENERATORSETTINGS_H

#include <QSettings>

class SlitScanGeneratorSettings : public QSettings
{
public:
    SlitScanGeneratorSettings();

    static void loadCurrentStylesheet();
    static void loadCurrentStyle();
};

#endif // SLITSCANGENERATORSETTINGS_H
