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
    stillCnt(0),
    stillDelta(60),
    stillStrip(true),
    stillSeparateFiles(false),
    stillGap(5),
    stillBorder(5),
    stillLineWidth(0.1),
    normalize(false),
    normalizeX(-1),
    normalizeY(-1),
    filterNotch(false),
    fiterNotchWavelength(15),
    fiterNotchWidth(0.5),
    modifyWhite(false),
    whitepointR(255),
    whitepointG(255),
    whitepointB(255),
    vid(nullptr),
    z(0),
    m_saving(false),
    m_savingFrame(0),
    stills(0)
{

}

void ProcessingTask::save(const QString &inifilename) const
{
    QSettings setall(inifilename, QSettings::IniFormat);
    saveBase(setall);
    setall.setValue("count", pis.size());

    int j=0;
    for (ProcessingTask::ProcessingItem pi: pis) {
        pi.save(setall, QString("item%1/").arg(j,3,10,QChar('0')));
        j++;
    }

    setall.sync();
}


void ProcessingTask::saveBase(QSettings &ini) const
{
    if (ini.fileName().size()>0) ini.setValue("input_file", QFileInfo(ini.fileName()).absoluteDir().relativeFilePath(filename));
    else ini.setValue("input_file", filename);

    ini.setValue("stills/count", stillCnt);
    ini.setValue("stills/delta", stillDelta);
    ini.setValue("stills/strip", stillStrip);
    ini.setValue("stills/separate_files", stillSeparateFiles);
    ini.setValue("stills/gap", stillGap);
    ini.setValue("stills/border", stillBorder);
    ini.setValue("stills/line_width", stillLineWidth);

    ini.setValue("normalize/enabled", normalize);
    ini.setValue("normalize/x", normalizeX);
    ini.setValue("normalize/y", normalizeY);

    ini.setValue("filter/notch/enabled", filterNotch);
    ini.setValue("filter/notch/wavelength", fiterNotchWavelength);
    ini.setValue("filter/notch/delta", fiterNotchWidth);

    ini.setValue("filter/whitepoint/enabled", modifyWhite);
    ini.setValue("filter/whitepoint/red", whitepointR);
    ini.setValue("filter/whitepoint/green", whitepointG);
    ini.setValue("filter/whitepoint/blue", whitepointB);
}


ProcessingTask::ProcessingItem::ProcessingItem():
    mode(ProcessingTask::Mode::XZ), location_x(-1), location_y(-1), angle(0), angleMode(AngleMode::AngleNone)
{

}

int ProcessingTask::ProcessingItem::angleModeForCombo() const {
    if (filteredAngleMode()==AngleMode::AnglePitch) return 1;
    return 0;
}

ProcessingTask::AngleMode ProcessingTask::ProcessingItem::filteredAngleMode() const {
    if (angle==0) return AngleMode::AngleNone;
    else return angleMode;
}

void ProcessingTask::ProcessingItem::save(QSettings &ini, const QString &basename) const
{
    if (mode==Mode::ZY) {
        ini.setValue(basename+"mode", "ZY");
    }
    if (mode==Mode::XZ) {
        ini.setValue(basename+"mode", "XZ");
    }
    ini.setValue(basename+"location_x", location_x);
    ini.setValue(basename+"location_y", location_y);
    ini.setValue(basename+"angle_mode", static_cast<int>(angleMode));
    ini.setValue(basename+"angle", angle);
}


void ProcessingTask::ProcessingItem::load(QSettings &ini, const QString &basename)
{
    if (ini.value(basename+"mode", "ZY").toString().toUpper().trimmed().left(2)=="ZY") mode=Mode::ZY;
    else mode=Mode::XZ;
    location_x=ini.value(basename+"location_x", location_x).toInt();
    location_y=ini.value(basename+"location_y", location_y).toInt();
    angleMode=static_cast<AngleMode>(ini.value(basename+"angle_mode", static_cast<int>(angleMode)).toInt());
    angle=ini.value(basename+"angle", angle).toDouble();
}

void ProcessingTask::load(const QString &inifilename)
{
    QSettings setall(inifilename, QSettings::IniFormat);
    int count=setall.value("count", 0).toInt();

    filename=QFileInfo(inifilename).absoluteDir().absoluteFilePath(setall.value("input_file", filename).toString());

    stillCnt=setall.value("stills/count", stillCnt).toInt();
    stillDelta=setall.value("stills/delta", stillDelta).toInt();
    stillStrip=setall.value("stills/strip", stillStrip).toBool();
    stillSeparateFiles=setall.value("stills/separate_files", stillSeparateFiles).toBool();
    stillGap=setall.value("stills/gap", stillGap).toDouble();
    stillBorder=setall.value("stills/border", stillBorder).toDouble();
    stillLineWidth=setall.value("stills/line_width", stillLineWidth).toDouble();

    normalize=setall.value("normalize/enabled", normalize).toBool();
    normalizeX=setall.value("normalize/x", normalizeX).toInt();
    normalizeY=setall.value("normalize/y", normalizeY).toInt();

    filterNotch=setall.value("filter/notch/enabled", filterNotch).toBool();
    fiterNotchWavelength=setall.value("filter/notch/wavelength", fiterNotchWavelength).toDouble();
    fiterNotchWidth=setall.value("filter/notch/delta", fiterNotchWidth).toDouble();

    modifyWhite=setall.value("filter/whitepoint/enabled", modifyWhite).toBool();
    whitepointR=setall.value("filter/whitepoint/red", whitepointR).toUInt();
    whitepointG=setall.value("filter/whitepoint/green", whitepointG).toUInt();
    whitepointB=setall.value("filter/whitepoint/blue", whitepointB).toUInt();

    pis.clear();
    for (int j=0; j<count; j++) {
        ProcessingTask::ProcessingItem pi;
        pi.load(setall, QString("item%1/").arg(j,3,10,QChar('0')));
        pis.append(pi);
    }
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
    save(allini);

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
                //qDebug()<<"output size: "<<results[j].width()<<"x"<<results[j].height()<<"x"<<results[j].depth()<<"x"<<results[j].spectrum();
                zs_vals.push_back(0);
            } else if (pi.mode==Mode::XZ) {
                if (pi.angleMode==AngleMode::AngleNone || pi.angle==0) {
                    line=extractXZ_atz(0, frame, pi.location_y);
                } else if (pi.angleMode==AngleMode::AngleRoll) {
                    line=extractXZ_atz_roll(0, outputFrames, frame, pi.location_x, pi.location_y, pi.angle);
                } else if (pi.angleMode==AngleMode::AnglePitch) {
                    line=extractXZ_atz_pitch(0, outputFrames, frame, pi.location_y, pi.angle, zout, &len);
                }
                results.push_back(cimg_library::CImg<uint8_t>());
                results[j].assign(line.width(), len, 1, 3);
                //qDebug()<<"output size: "<<results[j].width()<<"x"<<results[j].height()<<"x"<<results[j].depth()<<"x"<<results[j].spectrum();
                zs_vals.push_back(0);
            }
            QFileInfo fi(filename);
            QString fn=fi.absoluteDir().absoluteFilePath(QString("%1_stack%2.png").arg(fi.baseName()).arg(j+1, 3, 10,QChar('0')));
            QString fnfilt=fi.absoluteDir().absoluteFilePath(QString("%1_stack%2_filtered.png").arg(fi.baseName()).arg(j+1, 3, 10,QChar('0')));
            QString fnini=fi.absoluteDir().absoluteFilePath(QString("%1_stack%2.ini").arg(fi.baseName()).arg(j+1, 3, 10,QChar('0')));
            result_filenames.push_back(fn);
            resultfilt_filenames.push_back(fnfilt);
            result_inifilenames.push_back(fnini);
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
                    //qDebug()<<"extractZY_atz_pitch: z="<<z<<", zs_vals[j]="<<zs_vals[j]<<", line.height="<<line.height();
                }
                /*if (z0+line.height()-1>=results[j].width()) {
                    // resize of necessary
                    //qDebug()<<"resize "<<results[j].width()<<"x"<<results[j].height()<<"x"<<results[j].depth()<<"x"<<results[j].spectrum()<<"  -> "<<z0+line.height()<<"x"<<results[j].height()<<"x"<<results[j].depth()<<"x"<<results[j].spectrum();
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
                    line=extractXZ_atz(z, frame, pi.location_y);
                    zs_vals[j]++;
                } else if (pi.angleMode==AngleMode::AngleRoll) {
                    line=extractXZ_atz_roll(z, outputFrames, frame, pi.location_x, pi.location_y, pi.angle);
                    zs_vals[j]++;
                } else if (pi.angleMode==AngleMode::AnglePitch) {
                    line=extractXZ_atz_pitch(z, outputFrames, frame, pi.location_y, pi.angle, zs_vals[j]);
                    //qDebug()<<"extractXZ_atz_pitch: z="<<z<<", zs_vals[j]="<<zs_vals[j]<<", line.height="<<line.height();
                }
                /*if (z0+line.height()-1>=results[j].height()) {
                    // resize of necessary
                    //qDebug()<<"resize "<<results[j].width()<<"x"<<results[j].height()<<"x"<<results[j].depth()<<"x"<<results[j].spectrum()<<"  -> "<<results[j].width()<<"x"<<z0+line.height()<<"x"<<results[j].depth()<<"x"<<results[j].spectrum();
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
                    if (QFile::exists(fn)) QFile::remove(fn);
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
            if (QFile::exists(fn)) QFile::remove(fn);
            img.save(fn);
            bool hasMod=false;
            if (filterNotch) {
                applyFilterNotch(results[m_savingFrame], fiterNotchWavelength, fiterNotchWidth);
                hasMod=true;
            }
            if (modifyWhite) {
                applyWhitepointCorrection(results[m_savingFrame], whitepointR, whitepointG, whitepointB);
                hasMod=true;
            }
            if (hasMod) {
                QImage img=CImgToQImage(results[m_savingFrame]);
                if (QFile::exists(resultfilt_filenames[m_savingFrame])) QFile::remove(resultfilt_filenames[m_savingFrame]);
                img.save(resultfilt_filenames[m_savingFrame]);

            }
            QSettings set(fnini, QSettings::IniFormat);
            saveBase(set);
            pi.save(set, "");

            QImage imgs=CImgToQImage(stillStripImg[m_savingFrame]);
            QFileInfo fi(filename);
            QString fns=fi.absoluteDir().absoluteFilePath(QString("%1_stack%2_stillstrip.png").arg(fi.baseName()).arg(m_savingFrame+1, 3, 10,QChar('0')));
            if (QFile::exists(fns)) QFile::remove(fns);
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

    //unsigned char one[] = { 1 }, zero[] = { 0 };
    cimg_library::CImg<float> mask(img.width(),img.height(),1,1,1);
    double kmin=1.0/double(center+delta);
    double kmax=1.0/double(center-delta);

    cimg_forXY(mask,x,y) {
        const float kx=double(x-mask.width()/2)/double(mask.width());
        const float ky=double(y-mask.height()/2)/double(mask.height());
        const float kabs2=kx*kx+ky*ky;
        //const float kabs=sqrt(kabs2);
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
        sprintf(fn, "testoutput_notchfilter.bmp");
        imgrgb.save_bmp(fn);
    }

}

void ProcessingTask::applyWhitepointCorrection(cimg_library::CImg<uint8_t> &img, uint8_t red, uint8_t green, uint8_t blue, bool testoutput)
{
    const double r=red;
    const double g=green;
    const double b=blue;
    const double avgColor=(r+g+b)/3.0;
    const double rFactor=avgColor/r;
    const double gFactor=avgColor/g;
    const double bFactor=avgColor/b;

    cimg_forXYZ(img,x,y,z) {
        img(x,y,z,0)=static_cast<uint8_t>(qBound<double>(0.0,static_cast<double>(img(x,y,z,0))*rFactor,255.0));
        img(x,y,z,1)=static_cast<uint8_t>(qBound<double>(0.0,static_cast<double>(img(x,y,z,1))*gFactor,255.0));
        img(x,y,z,2)=static_cast<uint8_t>(qBound<double>(0.0,static_cast<double>(img(x,y,z,2))*bFactor,255.0));
    }

    if (testoutput) {
        char fn[1000];
        sprintf(fn, "testoutput_whitepoint.bmp");
        img.save_bmp(fn);
    }
}

