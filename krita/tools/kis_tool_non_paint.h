/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#if !defined KIS_TOOL_NON_PAINT_H_
#define KIS_TOOL_NON_PAINT_H_

#include <qcursor.h>
#include "kis_global.h"
#include "kis_types.h"
#include "kis_tool.h"

class QEvent;
class QKeyEvent;
class QPaintEvent;
class QMouseEvent;
class QRect;
class KDialog;
class KisDoc;
class KisView;

class KisToolNonPaint : public KisToolInterface {
	Q_OBJECT
	typedef KisToolInterface super;

public:
	KisToolNonPaint(KisView *view, KisDoc *doc);
	virtual ~KisToolNonPaint();
	
public:
	virtual void activate();
	virtual void setup();
	virtual void paint(QPainter& gc);
	virtual void paint(QPainter& gc, const QRect& rc);
	virtual void clear();
	virtual void clear(const QRect& rc);
	virtual void enter(QEvent *e);
	virtual void leave(QEvent *e);
	virtual void mousePress(QMouseEvent *e);
	virtual void mouseMove(QMouseEvent *e);
	virtual void mouseRelease(QMouseEvent *e);
	virtual void keyPress(QKeyEvent *e);
	virtual void keyRelease(QKeyEvent *e);
	virtual KDialog *options();
	virtual void save(KisToolMementoSP memento);
	virtual void restore(KisToolMementoSP memento);
	virtual void cursor(QWidget *w) const;
	virtual void setCursor(const QCursor& cursor);

public slots:
	virtual void activateSelf();

private:
	KisView *m_view;
	KisDoc *m_doc;
	QCursor m_cursor;
};

#endif // KIS_TOOL_NON_PAINT_H_

