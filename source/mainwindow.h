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
        void openVideo(const QString &filename=QString());
        void openExampleVideo();
        void recalcAndRedisplaySamples(int x, int y, double angle, int angleMode);
        void recalcAndRedisplaySamples();
        void ImageClicked(int x, int y);

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


        int lastX;
        int lastY;

        DisplayModes m_mode;

        /** \brief internal video, xyt-scaled version */
        cimg_library::CImg<uint8_t> m_video_xytscaled;
        /** \brief internal video, xy-scaled version */
        cimg_library::CImg<uint8_t> m_video_some_frames;
        QString m_filename;


        /** \brief path to the ffmpeg utility */
        QString m_ffmpegPath;

        ProcessingParameterTable* m_procModel;

        int video_everyNthFrame;
        double video_xyFactor;
        SlitScanGeneratorSettings m_settings;

        /** contains the translations for this application */
        QTranslator m_translator;
        /** \brief contains the translations for qt */
        QTranslator m_translatorQt;

        /** \brief contains the currently loaded language */
        QString m_currLang;
        /** \brief Path containing the translations (*.qm) */
        QString m_langPath;

        QActionGroup* m_langGroup;


        void loadINI(const QString& filename, QString *vfn);
        void loadFromTask(const ProcessingTask& task);
        void saveToTask(ProcessingTask& task) const;
        /** \brief this event is called, when a new translator is loaded or the system language is changed */
        void changeEvent(QEvent *event) override;
    private:
        /** \brief creates the language menu dynamically from the translations-subdirectory */
        void loadLanguages() ;
        /** \brief loads a language by the given language shortcur (e.g. de, en) */
        void loadLanguage(const QString &rLanguage);
};

#endif // MAINWINDOW_H
