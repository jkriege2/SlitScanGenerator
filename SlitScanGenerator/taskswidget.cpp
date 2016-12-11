#include "taskswidget.h"
#include "processingwidget.h"

TasksWidget::TasksWidget(QWidget *parent) :
    QWidget(parent),
    m_running(0)
{
    m_layout=new QVBoxLayout(this);
    setLayout(m_layout);

}

void TasksWidget::push_back(ProcessingThread *thr)
{
    m_threads.push_back(thr);
    m_widgets.push_back(new ProcessingWidget(this));
    m_layout->addWidget(m_widgets.back());
    m_layout->setContentsMargins(2,2,2,2);

    connect(thr, SIGNAL(threadDone(ProcessingThread *)), this, SLOT(threadDone(ProcessingThread *)));
    connect(m_widgets.back(), SIGNAL(cancelClicked()), thr, SLOT(canceled()));
    connect(thr, SIGNAL(wid_setMessage(QString)), m_widgets.back(), SLOT(setMessage(QString)));
    connect(thr, SIGNAL(wid_setRange(int,int)), m_widgets.back(), SLOT(setRange(int,int)));
    connect(thr, SIGNAL(wid_setValue(int)), m_widgets.back(), SLOT(setValue(int)));

    startNext();
}

void TasksWidget::threadDone(ProcessingThread *thread)
{
    int i= m_threads.indexOf(thread);
    if (i>=0) {
        delete m_threads[i];
        delete m_widgets[i];
        m_threads.removeAt(i);
        m_widgets.removeAt(i);
        m_running--;
    }

    startNext();
}

void TasksWidget::startNext()
{
    int maxT=QThread::idealThreadCount()-1;
    if (maxT<0) maxT=1;

    maxT=1;

    for (int i=0; i<m_threads.size(); i++) {
        if (m_running<maxT && !m_threads[i]->isWaiting()) {
            m_threads[i]->start();
            m_running++;
        }
    }
}

