/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#ifndef REMOVENOTECOMMAND_H
#define REMOVENOTECOMMAND_H

#include <kundo2command.h>

namespace MusicCore {
    class Chord;
    class Note;
}
class MusicShape;

class RemoveNoteCommand : public KUndo2Command {
public:
    RemoveNoteCommand(MusicShape* shape, MusicCore::Chord* chord, MusicCore::Note* note);
    virtual void redo();
    virtual void undo();
private:
    MusicCore::Chord* m_chord;
    MusicCore::Note* m_note;
    MusicShape* m_shape;
};

#endif // REMOVENOTECOMMAND_H
