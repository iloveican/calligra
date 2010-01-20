/*
* This file is part of the KDE project
*
* Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
*
* Contact: Amit Aggarwal <amit.5.aggarwal@nokia.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 or (at your option) any later version as published by
* the Free Software Foundation.
*
* This library is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301 USA
*
*/

#ifndef PRESENTATION_VARIABLE_H
#define PRESENTATION_VARIABLE_H

#include <KoVariable.h>

/**
 * This is a PresentationVariable for document declaration.
 * Ex: - Footer, Header, Date-time etc
 */
class PresentationVariable : public KoVariable
{
public:
    /**
     * Constructor.
     */
    PresentationVariable();

    void setProperties(const KoProperties *props);

    void variableMoved(const KoShape *shape, const QTextDocument *document, int posInDocument);

    ///reimplemented
    void saveOdf(KoShapeSavingContext & context);

    ///reimplemented
    bool loadOdf(const KoXmlElement & element, KoShapeLoadingContext & context);
    
     enum PresentationType{
         Header,
         Footer 
     };

private:
    PresentationType m_type;
};

#endif

