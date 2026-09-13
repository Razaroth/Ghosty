#include "icon.h"

#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

namespace {

QPainterPath ghostBody(qreal w, qreal h)
{
    QPainterPath path;
    path.moveTo(0.28 * w, 0.80 * h);
    path.lineTo(0.28 * w, 0.30 * h);
    // Dome over the top.
    path.cubicTo(0.28 * w, 0.05 * h, 0.72 * w, 0.05 * h, 0.72 * w, 0.30 * h);
    path.lineTo(0.72 * w, 0.80 * h);
    // Wavy bottom, right to left.
    path.quadTo(0.66 * w, 0.94 * h, 0.56 * w, 0.80 * h);
    path.quadTo(0.50 * w, 0.94 * h, 0.44 * w, 0.80 * h);
    path.quadTo(0.38 * w, 0.94 * h, 0.28 * w, 0.80 * h);
    path.closeSubpath();
    return path;
}

void drawGhost(QPainter &p, const QRectF &r, int fillAlpha)
{
    const qreal w = r.width();
    const qreal h = r.height();
    const qreal ox = r.x();
    const qreal oy = r.y();

    p.save();

    // Body
    QLinearGradient grad(0, oy + 0.05 * h, 0, oy + 0.85 * h);
    grad.setColorAt(0.0, QColor(252, 253, 255, fillAlpha));
    grad.setColorAt(1.0, QColor(214, 220, 234, fillAlpha));
    p.setPen(QPen(QColor(150, 160, 180, fillAlpha + 10), qMax(1.5, 0.045 * w)));
    p.setBrush(grad);
    p.drawPath(ghostBody(w, h).translated(ox, oy));

    // Arms (stubby nubs on each side)
    QPainterPath arm;
    arm.addEllipse(QRectF(ox + 0.11 * w, oy + 0.42 * h, 0.17 * w, 0.34 * h));
    arm.addEllipse(QRectF(ox + 0.72 * w, oy + 0.42 * h, 0.17 * w, 0.34 * h));
    p.drawPath(arm);

    // Blush
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(240, 168, 180, int(0.6 * fillAlpha)));
    p.drawEllipse(QRectF(ox + 0.30 * w, oy + 0.48 * h, 0.12 * w, 0.07 * h));
    p.drawEllipse(QRectF(ox + 0.58 * w, oy + 0.48 * h, 0.12 * w, 0.07 * h));

    // Eyes
    p.setBrush(QColor(52, 58, 74, fillAlpha + 15));
    p.drawEllipse(QRectF(ox + 0.38 * w, oy + 0.30 * h, 0.09 * w, 0.13 * h));
    p.drawEllipse(QRectF(ox + 0.53 * w, oy + 0.30 * h, 0.09 * w, 0.13 * h));

    // Mouth: a little "o"
    p.setBrush(QColor(52, 58, 74, fillAlpha + 15));
    p.drawEllipse(QRectF(ox + 0.465 * w, oy + 0.55 * h, 0.07 * w, 0.08 * h));

    p.restore();
}

} // namespace

static QPixmap paint(const QSize &size)
{
    QPixmap pm(size);
    pm.fill(Qt::transparent);

    const qreal w = size.width();
    const qreal h = size.height();

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);

    // A faint translucent ghost behind -> nods at the transparency theme.
    drawGhost(p, QRectF(0.06 * w, 0.18 * h, 0.60 * w, 0.64 * h), 95);

    // The front ghost.
    drawGhost(p, QRectF(0.26 * w, 0.05 * h, 0.66 * w, 0.66 * h), 250);

    return pm;
}

QIcon makeAppIcon()
{
    QIcon icon;
    icon.addPixmap(paint(QSize(22, 22)));
    icon.addPixmap(paint(QSize(32, 32)));
    icon.addPixmap(paint(QSize(64, 64)));
    return icon;
}