/*
 * $Id$
 *
 * Copyright 2000 by Laurent Montel, released under Artistic License.
 */

#include "kchartFontConfigPage.h"

#include "kchartFontConfigPage.moc"

#include <kapp.h>
#include <klocale.h>


#include <qlayout.h>
#include <qlabel.h>
#include <kfontdialog.h>

KChartFontConfigPage::KChartFontConfigPage(KChartParameters* params,QWidget* parent ) :
    QWidget( parent ),_params( params )
{

   QGridLayout *grid = new QGridLayout(this,5,4,15,7);

   list=new QListBox(this);
   list->resize( list->sizeHint() );
   grid->addMultiCellWidget(list,0,4,0,0);
   fontButton = new QPushButton( this);
   fontButton->setText(i18n("Font"));

   fontButton->resize( fontButton->sizeHint() );
   grid->addWidget( fontButton,2,1);

   listColor=new QListBox(this);
   listColor->resize( listColor->sizeHint() );
   grid->addMultiCellWidget(listColor,0,4,2,2);
   colorButton = new KColorButton( this);

   colorButton->resize( colorButton->sizeHint() );
   grid->addWidget( colorButton,2,3);
   grid->addColSpacing(0,list->width());
   grid->addColSpacing(2,listColor->width());
   //grid->addColSpacing(3,list->width());

   initList();
   connect(fontButton,SIGNAL(clicked()),this,SLOT(changeLabelFont()));
   connect(listColor,SIGNAL(highlighted(int )),this,SLOT(changeIndex(int)));
}

void KChartFontConfigPage::initList()
{
  list->insertItem(i18n("Title"));
  if(!_params->isPie())
  	{
  	list->insertItem(i18n("X-Title"));
  	list->insertItem(i18n("Y-Title"));
  	list->insertItem(i18n("X-Axis"));
  	list->insertItem(i18n("Y-Axis"));
  	}
  list->insertItem(i18n("Label"));
  list->setCurrentItem(0);
  int num=0;
  bool noEnough=false;
  //num <12 because there are 12 colors
  for(QStringList::Iterator it = _params->legend.begin();it  != _params->legend.end();++it,num++)
        {
        if(num<12)
                listColor->insertItem(*it);
        else
                {
                if(!noEnough)
                        {
                        listColor->insertItem(i18n("Not enough color"));
                        noEnough=true;
                        }
                }
        }
  listColor->setCurrentItem(0);
  for(unsigned int i=0;i<_params->legend.count();i++)
        {
        if(i<12)
                extColor.setColor(i,_params->ExtColor.color(i));
        }
  indice=0;
  colorButton->setColor(extColor.color(indice));
}

void KChartFontConfigPage::changeIndex(int index)
{
cout <<"Change Index "<<index<<endl;
if(index>11)
        colorButton->setEnabled(false);
else
        {
        if(!colorButton->isEnabled())
                colorButton->setEnabled(true);
        }
extColor.setColor(indice,colorButton->color());
colorButton->setColor(extColor.color(index));
indice=index;
}

void KChartFontConfigPage::changeLabelFont()
{
if(list->currentText()==i18n("Title"))
	{
 	if (KFontDialog::getFont( title,true,this ) == QDialog::Rejected )
	      return;
	}
else if(list->currentText()==i18n("X-Title"))
	{
	if (KFontDialog::getFont( xtitle,true,this ) == QDialog::Rejected )
	      return;
	}
else if(list->currentText()==i18n("Y-Title"))
	{
	if (KFontDialog::getFont( ytitle,true,this ) == QDialog::Rejected )
	      return;
	}
else if(list->currentText()==i18n("X-Axis"))
	{
	if (KFontDialog::getFont( xaxis,true,this ) == QDialog::Rejected )
	      return;
	}
else if(list->currentText()==i18n("Y-Axis"))
	{
	if (KFontDialog::getFont( yaxis,true,this ) == QDialog::Rejected )
	      return;
	}
else if(list->currentText()==i18n("Label"))
	{
	if (KFontDialog::getFont( label,true,this ) == QDialog::Rejected )
	      return;
	}
else
	{
	cout <<"Pb in listBox\n";
	}


}


void KChartFontConfigPage::init()
{

  title=_params->titleFont();
  xtitle=_params->xTitleFont();
  ytitle=_params->yTitleFont();
  xaxis=_params->xAxisFont();
  yaxis=_params->yAxisFont();
  label=_params->labelFont();

  for(int i=0;i<12;i++)
        {
         extColor.setColor(i,_params->ExtColor.color(i));
        }
  indice=0;
  colorButton->setColor(extColor.color(indice));
}
void KChartFontConfigPage::apply()
{
  _params->setLabelFont(label);
  if(!_params->isPie())
  	{
	_params->setXAxisFont(xaxis);
  	_params->setYAxisFont(yaxis);
  	_params->setXTitleFont(xtitle);
  	_params->setYTitleFont(ytitle);
  	}
  _params->setTitleFont(title);

  extColor.setColor(indice,colorButton->color());
  for(unsigned int i=0;i<extColor.count();i++)
        {
         _params->ExtColor.setColor(i,extColor.color(i));
        }

}
