/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef H_KW_SHAPE_CREATE_COMMAND_H
#define H_KW_SHAPE_CREATE_COMMAND_H

#include <kundo2command.h>

class KoShape;
class KoTextAnchor;
class KWDocument;

/// The undo / redo command for creating shapes
class KWShapeCreateCommand : public KUndo2Command
{
public:
    /**
     * Command used on creation of new shapes
     * @param controller the controller used to add/remove the shape from
     * @param shape the shape thats just been created.
     * @param parent the parent command used for macro commands
     */
    KWShapeCreateCommand(KWDocument *doc, KoShape *shape, KoTextAnchor *anchor, KUndo2Command *parent=0);
    virtual ~KWShapeCreateCommand();
    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();

private:
    KWDocument *m_document;
    KoShape *m_shape;
    KoTextAnchor *m_anchor;
    bool m_deleteShape;
};

#endif // H_KW_SHAPE_CREATE_COMMAND_H
