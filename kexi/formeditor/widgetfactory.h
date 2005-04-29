/* This file is part of the KDE project
   Copyright (C) 2003 Lucijan Busch <lucijan@gmx.at>
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004-2005 Jaroslaw Staniek <js@iidea.pl>

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

#ifndef KFORMDESIGNERWIDGETFACTORY_H
#define KFORMDESIGNERWIDGETFACTORY_H


#include <qobject.h>
#include <qguardedptr.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qasciidict.h>

#include <tristate.h>

// class QPixmap;
template<class type> class QValueVector;
template<class type> class QPtrList;
template<class type> class QDict;
class QWidget;
class QDomElement;
class QDomDocument;
class QVariant;
class QListView;
class KLineEdit;
class KTextEdit;
class KActionCollection;

namespace KFormDesigner {

class WidgetFactory;
class Container;
class ResizeHandleSet;
class ObjectTreeItem;

/**
 * This class holds properties of widget classes provided by a factory.
 */
class KFORMEDITOR_EXPORT WidgetInfo
{
	public:
		typedef QPtrList<WidgetInfo> List;
		typedef QAsciiDict<WidgetInfo> Dict;

		WidgetInfo(WidgetFactory *f);

		WidgetInfo(WidgetFactory *f, const char* parentFactoryName, const char* inheritedClassName);

		virtual ~WidgetInfo();

		//! \return a pixmap associated with the widget
		QString pixmap() const { return m_pixmap; }

		//! \return the class name of a widget e.g. 'QLineEdit'
		QCString className() const { return m_class; }

		/*! \return the name used to name widget, that will appear eg in scripts (must not contain spaces
		  nor non-latin1 characters) */
		QString namePrefix() const { return m_prefixName; }

		//! \return the real name e.g. 'Line Edit', showed eg in ObjectTreeView
		QString name() const { return m_name; }

		QString description() const { return m_desc; }
		QString includeFileName() const { return m_include; }
		QValueList<QCString> alternateClassNames() const { return m_alternateNames; }
		QString savingName() const { return m_saveName; }
		WidgetFactory *factory() const { return m_factory; }

		void setPixmap(const QString &p) { m_pixmap = p; }
		void setClassName(const QCString &s) { m_class = s; }
		void setName(const QString &n) { m_name = n; }
		void setNamePrefix(const QString &n) { m_prefixName = n; }
		void setDescription(const QString &desc) { m_desc = desc;}

		/*! Sets the C++ include file corresponding to this class,
		 that uic will need to add when creating the file. */
		void setIncludeFileName(const QString &name) { m_include = name;}

		/*! Sets alternate names for this class.
		 If this name is found when loading a .ui file, the className() will be used instead.
		 It allows to support both KDE and Qt versions of widget, without duplicating code.
		 As a rule, className() should always return a class name which is inherited from
		 alternate class. For example KListView class has alternate QListView class.

		 \a override parameter overrides class name of a widget,
		 even if it was implemented in other factory.
		 By default it's set to false, what means that no other class is overridden
		 by this widget class if there is already a class implementing it
		 (no matter in which factory).
		 By forced overriding existing class with other - custom, user
		 will be able to see more or less properties and experience different behaviour.
		 For example, in Kexi application, KLineEdit class contains additional
		 "datasource" property for binding to database sources.
		*/
		void addAlternateClassName(const QCString& alternateName, bool override = false);

		/*! \return true is a class \a alternateName is defined as alternate name with
		 'override' flag set to true, using addAlternateClassName().
		 If this flag is set to false (the default) or there's no such alternate class
		 name defined. */
		bool isOverriddenClassName(const QCString& alternateName) const;

		/*! Sets the name that will be written in the .ui file when saving.
		 This name must be one of alternate names (or loading will be impossible).

		 On form data saving to XML .ui format, saveName is used instead,
		 so .ui format is not broken and still usable with other software as Qt Designer.
		 Custom properties are saved as well with 'stdset' attribute set to 0. */
		void setSavingName(const QString &saveName) { m_saveName = saveName; }

		/*! Sets autoSync flag for property \a propertyName.
		 This allows to override autoSync flag for certain widget's property, because
		 e.g. KexiPropertyEditor can have autoSync flag set to false or true, but
		 not all properties have to comply with that.
		 \a flag equal to cancelled value means there is no overriding (the default). */
		void setAutoSyncForProperty(const char *propertyName, tristate flag);

		/*! \return autoSync override value (true or false) for \a propertyName.
		 If cancelled value is returned, there is no overriding (the default). */
		tristate autoSyncForProperty(const char *propertyName) const;

		QCString parentFactoryName() const { return m_parentFactoryName; }

		WidgetInfo* inheritedClass() const { return m_inheritedClass; }

	protected:
		QCString m_parentFactoryName, m_inheritedClassName; //!< Used for inheriting widgets between factories
		WidgetInfo* m_inheritedClass;

	private:
		QString m_pixmap;
		QCString m_class;
		QString m_name;
		QString m_prefixName;
		QString m_desc;
		QString m_include;
		QValueList<QCString> m_alternateNames;
		QAsciiDict<char> *m_overriddenAlternateNames;
		QString m_saveName;
		QGuardedPtr<WidgetFactory> m_factory;
		QAsciiDict<char> *m_propertiesWithDisabledAutoSync;

	friend class WidgetLibrary;
};

//! The base class for all widget Factories
/*! This is the class you need to inherit to create a new Factory. There are few
 virtuals you need to implement, and some other functions
 to implement if you want more features.\n \n

  <b>Widget Creation</b>\n
  To be able to create widgets, you need to implement the create() function, an classes(),
  which should return all the widgets supported by this factory.\n \n

  <b>GUI Integration</b>\n
  The following functions allow you to customize even more the look-n-feel of your widgets inside KFormDesigner.
  You can use createMenuActions() to add custom items in widget's context menu. The previewWidget()
  is called when the Form gets in Preview mode, and you have a last opportunity to remove all editing-related
  stuff (see eg \ref Spring class).\n
  You can also choose which properties to show in the Property Editor. By default, all properties are shown,
  but you can hide some using isPropertyVisible(). To add new properties, just define new Q_PROPERTY
  in widget class definition.\n \n

  <b>Inline editing</b>\n
  KFormDesigner allow you to edit the widget's contents inside Form, without using a dialog.
  You can of course customize the behaviour of your widgets, using startEditing(). There are some editing
  modes already implemented in WidgetFactroy, but you can create your own if you want:
  \li Editing using a line edit (createEditor()): a line edit is created on top of widget,
  where the user inputs text. As the text changes, changeText() is called
  (where you should set your widget's text and resize widget to fit the text if needed) and resizeEditor()
  to update editor's position when widget is moved/resized.\n
  \li Editing by disabling event filter: if you call disableFilter(), the event filter
   on the object is temporarily disabled, so the widget behaves as usual. This
  can be used for more complex widgets, such as spinbox, date/time edit, etc.
  \li Other modes: there are 3 other modes, to edit a string list: editList()
  (for combo box, listbox), to edit rich text: editRichText() (for labels, etc.)
  and to edit a listview: editListView(). \n \n

  <b>Widget saving/loading</b>\n
  You can also control how your widget are saved/loaded. You can choose which properties to save
   (see autoSaveProperties()), and save/load custom properties, ie
  properties that are not Q_PROPERTY but you want to save in the UI file. This is used eg to
   save combo box or listview contents (see saveSpecialProperty() and
  readSpecialProperty()). \n \n

  See the standard factories in formeditor/factories for an example of factories,
  and how to deal with complex widgets (eg tabwidget).
  */
class KFORMEDITOR_EXPORT WidgetFactory : public QObject
{
	Q_OBJECT
	public:
		WidgetFactory(QObject *parent=0, const char *name=0);
		virtual ~WidgetFactory();

		/*! Adds a new class described by \a w. */
		void addClass(WidgetInfo *w);

		/*! This method allows to force a class \a classname to hidden.
		 It is useful if you do not want a class to be available
		 (e.g. because it is not implemented well yet for our purposes).
		 All widget libraries are affected by this setting. */
		void hideClass(const char *classname);

		/**
		 * \return all classes which are provided by this factory
		 */
		const WidgetInfo::Dict classes() const { return m_classesByName; }

		/*!
		 * Creates a widget (and if needed a \ref Container)
		 * \return the created widget
		 * \param classname the classname of the widget, which should get created
		 * \param parent the parent for the created widget
		 * \param name the name of the created widget
		 * \param toplevel the toplevel Container (if a container should get created)
		 */
		virtual QWidget* create(const QCString &classname, QWidget *parent, const char *name,
					 KFormDesigner::Container *container)=0;

		virtual void createCustomActions(KActionCollection* ) {};

		/*! This function can be used to add custom items in widget \a w context
		menu \a menu. */
		virtual bool createMenuActions(const QCString &classname, QWidget *w, QPopupMenu *menu,
		    KFormDesigner::Container *container)=0;

		/*! Creates (if necessary) an editor to edit the contents of the widget directly in the Form
		   (eg creates a line edit to change the text of a label). \a classname is
		   the class the widget belongs to, \a w is the widget to edit
		   and \a container is the parent container of this widget (to access Form etc.).
		 */
		virtual bool startEditing(const QCString &classname, QWidget *w, Container *container)=0;

		/*! This function is called just before the Form is previewed. It allows widgets
		 to make changes before switching (ie for a Spring, hiding the cross) */
		virtual bool previewWidget(const QCString &classname, QWidget *widget, Container *container)=0;

		virtual bool clearWidgetContent(const QCString &classname, QWidget *w);

		/*! This function is called when FormIO finds a property, at save time,
		 that it cannot handle (ie not a normal property).
		This way you can save special properties, for example the contents of a listbox.
		  \sa readSpecialProperty()
		 */
		virtual bool saveSpecialProperty(const QCString &classname, const QString &name,
			const QVariant &value, QWidget *w,
			QDomElement &parentNode, QDomDocument &parent);

		/*! This function is called when FormIO finds a property or an unknown
		element in a .ui file. You can this way load a special property, for
		  example the contents of a listbox.
		   \sa saveSpecialProperty()
		*/
		virtual bool readSpecialProperty(const QCString &classname, QDomElement &node,
			QWidget *w, ObjectTreeItem *item);

		/*! This function is used to know whether the \a property for the widget \a w
		should be shown or not in the PropertyEditor. If \a multiple is true,
		then multiple widgets of the same class are selected, and you should
		only show properties shared by widgets (eg font, color). By default,
		all properties are shown if multiple == true, and none if multiple == false. */
		bool isPropertyVisible(const QCString &classname, QWidget *w, 
			const QCString &property, bool multiple);

		/*! You need to return here a list of the properties that should automatically be saved
		for a widget belonging to \a classname, and your custom properties (eg "text"
		for label or button, "contents" for combobox...). */
		virtual QValueList<QCString> autoSaveProperties(const QCString &classname)=0;

		/*! \return The i18n'ed name of the property whose name is \a name,
		 that will be displayed in PropertyEditor. */
		inline QString propertyDescForName(const QCString &name) { return m_propDesc[name]; };

		/*! \return The i18n'ed name of the property's value whose name is \a name. */
		inline QString propertyDescForValue(const QCString &name) { return m_propValDesc[name]; };

	protected:
		virtual bool isPropertyVisibleInternal(const QCString &classname, QWidget *w, 
			const QCString &property) { return false; }

		/*! This function creates a KLineEdit to input some text and edit a widget's contents.
		 This can be used in startEditing(). \a text is the text to display by default
		  in the line edit, \a w is the edited widget, \a geometry is the geometry the new line
		   edit should have, and \a align is Qt::AlignmentFlags of the new line edit.
		 */
		void createEditor(const QCString &classname, const QString &text,
			QWidget *w, Container *container, QRect geometry,
			int align,  bool useFrame=false, BackgroundMode background = Qt::NoBackground);

		/*! This function provides a simple editing mode : it justs disable event filtering
		for the widget, and it install it again when
		  the widget loose focus or Enter is pressed.
		*/
		void disableFilter(QWidget *w, Container *container);

		/*! This function creates a little dialog (a KEditListBox) to modify the contents
		 of a list (of strings). It can be used to modify the contents
		 of a combo box for instance. The modified list is copied
		 into \a list when the user presses "Ok".*/
		bool editList(QWidget *w, QStringList &list);

		/*! This function creates a little editor to modify rich text. It supports alignment,
		 subscript and superscript and all basic formatting properties.
		  If the user presses "Ok", the edited text is put in \a text.
		  If he presses "Cancel", nothing happens. */
		bool editRichText(QWidget *w, QString &text);

		/*! This function creates a dialog to modify the contents of a ListView. You can modify both
		columns and list items. The listview is automatically  updated if the user presses "Ok".*/
		void editListView(QListView *listview);

		/*! This function destroys the editor when it loses focus or Enter is pressed. */
		virtual bool  eventFilter(QObject *obj, QEvent *ev);

		/*! This function is used to modify a property of a widget (eg after editing it).
		Please use it instead of w->setProperty() to allow sync inside PropertyEditor.
		*/
		void changeProperty(const char *name, const QVariant &value, Container *container);

		/*! This function is called when the widget is resized, and the editor size needs to be updated. */
		virtual void resizeEditor(QWidget *widget, const QCString &classname);

//		/*! Adds the i18n'ed description of a property, which will be shown in PropertyEditor. */
//		void  addPropertyDescription(Container *container, const char *prop, const QString &desc);

//		/*! Adds the i18n'ed description of a property value, which will be shown in PropertyEditor. */
//		void  addValueDescription(Container *container, const char *value, const QString &desc);

		/*! \return true if at least one class defined by this factory inherits
		 a class from other factory. Used in WidgetLibrary::loadFactories()
		 to load factories in proper order. */
		bool inheritsFactories();

	protected slots:
		/*!
		Default implementation will change property "text".
		You have to reimplement this function for editing inside the Form to work if your widget's
		property you want to change isn't named "text".
		This slot is called when the line edit text changes, and you have to make
		it really change the good property of the widget using changeProperty() (text, or title, etc.).
		*/
		virtual bool changeText(const QString &newText);

		void changeTextInternal(const QString& text);

		void slotTextChanged();

		/*! This slot is called when the editor has lost focus or the user pressed Enter.
		It destroys the editor or installs again the event filter on the widget. */
		void resetEditor();

		/*! This slot is called when the editor is destroyed.*/
		void editorDeleted();
		void widgetDestroyed();

	protected:
		QGuardedPtr<QWidget> m_widget;
		QCString m_editedWidgetClass;
#ifdef KEXI_KTEXTEDIT
		QGuardedPtr<KTextEdit>  m_editor;
#else
		QGuardedPtr<KLineEdit>  m_editor;
#endif
		QString m_firstText;
		QGuardedPtr<ResizeHandleSet> m_handles;
		QGuardedPtr<Container> m_container;
//		WidgetInfo::List m_classes;
		WidgetInfo::Dict m_classesByName;
		QAsciiDict<char>* m_hiddenClasses;

		//! i18n stuff
		QMap<QCString, QString> m_propDesc;
		QMap<QCString, QString> m_propValDesc;

		/*! flag useful to decide whether to hide some properties. 
		 It's value is inherited from WidgetLibrary. */
		bool m_showAdvancedProperties; 

	friend class WidgetLibrary;
};

}
#endif
