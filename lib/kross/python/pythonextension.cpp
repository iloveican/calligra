/***************************************************************************
 * pythonextension.cpp
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

#include "pythonextension.h"
#include "pythonobject.h"

#include "../api/variant.h"
#include "../api/dict.h"
#include "../api/exception.h"

#include <kdebug.h>

using namespace Kross::Python;

PythonExtension::PythonExtension(Kross::Api::Object::Ptr object)
    : Py::PythonExtension<PythonExtension>()
    , m_object(object)
{
#ifdef KROSS_PYTHON_EXTENSION_CTOR_DEBUG
    kdDebug() << QString("Kross::Python::PythonExtension::Constructor objectname='%1' objectclass='%2'").arg(m_object->getName()).arg(m_object->getClassName()) << endl;
#endif

    //TODO determinate and return real dynamic objectname and documentation.
    behaviors().name("KrossPythonExtension");
    behaviors().doc(
        "The common KrossPythonExtension object enables passing "
        "of Kross::Api::Object's from C/C++ to Python and "
        "backwards in a transparent way."
    );
    behaviors().supportGetattr();

    m_proxymethod = new Py::MethodDefExt<PythonExtension>(
        "", // methodname, not needed cause we use the method only internaly.
        0, // method that should handle the callback, not needed cause proxyhandler will handle it.
        Py::method_varargs_call_handler_t( proxyhandler ), // callback handler
        "" // documentation
    );
}

PythonExtension::~PythonExtension()
{
#ifdef KROSS_PYTHON_EXTENSION_DTOR_DEBUG
    kdDebug() << QString("Kross::Python::PythonExtension::Destructor objectname='%1' objectclass='%2'").arg(m_object->getName()).arg(m_object->getClassName()) << endl;
#endif
    delete m_proxymethod;
}

Py::Object PythonExtension::str()
{
    QString s = m_object->getName();
    return toPyObject(s.isEmpty() ? m_object->getClassName() : s);
}

Py::Object PythonExtension::repr()
{
    return toPyObject( m_object->toString() );
}

Py::Object PythonExtension::getattr(const char* n)
{
#ifdef KROSS_PYTHON_EXTENSION_GETATTR_DEBUG
    kdDebug() << QString("Kross::Python::PythonExtension::getattr name='%1'").arg(n) << endl;
#endif

    if(n[0] == '_') {
        if(n == "__methods__") {
            Py::List methods;
            QStringList calls = m_object->getCalls();
            for(QStringList::Iterator it = calls.begin(); it != calls.end(); ++it) {
#ifdef KROSS_PYTHON_EXTENSION_GETATTR_DEBUG
                kdDebug() << QString("Kross::Python::PythonExtension::getattr name='%1' callable='%2'").arg(n).arg(*it) << endl;
#endif
                methods.append(Py::String( (*it).latin1() ));
            }
            return methods;
        }

        if(n == "__members__") {
            Py::List members;
            QMap<QString, Kross::Api::Object::Ptr> children = m_object->getChildren();
            QMap<QString, Kross::Api::Object::Ptr>::Iterator it( children.begin() );
            for(; it != children.end(); ++it) {
#ifdef KROSS_PYTHON_EXTENSION_GETATTR_DEBUG
                kdDebug() << QString("Kross::Python::PythonExtension::getattr n='%1' child='%2'").arg(n).arg(it.key()) << endl;
#endif
                members.append(Py::String( it.key().latin1() ));
            }
            return members;
        }

        //if(n == "__dict__") { kdDebug()<<QString("PythonExtension::getattr(%1) __dict__").arg(n)<<endl; return Py::None(); }
        //if(n == "__class__") { kdDebug()<<QString("PythonExtension::getattr(%1) __class__").arg(n)<<endl; return Py::None(); }

#ifdef KROSS_PYTHON_EXTENSION_GETATTR_DEBUG
        kdDebug() << QString("Kross::Python::PythonExtension::getattr name='%1' is a internal name.").arg(name) << endl;
#endif
        return Py::PythonExtension<PythonExtension>::getattr_methods(n);
    }

    // Redirect the call to our static proxy method which will take care
    // of handling the call.
    Py::Tuple self(2);
    self[0] = Py::Object(this);
    self[1] = Py::String(n);
    return Py::Object(PyCFunction_New( &m_proxymethod->ext_meth_def, self.ptr() ), true);
}

/*
Py::Object PythonExtension::getattr_methods(const char* n)
{
#ifdef KROSS_PYTHON_EXTENSION_GETATTRMETHOD_DEBUG
    kdDebug()<<"PythonExtension::getattr_methods name="<<n<<endl;
#endif
    return Py::PythonExtension<PythonExtension>::getattr_methods(n);
}

int PythonExtension::setattr(const char* name, const Py::Object& value)
{
#ifdef KROSS_PYTHON_EXTENSION_SETATTR_DEBUG
    kdDebug() << QString("PythonExtension::setattr name=%1 value=%2").arg(name).arg(value.as_string().c_str()) << endl;
#endif
    return Py::PythonExtension<PythonExtension>::setattr(name, value);
}
*/

Kross::Api::List::Ptr PythonExtension::toObject(const Py::Tuple& tuple)
{
#ifdef KROSS_PYTHON_EXTENSION_TOOBJECT_DEBUG
    kdDebug() << QString("Kross::Python::PythonExtension::toObject(Py::Tuple)") << endl;
#endif

    QValueList<Kross::Api::Object::Ptr> l;
    uint size = tuple.size();
    for(uint i = 0; i < size; i++)
        l.append( toObject( tuple[i] ) );
    return new Kross::Api::List(l);
}

Kross::Api::List::Ptr PythonExtension::toObject(const Py::List& list)
{
#ifdef KROSS_PYTHON_EXTENSION_TOOBJECT_DEBUG
    kdDebug() << QString("Kross::Python::PythonExtension::toObject(Py::List)") << endl;
#endif

    QValueList<Kross::Api::Object::Ptr> l;
    uint length = list.length();
    for(uint i = 0; i < length; i++)
        l.append( toObject( list[i] ) );
    return new Kross::Api::List(l);
}

Kross::Api::Dict::Ptr PythonExtension::toObject(const Py::Dict& dict)
{
    QMap<QString, Kross::Api::Object::Ptr> map;
    Py::List l = dict.keys();
    uint length = l.length();
    for(Py::List::size_type i = 0; i < length; ++i) {
        const char* n = l[i].str().as_string().c_str();
        map.replace(n, toObject( dict[n] ));
    }
    return new Kross::Api::Dict(map);
}

Kross::Api::Object::Ptr PythonExtension::toObject(const Py::Object& object)
{
#ifdef KROSS_PYTHON_EXTENSION_TOOBJECT_DEBUG
    kdDebug() << QString("Kross::Python::PythonExtension::toObject(Py::Object) object='%1'").arg(object.as_string().c_str()) << endl;
#endif

    if(object.isNumeric()) {
#ifdef KROSS_PYTHON_EXTENSION_TOOBJECT_DEBUG
        kdDebug() << QString("Kross::Python::PythonExtension::toObject(Py::Object) isNumeric") << endl;
#endif
        //FIXME add isUnsignedLong() to Py::Long (or create
        // an own Py::UnsignedLong class) and if true used it
        // rather then long to prevent overflows (needed to
        // handle e.g. uint correct!)

        //Py::Long l = object;
        //return new Kross::Api::Variant(Q_LLONG(long(l)));

        Py::Int i = object;
        return new Kross::Api::Variant(int(i));
    }

    if(object.isString()) {
#ifdef KROSS_PYTHON_EXTENSION_TOOBJECT_DEBUG
        kdDebug() << QString("Kross::Python::PythonExtension::toObject(Py::Object) isString='%1'").arg(object.as_string().c_str()) << endl;
#endif
        return new Kross::Api::Variant(QString(object.as_string().c_str()));
    }

    if(object.isTuple()) {
#ifdef KROSS_PYTHON_EXTENSION_TOOBJECT_DEBUG
        kdDebug() << QString("Kross::Python::PythonExtension::toObject(Py::Object) isTuple") << endl;
#endif
        Py::Tuple tuple = object;
        return toObject(tuple).data();
    }

    if(object.isList()) {
#ifdef KROSS_PYTHON_EXTENSION_TOOBJECT_DEBUG
        kdDebug() << QString("Kross::Python::PythonExtension::toObject(Py::Object) isList") << endl;
#endif
        Py::List list = object;
        return toObject(list).data();
    }

    if(object.isDict()) {
#ifdef KROSS_PYTHON_EXTENSION_TOOBJECT_DEBUG
        kdDebug() << QString("Kross::Python::PythonExtension::toObject(Py::Object) isDict") << endl;
#endif
        Py::Dict dict(object.ptr());
        return toObject(dict).data();
    }

    if(object.isInstance()) {
#ifdef KROSS_PYTHON_EXTENSION_TOOBJECT_DEBUG
        kdDebug() << QString("Kross::Python::PythonExtension::toObject(Py::Object) isInstance") << endl;
#endif
        return new PythonObject(object);
    }


    /*TODO
    if(object.isUnicode()) {
        Py::String s = object;
        return Kross::Api::Variant::create(QVariant(s.as_unicodestring().c_str()));
    }
    isMapping()
    isSequence()
    isTrue()
    */

    if(object == Py::None()) {
#ifdef KROSS_PYTHON_EXTENSION_TOOBJECT_DEBUG
        kdDebug() << QString("Kross::Python::PythonExtension::toObject(Py::Object) isNone") << endl;
#endif
        return 0;
    }


    Py::ExtensionObject<PythonExtension> extobj(object);
    PythonExtension* extension = extobj.extensionObject();
    if(! extension)
        throw Py::TypeError("Failed to determinate PythonExtension object.");
    if(! extension->m_object)
        throw Py::TypeError("Failed to convert the PythonExtension object into a Kross::Api::Object.");

#ifdef KROSS_PYTHON_EXTENSION_TOOBJECT_DEBUG
    kdDebug() << "Kross::Python::PythonExtension::toObject(Py::Object) successfully converted into Kross::Api::Object." << endl;
#endif
    return extension->m_object;
}

Py::Object PythonExtension::toPyObject(const QString& s)
{
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
    kdDebug() << QString("Kross::Python::PythonExtension::toPyObject(QString)") << endl;
#endif
    return s.isNull() ? Py::String() : Py::String(s.latin1());
}

Py::List PythonExtension::toPyObject(QStringList list)
{
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
    kdDebug() << QString("Kross::Python::PythonExtension::toPyObject(QStringList)") << endl;
#endif
    Py::List l;
    for(QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it)
        l.append(toPyObject(*it));
    return l;
}

Py::Dict PythonExtension::toPyObject(QMap<QString, QVariant> map)
{
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
    kdDebug() << QString("Kross::Python::PythonExtension::toPyObject(QMap<QString,QVariant>)") << endl;
#endif
    Py::Dict d;
    for(QMap<QString, QVariant>::Iterator it = map.begin(); it != map.end(); ++it)
        d.setItem(it.key().latin1(), toPyObject(it.data()));
    return d;
}

Py::List PythonExtension::toPyObject(QValueList<QVariant> list)
{
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
    kdDebug() << QString("Kross::Python::PythonExtension::toPyObject(QValueList<QVariant>)") << endl;
#endif
    Py::List l;
    for(QValueList<QVariant>::Iterator it = list.begin(); it != list.end(); ++it)
        l.append(toPyObject(*it));
    return l;
}

Py::Object PythonExtension::toPyObject(const QVariant& variant)
{
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
    kdDebug() << QString("Kross::Python::PythonExtension::toPyObject(QVariant) typename='%1'").arg(variant.typeName()) << endl;
#endif

    switch(variant.type()) {
        case QVariant::Invalid:
            return Py::None();
        case QVariant::Bool:
            return Py::Int(variant.toBool());
        case QVariant::Int:
            return Py::Int(variant.toInt());
        case QVariant::UInt:
            return Py::Long((unsigned long)variant.toUInt());
        case QVariant::Double:
            return Py::Float(variant.toDouble());
        case QVariant::Date:
        case QVariant::Time:
        case QVariant::DateTime:
        case QVariant::ByteArray:
        case QVariant::BitArray:
        case QVariant::CString:
        case QVariant::String:
            return toPyObject(variant.toString());
        case QVariant::StringList:
            return toPyObject(variant.toStringList());
        case QVariant::Map:
            return toPyObject(variant.toMap());
        case QVariant::List:
            return toPyObject(variant.toList());

        // To handle following both cases is a bit difficult
        // cause Python doesn't spend an easy possibility
        // for such large numbers (TODO maybe BigInt?). So,
        // we risk overflows here, but well...
        case QVariant::LongLong: {
            Q_LLONG l = variant.toLongLong();
            //return (l < 0) ? Py::Long((long)l) : Py::Long((unsigned long)l);
            return Py::Long((long)l);
            //return Py::Long(PyLong_FromLong( (long)l ), true);
        } break;
        case QVariant::ULongLong: {
            return Py::Long((unsigned long)variant.toULongLong());
        } break;

        default: {
            kdWarning() << QString("Kross::Python::PythonExtension::toPyObject(QVariant) Not possible to convert the QVariant type '%1' to a Py::Object.").arg(variant.typeName()) << endl;
            return Py::None();
        }
    }
}

Py::Object PythonExtension::toPyObject(Kross::Api::Object::Ptr object)
{
    if(! object) {
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
        kdDebug() << "Kross::Python::PythonExtension::toPyObject(Kross::Api::Object) is NULL => Py::None" << endl;
#endif
        return Py::None();
    }

    if(object->getClassName() == "Kross::Api::Variant") {
        QVariant v = static_cast<Kross::Api::Variant*>( object.data() )->getValue();
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
        kdDebug() << QString("Kross::Python::PythonExtension::toPyObject(Kross::Api::Object) is Kross::Api::Variant %1").arg(v.toString()) << endl;
#endif
        return toPyObject(v);
    }

    if(object->getClassName() == "Kross::Api::List") {
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
        kdDebug() << "Kross::Python::PythonExtension::toPyObject(Kross::Api::Object) is Kross::Api::List" << endl;
#endif
        Py::List pylist;
        Kross::Api::List* list = static_cast<Kross::Api::List*>( object.data() );
        QValueList<Kross::Api::Object::Ptr> valuelist = list->getValue();
        for(QValueList<Kross::Api::Object::Ptr>::Iterator it = valuelist.begin(); it != valuelist.end(); ++it)
            pylist.append( toPyObject(*it) ); // recursive
        return pylist;
    }

    if(object->getClassName() == "Kross::Api::Dict") {
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
        kdDebug() << "Kross::Python::PythonExtension::toPyObject(Kross::Api::Object) is Kross::Api::Dict" << endl;
#endif
        Py::Dict pydict;
        Kross::Api::Dict* dict = static_cast<Kross::Api::Dict*>( object.data() );
        QMap<QString, Kross::Api::Object::Ptr> valuedict = dict->getValue();
        for(QMap<QString, Kross::Api::Object::Ptr>::Iterator it = valuedict.begin(); it != valuedict.end(); ++it) {
            const char* n = it.key().latin1();
            pydict[ n ] = toPyObject( it.data() ); // recursive
        }
        return pydict;
    }

#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
    kdDebug() << QString("Trying to handle PythonExtension::toPyObject(%1) as PythonExtension").arg(object->getClassName()) << endl;
#endif
    return Py::asObject( new PythonExtension(object) );
}

Py::Tuple PythonExtension::toPyTuple(Kross::Api::List::Ptr list)
{
#ifdef KROSS_PYTHON_EXTENSION_TOPYOBJECT_DEBUG
    kdDebug() << QString("Kross::Python::PythonExtension::toPyTuple(Kross::Api::List) name='%1'").arg(list ? list->getName() : "NULL") << endl;
#endif
    uint count = list ? list->count() : 0;
    Py::Tuple tuple(count);
    for(uint i = 0; i < count; i++)
        tuple.setItem(i, toPyObject(list->item(i)));
    return tuple;
}

PyObject* PythonExtension::proxyhandler(PyObject *_self_and_name_tuple, PyObject *args)
{
    Py::Tuple tuple(_self_and_name_tuple);
    PythonExtension *self = static_cast<PythonExtension*>( tuple[0].ptr() );
    QString methodname = Py::String(tuple[1]).as_string().c_str();

    try {
        Kross::Api::List::Ptr arguments = toObject( Py::Tuple(args) );

#ifdef KROSS_PYTHON_EXTENSION_CALL_DEBUG
        kdDebug() << QString("Kross::Python::PythonExtension::proxyhandler methodname='%1' arguments='%2'").arg(methodname).arg(arguments->toString()) << endl;
#endif

        if(self->m_object->hasChild(methodname)) {
#ifdef KROSS_PYTHON_EXTENSION_CALL_DEBUG
            kdDebug() << QString("Kross::Python::PythonExtension::proxyhandler methodname='%1' is a child object of '%2'.").arg(methodname).arg(self->m_object->getName()) << endl;
#endif
            Py::Object result = toPyObject( self->m_object->getChild(methodname)->call(QString::null, arguments) );
            result.increment_reference_count();
            return result.ptr();
        }
#ifdef KROSS_PYTHON_EXTENSION_CALL_DEBUG
        kdDebug() << QString("Kross::Python::PythonExtension::proxyhandler try to call function with methodname '%1' in object '%2'.").arg(methodname).arg(self->m_object->getName()) << endl;
#endif
        Py::Object result = toPyObject( self->m_object->call(methodname, arguments) );
        result.increment_reference_count();
        return result.ptr();
    }
    catch(Kross::Api::Exception::Ptr e) {
        kdWarning() << "EXCEPTION in Kross::Python::PythonExtension::proxyhandler" << endl;
        throw Py::RuntimeError( (char*) e->toString().latin1() );
    }

    return Py_None;
}
