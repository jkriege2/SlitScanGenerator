#ifndef VIDEORANGESLIDER_H
#define VIDEORANGESLIDER_H

#include <QImage>
#include <QWidget>
#include <QVector>

class QMouseEvent;
class QNativeGestureEvent;
class QPaintEvent;
class QWheelEvent;

class VideoRangeSlider : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool previewOutsideRange READ previewOutsideRange WRITE setPreviewOutsideRange NOTIFY previewOutsideRangeChanged)

public:
    explicit VideoRangeSlider(QWidget *parent = nullptr);

    void setFrameCount(int frameCount);
    void setThumbnails(const QVector<QImage> &images, const QVector<int> &frames);
    int startFrame() const;
    int endFrame() const;
    int currentFrame() const;
    bool previewOutsideRange() const;

public slots:
    void setStartFrame(int frame);
    void setEndFrame(int frame);
    void setCurrentFrame(int frame);
    void setPreviewOutsideRange(bool enabled);

signals:
    void startFrameChanged(int frame);
    void endFrameChanged(int frame);
    void currentFrameChanged(int frame);
    void previewOutsideRangeChanged(bool enabled);

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    enum class DragMode { None, Start, End, Current };

    int frameToX(int frame) const;
    int xToFrame(int x) const;
    void setCurrentFrameFromUser(int frame);
    bool isNearRangeHandle(int x, int y, DragMode *mode=nullptr) const;

    int m_frameCount=1;
    int m_startFrame=1;
    int m_endFrame=1;
    int m_currentFrame=1;
    bool m_previewOutsideRange=false;
    QVector<QImage> m_thumbnails;
    QVector<int> m_thumbnailFrames;
    DragMode m_dragMode=DragMode::None;
    int m_wheelDeltaRemainder=0;
    int m_zoomGestureStartFrame=1;
    int m_zoomGestureEndFrame=1;
    double m_zoomGestureRemainder=0.0;
    bool m_zoomGestureActive=false;
};

#endif // VIDEORANGESLIDER_H