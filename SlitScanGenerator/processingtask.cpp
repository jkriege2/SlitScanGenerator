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
    setall.setValue("stills/gap", stillGap);
    setall.setValue("stills/border", stillBorder);
    setall.setValue("stills/line_width", stillLineWidth);

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
        setall.setValue(QString("item%1/location_x").arg(j,3,10,QChar('0')), pi.location_x);
        setall.setValue(QString("item%1/location_y").arg(j,3,10,QChar('0')), pi.location_y);
        setall.setValue(QString("item%1/angle_mode").arg(j,3,10,QChar('0')), static_cast<int>(pi.angleMode));
        setall.setValue(QString("item%1/angle").arg(j,3,10,QChar('0')), pi.angle);
        j++;
    }

    std::string err;
    vid=openFFMPEGVideo(filename.toStdString(), &err);
    if (vid && readFFMPEGFrame(frame, vid)) {
        int still_b=stillBorder/100.0*frame.width();
        int still_g=stillGap/100.0*frame.height();
        maxProg=pis.size()+2+getFrameCount(vid);
        prog=1;
        int j=0;
        for (ProcessingTask::ProcessingItem pi: pis) {
            cimg_library::CImg<uint8_t> line;
            bool zok=true;
            int zout=0;
            int len=outputFrames;
            if (pi.mode==Mode::ZY) {
                if (pi.angleMode==AngleMode::AngleNone || pi.angle==0) {
                    line=extractZY_atz(0, frame, pi.location_x);
                } else if (pi.angleMode==AngleMode::AngleRoll) {
                    line=extractZY_atz_roll(0, outputFrames, frame, pi.location_x, pi.location_y, pi.angle);
                } else if (pi.angleMode==AngleMode::AnglePitch) {
                    line=extractZY_atz_pitch(0, outputFrames, frame, pi.location_x, pi.angle, zout, &len);
                }
                results.push_back(cimg_library::CImg<uint8_t>());
                results[j].assign(len, line.width(), 1, 3);
                qDebug()<<"output size: "<<results[j].width()<<"x"<<results[j].height()<<"x"<<results[j].depth()<<"x"<<results[j].spectrum();
                zs_vals.push_back(0);
            } else if (pi.mode==Mode::XZ) {
                if (pi.angleMode==AngleMode::AngleNone || pi.angle==0) {
                    line=extractXZ_atz(0, frame, pi.location_x);
                } else if (pi.angleMode==AngleMode::AngleRoll) {
                    line=extractXZ_atz_roll(0, outputFrames, frame, pi.location_x, pi.location_y, pi.angle);
                } else if (pi.angleMode==AngleMode::AnglePitch) {
                    line=extractXZ_atz_pitch(0, outputFrames, frame, pi.location_y, pi.angle, zout, &len);
                }
                results.push_back(cimg_library::CImg<uint8_t>());
                results[j].assign(line.width(), len, 1, 3);
                qDebug()<<"output size: "<<results[j].width()<<"x"<<results[j].height()<<"x"<<results[j].depth()<<"x"<<results[j].spectrum();
                zs_vals.push_back(0);
            }
            QFileInfo fi(filename);
            QString fn=fi.absoluteDir().absoluteFilePath(QString("%1_stack%2.png").arg(fi.baseName()).arg(j+1, 3, 10,QChar('0')));
            QString fnfilt=fi.absoluteDir().absoluteFilePath(QString("%1_stack%2_filtered.png").arg(fi.baseName()).arg(j+1, 3, 10,QChar('0')));
            QString fnini=fi.absoluteDir().absoluteFilePath(QString("%1_stack%2.ini").arg(fi.baseName()).arg(j+1, 3, 10,QChar('0')));
            result_filenames.push_back(fn);
            resultfilt_filenames.push_back(fnfilt);
            result_inifilenames.push_back(fnini);
            setall.setValue(QString("item%1/file").arg(j,3,10,QChar('0')), QFileInfo(allini).absoluteDir().relativeFilePath(QFileInfo(fn).absoluteFilePath()));
            if (stillStrip && stillCnt>0) {
                stillStripImg.push_back(cimg_library::CImg<uint8_t>());
                stillStripImg[j].resize(frame.width()+2*still_b, still_b+(frame.height()+still_g)*stillCnt-still_g+still_b, 1, 3);
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
        int still_b=stillBorder/100.0*frame.width();
        int still_g=stillGap/100.0*frame.height();
        int stilllw=std::max<int>(1,stillLineWidth/100.0*frame.width());

        for (int j=0; j<results.size(); j++) {
            ProcessingTask::ProcessingItem pi=pis[j];

            cimg_library::CImg<uint8_t> line;
            if (pi.mode==Mode::ZY) {
                int z0=zs_vals[j];
                if (pi.angleMode==AngleMode::AngleNone || pi.angle==0) {
                    line=extractZY_atz(z, frame, pi.location_x);
                    zs_vals[j]++;
                } else if (pi.angleMode==AngleMode::AngleRoll) {
                    line=extractZY_atz_roll(z, outputFrames, frame, pi.location_x, pi.location_y, pi.angle);
                    zs_vals[j]++;
                } else if (pi.angleMode==AngleMode::AnglePitch) {
                    line=extractZY_atz_pitch(z, outputFrames, frame, pi.location_x, pi.angle, zs_vals[j]);
                    qDebug()<<"extractZY_atz_pitch: z="<<z<<", zs_vals[j]="<<zs_vals[j]<<", line.height="<<line.height();
                }
                /*if (z0+line.height()-1>=results[j].width()) {
                    // resize of necessary
                    qDebug()<<"resize "<<results[j].width()<<"x"<<results[j].height()<<"x"<<results[j].depth()<<"x"<<results[j].spectrum()<<"  -> "<<z0+line.height()<<"x"<<results[j].height()<<"x"<<results[j].depth()<<"x"<<results[j].spectrum();
                    results[j].resize(z0+line.height(), results[j].height(), results[j].depth(), results[j].spectrum());
                }*/
                if (zs_vals[j]>z0) {
                    for (int x=0; x<line.height(); x++) {
                        if (z0+x<results[j].width()) {
                            for (int y=0; y<line.width(); y++)
                            {
                                results[j](z0+x, y, 0,0)=line(y, x, 0, 0);
                                results[j](z0+x, y, 0,1)=line(y, x, 0, 1);
                                results[j](z0+x, y, 0,2)=line(y, x, 0, 2);
                            }
                        }
                    }
                }
            } else if (pi.mode==Mode::XZ) {
                int z0=zs_vals[j];
                if (pi.angleMode==AngleMode::AngleNone || pi.angle==0) {
                    line=extractXZ_atz(z, frame, pi.location_x);
                    zs_vals[j]++;
                } else if (pi.angleMode==AngleMode::AngleRoll) {
                    line=extractXZ_atz_roll(z, outputFrames, frame, pi.location_x, pi.location_y, pi.angle);
                    zs_vals[j]++;
                } else if (pi.angleMode==AngleMode::AnglePitch) {
                    line=extractXZ_atz_pitch(z, outputFrames, frame, pi.location_y, pi.angle, zs_vals[j]);
                    qDebug()<<"extractXZ_atz_pitch: z="<<z<<", zs_vals[j]="<<zs_vals[j]<<", line.height="<<line.height();
                }
                /*if (z0+line.height()-1>=results[j].height()) {
                    // resize of necessary
                    qDebug()<<"resize "<<results[j].width()<<"x"<<results[j].height()<<"x"<<results[j].depth()<<"x"<<results[j].spectrum()<<"  -> "<<results[j].width()<<"x"<<z0+line.height()<<"x"<<results[j].depth()<<"x"<<results[j].spectrum();
                    results[j].resize(results[j].width(), z0+line.height(), results[j].depth(), results[j].spectrum());

                }*/
                if (zs_vals[j]>z0) {
                    for (int y=0; y<line.height(); y++) {
                        if (z0+y<results[j].height()) {
                            for (int x=0; x<line.width(); x++)
                            {
                                results[j](x, z0+y, 0,0)=line(x, y, 0, 0);
                                results[j](x, z0+y, 0,1)=line(x, y, 0, 1);
                                results[j](x, z0+y, 0,2)=line(x, y, 0, 2);
                            }
                        }
                    }
                }
            }



            // process stills
            if (stills<stillCnt && z%stillDelta==0) {
                auto frame_s=frame;
                const unsigned char color[] = { 255,0,0 };
                if (pi. filteredAngleMode()==AngleMode::AngleNone && pi.mode==Mode::ZY && z<results[j].width()) {
                    if (pi.angleMode==AngleMode::AngleNone || pi.angle==0) {
                        for (int x=pi.location_x-stilllw/2; x<pi.location_x-stilllw/2+stilllw; x++) {
                            frame_s.draw_line(x,0,x,frame.height(), color);
                        }
                    }
                } else if (pi. filteredAngleMode()==AngleMode::AngleNone && pi.mode==Mode::XZ && z<results[j].height()) {
                    if (pi.angleMode==AngleMode::AngleNone || pi.angle==0) {
                        for (int y=pi.location_y-stilllw/2; y<pi.location_y-stilllw/2+stilllw; y++) {
                            frame_s.draw_line(0,y,frame.width(),y, color);
                        }
                    }
                }
                if (stillSeparateFiles) {
                    QFileInfo fi(filename);
                    QString fn=fi.absoluteDir().absoluteFilePath(QString("%1_stack%3_still%2.png").arg(fi.baseName()).arg(z+1, 3, 10,QChar('0')).arg(j+1, 3, 10,QChar('0')));
                    QImage img=CImgToQImage(frame_s);
                    img.save(fn);
                }
                if (stillStrip) {
                    cimg_forXY(frame,x,y) {
                        stillStripImg[j](still_b+x,still_b+y+stills*(still_g+frame.height()),0,0)=frame_s(x,y,0,0);
                        stillStripImg[j](still_b+x,still_b+y+stills*(still_g+frame.height()),0,1)=frame_s(x,y,0,1);
                        stillStripImg[j](still_b+x,still_b+y+stills*(still_g+frame.height()),0,2)=frame_s(x,y,0,2);
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
            if (filterNotch) {
                applyFilterNotch(results[m_savingFrame], fiterNotchWavelength, fiterNotchWidth);
                QImage img=CImgToQImage(results[m_savingFrame]);
                img.save(resultfilt_filenames[m_savingFrame]);
            }
            QSettings set(fnini, QSettings::IniFormat);
            if (pi.mode==Mode::ZY) {
                set.setValue("mode", "ZY");
            }
            if (pi.mode==Mode::XZ) {
                set.setValue("mode", "XZ");
            }
            set.setValue("angle_mode", static_cast<int>(pi.angleMode));
            set.setValue("angle", pi.angle);
            set.setValue("location_x", pi.location_x);
            set.setValue("location_y", pi.location_y);
            set.setValue("stills/count", stillCnt);
            set.setValue("stills/delta", stillDelta);
            set.setValue("stills/strip", stillStrip);
            set.setValue("stills/separate_files", stillSeparateFiles);
            set.setValue("stills/gap", stillGap);
            set.setValue("stills/border", stillBorder);
            set.setValue("stills/line_width", stillLineWidth);
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

void ProcessingTask::applyFilterNotch(cimg_library::CImg<uint8_t> &imgrgb, double center, double delta, bool testoutput)
{

    char fn[1000];
    int nx=ceil(log(imgrgb.width())/log(2));
    int ny=ceil(log(imgrgb.height())/log(2));
    int nnx=pow(2,nx);
    int nny=pow(2,ny);
    cimg_library::CImg<uint8_t> img(nnx, nny,1,1,0);
    int offx=(img.width()-imgrgb.width())/2;
    int offy=(img.height()-imgrgb.height())/2;

    unsigned char one[] = { 1 }, zero[] = { 0 };
    cimg_library::CImg<float> mask(img.width(),img.height(),1,1,1);
    double kmin=1.0/double(center+delta);
    double kmax=1.0/double(center-delta);

    cimg_forXY(mask,x,y) {
        const float kx=double(x-mask.width()/2)/double(mask.width());
        const float ky=double(y-mask.height()/2)/double(mask.height());
        const float kabs2=kx*kx+ky*ky;
        const float kabs=sqrt(kabs2);
        mask(x,y)=1;
        if (kabs2>=kmin*kmin && kabs2<=kmax*kmax) mask(x,y)=0;
        //else if (kabs<kmin) mask(x,y)=exp(-(kabs-kmin)*(kabs-kmin)/(2.0*2.0*2.0));
        //else if (kabs>kmax) mask(x,y)=exp(-(kmax-kabs)*(kmax-kabs)/(2.0*2.0*2.0));

    }
    mask.blur(2,2,2,false);
    if (testoutput) {
        sprintf(fn, "testmask.bmp");
        mask.get_normalize(0,255).save_bmp(fn);
    }

    cimg_forC(imgrgb, c) {
        img.fill(0);
        /*cimg_forXY(imgrgb,x,y) {
            img(offx+x,offy+y)=imgrgb(x,y,0,c);
        }*/
        img=imgrgb.get_channel(c);
        uint8_t cMin,cMax;
        cMin=img.min_max(cMax);
        img.resize(nnx, nny,1,1,0,1,0.5,0.5);//double(offx)/double(nnx),double(offy)/double(nny));
        if (offx>0 || offy>0) {
            // reflective boundary
        }
        if (testoutput) {
            sprintf(fn, "c%d_testinput.bmp", int(c));
            img.save_bmp(fn);
        }

        //get Fourier tranform image
        cimg_library::CImgList<float> F = img.get_FFT();
        cimglist_apply(F,shift)(img.width()/2,img.height()/2,0,0,2);
        //magnitude
        if (testoutput) {
            const cimg_library::CImg<unsigned char> mag = ((F[0].get_pow(2) + F[1].get_pow(2)).sqrt() + 1).log().normalize(0,255);
            sprintf(fn, "c%d_testfftabs.bmp", int(c));
            mag.save_bmp(fn);
            sprintf(fn, "c%d_testfft0.bmp", int(c));
            F[0].save_bmp(fn);
            sprintf(fn, "c%d_testfft1.bmp", int(c));
            F[1].save_bmp(fn);
        }

        cimg_library::CImgList<float> nF(F);
        cimglist_for(F,l) nF[l].mul(mask).shift(-img.width()/2,-img.height()/2,0,0,2);
        cimg_library::CImg<uint8_t> r = nF.FFT(true)[0].normalize(cMin,cMax);
        if (testoutput) {
            sprintf(fn, "c%d_testNF0.bmp", int(c));
            F[0].save_bmp(fn);
            sprintf(fn, "c%d_testNF1.bmp", int(c));
            F[1].save_bmp(fn);
        }
        cimg_forXY(imgrgb,x,y) {
            imgrgb(x,y,0,c)=r(x+offx,y+offy);
        }
    }
    if (testoutput) {
        sprintf(fn, "testoutput.bmp");
        imgrgb.save_bmp(fn);
    }

}
