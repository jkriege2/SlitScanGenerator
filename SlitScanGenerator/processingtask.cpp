#include "processingtask.h"
#include "cimg_tools.h"
#include <QFileInfo>
#include <QFile>
#include <QSettings>
#include <QDir>
#include <QDebug>

ProcessingTask::ProcessingTask():
    filename(),
    outputFrames(0),
    vid(nullptr),
    z(0),
    m_saving(false),
    m_savingFrame(0),
    stillCnt(0),
    stillDelta(60),
    stillStrip(true),
    stillSeparateFiles(false),
    stills(0),
    normalize(false),
    normalizeX(-1),
    normalizeY(-1)
{

}

bool ProcessingTask::processInit(int &prog, int &maxProg, QString &message, QString &error)
{
    prog=0;
    maxProg=100;
    message=QObject::tr("loading video ...");
    error="";
    z=0;
    m_saving=false;
    m_savingFrame=0;

    QFileInfo fi(filename);
    QString allini=fi.absoluteDir().absoluteFilePath(QString("%1.ini").arg(fi.baseName()));
    QSettings setall(allini, QSettings::IniFormat);
    setall.setValue("count", pis.size());
    setall.setValue("input_file", filename);
    setall.setValue("stills/count", stillCnt);
    setall.setValue("stills/delta", stillDelta);
    setall.setValue("stills/strip", stillStrip);
    setall.setValue("stills/separate_files", stillSeparateFiles);

    setall.setValue("normalize/enabled", normalize);
    setall.setValue("normalize/x", normalizeX);
    setall.setValue("normalize/y", normalizeY);

    setall.setValue("filter/notch/enabled", filterNotch);
    setall.setValue("filter/notch/wavelength", fiterNotchWavelength);
    setall.setValue("filter/notch/delta", fiterNotchWidth);

    int j=0;
    for (ProcessingTask::ProcessingItem pi: pis) {
        if (pi.mode==Mode::ZY) {
            setall.setValue(QString("item%1/mode").arg(j,3,10,QChar('0')), "ZY");
        }
        if (pi.mode==Mode::XZ) {
            setall.setValue(QString("item%1/mode").arg(j,3,10,QChar('0')), "XZ");
        }
        setall.setValue(QString("item%1/location").arg(j,3,10,QChar('0')), pi.location);
        j++;
    }

    std::string err;
    vid=openFFMPEGVideo(filename.toStdString(), &err);
    if (vid && readFFMPEGFrame(frame, vid)) {
        maxProg=pis.size()+2+getFrameCount(vid);
        prog=1;
        int j=0;
        for (ProcessingTask::ProcessingItem pi: pis) {
            if (pi.mode==Mode::ZY) {
                results.push_back(cimg_library::CImg<uint8_t>());
                results[j].assign(outputFrames, frame.height(), 1, 3);
            } else if (pi.mode==Mode::XZ) {
                results.push_back(cimg_library::CImg<uint8_t>());
                results[j].assign(frame.width(), outputFrames, 1, 3);
            }
            QFileInfo fi(filename);
            QString fn=fi.absoluteDir().absoluteFilePath(QString("%1_stack%2.png").arg(fi.baseName()).arg(j+1, 3, 10,QChar('0')));
            QString fnini=fi.absoluteDir().absoluteFilePath(QString("%1_stack%2.ini").arg(fi.baseName()).arg(j+1, 3, 10,QChar('0')));
            result_filenames.push_back(fn);
            result_inifilenames.push_back(fnini);
            setall.setValue(QString("item%1/file").arg(j,3,10,QChar('0')), QFileInfo(allini).absoluteDir().relativeFilePath(QFileInfo(fn).absoluteFilePath()));
            if (stillStrip && stillCnt>0) {
                stillStripImg.push_back(cimg_library::CImg<uint8_t>());
                stillStripImg[j].resize(frame.width()+10, 5+(frame.height()+5)*stillCnt, 1, 3);
            }
            j++;
        }

        return true;
    } else {
        error=err.c_str();
        return false;
    }
}

bool ProcessingTask::processStep(int &prog, int &maxProg, QString &message)
{
    prog++;
    maxProg=pis.size()+2+getFrameCount(vid);
    if (!m_saving) {
        // process old frame
        message=QObject::tr("processing frame %1/%2 ...").arg(z+1).arg(outputFrames);
        for (int j=0; j<results.size(); j++) {
            ProcessingTask::ProcessingItem pi=pis[j];
            if (pi.mode==Mode::ZY && z<results[j].width()) {
                for (int y=0; y<std::min(frame.height(),results[j].height()); y++)
                {                    
                    results[j](z, y, 0,0)=frame(pi.location, y, 0, 0);
                    results[j](z, y, 0,1)=frame(pi.location, y, 0, 1);
                    results[j](z, y, 0,2)=frame(pi.location, y, 0, 2);
                }
            } else if (pi.mode==Mode::XZ && z<results[j].height()) {
                for (int x=0; x<std::min(frame.width(),results[j].width()); x++)
                {
                    results[j](x, z, 0,0)=frame(x, pi.location,  0, 0);
                    results[j](x, z, 0,1)=frame(x, pi.location,  0, 1);
                    results[j](x, z, 0,2)=frame(x, pi.location,  0, 2);
                }
            }

            // process stills
            if (stills<stillCnt && z%stillDelta==0) {
                auto frame_s=frame;
                const unsigned char color[] = { 255,0,0 };
                if (pi.mode==Mode::ZY && z<results[j].width()) {
                    frame_s.draw_line(pi.location,0,pi.location,frame.height(), color);
                } else if (pi.mode==Mode::XZ && z<results[j].height()) {
                    frame_s.draw_line(0,pi.location,frame.width(),pi.location, color);
                }
                if (stillSeparateFiles) {
                    QFileInfo fi(filename);
                    QString fn=fi.absoluteDir().absoluteFilePath(QString("%1_stack%3_still%2.png").arg(fi.baseName()).arg(z+1, 3, 10,QChar('0')).arg(j+1, 3, 10,QChar('0')));
                    QImage img=CImgToQImage(frame_s);
                    img.save(fn);
                }
                if (stillStrip) {
                    cimg_forXY(frame,x,y) {
                        stillStripImg[j](5+x,5+y+stills*(5+frame.height()),0,0)=frame_s(x,y,0,0);
                        stillStripImg[j](5+x,5+y+stills*(5+frame.height()),0,1)=frame_s(x,y,0,1);
                        stillStripImg[j](5+x,5+y+stills*(5+frame.height()),0,2)=frame_s(x,y,0,2);
                    }
                }
            }
        }
        if (stills<stillCnt && z%stillDelta==0) {
            stills++;
        }

        // load next frame (if available)
        message=QObject::tr("processing frame %1/%2 ...").arg(z+1).arg(outputFrames);
        bool res=readFFMPEGFrame(frame, vid);
        z++;
        if (!res) {
            m_saving=true;
            m_savingFrame=0;
        }
        return true;
    } else {
        // 2 Step: saving frames
        if (static_cast<int>(m_savingFrame)<results.size()) {
            const ProcessingTask::ProcessingItem& pi=pis[m_savingFrame];
            // normalize image if necessary
            if (normalize) {
                if (pi.mode==Mode::ZY && normalizeY>=0 && normalizeY<results[m_savingFrame].height()) {
                    normalizeZY(results[m_savingFrame], normalizeY);
                }
                if (pi.mode==Mode::XZ && normalizeX>=0 && normalizeX<results[m_savingFrame].width()) {
                    normalizeXZ(results[m_savingFrame], normalizeX);
                }
            }

            QString fn=result_filenames[m_savingFrame];
            QString fnini=result_inifilenames[m_savingFrame];
            message=QObject::tr("saved result %2: '%1' ...").arg(fn).arg(m_savingFrame+1);
            QImage img=CImgToQImage(results[m_savingFrame]);
            img.save(fn);
            QSettings set(fnini, QSettings::IniFormat);
            if (pi.mode==Mode::ZY) {
                set.setValue("mode", "ZY");
            }
            if (pi.mode==Mode::XZ) {
                set.setValue("mode", "XZ");
            }
            set.setValue("location", pi.location);
            set.setValue("stills/count", stillCnt);
            set.setValue("stills/delta", stillDelta);
            set.setValue("stills/strip", stillStrip);
            set.setValue("stills/separate_files", stillSeparateFiles);
            set.setValue("normalize/enabled", normalize);
            set.setValue("normalize/x", normalizeX);
            set.setValue("normalize/y", normalizeY);
            set.setValue("filter/notch/enabled", filterNotch);
            set.setValue("filter/notch/wavelength", fiterNotchWavelength);
            set.setValue("filter/notch/delta", fiterNotchWidth);

            QImage imgs=CImgToQImage(stillStripImg[m_savingFrame]);
            QFileInfo fi(filename);
            QString fns=fi.absoluteDir().absoluteFilePath(QString("%1_stack%2_stillstrip.png").arg(fi.baseName()).arg(m_savingFrame+1, 3, 10,QChar('0')));
            imgs.save(fns);

        }

        m_savingFrame++;
        return (static_cast<int>(m_savingFrame)<results.size());
    }
}

void ProcessingTask::processFinalize()
{
    closeFFMPEGVideo(vid);
}

void ProcessingTask::normalizeZY(cimg_library::CImg<uint8_t> &img, int normalizeY)
{
    if (normalizeY>=0 && normalizeY<img.height()) {
        cimg_library::CImg<uint8_t> img_in=img;
        //qDebug()<<"img="<<img.width()<<"x"<<img.height()<<"x"<<img.depth()<<"x"<<img.spectrum();
        cimg_forC(img,c) {
            const auto ch=img_in.get_channel(c);
            const auto row=ch.get_row(normalizeY);
            const double avg0=row.mean();
            //qDebug()<<"c="<<c<<"  ch="<<ch.width()<<"x"<<ch.height()<<"x"<<ch.depth()<<"x"<<ch.spectrum()<<"  row="<<row.width()<<"x"<<row.height()<<"x"<<row.depth()<<"x"<<row.spectrum()<<"  => avg0="<<avg0;
            cimg_forXYZ(img,x,y,z) {
                const double v=double(img(x,y,z,c))*avg0/double(img_in(x,normalizeY,z,c));
                img(x,y,z,c)=(v<0)?0:((v>255)?255:v);
            }
        }
    }
}

void ProcessingTask::normalizeXZ(cimg_library::CImg<uint8_t> &img, int normalizeX)
{
    if (normalizeX>=0 && normalizeX<img.width()) {
        cimg_library::CImg<uint8_t> img_in=img;
        cimg_forC(img,c) {
            const auto ch=img_in.get_channel(c);
            const auto col=ch.get_column(normalizeX);
            const double avg0=col.mean();
            cimg_forXYZ(img,x,y,z) {
                const double v=double(img(x,y,z,c))*avg0/double(img_in(normalizeX,y,z,c));
                img(x,y,z,c)=(v<0)?0:((v>255)?255:v);
            }
        }
    }
}

void ProcessingTask::applyFilterNotch(cimg_library::CImg<uint8_t> &imgrgb, double center, double delta)
{
    cimg_forC(imgrgb, c) {
        cimg_library::CImg<uint8_t> img_in=imgrgb.get_channel(c);

        int nx=ceil(log(img_in.width())/log(2));
        int ny=ceil(log(img_in.height())/log(2));
        cimg_library::CImg<uint8_t> img(pow(2,nx), pow(2,ny),1,3,0);
        int offx=(img.width()-img_in.width())/2;
        int offy=(img.height()-img_in.height())/2;
        cimg_forXYC(img,x,y,c) {
            img(offx+x,offy+y,0,c)=img_in(x,y,0,c);
        }

        cimg_library::CImgList<> F = img.get_FFT();
        cimglist_apply(F,shift)(img.width()/2,img.height()/2,0,0,2);
        cimg_library::CImg<unsigned char> mask(img.width(),img.height(),1,1,1);
        unsigned char one[] = { 1 }, zero[] = { 0 };
        mask.fill(0).draw_circle(img.width()/2,img.height()/2,center+delta/2.0,zero).
                    draw_circle(img.width()/2,img.height()/2,center-delta/2.0,one);
        cimg_library::CImgList<> nF(F);
        cimglist_for(F,l) nF[l].mul(mask).shift(-img.width()/2,-img.height()/2,0,0,2);
        cimg_library::CImg<uint8_t> r = nF.FFT(true)[0].normalize(0,255);
        cimg_forXY(r,x,y) {
            imgrgb(std::max<int>(0,x-offx),std::max<int>(0,y-offy),0,c)=r(x,y);
        }
    }
}
