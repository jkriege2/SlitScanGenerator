#include "imageviewer.h"
#include <QMouseEvent>

ImageViewer::ImageViewer(QWidget *parent) : QLabel(parent)
{
    setMouseTracking(true);
    setAlignment(Qt::AlignTop|Qt::AlignLeft);
}

void ImageViewer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit mouseClicked(event->x(), event->y());
    }
    QLabel::mousePressEvent(event);
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit mouseClicked(event->x(), event->y());
    }
    QLabel::mouseMoveEvent(event);
}
