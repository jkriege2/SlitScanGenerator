#include "processingthread.h"
#include "processingwidget.h"
#include "processingtask.h"

ProcessingThread::ProcessingThread(ProcessingTask* item, QObject *parent) :
    QThread(parent),
    m_item(item),
    m_started(false),
    m_canceled(false),
    m_done(false)
{
}

ProcessingThread::~ProcessingThread() {
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
    m_canceled=true;
}

bool ProcessingThread::isWaiting() const
{
    return m_started.load();
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
            QString error;
            QString msg;
            int val=0;
            int valmax=100;
            if (m_item->processInit(val, valmax, msg, error)) {
                setMessage(msg);
                setRange(0, valmax);
                setValue(val);
                while (m_item->processStep(val, valmax, msg)) {
                    setMessage(msg);
                    setRange(0, valmax);
                    setValue(val);
                    if (m_canceled.load()) {
                        setMessage(tr("<font color=\"orange\"><b>CANCELED BY USER!!!</b></font>"));
                        setRange(0, 100);
                        setValue(100);
                        break;
                    }
                }
                setMessage(tr("FINISHED").arg(error));
                setRange(0, 100);
                setValue(100);
            } else {
                setMessage(tr("<font color=\"red\"><b>ERROR:</b> %1</font>").arg(error));
                setRange(0, 100);
                setValue(100);
            }
            m_item->processFinalize();
        }
    }
    m_done=true;
    threadStopped();
}

void ProcessingThread::setMessage(const QString &msg)
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
