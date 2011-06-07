/*
   KoReport Library
   Copyright (C) 2011 by Dag Andersen (danders@get2net.dk)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoSimpleOdtPrimitive.h"
#include <KoXmlWriter.h>
#include <KoDpi.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>

#include "renderobjects.h"

#include <kdebug.h>

KoSimpleOdtPrimitive::KoSimpleOdtPrimitive(OROPrimitive *primitive)
    : m_primitive(primitive)
{
}

KoSimpleOdtPrimitive::~KoSimpleOdtPrimitive()
{
}

bool KoSimpleOdtPrimitive::isValid() const
{
    return (bool)m_primitive;
}

void KoSimpleOdtPrimitive::setPrimitive(OROPrimitive *primitive)
{
    m_primitive = primitive;
}

int KoSimpleOdtPrimitive::pageNumber() const
{
    //NOTE OROPrimitive page starts at 0, odt starts at 1
    return isValid() ? m_primitive->page()->page() + 1 : 0;
}

void KoSimpleOdtPrimitive::setUID(int uid)
{
    m_uid = uid;
}

int KoSimpleOdtPrimitive::uid() const
{
    return m_uid;
}

QString KoSimpleOdtPrimitive::itemName() const
{
    return QString("Item_%1").arg(m_uid);
}

void KoSimpleOdtPrimitive::createStyle(KoGenStyles &coll)
{
    KoGenStyle gs(KoGenStyle::GraphicStyle, "graphic");
    gs.addProperty("draw:fill", "none");
    gs.addProperty("fo:margin", "0.000000000000000pt");
    gs.addProperty("style:horizontal-pos", "from-left");
    gs.addProperty("style:horizontal-rel", "page");
    gs.addProperty("style:vertical-pos", "from-top");
    gs.addProperty("style:vertical-rel", "page");
    gs.addProperty("style:wrap", "dynamic");
    gs.addProperty("style:wrap-dynamic-threshold", "0.000000000000000pt");

    m_frameStyleName = coll.insert(gs, "F");
}

void KoSimpleOdtPrimitive::createBody(KoXmlWriter *bodyWriter) const
{
}

void KoSimpleOdtPrimitive::commonAttributes(KoXmlWriter *bodyWriter) const
{
    // convert to inches
    qreal x = m_primitive->position().x() / KoDpi::dpiX();
    qreal y = m_primitive->position().y() / KoDpi::dpiX();
    qreal w = m_primitive->size().width() / KoDpi::dpiX();
    qreal h = m_primitive->size().height() / KoDpi::dpiY();

    bodyWriter->addAttribute("svg:x", QString("%1in").arg(x));
    bodyWriter->addAttribute("svg:y", QString("%1in").arg(y));
    bodyWriter->addAttribute("svg:width", QString("%1in").arg(w));
    bodyWriter->addAttribute("svg:height", QString("%1in").arg(h));
    bodyWriter->addAttribute("draw:z-index", "3");
}

bool KoSimpleOdtPrimitive::saveData(KoStore */*store*/, KoXmlWriter*) const
{
    return true;
}

