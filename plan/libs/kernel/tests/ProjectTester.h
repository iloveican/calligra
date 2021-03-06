/* This file is part of the KDE project
   Copyright (C) 2007 Dag Andersen <danders@get2net.dk>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KPlato_ProjectTester_h
#define KPlato_ProjectTester_h

#include <QtTest>

#include "kptproject.h"
#include "kptdatetime.h"
#include <KTempDir>

namespace QTest
{
    template<>
            char *toString(const KPlato::DateTime &dt)
    {
        return toString( dt.toString() );
    }
}

namespace KPlato
{

class Task;

class ProjectTester : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    
    void testAddTask();
    void testTakeTask();
    void testTaskAddCmd();
    void testTaskDeleteCmd();

    void schedule();
    void scheduleFullday();
    void scheduleFulldayDstSpring();
    void scheduleFulldayDstFall();

    void scheduleWithExternalAppointments();

    void reschedule();

    void materialResource();
    void requiredResource();

    void resourceWithLimitedAvailability();
    void unavailableResource();
    
    void team();

    // NOTE: It's not *mandatory* to schedule in wbs order but users expect it, so we'll try.
    // This test can be removed if for some important reason this isn't possible.
    void inWBSOrder();

    void resourceConflictALAP();
    void resourceConflictMustStartOn();
    void resourceConflictMustFinishOn();
    void fixedInterval();
    void estimateDuration();

private:
    Project *m_project;
    Calendar *m_calendar;
    Task *m_task;
    KTempDir m_tmp;
};

} //namespace KPlato

#endif
