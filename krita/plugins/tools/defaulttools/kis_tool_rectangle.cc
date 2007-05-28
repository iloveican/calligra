/*
 *  kis_tool_rectangle.cc - part of Krita
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@k.org>
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

#include <QPainter>

#include <kdebug.h>
#include <klocale.h>

#include "KoPointerEvent.h"
#include "kis_painter.h"
#include "kis_paintop_registry.h"
#include "kis_tool_rectangle.h"
#include "kis_cursor.h"
#include "kis_layer.h"
#include "KoCanvasBase.h"

KisToolRectangle::KisToolRectangle(KoCanvasBase * canvas)
    : KisToolShape(canvas, KisCursor::load("tool_rectangle_cursor.png", 6, 6)),
          m_dragging (false)
{
    setObjectName("tool_rectangle");

    m_painter = 0;
    m_currentImage = 0;
    m_dragStart = QPointF(0, 0);
    m_dragEnd = QPointF(0, 0);
 }

KisToolRectangle::~KisToolRectangle()
{
}

void KisToolRectangle::paint(QPainter& gc, const KoViewConverter &converter)
{
    double sx, sy;
    converter.zoom(&sx, &sy);

    gc.scale( sx/m_currentImage->xRes(), sy/m_currentImage->yRes() );
    if (m_dragging)
        paintRectangle(gc, QRect());
}


void KisToolRectangle::mousePressEvent(KoPointerEvent *e)
{
    if (!m_canvas || !m_currentImage) return;

    if (!m_currentBrush) return;

    if (e->button() == Qt::LeftButton) {
	QPointF pos = convertToPixelCoord(e);
	m_dragging = true;
	m_dragStart = m_dragCenter = m_dragEnd = pos;
	//draw(m_dragStart, m_dragEnd);
    }
}

void KisToolRectangle::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_dragging) {
	QPointF pos = convertToPixelCoord(event);
        // erase old lines on canvas
	// This is not enough, how to do?
	QRectF bound;
        bound.setTopLeft(m_dragStart);
        bound.setBottomRight(m_dragEnd);
	m_canvas->updateCanvas(convertToPt(bound.normalized()));
        //draw(m_dragStart, m_dragEnd);
        // move (alt) or resize rectangle
        if (event->modifiers() & Qt::AltModifier) {
            QPointF trans = pos - m_dragEnd;
            m_dragStart += trans;
            m_dragEnd += trans;
        } else {
            QPointF diag = pos - (event->modifiers() & Qt::ControlModifier
                    ? m_dragCenter : m_dragStart);
            // square?
            if (event->modifiers() & Qt::ShiftModifier) {
                double size = qMax(fabs(diag.x()), fabs(diag.y()));
                double w = diag.x() < 0 ? -size : size;
                double h = diag.y() < 0 ? -size : size;
                diag = QPointF(w, h);
            }

            // resize around center point?
            if (event->modifiers() & Qt::ControlModifier) {
                m_dragStart = m_dragCenter - diag;
                m_dragEnd = m_dragCenter + diag;
            } else {
                m_dragEnd = m_dragStart + diag;
            }
        }
        // draw new lines on canvas
        //draw(m_dragStart, m_dragEnd);
	//QRectF bound;
        bound.setTopLeft(m_dragStart);
        bound.setBottomRight(m_dragEnd);
        /* FIXME Which rectangle to repaint */
        m_canvas->updateCanvas(convertToPt(bound.normalized()));

        m_dragCenter = QPointF((m_dragStart.x() + m_dragEnd.x()) / 2,
                (m_dragStart.y() + m_dragEnd.y()) / 2);
    }
}

void KisToolRectangle::mouseReleaseEvent(KoPointerEvent *event)
{
    QPointF pos = convertToPixelCoord(event);

    if (!m_canvas)
        return;

    if (!m_currentImage)
        return;

    KisPaintDeviceSP device = m_currentLayer->paintDevice();
    if (!device) return;

    if (m_dragging && m_currentBrush && event->button() == Qt::LeftButton) {
        m_dragging = false;

        if (m_dragStart == m_dragEnd)
            return;

        delete m_painter;
        m_painter = new KisPainter( device );
        Q_CHECK_PTR(m_painter);

        m_painter->beginTransaction (i18n ("Rectangle"));
        m_painter->setBounds( m_currentImage->bounds() );
        m_painter->setPaintColor(m_currentFgColor);
        m_painter->setBackgroundColor(m_currentBgColor);
        m_painter->setFillStyle(fillStyle());
        m_painter->setStrokeStyle(strokeStyle());
        m_painter->setBrush(m_currentBrush);
        m_painter->setPattern(m_currentPattern);
        m_painter->setOpacity(m_opacity);
        m_painter->setCompositeOp(m_compositeOp);
        KisPaintOp * op = KisPaintOpRegistry::instance()->paintOp(m_currentPaintOp, m_currentPaintOpSettings, m_painter, m_currentImage);
        m_painter->setPaintOp(op);

        m_painter->paintRect(QRectF(m_dragStart, m_dragEnd), PRESSURE_DEFAULT/*event->pressure()*/, event->xTilt(), event->yTilt());
        QRegion bound = m_painter->dirtyRegion();
        device->setDirty( bound );
        notifyModified();
// Should not be necessary anymore
#if 0
        m_canvas->updateCanvas(convertToPt(bound.normalized()));
#endif

        m_canvas->addCommand(m_painter->endTransaction());

	delete m_painter;
	m_painter = 0;
    }
}

void KisToolRectangle::paintRectangle()
{
    if (m_canvas) {
        QPainter gc(m_canvas->canvasWidget());
        QRect rc;

        paintRectangle(gc, rc);
    }
}

void KisToolRectangle::paintRectangle(QPainter& gc, const QRect&)
{
    if (m_canvas) {
        QPen old = gc.pen();
        QPen pen(Qt::SolidLine);
        QPoint start;
        QPoint end;

        gc.setPen(pen);

	start = QPoint(static_cast<int>(m_dragStart.x()), static_cast<int>(m_dragStart.y()));
	end = QPoint(static_cast<int>(m_dragEnd.x()), static_cast<int>(m_dragEnd.y()));
	gc.drawRect(QRect(start, end));
        gc.setPen(old);
    }
}

#include "kis_tool_rectangle.moc"
