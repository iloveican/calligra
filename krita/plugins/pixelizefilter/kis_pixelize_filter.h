/*
 * This file is part of the KDE project
 *
 * Copyright (c) Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#ifndef _KIS_PIXELIZE_FILTER_H_
#define _KIS_PIXELIZE_FILTER_H_

#include "kis_filter.h"
#include "kis_view.h"
#include <kdebug.h>

class KisPixelizeFilterConfiguration : public KisFilterConfiguration
{
    public:
                KisPixelizeFilterConfiguration(Q_UINT32 pixelWidth, Q_UINT32 pixelHeight) : m_pixelWidth(pixelWidth), m_pixelHeight(pixelHeight) {};
    public:
                inline Q_UINT32 pixelWidth() { return m_pixelWidth; };
                inline Q_UINT32 pixelHeight() {return m_pixelHeight; };
        private:
                Q_UINT32 m_pixelWidth;
                Q_UINT32 m_pixelHeight;
};

class KisPixelizeFilter : public KisFilter
{
public:
    KisPixelizeFilter();
public:
    virtual void process(KisPaintDeviceImplSP,KisPaintDeviceImplSP, KisFilterConfiguration* , const QRect&);
    static inline KisID id() { return KisID("pixelize", i18n("Pixelize")); };
    virtual bool supportsPainting() { return true; }
    virtual bool supportsPreview() { return true; }
    virtual std::list<KisFilterConfiguration*> listOfExamplesConfiguration(KisPaintDeviceImplSP )
    { std::list<KisFilterConfiguration*> list; list.insert(list.begin(), new KisPixelizeFilterConfiguration(10,10)); return list; }
public:
    virtual KisFilterConfigWidget * createConfigurationWidget(QWidget* parent, KisPaintDeviceImplSP dev);
    virtual KisFilterConfiguration* configuration(QWidget*, KisPaintDeviceImplSP dev);
private:
    void pixelize(KisPaintDeviceImplSP src, KisPaintDeviceImplSP dst, int x, int y, int w, int h, int pixelWidth, int pixelHeight);
};

#endif
