/* This file is part of the KDE project
   Copyright (C) 1999,2000,2001 Montel Laurent <lmontel@mandrakesoft.com>

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



#include "kspread_dlg_list.h"
#include "kspread_view.h"
#include "kspread_table.h"
#include "kspread_undo.h"
#include "kspread_doc.h"
#include "kspread_autofill.h"

#include <kbuttonbox.h>


#include <kapp.h>
#include <klocale.h>
#include <qstringlist.h>
#include <qlayout.h>
#include <qgrid.h>



KSpreadList::KSpreadList( KSpreadView* parent, const char* name )
	: QDialog( parent, name,TRUE )
{
  QGridLayout *grid1 = new QGridLayout(this,10,3,15,7);  
  setCaption( i18n("Sort lists") );

  QLabel *lab=new QLabel(this);
  lab->setText(i18n("List:" ));    
  grid1->addWidget(lab,0,0);

  list=new QListBox(this);
  grid1->addMultiCellWidget(list,1,8,0,0);
  

  lab=new QLabel(this);
  lab->setText(i18n("Entry:" ));    
  grid1->addWidget(lab,0,1);
  
  entryList=new QMultiLineEdit(this);
  grid1->addMultiCellWidget(entryList,1,8,1,1);
  
  m_pRemove=new QPushButton(i18n("Remove"),this);
  grid1->addWidget(m_pRemove,3,2);

  m_pAdd=new QPushButton(i18n("Add"),this);
  grid1->addWidget(m_pAdd,1,2);

  m_pNew=new QPushButton(i18n("New"),this);
  grid1->addWidget(m_pNew,2,2);

  m_pModify=new QPushButton(i18n("Modify"),this);
  grid1->addWidget(m_pModify,4,2);

  m_pCopy=new QPushButton(i18n("Copy"),this);
  grid1->addWidget(m_pCopy,5,2);


  KButtonBox *bb = new KButtonBox( this );
  bb->addStretch();
  m_pOk = bb->addButton( i18n("OK") );
  m_pOk->setDefault( TRUE );
  m_pClose = bb->addButton( i18n( "Close" ) );
  bb->layout();
  grid1->addWidget(bb,9,1);
  
  m_pAdd->setEnabled(false);
  
  connect( m_pOk, SIGNAL( clicked() ), this, SLOT( slotOk() ) );
  connect( m_pClose, SIGNAL( clicked() ), this, SLOT( slotClose() ) );
  connect( m_pRemove, SIGNAL( clicked() ), this, SLOT( slotRemove() ) );
  connect( m_pAdd, SIGNAL( clicked() ), this, SLOT( slotAdd() ) );
  connect( m_pNew, SIGNAL( clicked() ), this, SLOT( slotNew() ) );
  connect( m_pModify, SIGNAL( clicked() ), this, SLOT( slotModify() ) );
  connect( m_pCopy, SIGNAL( clicked() ), this, SLOT( slotCopy() ) );
  connect( list, SIGNAL(doubleClicked(QListBoxItem *)),this,SLOT(slotDoubleClicked(QListBoxItem *)));
  init();
  entryList->setEnabled(false);
  m_pModify->setEnabled(false);
  resize( 600, 250 );

}

void KSpreadList::init()
{
  config = KSpreadFactory::global()->config(); 
  config->setGroup( "Parameters" );
  QStringList other=config->readListEntry("Other list");
  QString tmp;
  QStringList lst;
  for ( QStringList::Iterator it = other.begin(); it != other.end();++it ) 
    {
      if((*it)!="\\")
	tmp+=(*it)+", ";
      else if( it!=other.begin())
	{
	  tmp=tmp.left(tmp.length()-2);
	  lst.append(tmp);
	  tmp="";
	}
    }
  list->insertStringList(lst);  
}

void KSpreadList::slotDoubleClicked(QListBoxItem *)
{
  QString tmp=list->currentText();
  entryList->setText("");
  QStringList result=result.split(", ",tmp);
  int index=0;
  for ( QStringList::Iterator it = result.begin(); it != result.end();++it ) 
    {
      entryList->insertLine((*it),index);
      index++;
    }
  entryList->setEnabled(true);
  m_pModify->setEnabled(true);
}

void KSpreadList::slotAdd()
{
  m_pAdd->setEnabled(false);
  list->setEnabled(true);
  QString tmp;
  for(int i=0;i<entryList->numLines();i++)
    {
      if(!entryList->textLine(i).isEmpty())
	{
	  if(tmp.isEmpty())
	    tmp=entryList->textLine(i);
	  else
	    tmp+=", "+entryList->textLine(i);
	}
    }
  if(!tmp.isEmpty())
    list->insertItem(tmp,list->count());

  entryList->setText("");
  entryList->setEnabled(false);
  entryList->setFocus();
}

void KSpreadList::slotNew()
{
  m_pAdd->setEnabled(true);
  list->setEnabled(false);
  entryList->setText(""); 
  entryList->setEnabled(true);
}

void KSpreadList::slotRemove()
{
  list->removeItem(list->currentItem ());
}

void KSpreadList::slotOk()
{
  QStringList result;
  result.append("\\");
  for(unsigned int i=0;i<list->count();i++)
    {
      QStringList tmp=result.split(", ",list->text(i));
      result+=tmp;
      result+="\\";
    }
  config->setGroup( "Parameters" );
  config->writeEntry("Other list",result);
  //todo refresh AutoFillSequenceItem::other
  // I don't know how to do for the moment
  if(AutoFillSequenceItem::other!=0L)
    {
    delete(AutoFillSequenceItem::other);
    AutoFillSequenceItem::other=0L;
    }
  accept();
}

void KSpreadList::slotModify()
{
  
  if(list->currentItem ()!=-1 && !entryList->text().isEmpty())
    {
      QString tmp;
      for(int i=0;i<entryList->numLines();i++)
	{
	  if(!entryList->textLine(i).isEmpty())
	    {
	      if(tmp.isEmpty())
		tmp=entryList->textLine(i);
	      else
		tmp+=", "+entryList->textLine(i);
	    }
	}
      list->insertItem(tmp,list->currentItem());
      list->removeItem(list->currentItem());


      entryList->setText("");
      
    }
  entryList->setEnabled(false);
  m_pModify->setEnabled(false);
  
}

void KSpreadList::slotCopy()
{
  if(list->currentItem()!=-1)
    {
      list->insertItem(list->currentText(),list->count()); 
    }
}

void KSpreadList::slotClose()
{
  reject();
}


#include "kspread_dlg_list.moc"
