/* This file is part of the KDE project
   Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#include "KoPAPastePage.h"

#include <QBuffer>
#include <QString>
#include <KoOdfReadStore.h>
#include <KoXmlWriter.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>
#include <KoEmbeddedDocumentSaver.h>
#include "KoPALoadingContext.h"
#include "KoPADocument.h"
#include "KoPAMasterPage.h"
#include "KoPASavingContext.h"
#include "commands/KoPAPageInsertCommand.h"

#include <kdebug.h>

KoPAPastePage::KoPAPastePage( KoPADocument * doc, KoPAPageBase * activePage )
: m_doc( doc )
, m_activePage( activePage )
{
}

bool KoPAPastePage::process( const KoXmlElement & body, KoOdfReadStore & odfStore )
{
    KoOdfLoadingContext loadingContext( odfStore.styles(), odfStore.store(), m_doc->componentData() );
    KoPALoadingContext paContext(loadingContext, m_doc->resourceManager());

    QList<KoPAPageBase *> newMasterPages( m_doc->loadOdfMasterPages( odfStore.styles().masterPages(), paContext ) );
    QList<KoPAPageBase *> newPages( m_doc->loadOdfPages( body, paContext ) );

    // Check where to start inserting pages
    KoPAPageBase * insertAfterPage = 0;
    KoPAPageBase * insertAfterMasterPage = 0;
    if ( dynamic_cast<KoPAMasterPage *>( m_activePage ) || ( m_activePage == 0 && newPages.empty() ) ) {
        insertAfterMasterPage = m_activePage;
        insertAfterPage = m_doc->pages( false ).last();
    }
    else {
        insertAfterPage = m_activePage;
        insertAfterMasterPage = m_doc->pages( true ).last();
    }

    if ( !newPages.empty() ) {
        KoGenStyles mainStyles;
        QBuffer buffer;
        buffer.open( QIODevice::WriteOnly );
        KoXmlWriter xmlWriter( &buffer );
        KoEmbeddedDocumentSaver embeddedSaver;
        KoPASavingContext savingContext(xmlWriter, mainStyles, embeddedSaver, 1);
        savingContext.addOption( KoShapeSavingContext::UniqueMasterPages );
        QList<KoPAPageBase*> emptyList;
        QList<KoPAPageBase*> existingMasterPages = m_doc->pages( true );
        savingContext.setClearDrawIds( true );
        m_doc->saveOdfPages( savingContext, emptyList, existingMasterPages );

        QMap<QString, KoPAMasterPage*> masterPageNames;

        foreach ( KoPAPageBase * page, existingMasterPages )
        {
            KoPAMasterPage * masterPage = dynamic_cast<KoPAMasterPage*>( page );
            Q_ASSERT( masterPage );
            if ( masterPage ) {
                QString masterPageName( savingContext.masterPageName( masterPage ) );
                if ( !masterPageNames.contains( masterPageName ) ) {
                    masterPageNames.insert( masterPageName, masterPage );
                }
            }

        }

        m_doc->saveOdfPages( savingContext, emptyList, newMasterPages );

        QMap<KoPAMasterPage*, KoPAMasterPage*> masterPagesToUpdate;
        foreach ( KoPAPageBase * page, newMasterPages )
        {
            KoPAMasterPage * masterPage = dynamic_cast<KoPAMasterPage*>( page );
            Q_ASSERT( masterPage );
            if ( masterPage ) {
                QString masterPageName( savingContext.masterPageName( masterPage ) );
                QMap<QString, KoPAMasterPage*>::const_iterator existingMasterPage( masterPageNames.constFind( masterPageName ) );
                if ( existingMasterPage != masterPageNames.constEnd() ) {
                    masterPagesToUpdate.insert( masterPage, existingMasterPage.value() );
                }
            }
        }

        // update pages which have a duplicate master page
        foreach ( KoPAPageBase * page, newPages )
        {
            KoPAPage * p = dynamic_cast<KoPAPage*>( page );
            Q_ASSERT( p );
            if ( p ) {
                KoPAMasterPage * masterPage( p->masterPage() );
                QMap<KoPAMasterPage*, KoPAMasterPage*>::const_iterator pageIt( masterPagesToUpdate.constFind( masterPage ) );
                if ( pageIt != masterPagesToUpdate.constEnd() ) {
                    p->setMasterPage( pageIt.value() );
                }
            }
        }

        // delete duplicate master pages;
        QMap<KoPAMasterPage*, KoPAMasterPage*>::const_iterator pageIt( masterPagesToUpdate.constBegin() );
        for ( ; pageIt != masterPagesToUpdate.constEnd(); ++pageIt )
        {
            newMasterPages.removeAll( pageIt.key() );
            delete pageIt.key();
        }
    }

    KUndo2Command * cmd = 0;
    if ( m_doc->pageType() == KoPageApp::Slide ) {
        cmd = new KUndo2Command( i18ncp( "(qtundo-format)", "Paste Slide", "Paste Slides", qMax( newMasterPages.size(), newPages.size() ) ) );
    }
    else {
        cmd = new KUndo2Command( i18ncp( "(qtundo-format)", "Paste Page", "Paste Pages", qMax( newMasterPages.size(), newPages.size() ) ) );
    }

    foreach( KoPAPageBase * masterPage, newMasterPages )
    {
        new KoPAPageInsertCommand( m_doc, masterPage, insertAfterMasterPage, cmd );
        insertAfterMasterPage = masterPage;
    }

    foreach( KoPAPageBase * page, newPages )
    {
        new KoPAPageInsertCommand( m_doc, page, insertAfterPage, cmd );
        insertAfterPage = page;
    }

    m_doc->addCommand( cmd );

    return true;
}
