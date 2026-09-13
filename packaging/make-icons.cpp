// make-icons -- build-time helper that renders the Ghosty icons (PNG + SVG)
// which the package installs. Not part of the app itself; the drawing mirrors
// src/icon.cpp.
//
//   make-icons <output-icons-dir>
//
#include <QGuiApplication>
#include <QColor>
#include <QDir>
#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QtGlobal>

#include <cstdio>

namespace {

struct Ghost
{
    QRectF r;
    int alpha;
};

// Mirrors icon.cpp: shapes are expressed in 0..1 fractional units so the same
// geometry can be rasterized at any size (PNG) or emitted as an SVG path.

void render(const QSize &size, const QList<Ghost> &ghosts, QPainter &p)
{
    for (const Ghost &g : ghosts) {
        p.save();
        qreal a = g.alpha;
        QColor fill(235, 239, 248, int(255 * a / 255.0));
        QColor out(150, 160, 180, int(255 * a / 255.0));
        const qreal w = g.r.width();
        const qreal h = g.r.height();
        const qreal ox = g.r.x();
        const qreal oy = g.r.y();

        QPainterPath path;
        path.moveTo(ox + 0.28 * w, oy + 0.80 * h);
        path.lineTo(ox + 0.28 * w, oy + 0.30 * h);
        path.cubicTo(ox + 0.28 * w, oy + 0.05 * h, ox + 0.72 * w, oy + 0.05 * h, ox + 0.72 * w, oy + 0.30 * h);
        path.lineTo(ox + 0.72 * w, oy + 0.80 * h);
        path.quadTo(ox + 0.66 * w, oy + 0.94 * h, ox + 0.56 * w, oy + 0.80 * h);
        path.quadTo(ox + 0.50 * w, oy + 0.94 * h, ox + 0.44 * w, oy + 0.80 * h);
        path.quadTo(ox + 0.38 * w, oy + 0.94 * h, ox + 0.28 * w, oy + 0.80 * h);
        path.closeSubpath();

        p.setPen(QPen(out, qMax(1.5, 0.045 * w)));
        p.setBrush(fill);
        p.drawPath(path);

        QPainterPath arm;
        arm.addEllipse(QRectF(ox + 0.11 * w, oy + 0.42 * h, 0.17 * w, 0.34 * h));
        arm.addEllipse(QRectF(ox + 0.72 * w, oy + 0.42 * h, 0.17 * w, 0.34 * h));
        p.drawPath(arm);

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(240, 168, 180, int(0.6 * 255 * a / 255.0)));
        p.drawEllipse(QRectF(ox + 0.30 * w, oy + 0.48 * h, 0.12 * w, 0.07 * h));
        p.drawEllipse(QRectF(ox + 0.58 * w, oy + 0.48 * h, 0.12 * w, 0.07 * h));

        p.setBrush(QColor(52, 58, 74, int(255 * a / 255.0)));
        p.drawEllipse(QRectF(ox + 0.38 * w, oy + 0.30 * h, 0.09 * w, 0.13 * h));
        p.drawEllipse(QRectF(ox + 0.53 * w, oy + 0.30 * h, 0.09 * w, 0.13 * h));
        p.drawEllipse(QRectF(ox + 0.465 * w, oy + 0.55 * h, 0.07 * w, 0.08 * h));
        p.restore();
    }
}

void ghostShapes(const QRectF &r, QString &path,
                 QString &arm, QString &blush, QString &eyes, QString &mouth)
{
    const qreal w = r.width();
    const qreal h = r.height();
    const qreal ox = r.x();
    const qreal oy = r.y();
    auto n = [](const char *fmt, double v) -> QString { char b[64]; snprintf(b, sizeof b, fmt, v); return QString::fromLatin1(b); };
    path = QString::fromLatin1("M %1 %2 L %3 %4 C %5 %6 %7 %8 %9 %10 L %11 %12 Q %13 %14 %15 %16 Q %17 %18 %19 %20 Q %21 %22 %23 %24 Z")
        .arg(n("%.2f", ox + 0.28 * w), n("%.2f", oy + 0.80 * h),
             n("%.2f", ox + 0.28 * w), n("%.2f", oy + 0.30 * h),
             n("%.2f", ox + 0.28 * w), n("%.2f", oy + 0.05 * h),
             n("%.2f", ox + 0.72 * w), n("%.2f", oy + 0.05 * h),
             n("%.2f", ox + 0.72 * w), n("%.2f", oy + 0.30 * h),
             n("%.2f", ox + 0.72 * w), n("%.2f", oy + 0.80 * h),
             n("%.2f", ox + 0.66 * w), n("%.2f", oy + 0.94 * h),
             n("%.2f", ox + 0.56 * w), n("%.2f", oy + 0.80 * h),
             n("%.2f", ox + 0.50 * w), n("%.2f", oy + 0.94 * h),
             n("%.2f", ox + 0.44 * w), n("%.2f", oy + 0.80 * h),
             n("%.2f", ox + 0.38 * w), n("%.2f", oy + 0.94 * h),
             n("%.2f", ox + 0.28 * w), n("%.2f", oy + 0.80 * h));
    arm = QString::fromLatin1("<circle cx=\"%1\" cy=\"%2\" r=\"%3\"/>\n  "
                              "<circle cx=\"%4\" cy=\"%5\" r=\"%6\"/>")
        .arg(n("%.2f", ox + 0.195 * w), n("%.2f", oy + 0.59 * h), n("%.2f", 0.085 * w),
             n("%.2f", ox + 0.805 * w), n("%.2f", oy + 0.59 * h), n("%.2f", 0.085 * w));
    blush = QString::fromLatin1("<ellipse cx=\"%1\" cy=\"%2\" rx=\"%3\" ry=\"%4\"/>\n  "
                                "<ellipse cx=\"%5\" cy=\"%6\" rx=\"%7\" ry=\"%8\"/>")
        .arg(n("%.2f", ox + 0.36 * w), n("%.2f", oy + 0.515 * h), n("%.2f", 0.06 * w), n("%.2f", 0.035 * h),
             n("%.2f", ox + 0.64 * w), n("%.2f", oy + 0.515 * h), n("%.2f", 0.06 * w), n("%.2f", 0.035 * h));
    eyes = QString::fromLatin1("<ellipse cx=\"%1\" cy=\"%2\" rx=\"%3\" ry=\"%4\"/>\n  "
                               "<ellipse cx=\"%5\" cy=\"%6\" rx=\"%7\" ry=\"%8\"/>")
        .arg(n("%.2f", ox + 0.425 * w), n("%.2f", oy + 0.365 * h), n("%.2f", 0.045 * w), n("%.2f", 0.065 * h),
             n("%.2f", ox + 0.575 * w), n("%.2f", oy + 0.365 * h), n("%.2f", 0.045 * w), n("%.2f", 0.065 * h));
    mouth = QString::fromLatin1("<ellipse cx=\"%1\" cy=\"%2\" rx=\"%3\" ry=\"%4\"/>")
        .arg(n("%.2f", ox + 0.50 * w), n("%.2f", oy + 0.59 * h), n("%.2f", 0.035 * w), n("%.2f", 0.04 * h));
}

QString svgGhost(const QRectF &r, int alphaPct)
{
    QString path, arm, blush, eyes, mouth;
    ghostShapes(r, path, arm, blush, eyes, mouth);
    return QString::fromLatin1(
        "<g opacity=\"%1\">\n"
        "  <path d=\"%2\" fill=\"#EBEFF8\" stroke=\"#96A0B4\" stroke-width=\"0.045\" stroke-linejoin=\"round\"/>\n"
        "  <g fill=\"#EBEFF8\" stroke=\"#96A0B4\" stroke-width=\"0.045\" stroke-linejoin=\"round\">\n  %3\n  </g>\n"
        "  <g fill=\"#F0A8B4\">\n  %4\n  </g>\n"
        "  <g fill=\"#343A4A\">\n  %5\n  %6\n  </g>\n"
        "</g>")
        .arg(alphaPct)
        .arg(path, arm, blush, eyes, mouth);
}

} // namespace

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    const QString outDir = argc > 1 ? QString::fromUtf8(argv[1]) : QStringLiteral("icons");
    if (!QDir().mkpath(outDir)) {
        std::fprintf(stderr, "cannot create %s\n", qPrintable(outDir));
        return 1;
    }

    // PNGs
    const QList<Ghost> ghosts = {
        { QRectF(0.06, 0.18, 0.60, 0.64), 95 },
        { QRectF(0.26, 0.05, 0.66, 0.66), 250 },
    };
    const int sizes[] = { 22, 32, 48, 64, 128, 256 };
    for (int s : sizes) {
        QPixmap pm(s, s);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);
        render(QSize(s, s), ghosts, p);
        p.end();
        const QString rel = QStringLiteral("%1x%1/apps/ghosty.png").arg(s);
        QDir().mkpath(outDir + QLatin1Char('/') + rel.left(rel.lastIndexOf(QLatin1Char('/'))));
        if (!pm.save(outDir + QLatin1Char('/') + rel)) {
            std::fprintf(stderr, "cannot write %s/%s\n", qPrintable(outDir), qPrintable(rel));
            return 1;
        }
        std::printf("wrote %s/%s\n", qPrintable(outDir), qPrintable(rel));
    }

    // SVG
    const QRectF back(0.06, 0.18, 0.60, 0.64);
    const QRectF front(0.26, 0.05, 0.66, 0.66);
    const QByteArray svg =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 1 1\" width=\"64\" height=\"64\">\n"
        + svgGhost(back, 37).toUtf8() + "\n"
        + svgGhost(front, 100).toUtf8() + "\n"
        + "</svg>\n";

    const QString svgDir = outDir + QStringLiteral("/scalable/apps");
    QDir().mkpath(svgDir);
    QFile f(svgDir + QStringLiteral("/ghosty.svg"));
    if (!f.open(QIODevice::WriteOnly) || f.write(svg) < 0) {
        std::fprintf(stderr, "cannot write ghosty.svg\n");
        return 1;
    }
    f.close();
    std::printf("wrote %s/scalable/apps/ghosty.svg\n", qPrintable(outDir));
    return 0;
}