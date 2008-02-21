/* This file is part of the KDE project
   Copyright (C) 2008 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KPrMatrixWipeStrategy.h"

#include <QWidget>
#include <QPainter>
#include <kdebug.h>

static const int squaresPerRow = 15;
static const int squaresPerCol = 11;

KPrMatrixWipeStrategy::KPrMatrixWipeStrategy(KPrPageEffect::SubType subType, const char * smilType, const char *smilSubType, bool reverse)
    : KPrPageEffectStrategy( subType, smilType, smilSubType, reverse )
{
}

KPrMatrixWipeStrategy::~KPrMatrixWipeStrategy()
{
}

void KPrMatrixWipeStrategy::setup( const KPrPageEffect::Data &data, QTimeLine &timeLine )
{
    timeLine.setFrameRange( 0, maxIndex(squaresPerCol, squaresPerRow) );
}

static inline int floor(double d) { return (int) (d + 1e-5); }
static inline int ceil(double d) { return (int) (d + 1 - 1e-5); }

void KPrMatrixWipeStrategy::paintStep( QPainter &p, int currPos, const KPrPageEffect::Data &data )
{
    int width = data.m_widget->width();
    int height = data.m_widget->height();
    kDebug() << "width:" << width << "height:" << height;

    for (int i = 0; i < squaresPerRow; ++i) {
        for (int j = 0; j < squaresPerCol; ++j) {
            QRect rect(floor(qreal(width) / squaresPerRow * i), floor(qreal(height) / squaresPerCol * j),
                ceil(qreal(width) / squaresPerRow), ceil(qreal(height) / squaresPerCol));
            int square = squareIndex(i, j, squaresPerRow, squaresPerCol);
            if (square <= currPos) {
                p.drawPixmap( rect.topLeft(), data.m_newPage, rect );
            } else {
                p.drawPixmap( rect.topLeft(), data.m_oldPage, rect );
            }
        }
    }
}

void KPrMatrixWipeStrategy::next( const KPrPageEffect::Data &data )
{
    int lastPos = data.m_timeLine.frameForTime( data.m_lastTime );
    int currPos = data.m_timeLine.frameForTime( data.m_currentTime );

    int width = data.m_widget->width();
    int height = data.m_widget->height();
    for (int i = 0; i < squaresPerRow; ++i) {
        for (int j = 0; j < squaresPerCol; ++j) {
            QRect rect(floor(qreal(width) / squaresPerRow * i), floor(qreal(height) / squaresPerCol * j),
                ceil(qreal(width) / squaresPerRow), ceil(qreal(height) / squaresPerCol));
            int square = squareIndex(i, j, squaresPerRow, squaresPerCol);
            if (square <= currPos && square >= lastPos) {
                data.m_widget->update(rect);
            }
        }
    }
}

