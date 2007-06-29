/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
   Boston, MA 02110-1301, USA.
*/

#include "Filter.h"

#include <QList>
#include <QRect>
#include <QString>

#include "CellStorage.h"
#include "DatabaseRange.h"
#include "Doc.h"
#include "RowColumnFormat.h"
#include "Region.h"
#include "Sheet.h"
#include "Value.h"
#include "ValueConverter.h"

using namespace KSpread;

class AbstractCondition
{
public:
    virtual ~AbstractCondition() {}
    enum Type { And, Or, Condition };
    virtual Type type() const = 0;
    virtual void loadOdf() = 0;
    virtual void saveOdf() = 0;
    virtual bool evaluate(const DatabaseRange& database, int index) const = 0;
    virtual bool isEmpty() const = 0;
    virtual void removeConditions(int fieldNumber) = 0;
};

/**
 * OpenDocument, 8.7.2 Filter And
 */
class Filter::And : public AbstractCondition
{
public:
    And() {}
    And(const And& other);
    virtual ~And() { qDeleteAll(list); }
    virtual Type type() const { return AbstractCondition::And; }
    virtual void loadOdf() {}
    virtual void saveOdf() {}
    virtual bool evaluate(const DatabaseRange& database, int index) const
    {
        for (int i = 0; i < list.count(); ++i)
        {
            // lazy evaluation, stop on first false
            if (!list[i]->evaluate(database, index))
                return false;
        }
        return true;
    }
    virtual bool isEmpty() const { return list.isEmpty(); }
    virtual void removeConditions(int fieldNumber)
    {
        for (int i = 0; i < list.count(); ++i)
            list[i]->removeConditions(fieldNumber);
        QList<AbstractCondition*> list;
        for (int i = 0; i < list.count(); ++i)
        {
            if (!list[i]->isEmpty())
                list.append(this->list[i]);
        }
        this->list = list;
    }

public:
    QList<AbstractCondition*> list; // allowed: Or or Condition
};

/**
 * OpenDocument, 8.7.3 Filter Or
 */
class Filter::Or : public AbstractCondition
{
public:
    Or() {}
    Or(const Or& other);
    virtual ~Or() { qDeleteAll(list); }
    virtual Type type() const { return AbstractCondition::Or; }
    virtual void loadOdf() {}
    virtual void saveOdf() {}
    virtual bool evaluate(const DatabaseRange& database, int index) const
    {
        for (int i = 0; i < list.count(); ++i)
        {
            // lazy evaluation, stop on first true
            if (list[i]->evaluate(database, index))
                return true;
        }
        return false;
    }
    virtual bool isEmpty() const { return list.isEmpty(); }
    virtual void removeConditions(int fieldNumber)
    {
        for (int i = 0; i < list.count(); ++i)
            list[i]->removeConditions(fieldNumber);
        QList<AbstractCondition*> list;
        for (int i = 0; i < list.count(); ++i)
        {
            if (!list[i]->isEmpty())
                list.append(this->list[i]);
        }
        this->list = list;
    }

public:
    QList<AbstractCondition*> list; // allowed: And or Condition
};

/**
 * OpenDocument, 8.7.4 Filter Condition
 */
class Filter::Condition : public AbstractCondition
{
public:
    Condition(int fieldNumber, Comparison comparison, const QString& value,
              Qt::CaseSensitivity caseSensitivity, Mode mode)
        : fieldNumber(fieldNumber)
        , value(value)
        , operation(comparison)
        , caseSensitivity(caseSensitivity)
        , dataType(mode)
    {
    }
    virtual ~Condition() {}

    virtual Type type() const { return AbstractCondition::Condition; }
    virtual void loadOdf() {}
    virtual void saveOdf() {}
    virtual bool evaluate(const DatabaseRange& database, int index) const
    {
        const Sheet* sheet = (*database.range().constBegin())->sheet();
        const QRect range = database.range().lastRange();
        const int start = database.orientation() == Qt::Vertical ? range.left() : range.top();
        kDebug() << "index: " << index << " start: " << start << " fieldNumber: " << fieldNumber << endl;
        const Value value = database.orientation() == Qt::Vertical
                            ? sheet->cellStorage()->value(start + fieldNumber, index)
                            : sheet->cellStorage()->value(index, start + fieldNumber);
        const QString testString = sheet->doc()->converter()->asString(value).asString();
        switch (operation)
        {
            case Match:
            {
                kDebug() << "Match? " << this->value << " " << testString << endl;
                if (QString::compare(this->value, testString, caseSensitivity) == 0)
                    return true;
                break;
            }
            case NotMatch:
            {
                kDebug() << "Not Match? " << this->value << " " << testString << endl;
                if (QString::compare(this->value, testString, caseSensitivity) != 0)
                    return true;
                break;
            }
            default:
                break;
        }
        return false;
    }
    virtual bool isEmpty() const { return fieldNumber == -1; }
    virtual void removeConditions(int fieldNumber)
    {
        if (this->fieldNumber == fieldNumber)
            this->fieldNumber = -1;
    }

public:
    int fieldNumber;
    QString value; // Value?
    Comparison operation;
    Qt::CaseSensitivity caseSensitivity;
    Mode dataType;
};

Filter::And::And(const And& other)
    : AbstractCondition()
{
    for (int i = 0; i < other.list.count(); ++i)
    {
        if (!other.list[i])
            continue;
        else if (other.list[i]->type() == AbstractCondition::And)
            continue;
        else if (other.list[i]->type() == AbstractCondition::Or)
            list.append(new Filter::Or(*static_cast<Filter::Or*>(other.list[i])));
        else
            list.append(new Filter::Condition(*static_cast<Filter::Condition*>(other.list[i])));
    }
}

Filter::Or::Or(const Or& other)
    : AbstractCondition()
{
    for (int i = 0; i < other.list.count(); ++i)
    {
        if (!other.list[i])
            continue;
        else if (other.list[i]->type() == AbstractCondition::And)
            list.append(new Filter::And(*static_cast<Filter::And*>(other.list[i])));
        else if (other.list[i]->type() == AbstractCondition::Or)
            continue;
        else
            list.append(new Filter::Condition(*static_cast<Filter::Condition*>(other.list[i])));
    }
}


class Filter::Private
{
public:
    Private()
        : condition( 0 )
        , conditionSource(Self)
        , displayDuplicates(true)
    {
    }

    AbstractCondition* condition;
    QString targetRangeAddress; // Region?
    enum { Self, CellRange } conditionSource;
    QString conditionSourceRangeAddress; // Region?
    bool displayDuplicates;
};

Filter::Filter()
    : d( new Private )
{
}

Filter::Filter(const Filter& other)
    : d( new Private)
{
    if (!other.d->condition)
        d->condition = 0;
    else if (other.d->condition->type() == AbstractCondition::And)
        d->condition = new And(*static_cast<And*>(other.d->condition));
    else if (other.d->condition->type() == AbstractCondition::Or )
        d->condition = new Or(*static_cast<Or*>(other.d->condition));
    else
        d->condition = new Condition(*static_cast<Condition*>(other.d->condition));
    d->targetRangeAddress = other.d->targetRangeAddress;
    d->conditionSource = other.d->conditionSource;
    d->conditionSourceRangeAddress = other.d->conditionSourceRangeAddress;
    d->displayDuplicates = other.d->displayDuplicates;
}

Filter::~Filter()
{
    delete d->condition;
    delete d;
}

void Filter::addCondition(Composition composition,
                          int fieldNumber, Comparison comparison, const QString& value,
                          Qt::CaseSensitivity caseSensitivity, Mode mode)
{
    Condition* condition = new Condition(fieldNumber, comparison, value, caseSensitivity, mode);
    if (!d->condition)
    {
        d->condition = condition;
    }
    else if (composition == AndComposition)
    {
        if (d->condition->type() == AbstractCondition::And)
        {
            And* andComposition = static_cast<And*>(d->condition);
            andComposition->list.append(condition);
        }
        else
        {
            And* andComposition = new And();
            andComposition->list.append(d->condition);
            andComposition->list.append(condition);
            d->condition = andComposition;
        }
    }
    else // composition == OrComposition
    {
        if (d->condition->type() == AbstractCondition::Or)
        {
            Or* andComposition = static_cast<Or*>(d->condition);
            andComposition->list.append(condition);
        }
        else
        {
            Or* orComposition = new Or();
            orComposition->list.append(d->condition);
            orComposition->list.append(condition);
            d->condition = orComposition;
        }
    }
}

void Filter::removeConditions(int fieldNumber)
{
    if (fieldNumber == -1)
    {
        delete d->condition;
        d->condition = 0;
        return;
    }
    if (!d->condition)
        return;
    d->condition->removeConditions(fieldNumber);
    if (d->condition->isEmpty())
    {
        delete d->condition;
        d->condition = 0;
    }
}

bool Filter::isEmpty() const
{
    return d->condition ? d->condition->isEmpty() : true;
}

void Filter::apply(const DatabaseRange& database) const
{
    if (!d->condition)
        return;
    Sheet* const sheet = (*database.range().constBegin())->sheet();
    const QRect range = database.range().lastRange();
    const int start = database.orientation() == Qt::Vertical ? range.top() : range.left();
    const int end = database.orientation() == Qt::Vertical ? range.bottom() : range.right();
    for (int i = start + 1; i <= end; ++i)
    {
//         kDebug() << "Checking column/row " << i << endl;
        if (database.orientation() == Qt::Vertical)
        {
            sheet->nonDefaultRowFormat(i)->setFiltered(!d->condition->evaluate(database, i));
            if (d->condition->evaluate(database, i))
                kDebug() << "showing row " << i << endl;
            else
                kDebug() << "hiding row " << i << endl;
            sheet->emitHideRow();
        }
        else // database.orientation() == Qt::Horizontal
        {
            sheet->nonDefaultColumnFormat(i)->setFiltered(!d->condition->evaluate(database, i));
            sheet->emitHideColumn();
        }
    }
}
