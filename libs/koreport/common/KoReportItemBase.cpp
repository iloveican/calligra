/*
 * Kexi Report Plugin
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "KoReportItemBase.h"
#include <KLocale>
#include "krpos.h"
#include "krsize.h"
#include "krutils.h"
#include <koproperty/Set.h>

KoReportItemBase::KoReportItemBase()
{
    Z = 0;
    m_name = new KoProperty::Property("Name", QString(), i18n("Name"), i18n("Object Name"));
    m_name->setAutoSync(0);
}

KoReportItemBase::~KoReportItemBase() { }

void KoReportItemBase::addDefaultProperties()
{
    m_set->addProperty(m_name);
    m_set->addProperty(m_pos.property());
    m_set->addProperty(m_size.property());
}

bool KoReportItemBase::parseReportTextStyleData(const QDomElement & elemSource, KRTextStyleData & ts)
{
    if (elemSource.tagName() != "report:text-style")
        return false;
    ts.backgroundColor = QColor(elemSource.attribute("fo:background-color", "#ffffff"));
    ts.foregroundColor = QColor(elemSource.attribute("fo:foreground-color", "#000000"));

    bool ok;
    ts.backgroundOpacity = KRUtils::readPercent(elemSource, "fo:background-opacity", 100, &ok);
    if (!ok)
        return false;

    if (!KRUtils::readFontAttributes(elemSource, ts.font))
        return false;
    return true;
}

bool KoReportItemBase::parseReportLineStyleData(const QDomElement & elemSource, KRLineStyleData & ls)
{
    if (elemSource.tagName() == "report:line-style") {
        ls.lineColor = QColor(elemSource.attribute("report:line-color", "#ffffff"));
        ls.weight = elemSource.attribute("report:line-weight", "0").toInt();

        QString l = elemSource.attribute("report:line-style", "nopen");
        if (l == "nopen") {
            ls.style = Qt::NoPen;
        } else if (l == "solid") {
            ls.style = Qt::SolidLine;
        } else if (l == "dash") {
            ls.style = Qt::DashLine;
        } else if (l == "dot") {
            ls.style = Qt::DotLine;
        } else if (l == "dashdot") {
            ls.style = Qt::DashDotLine;
        } else if (l == "dashdotdot") {
            ls.style = Qt::DashDotDotLine;
        }
        return true;
    }
    return false;
}


bool KoReportItemBase::parseReportRect(const QDomElement & elemSource, KRPos *pos, KRSize *siz)
{
    QString sUnit = elemSource.attribute("svg:x", "1cm").right(2);
    const KoUnit unit = KoUnit::fromSymbol(sUnit);
	pos->setUnit(unit);
	siz->setUnit(unit);
	QPointF _pos;
	QSizeF _siz;
	
	_pos.setX(KoUnit::parseValue(elemSource.attribute("svg:x", "1cm")));
	_pos.setY(KoUnit::parseValue(elemSource.attribute("svg:y", "1cm")));
	_siz.setWidth(KoUnit::parseValue(elemSource.attribute("svg:width", "1cm")));
	_siz.setHeight(KoUnit::parseValue(elemSource.attribute("svg:height", "1cm")));
	
	pos->setPointPos(_pos);
	siz->setPointSize(_siz);
	
    return true;
}

void KoReportItemBase::setUnit(const KoUnit& u)
{
    m_pos.setUnit(u);
    m_size.setUnit(u);
}

int KoReportItemBase::render(OROPage* page, OROSection* section, QPointF offset, QVariant data, KRScriptHandler* script)
{
    Q_UNUSED(page)
    Q_UNUSED(section)
    Q_UNUSED(offset)
    Q_UNUSED(data)
    Q_UNUSED(script)
    return 0;
}

int KoReportItemBase::render(OROPage* page, OROSection* section, QPointF offset, KoReportData* data, KRScriptHandler* script)
{
    Q_UNUSED(page)
    Q_UNUSED(section)
    Q_UNUSED(offset)
    Q_UNUSED(data)
    Q_UNUSED(script)
    return 0;
}

QString KoReportItemBase::itemDataSource() const
{
    return QString();
}

KRPos KoReportItemBase::position()
{
    return m_pos;
}

KoProperty::Set* KoReportItemBase::propertySet() const
{
 return m_set;
}

bool KoReportItemBase::supportsSubQuery()
{
    return false;
}

QString KoReportItemBase::entityName()
{
    return m_name->value().toString();
}

void KoReportItemBase::setEntityName(const QString& n)
{
    m_name->setValue(n);
}
