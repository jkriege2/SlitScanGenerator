#ifndef PROCESSINGTHREAD_H
#define PROCESSINGTHREAD_H
#include <QThread>
#include <QObject>
#include <QVBoxLayout>
#include <atomic>
#include "processingtaskreporter.h"


class ProcessingWidget; // forward
class ProcessingTask;

class ProcessingThread : public QThread, public ProcessingTaskReporter
{
        Q_OBJECT
    public:
        explicit ProcessingThread(ProcessingTask* item, QObject *parent = 0);

        ~ProcessingThread();
        bool isDone() const;
        bool isWaiting() const;

        /** \copydoc ProcessingTaskReporter::reportFrameProgress() */
        virtual void reportFrameProgress(int currentFrame, int maxFrames, bool updateMessage=true) override;
        /** \copydoc ProcessingTaskReporter::reportMessage() */
        virtual void reportMessage(const std::string& message) override;
        /** \copydoc ProcessingTaskReporter::reportMessageSavedResult() */
        virtual void reportMessageSavedResult(const std::string& filename, int n) override;
        /** \copydoc ProcessingTaskReporter::reportMessageOpeningVideo() */
        virtual void reportMessageOpeningVideo() override;
        /** \copydoc ProcessingTaskReporter::reportErrorMessage() */
        virtual void reportErrorMessage(const std::string& message) override;
        /** \copydoc ProcessingTaskReporter::reportCanceledByUser() */
        virtual void reportCanceledByUser() override;
        /** \copydoc ProcessingTaskReporter::reportProcessingFinished() */
        virtual void reportProcessingFinished() override;
    signals:

        void threadDone(ProcessingThread* thread);
        void wid_setMessage(const QString& msg);
        void wid_setRange(int minn, int maxx);
        void wid_setValue(int val);

    protected slots:
        void threadStopped();
        void canceled();
    protected:
        virtual void run() override;
        void reportDlgMessage(const QString& msg);
        void setRange(int minn, int maxx);
        void setValue(int val);

        std::atomic<bool> m_started;
        std::atomic<bool> m_done;
        ProcessingTask* m_item;


};

#endif // PROCESSINGTHREAD_H
