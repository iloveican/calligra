/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

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

//#include "kotextformat.h"
#include "qrichtext_p.h" // for KoTextFormat
#include "kozoomhandler.h"
#include <kdebug.h>

void KoTextFormat::setPointSizeFloat( float size )
{
    if ( fn.pointSizeFloat() == size )
        return;
    fn.setPointSizeFloat( size );
    update();
}

void KoTextFormat::setStrikeOutLineType (StrikeOutLineType _type)
{
    if ( m_strikeOutLine == _type )
        return;
    m_strikeOutLine = _type;
    update();
}

void KoTextFormat::setUnderlineLineType (UnderlineLineType _type)
{
    if ( m_underlineLine == _type )
        return;
    m_underlineLine = _type;
    update();
}

void KoTextFormat::setUnderlineLineStyle (UnderlineLineStyle _type)
{
    if ( m_underlineLineStyle == _type )
        return;
    m_underlineLineStyle = _type;
    update();
}

void KoTextFormat::setStrikeOutLineStyle( StrikeOutLineStyle _type )
{
    if ( m_strikeOutLineStyle == _type )
        return;
    m_strikeOutLineStyle = _type;
    update();
}

void KoTextFormat::setTextBackgroundColor(const QColor &_col)
{
    if(m_textBackColor==_col)
        return;
    m_textBackColor=_col;
    update();
}
void KoTextFormat::setTextUnderlineColor(const QColor &_col)
{
    if ( m_textUnderlineColor == _col )
        return;
    m_textUnderlineColor=_col;
    update();
}

int KoTextFormat::compare( const KoTextFormat & format ) const
{
    int flags = 0;
    if ( fn.weight() != format.fn.weight() )
        flags |= KoTextFormat::Bold;
    if ( fn.italic() != format.fn.italic() )
        flags |= KoTextFormat::Italic;
    if ( textUnderlineColor()!=format.textUnderlineColor() ||
         underlineLineType()!= format.underlineLineType() ||
         underlineLineStyle() != format.underlineLineStyle())
        flags |= KoTextFormat::ExtendUnderLine;
    if ( fn.family() != format.fn.family() )
        flags |= KoTextFormat::Family;
    if ( fn.pointSize() != format.fn.pointSize() )
        flags |= KoTextFormat::Size;
    if ( color() != format.color() )
        flags |= KoTextFormat::Color;
    if ( vAlign() != format.vAlign() )
        flags |= KoTextFormat::VAlign;
    if ( strikeOutLineType() != format.strikeOutLineType()
        || underlineLineStyle() != format.underlineLineStyle())
        flags |= KoTextFormat::StrikeOut;
    if ( textBackgroundColor() != format.textBackgroundColor() )
        flags |= KoTextFormat::TextBackgroundColor;
    return flags;
}

QColor KoTextFormat::defaultTextColor( QPainter * painter )
{
    if ( painter->device()->devType() == QInternal::Printer )
        return Qt::black;
    return QApplication::palette().color( QPalette::Active, QColorGroup::Text );
}

float KoTextFormat::screenPointSize( const KoZoomHandler* zh, bool applyZoom ) const
{
    int pointSizeLU = font().pointSize();
    if ( vAlign() != KoTextFormat::AlignNormal )
        pointSizeLU = ( ( pointSizeLU * 2 ) / 3 );

    return applyZoom // true when painting, false when formatting
        ? zh->layoutUnitToFontSize( pointSizeLU, false /* forPrint */ )
        : KoTextZoomHandler::layoutUnitPtToPt( pointSizeLU );
}

QFont KoTextFormat::screenFont( const KoZoomHandler* zh, bool applyZoom ) const
{
    float pointSize = screenPointSize( zh, applyZoom );
    //kdDebug() << "KoTextFormat::screenFont applyZoom=" << applyZoom << " pointSize=" << pointSize << endl;
    // Compare if this is the size for which we cached the font metrics.
    // We have to do this very dynamically, because 2 views could be painting the same
    // stuff, with different zoom levels. So no absolute caching possible.
    if ( applyZoom )
    {
        /*if ( d->m_screenFont )
            kdDebug() << " d->m_screenFont->pointSizeFloat()=" << d->m_screenFont->pointSizeFloat() << endl;*/
        if ( !d->m_screenFont || pointSize != d->m_screenFont->pointSizeFloat() )
        {
            delete d->m_screenFont;
            d->m_screenFont = new QFont( font() );
            d->m_screenFont->setPointSizeFloat( pointSize );
            //kdDebug() << "KoTextFormat::screenFont created new font with size " << pointSize << endl;
        }
        return *d->m_screenFont;
    }
    else // cache in different vars
    {
        if ( !d->m_screenFontNoZoom || pointSize != d->m_screenFontNoZoom->pointSizeFloat() )
        {
            delete d->m_screenFontNoZoom;
            d->m_screenFontNoZoom = new QFont( font() );
            d->m_screenFontNoZoom->setPointSizeFloat( pointSize );
            //kdDebug() << "KoTextFormat::screenFont created new font with size " << pointSize << endl;
        }
        return *d->m_screenFontNoZoom;
    }
}

QFontMetrics KoTextFormat::screenFontMetrics( const KoZoomHandler* zh, bool applyZoom ) const
{
    float pointSize = screenPointSize( zh, applyZoom );
    if ( applyZoom )
    {
        if ( !d->m_screenFont )
            (void)screenFont( zh, applyZoom ); // we need it below, and this way it'll be ready for painting

        // Compare if this is the size for which we cached the font metrics.
        // We have to do this very dynamically, because 2 views could be painting the same
        // stuff, with different zoom levels. So no absolute caching possible.
        if ( !d->m_screenFontMetrics || pointSize != d->m_screenFont->pointSizeFloat() )
        {
            //kdDebug() << this << " KoTextFormat::screenFontMetrics pointSize=" << pointSize << " d->m_screenFont->pointSizeFloat()=" << d->m_screenFont->pointSizeFloat() << endl;
            QFont f( font() );
            f.setPointSizeFloat( pointSize );
            delete d->m_screenFontMetrics;
            d->m_screenFontMetrics = new QFontMetrics( f );
            //kdDebug() << "KoTextFormat::screenFontMetrics created new metrics with size " << pointSize << "   height:" << d->m_screenFontMetrics->height() << endl;
        }
        return *d->m_screenFontMetrics;
    }
    else // cache in different vars
    {
        if ( !d->m_screenFontNoZoom )
            (void)screenFont( zh, applyZoom ); // we need it below, and this way it'll be ready for painting

        // Compare if this is the size for which we cached the font metrics.
        // We have to do this very dynamically, because 2 views could be painting the same
        // stuff, with different zoom levels. So no absolute caching possible.
        if ( !d->m_screenFontMetricsNoZoom || pointSize != d->m_screenFontNoZoom->pointSizeFloat() )
        {
            //kdDebug() << this << " KoTextFormat::screenFontMetrics pointSize=" << pointSize << " d->m_screenFontNoZoom->pointSizeFloat()=" << d->m_screenFontNoZoom->pointSizeFloat() << endl;
            QFont f( font() );
            f.setPointSizeFloat( pointSize );
            delete d->m_screenFontMetricsNoZoom;
            d->m_screenFontMetricsNoZoom = new QFontMetrics( f );
            //kdDebug() << "KoTextFormat::screenFontMetrics created new metrics with size " << pointSize << "   height:" << d->m_screenFontMetricsNoZoom->height() << endl;
        }
        return *d->m_screenFontMetricsNoZoom;
    }
}

int KoTextFormat::charWidth( const KoZoomHandler* zh, bool applyZoom, const KoTextStringChar* c,
                             const KoTextParag* parag, int i ) const
{
    if ( c->c.unicode() == 0xad ) // soft hyphen
	return 0;
    Q_ASSERT( !c->isCustom() ); // actually it's a bit stupid to call this for custom items
    if( c->isCustom() ) {
	 if( c->customItem()->placement() == KoTextCustomItem::PlaceInline ) {
             // customitem width is in LU pixels. Convert to 100%-zoom-pixels (pt2pt==pix2pix)
             int w = qRound( KoTextZoomHandler::layoutUnitPtToPt( c->customItem()->width ) );
             return applyZoom ? ( w * zh->zoom() / 100 ) : w;
         }
         else
             return 0;
    }
    int pixelww;
    int r = c->c.row();
    if( r < 0x06 || r > 0x1f )
    {
        // The fast way: use the cached font metrics from KoTextFormat
        pixelww = this->screenFontMetrics( zh, applyZoom ).width( c->c );
    }
    else {
        // Here we have no choice, we need to create the format
        KoTextFormat tmpFormat( *this );  // make a copy
        tmpFormat.setPointSizeFloat( this->screenPointSize( zh, applyZoom ) );
        // complex text. We need some hacks to get the right metric here
        QString str;
        int pos = 0;
        if( i > 4 )
            pos = i - 4;
        int off = i - pos;
        int end = QMIN( parag->length(), i + 4 );
        while ( pos < end ) {
            str += parag->at(pos)->c;
            pos++;
        }
        pixelww = tmpFormat.width( str, off );
    }

#ifdef DEBUG_FORMATTER
    if ( applyZoom ) // ###
        qDebug( "\nKoTextFormatter::format: char=%s, LU-size=%d, LU-width=%d [equiv. to pix=%d] pixel-width=%d", // format=%s",
                QString(c->c).latin1(), this->font().pointSize(),
                ww, zh->layoutUnitToPixelX(ww), pixelww/*, this->key().latin1()*/ );
#endif
    return pixelww;
}

int KoTextFormat::height() const
{
    // Calculate height using 100%-zoom font
    int h = screenFontMetrics( 0L, false ).height();
    //kdDebug() << "KoTextFormat::height 100%-zoom font says h=" << h << " in LU:" << KoTextZoomHandler::ptToLayoutUnitPt(h) << endl;
    // Then scale to LU
    return qRound( KoTextZoomHandler::ptToLayoutUnitPt( h ) );
}

int KoTextFormat::ascent() const
{
    // Calculate ascent using 100%-zoom font
    int h = screenFontMetrics( 0L, false ).ascent();
    // Then scale to LU
    return qRound( KoTextZoomHandler::ptToLayoutUnitPt( h ) );
}

int KoTextFormat::descent() const
{
    // Calculate descent using 100%-zoom font
    int h = screenFontMetrics( 0L, false ).descent();
    // Then scale to LU
    return qRound( KoTextZoomHandler::ptToLayoutUnitPt( h ) );
}

int KoTextFormat::charWidthLU( const KoTextStringChar* c, const KoTextParag* parag, int i ) const
{
    return KoTextZoomHandler::ptToLayoutUnitPt( charWidth( 0L, false, c, parag, i ) );
}

int KoTextFormat::width( const QChar& ch ) const
{
    return KoTextZoomHandler::ptToLayoutUnitPt( screenFontMetrics( 0L, false ).width( ch ) );
}

//static
QString KoTextFormat::underlineStyleToString( KoTextFormat::UnderlineLineStyle _lineType )
{
    QString strLineType;
    switch ( _lineType )
    {
    case KoTextFormat::U_SOLID:
        strLineType ="solid";
        break;
    case KoTextFormat::U_DASH:
        strLineType ="dash";
        break;
    case KoTextFormat::U_DOT:
        strLineType ="dot";
        break;
    case KoTextFormat::U_DASH_DOT:
        strLineType="dashdot";
        break;
    case KoTextFormat::U_DASH_DOT_DOT:
        strLineType="dashdotdot";
        break;
    }
    return strLineType;
}

QString KoTextFormat::strikeOutStyleToString( KoTextFormat::StrikeOutLineStyle _lineType )
{
    QString strLineType;
    switch ( _lineType )
    {
    case KoTextFormat::S_SOLID:
        strLineType ="solid";
        break;
    case KoTextFormat::S_DASH:
        strLineType ="dash";
        break;
    case KoTextFormat::S_DOT:
        strLineType ="dot";
        break;
    case KoTextFormat::S_DASH_DOT:
        strLineType="dashdot";
        break;
    case KoTextFormat::S_DASH_DOT_DOT:
        strLineType="dashdotdot";
        break;
    }
    return strLineType;
}

KoTextFormat::UnderlineLineStyle KoTextFormat::stringToUnderlineStyle( const QString & _str )
{
    if ( _str =="solid")
        return KoTextFormat::U_SOLID;
    else if ( _str =="dash" )
        return KoTextFormat::U_DASH;
    else if ( _str =="dot" )
        return KoTextFormat::U_DOT;
    else if ( _str =="dashdot")
        return KoTextFormat::U_DASH_DOT;
    else if ( _str=="dashdotdot")
        return KoTextFormat::U_DASH_DOT_DOT;
    else
        return KoTextFormat::U_SOLID;
}

KoTextFormat::StrikeOutLineStyle KoTextFormat::stringToStrikeOutStyle( const QString & _str )
{
    if ( _str =="solid")
        return KoTextFormat::S_SOLID;
    else if ( _str =="dash" )
        return KoTextFormat::S_DASH;
    else if ( _str =="dot" )
        return KoTextFormat::S_DOT;
    else if ( _str =="dashdot")
        return KoTextFormat::S_DASH_DOT;
    else if ( _str=="dashdotdot")
        return KoTextFormat::S_DASH_DOT_DOT;
    else
        return KoTextFormat::S_SOLID;
}
