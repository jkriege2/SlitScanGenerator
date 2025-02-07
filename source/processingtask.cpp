#include "processingtask.h"
#include "cimg_tools.h"
#include "cpp_tools.h"
#include "imagewriterfactory.h"
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDebug>

QString ProcessingTask::InterpolationMethod2String(InterpolationMethod m)
{
    switch(m) {
    case ProcessingTask::InterpolationMethod::NearestNeighbor: return "nearest_neighbor";
    case ProcessingTask::InterpolationMethod::Linear: return "linear";
    case ProcessingTask::InterpolationMethod::Cubic: return "cubic";
    }
    return "nearest_neighbor";
}

ProcessingTask::InterpolationMethod ProcessingTask::String2InterpolationMethod(const QString &m)
{
    const QString mm=m.trimmed().toLower().simplified();
    if (mm=="lin" || mm=="linear") return ProcessingTask::InterpolationMethod::Linear;
    if (mm=="cubic" || mm=="cubig") return ProcessingTask::InterpolationMethod::Cubic;
    return ProcessingTask::InterpolationMethod::NearestNeighbor;
}

QString ProcessingTask::FileFormat2String(FileFormat m)
{
    switch(m) {
    case ProcessingTask::FileFormat::PNG: return "PNG";
    case ProcessingTask::FileFormat::JPEG: return "JPEG";

    case ProcessingTask::FileFormat::__COUNT:
        break;
    }
    return "PNG";
}

QString ProcessingTask::FileFormat2Extension(FileFormat m)
{
    switch(m) {
    case ProcessingTask::FileFormat::PNG: return "png";
    case ProcessingTask::FileFormat::JPEG: return "jpg";
    case ProcessingTask::FileFormat::__COUNT:
        break;
    }
    return "png";
}

ProcessingTask::FileFormat ProcessingTask::String2FileFormat(const QString &m)
{
    const QString mm=m.trimmed().toLower().simplified();
    if (mm=="png") return ProcessingTask::FileFormat::PNG;
    if (mm=="jpeg" || mm=="jpg") return ProcessingTask::FileFormat::JPEG;
    return ProcessingTask::FileFormat::PNG;
}

QString ProcessingTask::OutputTargetOptions2String(OutputTargetOptions m)
{
    switch(m) {
    case ProcessingTask::OutputTargetOptions::SameDirectoryAsInput: return "same_directory_as_input";
    case ProcessingTask::OutputTargetOptions::SubDirectoryPerInput: return "subdir_per_input";
    }
    return "same_directory_as_input";
}


ProcessingTask::OutputTargetOptions ProcessingTask::String2OutputTargetOptions(const QString &m)
{
    const QString mm=m.trimmed().toLower().simplified();
    if (mm=="same_directory_as_input" || mm=="same_directory_as_input" || mm=="same_directory" || mm=="input_directory" || mm=="input" || mm=="same") return ProcessingTask::OutputTargetOptions::SameDirectoryAsInput;
    if (mm=="subdir_per_input" || mm=="subdir" || mm=="subdirectory_per_input" || mm=="subdirectory" || mm=="sub") return ProcessingTask::OutputTargetOptions::SubDirectoryPerInput;
    return ProcessingTask::OutputTargetOptions::SameDirectoryAsInput;
}

interpolatingAtXYFunctor ProcessingTask::InterpolationMethod2XYFunctor(InterpolationMethod m)
{
    switch(m) {
    case ProcessingTask::InterpolationMethod::Linear:
        return [](const cimg_library::CImg<uint8_t>& img, const float fx, const float fy, const int z, const int c)->float {
                    return img.linear_atXY(fx,fy,z,c);
                };

    case ProcessingTask::InterpolationMethod::Cubic:
        return [](const cimg_library::CImg<uint8_t>& img, const float fx, const float fy, const int z, const int c)->float {
                    return img.cubic_atXY(fx,fy,z,c);
                };
    case ProcessingTask::InterpolationMethod::NearestNeighbor: break;

    }

    return [](const cimg_library::CImg<uint8_t>& img, const float fx, const float fy, const int z, const int c)->float {
                return img.atXY(static_cast<int>(fx),static_cast<int>(fy),z,c);
            };
}

ProcessingTask::ProcessingTask(std::shared_ptr<VideoReader> reader, std::shared_ptr<ImageWriter> writer, std::shared_ptr<ConfigIO> configio):
    do_not_save_anyting(false),
    filename(),
    outputBasename(),
    outputTarget(OutputTargetOptions::SubDirectoryPerInput),
    outputFileFormat(FileFormat::PNG),
    outputFileQuality(-1),
    outputFrames(0),
    interpolationMethod(InterpolationMethod::Cubic),
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
    firstFrame(-1),
    lastFrame(-1),
    z(0),
    z_all(0),
    m_saving(false),
    m_savingFrame(0),
    stills(0),
    m_reader(reader),
    m_writer(writer),
    m_configio(configio),
    m_reporter(nullptr),
    m_prog(0)
{

}

void ProcessingTask::setReporter(ProcessingTaskReporter *reporter)
{
    m_reporter=reporter;
}

void ProcessingTask::process()
{
    if (processInit()) {
        while (processStep()) {
        }
    }
    processFinalize();
}

void ProcessingTask::save(const QString &inifilename) const
{
    if (m_configio) {
        m_configio->open(inifilename.toStdString());
        saveBase(m_configio);
        m_configio->setValue("count", static_cast<size_t>(pis.size()));
        int j=0;
        for (ProcessingTask::ProcessingItem pi: pis) {
            pi.save(m_configio, QString("item%1/").arg(j,3,10,QChar('0')).toStdString());
            j++;
        }
        m_configio->close();
    }
}

QDir ProcessingTask::getOutputDir() const {
    const QFileInfo fi(filename);
    auto dir=fi.absoluteDir();
    auto basename=fi.baseName();
    if (!outputBasename.isEmpty() && basename.toLower()!=outputBasename.toLower()) basename+=outputBasename;
    if (outputTarget == OutputTargetOptions::SubDirectoryPerInput) {
        qDebug()<<"creating subdir "<<basename<<" in "<<dir.absolutePath();
        if (m_writer && m_writer->writesToDisk()){
            bool ok1=dir.mkdir(basename);
            qDebug()<<"  --> ok1="<<ok1<<"    ("<<dir.absolutePath()<<")";
        }
        bool ok2=dir.cd(basename);
        qDebug()<<"  --> ok2="<<ok2<<"    ("<<dir.absolutePath()<<")";
    }
    return dir;
}


void ProcessingTask::saveBase(std::shared_ptr<ConfigIO> ini) const
{
    if (!ini) return;
    if (ini->filename().size()>0) ini->setValue("input_file", QFileInfo(QString::fromStdString(ini->filename())).absoluteDir().relativeFilePath(filename).toStdString());
    else ini->setValue("input_file", filename.toStdString());
    ini->setValue("video_options/first_frame",firstFrame);
    ini->setValue("video_options/last_frame",lastFrame);

    ini->setValue("processing_options/output_basename",outputBasename.toStdString());
    ini->setValue("processing_options/output_target",OutputTargetOptions2String(outputTarget).toStdString());
    ini->setValue("processing_options/output_file_format",FileFormat2String(outputFileFormat).toStdString());
    ini->setValue("processing_options/output_file_quality",outputFileQuality);
    ini->setValue("processing_options/interpolation_method", InterpolationMethod2String(interpolationMethod).toStdString());

    ini->setValue("stills/count", stillCnt);
    ini->setValue("stills/delta", stillDelta);
    ini->setValue("stills/strip", stillStrip);
    ini->setValue("stills/separate_files", stillSeparateFiles);
    ini->setValue("stills/gap", stillGap);
    ini->setValue("stills/border", stillBorder);
    ini->setValue("stills/line_width", stillLineWidth);

    ini->setValue("normalize/enabled", normalize);
    ini->setValue("normalize/x", normalizeX);
    ini->setValue("normalize/y", normalizeY);

    ini->setValue("filter/notch/enabled", filterNotch);
    ini->setValue("filter/notch/wavelength", fiterNotchWavelength);
    ini->setValue("filter/notch/delta", fiterNotchWidth);

    ini->setValue("filter/whitepoint/enabled", modifyWhite);
    ini->setValue("filter/whitepoint/red", whitepointR);
    ini->setValue("filter/whitepoint/green", whitepointG);
    ini->setValue("filter/whitepoint/blue", whitepointB);
}


ProcessingTask::ProcessingItem::ProcessingItem(int x, int y):
    mode(ProcessingTask::Mode::XZ),
    location_x(x),
    location_y(y),
    angle(0),
    angleMode(AngleMode::AngleNone),
    slit_width(1),
    addBefore(AddBeforeAfterMode::None),
    addAfter(AddBeforeAfterMode::None),
    z_step(0)
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

void ProcessingTask::ProcessingItem::save(std::shared_ptr<ConfigIO> ini, const std::string &basename) const
{
    if (mode==Mode::ZY) {
        ini->setValue(basename+"mode", "ZY");
    }
    if (mode==Mode::XZ) {
        ini->setValue(basename+"mode", "XZ");
    }
    ini->setValue(basename+"location_x", location_x);
    ini->setValue(basename+"location_y", location_y);
    ini->setValue(basename+"angle_mode", static_cast<int>(angleMode));
    ini->setValue(basename+"angle", angle);
    ini->setValue(basename+"slit_width", slit_width);
    ini->setValue(basename+"z_step", z_step);
    ini->setValue(basename+"add_before", static_cast<int>(addBefore));
    ini->setValue(basename+"add_after", static_cast<int>(addAfter));
}


void ProcessingTask::ProcessingItem::load(std::shared_ptr<ConfigIO> ini, const std::string &basename)
{
    if (QString::fromStdString(ini->value(basename+"mode", std::string("ZY"))).toUpper().trimmed().left(2)=="ZY") mode=Mode::ZY;
    else mode=Mode::XZ;
    location_x=ini->value(basename+"location_x", location_x);
    location_y=ini->value(basename+"location_y", location_y);
    angleMode=static_cast<AngleMode>(ini->value(basename+"angle_mode", static_cast<int>(angleMode)));
    angle=ini->value(basename+"angle", angle);
    slit_width=ini->value(basename+"slit_width", slit_width);
    z_step=ini->value(basename+"z_step", z_step);
    addBefore=static_cast<AddBeforeAfterMode>(ini->value(basename+"add_before", static_cast<int>(addBefore)));
    addAfter=static_cast<AddBeforeAfterMode>(ini->value(basename+"add_after", static_cast<int>(addAfter)));
}

void ProcessingTask::load(const QString &inifilename)
{
    if (m_configio) {
        m_configio->open(inifilename.toStdString());

        int count=m_configio->value("count", 0);

        filename=QFileInfo(inifilename).absoluteDir().absoluteFilePath(QString::fromStdString(m_configio->value("input_file", filename.toStdString())));

        firstFrame=m_configio->value("video_options/first_frame",firstFrame);
        lastFrame=m_configio->value("video_options/last_frame",lastFrame);

        outputBasename=QString::fromStdString(m_configio->value("processing_options/output_basename",outputBasename.toStdString()));
        outputFileQuality=m_configio->value("processing_options/output_file_quality",outputFileQuality);
        outputTarget=String2OutputTargetOptions(QString::fromStdString(m_configio->value("processing_options/output_target", OutputTargetOptions2String(outputTarget).toStdString())));
        outputFileFormat=String2FileFormat(QString::fromStdString(m_configio->value("processing_options/output_file_format", FileFormat2String(outputFileFormat).toStdString())));

        interpolationMethod=String2InterpolationMethod(QString::fromStdString(m_configio->value("processing_options/interpolation_method", InterpolationMethod2String(InterpolationMethod::NearestNeighbor).toStdString())));

        stillCnt=m_configio->value("stills/count", stillCnt);
        stillDelta=m_configio->value("stills/delta", stillDelta);
        stillStrip=m_configio->value("stills/strip", stillStrip);
        stillSeparateFiles=m_configio->value("stills/separate_files", stillSeparateFiles);
        stillGap=m_configio->value("stills/gap", stillGap);
        stillBorder=m_configio->value("stills/border", stillBorder);
        stillLineWidth=m_configio->value("stills/line_width", stillLineWidth);

        normalize=m_configio->value("normalize/enabled", normalize);
        normalizeX=m_configio->value("normalize/x", normalizeX);
        normalizeY=m_configio->value("normalize/y", normalizeY);

        filterNotch=m_configio->value("filter/notch/enabled", filterNotch);
        fiterNotchWavelength=m_configio->value("filter/notch/wavelength", fiterNotchWavelength);
        fiterNotchWidth=m_configio->value("filter/notch/delta", fiterNotchWidth);

        modifyWhite=m_configio->value("filter/whitepoint/enabled", modifyWhite);
        whitepointR=m_configio->value("filter/whitepoint/red", whitepointR);
        whitepointG=m_configio->value("filter/whitepoint/green", whitepointG);
        whitepointB=m_configio->value("filter/whitepoint/blue", whitepointB);

        pis.clear();
        for (int j=0; j<count; j++) {
            ProcessingTask::ProcessingItem pi;
            pi.load(m_configio, QString("item%1/").arg(j,3,10,QChar('0')).toStdString());
            pis.append(pi);
        }

        m_configio->close();
    }
}





bool ProcessingTask::processInit()
{
    TIME_BLOCK_SW(timer, "processInit()");
    if (!m_writer) m_writer=makeSharedImageWriter(outputFileFormat);
    if (!m_reader) {
        if (m_reporter) m_reporter->reportErrorMessage("no reader provided!");
        return false;
    }
    if (m_reporter) m_reporter->reportMessageOpeningVideo();
    z=0;
    z_all=0;
    m_saving=false;
    m_savingFrame=0;
    m_prog=1;
    m_outputDir=getOutputDir();

    const QFileInfo fi(filename);

    if (outputBasename.isEmpty()) outputBasename=fi.baseName();
    const QString allini=fi.absoluteDir().absoluteFilePath(QString("%1.ini").arg(outputBasename));
    if (!do_not_save_anyting) save(allini);

    if (m_reader->open(filename.toStdString()) && m_reader->readNext(frame)) {
        if (firstFrame>=1) {
            // skip first firstFrame frames
            for (int i=0; i<firstFrame-1; i++) {
                if (!m_reader->readNext(frame)) break;
                z_all++;
            }
        }
        if (outputFrames<=0) {
            if (firstFrame>=1 && lastFrame>=2) {
                outputFrames=lastFrame-firstFrame;
            } else {
                outputFrames=m_reader->getFrameCount();
            }
        }
        int still_b=stillBorder/100.0*frame.width();
        int still_g=stillGap/100.0*frame.height();
        if (m_reporter) m_reporter->reportFrameProgress(1, pis.size()+2+m_reader->getFrameCount());
        int j=0;
        for (ProcessingTask::ProcessingItem pi: pis) {
            cimg_library::CImg<uint8_t> line;
            int zout=0;
            int len=outputFrames/pi.get_z_step()+1;
            int addedOutputImageLengthBefore=0;
            int addedOutputImageLengthAfter=0;
            if (pi.mode==Mode::ZY) {
                if (pi.angleMode==AngleMode::AngleNone || fabs(pi.angle)<0.0001) {
                    line=extractZY_atz(0, frame, pi.location_x, pi.get_slit_offset(), pi.get_slit_width());
                    if (pi.addBefore==AddBeforeAfterMode::ToLower) addedOutputImageLengthBefore+=pi.location_x;
                    else if (pi.addBefore==AddBeforeAfterMode::ToHigher) addedOutputImageLengthBefore+=frame.width()-pi.location_x-1;
                    if (pi.addAfter==AddBeforeAfterMode::ToLower) addedOutputImageLengthAfter+=pi.location_x-1;
                    else if (pi.addAfter==AddBeforeAfterMode::ToHigher) addedOutputImageLengthAfter+=frame.width()-pi.location_x-1;
                } else if (pi.angleMode==AngleMode::AngleRoll) {
                    line=extractZY_atz_roll(0, outputFrames, frame, pi.location_x, pi.location_y, pi.angle,InterpolationMethod2XYFunctor(interpolationMethod), pi.get_slit_offset(), pi.get_slit_width());
                } else if (pi.angleMode==AngleMode::AnglePitch) {
                    line=extractZY_atz_pitch(0, outputFrames, frame, pi.location_x, pi.angle,InterpolationMethod2XYFunctor(interpolationMethod), pi.get_slit_offset(), pi.get_slit_width(), zout, &len);
                }
                results.push_back(ResultData());
                const int outputpixels=len*line.height();
                qDebug()<<"outputpixels="<<outputpixels<<", addedOutputImageLengthBefore="<<addedOutputImageLengthAfter<<", addedOutputImageLengthAfter="<<addedOutputImageLengthAfter<<", pi.addBefore="<<static_cast<int>(pi.addBefore)<<", pi.addAfter="<<static_cast<int>(pi.addAfter);
                results[j].img.assign(outputpixels+addedOutputImageLengthBefore+addedOutputImageLengthAfter+100, line.width(), 1, 3);
                results[j].maxX=1;
                results[j].maxY=line.width();
                results[j].zs_val=0;
                results[j].z=0;
                results[j].output_zs=addedOutputImageLengthBefore;
                results[j].addAfterImageSTackPixels=addedOutputImageLengthAfter;
                if (pi.addBefore==AddBeforeAfterMode::ToLower) {
                    for (int c=0; c<3; c++) {
                        for (int y=0; y<qMin(frame.height(),results[j].img.height()); y++) {
                            for(int x=0; x<addedOutputImageLengthBefore; x++) {
                                if (x<results[j].img.width() && x<frame.width()) {
                                    results[j].img(x,y,0,c)=frame(x,y,0,c);
                                }
                            }
                        }
                    }
                }
                else if (pi.addBefore==AddBeforeAfterMode::ToHigher) {
                    for (int c=0; c<3; c++) {
                        for (int y=0; y<qMin(frame.height(),results[j].img.height()); y++) {
                            for(int x=0; x<addedOutputImageLengthBefore; x++) {
                                const int xf=pi.location_x+1+x;
                                const int xr=addedOutputImageLengthBefore-1-x;
                                if (xf>=0 && xf<frame.width() && xr>=0 && xr<results[j].img.width()) results[j].img(xr,y,0,c)=frame(xf,y,0,c);
                            }
                        }
                    }
                }

                //qDebug()<<"output size: "<<cimgsize2string(results[j].img);
            } else if (pi.mode==Mode::XZ) {
                if (pi.angleMode==AngleMode::AngleNone || fabs(pi.angle)<0.0001) {
                    line=extractXZ_atz(0, frame, pi.location_y, pi.get_slit_offset(), pi.get_slit_width());
                    if (pi.addBefore==AddBeforeAfterMode::ToLower) addedOutputImageLengthBefore+=pi.location_y;
                    else if (pi.addBefore==AddBeforeAfterMode::ToHigher) addedOutputImageLengthBefore+=frame.height()-pi.location_y-1;
                    if (pi.addAfter==AddBeforeAfterMode::ToLower) addedOutputImageLengthAfter+=pi.location_y;
                    else if (pi.addAfter==AddBeforeAfterMode::ToHigher) addedOutputImageLengthAfter+=frame.height()-pi.location_y-1;
                } else if (pi.angleMode==AngleMode::AngleRoll) {
                    line=extractXZ_atz_roll(0, outputFrames, frame, pi.location_x, pi.location_y, pi.angle,InterpolationMethod2XYFunctor(interpolationMethod), pi.get_slit_offset(), pi.get_slit_width());
                } else if (pi.angleMode==AngleMode::AnglePitch) {
                    line=extractXZ_atz_pitch(0, outputFrames, frame, pi.location_y, pi.angle,InterpolationMethod2XYFunctor(interpolationMethod), pi.get_slit_offset(), pi.get_slit_width(), zout, &len);
                }
                results.push_back(ResultData());
                results[j].img.assign(line.width(), len*line.height()+addedOutputImageLengthBefore+addedOutputImageLengthAfter, 1, 3);
                results[j].maxX=line.width();
                results[j].maxY=1;
                results[j].zs_val=0;
                results[j].z=0;
                results[j].output_zs=addedOutputImageLengthBefore;
                results[j].addAfterImageSTackPixels=addedOutputImageLengthAfter;
                if (pi.addBefore==AddBeforeAfterMode::ToLower) {
                    for (int c=0; c<3; c++) {
                        for (int y=0; y<addedOutputImageLengthBefore; y++) {
                            if (y<results[j].img.height() && y<frame.height()) {
                                for(int x=0; x<qMin(frame.width(), results[j].img.width()); x++) {
                                    results[j].img(x,y,0,c)=frame(x,y,0,c);
                                }
                            }
                        }
                    }
                }
                else if (pi.addBefore==AddBeforeAfterMode::ToHigher) {
                    for (int c=0; c<3; c++) {
                        for (int y=0; y<addedOutputImageLengthBefore; y++) {
                            for(int x=0; x<qMin(frame.width(), results[j].img.width()); x++) {
                                const int yf=pi.location_y+1+y;
                                const int yr=addedOutputImageLengthBefore-1-y;
                                if (yf>=0 && yf<frame.height() && yr>=0 && yr<results[j].img.height()) results[j].img(x,yr,0,c)=frame(x,yf,0,c);
                            }
                        }
                    }
                }
                //qDebug()<<"output size: "<<cimgsize2string(results[j].img);
            }
            const QString fn=m_outputDir.absoluteFilePath(QString("%1_stack%2.%3").arg(outputBasename).arg(j+1, 3, 10,QChar('0')).arg(FileFormat2Extension(outputFileFormat)));
            const QString fnfilt=m_outputDir.absoluteFilePath(QString("%1_stack%2_filtered.%3").arg(outputBasename).arg(j+1, 3, 10,QChar('0')).arg(FileFormat2Extension(outputFileFormat)));
            const QString fnini=m_outputDir.absoluteFilePath(QString("%1_stack%2.ini").arg(outputBasename).arg(j+1, 3, 10,QChar('0')));
            results[j].filename=fn;
            results[j].filt_filename=fnfilt;
            results[j].inifilename=fnini;
            if (stillStrip && stillCnt>0) {
                stillStripImg.push_back(cimg_library::CImg<uint8_t>());
                stillStripImg[j].resize(frame.width()+2*still_b, still_b+(frame.height()+still_g)*stillCnt-still_g+still_b, 1, 3);
            }
            j++;
        }

        return true;
    } else {
        if (m_reader->hadError()) {
            if (m_reporter) m_reporter->reportErrorMessage(m_reader->getLastError());
        } else {
            if (m_reporter) m_reporter->reportErrorMessage("unknown opening reading video '"+filename.toStdString()+"'!");
        }
        return false;
    }
}

bool ProcessingTask::processStep()
{
    TIME_BLOCK_SW(timer, "processStep()");
    if (!m_reader) {
        if (m_reporter) m_reporter->reportErrorMessage("no reader provided!");
        return false;
    }

    m_prog++;
    if (!m_saving) {
        // process old frame
        if (m_reporter) m_reporter->reportFrameProgress(z+1, outputFrames);
        int still_b=stillBorder/100.0*frame.width();
        int still_g=stillGap/100.0*frame.height();
        int stilllw=std::max<int>(1,stillLineWidth/100.0*frame.width());

        for (int j=0; j<results.size(); j++) {
            const ProcessingTask::ProcessingItem& pi=pis[j];
            ProcessingTask::ResultData& res=results[j];

            /*if (pi.get_z_step()>1) {
                qDebug()<<"j="<<j<<": z="<<z<<", res.zs_val="<<res.zs_val<<", res.z="<<res.z<<", res.output_zs="<<res.output_zs<<", pi.get_z_step()="<<pi.get_z_step();
            }*/

            if (pi.get_z_step()==1 || ((z%pi.get_z_step())==0)) {
                cimg_library::CImg<uint8_t> line;
                if (pi.mode==Mode::ZY) {
                    const int z0=res.zs_val;
                    if (pi.angleMode==AngleMode::AngleNone || pi.angle==0) {
                        TIME_BLOCK_SW(timer, "extractZY_atz()");
                        line=extractZY_atz(z, frame, pi.location_x, pi.get_slit_offset(), pi.get_slit_width());
                        res.zs_val++;
                    } else if (pi.angleMode==AngleMode::AngleRoll) {
                        TIME_BLOCK_SW(timer, "extractZY_atz_roll()");
                        line=extractZY_atz_roll(z, outputFrames, frame, pi.location_x, pi.location_y, pi.angle,InterpolationMethod2XYFunctor(interpolationMethod), pi.get_slit_offset(), pi.get_slit_width());
                        res.zs_val++;
                    } else if (pi.angleMode==AngleMode::AnglePitch) {
                        TIME_BLOCK_SW(timer, "extractZY_atz_pitch()");
                        line=extractZY_atz_pitch(z, outputFrames, frame, pi.location_x, pi.angle,InterpolationMethod2XYFunctor(interpolationMethod), pi.get_slit_offset(), pi.get_slit_width(), res.zs_val);
                        //qDebug()<<"extractZY_atz_pitch: z="<<z<<", res.zs_val="<<res.zs_val<<", line.height="<<line.height();
                    }
                    /*if (z0+line.height()-1>=res.width()) {
                        // resize of necessary
                        //qDebug()<<"resize "<<cimgsize2string(res)<<"  -> "<<z0+line.height()<<"x"<<res.height()<<"x"<<res.depth()<<"x"<<res.spectrum();
                        res.resize(z0+line.height(), res.height(), res.depth(), res.spectrum());
                    }*/
                    if (res.zs_val>z0) {
                        TIME_BLOCK_SW(timer, "StoreLine")
                        for (int c=0; c<3; c++) {
                            for (int x=0; x<line.height(); x++) {
                                if (res.output_zs<res.img.width()) {
                                    res.maxX=qMax(res.output_zs+x, res.maxX);
                                    for (int y=0; y<line.width(); y++)
                                    {
                                        res.img(res.output_zs+x, y, 0,c)=line(y, x, 0, c);
                                    }
                                }
                            }
                        }
                        res.output_zs+=line.height();
                    }
                } else if (pi.mode==Mode::XZ) {
                    const int z0=res.zs_val;
                    if (pi.angleMode==AngleMode::AngleNone || pi.angle==0) {
                        TIME_BLOCK_SW(timer, "extractXZ_atz()");
                        line=extractXZ_atz(z, frame, pi.location_y, pi.get_slit_offset(), pi.get_slit_width());
                        res.zs_val++;
                    } else if (pi.angleMode==AngleMode::AngleRoll) {
                        TIME_BLOCK_SW(timer, "extractXZ_atz_roll()");
                        line=extractXZ_atz_roll(z, outputFrames, frame, pi.location_x, pi.location_y, pi.angle,InterpolationMethod2XYFunctor(interpolationMethod), pi.get_slit_offset(), pi.get_slit_width());
                        res.zs_val++;
                    } else if (pi.angleMode==AngleMode::AnglePitch) {
                        TIME_BLOCK_SW(timer, "extractXZ_atz_pitch()");
                        line=extractXZ_atz_pitch(z, outputFrames, frame, pi.location_y, pi.angle,InterpolationMethod2XYFunctor(interpolationMethod), pi.get_slit_offset(), pi.get_slit_width(), res.zs_val);
                        //qDebug()<<"extractXZ_atz_pitch: z="<<z<<", res.zs_val="<<res.zs_val<<", line.height="<<line.height();
                    }
                    /*if (z0+line.height()-1>=res.height()) {
                        // resize of necessary
                        //qDebug()<<"resize "<<cimgsize2string(res)<<"  -> "<<res.width()<<"x"<<z0+line.height()<<"x"<<res.depth()<<"x"<<res.spectrum();
                        res.resize(res.width(), z0+line.height(), res.depth(), res.spectrum());

                    }*/
                    if (res.zs_val>z0) {
                        TIME_BLOCK_SW(timer, "StoreLine()");
                        for (int c=0; c<3; c++) {
                            for (int y=0; y<line.height(); y++) {
                                res.maxY=qMax(res.output_zs+y, res.maxY);
                                if (res.output_zs<res.img.height()) {
                                    for (int x=0; x<line.width(); x++)
                                    {
                                        res.img(x, res.output_zs+y, 0,c)=line(x, y, 0, c);
                                    }
                                }
                            }
                        }
                        res.output_zs+=line.height();
                    }
                }
                results[j].z++;
            } else {
                if (pi.angleMode==AngleMode::AnglePitch) {
                    // for pitch we need a dummy extract that WILL UPDATE res.zs_val
                    // for AngleRoll and AngleNone this is not required, as these always step +1 through the stack
                    TIME_BLOCK_SW(timer, "extractZY_atz_pitch()");
                    extractZY_atz_pitch(z, outputFrames, frame, pi.location_x, pi.angle,InterpolationMethod2XYFunctor(interpolationMethod), pi.get_slit_offset(), pi.get_slit_width(), res.zs_val);
                    //qDebug()<<"DUMMY: extractZY_atz_pitch: z="<<z<<", res.zs_val="<<res.zs_val;
                }
            }




            // process stills
            if (stills<stillCnt && z%stillDelta==0) {
                auto frame_s=frame;
                const unsigned char color[] = { 255,0,0 };
                if (pi. filteredAngleMode()==AngleMode::AngleNone && pi.mode==Mode::ZY && z<res.img.width()) {
                    if (pi.angleMode==AngleMode::AngleNone || pi.angle==0) {
                        for (int x=pi.location_x-stilllw/2; x<pi.location_x-stilllw/2+stilllw; x++) {
                            frame_s.draw_line(x,0,x,frame.height(), color);
                        }
                    }
                } else if (pi. filteredAngleMode()==AngleMode::AngleNone && pi.mode==Mode::XZ && z<res.img.height()) {
                    if (pi.angleMode==AngleMode::AngleNone || pi.angle==0) {
                        for (int y=pi.location_y-stilllw/2; y<pi.location_y-stilllw/2+stilllw; y++) {
                            frame_s.draw_line(0,y,frame.width(),y, color);
                        }
                    }
                }
                if (stillSeparateFiles) {
                    const QFileInfo fi(filename);
                    const QString fn=m_outputDir.absoluteFilePath(QString("%1_stack%3_still%2.%4").arg(outputBasename).arg(z+1, 3, 10,QChar('0')).arg(j+1, 3, 10,QChar('0')).arg(FileFormat2Extension(outputFileFormat)));
                    if (m_writer) m_writer->saveImage(fn.toStdString(), ImageWriter::StillImage, frame_s);
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
        if (m_reporter) m_reporter->reportFrameProgress(z+1, outputFrames);
        bool res;
        {
            TIME_BLOCK_SW(timer, "readNext()");
            z_all++;
            res=m_reader->readNext(frame);
        }
        z++;
        if (!res || z_all>=lastFrame-1) {
            m_saving=true;
            m_savingFrame=0;
        }
        return true;
    } else {
        // 2 Step: saving frames
        if (static_cast<int>(m_savingFrame)<results.size()) {
            TIME_BLOCK_SW(timer, "SaveResults()");

            const ProcessingTask::ProcessingItem& pi=pis[m_savingFrame];
            ProcessingTask::ResultData & res=results[m_savingFrame];


            if (m_reporter) m_reporter->reportMessageSavedResult(res.filename.toStdString(), m_savingFrame+1);


            qDebug()<<"finalizing j="<<m_savingFrame<<":    res.output_zs= "<<res.output_zs<<", res.addAfterImageSTackPixels="<<res.addAfterImageSTackPixels<<", res.img="<<CImgSize2String(res.img)<<", pi.addAfter="<<static_cast<int>(pi.addAfter)<<" frame="<<CImgSize2String(frame);

            cimg_library::CImg<uint8_t> line;
            if (pi.mode==Mode::ZY) {
                // ensure that the addedAfter image is also accounted for
                if (pi.addAfter==AddBeforeAfterMode::ToLower) {
                    for (int c=0; c<3; c++) {
                        for (int y=0; y<qMin(res.img.height(),frame.height()); y++) {
                            for(int x=0; x<res.addAfterImageSTackPixels; x++) {
                                const int xf=pi.location_x-1-x;
                                const int xr=res.output_zs+x;
                                if (xf>=0 && xf<frame.width() && xr>=0 && xr<res.img.width()) {
                                    res.img(xr,y,0,c)=frame(xf,y,0,c);
                                    res.maxX=qMax(res.maxX, xr);
                                }
                            }
                        }
                    }
                }
                else if (pi.addAfter==AddBeforeAfterMode::ToHigher) {
                    for (int c=0; c<3; c++) {
                        for (int y=0; y<qMin(res.img.height(),frame.height()); y++) {
                            for(int x=0; x<res.addAfterImageSTackPixels; x++) {
                                const int xf=pi.location_x+1+x;
                                const int xr=res.output_zs+x;
                                if (xf>=0 && xf<frame.width() && xr>=0 && xr<res.img.width()) {
                                    res.img(xr,y,0,c)=frame(xf,y,0,c);
                                    res.maxX=qMax(res.maxX, xr);
                                }
                            }
                        }
                    }
                }
                res.output_zs+=res.addAfterImageSTackPixels;

            } else if (pi.mode==Mode::XZ) {
                // ensure that the addedAfter image is also accounted for
                if (pi.addAfter==AddBeforeAfterMode::ToLower) {
                    for (int c=0; c<3; c++) {
                        for (int y=0; y<res.addAfterImageSTackPixels; y++) {
                            res.maxY=qMax(res.maxY, res.output_zs+y);
                            for(int x=0; x<qMin(res.img.width(),frame.width()); x++) {
                                const int yf=pi.location_y-1-y;
                                const int yr=res.output_zs+y;
                                if (yf>=0 && yf<frame.height() && yr>=0 && yr<res.img.height()) res.img(x,yr,0,c)=frame(x,yf,0,c);
                            }
                        }
                    }
                }
                else if (pi.addAfter==AddBeforeAfterMode::ToHigher) {
                    for (int c=0; c<3; c++) {
                        for (int y=0; y<res.addAfterImageSTackPixels; y++) {
                            res.maxY=qMax(res.maxY, res.output_zs+y);
                            for(int x=0; x<qMin(res.img.width(),frame.width()); x++) {
                                const int yf=pi.location_y+1+y;
                                const int yr=res.output_zs+y;
                                if (yf>=0 && yf<frame.height() && yr>=0 && yr<res.img.height()) res.img(x,yr,0,c)=frame(x,yf,0,c);
                            }
                        }
                    }
                }
                res.output_zs+=res.addAfterImageSTackPixels;
            }


            qDebug()<<"img "<<m_savingFrame<<" ("<<CImgSize2String(res.img)<<"): maxX="<<res.maxX<<", maxY="<<res.maxY;
            auto unfilteredImage=res.img.get_crop(0,0,qMin(res.img.width()-1,res.maxX),qMin(res.img.height()-1,res.maxY));
            auto filteredImage=unfilteredImage;

            bool hasMod=false;
            // normalize image if necessary
            if (normalize) {
                if (pi.mode==Mode::ZY && normalizeY>=0 && normalizeY<filteredImage.height()) {
                    qDebug()<<"normalizeZY: normalizeY="<<normalizeY<<" ("<<CImgSize2String(filteredImage)<<")";
                    normalizeZY(filteredImage, normalizeY);
                    hasMod=true;
                }
                if (pi.mode==Mode::XZ && normalizeX>=0 && normalizeX<filteredImage.width()) {
                    qDebug()<<"normalizeXZ: normalizeX="<<normalizeX<<" ("<<CImgSize2String(filteredImage)<<")";
                    normalizeXZ(filteredImage, normalizeX);
                    hasMod=true;
                }
            }
            if (filterNotch) {
                applyFilterNotch(filteredImage, fiterNotchWavelength, fiterNotchWidth);
                hasMod=true;
            }
            if (modifyWhite) {
                applyWhitepointCorrection(filteredImage, whitepointR, whitepointG, whitepointB);
                hasMod=true;
            }

            if (hasMod) {
                qDebug()<<"saving "<<CImgSize2String(unfilteredImage)<<" to "<<res.filename;
                if (m_writer) res.filename=QString::fromStdString(m_writer->saveImage(res.filename.toStdString(), ImageWriter::IntermediateImage, unfilteredImage));
                qDebug()<<"   --> "<<res.filename;
                qDebug()<<"saving filtered "<<CImgSize2String(filteredImage)<<" to "<<res.filt_filename;
                if (m_writer) res.filt_filename=QString::fromStdString(m_writer->saveImage(res.filt_filename.toStdString(), ImageWriter::FinalImage, filteredImage));
                qDebug()<<"   --> "<<res.filt_filename;
            } else {
                qDebug()<<"saving "<<CImgSize2String(unfilteredImage)<<" to "<<res.filename;
                if (m_writer) res.filename=QString::fromStdString(m_writer->saveImage(res.filename.toStdString(), ImageWriter::FinalImage, unfilteredImage));
                qDebug()<<"   --> "<<res.filename;
            }

            if (!do_not_save_anyting) {
                if (m_configio) {
                    m_configio->open(res.inifilename.toStdString());
                    saveBase(m_configio);
                    pi.save(m_configio, "");
                    m_configio->close();
                }
            }
            if (m_savingFrame<stillStripImg.size()) {
                const QFileInfo fi(filename);
                QString fns=m_outputDir.absoluteFilePath(QString("%1_stack%2_stillstrip.%3").arg(outputBasename).arg(m_savingFrame+1, 3, 10,QChar('0')).arg(FileFormat2Extension(outputFileFormat)));
                qDebug()<<"saving strip "<<CImgSize2String(stillStripImg[m_savingFrame])<<" to "<<fns;
                if (m_writer) fns=QString::fromStdString(m_writer->saveImage(fns.toStdString(), ImageWriter::StillStrip, stillStripImg[m_savingFrame]));
                qDebug()<<"   --> "<<fns;
            }

        }

        m_savingFrame++;
        return (static_cast<int>(m_savingFrame)<results.size());
    }
}

void ProcessingTask::processFinalize()
{
    TIME_BLOCK_SW(timer, "processFinalize()");

    if (m_reader) m_reader->close();
}

void ProcessingTask::normalizeZY(cimg_library::CImg<uint8_t> &img, int normalizeY)
{
    TIME_BLOCK_SW(timer, "normalizeZY()");
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
    TIME_BLOCK_SW(timer, "normalizeXZ()");
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
    TIME_BLOCK_SW(timer, "applyFilterNotch()");

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
    TIME_BLOCK_SW(timer, "applyWhitepointCorrection()");
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



ProcessingTask::ResultData::ResultData():
    maxX(0),
    maxY(0),
    zs_val(0),
    output_zs(0),
    addAfterImageSTackPixels(0),
    z(0)
{

}
