/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "DocumentModel.h"

#include "Document.h"

DocumentModel::DocumentModel( QObject* parent, Document *document ) : KoDocumentSectionModel(parent), m_document(document)
{
  Q_ASSERT(m_document);
}

DocumentModel::~DocumentModel()
{
}

int DocumentModel::rowCount(const QModelIndex &parent) const
{
  if(not parent.isValid())
  {
    return m_document->sections().count();
  } else {
    return static_cast<SectionGroup*>(parent.internalPointer())->sections().count();
  }
}

int DocumentModel::columnCount(const QModelIndex &parent) const
{
  return 1;
}
