/* This file is part of the KDE project
   Copyright (C) 2002 Bo Thorsen  bo@sonofthor.dk
   Copyright (C) 2004 Dag Andersen <danders@get2net.dk>

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

#ifndef KPTTASKDIALOG_H
#define KPTTASKDIALOG_H

#include <kdialogbase.h>
#include "kptresource.h"
#include <qtable.h>

class KLineEdit;
class KMacroCommand;
class QTextEdit;
class QDateTimeEdit;
class QSpinBox;
class QButtonGroup;
class QListBox;
class QTable;
class QDateTime;

//TODO ui files are not in the KPlato namespace!!
class KPTTaskGeneralPanelBase;
class KPTTaskNotesPanelBase;

namespace KPlato
{

class KPTRequestResourcesPanel;
class KPTPart;
class KPTTask;

class KPTTaskDialog : public KDialogBase {
    Q_OBJECT
public:
    KPTTaskDialog(KPTTask &task, QWidget *parent=0, const char *name=0);

    KMacroCommand *buildCommand(KPTPart *part);

protected slots:
    void slotOk();

private:
    KPTTask &m_task;
    KLineEdit *m_name;
    KLineEdit *m_leader;
    QTextEdit *m_description;

    KPTTaskGeneralPanelBase *m_generalTab;
    KPTRequestResourcesPanel *m_resourcesTab;
    KPTTaskNotesPanelBase *m_notesTab;

    // TODO: Duration and risk fields
};

} //KPlato namespace

#endif // KPTTASKDIALOG_H
