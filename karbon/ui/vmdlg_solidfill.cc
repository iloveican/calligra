/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#include <klocale.h>

#include <qlayout.h>
#include <kcolordlg.h>
#include <qlabel.h>
#include <qspinbox.h>

#include "vmdlg_solidfill.h"

VMDlgSolidFill::VMDlgSolidFill() : QTabDialog ( 0L, 0, true )
{
	setCaption(i18n( "Solid Fill" ));
	setCancelButton();
	QGridLayout *mainLayout;

	mRGBWidget = new QWidget( this );
	mainLayout = new QGridLayout(mRGBWidget, 6, 2);
	mColorSelector = new KHSSelector( mRGBWidget );
	mColorSelector->setMinimumHeight(255);
	mainLayout->addMultiCellWidget (mColorSelector, 0, 0, 0, 5);

	//RGB
	QLabel *mRedText = new QLabel(i18n("R:"), mRGBWidget);
	mRedText->setAlignment(Right);
	mainLayout->addWidget(mRedText, 1, 0);
	mRed = new QSpinBox( 0, 255, 1, mRGBWidget);
	mainLayout->addWidget(mRed, 1, 1);
	QLabel *mGreenText = new QLabel(i18n("G:"), mRGBWidget);
	mGreenText->setAlignment(Right);
	mainLayout->addWidget(mGreenText, 1, 2);
	mGreen = new QSpinBox( 0, 255, 1, mRGBWidget);
	mainLayout->addWidget(mGreen, 1, 3);
	QLabel *mBlueText = new QLabel(i18n("B:"), mRGBWidget);
	mBlueText->setAlignment(Right);
	mainLayout->addWidget(mBlueText, 1, 4);
	mBlue = new QSpinBox( 0, 255, 1, mRGBWidget);
	mainLayout->addWidget(mBlue, 1, 5);
	
	//HSV
	QLabel *mHueText = new QLabel(i18n("H:"), mRGBWidget);
	mHueText->setAlignment(Right);
	mainLayout->addWidget(mHueText, 2, 0);
	mHue = new QSpinBox( 0, 359, 1, mRGBWidget);
	mainLayout->addWidget(mHue, 2, 1);
	QLabel *mSatText = new QLabel(i18n("S:"), mRGBWidget);
	mSatText->setAlignment(Right);
	mainLayout->addWidget(mSatText, 2, 2);
	mSaturation = new QSpinBox( 0, 255, 1, mRGBWidget);
	mainLayout->addWidget(mSaturation, 2, 3);
	QLabel *mBrText = new QLabel(i18n("V:"), mRGBWidget);
	mBrText->setAlignment(Right);
	mainLayout->addWidget(mBrText, 2, 4);
	mBrightness = new QSpinBox( 0, 255, 1, mRGBWidget);
	mainLayout->addWidget(mBrightness, 2, 5);

	mainLayout->activate();

	addTab(mRGBWidget, i18n("RGB"));
	
	setFixedSize(baseSize());
}

#include "vmdlg_solidfill.moc"
