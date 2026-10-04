#include "videorangeslider.h"

#include <QEvent>
#include <QHelpEvent>
#include <QMouseEvent>
#include <QNativeGestureEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QToolTip>
#include <QtMath>
#include <QWheelEvent>

VideoRangeSlider::VideoRangeSlider(QWidget *parent) : QWidget(parent)
{
    setMinimumHeight(96);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setToolTip(tr("Drag the handles to set the range; two-finger scroll scrubs the preview, pinch zooms the range, and double-click restores the full range."));
}

void VideoRangeSlider::setFrameCount(int frameCount)
{
    m_frameCount=qMax(1, frameCount);
    m_startFrame=qBound(1, m_startFrame, m_frameCount);
    m_endFrame=qBound(m_startFrame, m_endFrame, m_frameCount);
    m_currentFrame=qBound(1, m_currentFrame, m_frameCount);
    if (!m_previewOutsideRange) m_currentFrame=qBound(m_startFrame, m_currentFrame, m_endFrame);
    update();
}

void VideoRangeSlider::setThumbnails(const QVector<QImage> &images, const QVector<int> &frames)
{
    m_thumbnails=images;
    m_thumbnailFrames=frames;
    if (m_thumbnailFrames.size()>m_thumbnails.size()) m_thumbnailFrames.resize(m_thumbnails.size());
    if (m_thumbnails.size()>m_thumbnailFrames.size()) m_thumbnails.resize(m_thumbnailFrames.size());
    update();
}

void VideoRangeSlider::setStartFrame(int frame)
{
    const int value=qBound(1, frame, m_endFrame);
    if (value==m_startFrame) return;
    m_startFrame=value;
    if (!m_previewOutsideRange && m_currentFrame<m_startFrame) {
        m_currentFrame=m_startFrame;
        emit currentFrameChanged(m_currentFrame);
    }
    update();
}

void VideoRangeSlider::setEndFrame(int frame)
{
    const int value=qBound(m_startFrame, frame, m_frameCount);
    if (value==m_endFrame) return;
    m_endFrame=value;
    if (!m_previewOutsideRange && m_currentFrame>m_endFrame) {
        m_currentFrame=m_endFrame;
        emit currentFrameChanged(m_currentFrame);
    }
    update();
}

void VideoRangeSlider::setCurrentFrame(int frame)
{
    int value=qBound(1, frame, m_frameCount);
    if (!m_previewOutsideRange) value=qBound(m_startFrame, value, m_endFrame);
    if (value==m_currentFrame) return;
    m_currentFrame=value;
    update();
}

int VideoRangeSlider::startFrame() const { return m_startFrame; }
int VideoRangeSlider::endFrame() const { return m_endFrame; }
int VideoRangeSlider::currentFrame() const { return m_currentFrame; }
bool VideoRangeSlider::previewOutsideRange() const { return m_previewOutsideRange; }

void VideoRangeSlider::setPreviewOutsideRange(bool enabled)
{
    if (m_previewOutsideRange==enabled) return;
    m_previewOutsideRange=enabled;
    if (!enabled) {
        const int clamped=qBound(m_startFrame, m_currentFrame, m_endFrame);
        if (clamped!=m_currentFrame) {
            m_currentFrame=clamped;
            emit currentFrameChanged(m_currentFrame);
        }
    }
    emit previewOutsideRangeChanged(m_previewOutsideRange);
    update();
}

int VideoRangeSlider::frameToX(int frame) const
{
    const int left=10;
    const int right=qMax(left+1, width()-10);
    if (m_frameCount<=1) return left;
    const double fraction=double(qBound(1, frame, m_frameCount)-1)/double(m_frameCount-1);
    return left+qRound(fraction*(right-left));
}

int VideoRangeSlider::xToFrame(int x) const
{
    const int left=10;
    const int right=qMax(left+1, width()-10);
    const double fraction=qBound(0.0, double(x-left)/double(right-left), 1.0);
    return 1+qRound(fraction*double(m_frameCount-1));
}

bool VideoRangeSlider::isNearRangeHandle(int x, int y, DragMode *mode) const
{
    const int trackY=height()-25;
    if (y<trackY-4 || y>trackY+12) return false;

    const int startDistance=qAbs(x-frameToX(m_startFrame));
    const int endDistance=qAbs(x-frameToX(m_endFrame));
    if (startDistance>12 && endDistance>12) return false;
    if (mode) *mode=(startDistance<=endDistance) ? DragMode::Start : DragMode::End;
    return true;
}

void VideoRangeSlider::setCurrentFrameFromUser(int frame)
{
    int value=qBound(1, frame, m_frameCount);
    if (!m_previewOutsideRange) value=qBound(m_startFrame, value, m_endFrame);
    if (value==m_currentFrame) return;
    m_currentFrame=value;
    emit currentFrameChanged(m_currentFrame);
    update();
}

void VideoRangeSlider::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.fillRect(rect(), palette().color(QPalette::Base));

    const int trackY=height()-25;
    const QRect thumbnailArea(10, 18, qMax(1, width()-20), qMax(24, trackY-26));
    painter.fillRect(thumbnailArea, QColor(24, 27, 29));

    if (!m_thumbnails.isEmpty()) {
        for (int i=0; i<m_thumbnails.size(); ++i) {
            const int frame=(i<m_thumbnailFrames.size()) ? m_thumbnailFrames[i] : 1;
            const int center=frameToX(frame);
            const int left=(i==0) ? thumbnailArea.left() : (frameToX(m_thumbnailFrames[i-1])+center)/2;
            const int right=(i==m_thumbnails.size()-1) ? thumbnailArea.right() : (center+frameToX(m_thumbnailFrames[i+1]))/2;
            const QRect tile(qBound(thumbnailArea.left(),left,thumbnailArea.right()), thumbnailArea.top(),
                             qMax(1,qBound(thumbnailArea.left(),right,thumbnailArea.right())-qBound(thumbnailArea.left(),left,thumbnailArea.right())+1),
                             thumbnailArea.height());
            const QImage &thumbnail=m_thumbnails[i];
            const double tileAspect=double(tile.width())/qMax(1,tile.height());
            const double imageAspect=double(thumbnail.width())/qMax(1,thumbnail.height());
            QRect source(0,0,thumbnail.width(),thumbnail.height());
            if (imageAspect>tileAspect) {
                source.setWidth(qMax(1,qRound(thumbnail.height()*tileAspect)));
                source.moveLeft((thumbnail.width()-source.width())/2);
            } else {
                source.setHeight(qMax(1,qRound(thumbnail.width()/tileAspect)));
                source.moveTop((thumbnail.height()-source.height())/2);
            }
            painter.drawImage(tile, thumbnail, source);
            painter.setPen(QColor(255,255,255,45));
            painter.drawLine(tile.topRight(), tile.bottomRight());
        }
    }

    const int startX=frameToX(m_startFrame);
    const int endX=frameToX(m_endFrame);
    painter.fillRect(QRect(thumbnailArea.left(), thumbnailArea.top(), qMax(0,startX-thumbnailArea.left()), thumbnailArea.height()), QColor(30,30,30,170));
    painter.fillRect(QRect(endX+1, thumbnailArea.top(), qMax(0,thumbnailArea.right()-endX), thumbnailArea.height()), QColor(30,30,30,170));

    painter.setPen(QPen(palette().color(QPalette::Mid), 2));
    painter.drawLine(10, trackY, width()-10, trackY);
    painter.setPen(QPen(palette().color(QPalette::Highlight), 4));
    painter.drawLine(startX, trackY, endX, trackY);

    QPainterPath startHandle;
    startHandle.moveTo(startX-6, trackY+1);
    startHandle.lineTo(startX+6, trackY+1);
    startHandle.lineTo(startX, trackY+10);
    startHandle.closeSubpath();
    QPainterPath endHandle;
    endHandle.moveTo(endX-6, trackY+1);
    endHandle.lineTo(endX+6, trackY+1);
    endHandle.lineTo(endX, trackY+10);
    endHandle.closeSubpath();
    painter.setPen(Qt::NoPen);
    painter.setBrush(palette().color(QPalette::Highlight));
    painter.drawPath(startHandle);
    painter.drawPath(endHandle);

    const int currentX=frameToX(m_currentFrame);
    const QColor currentColor(220,45,55);
    painter.setPen(QPen(currentColor, 3));
    painter.drawLine(currentX, thumbnailArea.top(), currentX, trackY-1);
    painter.setPen(QPen(QColor(255,255,255), 1));
    painter.setBrush(currentColor);
    painter.drawRoundedRect(QRect(currentX-7,2,14,10),3,3);
    QPainterPath topHandle;
    topHandle.moveTo(currentX-6,11);
    topHandle.lineTo(currentX+6,11);
    topHandle.lineTo(currentX,18);
    topHandle.closeSubpath();
    painter.drawPath(topHandle);
}

bool VideoRangeSlider::event(QEvent *event)
{
    if (event->type()==QEvent::NativeGesture) {
        auto *gesture=static_cast<QNativeGestureEvent*>(event);
        if (gesture->gestureType()==Qt::BeginNativeGesture) {
            m_zoomGestureActive=true;
            m_zoomGestureStartFrame=m_startFrame;
            m_zoomGestureEndFrame=m_endFrame;
            m_zoomGestureRemainder=0.0;
            event->accept();
            return true;
        }
        if (gesture->gestureType()==Qt::EndNativeGesture) {
            m_zoomGestureActive=false;
            m_zoomGestureRemainder=0.0;
            event->accept();
            return true;
        }
        if (gesture->gestureType()==Qt::ZoomNativeGesture) {
            if (!m_zoomGestureActive) {
                m_zoomGestureActive=true;
                m_zoomGestureStartFrame=m_startFrame;
                m_zoomGestureEndFrame=m_endFrame;
                m_zoomGestureRemainder=0.0;
            }
            m_zoomGestureRemainder+=gesture->value();
            const int originalLength=m_zoomGestureEndFrame-m_zoomGestureStartFrame+1;
            const int newLength=qBound(1,qRound(originalLength*qExp(-2.0*m_zoomGestureRemainder)),m_frameCount);
            const double center=(m_zoomGestureStartFrame+m_zoomGestureEndFrame)/2.0;
            int newStart=qRound(center-(newLength-1)/2.0);
            newStart=qMax(1,newStart);
            int newEnd=newStart+newLength-1;
            if (newEnd>m_frameCount) {
                newEnd=m_frameCount;
                newStart=newEnd-newLength+1;
            }
            if (newStart!=m_startFrame || newEnd!=m_endFrame) {
                m_startFrame=newStart;
                m_endFrame=newEnd;
                emit startFrameChanged(m_startFrame);
                emit endFrameChanged(m_endFrame);
                if (!m_previewOutsideRange) {
                    const int clamped=qBound(m_startFrame,m_currentFrame,m_endFrame);
                    if (clamped!=m_currentFrame) {
                        m_currentFrame=clamped;
                        emit currentFrameChanged(m_currentFrame);
                    }
                }
                update();
            }
            event->accept();
            return true;
        }
    }
    if (event->type()==QEvent::ToolTip) {
        auto *helpEvent=static_cast<QHelpEvent*>(event);
        const QPoint position=helpEvent->pos();
        DragMode rangeHandle=DragMode::None;
        if (isNearRangeHandle(position.x(),position.y(),&rangeHandle)) {
            const bool isStart=rangeHandle==DragMode::Start;
            const QString label=isStart ? tr("Start frame") : tr("End frame");
            const int frame=isStart ? m_startFrame : m_endFrame;
            QToolTip::showText(helpEvent->globalPos(), tr("%1: %2 (inclusive)\nDrag or scroll to adjust.").arg(label).arg(frame), this);
            event->accept();
            return true;
        }

        if (qAbs(position.x()-frameToX(m_currentFrame))<=10 && position.y()<=height()-25) {
            QToolTip::showText(helpEvent->globalPos(), tr("Preview frame: %1\nDrag or scroll to change the preview.").arg(m_currentFrame), this);
            event->accept();
            return true;
        }
        QToolTip::hideText();
        event->ignore();
        return true;
    }
    return QWidget::event(event);
}

void VideoRangeSlider::mousePressEvent(QMouseEvent *event)
{
    if (event->button()!=Qt::LeftButton) return;
    const int x=qRound(event->position().x());
    const int y=qRound(event->position().y());
    DragMode rangeHandle=DragMode::None;
    if (isNearRangeHandle(x,y,&rangeHandle)) {
        m_dragMode=rangeHandle;
    } else {
        m_dragMode=DragMode::Current;
    }
    mouseMoveEvent(event);
}

void VideoRangeSlider::mouseDoubleClickEvent(QMouseEvent *event)
{
    const int y=qRound(event->position().y());
    const int trackY=height()-25;
    if (event->button()==Qt::LeftButton && y>=18 && y<trackY) {
        m_dragMode=DragMode::None;
        const bool startChanged=m_startFrame!=1;
        const bool endChanged=m_endFrame!=m_frameCount;
        m_startFrame=1;
        m_endFrame=m_frameCount;
        if (startChanged) emit startFrameChanged(m_startFrame);
        if (endChanged) emit endFrameChanged(m_endFrame);
        update();
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

void VideoRangeSlider::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton) || m_dragMode==DragMode::None) return;
    const int frame=xToFrame(qRound(event->position().x()));
    if (m_dragMode==DragMode::Start) {
        const int value=qMin(frame, m_endFrame);
        if (value!=m_startFrame) {
            m_startFrame=value;
            emit startFrameChanged(value);
            if (!m_previewOutsideRange && m_currentFrame<m_startFrame) setCurrentFrameFromUser(m_startFrame);
            update();
        }
    } else if (m_dragMode==DragMode::End) {
        const int value=qMax(frame, m_startFrame);
        if (value!=m_endFrame) {
            m_endFrame=value;
            emit endFrameChanged(value);
            if (!m_previewOutsideRange && m_currentFrame>m_endFrame) setCurrentFrameFromUser(m_endFrame);
            update();
        }
    } else {
        setCurrentFrameFromUser(frame);
    }
}

void VideoRangeSlider::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button()==Qt::LeftButton) m_dragMode=DragMode::None;
}

void VideoRangeSlider::wheelEvent(QWheelEvent *event)
{
    int delta=event->angleDelta().y();
    if (delta==0) delta=event->angleDelta().x();
    if (delta==0) {
        delta=event->pixelDelta().y()*3;
        if (delta==0) delta=event->pixelDelta().x()*3;
    }
    m_wheelDeltaRemainder+=delta;
    const int steps=m_wheelDeltaRemainder/120;
    m_wheelDeltaRemainder-=steps*120;
    if (steps==0) {
        event->accept();
        return;
    }

    DragMode rangeHandle=DragMode::None;
    const QPoint position=event->position().toPoint();
    if (isNearRangeHandle(position.x(),position.y(),&rangeHandle)) {
        if (rangeHandle==DragMode::Start) {
            const int value=qBound(1,m_startFrame-steps,m_endFrame);
            if (value!=m_startFrame) {
                m_startFrame=value;
                emit startFrameChanged(value);
                if (!m_previewOutsideRange && m_currentFrame<m_startFrame) setCurrentFrameFromUser(m_startFrame);
                update();
            }
        } else {
            const int value=qBound(m_startFrame,m_endFrame-steps,m_frameCount);
            if (value!=m_endFrame) {
                m_endFrame=value;
                emit endFrameChanged(value);
                if (!m_previewOutsideRange && m_currentFrame>m_endFrame) setCurrentFrameFromUser(m_endFrame);
                update();
            }
        }
    } else {
        setCurrentFrameFromUser(m_currentFrame-steps);
    }
    event->accept();
}