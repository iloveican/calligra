/*
 * modify_selection.h -- Part of Krita
 *
 * Copyright (c) 2006 Michael Thaler (michael.thaler@physik.tu-muenchen.de)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef MODIFY_SELECTION_H
#define MODIFY_SELECTION_H

#include <QVariant>

#include <kparts/plugin.h>

class KisView2;

class ModifySelection : public KParts::Plugin
{
    Q_OBJECT
public:
    ModifySelection(QObject *parent, const QVariantList &);
    virtual ~ModifySelection();

private slots:

    void slotUpdateGUI();

    void slotGrowSelection();
    void slotShrinkSelection();
    void slotBorderSelection();
    void slotFeatherSelection();

private:

    KisView2 * m_view;

    KAction *m_growSelection;
    KAction *m_shrinkSelection;
    KAction *m_borderSelection;
    KAction *m_featherSelection;
};

#endif // MODIFY_SELECTION_H
