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
        emit mouseClicked(event->position().x(), event->position().y());
    }
    QLabel::mousePressEvent(event);
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit mouseClicked(event->position().x(), event->position().y());
    }
    emit mouseMoved(event->position().x(), event->position().y());
    QLabel::mouseMoveEvent(event);
}
