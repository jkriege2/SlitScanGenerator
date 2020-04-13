#ifndef PROCESSINGTASK_H
#define PROCESSINGTASK_H
#include <QString>
#include <vector>
#include <QSettings>
#include "ffmpeg_tools.h"
#include "cimg_tools.h"

struct ProcessingTask
{
    public:
        enum class Mode {
            XZ,
            ZY
        };

        enum class AngleMode {
            AngleNone=0,
            AngleRoll=1,
            AnglePitch=2,
        };

        struct ProcessingItem {
        public:
            ProcessingItem();
            Mode mode;
            int location_x;
            int location_y;
            double angle;
            AngleMode angleMode;

            int angleModeForCombo() const;

            AngleMode filteredAngleMode() const;

            void save(QSettings& ini, const QString& basename) const;
            void load(QSettings &ini, const QString &basename) ;
        };

        ProcessingTask();

        QString filename;
        int outputFrames;
        QVector<ProcessingTask::ProcessingItem> pis;

        int stillCnt;
        int stillDelta;
        bool stillStrip;
        bool stillSeparateFiles;
        double stillGap;
        double stillBorder;
        double stillLineWidth;

        bool normalize;
        int normalizeX;
        int normalizeY;

        bool filterNotch;
        double fiterNotchWavelength;
        double fiterNotchWidth;

        void save(const QString& inifilename) const;
        void load(const QString& inifilename);

        bool processInit(int &prog, int &maxProg, QString &message, QString &error);
        bool processStep(int& prog, int& maxProg, QString &message);
        void processFinalize();

        static void normalizeZY(cimg_library::CImg<uint8_t> &img, int normalizeY);
        static void normalizeXZ(cimg_library::CImg<uint8_t> &img, int normalizeX);
        static void applyFilterNotch(cimg_library::CImg<uint8_t> &img, double center, double delta, bool testoutput=false);
    private:
        QVector<cimg_library::CImg<uint8_t> > results;
        QVector<int> zs_vals;
        QVector<QString> result_filenames;
        QVector<QString> resultfilt_filenames;
        QVector<QString> result_inifilenames;
        FFMPEGVideo* vid;
        int z;
        cimg_library::CImg<uint8_t> frame;
        QVector<cimg_library::CImg<uint8_t> > stillStripImg;
        bool m_saving;
        int m_savingFrame;
        int stills;

        void saveBase(QSettings& ini) const;

};

#endif // PROCESSINGTASK_H
