/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef TESTSHAPEREORDERCOMMAND_H
#define TESTSHAPEREORDERCOMMAND_H

#include <QtTest>

class TestShapeReorderCommand : public QObject
{
    Q_OBJECT
public:
    TestShapeReorderCommand();
    ~TestShapeReorderCommand();
private slots:
    void testZIndexSorting();
    void testRunThroughSorting();
    void testParentChildSorting();

    void testBringToFront();
    void testSendToBack();
    void testMoveUp();
    void testMoveDown();
    void testMoveUpOverlapping();
    void testMoveDownOverlapping();
    void testSendToBackChildren();
    void testNoCommand();
};

#endif // TESTSHAPEREORDERCOMMAND_H
