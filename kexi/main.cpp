/* This file is part of the KDE project
    
    begin                : Sun Jun  9 12:15:11 CEST 2002
    copyright            : (C) 2003 by lucijan busch, Joseph Wenninger
    email                : lucijan@gmx.at, jowenn@kde.org
   
   Copyright (C) 2003-2005 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <kapplication.h>
#include <dcopclient.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kiconloader.h>
#include <kstandarddirs.h>

#include "core/kexiproject.h"
#include "core/kexidialogbase.h"
#include "core/kexi.h"
#include "main/keximainwindowimpl.h"
#include "main/startup/KexiStartup.h"

extern "C" int kdemain(int argc, char *argv[])
{
#ifdef CUSTOM_VERSION
# include "custom_startup.h"
#endif
	Kexi::initCmdLineArgs( argc, argv );

	bool GUIenabled = true;
	QWidget *dummyWidget = 0; //needed to have icon for dialogs before KexiMainWindowImpl is created
//! @todo switch GUIenabled off when needed
	KApplication app(true, GUIenabled);
#ifdef KEXI_STANDALONE
	KGlobal::locale()->removeCatalogue("kexi");
	KGlobal::locale()->insertCatalogue("standalone_kexi");
#endif

	if (GUIenabled) {
		dummyWidget = new QWidget();
		dummyWidget->setIcon( DesktopIcon( "kexi" ) );
		app.setMainWidget(dummyWidget);
	}

	tristate res = Kexi::startupHandler().init(argc, argv);
	if (!res)
		return 1;
	if (~res)
		return 0;
	
	kdDebug() << "startupActions OK" <<endl;

	/* Exit requested, e.g. after database removing. */
	if (Kexi::startupHandler().action() == KexiStartupData::Exit)
		return 0;

	KexiMainWindowImpl *win = new KexiMainWindowImpl();

	if (true != win->startup()) {
		delete win;
		return 1;
	}

	app.setMainWidget(win);
	delete dummyWidget;
	win->show();
	app.processEvents();//allow refresh our app

#ifdef CUSTOM_VERSION
# include "custom_exec.h"
#endif

	return app.exec();
}

