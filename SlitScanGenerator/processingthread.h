#ifndef PROCESSINGTHREAD_H
#define PROCESSINGTHREAD_H
#include <QThread>
#include <QObject>
#include <QVBoxLayout>
#include <atomic>


class ProcessingWidget; // forward
class ProcessingTask;

class ProcessingThread : public QThread
{
        Q_OBJECT
    public:
        explicit ProcessingThread(ProcessingTask* item, QObject *parent = 0);

        ~ProcessingThread();
        bool isDone() const;
        bool isWaiting() const;
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
        void setMessage(const QString& msg);
        void setRange(int minn, int maxx);
        void setValue(int val);

        std::atomic<bool> m_canceled;
        std::atomic<bool> m_started;
        std::atomic<bool> m_done;
        ProcessingTask* m_item;


};

#endif // PROCESSINGTHREAD_H
