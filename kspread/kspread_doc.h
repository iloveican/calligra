#ifndef __kspread_h__
#define __kspread_h__

class KSpreadDoc;

#include <part_frame_impl.h>
#include <document_impl.h>

#include <qobject.h>
#include <qrect.h>
#include <qstring.h>
#include <qpainter.h>
#include <qprinter.h>

#include "kmetaedit.h"

#include "kspread.h"
#include "kspread_python.h"
#include "kspread_undo.h"
#include "kspread_view.h"
#include "kspread_map.h"

#define MIME_TYPE "application/x-kspread"
#define EDITOR "IDL:KSpread/Document:1.0"

/**
 * This class holds the data that makes up a spreadsheet.
 */
class KSpreadDoc : public QObject,
		   virtual public Document_impl,
		   virtual public KSpread::Document_skel
{
    Q_OBJECT
public:
    // C++
    /**
     * Paper Sizes.
     */
    enum PaperFormat { A3, A4, A5, LETTER, EXECUTIVE };
    
    KSpreadDoc();
    ~KSpreadDoc();
  
    virtual void cleanUp();

    // IDL
    virtual CORBA::Boolean open( const char *_filename );
    virtual CORBA::Boolean saveAs( const char *_filename, const char *_format );

    virtual OPParts::View_ptr createView();
  
    virtual void viewList( OPParts::Document::ViewList*& _list );
  
    virtual char* mimeType() { return CORBA::string_dup( MIME_TYPE ); }
  
    virtual CORBA::Boolean isModified() { return m_bModified; }

    // C++
    /**
     * @return a pointer to a new KSpreadTable. The KSpreadTable is not added to the map
     *         nor added to the GUI.
     */
    KSpreadTable* createTable();
    
    /**
     * Adds a KSpreadTable to the GUI and makes it active. In addition the KSpreadTable is
     * added to the map.
     *
     * @see KSpreadView
     * @see KSpreadMap
     */
     void addTable( KSpreadTable *_table );

     KSpreadMap *map() { return m_pMap; }
  
    /**
     * @return the printable width of the paper in millimeters.
     */
    float printableWidth() { return m_paperWidth - m_leftBorder - m_rightBorder; }

    /**
     * @return the printable height of the paper in millimeters.
     */
    float printableHeight() { return m_paperHeight - m_topBorder - m_bottomBorder; }

    float paperHeight() { return m_paperHeight; }
    float paperWidth() { return m_paperWidth; }
    
    /**
     * @return the left border in millimeters
     */
    float leftBorder() { return m_leftBorder; }
    /**
     * @return the right border in millimeters
     */
    float rightBorder() { return m_rightBorder; }
    /**
     * @return the top border in millimeters
     */
    float topBorder() { return m_topBorder; }
    /**
     * @return the bottom border in millimeters
     */
    float bottomBorder() { return m_bottomBorder; }
    
    /**
     * @return the orientation of the paper.
     */
    QPrinter::Orientation orientation() { return m_orientation; }
    /**
     * @return the ascii name of the paper orientation ( like Portrait, Landscape )
     */
    const char* orientationString();

    /**
     * @return the paper format.
     */
    PaperFormat paperFormat() { return m_paperFormat; }
    /**
     * @return the ascii name of the paper format ( like A4, Letter etc. )
     */
    const char* paperFormatString();

    /**
     * Changes the paper layout and repaints the currently displayed KSpreadTable.
     */
    void setPaperLayout( float _leftBorder, float _topBorder, float _rightBorder, float _bottomBoder,
			 PaperFormat _paper, QPrinter::Orientation orientation );
    /**
     * A convenience function using a string as paper format and orientation.
     */
    void setPaperLayout( float _leftBorder, float _topBorder, float _rightBorder, float _bottomBoder,
			 const char* _paper, const char* _orientation );

    QString headLeft( int _p, const char *_t  ) { if ( m_headLeft.isNull() ) return "";
                                    return completeHeading( m_headLeft.data(), _p, _t ); }
    QString headMid( int _p, const char *_t ) { if ( m_headMid.isNull() ) return "";
                                    return completeHeading( m_headMid.data(), _p, _t ); }
    QString headRight( int _p, const char *_t ) { if ( m_headRight.isNull() ) return "";
                                    return completeHeading( m_headRight.data(), _p, _t ); }
    QString footLeft( int _p, const char *_t ) { if ( m_footLeft.isNull() ) return "";
                                    return completeHeading( m_footLeft.data(), _p, _t ); }
    QString footMid( int _p, const char *_t ) { if ( m_footMid.isNull() ) return "";
                                    return completeHeading( m_footMid.data(), _p, _t ); }
    QString footRight( int _p, const char *_t ) { if ( m_footRight.isNull() ) return "";
                                    return completeHeading( m_footRight.data(), _p, _t ); }

    QString headLeft() { if ( m_headLeft.isNull() ) return ""; return m_headLeft.data(); }
    QString headMid() { if ( m_headMid.isNull() ) return ""; return m_headMid.data(); }
    QString headRight() { if ( m_headRight.isNull() ) return ""; return m_headRight.data(); }
    QString footLeft() { if ( m_footLeft.isNull() ) return ""; return m_footLeft.data(); }
    QString footMid() { if ( m_footMid.isNull() ) return ""; return m_footMid.data(); }
    QString footRight() { if ( m_footRight.isNull() ) return ""; return m_footRight.data(); }

    void setHeadFootLine( const char *_headl, const char *_headm, const char *_headr,
			  const char *_footl, const char *_footm, const char *_footr );

    /**
     * @return the URL of this part if it has been created from a file ( @ref #load )
     *         or if it has been saved to a file ( @ref #save ). Otherwise the
     *         return value is 0L.
     */
    const char* url() { return m_strFileURL.data(); }

    virtual void setModified( bool _c ) { m_bModified = _c; }

    KSpreadPythonModule *pythonModule() { return m_pPython; }
    void reloadScripts();
    
    bool editPythonCode();
    void endEditPythonCode();

    void undo();
    void redo();
    KSpreadUndo *undoBuffer() { return m_pUndoBuffer; }

    virtual void printMap( QPainter &_painter );

  // HACK
    void enableUndo( bool _b ) {}
  // HACK
    void enableRedo( bool _b ) {}
        
protected:
    void initPython();

    /**
     * Looks at @ref #m_paperFormat and calculates @ref #m_paperWidth and @ref #m_paperHeight.
     */
    void calcPaperSize();

    /**
     * Replaces macros like <name>, <file>, <date> etc. in the string and
     * returns the modified one.
     * 
     * @param _page is the page number for which the heading is produced.
     * @param _KSpreadTable is the name of the KSpreadTable for which we generate the headings.
     */
    QString completeHeading( const char *_data, int _page, const char *_KSpreadTable );

    KSpreadMap *m_pMap;
  
    /**
     * This variable is used to give every KSpreadTable a unique default name.
     *
     * @see #newKSpreadTable
     */
    int m_iTableId;

    /**
     * The orientation of the paper.
     */
    QPrinter::Orientation m_orientation;
    /**
     * Tells about the currently seleced paper size.
     */
    PaperFormat m_paperFormat;
    /**
     * The paper width in millimeters. Dont change this value, it is calculated by
     * @ref #calcPaperSize from the value @ref #m_paperFormat.
     */
    float m_paperWidth;
    /**
     * The paper height in millimeters. Dont change this value, it is calculated by
     * @ref #calcPaperSize from the value @ref #m_paperFormat.
     */
    float m_paperHeight;    
    /**
     * The left border in millimeters.
     */
    float m_leftBorder;
    /**
     * The right border in millimeters.
     */
    float m_rightBorder;
    /**
     * The top border in millimeters.
     */
    float m_topBorder;
    /**
     * The right border in millimeters.
     */
    float m_bottomBorder;

    QString m_headLeft;
    QString m_headRight;
    QString m_headMid;
    QString m_footLeft;
    QString m_footRight;
    QString m_footMid;
    
    /**
     * The URL of the this part. This variable is only set if the @ref #load function
     * had been called with an URL as argument.
     *
     * @see #load
     */
    QString m_strFileURL;

    /**
     * Indicates wether the user should save the document before deleting it.
     *
     * @see #isModified
     */
    bool m_bModified;

    KSpreadPythonModule *m_pPython;

    KMetaEditor *m_pEditor;
    /**
     * Used by @ref #editor
     */
    QString m_editorBuffer;

    KSpreadUndo *m_pUndoBuffer;

    QList<KSpreadView> m_lstViews;
};

#endif

