/***************************************************************************
 * manager.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "manager.h"
#include "interpreter.h"
#include "action.h"

#include <QObject>
#include <QMetaObject>
#include <QFile>
#include <QRegExp>
#include <QAbstractItemModel>
#include <QFileInfo>

#include <kapplication.h>
#include <kactioncollection.h>
#include <klibloader.h>
#include <klocale.h>
#include <kstaticdeleter.h>
#include <kdialog.h>
#include <kicon.h>
#include <kconfig.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kmimetype.h>
#include <kmenu.h>

extern "C"
{
    typedef QObject* (*def_module_func)();
}

using namespace Kross;

namespace Kross {

    /// @internal
    class Manager::Private
    {
        public:
            /// List of \a InterpreterInfo instances.
            QMap<QString, InterpreterInfo* > interpreterinfos;

            /// Loaded modules.
            QMap<QString, QPointer<QObject> > modules;

            /// The collection of \a Action instances.
            KActionCollection* actioncollection;
    };

}

static KStaticDeleter<Manager> m_manager;
static Manager* _self = 0;

Manager& Manager::self()
{
    if(! _self)
        m_manager.setObject(_self, new Manager());
    return *_self;
}

Manager::Manager()
    : QObject()
    , ChildrenInterface()
    , d( new Private() )
{
    d->actioncollection = new KActionCollection(this);

#ifdef KROSS_PYTHON_LIBRARY
    QString pythonlib = QFile::encodeName( KLibLoader::self()->findLibrary(KROSS_PYTHON_LIBRARY) );
    if(! pythonlib.isEmpty()) { // If the Kross Python plugin exists we offer is as supported scripting language.
        InterpreterInfo::Option::Map pythonoptions;
        d->interpreterinfos.insert("python",
            new InterpreterInfo("python",
                pythonlib, // library
                "*.py", // file filter-wildcard
                QStringList() << /* "text/x-python" << */ "application/x-python", // mimetypes
                pythonoptions // options
            )
        );
    } else {
        #ifdef KROSS_INTERPRETER_DEBUG
            krossdebug("Python interpreter for kross is unavailable");
        #endif
    }
#endif

#ifdef KROSS_RUBY_LIBRARY
    QString rubylib = QFile::encodeName( KLibLoader::self()->findLibrary(KROSS_RUBY_LIBRARY) );
    if(! rubylib.isEmpty()) { // If the Kross Ruby plugin exists we offer is as supported scripting language.
        InterpreterInfo::Option::Map rubyoptions;
        rubyoptions.insert("safelevel",
            new InterpreterInfo::Option(
                i18n("Level of safety of the Ruby interpreter"),
                QVariant(0) // 0 -> unsafe, 4 -> very safe
            )
        );
        d->interpreterinfos.insert("ruby",
            new InterpreterInfo("ruby",
                rubylib, // library
                "*.rb", // file filter-wildcard
                QStringList() << /* "text/x-ruby" << */ "application/x-ruby", // mimetypes
                rubyoptions // options
            )
        );
    } else {
        #ifdef KROSS_INTERPRETER_DEBUG
            krossdebug("Ruby interpreter for kross is unavailable");
        #endif
    }
#endif

#ifdef KROSS_KJS_LIBRARY
    QString kjslib = QFile::encodeName( KLibLoader::self()->findLibrary(KROSS_KJS_LIBRARY) );
    if(! kjslib.isEmpty()) { // If the Kjs plugin exists we offer is as supported scripting language.
        InterpreterInfo::Option::Map kjsoptions;
        kjsoptions.insert("restricted",
            new InterpreterInfo::Option(
                i18n("Restricted mode for untrusted scripts"),
                QVariant(true) // per default enabled
            )
        );
        d->interpreterinfos.insert("javascript",
            new InterpreterInfo("javascript",
                kjslib, // library
                "*.js", // file filter-wildcard
                QStringList() << "application/x-javascript", // mimetypes
                kjsoptions // options
            )
        );
    } else {
        #ifdef KROSS_INTERPRETER_DEBUG
            krossdebug("KDE JavaScript interpreter for kross is unavailable");
        #endif
    }
#endif

    // publish ourself.
    ChildrenInterface::addObject(this, "Kross");
}

Manager::~Manager()
{
    for(QMap<QString, InterpreterInfo* >::Iterator it = d->interpreterinfos.begin(); it != d->interpreterinfos.end(); ++it)
        delete it.value();
    for(QMap<QString, QPointer<QObject> >::Iterator it = d->modules.begin(); it != d->modules.end(); ++it)
        delete it.value();
    delete d;
}

QMap<QString, InterpreterInfo*> Manager::interpreterInfos()
{
    return d->interpreterinfos;
}

bool Manager::hasInterpreterInfo(const QString& interpretername) const
{
    return d->interpreterinfos.contains(interpretername);
}

InterpreterInfo* Manager::interpreterInfo(const QString& interpretername)
{
    return d->interpreterinfos[interpretername];
}

const QString Manager::interpreternameForFile(const QString& file)
{
    QRegExp rx;
    rx.setPatternSyntax(QRegExp::Wildcard);
    for(QMap<QString, InterpreterInfo*>::Iterator it = d->interpreterinfos.begin(); it != d->interpreterinfos.end(); ++it) {
        rx.setPattern((*it)->wildcard());
        if( file.contains(rx) )
            return (*it)->interpreterName();
    }
    return QString::null;
}

Interpreter* Manager::interpreter(const QString& interpretername)
{
    if(! d->interpreterinfos.contains(interpretername)) {
        krosswarning( QString("No such interpreter '%1'").arg(interpretername) );
        return 0;
    }
    return d->interpreterinfos[interpretername]->interpreter();
}

QStringList Manager::interpreters()
{
    QStringList list;
    QMap<QString, InterpreterInfo*>::Iterator it( d->interpreterinfos.begin() );
    for(; it != d->interpreterinfos.end(); ++it)
        list << it.key();
    return  list;
}

bool Manager::readConfig()
{
    KConfig* config = KApplication::kApplication()->sessionConfig();
    krossdebug( QString("Manager::readConfig hasGroup=%1 isReadOnly=%2 isImmutable=%3 ConfigState=%4").arg(config->hasGroup("scripts")).arg(config->isReadOnly()).arg(config->isImmutable()).arg(config->getConfigState()) );
    if(! config->hasGroup("scripts"))
        return false;

    // we need to remember the current names, to be able to remove "expired" actions later.
    QStringList actionnames;
    foreach(KAction* a, d->actioncollection->actions())
        actionnames.append( a->objectName() );

    // iterate now through the items in the [scripts]-section
    config->setGroup("scripts");
    foreach(QString name, config->readEntry("names", QStringList())) {
        bool needsupdate = actionnames.contains( name );
        if( needsupdate )
            actionnames.removeAll( name );

        QString text = config->readEntry(QString("%1_text").arg(name).toLatin1());
        QString description = config->readEntry(QString("%1_description").arg(name).toLatin1());
        QString icon = config->readEntry(QString("%1_icon").arg(name).toLatin1());
        QString file = config->readEntry(QString("%1_file").arg(name).toLatin1());
        QString interpreter = config->readEntry(QString("%1_interpreter").arg(name).toLatin1());

        if( text.isEmpty() )
            text = file;
        if( description.isEmpty() )
            description = text.isEmpty() ? name : text;
        if( icon.isEmpty() )
            icon = KMimeType::iconNameForUrl( KUrl(file) );

        Action* action = needsupdate
            ? dynamic_cast< Action* >( d->actioncollection->action(name) )
            : new Action(d->actioncollection, name);
        Q_ASSERT(action);

        action->setText(text);
        action->setDescription(description);
        if( ! icon.isNull() )
            action->setIcon(KIcon(icon));
        if( ! interpreter.isNull() )
            action->setInterpreter(interpreter);
        action->setFile(file);

        connect(action, SIGNAL( started(Kross::Action*) ), this, SIGNAL( started(Kross::Action*)) );
        connect(action, SIGNAL( finished(Kross::Action*) ), this, SIGNAL( finished(Kross::Action*) ));
    }

    // remove actions that are not valid anymore
    foreach(QString n, actionnames) {
        KAction* a = d->actioncollection->action(n);
        Q_ASSERT(a);
        d->actioncollection->remove(a);
        delete a;
    }

    return true;
}

bool Manager::writeConfig()
{
    KConfig* config = KApplication::kApplication()->sessionConfig();
    krossdebug( QString("Manager::writeConfig hasGroup=%1 isReadOnly=%2 isImmutable=%3 ConfigState=%4").arg(config->hasGroup("scripts")).arg(config->isReadOnly()).arg(config->isImmutable()).arg(config->getConfigState()) );
    if(config->isReadOnly())
        return false;

    config->deleteGroup("scripts"); // remove old entries
    config->setGroup("scripts"); // according to the documentation it's needed to re-set the group after delete.

    QStringList names;
    foreach(KAction* a, d->actioncollection->actions(QString::null)) {
        Action* action = static_cast< Action* >(a);
        const QString name = action->objectName();
        names << name;
        config->writeEntry(QString("%1_text").arg(name).toLatin1(), action->text());
        config->writeEntry(QString("%1_description").arg(name).toLatin1(), action->description());

        //TODO hmmm... kde4's KIcon / Qt4's QIcon does not allow to reproduce the iconname?
        //config->writeEntry(QString("%1_icon").arg(name).toLatin1(), action->icon());

        config->writeEntry(QString("%1_file").arg(name).toLatin1(), action->file());
        config->writeEntry(QString("%1_interpreter").arg(name).toLatin1(), action->interpreter());
    }

    config->writeEntry("names", names);
    //config->sync();
    return true;
}

KActionCollection* Manager::actionCollection() const
{
    return d->actioncollection;
}

bool Manager::hasAction(const QString& name)
{
    return findChild< Action* >(name) != 0L;
}

QObject* Manager::action(const QString& name)
{
    Action* action = findChild< Action* >(name);
    if(! action) {
        action = new Action(name);
        action->setParent(this);
        d->actioncollection->insert(action); //FIXME should we really remember the action?
    }
    return action;
}

QObject* Manager::module(const QString& modulename)
{
    if( d->modules.contains(modulename) ) {
        QObject* obj = d->modules[modulename];
        if( obj )
            return obj;
    }

    if( modulename.isEmpty() || modulename.contains( QRegExp("[^a-zA-Z0-9]") ) ) {
        krosswarning( QString("Invalid module name '%1'").arg(modulename) );
        return 0;
    }

    QByteArray libraryname = QString("krossmodule%1").arg(modulename).toLower().toLatin1();

    KLibLoader* loader = KLibLoader::self();
    KLibrary* lib = loader->globalLibrary( libraryname );
    if( ! lib ) {
        krosswarning( QString("Failed to load module '%1': %2").arg(modulename).arg(loader->lastErrorMessage()) );
        return 0;
    }

    def_module_func func;
    func = (def_module_func) lib->symbol("krossmodule");
    if( ! func ) {
        krosswarning( QString("Failed to determinate init function in module '%1'").arg(modulename) );
        return 0;
    }

    QObject* module = (QObject*) (func)(); // call the function
    lib->unload(); // unload the library

    if( ! module ) {
        krosswarning( QString("Failed to load module object '%1'").arg(modulename) );
        return 0;
    }

    //krossdebug( QString("Manager::module Module successfully loaded: modulename=%1 module.objectName=%2 module.className=%3").arg(modulename).arg(module->objectName()).arg(module->metaObject()->className()) );
    d->modules.insert(modulename, module);
    return module;
}

#include "manager.moc"
