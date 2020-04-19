#ifndef QT_TOOLS_H
#define QT_TOOLS_H

#include <QApplication>

struct QOverrideCursorGuard {
    QOverrideCursorGuard(const QCursor & cur);
    ~QOverrideCursorGuard();

    QOverrideCursorGuard(const QOverrideCursorGuard&)=delete;
    QOverrideCursorGuard& operator=(const QOverrideCursorGuard&)=delete;
    QOverrideCursorGuard(QOverrideCursorGuard&&)=default;
    QOverrideCursorGuard& operator=(QOverrideCursorGuard&&)=default;

    void changeCursor(const QCursor & cur);
    void resetCursor();
private:
    bool m_wasreset;
};

#endif // QT_TOOLS_H
