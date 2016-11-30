#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QLabel>
#include <QMouseEvent>

class ImageViewer : public QLabel
{
        Q_OBJECT
    public:
        explicit ImageViewer(QWidget *parent = 0);
    signals:
        void mouseClicked(int x, int y);
    protected:
        void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
        void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
};

#endif // IMAGEVIEWER_H
