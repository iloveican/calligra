/* This file is part of the KDE project
   Copyright (C) 2002   Lucijan Busch <lucijan@gmx.at>
   Daniel Molkentin <molkentin@kde.org>
   Joseph Wenninger <jowenn@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#include "kexidbdriver.h"

KexiDBDriver::KexiDBDriver(QString driver):m_loaded(false),m_db(0),m_service(0)
{
	m_driver = driver;
}

void
KexiDBDriver::setService(KService::Ptr service)
{
	m_service = service;
}

KexiDB*
KexiDBDriver::db()
{
	return m_db;
}

KService::Ptr
KexiDBDriver::service() const
{
	return m_service;
}

QString
KexiDBDriver::driver()
{
	return m_driver;
}

bool
KexiDBDriver::loaded() const
{
	return m_loaded;
}

KexiDBDriver::~KexiDBDriver()
{
}
