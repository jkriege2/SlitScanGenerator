#include "mainwindow.h"
#include <QApplication>
#include <QStringList>
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

    MainWindow w;
    //w.setStyleSheet(a.styleSheet());
    w.show();

    return a.exec();
}
