/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef __kpresenter_dlg_config__
#define __kpresenter_dlg_config__

#include <kdialogbase.h>

class KPresenterView;
class KPresenterDoc;
class KIntNumInput;
class KColorButton;
class KConfig;
class KSpellConfig;
class QCheckBox;
class KLineEdit;
class KDoubleNumInput;
class KoSpellConfigWidget;

class configureInterfacePage : public QWidget
{
    Q_OBJECT
public:
    configureInterfacePage( KPresenterView *_view, QWidget *parent = 0, char *name = 0 );
    void apply();
    void slotDefault();
private:
    KPresenterView* m_pView;
    KConfig* config;
    int oldNbRecentFiles;
    KDoubleNumInput* indent;
    KIntNumInput* recentFiles;
    QCheckBox *showRuler, *showStatusBar;
};

class configureColorBackground : public QWidget
{
    Q_OBJECT
public:
    configureColorBackground( KPresenterView* _view, QWidget *parent = 0, char *name = 0 );
    void apply();
    void slotDefault();
protected:
    KPresenterView* m_pView;
    KColorButton* bgColor;
    KColorButton* gridColor;
    KConfig* config;
    QColor oldBgColor;
    QColor oldGridColor;
} ;

class ConfigureSpellPage : public QObject
{
    Q_OBJECT
public:
    ConfigureSpellPage( KPresenterView *_view, QVBox *box, char *name = 0 );
    void apply();
    void slotDefault();
private:
    KPresenterView* m_pView;
    KoSpellConfigWidget *m_spellConfigWidget;
    KConfig* config;
};

class ConfigureMiscPage : public QObject
{
    Q_OBJECT
public:
    ConfigureMiscPage( KPresenterView  *_view, QVBox *box, char *name = 0 );
    KCommand *apply();
    void slotDefault();
private:
    KPresenterView* m_pView;
    KConfig* config;
    KIntNumInput* m_undoRedoLimit;
    int m_oldNbRedo;
    QCheckBox* m_displayLink, *m_displayComment, *m_underlineLink, *m_displayFieldCode;
    KDoubleNumInput * resolutionY, * resolutionX;
    KDoubleNumInput *m_rotation;
    QCheckBox *cbSideBarRefresh;
    bool m_oldRefreshSideBar;
};

class ConfigureDefaultDocPage : public QObject
{
    Q_OBJECT
public:
    ConfigureDefaultDocPage( KPresenterView  *_view, QVBox *box, char *name = 0 );
    ~ConfigureDefaultDocPage();
    KCommand *apply();
    void slotDefault();
public slots:
    void selectNewDefaultFont();
private:
    int oldAutoSaveValue;
    int m_oldStartingPage;
    double m_oldTabStopWidth;
    bool m_oldBackupFile;

    KPresenterView* m_pView;
    KConfig* config;
    QFont *font;
    QLabel *fontName;
    KIntNumInput* autoSave;
    KIntNumInput* m_variableNumberOffset;
    KDoubleNumInput *m_tabStopWidth;
    QCheckBox *m_cursorInProtectedArea;
    QCheckBox *m_createBackupFile;
    QCheckBox *m_directInsertCursor;
};

class ConfigureToolsPage : public QObject
{
    Q_OBJECT
public:
    ConfigureToolsPage( KPresenterView  *_view, QVBox *box, char *name = 0 );
    ~ConfigureToolsPage();
    void apply();
    void slotDefault();
public slots:

private:
    KPresenterView* m_pView;
    KConfig* config;

    ConfPenDia *m_confPenDia;
    ConfPieDia *m_confPieDia;
    ConfRectDia *m_confRectDia;
    ConfBrushDia *m_confBrushDia;
    ConfPolygonDia *m_confPolygonDia;
};

class ConfigurePathPage : public QObject
{
    Q_OBJECT
public:
    ConfigurePathPage( KPresenterView *_view, QVBox *box, char *name = 0 );
    void slotDefault();
    void apply();
private slots:
    void slotModifyPath();
    void slotSelectionChanged(QListViewItem *);
private:
    KPresenterView* m_pView;
    KConfig* config;
    KListView* m_pPathView;
    QPushButton *m_modifyPath;
};


class KPConfig : public KDialogBase
{
    Q_OBJECT
public:
    enum { KP_INTERFACE = 1, KP_COLOR=2, KP_KSPELL=4,KP_MISC=8, KP_DOC=16, KP_TOOLS=32,KP_PATH = 64 };
    KPConfig( KPresenterView* parent );
public slots:
    void slotApply();
    void slotDefault();
    void openPage(int flags);
private:
    configureInterfacePage *_interfacePage;
    configureColorBackground *_colorBackground;
    ConfigureSpellPage *_spellPage;
    ConfigureMiscPage *_miscPage;
    ConfigureDefaultDocPage *_defaultDocPage;
    ConfigureToolsPage *_toolsPage;
    ConfigurePathPage *m_pathPage;
    KPresenterDoc* m_doc;

};

#endif
