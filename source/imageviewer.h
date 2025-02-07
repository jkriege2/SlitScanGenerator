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
        void mouseMoved(int x, int y);
        void mouseDragged(int x, int y, Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);
        void mouseDraggedDXDY(int x, int y, int x2, int y2, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);
    protected:
        void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
        void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
        void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    private:
        int move_begin_x=0;
        int move_begin_y=0;
        Qt::MouseButton move_begin_button=Qt::NoButton;
};

#endif // IMAGEVIEWER_H
