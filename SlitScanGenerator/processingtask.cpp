#include "processingtask.h"
#include "cimg_tools.h"
#include <QFileInfo>
#include <QFile>
#include <QSettings>
#include <QDir>

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
                    auto avg0=results[m_savingFrame].get_row(normalizeY).get_channel(0).mean();
                    auto avg1=results[m_savingFrame].get_row(normalizeY).get_channel(1).mean();
                    auto avg2=results[m_savingFrame].get_row(normalizeY).get_channel(2).mean();

                    cimg_forXY(results[m_savingFrame],x,y) {
                        results[m_savingFrame](x,y,0,0)=results[m_savingFrame](x,y,0,0)*avg0/results[m_savingFrame](x,normalizeY,0,0);
                        results[m_savingFrame](x,y,0,1)=results[m_savingFrame](x,y,0,1)*avg1/results[m_savingFrame](x,normalizeY,0,1);
                        results[m_savingFrame](x,y,0,2)=results[m_savingFrame](x,y,0,2)*avg2/results[m_savingFrame](x,normalizeY,0,2);
                    }
                }
                if (pi.mode==Mode::XZ && normalizeX>=0 && normalizeX<results[m_savingFrame].width()) {
                    auto avg0=results[m_savingFrame].get_column(normalizeX).get_channel(0).mean();
                    auto avg1=results[m_savingFrame].get_column(normalizeX).get_channel(1).mean();
                    auto avg2=results[m_savingFrame].get_column(normalizeX).get_channel(2).mean();

                    cimg_forXY(results[m_savingFrame],x,y) {
                        results[m_savingFrame](x,y,0,0)=results[m_savingFrame](x,y,0,0)*avg0/results[m_savingFrame](normalizeX,y,0,0);
                        results[m_savingFrame](x,y,0,1)=results[m_savingFrame](x,y,0,1)*avg1/results[m_savingFrame](normalizeX,y,0,1);
                        results[m_savingFrame](x,y,0,2)=results[m_savingFrame](x,y,0,2)*avg2/results[m_savingFrame](normalizeX,y,0,2);
                    }
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
