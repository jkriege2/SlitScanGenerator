#include "imagewriter_png.h"
#include <QFileInfo>
#include <QFile>
#include <QString>
#include <QImage>
#include <QDir>
#include "cimg_tools.h"

std::string ImageWriter_PNG::saveImage(const std::string &filename, ImageType /*type*/, const cimg_library::CImg<uint8_t> &frame, int /*quality*/) const
{
    const QFileInfo fi(filename.c_str());
    const QString fn=fi.absoluteDir().absoluteFilePath(fi.baseName()+".png");
    const QImage img=CImgToQImage(frame);
    if (QFile::exists(fn)) QFile::remove(fn);
    qDebug()<<"  SAVING PNG "<<fn;
    const bool ok=img.save(fn, "PNG");
    qDebug()<<"    --> "<<ok;
    return fn.toStdString();
}
