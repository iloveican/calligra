/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "kexiscrollview.h"

#include <qcursor.h>
#include <qobjectlist.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kdebug.h>
#include <kstaticdeleter.h>
#include <klocale.h>

#include <utils/kexirecordnavigator.h>

// @todo warning: not reentrant!
static KStaticDeleter<QPixmap> KexiScrollView_bufferPm_deleter;
QPixmap* KexiScrollView_bufferPm = 0;

KexiScrollView::KexiScrollView(QWidget *parent, bool preview)
 : QScrollView(parent, "kexiscrollview", WStaticContents)
 , m_widget(0)
 , m_helpFont(font())
 , m_preview(preview)
 , m_navPanel(0)
{
	setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
	viewport()->setPaletteBackgroundColor(colorGroup().mid());
	QColor fc = palette().active().foreground(),
		bc = viewport()->paletteBackgroundColor();
	m_helpColor	= QColor((fc.red()+bc.red()*2)/3, (fc.green()+bc.green()*2)/3, 
		(fc.blue()+bc.blue()*2)/3);
	m_helpFont.setPointSize( m_helpFont.pointSize() * 3 );

	setFocusPolicy(WheelFocus);

	//initial resize mode is always manual;
	//will be changed on show(), if needed
	setResizePolicy(Manual);

	viewport()->setMouseTracking(true);
	m_resizing = false;
	m_enableResizing = true;
	m_snapToGrid = false;
	m_gridX = 0;
	m_gridY = 0;
	m_outerAreaVisible = true;

	connect(&m_delayedResize, SIGNAL(timeout()), this, SLOT(refreshContentsSize()));
	m_smodeSet = false;
	if (m_preview) {
		refreshContentsSizeLater(true, true);
//! @todo allow to hide navigator
		updateScrollBars();
		m_navPanel = new KexiRecordNavigator(this, leftMargin(), "nav");
		m_navPanel->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
	}
}

KexiScrollView::~KexiScrollView()
{
}

void
KexiScrollView::setWidget(QWidget *w)
{
	addChild(w);
	m_widget = w;
}

void
KexiScrollView::setRecordNavigatorVisible(bool visible)
{
	if(m_navPanel->isVisible() && !visible)
		m_navPanel->hide();
	else if(visible)  {
		m_navPanel->show();
		updateNavPanelGeometry();
	}
}

void
KexiScrollView::setSnapToGrid(bool enable, int gridX, int gridY)
{
	m_snapToGrid = enable;
	if(enable) {
		m_gridX = gridX;
		m_gridY = gridY;
	}
}

void
KexiScrollView::refreshContentsSizeLater(bool horizontal, bool vertical)
{
	if (!m_smodeSet) {
		m_smodeSet = true;
		m_vsmode = vScrollBarMode();
		m_hsmode = hScrollBarMode();
	}
//	if (vertical)
		setVScrollBarMode(QScrollView::AlwaysOff);
	//if (horizontal)
		setHScrollBarMode(QScrollView::AlwaysOff);
	updateScrollBars();
	m_delayedResize.start( 100, true );
}

void
KexiScrollView::refreshContentsSize()
{
	if(!m_widget)
		return;
	if (m_preview) {
		resizeContents(m_widget->width(), m_widget->height());
		setVScrollBarMode(m_vsmode);
		setHScrollBarMode(m_hsmode);
		m_smodeSet = false;
		updateScrollBars();
	}
	else {
		// Ensure there is always space to resize Form
		if(m_widget->width() + 200 > contentsWidth())
			resizeContents(m_widget->width() + 300, contentsHeight());
		if(m_widget->height() + 200 > contentsHeight())
			resizeContents(contentsWidth(), m_widget->height() + 300);
	}
}

void
KexiScrollView::updateNavPanelGeometry()
{
	if (m_navPanel)
		m_navPanel->updateGeometry(leftMargin());
}

void
KexiScrollView::contentsMousePressEvent(QMouseEvent *ev)
{
	if(!m_widget)
		return;

	QRect r3(0, 0, m_widget->width() + 4, m_widget->height() + 4);
	if(!r3.contains(ev->pos())) // clicked outside form
		//m_form->resetSelection();
		emit outerAreaClicked();

	if(!m_enableResizing)
		return;

	QRect r(m_widget->width(),  0, 4, m_widget->height() + 4); // right limit
	QRect r2(0, m_widget->height(), m_widget->width() + 4, 4); // bottom limit
	if(r.contains(ev->pos()) || r2.contains(ev->pos()))
	{
		m_resizing = true;
		emit resizingStarted();
	}
}

void
KexiScrollView::contentsMouseReleaseEvent(QMouseEvent *)
{
	if(m_resizing) {
		m_resizing = false;
		emit resizingEnded();
	}

	unsetCursor();
}

void
KexiScrollView::contentsMouseMoveEvent(QMouseEvent *ev)
{
	if(!m_widget || !m_enableResizing)
		return;

	if(m_resizing) // resize widget
	{
		int tmpx = ev->x(), tmpy = ev->y();
		const int exceeds_x = (tmpx - contentsX() + 5) - clipper()->width();
		const int exceeds_y = (tmpy - contentsY() + 5) - clipper()->height();
		if (exceeds_x > 0)
			tmpx -= exceeds_x;
		if (exceeds_y > 0)
			tmpy -= exceeds_y;
		if ((tmpx - contentsX()) < 0)
			tmpx = contentsX();
		if ((tmpy - contentsY()) < 0)
			tmpy = contentsY();

		// we look for the max widget right() (or bottom()), which would be the limit for form resizing (not to hide widgets)
		QObjectList *list = m_widget->queryList("QWidget", 0, true, false /* not recursive*/);
		for(QObject *o = list->first(); o; o = list->next())
		{
			QWidget *w = (QWidget*)o;
			tmpx = QMAX(tmpx, (w->geometry().right() + 10));
			tmpy = QMAX(tmpy, (w->geometry().bottom() + 10));
		}
		delete list;

		int neww = -1, newh;
		if(cursor().shape() == QCursor::SizeHorCursor)
		{
			if(m_snapToGrid)
				neww = int( float(tmpx) / float(m_gridX) + 0.5 ) * m_gridX;
			else
				neww = tmpx;
			newh = m_widget->height();
		}
		else if(cursor().shape() == QCursor::SizeVerCursor)
		{
			neww = m_widget->width();
			if(m_snapToGrid)
				newh = int( float(tmpy) / float(m_gridY) + 0.5 ) * m_gridY;
			else
				newh = tmpy;
		}
		else if(cursor().shape() == QCursor::SizeFDiagCursor)
		{
			if(m_snapToGrid) {
				neww = int( float(tmpx) / float(m_gridX) + 0.5 ) * m_gridX;
				newh = int( float(tmpy) / float(m_gridY) + 0.5 ) * m_gridY;
			} else {
				neww = tmpx;
				newh = tmpy;
			}
		}
		//needs update?
		if (neww!=-1 && m_widget->size() != QSize(neww, newh)) {
			m_widget->resize( neww, newh );
			refreshContentsSize();
			updateContents();
		}
	}
	else // update mouse cursor
	{
		QPoint p = ev->pos();
		QRect r(m_widget->width(),  0, 4, m_widget->height()); // right
		QRect r2(0, m_widget->height(), m_widget->width(), 4); // bottom
		QRect r3(m_widget->width(), m_widget->height(), 4, 4); // bottom-right corner

		if(r.contains(p))
			setCursor(QCursor::SizeHorCursor);
		else if(r2.contains(p))
			setCursor(QCursor::SizeVerCursor);
		else if(r3.contains(p))
			setCursor(QCursor::SizeFDiagCursor);
		else
			unsetCursor();
	}
}

void
KexiScrollView::drawContents( QPainter * p, int clipx, int clipy, int clipw, int cliph ) 
{
	QScrollView::drawContents(p, clipx, clipy, clipw, cliph);
	if (m_widget) {
		if(m_preview || !m_outerAreaVisible)
			return;

		//draw right and bottom borders
		const int wx = childX(m_widget);
		const int wy = childY(m_widget);
		p->setPen(palette().active().foreground());
		p->drawLine(wx+m_widget->width(), wy, wx+m_widget->width(), wy+m_widget->height());
		p->drawLine(wx, wy+m_widget->height(), wx+m_widget->width(), wy+m_widget->height());


		if (!KexiScrollView_bufferPm) {
			//create flicker-less buffer
			QString txt(i18n("Outer area"));
			QFontMetrics fm(m_helpFont);
			const int txtw = fm.width(txt), txth = fm.height();
			KexiScrollView_bufferPm_deleter.setObject( KexiScrollView_bufferPm, 
				new QPixmap(txtw, txth) );
			if (!KexiScrollView_bufferPm->isNull()) {
				//create pixmap once
				KexiScrollView_bufferPm->fill( viewport()->paletteBackgroundColor() );
				QPainter *pb = new QPainter(KexiScrollView_bufferPm, this);
				pb->setPen(m_helpColor);
				pb->setFont(m_helpFont);
				pb->drawText(0, 0, txtw, txth, Qt::AlignLeft|Qt::AlignTop, txt);
				delete pb;
			}
		}
		if (!KexiScrollView_bufferPm->isNull()) {
			p->drawPixmap((contentsWidth() + m_widget->width() - KexiScrollView_bufferPm->width())/2,
				(m_widget->height()-KexiScrollView_bufferPm->height())/2, 
				*KexiScrollView_bufferPm);
			p->drawPixmap((m_widget->width() - KexiScrollView_bufferPm->width())/2,
				(contentsHeight() + m_widget->height() - KexiScrollView_bufferPm->height())/2, 
				*KexiScrollView_bufferPm);
		}
	}
}

void
KexiScrollView::leaveEvent( QEvent *e )
{
	QWidget::leaveEvent(e);
	m_widget->update(); //update form elements on too fast mouse move
}

void
KexiScrollView::setHBarGeometry( QScrollBar & hbar, int x, int y, int w, int h )
{
/*todo*/
	kdDebug(44021)<<"KexiTableView::setHBarGeometry"<<endl;
	if (m_navPanel && m_navPanel->isVisible()) {
		m_navPanel->setHBarGeometry( hbar, x, y, w, h );
	}
	else {
		hbar.setGeometry( x, y, w, h );
	}
}


#include "kexiscrollview.moc"

