/*
    KoReport report engine for Calligra Office>
    Copyright (C) 2011  Adam Pigg <adam@piggz.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef KOREPORTASYNCITEMBASE_H
#define KOREPORTASYNCITEMBASE_H

#include <KoReportItemBase.h>



class KOREPORT_EXPORT KoReportASyncItemBase : public KoReportItemBase
{
    Q_OBJECT
public:
    
    virtual int render(OROPage* page, OROSection* section,  QPointF offset, QVariant data, KRScriptHandler *script) = 0;
    
protected:    
    
signals:
    void finishedRendering();
    
public:
    
private:
    
};

#endif // KOREPORTASYNCITEMBASE_H
