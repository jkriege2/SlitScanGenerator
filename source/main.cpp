#include "mainwindow.h"
#include <QApplication>
#include <QStringList>
#include <QTranslator>
#include "slitscangeneratorsettings.h"

int main(int argc, char *argv[])
{
    QStringList paths=QCoreApplication::libraryPaths();
    paths.prepend("./");
    paths.prepend("./plugins/");
    QCoreApplication::setLibraryPaths(paths);
    QApplication a(argc, argv);
    paths.prepend(QCoreApplication::applicationDirPath());
    paths.prepend(QCoreApplication::applicationDirPath()+"/plugins/");
    QCoreApplication::setLibraryPaths(paths);

    SlitScanGeneratorSettings::loadCurrentStylesheet();

    bool bTranslationLoaded = false;
    QTranslator translator;
    const char *aszTranslationDirs[] =
        {
            ".", "../translations", ":/translations", 0
        };
    for (const char **ppszDir = aszTranslationDirs; *ppszDir != 0; ++ ppszDir)
    {
        bTranslationLoaded = translator.load(QLocale::system(), "example-10", "_", *ppszDir);
        if (bTranslationLoaded)
            break;
    }

    MainWindow w;
    //w.setStyleSheet(a.styleSheet());
    w.show();

    return a.exec();
}
