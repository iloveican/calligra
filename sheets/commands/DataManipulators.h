/* This file is part of the KDE project
   Copyright (C) 2006 Tomas Mecir <mecirt@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#ifndef CALLIGRA_SHEETS_DATA_MANIPULATORS
#define CALLIGRA_SHEETS_DATA_MANIPULATORS

#include "AbstractRegionCommand.h"
#include "Global.h"
#include "Style.h"
#include "Value.h"

#include "calligra_sheets_export.h"


namespace Calligra
{
namespace Sheets
{

/**
 * \ingroup Commands
 * \brief Abstract command for setting values.
 */
class CALLIGRA_SHEETS_COMMON_EXPORT AbstractDataManipulator : public AbstractRegionCommand
{
public:
    AbstractDataManipulator(KUndo2Command* parent = 0);
    virtual ~AbstractDataManipulator();

    virtual bool process(Element* element);

protected:
    /** Return new value. row/col are relative to sheet, not element.
    If the function sets *parse to true, the value will be treated as an
    user-entered string and parsed by Cell. */
    virtual Value newValue(Element *element, int col, int row,
                           bool *parse, Format::Type *fmtType) = 0;

    /** do we want to change this cell ? */
    virtual bool wantChange(Element *element, int col, int row) {
        Q_UNUSED(element)
        Q_UNUSED(col)
        Q_UNUSED(row)
        return true;
    }

    /**
     * Starts the undo recording.
     */
    virtual bool preProcessing();

    /**
     * Processes the region. Calls process(Element*).
     */
    virtual bool mainProcessing();

    /**
     * Stops the undo recording and stores the old data.
     */
    virtual bool postProcessing();
};

/**
 * \ingroup Commands
 * \brief Abstract command for setting values/styles.
 */
class AbstractDFManipulator : public AbstractDataManipulator
{
public:
    AbstractDFManipulator(KUndo2Command *parent = 0);
    virtual ~AbstractDFManipulator();
    virtual bool process(Element* element);

    /** returns whether this manipulator changes formats */
    bool changeFormat() {
        return m_changeformat;
    }
    /** set whether this manipulator changes formats */
    void setChangeFormat(bool chf) {
        m_changeformat = chf;
    }
protected:
    /** this method should return new format for a given cell */
    virtual Style newFormat(Element *element, int col, int row) = 0;

    bool m_changeformat : 1;
};


/**
 * \ingroup Commands
 * \brief Sets values of a cell range.
 */
class DataManipulator : public AbstractDataManipulator
{
public:
    DataManipulator(KUndo2Command* parent = 0);
    virtual ~DataManipulator();
    void setParsing(bool val) {
        m_parsing = val;
    }
    void setExpandMatrix(bool expand) {
        m_expandMatrix = expand;
    }
    /** set the values for the range. Can be either a single value, or
    a value array */
    void setValue(Value val) {
        m_data = val;
    }
    /** If set, all cells shall be switched to this format. If parsing is
    true, the resulting value may end up being different. */
    void setFormat(Format::Type fmtType) {
        m_format = fmtType;
    }
protected:
    virtual bool preProcessing();
    virtual bool process(Element* element);
    virtual Value newValue(Element *element, int col, int row, bool *, Format::Type *);

    Value m_data;
    Format::Type m_format;
    bool m_parsing : 1;
    bool m_expandMatrix : 1;
};


/**
 * \ingroup Commands
 * \brief Fills a value series into a cell range.
 */
class SeriesManipulator : public AbstractDataManipulator
{
public:
    enum Series { Column, Row, Linear, Geometric };

    SeriesManipulator();
    virtual ~SeriesManipulator();

    /** Setup the series. This sets the necessary parameters, and also the
    correct range. */
    void setupSeries(const QPoint &_marker, double start, double end,
                     double step, Series mode, Series type);
protected:
    virtual Value newValue(Element *element, int col, int row, bool *,
                           Format::Type *);

    Series m_type;
    Value m_start, m_step, m_prev;
    int m_last;
};


/**
 * \ingroup Commands
 * \brief Fills values into a cell range.
 */
class FillManipulator : public AbstractDFManipulator
{
public:
    FillManipulator();
    virtual ~FillManipulator();

    enum Direction { Up = 0, Down, Left, Right };

    void setDirection(Direction d) {
        m_dir = d;
    }
protected:
    virtual Value newValue(Element *element, int col, int row,
                           bool *parse, Format::Type *fmtType);
    virtual Style newFormat(Element *element, int col, int row);
    Direction m_dir;
};


/**
 * \ingroup Commands
 * \brief Converts string values to upper-/lowercase.
 */
class CaseManipulator: public AbstractDataManipulator
{
public:
    CaseManipulator();
    virtual ~CaseManipulator();

    enum CaseMode {
        Upper = 0,
        Lower,
        FirstUpper
    };
    void changeMode(CaseMode mode) {
        m_mode = mode;
    }
    void changeLowerCase();
    void changeFirstUpper();
protected:
    virtual Value newValue(Element *element, int col, int row,
                           bool *parse, Format::Type *fmtType);

    /** do we want to change this cell ? */
    virtual bool wantChange(Element *element, int col, int row);

    CaseMode m_mode;
};


/**
 * \ingroup Commands
 * \brief Inserts/Removes cells by shifting other cells.
 */
class ShiftManipulator : public AbstractRegionCommand
{
public:
    enum Direction { ShiftRight, ShiftBottom };
    ShiftManipulator(KUndo2Command* parent = 0);
    virtual ~ShiftManipulator();
    void setDirection(Direction direction) {
        m_direction = direction;
    }
    virtual void setReverse(bool reverse);

protected:
    bool process(Element*);
    virtual bool preProcessing();
    virtual bool mainProcessing();
    virtual bool postProcessing();

private:
    Direction m_direction;

    enum Mode { Insert, Delete };
    Mode m_mode;
};

} // namespace Sheets
} // namespace Calligra

#endif  // CALLIGRA_SHEETS_DATA_MANIPULATORS
