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


#include "kostyle.h"
#include "koStylist.h"
#include "koStylist.moc"
#include <koFontDia.h>

#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcombobox.h>

#include <klocale.h>
//#include <kotextdocument.h>
#include <kiconloader.h>
#include <kdebug.h>
//#include "kotextparag.h"
#include "kozoomhandler.h"
#include <koGlobal.h>
#include <qcheckbox.h>
#include <qlayout.h>

/******************************************************************/
/* Class: KoStyleManager                                          */
/******************************************************************/

/* keep 2 qlists with the styles.
   1 of the origs, another with the changed ones (in order of the stylesList)
   When an orig entry is empty and the other is not, a new one has to be made,
   when the orig is present and the other is not, the orig has to be deleted.
   Otherwise all changes are copied from the changed ones to the origs on OK.
   OK updates the doc if styles are deleted.
   The dtor frees all the changed ones.
*/
/* Months later the above seems SOO stupid.. Just should have created a small class
   containing the orig and the copy and an enum plus some simple methods..
   Well; just keep that for those loonly uninspiring days :) (Thomas Z)
*/
class KoStyleManagerPrivate
{
public:
    KoStylePreview* preview;
    QCheckBox* cbIncludeInTOC;
};

KoStyleManager::KoStyleManager( QWidget *_parent,KoUnit::Unit unit, const QPtrList<KoParagStyle> & style, const QString & activeStyleName)
    : KDialogBase( _parent, "Stylist", true,
                   i18n("Style Manager"),
                   KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Apply )
{
    d = new KoStyleManagerPrivate;
    //setWFlags(getWFlags() || WDestructiveClose);
    m_currentStyle =0L;
    noSignals=true;
    m_origStyles.setAutoDelete(false);
    m_changedStyles.setAutoDelete(false);
    setupWidget(style); // build the widget with the buttons and the list selector.
    addGeneralTab();
    KoStyleFontTab * fontTab = new KoStyleFontTab( m_tabs );
    addTab( fontTab );

    KoStyleParagTab *newTab = new KoStyleParagTab( m_tabs );
    newTab->setWidget( new KoIndentSpacingWidget( unit, true,-1/*no limit*/,newTab ) );
    addTab( newTab );

    newTab = new KoStyleParagTab( m_tabs );
    newTab->setWidget( new KoParagAlignWidget( newTab ) );
    addTab( newTab );

    newTab = new KoStyleParagTab( m_tabs );
    newTab->setWidget( new KoParagBorderWidget( newTab ) );
    addTab( newTab );

    newTab = new KoStyleParagTab( m_tabs );
    newTab->setWidget( new KoParagCounterWidget( false , newTab ) );
    addTab( newTab );

    newTab = new KoStyleParagTab( m_tabs );
    newTab->setWidget( new KoParagTabulatorsWidget( unit, -1, newTab ) );
    addTab( newTab );

    QListBoxItem * item = m_stylesList->findItem (activeStyleName);
    if ( item )
        m_stylesList->setCurrentItem( m_stylesList->index(item) );
    else
        m_stylesList->setCurrentItem( 0 );

    noSignals=false;
    switchStyle();
    setInitialSize( QSize( 600, 570 ) );
}

KoStyleManager::~KoStyleManager()
{
    for (unsigned int i =0 ; m_origStyles.count() > i ; i++) {
        KoParagStyle *orig = m_origStyles.at(i);
        KoParagStyle *changed = m_changedStyles.at(i);
        if( orig && changed && orig != changed ) // modified style, we can delete the changed one now that changes have been applied
            delete changed;
    }

    delete d;
}

void KoStyleManager::addTab( KoStyleManagerTab * tab )
{
    m_tabsList.append( tab );
    m_tabs->insertTab( tab, tab->tabName() );
    tab->layout()->activate();
}

void KoStyleManager::setupWidget(const QPtrList<KoParagStyle> & styleList)
{
    QFrame * frame1 = makeMainWidget();
    QGridLayout *frame1Layout = new QGridLayout( frame1, 0, 0, // auto
                                                 KDialog::marginHint(), KDialog::spacingHint() );
    QPtrListIterator<KoParagStyle> style( styleList );
    numStyles = styleList.count();
    m_stylesList = new QListBox( frame1, "stylesList" );
    for ( ; style.current() ; ++style )
    {
        m_stylesList->insertItem( style.current()->translatedName() );
        m_origStyles.append( style.current() );
        m_changedStyles.append( style.current() );
        m_styleOrder<< style.current()->name();
    }

    frame1Layout->addMultiCellWidget( m_stylesList, 0, 0, 0, 1 );


    m_moveUpButton = new QPushButton( frame1, "moveUpButton" );
    m_moveUpButton->setPixmap( BarIcon( "up", KIcon::SizeSmall ) );
    connect( m_moveUpButton, SIGNAL( clicked() ), this, SLOT( moveUpStyle() ) );
    frame1Layout->addWidget( m_moveUpButton, 1, 1 );

    m_moveDownButton = new QPushButton( frame1, "moveDownButton" );
    m_moveDownButton->setPixmap( BarIcon( "down", KIcon::SizeSmall ) );
    connect( m_moveDownButton, SIGNAL( clicked() ), this, SLOT( moveDownStyle() ) );
    frame1Layout->addWidget( m_moveDownButton, 1, 0 );


    m_deleteButton = new QPushButton( frame1, "deleteButton" );
    m_deleteButton->setText( i18n( "&Delete" ) );
    connect( m_deleteButton, SIGNAL( clicked() ), this, SLOT( deleteStyle() ) );

    frame1Layout->addWidget( m_deleteButton, 2, 1 );

    m_newButton = new QPushButton( frame1, "newButton" );
    m_newButton->setText( i18n( "New" ) );
    connect( m_newButton, SIGNAL( clicked() ), this, SLOT( addStyle() ) );

    frame1Layout->addWidget( m_newButton, 2, 0 );

    m_tabs = new QTabWidget( frame1 );
    frame1Layout->addMultiCellWidget( m_tabs, 0, 2, 2, 2 );

    connect( m_stylesList, SIGNAL( selectionChanged() ), this, SLOT( switchStyle() ) );
    connect( m_tabs, SIGNAL( currentChanged ( QWidget * ) ), this, SLOT( switchTabs() ) );
}

void KoStyleManager::addGeneralTab() {
    QWidget *tab = new QWidget( m_tabs );

    QGridLayout *tabLayout = new QGridLayout( tab );
    tabLayout->setSpacing( KDialog::spacingHint() );
    tabLayout->setMargin( KDialog::marginHint() );

    m_nameString = new QLineEdit( tab );
    m_nameString->resize(m_nameString->sizeHint() );
    connect( m_nameString, SIGNAL( textChanged( const QString &) ), this, SLOT( renameStyle(const QString &) ) );

    tabLayout->addWidget( m_nameString, 0, 1 );

    QLabel *nameLabel = new QLabel( tab );
    nameLabel->setText( i18n( "Name:" ) );
    nameLabel->resize(nameLabel->sizeHint());
    nameLabel->setAlignment( AlignRight | AlignVCenter );

    tabLayout->addWidget( nameLabel, 0, 0 );

    m_styleCombo = new QComboBox( FALSE, tab, "styleCombo" );

    tabLayout->addWidget( m_styleCombo, 1, 1 );

    QLabel *nextStyleLabel = new QLabel( tab );
    nextStyleLabel->setText( i18n( "Next style:" ) );
    nextStyleLabel->setAlignment( AlignRight | AlignVCenter );

    tabLayout->addWidget( nextStyleLabel, 1, 0 );

    m_inheritCombo = new QComboBox( FALSE, tab, "inheritCombo" );
    tabLayout->addWidget( m_inheritCombo, 2, 1 );

    QLabel *inheritStyleLabel = new QLabel( tab );
    inheritStyleLabel->setText( i18n( "Inherit style:" ) );
    inheritStyleLabel->setAlignment( AlignRight | AlignVCenter );

    tabLayout->addWidget( inheritStyleLabel, 2, 0 );

    d->cbIncludeInTOC = new QCheckBox( i18n("Include in table of contents"), tab );
    tabLayout->addMultiCellWidget( d->cbIncludeInTOC, 3, 3, 0, 1 );

    d->preview = new KoStylePreview( i18n( "Preview" ), i18n( "The quick brown fox jumps over the lazy dog" ), tab, "stylepreview" );

    tabLayout->addMultiCellWidget( d->preview, 4, 4, 0, 1 );

    m_tabs->insertTab( tab, i18n( "General" ) );

    m_inheritCombo->insertItem( i18n("<None>"));

    for ( unsigned int i = 0; i < m_stylesList->count(); i++ ) {
        m_styleCombo->insertItem( m_stylesList->text(i));
        m_inheritCombo->insertItem( m_stylesList->text(i));
    }

}

void KoStyleManager::switchStyle() {
    kdDebug(32500) << "KoStyleManager::switchStyle noSignals=" << noSignals << endl;
    if(noSignals) return;
    noSignals=true;

    if(m_currentStyle !=0L)
        save();

    m_currentStyle = 0L;
    int num = styleIndex( m_stylesList->currentItem() );
    kdDebug(32500) << "KoStyleManager::switchStyle switching to " << num << endl;
    if(m_origStyles.at(num) == m_changedStyles.at(num)) {
        m_currentStyle = new KoParagStyle( *m_origStyles.at(num) );
        m_changedStyles.take(num);
        m_changedStyles.insert(num, m_currentStyle);
    } else {
        m_currentStyle = m_changedStyles.at(num);
    }
    updateGUI();

    noSignals=false;
}

void KoStyleManager::switchTabs()
{
    // Called when the user switches tabs
    // We call save() to update our style, for the preview on the 1st tab
    save();
    updatePreview();
}

// Return the index of the a style from its position in the GUI
// (e.g. in m_stylesList or m_styleCombo). This index is used in
// the m_origStyles and m_changedStyles lists.
// The reason for the difference is that a deleted style is removed
// from the GUI but not from the internal lists.
int KoStyleManager::styleIndex( int pos ) {
    int p = 0;
    for(unsigned int i=0; i < m_changedStyles.count(); i++) {
        // Skip deleted styles, they're no in m_stylesList anymore
        KoParagStyle * style = m_changedStyles.at(i);
        if ( !style ) continue;
        if ( p == pos )
            return i;
        ++p;
    }
    kdWarning() << "KoStyleManager::styleIndex no style found at pos " << pos << endl;

#ifdef __GNUC_
#warning implement undo/redo
#endif

    return 0;
}

// Update the GUI so that it shows m_currentStyle
void KoStyleManager::updateGUI() {
    kdDebug(32500) << "KoStyleManager::updateGUI m_currentStyle=" << m_currentStyle << " " << m_currentStyle->name() << endl;
    QPtrListIterator<KoStyleManagerTab> it( m_tabsList );
    for ( ; it.current() ; ++it )
    {
        it.current()->setStyle( m_currentStyle );
        it.current()->update();
    }

    m_nameString->setText(m_currentStyle->translatedName());

    QString followingName = m_currentStyle->followingStyle() ? m_currentStyle->followingStyle()->translatedName() : QString::null;
    kdDebug(32500) << "KoStyleManager::updateGUI updating combo to " << followingName << endl;
    for ( int i = 0; i < m_styleCombo->count(); i++ ) {
        if ( m_styleCombo->text( i ) == followingName ) {
            m_styleCombo->setCurrentItem( i );
            kdDebug(32500) << "found at " << i << endl;
            break;
        }
    }

    QString inheritName = m_currentStyle->parentStyle() ? m_currentStyle->parentStyle()->translatedName() : QString::null;
    kdDebug(32500) << "KoStyleManager::updateGUI updating combo to " << inheritName << endl;
    for ( int i = 0; i < m_inheritCombo->count(); i++ ) {
        if ( m_inheritCombo->text( i ) == inheritName ) {
            m_inheritCombo->setCurrentItem( i );
            kdDebug(32500) << "found at " << i << endl;
            break;
        }
        else
            m_inheritCombo->setCurrentItem( 0 );//none !!!
    }

    d->cbIncludeInTOC->setChecked( m_currentStyle->isOutline() );

    // update delete button (can't delete first style);
    m_deleteButton->setEnabled(m_stylesList->currentItem() != 0);

    m_moveUpButton->setEnabled(m_stylesList->currentItem() != 0);
    m_moveDownButton->setEnabled(m_stylesList->currentItem()!=(int)m_stylesList->count()-1);

    updatePreview();
}

void KoStyleManager::updatePreview()
{
    d->preview->setStyle(m_currentStyle);
    d->preview->repaint(true);
}

void KoStyleManager::save() {
    if(m_currentStyle) {
        // save changes from UI to object.
        QPtrListIterator<KoStyleManagerTab> it( m_tabsList );
        for ( ; it.current() ; ++it )
            it.current()->save();

	// Rename the style - only if it's actually been renamed.
	// Take care of not renaming a style from its English name to its translated name by mistake,
	// this would break the TOC stuff.
        if ( m_currentStyle->name() != m_nameString->text() &&
            m_currentStyle->translatedName() != m_nameString->text() )
        {
            m_currentStyle->setName( m_nameString->text() );
        }

        int indexNextStyle = styleIndex( m_styleCombo->currentItem() );
        m_currentStyle->setFollowingStyle( m_origStyles.at( indexNextStyle ) ); // point to orig, not changed! (#47377)
        m_currentStyle->setParentStyle( style( m_inheritCombo->currentText() ) );
        m_currentStyle->setOutline( d->cbIncludeInTOC->isChecked() );
    }
}

KoParagStyle * KoStyleManager::style( const QString & _name )
{
    for(unsigned int i=0; i < m_changedStyles.count(); i++) {
        // Skip deleted styles, they're no in m_stylesList anymore
        KoParagStyle * style = m_changedStyles.at(i);
        if ( !style ) continue;
        if ( style->name() == _name)
            return style;
    }
    return 0;
}

void KoStyleManager::addStyle() {
    save();

    QString str = i18n( "New Style Template (%1)" ).arg(numStyles++);
    if ( m_currentStyle )
    {
        m_currentStyle = new KoParagStyle( *m_currentStyle ); // Create a new style, initializing from the current one
        m_currentStyle->setName( str );
    }
    else
        m_currentStyle = new KoParagStyle( str );
    m_currentStyle->setFollowingStyle( m_currentStyle ); // #45868

    noSignals=true;
    m_origStyles.append(0L);
    m_changedStyles.append(m_currentStyle);
    m_stylesList->insertItem( str );
    m_styleCombo->insertItem( str );
    m_inheritCombo->insertItem( str );
    m_stylesList->setCurrentItem( m_stylesList->count() - 1 );
    noSignals=false;
    m_styleOrder<< str;

    updateGUI();
}

void KoStyleManager::updateFollowingStyle( KoParagStyle *s )
{
    for ( KoParagStyle* p = m_changedStyles.first(); p != 0L; p = m_changedStyles.next() )
    {
        if ( p->followingStyle() == s)
            p->setFollowingStyle(p);
    }

}

void KoStyleManager::updateInheritStyle( KoParagStyle *s )
{
    for ( KoParagStyle* p = m_changedStyles.first(); p != 0L; p = m_changedStyles.next() )
    {
        //when we remove style, we must replace inherite style to 0L
        //when parent style was removed.
        //##########Laurent change inherited style attribute
        if ( p->parentStyle() == s)
            p->setParentStyle(0L);
    }

}

void KoStyleManager::deleteStyle() {

    unsigned int cur = styleIndex( m_stylesList->currentItem() );
    unsigned int curItem = m_stylesList->currentItem();
    QString name = m_stylesList->currentText();
    KoParagStyle *s = m_changedStyles.at(cur);
    m_styleOrder.remove( s->name());
    updateFollowingStyle( s );
    updateInheritStyle( s );
    Q_ASSERT( s == m_currentStyle );
    delete s;
    m_currentStyle = 0L;
    m_changedStyles.remove(cur);
    m_changedStyles.insert(cur,0L);

    // Done with noSignals still false, so that when m_stylesList changes the current item
    // we display it automatically
    m_stylesList->removeItem(curItem);
    m_styleCombo->removeItem(curItem);

    m_inheritCombo->listBox()->removeItem( m_inheritCombo->listBox()->index(m_inheritCombo->listBox()->findItem(name )));

    numStyles--;
    m_stylesList->setSelected( m_stylesList->currentItem(), true );
}

void KoStyleManager::moveUpStyle()
{
    if ( m_currentStyle )
        save();
    unsigned int pos = 0;
    QString currentStyleName=m_stylesList->currentText ();

    int pos2 = m_styleOrder.findIndex( currentStyleName );
    if ( pos2 != -1 )
    {
        m_styleOrder.remove( m_styleOrder.at(pos2));
        m_styleOrder.insert( m_styleOrder.at(pos2-1), currentStyleName);
    }

    pos=m_stylesList->currentItem();
    noSignals=true;
    m_stylesList->changeItem( m_stylesList->text( pos-1 ), pos );
    m_styleCombo->changeItem( m_stylesList->text( pos-1 ), pos );

    m_stylesList->changeItem( currentStyleName, pos-1 );
    m_styleCombo->changeItem( currentStyleName, pos-1 );

    m_stylesList->setCurrentItem( m_stylesList->currentItem() );
    noSignals=false;

    updateGUI();
}

void KoStyleManager::moveDownStyle()
{
    if ( m_currentStyle )
        save();
    unsigned int pos = 0;
    QString currentStyleName=m_stylesList->currentText ();
    int pos2 = m_styleOrder.findIndex( currentStyleName );
    if ( pos2 != -1 )
    {
        m_styleOrder.remove( m_styleOrder.at(pos2));
        m_styleOrder.insert( m_styleOrder.at(pos2+1), currentStyleName);
    }


    pos=m_stylesList->currentItem();
    noSignals=true;
    m_stylesList->changeItem( m_stylesList->text ( pos+1 ),pos);
    m_styleCombo->changeItem( m_stylesList->text ( pos+1 ),pos);
    m_stylesList->changeItem( currentStyleName ,pos+1);
    m_styleCombo->changeItem( currentStyleName ,pos+1);
    m_stylesList->setCurrentItem( m_stylesList->currentItem() );
    noSignals=false;

    updateGUI();
}

void KoStyleManager::slotOk() {
    save();
    apply();
    KDialogBase::slotOk();
}

void KoStyleManager::slotApply() {
    save();
    apply();
    KDialogBase::slotApply();
}

void KoStyleManager::apply() {
    noSignals=true;
    KoStyleChangeDefMap styleChanged;
    QPtrList<KoParagStyle> removeStyle;
    for (unsigned int i =0 ; m_origStyles.count() > i ; i++) {
        if(m_origStyles.at(i) == 0L && m_changedStyles.at(i)!=0L) {           // newly added style
            kdDebug(32500) << "adding new " << m_changedStyles.at(i)->name() << " (" << i << ")" << endl;
            KoParagStyle *tmp = addStyleTemplate(m_changedStyles.take(i));
            m_changedStyles.insert(i, tmp);
        } else if(m_changedStyles.at(i) == 0L && m_origStyles.at(i) != 0L) { // deleted style
            kdDebug(32500) << "deleting orig " << m_origStyles.at(i)->name() << " (" << i << ")" << endl;

            KoParagStyle *orig = m_origStyles.at(i);
            //applyStyleChange( orig, -1, -1 );
            KoStyleChangeDef tmp( -1,-1);
            styleChanged.insert( orig, tmp);

            removeStyle.append( orig );
            // Note that the style is never deleted (we'll need it for undo/redo purposes)

        } else if(m_changedStyles.at(i) != 0L && m_origStyles.at(i)!=0L) { // simply updated style
            kdDebug(32500) << "update style " << m_changedStyles.at(i)->name() << " (" << i << ")" << endl;
            KoParagStyle *orig = m_origStyles.at(i);
            KoParagStyle *changed = m_changedStyles.at(i);
            if ( orig != changed )
            {
                int paragLayoutChanged = orig->paragLayout().compare( changed->paragLayout() );
                int formatChanged = orig->format().compare( changed->format() );
                //kdDebug(32500) << "old format " << orig->format().key() << " pointsize " << orig->format().pointSizeFloat() << endl;
                //kdDebug(32500) << "new format " << changed->format().key() << " pointsize " << changed->format().pointSizeFloat() << endl;

                // Copy everything from changed to orig
                *orig = *changed;

                // Apply the change selectively - i.e. only what changed
                //applyStyleChange( orig, paragLayoutChanged, formatChanged );
                if ( formatChanged != 0 || paragLayoutChanged != 0 ) {
                    KoStyleChangeDef tmp(paragLayoutChanged, formatChanged);
                    styleChanged.insert( orig, tmp );
                }

            }

        }// else
         //     kdDebug(32500) << "has not changed " <<  m_changedStyles.at(i)->name() << " (" << i << ")" <<  endl;
    }

    applyStyleChange( styleChanged );

    KoParagStyle *tmp = 0L;
    for ( tmp = removeStyle.first(); tmp ;tmp = removeStyle.next() )
        removeStyleTemplate( tmp );

    updateStyleListOrder( m_styleOrder);
    updateAllStyleLists();
    noSignals=false;
}

void KoStyleManager::renameStyle(const QString &theText) {
    if(noSignals) return;
    noSignals=true;

    int index = m_stylesList->currentItem();
    kdDebug(32500) << "KoStyleManager::renameStyle " << index << " to " << theText << endl;

    // rename only in the GUI, not even in the underlying objects (save() does it).
    kdDebug(32500) << "KoStyleManager::renameStyle before " << m_styleCombo->currentText() << endl;
    m_styleCombo->changeItem( theText, index );
    m_inheritCombo->changeItem( theText, index+1 );
    m_styleOrder[index]=theText;
    kdDebug(32500) << "KoStyleManager::renameStyle after " << m_styleCombo->currentText() << endl;
    m_stylesList->changeItem( theText, index );

    // Check how many styles with that name we have now
    int synonyms = 0;
    for ( int i = 0; i < m_styleCombo->count(); i++ ) {
        if ( m_styleCombo->text( i ) == m_stylesList->currentText() )
            ++synonyms;
    }
    Q_ASSERT( synonyms > 0 ); // should have found 'index' at least !
    noSignals=false;
    // Can't close the dialog if two styles have the same name
    bool state=!theText.isEmpty() && (synonyms == 1);
    enableButtonOK(state );
    enableButtonApply(state);
    m_deleteButton->setEnabled(state&&(m_stylesList->currentItem() != 0));
    m_newButton->setEnabled(state);
    m_stylesList->setEnabled( state );
    if ( state )
    {
        m_moveUpButton->setEnabled(m_stylesList->currentItem() != 0);
        m_moveDownButton->setEnabled(m_stylesList->currentItem()!=(int)m_stylesList->count()-1);
    }
    else
    {
        m_moveUpButton->setEnabled(false);
        m_moveDownButton->setEnabled(false);
    }
}

/////////////

KoStyleParagTab::KoStyleParagTab( QWidget * parent )
    : KoStyleManagerTab( parent )
{
    ( new QVBoxLayout( this ) )->setAutoAdd( true );
    m_widget = 0L;
}

void KoStyleParagTab::update()
{
     m_widget->display( m_style->paragLayout() );
}

void KoStyleParagTab::save()
{
     m_widget->save( m_style->paragLayout() );
}

void KoStyleParagTab::setWidget( KoParagLayoutWidget * widget )
{
    m_widget = widget;
}

void KoStyleParagTab::resizeEvent( QResizeEvent *e )
{
    QWidget::resizeEvent( e );
    if ( m_widget ) m_widget->resize( size() );
}

KoStyleFontTab::KoStyleFontTab( QWidget * parent )
    : KoStyleManagerTab( parent )
{
    ( new QVBoxLayout( this ) )->setAutoAdd( true );
    m_chooser = new KoFontChooser( this, 0, true, KFontChooser::SmoothScalableFonts);
    m_zoomHandler = new KoZoomHandler;
}

KoStyleFontTab::~KoStyleFontTab()
{
    delete m_zoomHandler;
}

void KoStyleFontTab::update()
{
    m_chooser->setFormat( m_style->format() );

#if 0
    bool subScript = m_style->format().vAlign() == KoTextFormat::AlignSubScript;
    bool superScript = m_style->format().vAlign() == KoTextFormat::AlignSuperScript;
    QFont fn = m_style->format().font();
    kdDebug()<<" fn.bold() :"<<fn.bold()<<" fn.italic():"<<fn.italic()<<endl;
    kdDebug()<<" fn.family() :"<<fn.family()<<endl;
    m_chooser->setFont( fn, subScript, superScript );
    m_chooser->setColor( m_style->format().color() );
    QColor col=m_style->format().textBackgroundColor();
    col=col.isValid() ? col : QApplication::palette().color( QPalette::Active, QColorGroup::Base );
    m_chooser->setBackGroundColor(col);

    m_chooser->setUnderlineColor( m_style->format().textUnderlineColor());

    m_chooser->setUnderlineType(m_style->format().underlineType());
    m_chooser->setUnderlineStyle(m_style->format().underlineStyle());
    m_chooser->setStrikeOutStyle(m_style->format().strikeOutStyle());
    m_chooser->setStrikeOutlineType(m_style->format().strikeOutType());
    m_chooser->setShadowText( m_style->format().shadowText());
    m_chooser->setFontAttribute( m_style->format().attributeFont());
    m_chooser->setWordByWord( m_style->format().wordByWord());
    m_chooser->setRelativeTextSize( m_style->format().relativeTextSize());
    m_chooser->setOffsetFromBaseLine( m_style->format().offsetFromBaseLine());
    m_chooser->setLanguage( m_style->format().language());
    m_chooser->setHyphenation( m_style->format().hyphenation());
#endif
}

void KoStyleFontTab::save()
{
    m_style->format() = m_chooser->newFormat();
#if 0
    QFont fn = m_chooser->getNewFont();
    kdDebug()<<" save fn.bold() :"<<fn.bold()<<" fn.italic():"<<fn.italic()<<endl;
    kdDebug()<<" save fn.family() :"<<fn.family()<<endl;

    m_style->format().setFont( fn );
    if ( m_chooser->getSubScript() )
        m_style->format().setVAlign( KoTextFormat::AlignSubScript );
    else if ( m_chooser->getSuperScript() )
        m_style->format().setVAlign( KoTextFormat::AlignSuperScript );
    else
        m_style->format().setVAlign( KoTextFormat::AlignNormal );
    m_style->format().setColor( m_chooser->color() );
    if(m_chooser->backGroundColor()!=QApplication::palette().color( QPalette::Active, QColorGroup::Base ))
        m_style->format().setTextBackgroundColor(m_chooser->backGroundColor());

    m_style->format().setTextUnderlineColor(m_chooser->underlineColor());
    m_style->format().setUnderlineType (m_chooser->getUnderlineType());
    m_style->format().setUnderlineStyle (m_chooser->getUnderlineStyle());
    m_style->format().setStrikeOutStyle( m_chooser->getStrikeOutStyle() );
    m_style->format().setStrikeOutType (m_chooser->getStrikeOutType());
    m_style->format().setShadowText(m_chooser->getShadowText());
    m_style->format().setWordByWord( m_chooser->getWordByWord());
    m_style->format().setRelativeTextSize( m_chooser->getRelativeTextSize());
    m_style->format().setAttributeFont( m_chooser->getFontAttribute());
    m_style->format().setOffsetFromBaseLine( m_chooser->getOffsetFromBaseLine());

    m_style->format().setLanguage( m_chooser->getLanguage());
    m_style->format().setHyphenation( m_chooser->getHyphenation());
#endif
}

QString KoStyleFontTab::tabName()
{
    return i18n("Font");
}

void KoStyleFontTab::resizeEvent( QResizeEvent *e )
{
    QWidget::resizeEvent( e );
    if ( m_chooser ) m_chooser->resize( size() );
}
