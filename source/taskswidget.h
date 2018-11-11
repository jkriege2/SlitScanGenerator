#ifndef TASKSWIDGET_H
#define TASKSWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include "processingthread.h"


class TasksWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit TasksWidget(QWidget *parent = 0);
        void push_back(ProcessingThread* thr);
    protected slots:
        void threadDone(ProcessingThread* thread);
        void startNext();
    private:
        QVBoxLayout* m_layout;
        QVector<ProcessingThread*> m_threads;
        QVector<ProcessingWidget*> m_widgets;
        int m_running;
};

#endif // TASKSWIDGET_H
