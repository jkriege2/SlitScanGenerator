#include "processingwidget.h"
#include "ui_processingwidget.h"

ProcessingWidget::ProcessingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProcessingWidget),
    m_canceled(false)
{
    ui->setupUi(this);
}

ProcessingWidget::~ProcessingWidget()
{
    delete ui;
}

bool ProcessingWidget::wasCanceled() const
{
    return m_canceled;
}

void ProcessingWidget::setMessage(const QString &msg)
{
    ui->label->setText(msg);
}

void ProcessingWidget::setRange(int minn, int maxx)
{
    ui->progressBar->setRange(minn, maxx);
}

void ProcessingWidget::setValue(int val)
{
    ui->progressBar->setValue(val);
}

void ProcessingWidget::on_btnCancel_clicked()
{
    m_canceled=true;
    ui->btnCancel->setEnabled(false);
    emit cancelClicked();
}

