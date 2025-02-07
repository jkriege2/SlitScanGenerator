#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <cstdint>
#include <QLabel>
#include <QSettings>
#include <QActionGroup>
#include <QTranslator>
#include "imageviewer.h"
#include "processingparametertable.h"
#include "cimg_tools.h"
#include "ffmpeg_tools.h"
#include "slitscangeneratorsettings.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

    protected slots:
        void saveINI();
        void loadINI();
        void firstFrameChanged(int value);
        void lastFrameChanged(int value);
        void openVideo(const QString &filename=QString(), const QString& ini_in=QString());
        void openExampleVideo();
        void storeProcessingItemToGUIWidgetsAndRedisplayScan(const ProcessingTask::ProcessingItem &pi);
        void redisplayCurrentScan();
        void updateGUIAndRedisplay();
        void ImageClicked(int x, int y);
        void imageDraggedDXDY(int x, int y, int x2, int y2, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);

        void on_btnAddXZ_clicked();
        void on_btnAddZY_clicked();
        void on_btnDelete_clicked();
        void on_btnDeleteAll_clicked();
        void processAll();
        void processINIFile();
        void tableRowClicked(const QModelIndex &index);
        void setWidgetsEnabledForCurrentMode();
        void showAbout();
        void showSettings();
        void test();
        /** \brief this slot is called by the language menu actions */
        void slotLanguageChanged(QAction *action);
        void updateProcessingItemWidgetsEnabledStates();
    private:
        Ui::MainWindow *ui;

        ImageViewer* labXY;
        QLabel* labXZ;
        QLabel* labYZ;
    protected:

        enum class DisplayModes {
            unloaded,
            loaded
        };

        /** \brief current x-center-position of XZ/YZ-cuts, in reduced frame coordinates */
        int lastX_reducedCoords;
        /** \brief current y-center-position of XZ/YZ-cuts, in reduced frame coordinates */
        int lastY_reducedCoords;
        /** \brief current display mode */
        DisplayModes m_mode;

        /** \brief internal video, xyt-scaled version */
        cimg_library::CImg<uint8_t> m_video_xytscaled;
        /** \brief internal video, xy-scaled version */
        cimg_library::CImg<uint8_t> m_video_some_frames;
        /** \brief currently opened video */
        QString m_filename;


        /** \brief path to the ffmpeg utility */
        QString m_ffmpegPath;

        /** \brief model for the ProcessingTask::ProcessingItem list */
        ProcessingParameterTable* m_procModel;

        /** \brief reduction of video resolution in m_video_xytscaled in time-direction (only every this-th frame is contained in m_video_xytscaled) */
        int video_everyNthFrame;
        /** \brief reduction factor of video resolution in m_video_xytscaled in x/y-direction (4 means, the width and height are divided by 4!) */
        double video_xyFactor;

        /** \brief manages the application settings */
        SlitScanGeneratorSettings m_settings;

        /** contains the translations for this application */
        QTranslator m_translator;
        /** \brief contains the translations for qt */
        QTranslator m_translatorQt;

        /** \brief contains the currently loaded language */
        QString m_currLang;
        /** \brief Path containing the translations (*.qm) */
        QString m_langPath;
        /** \brief action group that allows to sleect a language for the GUI */
        QActionGroup* m_langGroup;


        void loadINI(const QString& filename, QString *vfn);
        void loadFromTask(const ProcessingTask& task);
        void saveToTask(ProcessingTask& task, double xyScaling=1.0, double tScaling=1.0) const;
        /** \brief this event is called, when a new translator is loaded or the system language is changed */
        void changeEvent(QEvent *event) override;
        ProcessingTask::ProcessingItem fillProcessingItem(ProcessingTask::Mode mode) const;
        ProcessingTask::ProcessingItem fillProcessingItem(int locX, int locY, int zstep, ProcessingTask::Mode mode) const;
        void setLastXY(int x, int y);
    private:
        /** \brief creates the language menu dynamically from the translations-subdirectory */
        void loadLanguages() ;
        /** \brief loads a language by the given language shortcur (e.g. de, en) */
        void loadLanguage(const QString &rLanguage);
};

#endif // MAINWINDOW_H
