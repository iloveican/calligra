/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_COORDINATES_CONVERTER_TEST_H
#define KIS_COORDINATES_CONVERTER_TEST_H

#include <QtTest>

class KisCoordinatesConverterTest : public QObject
{
    Q_OBJECT

private slots:
    void testConversion();
    void testImageCropping();
    void testTransformations();
    void testConsistency();
    void testRotation();
    void testMirroring();
    void testMirroringCanvasBiggerThanImage();
};

#endif /* KIS_COORDINATES_CONVERTER_TEST_H */

