/* This file is part of the KDE project
   Copyright (C) 2002 Norbert Andres, nandres@web.de

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


#ifndef __kspread_dlg_subtotal__
#define __kspread_dlg_subtotal__

#include <kdialogbase.h>

#include <qpoint.h>
#include <qrect.h>

class KSpreadView;
class KSpreadSheet;
class KSpreadSubtotal;

class QLineEdit;

class KSpreadSubtotalDlg : public KDialogBase
{ 
  Q_OBJECT

 public:
  KSpreadSubtotalDlg( KSpreadView * parent, QRect const & selection, 
                      const char * name );
  ~KSpreadSubtotalDlg();

  QRect const & selection() const { return m_selection; }
  KSpreadSheet * table() const { return m_pTable; }

 private slots:
  void slotOk();
  void slotCancel();
  void slotUser1();

 private:
  KSpreadView  *    m_pView;
  KSpreadSheet *    m_pTable;
  QRect             m_selection;
  KSpreadSubtotal * m_dialog;

  void fillColumnBoxes();
  void fillFunctionBox();
  void removeSubtotalLines();
  bool addSubtotal( int mainCol, int column, int row, int topRow, 
                    bool addRow, QString const & text );
};

#endif

