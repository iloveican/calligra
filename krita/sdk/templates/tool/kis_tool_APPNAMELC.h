#ifndef _KIS_TOOL_%{APPNAMEUC}_H_
#define _KIS_TOOL_%{APPNAMEUC}_H_

#include <kis_tool.h>
#include <KoToolFactoryBase.h>

class KisCanvas2;

class KisTool%{APPNAME} : public KisTool {
    Q_OBJECT
public:
    KisTool%{APPNAME}(KoCanvasBase * canvas);
    virtual ~KisTool%{APPNAME}();

    //
    // KisToolPaint interface
    //

    virtual quint32 priority() { return 3; }
    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent *event);
    virtual void mouseReleaseEvent(KoPointerEvent *event);

// Uncomment if you have a configuration widget
//     QWidget* createOptionWidget();

public slots:
    virtual void activate(bool temp = false);
    void deactivate();

protected:
    
    virtual void paint(QPainter& gc, const KoViewConverter &converter);

protected:
    KisCanvas2* m_canvas;
};


class KisTool%{APPNAME}Factory : public KoToolFactoryBase {

public:
    KisTool%{APPNAME}Factory(QObject *parent)
        : KoToolFactoryBase(parent, "KisTool%{APPNAME}", i18n( "%{APPNAME}" ))
        {
            setToolTip( i18n( "%{APPNAME}" ) );
            setToolType( TOOL_TYPE_VIEW );
            setIcon( "tool_%{APPNAMELC}" );
            setPriority( 0 );
        };


    virtual ~KisTool%{APPNAME}Factory() {}

    virtual KoToolBase * createTool(KoCanvasBase * canvas) {
        return new KisTool%{APPNAME}(canvas);
    }

};


#endif

