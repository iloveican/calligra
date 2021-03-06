/* This file is part of the KDE project
   Copyright (C) 2005-2011 Jarosław Staniek <staniek@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#include "kexiformmanager.h"
#include "widgets/kexidbform.h"
#include "widgets/kexidbautofield.h"
#include "kexiformscrollview.h"
#include "kexiformview.h"
#include "kexidatasourcepage.h"

#include <QToolButton>
#include <KAction>
#include <KToggleAction>
#include <KActionCollection>
#include <KPageDialog>
#include <KTextEdit>
#include <KToolBar>

//2.0 #include <formeditor/formmanager.h>
//2.0 #include <formeditor/widgetpropertyset.h>
#include <formeditor/form.h>
#include <formeditor/widgetlibrary.h>
#include <formeditor/commands.h>
#include <formeditor/objecttree.h>
#include <formeditor/formIO.h>
#include <formeditor/kexiactionselectiondialog.h>
#include <formeditor/WidgetTreeWidget.h>

#include <koproperty/Set.h>
#include <koproperty/Property.h>
#include <widget/properties/KexiCustomPropertyFactory.h>
#include <core/KexiMainWindowIface.h>
#include <kexiutils/SmallToolButton.h>

class KexiFormManagerPrivate {
public:
    KexiFormManagerPrivate() : part(0)
        , q(this)
    {
        features = KFormDesigner::Form::NoFeatures;
        widgetActionGroup = new KFormDesigner::ActionGroup(&q);
#ifdef KFD_SIGSLOTS
        dragConnectionAction = 0;
#endif
        widgetTree = 0;
        collection = 0;
    }
    ~KexiFormManagerPrivate() {
    }
    KexiFormPart* part;
    KFormDesigner::WidgetLibrary* lib;
    KFormDesigner::ActionGroup* widgetActionGroup;
    KFormDesigner::WidgetTreeWidget *widgetTree;
    KActionCollection  *collection;
    KFormDesigner::Form::Features features;
    KToggleAction *pointerAction;
#ifdef KFD_SIGSLOTS
    KToggleAction *dragConnectionAction;
#endif
    KToggleAction *snapToGridAction;

    KexiFormManager q;
};

K_GLOBAL_STATIC(KexiFormManagerPrivate, g_private)

KexiFormManager* KexiFormManager::self()
{
    return &g_private->q;
}

KexiFormManager::KexiFormManager(KexiFormManagerPrivate *p)
        : QObject()
        , d( p )
{
//2.0 unused    m_emitSelectionSignalsUpdatesPropertySet = true;
    KexiCustomPropertyFactory::init();
}

KexiFormManager::~KexiFormManager()
{
}

void KexiFormManager::init(KexiFormPart *part, KFormDesigner::WidgetTreeWidget *widgetTree)
{
/* @todo add configuration for supported factory groups */
    QStringList supportedFactoryGroups;
    supportedFactoryGroups += "kexi";
    d->lib = new KFormDesigner::WidgetLibrary(this, supportedFactoryGroups);
    d->lib->setAdvancedPropertiesVisible(false);

    connect(d->lib, SIGNAL(widgetCreated(QWidget*)),
            this, SLOT(slotWidgetCreatedByFormsLibrary(QWidget*)));
    connect(d->lib, SIGNAL(widgetActionToggled(const QByteArray&)),
        this, SLOT(slotWidgetActionToggled(const QByteArray&)));

    d->part = part;
    KActionCollection *col = /*tmp*/ new KActionCollection(this); // 2.0 d->part->actionCollectionForMode(Kexi::DesignViewMode);
    if (col) {
        createActions( col );

    //connect actions provided by widget factories
//moved from KexiFormPart
        connect(col->action("widget_assign_action"), SIGNAL(activated()),
                this, SLOT(slotAssignAction()));
    }

    d->widgetTree = widgetTree;
    if (d->widgetTree) {
#ifdef __GNUC__
#warning "Port this: connect()"
#else
#pragma WARNING( Port this: connect() )
#endif
#ifdef __GNUC__
#warning "Port code related to KFormDesigner::FormManager::m_treeview here"
#else
#pragma WARNING( Port code related to KFormDesigner::FormManager::m_treeview here )
#endif
//todo        connect(m_propSet, SIGNAL(widgetNameChanged(const QByteArray&, const QByteArray&)),
//todo                m_treeview, SLOT(renameItem(const QByteArray&, const QByteArray&)));
    }
}

//moved from KFormDesigner::FormManager
KFormDesigner::ActionGroup* KexiFormManager::widgetActionGroup() const
{
    return d->widgetActionGroup;
}

//moved from KFormDesigner::FormManager
void KexiFormManager::createActions(KActionCollection* collection)
{
    d->collection = collection;
//    KXMLGUIClient* client = (KXMLGUIClient*)d->collection->parentGUIClient();

    d->lib->createWidgetActions(d->widgetActionGroup);
//! @todo insertWidget() slot?
//2.0    d->lib->createWidgetActions(client, d->collection,
//2.0                                this, SLOT(insertWidget(const QByteArray &)));

#ifdef KFD_SIGSLOTS
    if (d->features & KFormDesigner::Form::EnableConnections) {
        // nothing
    }
    else {
        d->dragConnectionAction = new KToggleAction(
            KIcon("signalslot"), i18n("Connect Signals/Slots"), d->collection);
        d->dragConnectionAction->setObjectName("drag_connection");
//        d->widgetActionGroup->addAction(d->dragConnectionAction);
        connect(d->dragConnectionAction, SIGNAL(triggered()),
                this, SLOT(startCreatingConnection()));
        d->dragConnectionAction->setChecked(false);
    }
#endif

    d->pointerAction = new KToggleAction(
        KIcon("mouse_pointer"), i18n("Pointer"), d->collection);
    d->pointerAction->setObjectName("edit_pointer");
    d->widgetActionGroup->addAction(d->pointerAction);
    connect(d->pointerAction, SIGNAL(triggered()),
            this, SLOT(slotPointerClicked()));
    d->pointerAction->setChecked(true);

    d->snapToGridAction = new KToggleAction(
        i18n("Snap to Grid"), d->collection);
    d->snapToGridAction->setObjectName("snap_to_grid");
//    d->widgetActionGroup->addAction(d->snapToGridAction);
//    d->snapToGridAction->setChecked(true);

#if 0 // 2.0: todo
    // Create the Style selection action (with a combo box in toolbar and submenu items)
    KSelectAction *styleAction = new KSelectAction(
        i18n("Style"), d->collection);
    styleAction->setObjectName("change_style");
    connect(styleAction, SIGNAL(triggered()),
            this, SLOT(slotStyle()));
    styleAction->setEditable(false);

//js: unused? KGlobalGroup cg = KGlobal::config()->group("General");
    QString currentStyle(kapp->style()->objectName().toLower());
    const QStringList styles = QStyleFactory::keys();
    styleAction->setItems(styles);
    styleAction->setCurrentItem(0);

    QStringList::ConstIterator endIt = styles.constEnd();
    int idx = 0;
    for (QStringList::ConstIterator it = styles.constBegin(); it != endIt; ++it, ++idx) {
        if ((*it).toLower() == currentStyle) {
            styleAction->setCurrentItem(idx);
            break;
        }
    }
    styleAction->setToolTip(i18n("Set the current view style."));
    styleAction->setMenuAccelsEnabled(true);
#endif

    d->lib->addCustomWidgetActions(d->collection);

#ifdef KEXI_DEBUG_GUI
    KConfigGroup generalGroup(KGlobal::config()->group("General"));
    if (generalGroup.readEntry("ShowInternalDebugger", false)) {
        KAction *a = new KAction(KIcon("run-build-file"), i18n("Show Form UI Code"), this);
        d->collection->addAction("show_form_ui", a);
        a->setShortcut(Qt::CTRL + Qt::Key_U);
        connect(a, SIGNAL(triggered()), this, SLOT(showFormUICode()));
    }
#endif

//! @todo move elsewhere
    {
        // (from obsolete kexiformpartinstui.rc)
        QStringList formActions;
        formActions
            << "edit_pointer"
            << QString() //sep
#ifndef KEXI_NO_AUTOFIELD_WIDGET
            << "library_widget_KexiDBAutoField"
#endif
            << "library_widget_KexiDBLabel"
            << "library_widget_KexiDBLineEdit"
            << "library_widget_KexiDBTextEdit"
            << "library_widget_KexiDBComboBox"
            << "library_widget_KexiDBCheckBox"
            << "library_widget_KexiDBImageBox"
            << QString() //sep
            << "library_widget_KPushButton"
            << QString() //sep
            << "library_widget_KexiFrame"
            << "library_widget_QGroupBox"
            << "library_widget_KFDTabWidget"
            << QString() //sep
            << "library_widget_Line"
#ifndef KEXI_NO_FORM_LAYOUTS
            << "library_widget_Spacer"
#endif
#ifndef KEXI_NO_FORM_SPRING_ELEMENT
            << "library_widget_Spring"
            << QString() //sep
#endif
#ifdef CAN_USE_QTWEBKIT
            << "library_widget_WebBrowserWidget"
#endif
#ifdef CAN_USE_MARBLE
            << "library_widget_MapBrowserWidget"
#endif
            << "library_widget_KexiDBSlider"
            << "library_widget_KexiDBProgressBar"
            << "library_widget_KexiDBCommandLinkButton"
            << "library_widget_KexiDBDatePicker"
            << QString() //sep
            ;
        KexiMainWindowIface *win = KexiMainWindowIface::global();
        foreach( const QString& actionName_, formActions ) {
            QAction *a;
            const QString actionName(actionName_.startsWith(':') ? actionName_.mid(1) : actionName_);
            if (actionName.isEmpty()) {
                a = new QAction(this);
                a->setSeparator(true);
            }
            else {
                a = d->widgetActionGroup->action(actionName);
            }
            if (actionName_.startsWith(':')) {  // icon only
                KexiSmallToolButton *btn = new KexiSmallToolButton(a, win->toolBar("form"));
                btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
                win->appendWidgetToToolbar("form", btn);
            }
            else {
                win->addToolBarAction("form", a);
            }
        }

        QSet<QString> iconOnlyActions;
        iconOnlyActions << "show_form_ui";
        const QList<QAction*> actions( d->collection->actions() );
        foreach( QAction *a, actions ) {
            if (iconOnlyActions.contains(a->objectName())) { // icon only
                KexiSmallToolButton *btn = new KexiSmallToolButton(a, win->toolBar("form"));
                btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
                win->appendWidgetToToolbar("form", btn);
            }
            else {
                win->addToolBarAction("form", a);
            }
        }
    }
}

// moved from KexiFormPart
void KexiFormManager::slotWidgetCreatedByFormsLibrary(QWidget* widget)
{
    QList<QMetaMethod> _signals(KexiUtils::methodsForMetaObject(
                                    widget->metaObject(), QMetaMethod::Signal));

    if (!_signals.isEmpty()) {
        const char *handleDragMoveEventSignal = "handleDragMoveEvent(QDragMoveEvent*)";
        const char *handleDropEventSignal = "handleDropEvent(QDropEvent*)";
        KexiFormView *formView = KexiUtils::findParent<KexiFormView*>(widget);

        foreach(const QMetaMethod& method, _signals) {
            if (0 == qstrcmp(method.signature(), handleDragMoveEventSignal)) {
                kDebug() << method.signature();
                if (formView) {
                    connect(widget, SIGNAL(handleDragMoveEvent(QDragMoveEvent*)),
                            formView, SLOT(slotHandleDragMoveEvent(QDragMoveEvent*)));
                }
            } else if (0 == qstrcmp(method.signature(), handleDropEventSignal)) {
                kDebug() << method.signature();
                if (formView) {
                    connect(widget, SIGNAL(handleDropEvent(QDropEvent*)),
                            formView, SLOT(slotHandleDropEvent(QDropEvent*)));
                }
            }
        }
    }
}

void KexiFormManager::slotWidgetActionToggled(const QByteArray& action)
{
    KexiFormView* fv = activeFormViewWidget();
    if (fv) {
        fv->form()->enterWidgetInsertingState(action);
    }
}

KFormDesigner::WidgetLibrary* KexiFormManager::library() const
{
    return d->lib;
}

QAction* KexiFormManager::action(const char* name)
{
    KActionCollection *col = d->part->actionCollectionForMode(Kexi::DesignViewMode);
    if (!col)
        return 0;
    QString n(translateName(name));
    QAction *a = col->action(n);
    if (a)
        return a;
    if (activeFormViewWidget()) {
        a = KexiMainWindowIface::global()->actionCollection()->action(n);
        if (a)
            return a;
    }
    return d->collection->action(name);
}

KexiFormView* KexiFormManager::activeFormViewWidget() const
{
    KexiWindow *currentWindow = KexiMainWindowIface::global()->currentWindow();
    if (!currentWindow)
        return 0;
    KexiView *currentView = currentWindow->selectedView();
    KFormDesigner::Form *form;
    if (!currentView
        || currentView->viewMode()!=Kexi::DesignViewMode
        || !dynamic_cast<KexiFormView*>(currentView)
        || !(form = dynamic_cast<KexiFormView*>(currentView)->form())
       )
    {
        return 0;
    }
    KexiDBForm *dbform = dynamic_cast<KexiDBForm*>(form->formWidget());
    KexiFormScrollView *scrollViewWidget = dynamic_cast<KexiFormScrollView*>(dbform->dataAwareObject());
    if (!scrollViewWidget)
        return 0;
    return dynamic_cast<KexiFormView*>(scrollViewWidget->parent());
}

void KexiFormManager::enableAction(const char* name, bool enable)
{
    KexiFormView* formViewWidget = activeFormViewWidget();
    if (!formViewWidget)
        return;
    formViewWidget->setAvailable(translateName(name).toLatin1(), enable);
}

void KexiFormManager::setFormDataSource(const QString& partClass, const QString& name)
{
    KexiFormView* formViewWidget = activeFormViewWidget();
    if (!formViewWidget)
        return;
    KexiDBForm* formWidget = dynamic_cast<KexiDBForm*>(formViewWidget->form()->widget());
    if (!formWidget)
        return;

    QString oldDataSourcePartClass(formWidget->dataSourcePartClass());
    QString oldDataSource(formWidget->dataSource());
    if (partClass != oldDataSourcePartClass || name != oldDataSource) {
        QHash<QByteArray, QVariant> propValues;
        propValues.insert("dataSource", name);
        propValues.insert("dataSourcePartClass", partClass);
        KFormDesigner::PropertyCommandGroup *group = new KFormDesigner::PropertyCommandGroup(
//            *formViewWidget->form(),
            i18n("Set Form's Data Source to \"%1\"", name));
        formViewWidget->form()->createPropertyCommandsInDesignMode(
            formWidget, propValues, group, true /*addToActiveForm*/);
    }
}

void KexiFormManager::setDataSourceFieldOrExpression(
    const QString& string, const QString& caption, KexiDB::Field::Type type)
{
    KexiFormView* formViewWidget = activeFormViewWidget();
    if (!formViewWidget)
        return;
    
    KoProperty::Set& set = formViewWidget->form()->propertySet();
    if (!set.contains("dataSource"))
        return;

    set["dataSource"].setValue(string);

    if (set.propertyValue("autoCaption", false).toBool()) {
        set.changePropertyIfExists("fieldCaptionInternal", caption);
    }
    if (set.propertyValue("widgetType").toString() == "Auto") {
        set.changePropertyIfExists("fieldTypeInternal", type);
    }
}

void KexiFormManager::insertAutoFields(const QString& sourcePartClass, const QString& sourceName,
                                       const QStringList& fields)
{
    KexiFormView* formViewWidget = activeFormViewWidget();
    if (!formViewWidget || !formViewWidget->form() || !formViewWidget->form()->activeContainer())
        return;
    formViewWidget->insertAutoFields(sourcePartClass, sourceName, fields,
                                     formViewWidget->form()->activeContainer());
}

void KexiFormManager::slotHistoryCommandExecuted(KFormDesigner::Command *command)
{
//    const KFormDesigner::CommandGroup *group = dynamic_cast<const KFormDesigner::CommandGroup*>(command);
    if (command->childCount() == 2) {
        KexiFormView* formViewWidget = activeFormViewWidget();
        if (!formViewWidget)
            return;
        KexiDBForm* formWidget = dynamic_cast<KexiDBForm*>(formViewWidget->form()->widget());
        if (!formWidget)
            return;
        const KFormDesigner::PropertyCommand* pc1
            = dynamic_cast<const KFormDesigner::PropertyCommand*>(command->child(0));
        const KFormDesigner::PropertyCommand* pc2
            = dynamic_cast<const KFormDesigner::PropertyCommand*>(command->child(1));
        if (pc1 && pc2 && pc1->propertyName() == "dataSource" && pc2->propertyName() == "dataSourcePartClass") {
            const QHash<QByteArray, QVariant>::const_iterator it1(pc1->oldValues().constBegin());
            const QHash<QByteArray, QVariant>::const_iterator it2(pc2->oldValues().constBegin());
            if (it1.key() == formWidget->objectName() && it2.key() == formWidget->objectName())
                d->part->dataSourcePage()->setFormDataSource(
                    formWidget->dataSourcePartClass(), formWidget->dataSource());
        }
    }
}

// moved from FormManager
void KexiFormManager::showFormUICode()
{
#ifdef KEXI_DEBUG_GUI
    KexiFormView* formView = activeFormViewWidget();
    if (!formView)
        return;
    formView->form()->resetInlineEditor();

    QString uiCode;
    const int indent = 2;
    if (!KFormDesigner::FormIO::saveFormToString(formView->form(), uiCode, indent)) {
        //! @todo show err?
        return;
    }

    KPageDialog uiCodeDialog;
    uiCodeDialog.setFaceType(KPageDialog::Tabbed);
    uiCodeDialog.setModal(true);
    uiCodeDialog.setWindowTitle(i18n("Form's UI Code"));
    uiCodeDialog.setButtons(KDialog::Close);
    uiCodeDialog.resize(700, 600);
    KTextEdit *currentUICodeDialogEditor = new KTextEdit(&uiCodeDialog);
    uiCodeDialog.addPage(currentUICodeDialogEditor, i18n("Current"));
    currentUICodeDialogEditor->setReadOnly(true);
    QFont f(currentUICodeDialogEditor->font());
    f.setFamily("courier");
    currentUICodeDialogEditor->setFont(f);

    KTextEdit *originalUICodeDialogEditor = new KTextEdit(&uiCodeDialog);
    uiCodeDialog.addPage(originalUICodeDialogEditor, i18n("Original"));
    originalUICodeDialogEditor->setReadOnly(true);
    originalUICodeDialogEditor->setFont(f);
    currentUICodeDialogEditor->setPlainText(uiCode);
    //indent and set our original doc as well:
    QDomDocument doc;
    doc.setContent(formView->form()->m_recentlyLoadedUICode);
    originalUICodeDialogEditor->setPlainText(doc.toString(indent));
    uiCodeDialog.exec();
#endif
}

void KexiFormManager::slotAssignAction()
{
    KexiFormView* formView = activeFormViewWidget();
    if (!formView)
        return;
    KFormDesigner::Form *form = formView->form();
    KexiDBForm *dbform = 0;
    if (form->mode() != KFormDesigner::Form::DesignMode
        || !(dbform = dynamic_cast<KexiDBForm*>(form->formWidget())))
    {
        return;
    }

    KoProperty::Set& set = form->propertySet();

    KexiFormEventAction::ActionData data;
    const KoProperty::Property &onClickActionProp = set.property("onClickAction");
    if (!onClickActionProp.isNull())
        data.string = onClickActionProp.value().toString();

    const KoProperty::Property &onClickActionOptionProp = set.property("onClickActionOption");
    if (!onClickActionOptionProp.isNull())
        data.option = onClickActionOptionProp.value().toString();

    KexiFormScrollView *scrollViewWidget
        = dynamic_cast<KexiFormScrollView*>(dbform->dataAwareObject());
    if (!scrollViewWidget)
        return;
    KexiFormView* formViewWidget = dynamic_cast<KexiFormView*>(scrollViewWidget->parent());
    if (!formViewWidget)
        return;

    KexiActionSelectionDialog dlg(dbform, data,
                                  set.property("objectName").value().toString());

    if (dlg.exec() == QDialog::Accepted) {
        data = dlg.currentAction();
        //update property value
        set.changeProperty("onClickAction", data.string);
        set.changeProperty("onClickActionOption", data.option);
    }
}

void KexiFormManager::slotPointerClicked()
{
    KexiFormView* formView = activeFormViewWidget();
    if (!formView)
        return;
    formView->form()->enterWidgetSelectingState();
}

#include "kexiformmanager.moc"

