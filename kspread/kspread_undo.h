#ifndef __kspread_undo_h__
#define __kspread_undo_h__

class KSpreadUndo;
class KSpreadUndoAction;
class KSpreadTable;

#include "kspread_doc.h"
#include "kspread_layout.h"
#include "kspread_cell.h"
#include "kspread_table.h"

#include <qstack.h>
#include <qstring.h>
#include <qrect.h>

class KSpreadUndoAction
{
public:
    KSpreadUndoAction( KSpreadDoc *_doc ) { m_pDoc = _doc; }
    virtual ~KSpreadUndoAction() { }
    
    virtual void undo() { }
    virtual void redo() { }

protected:
    KSpreadDoc *m_pDoc; 
};

class KSpreadUndoDeleteColumn : public KSpreadUndoAction
{
public:
    KSpreadUndoDeleteColumn( KSpreadDoc *_doc, KSpreadTable *_table, int _column );
    virtual ~KSpreadUndoDeleteColumn();
    
    virtual void undo();
    virtual void redo();

    void appendCell( KSpreadCell *_cell );
    void setColumnLayout( ColumnLayout *l ) { m_pColumnLayout = l; }
    
protected:
    KSpreadTable *m_pTable;
    int m_iColumn;
    QList<KSpreadCell> m_lstCells;
    ColumnLayout *m_pColumnLayout;
};

class KSpreadUndoInsertColumn : public KSpreadUndoAction
{
public:
    KSpreadUndoInsertColumn( KSpreadDoc *_doc, KSpreadTable *_table, int _column );
    virtual ~KSpreadUndoInsertColumn();
    
    virtual void undo();
    virtual void redo();
    
protected:
    KSpreadTable *m_pTable;
    int m_iColumn;
};

class KSpreadUndoDeleteRow : public KSpreadUndoAction
{
public:
    KSpreadUndoDeleteRow( KSpreadDoc *_doc, KSpreadTable *_table, int _row );
    virtual ~KSpreadUndoDeleteRow();
    
    virtual void undo();
    virtual void redo();

    void appendCell( KSpreadCell *_cell );
    void setRowLayout( RowLayout *l ) { m_pRowLayout = l; }
    
protected:
    KSpreadTable *m_pTable;
    int m_iRow;
    QList<KSpreadCell> m_lstCells;
    RowLayout *m_pRowLayout;
};

class KSpreadUndoInsertRow : public KSpreadUndoAction
{
public:
    KSpreadUndoInsertRow( KSpreadDoc *_doc, KSpreadTable *_table, int _row );
    virtual ~KSpreadUndoInsertRow();
    
    virtual void undo();
    virtual void redo();
    
protected:
    KSpreadTable *m_pTable;
    int m_iRow;
};

class KSpreadUndoSetText : public KSpreadUndoAction
{
public:
    KSpreadUndoSetText( KSpreadDoc *_doc, KSpreadTable *_table, const char *_data, int _column, int _row );
    virtual ~KSpreadUndoSetText();

    virtual void undo();
    virtual void redo();
    
protected:
    KSpreadTable *m_pTable;
    int m_iRow;
    int m_iColumn;
    QString m_strText;
    QString m_strRedoText; 
};
    
class KSpreadUndoCellLayout : public KSpreadUndoAction
{
public:
    KSpreadUndoCellLayout( KSpreadDoc *_doc, KSpreadTable *_table, QRect &_selection );
    virtual ~KSpreadUndoCellLayout();

    virtual void undo();
    virtual void redo();

    void copyLayout( QList<KSpreadLayout> &list);
    
protected:
    QRect m_rctRect;
    QList<KSpreadLayout> m_lstLayouts;
    QList<KSpreadLayout> m_lstRedoLayouts;
    KSpreadTable *m_pTable;
};

class KSpreadUndoDelete : public KSpreadUndoAction
{ 
public:
    KSpreadUndoDelete( KSpreadDoc *_doc, KSpreadTable *_table, QRect &_rect );
    virtual ~KSpreadUndoDelete();
    
    virtual void undo();
    virtual void redo();

protected:
    QRect m_rctRect;
    QByteArray m_array;
    KSpreadTable *m_pTable;
};
    
class KSpreadUndo
{
public:
    KSpreadUndo( KSpreadDoc *_doc );
    ~KSpreadUndo();
    
    void undo();
    void redo();
    void clear();

    void lock() { m_bLocked = TRUE; }
    void unlock() { m_bLocked = FALSE; }
    bool isLocked() { return m_bLocked; }
    
    bool hasUndoActions() { return !m_stckUndo.isEmpty(); }
    bool hasRedoActions() { return !m_stckRedo.isEmpty(); }

    void appendUndo( KSpreadUndoAction *_action );
    
protected:
    QStack<KSpreadUndoAction> m_stckUndo;
    QStack<KSpreadUndoAction> m_stckRedo;

    KSpreadDoc *m_pDoc;

    bool m_bLocked;
};

#endif
