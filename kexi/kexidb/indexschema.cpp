/* This file is part of the KDE project
   Copyright (C) 2003 Jaroslaw Staniek <js@iidea.pl>

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

#include <kexidb/indexschema.h>

#include <kexidb/driver.h>
#include <kexidb/connection.h>
#include <kexidb/tableschema.h>

#include <assert.h>

#include <kdebug.h>

using namespace KexiDB;

IndexSchema::IndexSchema(TableSchema *tableSchema)
	: FieldList(false)//fields are not owned by IndexSchema object
	, SchemaData(KexiDB::IndexObjectType)
	, m_tableSchema(tableSchema)
//	, m_conn( conn )
	, m_primary( false )
	, m_isAutoGenerated( false )
{
	m_refs_from.setAutoDelete(0); //refs at foreign-side are owned
}

IndexSchema::~IndexSchema()
{
	/* list of references to the table (of this index), i.e. any such reference in which
	 the table is at 'foreign' side will be cleared and references will be destroyed.
	 So, we need to detach all these references from referenced-side, corresponding indices.
	*/

	Reference *ref = m_refs_from.first();
	while (ref) {
		if (ref->referencedIndex()) {
			ref->referencedIndex()->detachReference(ref);
		}
		ref = m_refs_from.next();
	}
	//ok, now m_refs_from will be just cleared automatically
}

FieldList& IndexSchema::addField(Field *field)
{
	if (field->table() != m_tableSchema) {
		KexiDBDbg << "IndexSchema::addField(" << (field ? field->name() : 0) 
		 << "): WARNING: field doas not belong to the same table '"
		 << (field && field->table() ? field->table()->name() : 0) 
		 << "'as index!" << endl;
		return *this;
	}
	return FieldList::addField(field);
}


KexiDB::TableSchema* IndexSchema::table() const
{
	return m_tableSchema;
}

bool IndexSchema::isAutoGenerated() const
{
	return m_isAutoGenerated;
}

void IndexSchema::setAutoGenerated(bool set)
{
	m_isAutoGenerated = set;
}

bool IndexSchema::isPrimaryKey() const
{
	return m_primary;
}

void IndexSchema::setPrimaryKey(bool set)
{
	m_primary = set;
	if (m_primary)
		m_unique = true;
}

bool IndexSchema::isUnique() const
{
	return m_unique;
}

void IndexSchema::setUnique(bool set)
{
	m_unique=set;
	if (!m_unique)
		m_primary=false;
}
		
void IndexSchema::debug() const
{
	KexiDBDbg << "INDEX " 
		<< (m_isAutoGenerated ? "AUTOGENERATED " : "")
		<< (m_primary ? "PRIMARY " : "")
		<< ((!m_primary) && m_unique ? "UNIQUE " : "")
		<< schemaDataDebugString() << endl;
	FieldList::debug();
}

		/*! Attaches reference definition \a ref to this IndexSchema object. 
		 If \a ref reference has this IndexSchema defined at the reference-side,
		 \a ref is added to referencesTo() list, otherwise (it is has this IndexSchema 
		 defined at the foreign-side) \a ref is added to referencesFrom() list.
		 For the latter case, attached \a ref object is now owned by this IndexSchema object. 

		 Note: call detachReference() for IndexSchema object that \a ref 
		 was previously attached to, if any. */
		void attachReference(Reference *ref);

		/*! Detaches reference definition \a ref to this IndexSchema object
		 from referencesTo() or referencesFrom() (depending on which side of the reference
		 is this IndexSchem object.

		 Note: If \a ref was detached from referencesFrom() list, this \a ref object now has no parent, 
		 so attach it to any or destruct the reference. 
		*/
		void detachReference(Reference *ref);

void IndexSchema::attachReference(Reference *ref)
{
	if (!ref)
		return;
	if (ref->foreignIndex()==this) {
		if (m_refs_from.find(ref)==-1) {
			m_refs_from.append(ref);
		}
	}
	else if (ref->referencedIndex()==this) {
		if (m_refs_to.find(ref)==-1) {
			m_refs_to.append(ref);
		}
	}
}

void IndexSchema::detachReference(Reference *ref)
{
	if (!ref)
		return;
	m_refs_from.take( m_refs_from.find(ref) ); //for sanity
	m_refs_to.take( m_refs_to.find(ref) ); //for sanity
}
