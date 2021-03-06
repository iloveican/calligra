/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
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

#include <cmath>

#include "kis_coordinates_converter.h"

#include <QTransform>
#include <KoViewConverter.h>

#include <kis_config.h>
#include <kis_image.h>


struct KisCoordinatesConverter::Private {
    Private():
        isXAxisMirrored(false), isYAxisMirrored(false), rotationAngle(0.0) { }
    
    KisImageWSP image;
    
    bool isXAxisMirrored;
    bool isYAxisMirrored;
    qreal rotationAngle;
    QSizeF canvasWidgetSize;
    QPointF documentOffset;
    QPointF documentOrigin;
    
    QTransform flakeToWidget;
    QTransform imageToDocument;
    QTransform documentToFlake;
    QTransform widgetToViewport;
};

void KisCoordinatesConverter::recalculateTransformations() const
{
    if(!m_d->image) return;

    m_d->imageToDocument = QTransform::fromScale(1 / m_d->image->xRes(),
                                                 1 / m_d->image->yRes());

    qreal zoomX, zoomY;
    KoZoomHandler::zoom(&zoomX, &zoomY);
    m_d->documentToFlake = QTransform::fromScale(zoomX, zoomY);

    QRectF irect = imageRectInWidgetPixels();
    QRectF wrect = QRectF(QPoint(0,0), m_d->canvasWidgetSize);
    QRectF rrect = irect & wrect;

    QTransform reversedTransform = flakeToWidgetTransform().inverted();
    QRectF     canvasBounds      = reversedTransform.mapRect(rrect);
    QPointF    offset            = canvasBounds.topLeft();

    m_d->widgetToViewport = reversedTransform * QTransform::fromTranslate(-offset.x(), -offset.y());
}


KisCoordinatesConverter::KisCoordinatesConverter()
    : m_d(new Private) { }

KisCoordinatesConverter::~KisCoordinatesConverter()
{
    delete m_d;
}

void KisCoordinatesConverter::setCanvasWidgetSize(QSize size)
{
    m_d->canvasWidgetSize = size;
    recalculateTransformations();
}

void KisCoordinatesConverter::setImage(KisImageWSP image)
{
    m_d->image = image;
    recalculateTransformations();
}

void KisCoordinatesConverter::setDocumentOrigin(const QPoint& origin)
{
    QPointF diff = origin - m_d->documentOrigin;
    
    m_d->documentOrigin = origin;
    m_d->flakeToWidget *= QTransform::fromTranslate(diff.x(), diff.y());
    recalculateTransformations();
}

void KisCoordinatesConverter::setDocumentOffset(const QPoint& offset)
{
    QPointF diff = m_d->documentOffset - offset;
    
    m_d->documentOffset = offset;
    m_d->flakeToWidget *= QTransform::fromTranslate(diff.x(), diff.y());
    recalculateTransformations();
}

QPoint KisCoordinatesConverter::documentOrigin() const
{
    return QPoint(int(m_d->documentOrigin.x()), int(m_d->documentOrigin.y()));
}

QPoint KisCoordinatesConverter::documentOffset() const
{
    return QPoint(int(m_d->documentOffset.x()), int(m_d->documentOffset.y()));
}

qreal KisCoordinatesConverter::rotationAngle() const
{
    return m_d->rotationAngle;
}

void KisCoordinatesConverter::setZoom(qreal zoom)
{
    KoZoomHandler::setZoom(zoom);
    recalculateTransformations();
}

void KisCoordinatesConverter::rotate(QPointF center, qreal angle)
{
    QTransform rot;
    rot.rotate(angle);
    
    m_d->flakeToWidget *= QTransform::fromTranslate(-center.x(),-center.y());
    m_d->flakeToWidget *= rot;
    m_d->flakeToWidget *= QTransform::fromTranslate(center.x(), center.y());
    m_d->rotationAngle = std::fmod(m_d->rotationAngle + angle, 360.0);
    recalculateTransformations();
}

void KisCoordinatesConverter::mirror(QPointF center, bool mirrorXAxis, bool mirrorYAxis, bool keepOrientation)
{
    bool       doXMirroring = m_d->isXAxisMirrored ^ mirrorXAxis;
    bool       doYMirroring = m_d->isYAxisMirrored ^ mirrorYAxis;
    qreal      scaleX       = doYMirroring ? -1.0 : 1.0;
    qreal      scaleY       = doXMirroring ? -1.0 : 1.0;
    QTransform mirror       = QTransform::fromScale(scaleX, scaleY);
    
    QTransform rot;
    rot.rotate(m_d->rotationAngle);
    
    m_d->flakeToWidget *= QTransform::fromTranslate(-center.x(),-center.y());
    
    if(keepOrientation)
        m_d->flakeToWidget *= rot.inverted();
    
    m_d->flakeToWidget *= mirror;
    
    if(keepOrientation)
        m_d->flakeToWidget *= rot;
    
    m_d->flakeToWidget *= QTransform::fromTranslate(center.x(),center.y());
    
    if(!keepOrientation && (doXMirroring ^ doYMirroring))
        m_d->rotationAngle = -m_d->rotationAngle;
    
    m_d->isXAxisMirrored = mirrorXAxis;
    m_d->isYAxisMirrored = mirrorYAxis;
    recalculateTransformations();
}

void KisCoordinatesConverter::resetRotation(QPointF center)
{
    QTransform rot;
    rot.rotate(-m_d->rotationAngle);
    
    m_d->flakeToWidget *= QTransform::fromTranslate(-center.x(), -center.y());
    m_d->flakeToWidget *= rot;
    m_d->flakeToWidget *= QTransform::fromTranslate(center.x(), center.y());
    m_d->rotationAngle = 0.0;
    recalculateTransformations();
}

QTransform KisCoordinatesConverter::imageToWidgetTransform() const{
    return m_d->imageToDocument * m_d->documentToFlake * m_d->flakeToWidget;
}

QTransform KisCoordinatesConverter::imageToDocumentTransform() const {
    return m_d->imageToDocument;
}

QTransform KisCoordinatesConverter::flakeToWidgetTransform() const {
    return m_d->flakeToWidget;
}

QTransform KisCoordinatesConverter::documentToWidgetTransform() const
{
    return m_d->documentToFlake * m_d->flakeToWidget;
}

QTransform KisCoordinatesConverter::viewportToWidgetTransform() const {
    return m_d->widgetToViewport.inverted();
}

QTransform KisCoordinatesConverter::imageToViewportTransform() const {
    return m_d->imageToDocument * m_d->documentToFlake * m_d->flakeToWidget * m_d->widgetToViewport;
}

void KisCoordinatesConverter::getQPainterCheckersInfo(QTransform *transform,
                                                      QPointF *brushOrigin,
                                                      QPolygonF *polygon) const
{
    /**
     * Qt has different rounding for QPainter::drawRect/drawImage.
     * The image is rounded mathematically, while rect in aligned
     * to the next integer. That causes transparent line appear on
     * the canvas.
     *
     * See: https://bugreports.qt.nokia.com/browse/QTBUG-22827
     */

    QRectF imageRect = imageRectInViewportPixels();
    imageRect.adjust(0,0,-0.5,-0.5);

    KisConfig cfg;
    if (cfg.scrollCheckers()) {
        *transform = viewportToWidgetTransform();
        *polygon = imageRect;
        *brushOrigin = imageToViewport(QPointF(0,0));
    }
    else {
        *transform = QTransform();
        *polygon = viewportToWidgetTransform().map(imageRect);
        *brushOrigin = QPoint(0,0);
    }
}

void KisCoordinatesConverter::getOpenGLCheckersInfo(QTransform *textureTransform,
                                                    QTransform *modelTransform,
                                                    QRectF *textureRect,
                                                    QRectF *modelRect) const
{
    KisConfig cfg;
    QRectF viewportRect = imageRectInViewportPixels();

    if(cfg.scrollCheckers()) {
        *textureTransform = QTransform();
        *textureRect = QRectF(0, 0, viewportRect.width(),viewportRect.height());
    }
    else {
        *textureTransform = viewportToWidgetTransform();
        *textureRect = viewportRect;
    }

    *modelTransform = viewportToWidgetTransform();
    *modelRect = viewportRect;
}

QPointF KisCoordinatesConverter::imageCenterInWidgetPixel() const
{
    if(!m_d->image)
        return QPointF();
    
    QPolygonF poly = imageToWidget(QPolygon(m_d->image->bounds()));
    return (poly[0] + poly[1] + poly[2] + poly[3]) / 4.0;
}


// these functions return a bounding rect if the canvas is rotated

QRectF KisCoordinatesConverter::imageRectInWidgetPixels() const
{
    if(!m_d->image) return QRectF();
    return imageToWidget(m_d->image->bounds());
}

QRectF KisCoordinatesConverter::imageRectInViewportPixels() const
{
    if(!m_d->image) return QRectF();
    return imageToViewport(m_d->image->bounds());
}

QSizeF KisCoordinatesConverter::imageSizeInFlakePixels() const
{
    if(!m_d->image) return QSizeF();

    qreal scaleX, scaleY;
    imageScale(&scaleX, &scaleY);
    QSize imageSize = m_d->image->size();

    return QSizeF(imageSize.width() * scaleX, imageSize.height() * scaleY);
}

QRectF KisCoordinatesConverter::widgetRectInFlakePixels() const
{
    return widgetToFlake(QRectF(QPoint(0,0), m_d->canvasWidgetSize));
}

QPoint KisCoordinatesConverter::updateOffsetAfterTransform() const
{
    QPointF oldOffset = m_d->documentOrigin + m_d->documentOffset;
    QPointF newOffset = -imageRectInWidgetPixels().topLeft();
    QPointF shift     = newOffset - oldOffset;
    
    m_d->documentOffset = newOffset;
    return shift.toPoint();
}

QPointF KisCoordinatesConverter::flakeCenterPoint() const
{
    QRectF widgetRect = widgetRectInFlakePixels();
    return QPointF(widgetRect.left() + widgetRect.width() / 2,
                   widgetRect.top() + widgetRect.height() / 2);
}

QPointF KisCoordinatesConverter::widgetCenterPoint() const
{
    return QPointF(m_d->canvasWidgetSize.width() / 2.0, m_d->canvasWidgetSize.height() / 2.0);
}

void KisCoordinatesConverter::imageScale(qreal *scaleX, qreal *scaleY) const
{
    // get the x and y zoom level of the canvas
    qreal zoomX, zoomY;
    KoZoomHandler::zoom(&zoomX, &zoomY);

    // Get the KisImage resolution
    qreal resX = m_d->image->xRes();
    qreal resY = m_d->image->yRes();

    // Compute the scale factors
    *scaleX = zoomX / resX;
    *scaleY = zoomY / resY;
}
