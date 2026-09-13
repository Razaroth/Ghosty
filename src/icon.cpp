#include "icon.h"

#include <QColor>
#include <QPainter>
#include <QPixmap>

static QPixmap paint(const QSize &size, qreal base)
{
    QPixmap pm(size);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);

    const qreal w = size.width();
    const qreal h = size.height();

    // A translucent "second window" behind.
    QColor back(88, 88, 96, 90);
    p.setBrush(back);
    p.setPen(QPen(QColor(210, 210, 215, 220), w * 0.045));
    p.drawRoundedRect(QRectF(w * 0.06, h * 0.10, w * 0.62, h * 0.62), w * 0.14, w * 0.14);

    // The translucent front window.
    QColor front(150, 150, 165, int(210 * base));
    p.setBrush(front);
    p.setPen(QPen(QColor(235, 235, 240, 235), w * 0.045));
    p.drawRoundedRect(QRectF(w * 0.34, h * 0.28, w * 0.60, h * 0.60), w * 0.14, w * 0.14);

    // A small "slider" handle hint.
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(90, 160, 255, 235));
    p.drawRoundedRect(QRectF(w * 0.48, h * 0.16, w * 0.16, h * 0.09), w * 0.045, w * 0.045);

    return pm;
}

QIcon makeAppIcon()
{
    QIcon icon;
    icon.addPixmap(paint(QSize(22, 22), 0.72));
    icon.addPixmap(paint(QSize(32, 32), 0.72));
    icon.addPixmap(paint(QSize(64, 64), 0.72));
    return icon;
}