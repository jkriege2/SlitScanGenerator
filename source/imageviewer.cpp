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
    move_begin_button=event->button();
    move_begin_x=event->position().x();
    move_begin_y=event->position().y();
    QLabel::mousePressEvent(event);
}

void ImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
    move_begin_button=Qt::NoButton;
    move_begin_x=0;
    move_begin_y=0;
    QLabel::mouseReleaseEvent(event);
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    //qDebug()<<"dragging at "<<event->position()<<event->button();
    emit mouseMoved(event->position().x(), event->position().y());
    if (event->buttons()!=Qt::NoButton) {
        if (event->buttons()==move_begin_button) {
            emit mouseDraggedDXDY(move_begin_x, move_begin_y, event->position().x(), event->position().y(), move_begin_button, event->modifiers());
        }
        //qDebug()<<"dragging with buttons at "<<event->position()<<event->button();
        emit mouseDragged(event->position().x(), event->position().y(), event->button(), event->buttons(), event->modifiers());
    }
    QLabel::mouseMoveEvent(event);
}
