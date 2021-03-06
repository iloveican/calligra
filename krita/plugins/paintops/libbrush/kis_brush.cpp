/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_brush.h"

#include <QDomElement>
#include <QFile>
#include <QPoint>

#include <kis_debug.h>
#include <klocale.h>

#include <KoColor.h>
#include <KoColorSpaceRegistry.h>

#include "kis_datamanager.h"
#include "kis_paint_device.h"
#include "kis_global.h"
#include "kis_boundary.h"
#include "kis_image.h"
#include "kis_scaled_brush.h"
#include "kis_qimage_mask.h"
#include "kis_iterator_ng.h"
#include "kis_brush_registry.h"

const static int MAXIMUM_MIPMAP_SCALE = 10;
const static int MAXIMUM_MIPMAP_SIZE  = 400;

KisBrush::ColoringInformation::~ColoringInformation()
{
}

KisBrush::PlainColoringInformation::PlainColoringInformation(const quint8* color) : m_color(color)
{
}

KisBrush::PlainColoringInformation::~PlainColoringInformation()
{
}

const quint8* KisBrush::PlainColoringInformation::color() const
{
    return m_color;
}

void KisBrush::PlainColoringInformation::nextColumn()
{
}

void KisBrush::PlainColoringInformation::nextRow()
{
}

KisBrush::PaintDeviceColoringInformation::PaintDeviceColoringInformation(const KisPaintDeviceSP source, int width)
    : m_source(source)
    , m_iterator(m_source->createHLineConstIteratorNG(0, 0, width))
{
}

KisBrush::PaintDeviceColoringInformation::~PaintDeviceColoringInformation()
{
}

const quint8* KisBrush::PaintDeviceColoringInformation::color() const
{
    return m_iterator->oldRawData();
}

void KisBrush::PaintDeviceColoringInformation::nextColumn()
{
    m_iterator->nextPixel();
}
void KisBrush::PaintDeviceColoringInformation::nextRow()
{
    m_iterator->nextRow();
}


struct KisBrush::Private {
    Private()
        : boundary(0)
        , angle(0)
        , scale(1.0)
        , hasColor(false)
	, brushType(INVALID)
    {}

    ~Private() {
        delete boundary;
    }

    mutable KisBoundary* boundary;
    qreal angle;
    qreal scale;
    bool hasColor;
    enumBrushType brushType;

    qint32 width;
    qint32 height;
    double spacing;
    QPointF hotSpot;
    mutable QVector<KisScaledBrush> scaledBrushes;


};

KisBrush::KisBrush()
    : KoResource("")
    , d(new Private)
{
}

KisBrush::KisBrush(const QString& filename)
    : KoResource(filename)
    , d(new Private)
{
}

KisBrush::KisBrush(const KisBrush& rhs)
    : KoResource("")
    , KisShared()
    , d(new Private)
{
    m_image = rhs.m_image;
    d->brushType = rhs.d->brushType;
    d->width = rhs.d->width;
    d->height = rhs.d->height;
    d->spacing = rhs.d->spacing;
    d->hotSpot = rhs.d->hotSpot;
    d->scaledBrushes.clear();
    d->hasColor = rhs.d->hasColor;
    d->angle = rhs.d->angle;
    d->scale = rhs.d->scale;
    setFilename(rhs.filename());
    // don't copy the boundery, it will be regenerated -- see bug 291910
}

KisBrush::~KisBrush()
{
    clearScaledBrushes();
    delete d;
}

QImage KisBrush::image() const
{
    return m_image;
}

qint32 KisBrush::width() const
{
    return d->width;
}

void KisBrush::setWidth(qint32 width)
{
    d->width = width;
}

qint32 KisBrush::height() const
{
    return d->height;
}

void KisBrush::setHeight(qint32 height)
{
    d->height = height;
}

void KisBrush::setHotSpot(QPointF pt)
{
    double x = pt.x();
    double y = pt.y();

    if (x < 0)
        x = 0;
    else if (x >= width())
        x = width() - 1;

    if (y < 0)
        y = 0;
    else if (y >= height())
        y = height() - 1;

    d->hotSpot = QPointF(x, y);
}

QPointF KisBrush::hotSpot(double scaleX, double scaleY, double rotation) const
{
    Q_UNUSED(scaleY);
    double w = maskWidth( scaleX, rotation);
    double h = maskHeight( scaleX, rotation);

    // The smallest brush we can produce is a single pixel.
    if (w < 1) {
        w = 1;
    }

    if (h < 1) {
        h = 1;
    }

    // XXX: This should take d->hotSpot into account, though it
    // isn't specified by gimp brushes so it would default to the center
    // anyway.
    QPointF p(w / 2, h / 2);
    return p;
}


bool KisBrush::hasColor() const
{
    return d->hasColor;
}

void KisBrush::setHasColor(bool hasColor)
{
    d->hasColor = hasColor;
}


bool KisBrush::canPaintFor(const KisPaintInformation& /*info*/)
{
    return true;
}

void KisBrush::setImage(const QImage& image)
{
    Q_ASSERT(!image.isNull());
    m_image = image;

    setWidth(image.width());
    setHeight(image.height());

    clearScaledBrushes();

}

void KisBrush::setBrushType(enumBrushType type)
{
    d->brushType = type;
}

enumBrushType KisBrush::brushType() const
{
    return d->brushType;
}

void KisBrush::toXML(QDomDocument& /*document*/ , QDomElement& element) const
{
    element.setAttribute("BrushVersion", "2");
}

KisBrushSP KisBrush::fromXML(const QDomElement& element)
{
    KisBrushSP brush = KisBrushRegistry::instance()->getOrCreateBrush(element);
    if(brush && element.attribute("BrushVersion", "1") == "1")
    {
        brush->setScale(brush->scale() * 2.0);
    }
    return brush;
}

qint32 KisBrush::maskWidth(double scale, double angle) const
{
    angle += d->angle;

    // Make sure the angle stay in [0;2*M_PI]
    if(angle < 0) angle += 2 * M_PI;
    if(angle > 2 * M_PI) angle -= 2 * M_PI;
    scale *= d->scale;

    double width_ = width() * scale;
    if(angle == 0.0) return (qint32)ceil(width_ + 1);

    double height_ = height() * scale;

    // Add one for sub-pixel shift
    if (angle >= 0.0 && angle < M_PI_2) {
        return qAbs(static_cast<qint32>(ceil(width_ * cos(angle) + height_ * sin(angle)) + 1));
    } else if (angle >= M_PI_2 && angle < M_PI) {
        return qAbs(static_cast<qint32>(ceil(-width_ * cos(angle) + height_ * sin(angle)) + 1));
    } else if (angle >= M_PI && angle < (M_PI + M_PI_2)) {
        return qAbs(static_cast<qint32>(ceil(-width_ * cos(angle) - height_ * sin(angle)) + 1));
    } else {
        return qAbs(static_cast<qint32>(ceil(width_ * cos(angle) - height_ * sin(angle)) + 1));
    }
}

qint32 KisBrush::maskHeight(double scale, double angle) const
{
    angle += d->angle;

    // Make sure the angle stay in [0;2*M_PI]
    if(angle < 0) angle += 2 * M_PI;
    if(angle > 2 * M_PI) angle -= 2 * M_PI;
    scale *= d->scale;
    double height_ = height() * scale;
    if(angle == 0.0) return ceil(height_ + 1);

    double width_ = width() * scale;

    // Add one for sub-pixel shift
    if (angle >= 0.0 && angle < M_PI_2) {
        return qAbs(static_cast<qint32>(ceil(width_ * sin(angle) + height_ * cos(angle)) + 1));
    } else if (angle >= M_PI_2 && angle < M_PI) {
        return qAbs(static_cast<qint32>(ceil(width_ * sin(angle) - height_ * cos(angle)) + 1));
    } else if (angle >= M_PI && angle < (M_PI + M_PI_2)) {
        return qAbs(static_cast<qint32>(ceil(-width_ * sin(angle) - height_ * cos(angle)) + 1));
    } else {
        return qAbs(static_cast<qint32>(ceil(-width_ * sin(angle) + height_ * cos(angle)) + 1));
    }
}

double KisBrush::maskAngle(double angle) const
{
    angle += d->angle;

    // Make sure the angle stay in [0;2*M_PI]
    if(angle < 0)      { angle += 2*M_PI; }
    if(angle > 2*M_PI) { angle -= 2*M_PI; }

    return angle;
}

quint32 KisBrush::brushIndex(const KisPaintInformation& info) const
{
    Q_UNUSED(info);
    return 0;
}

double KisBrush::xSpacing(double scale) const
{
    return width() * scale * d->spacing * d->scale;
}

double KisBrush::ySpacing(double scale) const
{
    return height() * scale * d->spacing * d->scale;
}

void KisBrush::setSpacing(double s)
{
    if (s < 0.02) s = 0.02;
    d->spacing = s;
}

double KisBrush::spacing() const
{
    return d->spacing;
}
void KisBrush::mask(KisFixedPaintDeviceSP dst, double scaleX, double scaleY, double angle, const KisPaintInformation& info , double subPixelX, double subPixelY, qreal softnessFactor) const
{
    generateMaskAndApplyMaskOrCreateDab(dst, 0, scaleX, scaleY, angle, info, subPixelX, subPixelY, softnessFactor);
}

void KisBrush::mask(KisFixedPaintDeviceSP dst, const KoColor& color, double scaleX, double scaleY, double angle, const KisPaintInformation& info, double subPixelX, double subPixelY, qreal softnessFactor) const
{
    PlainColoringInformation pci(color.data());
    generateMaskAndApplyMaskOrCreateDab(dst, &pci, scaleX, scaleY, angle, info, subPixelX, subPixelY, softnessFactor);
}

void KisBrush::mask(KisFixedPaintDeviceSP dst, const KisPaintDeviceSP src, double scaleX, double scaleY, double angle, const KisPaintInformation& info, double subPixelX, double subPixelY, qreal softnessFactor) const
{
    PaintDeviceColoringInformation pdci(src, maskWidth(scaleX, angle));
    generateMaskAndApplyMaskOrCreateDab(dst, &pdci, scaleX, scaleY, angle, info, subPixelX, subPixelY, softnessFactor);
}


void KisBrush::generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst,
                                                   ColoringInformation* coloringInformation,
                                                   double scaleX, double scaleY, double angle,
                                                   const KisPaintInformation& info_,
                                                   double subPixelX, double subPixelY, qreal softnessFactor) const
{
    Q_ASSERT(valid());
    Q_UNUSED(info_);
    Q_UNUSED(softnessFactor);

    angle += d->angle;

    // Make sure the angle stay in [0;2*M_PI]
    if(angle < 0) angle += 2 * M_PI;
    if(angle > 2 * M_PI) angle -= 2 * M_PI;
    scaleX *= d->scale;
    scaleY *= d->scale;

    const KoColorSpace* cs = dst->colorSpace();
    quint32 pixelSize = cs->pixelSize();

    double scale = 0.5 * (scaleX + scaleY);

    KisQImagemaskSP outputMask = createMask(scale, subPixelX, subPixelY);

    if (angle != 0) {
        outputMask->rotation(angle);
    }

    qint32 maskWidth = outputMask->width();
    qint32 maskHeight = outputMask->height();

    if (coloringInformation || dst->data() == 0 || dst->bounds().isEmpty()) {
        // Lazy initialization
        dst->setRect(QRect(0, 0, maskWidth, maskHeight));
        dst->initialize();
    }

    {
        QSize dabSize = dst->bounds().size();
        if (dabSize.width() != maskWidth || dabSize.height() != maskHeight) {
            qWarning() << "WARNING: KisBrush::generateMaskAndApplyMaskOrCreateDab";
            qWarning() << "         the sizes of the mask and the supplied dab are not"
                       << "equal. We shall workaround it now, but please report a bug.";
            qWarning() << "        " << ppVar(maskWidth) << ppVar(maskHeight);
            qWarning() << "        " << ppVar(dabSize);

            dst->setRect(QRect(0, 0, maskWidth, maskHeight));
            dst->initialize();
        }
    }

    Q_ASSERT(dst->bounds().size().width() >= maskWidth && dst->bounds().size().height() >= maskHeight);

    quint8* color = 0;

    if (coloringInformation) {
        if (dynamic_cast<PlainColoringInformation*>(coloringInformation)) {
            color = const_cast<quint8*>(coloringInformation->color());
        }
    }

    quint8* dabPointer = dst->data();
    quint8* rowPointer = dabPointer;
    int rowWidth = dst->bounds().width();

    for (int y = 0; y < maskHeight; y++) {
        quint8* maskPointer = outputMask->scanline(y);
        if (coloringInformation) {
            for (int x = 0; x < maskWidth; x++) {
                if (color) {
                    memcpy(dabPointer, color, pixelSize);
                } else {
                    memcpy(dabPointer, coloringInformation->color(), pixelSize);
                    coloringInformation->nextColumn();
                }
                dabPointer += pixelSize;
            }
        }
        cs->applyAlphaU8Mask(rowPointer, maskPointer, maskWidth);
        rowPointer += rowWidth * pixelSize;
        dabPointer = rowPointer;

        if (!color && coloringInformation) {
            coloringInformation->nextRow();
        }
    }
}

KisFixedPaintDeviceSP KisBrush::paintDevice(const KoColorSpace * colorSpace,
                                            double scale, double angle,
                                            const KisPaintInformation& info,
                                            double subPixelX, double subPixelY) const
{
    Q_ASSERT(valid());
    Q_UNUSED(colorSpace);
    Q_UNUSED(info);
    angle += d->angle;

    // Make sure the angle stay in [0;2*M_PI]
    if(angle < 0) angle += 2 * M_PI;
    if(angle > 2 * M_PI) angle -= 2 * M_PI;
    scale *= d->scale;
    if (d->scaledBrushes.isEmpty()) {
        createScaledBrushes();
    }

    const KisScaledBrush *aboveBrush = 0;
    const KisScaledBrush *belowBrush = 0;

    findScaledBrushes(scale, &aboveBrush, &belowBrush);
    Q_ASSERT(aboveBrush != 0);

    QImage outputImage;

    if (belowBrush != 0) {
        // We're in between two brushes. Interpolate between them.

        QImage scaledAboveImage = scaleImage(aboveBrush, scale, subPixelX, subPixelY);

        QImage scaledBelowImage = scaleImage(belowBrush, scale, subPixelX, subPixelY);

        double t = (scale - belowBrush->scale()) / (aboveBrush->scale() - belowBrush->scale());

        outputImage = interpolate(scaledBelowImage, scaledAboveImage, t);

    } else {
        if (Eigen::ei_isApprox(scale, aboveBrush->scale())) {
            // Exact match.
            outputImage = scaleImage(aboveBrush, scale, subPixelX, subPixelY);

        } else {
            // We are smaller than the smallest brush, which is always 1x1.
            double s = scale / aboveBrush->scale();
            outputImage = scaleSinglePixelImage(s, aboveBrush->image().pixel(0, 0), subPixelX, subPixelY);

        }
    }

    if (angle != 0.0)
    {
        outputImage = outputImage.transformed(QTransform().rotate(-angle * 180 / M_PI));
    }

    int outputWidth = outputImage.width();
    int outputHeight = outputImage.height();

    KisFixedPaintDeviceSP dab = new KisFixedPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    Q_CHECK_PTR(dab);
    dab->setRect(outputImage.rect());
    dab->initialize();
    quint8* dabPointer = dab->data();
    quint32 pixelSize = dab->pixelSize();

    for (int y = 0; y < outputHeight; y++) {
        const QRgb *scanline = reinterpret_cast<const QRgb *>(outputImage.scanLine(y));
        for (int x = 0; x < outputWidth; x++) {
            QRgb pixel = scanline[x];

            int red = qRed(pixel);
            int green = qGreen(pixel);
            int blue = qBlue(pixel);
            int alpha = qAlpha(pixel);

            // Scaled images are in pre-multiplied alpha form so
            // divide by alpha.
            // XXX: Is alpha != 0 ever true?
            // channel order is BGRA
            if (alpha != 0) {
                dabPointer[2] = (red * 255) / alpha;
                dabPointer[1] = (green * 255) / alpha;
                dabPointer[0] = (blue * 255) / alpha;
                dabPointer[3] = alpha;
            } else {
                dabPointer[2] = red;
                dabPointer[1] = green;
                dabPointer[0] = blue;
                dabPointer[3] = 0;
            }

            dabPointer += pixelSize;

        }
    }
    return dab;
}

void KisBrush::clearScaledBrushes()
{
    d->scaledBrushes.clear();
}

void KisBrush::createScaledBrushes() const
{
    if (!d->scaledBrushes.isEmpty()) {
        const_cast<KisBrush*>(this)->clearScaledBrushes();
    }

    if (image().isNull()) {
        return;
    }

    // Construct a series of brushes where each one's dimensions are
    // half the size of the previous one.
    // IMORTANT: and make sure that a brush with a size > MAXIMUM_MIPMAP_SIZE
    // will not get scaled up anymore or the memory consumption gets too high
    // also don't scale the brush up more then MAXIMUM_MIPMAP_SCALE times
    int scale  = qBound(1, MAXIMUM_MIPMAP_SIZE*2 / qMax(image().width(),image().height()), MAXIMUM_MIPMAP_SCALE);
    int width  = ceil((double)(image().width()  * scale));
    int height = ceil((double)(image().height() * scale));

    QImage scaledImage;
    while (true) {

        if (width >= image().width() && height >= image().height()) {
            scaledImage = scaleImage(image(), width, height);
        } else {
            // Scale down the previous image once we're below 1:1.
            scaledImage = scaleImage(scaledImage, width, height);
        }

        KisQImagemaskSP scaledMask = KisQImagemaskSP(new KisQImagemask(scaledImage, hasColor()));
        Q_CHECK_PTR(scaledMask);

        double xScale = static_cast<double>(width) / image().width();
        double yScale = static_cast<double>(height) / image().height();
        double scale = xScale;

        d->scaledBrushes.append(KisScaledBrush(scaledMask, hasColor() ? scaledImage : QImage(), scale, xScale, yScale));

        if (width == 1 && height == 1) {
            break;
        }

        // Round up so that we never have to scale an image by less than 1/2.
        width = (width + 1) / 2;
        height = (height + 1) / 2;

    }
}

KisQImagemaskSP KisBrush::createMask(double scale, double subPixelX, double subPixelY) const
{
    if (d->scaledBrushes.isEmpty()) {
        createScaledBrushes();
    }

    const KisScaledBrush *aboveBrush = 0;
    const KisScaledBrush *belowBrush = 0;

    findScaledBrushes(scale, &aboveBrush,  &belowBrush);
    Q_ASSERT(aboveBrush != 0);

    // get the right mask
    KisQImagemaskSP outputMask = KisQImagemaskSP(0);

    if (belowBrush != 0) {
        double t = (scale - belowBrush->scale()) / (aboveBrush->scale() - belowBrush->scale());

        outputMask = scaleMask( (t >= 0.5) ? aboveBrush : belowBrush, scale, subPixelX, subPixelY);
    } else {
        if (Eigen::ei_isApprox(scale, aboveBrush->scale())) {
            // Exact match.
            outputMask = scaleMask(aboveBrush, scale, subPixelX, subPixelY);
        } else {
            // We are smaller than the smallest mask, which is always 1x1.
            double s = scale / aboveBrush->scale();
            outputMask = scaleSinglePixelMask(s, aboveBrush->mask()->alphaAt(0, 0), subPixelX, subPixelY);
        }
    }

    return outputMask;
}

KisQImagemaskSP KisBrush::scaleMask(const KisScaledBrush *srcBrush, double scale, double subPixelX, double subPixelY) const
{
    // Add one pixel for sub-pixel shifting
    int dstWidth = static_cast<int>(ceil(scale * width())) + 1;
    int dstHeight = static_cast<int>(ceil(scale * height())) + 1;

    KisQImagemaskSP dstMask = KisQImagemaskSP(new KisQImagemask(dstWidth, dstHeight, false));
    Q_CHECK_PTR(dstMask);

    KisQImagemaskSP srcMask = srcBrush->mask();

    // Compute scales to map the scaled brush onto the required scale.
    double xScale = srcBrush->xScale() / scale;
    double yScale = srcBrush->yScale() / scale;

    int srcWidth = srcMask->width();
    int srcHeight = srcMask->height();

    for (int dstY = 0; dstY < dstHeight; dstY++) {
        for (int dstX = 0; dstX < dstWidth; dstX++) {

            double srcX = (dstX - subPixelX + 0.5) * xScale;
            double srcY = (dstY - subPixelY + 0.5) * yScale;

            srcX -= 0.5;
            srcY -= 0.5;

            int leftX = static_cast<int>(srcX);

            if (srcX < 0) {
                leftX--;
            }

            double xInterp = srcX - leftX;

            int topY = static_cast<int>(srcY);

            if (srcY < 0) {
                topY--;
            }

            double yInterp = srcY - topY;

            quint8 topLeft = (leftX >= 0 && leftX < srcWidth && topY >= 0 && topY < srcHeight) ? srcMask->alphaAt(leftX, topY) : OPACITY_TRANSPARENT_U8;
            quint8 topRight = (leftX + 1 >= 0 && leftX + 1 < srcWidth && topY >= 0 && topY < srcHeight) ? srcMask->alphaAt(leftX + 1, topY) : OPACITY_TRANSPARENT_U8;
            quint8 bottomLeft = (leftX >= 0 && leftX < srcWidth && topY + 1 >= 0 && topY + 1 < srcHeight) ? srcMask->alphaAt(leftX, topY + 1) : OPACITY_TRANSPARENT_U8;
            quint8 bottomRight = (leftX + 1 >= 0 && leftX + 1 < srcWidth && topY + 1 >= 0 && topY + 1 < srcHeight) ? srcMask->alphaAt(leftX + 1, topY + 1) : OPACITY_TRANSPARENT_U8;

            double a = 1 - xInterp;
            double b = 1 - yInterp;

            // Bi-linear interpolation
            int d = static_cast<int>(a * b * topLeft
                                     + a * (1 - b) * bottomLeft
                                     + (1 - a) * b * topRight
                                     + (1 - a) * (1 - b) * bottomRight + 0.5);

            if (d < OPACITY_TRANSPARENT_U8) {
                d = OPACITY_TRANSPARENT_U8;
            } else if (d > OPACITY_OPAQUE_U8) {
                d = OPACITY_OPAQUE_U8;
            }

            dstMask->setAlphaAt(dstX, dstY, static_cast<quint8>(d));
        }
    }

    return dstMask;
}

QImage KisBrush::scaleImage(const KisScaledBrush *srcBrush, double scale, double subPixelX, double subPixelY) const
{
    // Add one pixel for sub-pixel shifting
    int dstWidth = static_cast<int>(ceil(scale * width())) + 1;
    int dstHeight = static_cast<int>(ceil(scale * height())) + 1;

    QImage dstImage(dstWidth, dstHeight, QImage::Format_ARGB32);

    QImage srcImage = srcBrush->image();
    if (srcImage.format() != QImage::Format_ARGB32)
    {
        srcImage = srcImage.convertToFormat(QImage::Format_ARGB32);
    }

    // Compute scales to map the scaled brush onto the required scale.
    double xScale = srcBrush->xScale() / scale;
    double yScale = srcBrush->yScale() / scale;

    int srcWidth = srcImage.width();
    int srcHeight = srcImage.height();

    for (int dstY = 0; dstY < dstHeight; dstY++) {

        double srcY = (dstY - subPixelY + 0.5) * yScale;
        srcY -= 0.5;
        int topY = static_cast<int>(srcY);
        if (srcY < 0) {
            topY--;
        }

        QRgb *dstPixel = reinterpret_cast<QRgb *>(dstImage.scanLine(dstY));
        QRgb *srcPixel = reinterpret_cast<QRgb *>(srcImage.scanLine(topY));
        QRgb *srcPixelPlusOne = reinterpret_cast<QRgb *>(srcImage.scanLine(topY + 1));

        for (int dstX = 0; dstX < dstWidth; dstX++) {

            double srcX = (dstX - subPixelX + 0.5) * xScale;


            srcX -= 0.5;


            int leftX = static_cast<int>(srcX);

            if (srcX < 0) {
                leftX--;
            }

            double xInterp = srcX - leftX;

            double yInterp = srcY - topY;

            QRgb topLeft = (leftX >= 0 && leftX < srcWidth && topY >= 0 && topY < srcHeight) ? srcPixel[leftX] : qRgba(0, 0, 0, 0);
            QRgb bottomLeft = (leftX >= 0 && leftX < srcWidth && topY + 1 >= 0 && topY + 1 < srcHeight) ? srcPixelPlusOne[leftX] : qRgba(0, 0, 0, 0);
            QRgb topRight = (leftX + 1 >= 0 && leftX + 1 < srcWidth && topY >= 0 && topY < srcHeight) ? srcPixel[leftX + 1] : qRgba(0, 0, 0, 0);
            QRgb bottomRight = (leftX + 1 >= 0 && leftX + 1 < srcWidth && topY + 1 >= 0 && topY + 1 < srcHeight) ? srcPixelPlusOne[leftX + 1] : qRgba(0, 0, 0, 0);

            double a = 1 - xInterp;
            double b = 1 - yInterp;

            // Bi-linear interpolation. Image is pre-multiplied by alpha.
            int red = static_cast<int>(a * b * qRed(topLeft)
                                       + a * (1 - b) * qRed(bottomLeft)
                                       + (1 - a) * b * qRed(topRight)
                                       + (1 - a) * (1 - b) * qRed(bottomRight) + 0.5);
            int green = static_cast<int>(a * b * qGreen(topLeft)
                                         + a * (1 - b) * qGreen(bottomLeft)
                                         + (1 - a) * b * qGreen(topRight)
                                         + (1 - a) * (1 - b) * qGreen(bottomRight) + 0.5);
            int blue = static_cast<int>(a * b * qBlue(topLeft)
                                        + a * (1 - b) * qBlue(bottomLeft)
                                        + (1 - a) * b * qBlue(topRight)
                                        + (1 - a) * (1 - b) * qBlue(bottomRight) + 0.5);
            int alpha = static_cast<int>(a * b * qAlpha(topLeft)
                                         + a * (1 - b) * qAlpha(bottomLeft)
                                         + (1 - a) * b * qAlpha(topRight)
                                         + (1 - a) * (1 - b) * qAlpha(bottomRight) + 0.5);

            if (red < 0) {
                red = 0;
            } else if (red > 255) {
                red = 255;
            }

            if (green < 0) {
                green = 0;
            } else if (green > 255) {
                green = 255;
            }

            if (blue < 0) {
                blue = 0;
            } else if (blue > 255) {
                blue = 255;
            }

            if (alpha < 0) {
                alpha = 0;
            } else if (alpha > 255) {
                alpha = 255;
            }

            dstPixel[dstX] = qRgba(red, green, blue, alpha);
        }
    }

    return dstImage;
}

QImage KisBrush::scaleImage(const QImage& _srcImage, int width, int height)
{
    QImage scaledImage;
    QImage srcImage = _srcImage; // detaches!
    //QString filename;
    if (srcImage.format() != QImage::Format_ARGB32)
    {
        srcImage = srcImage.convertToFormat(QImage::Format_ARGB32);
    }

    int srcWidth = srcImage.width();
    int srcHeight = srcImage.height();

    double xScale = static_cast<double>(srcWidth) / width;
    double yScale = static_cast<double>(srcHeight) / height;

    if (xScale > 2 || yScale > 2 || xScale < 1 || yScale < 1) {
        // smoothScale gives better results when scaling an image up
        // or scaling it to less than half size.
        scaledImage = srcImage.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        //filename = QString("smoothScale_%1x%2.png").arg(width).arg(height);
    } else {
        scaledImage = QImage(width, height, srcImage.format());

        for (int dstY = 0; dstY < height; dstY++) {

            double srcY = (dstY + 0.5) * yScale;
            srcY -= 0.5;
            int topY = static_cast<int>(srcY);

            if (srcY < 0) {
                topY--;
            }

            QRgb *dstPixel = reinterpret_cast<QRgb *>(scaledImage.scanLine(dstY));
            QRgb *srcPixel = reinterpret_cast<QRgb *>(srcImage.scanLine(topY));
            QRgb *srcPixelPlusOne = reinterpret_cast<QRgb *>(srcImage.scanLine(topY + 1));

            for (int dstX = 0; dstX < width; dstX++) {

                double srcX = (dstX + 0.5) * xScale;

                srcX -= 0.5;

                int leftX = static_cast<int>(srcX);

                if (srcX < 0) {
                    leftX--;
                }

                double xInterp = srcX - leftX;

                double yInterp = srcY - topY;

                QRgb topLeft = (leftX >= 0 && leftX < srcWidth && topY >= 0 && topY < srcHeight) ? srcPixel[leftX] : qRgba(0, 0, 0, 0);
                QRgb bottomLeft = (leftX >= 0 && leftX < srcWidth && topY + 1 >= 0 && topY + 1 < srcHeight) ? srcPixelPlusOne[leftX] : qRgba(0, 0, 0, 0);
                QRgb topRight = (leftX + 1 >= 0 && leftX + 1 < srcWidth && topY >= 0 && topY < srcHeight) ? srcPixel[leftX] : qRgba(0, 0, 0, 0);
                QRgb bottomRight = (leftX + 1 >= 0 && leftX + 1 < srcWidth && topY + 1 >= 0 && topY + 1 < srcHeight) ? srcPixelPlusOne[leftX + 1] : qRgba(0, 0, 0, 0);

                double a = 1 - xInterp;
                double b = 1 - yInterp;

                int red;
                int green;
                int blue;
                int alpha;

                if (srcImage.hasAlphaChannel()) {
                    red = static_cast<int>(a * b * qRed(topLeft)         * qAlpha(topLeft)
                                           + a * (1 - b) * qRed(bottomLeft)             * qAlpha(bottomLeft)
                                           + (1 - a) * b * qRed(topRight)               * qAlpha(topRight)
                                           + (1 - a) * (1 - b) * qRed(bottomRight)      * qAlpha(bottomRight) + 0.5);
                    green = static_cast<int>(a * b * qGreen(topLeft)     * qAlpha(topLeft)
                                             + a * (1 - b) * qGreen(bottomLeft)           * qAlpha(bottomLeft)
                                             + (1 - a) * b * qGreen(topRight)             * qAlpha(topRight)
                                             + (1 - a) * (1 - b) * qGreen(bottomRight)    * qAlpha(bottomRight) + 0.5);
                    blue = static_cast<int>(a * b * qBlue(topLeft)       * qAlpha(topLeft)
                                            + a * (1 - b) * qBlue(bottomLeft)            * qAlpha(bottomLeft)
                                            + (1 - a) * b * qBlue(topRight)              * qAlpha(topRight)
                                            + (1 - a) * (1 - b) * qBlue(bottomRight)     * qAlpha(bottomRight) + 0.5);
                    alpha = static_cast<int>(a * b * qAlpha(topLeft)
                                             + a * (1 - b) * qAlpha(bottomLeft)
                                             + (1 - a) * b * qAlpha(topRight)
                                             + (1 - a) * (1 - b) * qAlpha(bottomRight) + 0.5);

                    if (alpha != 0) {
                        red /= alpha;
                        green /= alpha;
                        blue /= alpha;
                    }
                } else {
                    red = static_cast<int>(a * b * qRed(topLeft)
                                           + a * (1 - b) * qRed(bottomLeft)
                                           + (1 - a) * b * qRed(topRight)
                                           + (1 - a) * (1 - b) * qRed(bottomRight) + 0.5);
                    green = static_cast<int>(a * b * qGreen(topLeft)
                                             + a * (1 - b) * qGreen(bottomLeft)
                                             + (1 - a) * b * qGreen(topRight)
                                             + (1 - a) * (1 - b) * qGreen(bottomRight) + 0.5);
                    blue = static_cast<int>(a * b * qBlue(topLeft)
                                            + a * (1 - b) * qBlue(bottomLeft)
                                            + (1 - a) * b * qBlue(topRight)
                                            + (1 - a) * (1 - b) * qBlue(bottomRight) + 0.5);
                    alpha = 255;
                }

                if (red < 0) {
                    red = 0;
                } else if (red > 255) {
                    red = 255;
                }

                if (green < 0) {
                    green = 0;
                } else if (green > 255) {
                    green = 255;
                }

                if (blue < 0) {
                    blue = 0;
                } else if (blue > 255) {
                    blue = 255;
                }

                if (alpha < 0) {
                    alpha = 0;
                } else if (alpha > 255) {
                    alpha = 255;
                }

                dstPixel[dstX] = qRgba(red, green, blue, alpha);
            }
        }

        //filename = QString("bilinear_%1x%2.png").arg(width).arg(height);
    }

    //scaledImage.save(filename, "PNG");

    return scaledImage;
}

void KisBrush::findScaledBrushes(double scale, const KisScaledBrush **aboveBrush, const KisScaledBrush **belowBrush) const
{
    int current = 0;

    while (true) {
        *aboveBrush = &(d->scaledBrushes[current]);

        if (Eigen::ei_isApprox(scale, (*aboveBrush)->scale())) {
            // Scale matches exactly
            break;
        }

        if (current == d->scaledBrushes.count() - 1) {
            // This is the last one
            break;
        }

        if (scale > d->scaledBrushes[current + 1].scale()) {
            // We fit in between the two.
            *belowBrush = &(d->scaledBrushes[current + 1]);
            break;
        }

        current++;
    }
}

KisQImagemaskSP KisBrush::scaleSinglePixelMask(double scale, quint8 maskValue, double subPixelX, double subPixelY)
{
    int srcWidth = 1;
    int srcHeight = 1;
    int dstWidth = 2;
    int dstHeight = 2;
    KisQImagemaskSP outputMask = KisQImagemaskSP(new KisQImagemask(dstWidth, dstHeight));
    Q_CHECK_PTR(outputMask);

    double a = subPixelX;
    double b = subPixelY;

    for (int y = 0; y < dstHeight; y++) {

        for (int x = 0; x < dstWidth; x++) {

            quint8 topLeft = (x > 0 && y > 0) ? maskValue : OPACITY_TRANSPARENT_U8;
            quint8 bottomLeft = (x > 0 && y < srcHeight) ? maskValue : OPACITY_TRANSPARENT_U8;
            quint8 topRight = (x < srcWidth && y > 0) ? maskValue : OPACITY_TRANSPARENT_U8;
            quint8 bottomRight = (x < srcWidth && y < srcHeight) ? maskValue : OPACITY_TRANSPARENT_U8;

            // Bi-linear interpolation
            int d = static_cast<int>(a * b * topLeft
                                     + a * (1 - b) * bottomLeft
                                     + (1 - a) * b * topRight
                                     + (1 - a) * (1 - b) * bottomRight + 0.5);

            // Multiply by the square of the scale because a 0.5x0.5 pixel
            // has 0.25 the value of the 1x1.
            d = static_cast<int>(d * scale * scale + 0.5);

            if (d < OPACITY_TRANSPARENT_U8) {
                d = OPACITY_TRANSPARENT_U8;
            } else if (d > OPACITY_OPAQUE_U8) {
                d = OPACITY_OPAQUE_U8;
            }

            outputMask->setAlphaAt(x, y, static_cast<quint8>(d));
        }
    }

    return outputMask;
}

QImage KisBrush::scaleSinglePixelImage(double scale, QRgb pixel, double subPixelX, double subPixelY)
{
    int srcWidth = 1;
    int srcHeight = 1;
    int dstWidth = 2;
    int dstHeight = 2;

    QImage outputImage(dstWidth, dstHeight, QImage::Format_ARGB32);

    double a = subPixelX;
    double b = subPixelY;

    for (int y = 0; y < dstHeight; y++) {

        QRgb *dstPixel = reinterpret_cast<QRgb *>(outputImage.scanLine(y));

        for (int x = 0; x < dstWidth; x++) {

            QRgb topLeft = (x > 0 && y > 0) ? pixel : qRgba(0, 0, 0, 0);
            QRgb bottomLeft = (x > 0 && y < srcHeight) ? pixel : qRgba(0, 0, 0, 0);
            QRgb topRight = (x < srcWidth && y > 0) ? pixel : qRgba(0, 0, 0, 0);
            QRgb bottomRight = (x < srcWidth && y < srcHeight) ? pixel : qRgba(0, 0, 0, 0);

            // Bi-linear interpolation. Images are in pre-multiplied form.
            int red = static_cast<int>(a * b * qRed(topLeft)
                                       + a * (1 - b) * qRed(bottomLeft)
                                       + (1 - a) * b * qRed(topRight)
                                       + (1 - a) * (1 - b) * qRed(bottomRight) + 0.5);
            int green = static_cast<int>(a * b * qGreen(topLeft)
                                         + a * (1 - b) * qGreen(bottomLeft)
                                         + (1 - a) * b * qGreen(topRight)
                                         + (1 - a) * (1 - b) * qGreen(bottomRight) + 0.5);
            int blue = static_cast<int>(a * b * qBlue(topLeft)
                                        + a * (1 - b) * qBlue(bottomLeft)
                                        + (1 - a) * b * qBlue(topRight)
                                        + (1 - a) * (1 - b) * qBlue(bottomRight) + 0.5);
            int alpha = static_cast<int>(a * b * qAlpha(topLeft)
                                         + a * (1 - b) * qAlpha(bottomLeft)
                                         + (1 - a) * b * qAlpha(topRight)
                                         + (1 - a) * (1 - b) * qAlpha(bottomRight) + 0.5);

            // Multiply by the square of the scale because a 0.5x0.5 pixel
            // has 0.25 the value of the 1x1.
            alpha = static_cast<int>(alpha * scale * scale + 0.5);

            // Apply to the color channels too since we are
            // storing pre-multiplied by alpha.
            red = static_cast<int>(red * scale * scale + 0.5);
            green = static_cast<int>(green * scale * scale + 0.5);
            blue = static_cast<int>(blue * scale * scale + 0.5);

            if (red < 0) {
                red = 0;
            } else if (red > 255) {
                red = 255;
            }

            if (green < 0) {
                green = 0;
            } else if (green > 255) {
                green = 255;
            }

            if (blue < 0) {
                blue = 0;
            } else if (blue > 255) {
                blue = 255;
            }

            if (alpha < 0) {
                alpha = 0;
            } else if (alpha > 255) {
                alpha = 255;
            }

            dstPixel[x] = qRgba(red, green, blue, alpha);
        }
    }

    return outputImage;
}

QImage KisBrush::interpolate(const QImage& image1, const QImage& image2, double t)
{
    Q_ASSERT((image1.width() == image2.width()) && (image1.height() == image2.height()));

    int width = image1.width();
    int height = image1.height();

    QImage outputImage(width, height, QImage::Format_ARGB32);

    for (int y = 0; y < height; y++) {
        QRgb *dstPixel = reinterpret_cast<QRgb *>(outputImage.scanLine(y));
        for (int x = 0; x < width; x++) {
            QRgb image1pixel = image1.pixel(x, y);
            QRgb image2pixel = image2.pixel(x, y);

            // Images are in pre-multiplied alpha format.
            int red = static_cast<int>((1 - t) * qRed(image1pixel) + t * qRed(image2pixel) + 0.5);
            int green = static_cast<int>((1 - t) * qGreen(image1pixel) + t * qGreen(image2pixel) + 0.5);
            int blue = static_cast<int>((1 - t) * qBlue(image1pixel) + t * qBlue(image2pixel) + 0.5);
            int alpha = static_cast<int>((1 - t) * qAlpha(image1pixel) + t * qAlpha(image2pixel) + 0.5);

            if (red < 0) {
                red = 0;
            } else if (red > 255) {
                red = 255;
            }

            if (green < 0) {
                green = 0;
            } else if (green > 255) {
                green = 255;
            }

            if (blue < 0) {
                blue = 0;
            } else if (blue > 255) {
                blue = 255;
            }

            if (alpha < 0) {
                alpha = 0;
            } else if (alpha > 255) {
                alpha = 255;
            }

            dstPixel[x] = qRgba(red, green, blue, alpha);
        }
    }

    return outputImage;
}

void KisBrush::resetBoundary()
{
    delete d->boundary;
    d->boundary = 0;
}

void KisBrush::generateBoundary() const
{
    KisFixedPaintDeviceSP dev;

    if (brushType() == IMAGE || brushType() == PIPE_IMAGE) {
        dev = paintDevice(KoColorSpaceRegistry::instance()->rgb8(), 1.0/scale(), -angle(), KisPaintInformation());
    } else {
        const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
        dev = new KisFixedPaintDevice(cs);
        mask(dev, KoColor(Qt::black, cs) , 1.0/scale(), 1.0/scale(), -angle(), KisPaintInformation());
    }

    d->boundary = new KisBoundary(dev);
    d->boundary->generateBoundary();
}

const KisBoundary* KisBrush::boundary() const
{
    if (!d->boundary)
        generateBoundary();
    return d->boundary;
}

void KisBrush::setScale(qreal _scale)
{
  d->scale = _scale;
}

qreal KisBrush::scale() const
{
  return d->scale;
}

void KisBrush::setAngle(qreal _rotation)
{
  d->angle = _rotation;
}

qreal KisBrush::angle() const
{
  return d->angle;
}

QPainterPath KisBrush::outline() const
{
    return boundary()->path();
}
