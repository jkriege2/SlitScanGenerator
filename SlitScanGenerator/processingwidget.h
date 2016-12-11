#ifndef PROCESSINGWIDGET_H
#define PROCESSINGWIDGET_H

#include <QWidget>

namespace Ui {
    class ProcessingWidget;
}

class ProcessingWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit ProcessingWidget(QWidget *parent = 0);
        ~ProcessingWidget();
        bool wasCanceled() const;
    public slots:
        void setMessage(const QString& msg);
        void setRange(int minn, int maxx);
        void setValue(int val);
    signals:
        void cancelClicked();
    protected slots:
        void on_btnCancel_clicked();
    private:
        Ui::ProcessingWidget *ui;
        bool m_canceled;
};

#endif // PROCESSINGWIDGET_H
