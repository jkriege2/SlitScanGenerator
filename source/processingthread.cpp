#include "processingthread.h"
#include "processingwidget.h"
#include "processingtask.h"

ProcessingThread::ProcessingThread(ProcessingTask* item, QObject *parent) :
    QThread(parent),
    m_started(false),
    m_done(false),
    m_item(item)
{
    item->setReporter(this);
}

ProcessingThread::~ProcessingThread() {
    canceled();
    wait();
    if (m_item) {
        delete m_item;
        m_item=nullptr;
    }
}

void ProcessingThread::threadStopped()
{
    emit threadDone(this);
}

void ProcessingThread::canceled()
{
    setReporterWasCanceled();
}

bool ProcessingThread::isWaiting() const
{
    return m_started.load();
}

void ProcessingThread::reportFrameProgress(int currentFrame, int maxFrames, bool updateMessage)
{
    setRange(0, maxFrames);
    setValue(currentFrame+1);
    if (updateMessage) reportDlgMessage(tr("Processing Frame %1/%2 ...").arg(currentFrame+1).arg(maxFrames));
}

void ProcessingThread::reportMessage(const std::string &message)
{
    reportDlgMessage(QString::fromStdString(message));
}

void ProcessingThread::reportMessageSavedResult(const std::string &filename, int n)
{
    reportDlgMessage(tr("Saved Result %2: '%1' ...").arg(QString::fromStdString(filename)).arg(n));
}

void ProcessingThread::reportMessageOpeningVideo()
{
    reportDlgMessage(tr("Opening Video ..."));

}

void ProcessingThread::reportErrorMessage(const std::string &message)
{
    reportDlgMessage("<font color=\"orange\"><b>"+tr("ERROR:")+"</b> <i>"+QString::fromStdString(message)+"</i></font>");
    setRange(0, 100);
    setValue(100);
}

void ProcessingThread::reportCanceledByUser()
{
    reportDlgMessage("<font color=\"orange\"><b>"+tr("CANCELED BY USER!!!")+"</b></font>");
    setRange(0, 100);
    setValue(100);
}

void ProcessingThread::reportProcessingFinished()
{
    reportDlgMessage("<font color=\"darkgreen\"><b>"+tr("FINISHED!!!")+"</b></font>");
    setRange(0, 100);
    setValue(100);
}

bool ProcessingThread::isDone() const
{
    return m_done.load();
}

void ProcessingThread::run()
{
    if (!m_started.load()) {
        m_started=true;
        if (m_item) {
            if (m_item->processInit()) {
                while (m_item->processStep()) {
                    if (getReporterWasCanceled()) {
                        reportCanceledByUser();
                        break;
                    }
                }
                reportProcessingFinished();
            }
            m_item->processFinalize();
        }
    }
    m_done=true;
    threadStopped();
}

void ProcessingThread::reportDlgMessage(const QString &msg)
{
    emit wid_setMessage(msg);
}

void ProcessingThread::setRange(int minn, int maxx)
{
    emit wid_setRange(minn, maxx);
}

void ProcessingThread::setValue(int val)
{
    emit wid_setValue(val);
}
