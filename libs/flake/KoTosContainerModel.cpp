/* This file is part of the KDE project
   Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoTosContainerModel.h"

#include "KoTextShapeDataBase.h"
#include "KoTosContainer.h"

#include <KDebug>

KoTosContainerModel::KoTosContainerModel()
: m_textShape(0)
{
}

KoTosContainerModel::~KoTosContainerModel()
{
}

void KoTosContainerModel::add(KoShape *shape)
{
    // make sure shape is a text shape
    KoTextShapeDataBase *shapeData = qobject_cast<KoTextShapeDataBase*>(shape->userData());
    Q_ASSERT(shapeData != 0);
    if (shapeData) {
        m_textShape = shape;
    }
}

void KoTosContainerModel::remove(KoShape *shape)
{
    Q_ASSERT(m_textShape == 0 || shape == m_textShape);
    if (shape == m_textShape) {
        m_textShape = 0;
    }
}

void KoTosContainerModel::setClipped(const KoShape *shape, bool clipping)
{
    Q_UNUSED(shape);
    Q_UNUSED(clipping);
}

bool KoTosContainerModel::isClipped(const KoShape *shape) const
{
    Q_UNUSED(shape)
    return false;
}

void KoTosContainerModel::setInheritsTransform(const KoShape *shape, bool inherit)
{
    Q_UNUSED(shape);
    Q_UNUSED(inherit);
}

bool KoTosContainerModel::inheritsTransform(const KoShape *shape) const
{
    Q_UNUSED(shape)
    return true;
}

bool KoTosContainerModel::isChildLocked(const KoShape *child) const
{
    Q_ASSERT(child == m_textShape);
    Q_ASSERT(child->parent());
    // TODO do we need to guard this?
    return child->isGeometryProtected() || child->parent()->isGeometryProtected();
}

int KoTosContainerModel::count() const
{
    return m_textShape != 0 ? 1 : 0;
}

QList<KoShape*> KoTosContainerModel::shapes() const
{
    QList<KoShape*> shapes;
    if (m_textShape) {
        shapes << m_textShape;
    }
    return shapes;
}

void KoTosContainerModel::containerChanged(KoShapeContainer *container, KoShape::ChangeType type)
{
    kDebug(30006) << "change type:" << type << KoShape::SizeChanged << KoShape::ContentChanged;
    if (type != KoShape::SizeChanged && type != KoShape::ContentChanged) {
        return;
    }
    KoTosContainer *tosContainer = dynamic_cast<KoTosContainer*>(container);
    kDebug(30006) << "tosContainer" << tosContainer;
    if (tosContainer) {
        kDebug(30006) << "behaviour" << tosContainer->resizeBehavior() << KoTosContainer::TextFollowsPreferredTextRect;
    }
    if ( m_textShape && tosContainer && tosContainer->resizeBehavior() != KoTosContainer::TextFollowsPreferredTextRect ) {
        kDebug(30006) << "change type setSize";
        m_textShape->setSize(tosContainer->size());
    }
}
