#include "mainwindow.h"
#include <QApplication>
#include <QStringList>

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
    MainWindow w;
    w.show();

    return a.exec();
}
