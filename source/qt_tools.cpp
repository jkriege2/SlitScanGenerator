#include "qt_tools.h"


QOverrideCursorGuard::QOverrideCursorGuard(const QCursor &cur):
    m_wasreset(false)
{
    QApplication::setOverrideCursor(cur);
}

QOverrideCursorGuard::~QOverrideCursorGuard()
{
    resetCursor();
}

void QOverrideCursorGuard::changeCursor(const QCursor &cur)
{
    if (!m_wasreset) QApplication::changeOverrideCursor(cur);
}

void QOverrideCursorGuard::resetCursor()
{
    if (!m_wasreset) QApplication::restoreOverrideCursor();
    m_wasreset=true;
}
