/******************************************************************/ 
/* KWord - (c) by Reginald Stadlbauer and Torben Weis 1997-1998   */
/* Version: 0.0.1                                                 */
/* Author: Reginald Stadlbauer, Torben Weis                       */
/* E-Mail: reggie@kde.org, weis@kde.org                           */
/* Homepage: http://boch35.kfunigraz.ac.at/~rs                    */
/* needs c++ library Qt (http://www.troll.no)                     */
/* written for KDE (http://www.kde.org)                           */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* License: GNU GPL                                               */
/******************************************************************/
/* Module: Stylist                                                */
/******************************************************************/

#include "kword_doc.h"
#include "paraglayout.h"
#include "format.h"
#include "font.h"

#include "stylist.h"
#include "stylist.moc"

/******************************************************************/
/* Class: KWStyleManager                                          */
/******************************************************************/

/*================================================================*/
KWStyleManager::KWStyleManager(QWidget *_parent,KWordDocument *_doc,QStrList _fontList)
  : QTabDialog(_parent,"",true)
{
  fontList = _fontList;
  doc = _doc;
  editor = 0L;

  setupTab1();
  setupTab2();

  setCancelButton(i18n("Cancel"));
  setOkButton(i18n("OK"));

  connect(this,SIGNAL(applyButtonPressed()),this,SLOT(apply()));
}

/*================================================================*/
void KWStyleManager::setupTab1()
{
  tab1 = new QWidget(this);

  grid1 = new QGridLayout(tab1,1,2,15,7);

  lStyleList = new QListBox(tab1);
  for (unsigned int i = 0;i < doc->paragLayoutList.count();i++)
    lStyleList->insertItem(doc->paragLayoutList.at(i)->getName());
  connect(lStyleList,SIGNAL(selected(int)),this,SLOT(editStyle(int)));
  lStyleList->setCurrentItem(0);
  grid1->addWidget(lStyleList,0,0);

  bButtonBox = new KButtonBox(tab1,KButtonBox::VERTICAL);
  bAdd = bButtonBox->addButton(i18n("&Add.."),false);
  bDelete = bButtonBox->addButton(i18n("&Delete"),false);
  bButtonBox->addStretch();
  bEdit = bButtonBox->addButton(i18n("&Edit..."),false);
  connect(bEdit,SIGNAL(clicked()),this,SLOT(editStyle()));
  bCopy = bButtonBox->addButton(i18n("&Copy..."),false); 
  bButtonBox->addStretch();
  bUp = bButtonBox->addButton(i18n("Up"),false); 
  bDown = bButtonBox->addButton(i18n("D&own"),false); 
  bButtonBox->layout();
  grid1->addWidget(bButtonBox,0,1);

  grid1->addColSpacing(0,lStyleList->width());
  grid1->addColSpacing(1,bButtonBox->width());
  grid1->setColStretch(0,1);

  grid1->addRowSpacing(0,lStyleList->height());
  grid1->addRowSpacing(0,bButtonBox->height());
  grid1->setRowStretch(0,1);

  grid1->activate();

  addTab(tab1,i18n("Style Manager"));
}

/*================================================================*/
void KWStyleManager::setupTab2()
{
  tab2 = new QWidget(this);

  grid2 = new QGridLayout(tab2,4,2,15,7);

  cFont = new QComboBox(false,tab2);
  cFont->insertItem(i18n("Don't update Fonts"));
  cFont->insertItem(i18n("Update Font Families only"));
  cFont->insertItem(i18n("Update Font Families and Attributes"));
  cFont->resize(cFont->sizeHint());
  grid2->addWidget(cFont,0,0);

  cColor = new QComboBox(false,tab2);
  cColor->insertItem(i18n("Don't update Colors"));
  cColor->insertItem(i18n("Update Colors"));
  cColor->resize(cFont->sizeHint());
  grid2->addWidget(cColor,1,0);

  cIndent = new QComboBox(false,tab2);
  cIndent->insertItem(i18n("Don't update Indents and Spacing"));
  cIndent->insertItem(i18n("Update Indents and Spacing"));
  cIndent->resize(cFont->sizeHint());
  grid2->addWidget(cIndent,2,0);

  cAlign = new QComboBox(false,tab2);
  cAlign->insertItem(i18n("Don't update Aligns/Flows"));
  cAlign->insertItem(i18n("Update Aligns/Flows"));
  cAlign->resize(cFont->sizeHint());
  grid2->addWidget(cAlign,0,1);

  cNumbering = new QComboBox(false,tab2);
  cNumbering->insertItem(i18n("Don't update Numbering"));
  cNumbering->insertItem(i18n("Update Numbering"));
  cNumbering->resize(cFont->sizeHint());
  grid2->addWidget(cNumbering,1,1);

  cBorder = new QComboBox(false,tab2);
  cBorder->insertItem(i18n("Don't update Borders"));
  cBorder->insertItem(i18n("Update Borders"));
  cBorder->resize(cFont->sizeHint());
  grid2->addWidget(cBorder,2,1);

  grid2->addColSpacing(0,cFont->width());
  grid2->addColSpacing(1,cAlign->width());
  grid2->addColSpacing(0,cColor->width());
  grid2->addColSpacing(1,cNumbering->width());
  grid2->addColSpacing(0,cIndent->width());
  grid2->addColSpacing(1,cBorder->width());
  grid2->setColStretch(0,1);
  grid2->setColStretch(1,1);

  grid2->addRowSpacing(0,cFont->height());
  grid2->addRowSpacing(1,cColor->height());
  grid2->addRowSpacing(2,cIndent->height());
  grid2->setRowStretch(0,0);
  grid2->setRowStretch(1,0);
  grid2->setRowStretch(2,0);
  grid2->setRowStretch(3,1);

  grid2->activate();

  addTab(tab2,i18n("Update Configuration"));

  if (doc->getApplyStyleTemplate() & KWordDocument::U_FONT_FAMILY)
    cFont->setCurrentItem(1);
  if (doc->getApplyStyleTemplate() & KWordDocument::U_FONT_ALL)
    cFont->setCurrentItem(2);
  if (doc->getApplyStyleTemplate() & KWordDocument::U_COLOR)
    cColor->setCurrentItem(1);
  if (doc->getApplyStyleTemplate() & KWordDocument::U_INDENT)
    cIndent->setCurrentItem(1);
  if (doc->getApplyStyleTemplate() & KWordDocument::U_BORDER)
    cBorder->setCurrentItem(1);
  if (doc->getApplyStyleTemplate() & KWordDocument::U_ALIGN)
    cAlign->setCurrentItem(1);
  if (doc->getApplyStyleTemplate() & KWordDocument::U_NUMBERING)
    cNumbering->setCurrentItem(1);
}

/*================================================================*/
void KWStyleManager::editStyle()
{
  if (editor)
    {
      delete editor;
      editor = 0L;
    }

  editor = new KWStyleEditor(0L,doc->paragLayoutList.at(lStyleList->currentItem()),doc,fontList);
  editor->setCaption(i18n("KWord - Stylist"));
  editor->show();
}

/*================================================================*/
void KWStyleManager::apply()
{
  int f = 0;
  
  if (cFont->currentItem() == 1)
    f = f | KWordDocument::U_FONT_FAMILY;
  else if (cFont->currentItem() == 2)
    f = f | KWordDocument::U_FONT_ALL;

  if (cColor->currentItem() == 1)
    f = f | KWordDocument::U_COLOR;
  if (cAlign->currentItem() == 1)
    f = f | KWordDocument::U_ALIGN;
  if (cBorder->currentItem() == 1)
    f = f | KWordDocument::U_BORDER;
  if (cNumbering->currentItem() == 1)
    f = f | KWordDocument::U_NUMBERING;
  if (cIndent->currentItem() == 1)
    f = f | KWordDocument::U_INDENT;

  doc->setApplyStyleTemplate(f);
}

/******************************************************************/
/* Class: KWStylePreview                                          */
/******************************************************************/

/*================================================================*/
void KWStylePreview::drawContents(QPainter *painter)
{
  QRect r = contentsRect();
  QFontMetrics fm(font());

  painter->fillRect(r.x() + fm.width('W'),r.y() + fm.height(),r.width() - 2 * fm.width('W'),r.height() - 2 * fm.height(),white);
  painter->setClipRect(r.x() + fm.width('W'),r.y() + fm.height(),r.width() - 2 * fm.width('W'),r.height() - 2 * fm.height());

  QFont f(style->getFormat().getUserFont()->getFontName(),style->getFormat().getPTFontSize());
  f.setBold(style->getFormat().getWeight() == 75 ? true : false);
  f.setItalic(static_cast<bool>(style->getFormat().getItalic()));
  f.setUnderline(static_cast<bool>(style->getFormat().getUnderline()));

  QColor c(style->getFormat().getColor());

  painter->setPen(QPen(c));
  painter->setFont(f);

  fm = QFontMetrics(f);
  int y = height() / 2 - fm.height() / 2;

  painter->drawText(20 + style->getPTFirstLineLeftIndent() + style->getPTLeftIndent(),
		    y,fm.width(i18n("KWord, KOffice's Wordprocessor")),fm.height(),0,i18n("KWord, KOffice's Wordprocessor"));
}

/******************************************************************/
/* Class: KWStyleEditor                                           */
/******************************************************************/

/*================================================================*/
KWStyleEditor::KWStyleEditor(QWidget *_parent,KWParagLayout *_style,KWordDocument *_doc,QStrList _fontList)
  : QTabDialog(_parent,"",true)
{
  fontList = _fontList;
  paragDia = 0;
  ostyle = _style;
  style = new KWParagLayout(_doc,false);
  *style = *_style;
  doc = _doc;
  setupTab1();

  setCancelButton(i18n("Cancel"));
  setOkButton(i18n("OK"));

  connect(this,SIGNAL(applyButtonPressed()),this,SLOT(apply()));
}

/*================================================================*/
void KWStyleEditor::setupTab1()
{
  tab1 = new QWidget(this);

  grid1 = new QGridLayout(tab1,3,1,15,7);

  nwid = new QWidget(tab1);
  grid2 = new QGridLayout(nwid,1,2,15,7);

  lName = new QLabel(i18n("Name:"),nwid);
  lName->resize(lName->sizeHint());
  grid2->addWidget(lName,0,0);

  eName = new QLineEdit(nwid);
  eName->resize(eName->sizeHint());
  eName->setText(style->getName());
  grid2->addWidget(eName,0,1);

  if (style->getName() == QString(i18n("Standard")) ||
      style->getName() == QString(i18n("Head 1")) ||
      style->getName() == QString(i18n("Head 2")) ||
      style->getName() == QString(i18n("Head 3")) ||
      style->getName() == QString(i18n("Enumearted List")) ||
      style->getName() == QString(i18n("Bullet List")) ||
      style->getName() == QString(i18n("Alphabetical List")))
    eName->setEnabled(false);

  grid2->addRowSpacing(0,lName->height());
  grid2->addRowSpacing(0,eName->height());

  grid2->addColSpacing(0,lName->width());
  grid2->addColSpacing(1,eName->width());
  grid2->setColStretch(1,1);
  
  grid2->activate();

  grid1->addWidget(nwid,0,0);

  preview = new KWStylePreview(i18n("Preview"),tab1,style);
  grid1->addWidget(preview,1,0);

  bButtonBox = new KButtonBox(tab1);
  bFont = bButtonBox->addButton(i18n("&Font..."),true);
  connect(bFont,SIGNAL(clicked()),this,SLOT(changeFont()));
  bButtonBox->addStretch();
  bColor = bButtonBox->addButton(i18n("&Color..."),true);
  connect(bColor,SIGNAL(clicked()),this,SLOT(changeColor()));
  bButtonBox->addStretch();
  bSpacing = bButtonBox->addButton(i18n("&Spacing and Indents..."),true);
  connect(bSpacing,SIGNAL(clicked()),this,SLOT(changeSpacing()));
  bButtonBox->addStretch();
  bAlign = bButtonBox->addButton(i18n("&Alignment..."),true);
  connect(bAlign,SIGNAL(clicked()),this,SLOT(changeAlign()));
  bButtonBox->addStretch();
  bBorders = bButtonBox->addButton(i18n("&Borders..."),true);
  connect(bBorders,SIGNAL(clicked()),this,SLOT(changeBorders()));
  bButtonBox->addStretch();
  bNumbering = bButtonBox->addButton(i18n("&Numbering..."),true);
  connect(bNumbering,SIGNAL(clicked()),this,SLOT(changeNumbering()));
  bButtonBox->layout();
  grid1->addWidget(bButtonBox,2,0);

  grid1->addColSpacing(0,nwid->width());
  grid1->addColSpacing(0,preview->width());
  grid1->addColSpacing(0,bButtonBox->width());
  grid1->setColStretch(0,1);

  grid1->addRowSpacing(0,nwid->height());
  grid1->addRowSpacing(1,100);
  grid1->addRowSpacing(2,bButtonBox->height());
  grid1->setRowStretch(1,1);

  grid1->activate();

  addTab(tab1,i18n("Style Editor"));
}

/*================================================================*/
void KWStyleEditor::changeFont()
{
  QFont f(style->getFormat().getUserFont()->getFontName(),style->getFormat().getPTFontSize());
  f.setBold(style->getFormat().getWeight() == 75 ? true : false);
  f.setItalic(static_cast<bool>(style->getFormat().getItalic()));
  f.setUnderline(static_cast<bool>(style->getFormat().getUnderline()));

  if (KFontDialog::getFont(f))
    {
      style->getFormat().setUserFont(new KWUserFont(doc,f.family()));
      style->getFormat().setPTFontSize(f.pointSize());
      style->getFormat().setWeight(f.bold() ? 75 : 50);
      style->getFormat().setItalic(static_cast<int>(f.italic()));
      style->getFormat().setUnderline(static_cast<int>(f.underline()));
      preview->repaint(true);
    }
}

/*================================================================*/
void KWStyleEditor::changeColor()
{
  QColor c(style->getFormat().getColor());
  if (KColorDialog::getColor(c))
    {
      style->getFormat().setColor(c);
      preview->repaint(true);
    }
}

/*================================================================*/
void KWStyleEditor::changeSpacing()
{
  if (paragDia)
    {
      disconnect(paragDia,SIGNAL(applyButtonPressed()),this,SLOT(paragDiaOk()));
      paragDia->close();
      delete paragDia;
      paragDia = 0;
    }
  paragDia = new KWParagDia(0,"",fontList,KWParagDia::PD_SPACING);
  paragDia->setCaption(i18n("KWord - Paragraph Spacing"));
  connect(paragDia,SIGNAL(applyButtonPressed()),this,SLOT(paragDiaOk()));
  paragDia->setSpaceBeforeParag(style->getMMParagHeadOffset());
  paragDia->setSpaceAfterParag(style->getMMParagFootOffset());
  paragDia->setLineSpacing(style->getPTLineSpacing());
  paragDia->setLeftIndent(style->getMMLeftIndent());
  paragDia->setFirstLineIndent(style->getMMFirstLineLeftIndent());
  paragDia->show();
}

/*================================================================*/
void KWStyleEditor::changeAlign()
{
  if (paragDia)
    {
      disconnect(paragDia,SIGNAL(applyButtonPressed()),this,SLOT(paragDiaOk()));
      paragDia->close();
      delete paragDia;
      paragDia = 0;
    }
  paragDia = new KWParagDia(0,"",fontList,KWParagDia::PD_FLOW);
  paragDia->setCaption(i18n("KWord - Paragraph Flow (Alignment)"));
  connect(paragDia,SIGNAL(applyButtonPressed()),this,SLOT(paragDiaOk()));
  paragDia->setFlow(style->getFlow());
  paragDia->show();
}

/*================================================================*/
void KWStyleEditor::changeBorders()
{
  if (paragDia)
    {
      disconnect(paragDia,SIGNAL(applyButtonPressed()),this,SLOT(paragDiaOk()));
      paragDia->close();
      delete paragDia;
      paragDia = 0;
    }
  paragDia = new KWParagDia(0,"",fontList,KWParagDia::PD_BORDERS);
  paragDia->setCaption(i18n("KWord - Paragraph Borders"));
  connect(paragDia,SIGNAL(applyButtonPressed()),this,SLOT(paragDiaOk()));
  paragDia->setLeftBorder(style->getLeftBorder());
  paragDia->setRightBorder(style->getRightBorder());
  paragDia->setTopBorder(style->getTopBorder());
  paragDia->setBottomBorder(style->getBottomBorder());
  paragDia->show();
}

/*================================================================*/
void KWStyleEditor::changeNumbering()
{
  if (paragDia)
    {
      disconnect(paragDia,SIGNAL(applyButtonPressed()),this,SLOT(paragDiaOk()));
      paragDia->close();
      delete paragDia;
      paragDia = 0;
    }
  paragDia = new KWParagDia(0,"",fontList,KWParagDia::PD_NUMBERING);
  paragDia->setCaption(i18n("KWord - Numbering"));
  connect(paragDia,SIGNAL(applyButtonPressed()),this,SLOT(paragDiaOk()));
  paragDia->setCounter(style->getCounter());
  paragDia->show();
}

/*================================================================*/
void KWStyleEditor::paragDiaOk()
{
  switch (paragDia->getFlags())
    {
    case KWParagDia::PD_SPACING:
      {
	style->setMMParagHeadOffset(static_cast<int>(paragDia->getSpaceBeforeParag()));
	style->setMMParagFootOffset(static_cast<int>(paragDia->getSpaceAfterParag()));
	style->setPTLineSpacing(static_cast<int>(paragDia->getLineSpacing()));
	style->setMMLeftIndent(static_cast<int>(paragDia->getLeftIndent()));
	style->setMMFirstLineLeftIndent(static_cast<int>(paragDia->getFirstLineIndent()));
      } break;
    case KWParagDia::PD_FLOW:
      style->setFlow(paragDia->getFlow());
      break;
    case KWParagDia::PD_BORDERS:
      {
	style->setLeftBorder(paragDia->getLeftBorder());
	style->setRightBorder(paragDia->getRightBorder());
	style->setTopBorder(paragDia->getTopBorder());
	style->setBottomBorder(paragDia->getBottomBorder());
      } break;
    case KWParagDia::PD_NUMBERING:
      style->setCounter(paragDia->getCounter());
      break;
    default: break;
    }

  preview->repaint(true);
}
/*================================================================*/
void KWStyleEditor::apply()
{ 
  *ostyle = *style; 
}
