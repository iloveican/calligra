/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KO_CTL_TEMPLATES_REGISTRY_H_
#define _KO_CTL_TEMPLATES_REGISTRY_H_

#include <QList>

namespace OpenCTL
{
class Template;
}

class KoCtlTemplatesRegistry
{
    KoCtlTemplatesRegistry();
    ~KoCtlTemplatesRegistry();
public:
    static const KoCtlTemplatesRegistry* instance();
    const QList<OpenCTL::Template*>& compositeOps() const;
private:
    struct Private;
    Private* const d;
};

#endif
