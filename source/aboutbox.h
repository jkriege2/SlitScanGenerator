#ifndef ABOUTBOX_H
#define ABOUTBOX_H

#include <QDialog>

namespace Ui {
    class AboutBox;
}

class AboutBox : public QDialog
{
        Q_OBJECT

    public:
        explicit AboutBox(QWidget *parent = 0);
        ~AboutBox();

    protected slots:
        void openURL(const QUrl& url);
    private:
        Ui::AboutBox *ui;
};

#endif // ABOUTBOX_H
