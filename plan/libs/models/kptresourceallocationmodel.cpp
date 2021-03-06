/* This file is part of the KDE project
  Copyright (C) 2009, 2012 Dag Andersen danders@get2net>

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

#include "kptresourceallocationmodel.h"

#include "kptcommonstrings.h"
#include "kptcommand.h"
#include "kptitemmodelbase.h"
#include "kptcalendar.h"
#include "kptduration.h"
#include "kptnode.h"
#include "kptproject.h"
#include "kpttask.h"
#include "kptresource.h"
#include "kptdatetime.h"
#include "kptdebug.h"

#include <QMimeData>
#include <QObject>
#include <QStringList>

#include <kaction.h>
#include <kicon.h>
#include <kglobal.h>
#include <klocale.h>
#include <kactioncollection.h>
#include <kxmlguifactory.h>


namespace KPlato
{

//--------------------------------------

ResourceAllocationModel::ResourceAllocationModel( QObject *parent )
    : QObject( parent ),
    m_project( 0 ),
    m_task( 0 )
{
}

ResourceAllocationModel::~ResourceAllocationModel()
{
}

const QMetaEnum ResourceAllocationModel::columnMap() const
{
    return metaObject()->enumerator( metaObject()->indexOfEnumerator("Properties") );
}

void ResourceAllocationModel::setProject( Project *project )
{
    m_project = project;
}

void ResourceAllocationModel::setTask( Task *task )
{
    m_task = task;
}

int ResourceAllocationModel::propertyCount() const
{
    return columnMap().keyCount();
}

QVariant ResourceAllocationModel::name( const Resource *res, int role ) const
{
    //kDebug(planDbg())<<res->name()<<","<<role;
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            return res->name();
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant ResourceAllocationModel::name( const  ResourceGroup *res, int role ) const
{
    //kDebug(planDbg())<<res->name()<<","<<role;
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            return res->name();
            break;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant ResourceAllocationModel::type( const Resource *res, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            return res->typeToString( true );
        case Role::EnumList:
            return res->typeToStringList( true );
        case Role::EnumListValue:
            return (int)res->type();
        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant ResourceAllocationModel::type( const ResourceGroup *res, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            return res->typeToString( true );
        case Role::EnumList:
            return res->typeToStringList( true );
        case Role::EnumListValue:
            return (int)res->type();
        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}


QVariant ResourceAllocationModel::allocation( const ResourceGroup *group, const Resource *res, int role ) const
{
    if ( m_task == 0 ) {
        return QVariant();
    }
    const ResourceGroupRequest *rg = m_task->requests().find( group );
    const ResourceRequest *rr = 0;
    if ( rg ) {
        rr = rg->find( res );
    }
    switch ( role ) {
        case Qt::DisplayRole: {
            int units = rr ? rr->units() : 0;
            // xgettext: no-c-format
            return i18nc( "<value>%", "%1%", units );
        }
        case Qt::EditRole:
            return rr ? rr->units() : 0;;
        case Qt::ToolTipRole: {
            int units = rr ? rr->units() : 0;
            if ( units == 0 ) {
                return i18nc( "@info:tooltip", "Not allocated" );
            }
            // xgettext: no-c-format
            return i18nc( "@info:tooltip", "Allocated units: %1%", units );
        }
        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
        case Role::Minimum:
            return 0;
        case Role::Maximum:
            return 100;
        case Qt::CheckStateRole:
            return Qt::Unchecked;

    }
    return QVariant();
}

QVariant ResourceAllocationModel::allocation( const ResourceGroup *res, int role ) const
{
    if ( m_task == 0 ) {
        return QVariant();
    }
    const ResourceGroupRequest *req = m_task->requests().find( res );
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return req ? req->units() : 0;
        case Qt::ToolTipRole:
            return QVariant();
        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
        case Role::Minimum:
            return 0;
        case Role::Maximum:
            return res->numResources();
    }
    return QVariant();
}

QVariant ResourceAllocationModel::maximum( const Resource *res, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
            // xgettext: no-c-format
            return i18nc( "<value>%", "%1%", res->units() );
        case Qt::EditRole:
            return res->units();
        case Qt::ToolTipRole:
            // xgettext: no-c-format
            return i18n( "Maximum units available: %1%", res->units() );
        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant ResourceAllocationModel::required( const Resource *res, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole: {
            QStringList lst;
            foreach ( Resource *r, res->requiredResources() ) {
                lst << r->name();
            }
            return lst.join( "," );
        }
        case Qt::EditRole:
            return QVariant();//Not used
        case Qt::ToolTipRole:
            return QVariant();
        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;
        case Qt::StatusTipRole:
            return QVariant();
        case Qt::WhatsThisRole:
            return i18nc( "@info:whatsthis", "<title>Required Resources</title>"
            "<para>A working resource can be assigned to one or more required resources."
            " A required resource is a material resource that the working resource depends on"
            " in order to do the work.</para>"
            "<para>To be able to use a material resource as a required resource, the material resource"
            " must be part of a group of type <emphasis>Material</emphasis>.</para>" );
    }
    return QVariant();
}

QVariant ResourceAllocationModel::maximum( const ResourceGroup *res, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return res->numResources();
        case Qt::ToolTipRole:
            return i18np( "There is %1 resource available in this group", "There are %1 resources available in this group", res->numResources() );
        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant ResourceAllocationModel::data( const ResourceGroup *group, const Resource *resource, int property, int role ) const
{
    QVariant result;
    if ( resource == 0 ) {
        return result;
    }
    switch ( property ) {
        case RequestName: result = name( resource, role ); break;
        case RequestType: result = type( resource, role ); break;
        case RequestAllocation: result = allocation( group, resource, role ); break;
        case RequestMaximum: result = maximum( resource, role ); break;
        case RequestRequired: result = required( resource, role ); break;
        default:
            kDebug(planDbg())<<"data: invalid display value: property="<<property;
            break;
    }
    return result;
}

QVariant ResourceAllocationModel::data( const ResourceGroup *group, int property, int role ) const
{
    QVariant result;
    if ( group == 0 ) {
        return result;
    }
    switch ( property ) {
        case RequestName: result = name( group, role ); break;
        case RequestType: result = type( group, role ); break;
        case RequestAllocation: result = allocation( group, role ); break;
        case RequestMaximum: result = maximum( group, role ); break;
        default:
            if ( role == Qt::DisplayRole ) {
                if ( property < propertyCount() ) {
                    result = "";
                } else {
                    kDebug(planDbg())<<"data: invalid display value column"<<property;;
                    return QVariant();
                }
            }
            break;
    }
    return result;
}


QVariant ResourceAllocationModel::headerData( int section, int role )
{
    if ( role == Qt::DisplayRole ) {
        switch ( section ) {
            case RequestName: return i18n( "Name" );
            case RequestType: return i18n( "Type" );
            case RequestAllocation: return i18n( "Allocation" );
            case RequestMaximum: return i18nc( "@title:column", "Available" );
            case RequestRequired: return i18nc( "@title:column", "Required Resources" );
            default: return QVariant();
        }
    } else if ( role == Qt::TextAlignmentRole ) {
        switch (section) {
            case 0: return QVariant();
            default: return Qt::AlignCenter;
        }
    } else if ( role == Qt::ToolTipRole ) {
        switch ( section ) {
            case RequestName: return ToolTip::resourceName();
            case RequestType: return ToolTip::resourceType();
            case RequestAllocation: return i18n( "Resource allocation" );
            case RequestMaximum: return i18nc( "@info:tootip", "Available resources or resource units" );
            case RequestRequired: return i18nc( "@info:tootip", "Required material resources" );
            default: return QVariant();
        }
    } else if ( role == Qt::WhatsThisRole ) {
        switch ( section ) {
            case RequestRequired:
                return i18nc( "@info:whatsthis", "<title>Required Resources</title>"
                "<para>A working resource can be assigned to one or more required resources."
                " A required resource is a material resource that the working resource depends on"
                " in order to do the work.</para>"
                "<para>To be able to use a material resource as a required resource, the material resource"
                " must be part of a group of type Material.</para>" );
            default: return QVariant();
        }
    }
    return QVariant();
}
//--------------------------------------

ResourceAllocationItemModel::ResourceAllocationItemModel( QObject *parent )
    : ItemModelBase( parent )
{
}

ResourceAllocationItemModel::~ResourceAllocationItemModel()
{
}

void ResourceAllocationItemModel::slotResourceToBeInserted( const ResourceGroup *group, int row )
{
    //kDebug(planDbg())<<group->name()<<","<<row;
    beginInsertRows( index( group ), row, row );
}

void ResourceAllocationItemModel::slotResourceInserted( const Resource */*resource */)
{
    //kDebug(planDbg())<<resource->name();
    endInsertRows();
    emit layoutChanged(); //HACK to make the right view react! Bug in qt?
}

void ResourceAllocationItemModel::slotResourceToBeRemoved( const Resource *resource )
{
    //kDebug(planDbg())<<resource->name();
    int row = index( resource ).row();
    beginRemoveRows( index( resource->parentGroup() ), row, row );
}

void ResourceAllocationItemModel::slotResourceRemoved( const Resource */*resource */)
{
    //kDebug(planDbg())<<resource->name();
    endRemoveRows();
}

void ResourceAllocationItemModel::slotResourceGroupToBeInserted( const ResourceGroup */*group*/, int row )
{
    //kDebug(planDbg())<<group->name();
    beginInsertRows( QModelIndex(), row, row );
}

void ResourceAllocationItemModel::slotResourceGroupInserted( const ResourceGroup */*group */)
{
    //kDebug(planDbg())<<group->name();
    endInsertRows();
}

void ResourceAllocationItemModel::slotResourceGroupToBeRemoved( const ResourceGroup *group )
{
    //kDebug(planDbg())<<group->name();
    int row = index( group ).row();
    beginRemoveRows( QModelIndex(), row, row );
}

void ResourceAllocationItemModel::slotResourceGroupRemoved( const ResourceGroup */*group */)
{
    //kDebug(planDbg())<<group->name();
    endRemoveRows();
}

void ResourceAllocationItemModel::setProject( Project *project )
{
    if ( m_project ) {
        disconnect( m_project, SIGNAL( resourceChanged( Resource* ) ), this, SLOT( slotResourceChanged( Resource* ) ) );
        disconnect( m_project, SIGNAL( resourceGroupChanged( ResourceGroup* ) ), this, SLOT( slotResourceGroupChanged( ResourceGroup* ) ) );

        disconnect( m_project, SIGNAL( resourceGroupToBeAdded( const ResourceGroup*, int ) ), this, SLOT( slotResourceGroupToBeInserted( const ResourceGroup*, int ) ) );

        disconnect( m_project, SIGNAL( resourceGroupToBeRemoved( const ResourceGroup* ) ), this, SLOT( slotResourceGroupToBeRemoved( const ResourceGroup* ) ) );

        disconnect( m_project, SIGNAL( resourceToBeAdded( const ResourceGroup*, int ) ), this, SLOT( slotResourceToBeInserted( const ResourceGroup*, int ) ) );

        disconnect( m_project, SIGNAL( resourceToBeRemoved( const Resource* ) ), this, SLOT( slotResourceToBeRemoved( const Resource* ) ) );

        disconnect( m_project, SIGNAL( resourceGroupAdded( const ResourceGroup* ) ), this, SLOT( slotResourceGroupInserted( const ResourceGroup* ) ) );

        disconnect( m_project, SIGNAL( resourceGroupRemoved( const ResourceGroup* ) ), this, SLOT( slotResourceGroupRemoved( const ResourceGroup* ) ) );

        disconnect( m_project, SIGNAL( resourceAdded( const Resource* ) ), this, SLOT( slotResourceInserted( const Resource* ) ) );

        disconnect( m_project, SIGNAL( resourceRemoved( const Resource* ) ), this, SLOT( slotResourceRemoved( const Resource* ) ) );

    }
    m_project = project;
    if ( m_project ) {
        connect( m_project, SIGNAL( resourceChanged( Resource* ) ), this, SLOT( slotResourceChanged( Resource* ) ) );
        connect( m_project, SIGNAL( resourceGroupChanged( ResourceGroup* ) ), this, SLOT( slotResourceGroupChanged( ResourceGroup* ) ) );

        connect( m_project, SIGNAL( resourceGroupToBeAdded( const ResourceGroup*, int ) ), this, SLOT( slotResourceGroupToBeInserted( const ResourceGroup*, int ) ) );

        connect( m_project, SIGNAL( resourceGroupToBeRemoved( const ResourceGroup* ) ), this, SLOT( slotResourceGroupToBeRemoved( const ResourceGroup* ) ) );

        connect( m_project, SIGNAL( resourceToBeAdded( const ResourceGroup*, int ) ), this, SLOT( slotResourceToBeInserted( const ResourceGroup*, int ) ) );

        connect( m_project, SIGNAL( resourceToBeRemoved( const Resource* ) ), this, SLOT( slotResourceToBeRemoved( const Resource* ) ) );

        connect( m_project, SIGNAL( resourceGroupAdded( const ResourceGroup* ) ), this, SLOT( slotResourceGroupInserted( const ResourceGroup* ) ) );

        connect( m_project, SIGNAL( resourceGroupRemoved( const ResourceGroup* ) ), this, SLOT( slotResourceGroupRemoved( const ResourceGroup* ) ) );

        connect( m_project, SIGNAL( resourceAdded( const Resource* ) ), this, SLOT( slotResourceInserted( const Resource* ) ) );

        connect( m_project, SIGNAL( resourceRemoved( const Resource* ) ), this, SLOT( slotResourceRemoved( const Resource* ) ) );

    }
    m_model.setProject( m_project );
}

void ResourceAllocationItemModel::setTask( Task *task )
{
    if ( task == m_model.task() ) {
        return;
    }
    if ( m_model.task() == 0 ) {
        filldata( task );
        m_model.setTask( task );
        reset();
        return;
    }
    if ( task ) {
        emit layoutAboutToBeChanged();
        filldata( task );
        m_model.setTask( task );
        emit layoutChanged();
    }
}

void ResourceAllocationItemModel::filldata( Task *task )
{
    qDeleteAll( m_resourceCache );
    m_resourceCache.clear();
    qDeleteAll( m_groupCache );
    m_groupCache.clear();
    m_requiredChecked.clear();
    if ( task ) {
        foreach ( const ResourceGroup *g, m_project->resourceGroups() ) {
            const ResourceGroupRequest *gr = task->requests().find( g );
            if ( gr ) {
                m_groupCache[ g ] = new ResourceGroupRequest( *gr );
            }
        }
        foreach ( const Resource *r, m_project->resourceList() ) {
            const ResourceRequest *rr = task->requests().find( r );
            if ( rr ) {
                m_resourceCache[ r ] = new ResourceRequest( *rr );
                if ( ! m_resourceCache[ r ]->requiredResources().isEmpty() ) {
                    m_requiredChecked[ r ] = Qt::Checked;
                }
            }
        }
    }
}

bool ResourceAllocationItemModel::hasMaterialResources() const
{
    if ( ! m_project ) {
        return false;
    }
    foreach ( const ResourceGroup *g, m_project->resourceGroups() ) {
        if ( g->type() == ResourceGroup::Type_Material ) {
            foreach ( const Resource *r, g->resources() ) {
                if ( r->type() == Resource::Type_Material ) {
                    return true;
                }
            }
        }
    }
    return false;
}

Qt::ItemFlags ResourceAllocationItemModel::flags( const QModelIndex &index ) const
{
    Qt::ItemFlags flags = ItemModelBase::flags( index );
    if ( !m_readWrite ) {
        //kDebug(planDbg())<<"read only"<<flags;
        return flags &= ~Qt::ItemIsEditable;
    }
    if ( !index.isValid() ) {
        //kDebug(planDbg())<<"invalid"<<flags;
        return flags;
    }
    switch ( index.column() ) {
        case ResourceAllocationModel::RequestAllocation:
            flags |= ( Qt::ItemIsEditable | Qt::ItemIsUserCheckable );
            break;
        case ResourceAllocationModel::RequestRequired: {
            Resource *r = resource( index );
            if ( r && r->type() != Resource::Type_Work ) {
                flags &= ~( Qt::ItemIsEditable | Qt::ItemIsUserCheckable );
            } else if ( m_resourceCache.contains( r ) && m_resourceCache[ r ]->units() > 0 ) {
                flags |= ( Qt::ItemIsEditable | Qt::ItemIsUserCheckable );
                if ( ! hasMaterialResources() ) {
                    flags &= ~Qt::ItemIsEnabled;
                }
            }
            break;
        }
        default:
            flags &= ~Qt::ItemIsEditable;
            break;
    }
    return flags;
}


QModelIndex ResourceAllocationItemModel::parent( const QModelIndex &index ) const
{
    if ( !index.isValid() || m_project == 0 ) {
        return QModelIndex();
    }
    //kDebug(planDbg())<<index.internalPointer()<<":"<<index.row()<<","<<index.column();

    Resource *r = qobject_cast<Resource*>( object( index ) );
    if ( r && r->parentGroup() ) {
        // only resources have parent
        int row = m_project->indexOf(  r->parentGroup() );
        return createIndex( row, 0, r->parentGroup() );
    }

    return QModelIndex();
}

QModelIndex ResourceAllocationItemModel::index( int row, int column, const QModelIndex &parent ) const
{
    if ( m_project == 0 || column < 0 || column >= columnCount() || row < 0 ) {
        return QModelIndex();
    }
    if ( ! parent.isValid() ) {
        if ( row < m_project->numResourceGroups() ) {
            return createIndex( row, column, m_project->resourceGroupAt( row ) );
        }
        return QModelIndex();
    }
    QObject *p = object( parent );
    ResourceGroup *g = qobject_cast<ResourceGroup*>( p );
    if ( g ) {
        if ( row < g->numResources() ) {
            return createIndex( row, column, g->resourceAt( row ) );
        }
        return QModelIndex();
    }
    return QModelIndex();
}

QModelIndex ResourceAllocationItemModel::index( const Resource *resource ) const
{
    if ( m_project == 0 || resource == 0 ) {
        return QModelIndex();
    }
    Resource *r = const_cast<Resource*>(resource);
    int row = -1;
    ResourceGroup *par = r->parentGroup();
    if ( par ) {
        row = par->indexOf( r );
        return createIndex( row, 0, r );
    }
    return QModelIndex();
}

QModelIndex ResourceAllocationItemModel::index( const ResourceGroup *group ) const
{
    if ( m_project == 0 || group == 0 ) {
        return QModelIndex();
    }
    ResourceGroup *g = const_cast<ResourceGroup*>(group);
    int row = m_project->indexOf( g );
    return createIndex( row, 0, g );

}

int ResourceAllocationItemModel::columnCount( const QModelIndex &/*parent*/ ) const
{
    return m_model.propertyCount();
}

int ResourceAllocationItemModel::rowCount( const QModelIndex &parent ) const
{
    if ( m_project == 0 || m_model.task() == 0 ) {
        return 0;
    }
    if ( ! parent.isValid() ) {
        return m_project->numResourceGroups();
    }
    QObject *p = object( parent );
    ResourceGroup *g = qobject_cast<ResourceGroup*>( p );
    if ( g ) {
        return g->numResources();
    }
    return 0;
}

QVariant ResourceAllocationItemModel::allocation( const ResourceGroup *group, const Resource *res, int role ) const
{
    if ( m_model.task() == 0 ) {
        return QVariant();
    }
    if ( ! m_resourceCache.contains( res ) ) {
        if ( role == Qt::EditRole ) {
            ResourceRequest *req = m_model.task()->requests().find( res );
            if ( req == 0 ) {
                req = new ResourceRequest( const_cast<Resource*>( res ), 0 );
            }
            const_cast<ResourceAllocationItemModel*>( this )->m_resourceCache.insert( res, req );
            return req->units();
        }
        return m_model.allocation( group, res, role );
    }
    switch ( role ) {
        case Qt::DisplayRole: {
            // xgettext: no-c-format
            return i18nc( "<value>%", "%1%", m_resourceCache[ res ]->units() );
        }
        case Qt::EditRole:
            return m_resourceCache[ res ]->units();
        case Qt::ToolTipRole: {
            if ( res->units() == 0 ) {
                return i18nc( "@info:tooltip", "Not allocated" );
            }
            return i18nc( "@info:tooltip", "%1 allocated out of %2 available", allocation( group, res, Qt::DisplayRole ).toString(), m_model.maximum( res, Qt::DisplayRole ).toString() );
        }
        case Qt::CheckStateRole:
            return m_resourceCache[ res ]->units() == 0 ? Qt::Unchecked : Qt::Checked;
        default:
            return m_model.allocation( group, res, role );
    }
    return QVariant();
}

int ResourceAllocationItemModel::requestedResources( const ResourceGroup *res ) const
{
    int c = 0;
    foreach ( const Resource *r, res->resources() ) {
        if ( m_resourceCache.contains( r ) &&  m_resourceCache[ r ]->units() > 0 ) {
            ++c;
        }
    }
    return c;
}

QVariant ResourceAllocationItemModel::allocation( const ResourceGroup *res, int role ) const
{
    if ( m_model.task() == 0 ) {
        return QVariant();
    }
    if ( ! m_groupCache.contains( res ) ) {
        return m_model.allocation( res, role );
    }
    switch ( role ) {
        case Qt::DisplayRole:
            return QString(" %1 (%2)" )
                        .arg( qMax( m_groupCache[ res ]->units(), allocation( res, Role::Minimum ).toInt() ) )
                        .arg(requestedResources( res ) );
        case Qt::EditRole:
            return qMax( m_groupCache[ res ]->units(), allocation( res, Role::Minimum ).toInt() );
        case Qt::ToolTipRole: {
            QString s1 = i18ncp( "@info:tooltip",
                                 "%1 resource requested for dynamic allocation",
                                 "%1 resources requested for dynamic allocation",
                                 allocation( res, Qt::EditRole ).toInt() );
            QString s2 = i18ncp( "@info:tooltip",
                                 "%1 resource allocated",
                                 "%1 resources allocated",
                                 requestedResources( res ) );

            return i18nc( "@info:tooltip", "%1<nl/>%2", s1, s2 );
        }
        case Qt::WhatsThisRole: {
            return i18nc( "@info:whatsthis",
                          "<title>Group allocations</title>"
                          "<para>You can allocate a number of resources from a group and let"
                          " the scheduler select from the available resources at the time of scheduling.</para>"
                          " These dynamically allocated resources will be in addition to any resource you have allocated specifically." );
        }
        case Role::Minimum: {
            return 0;
        }
        case Role::Maximum: {
            return res->numResources() - requestedResources( res );
        }
        default:
            return m_model.allocation( res, role );
    }
    return QVariant();
}

bool ResourceAllocationItemModel::setAllocation( Resource *res, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole: {
            m_resourceCache[ res ]->setUnits( value.toInt() );
            QModelIndex idx = index( res->parentGroup() );
            emit dataChanged( index( idx.row(), 0, QModelIndex() ), index( idx.row(), columnCount() - 1, QModelIndex() ) );
            return true;
        }
        case Qt::CheckStateRole: {
            if ( ! m_resourceCache.contains( res ) ) {
                m_resourceCache[ res ] = new ResourceRequest( res, 0 );
            }
            if ( m_resourceCache[ res ]->units() == 0 ) {
                m_resourceCache[ res ]->setUnits( 100 );
                ResourceGroup *g = res->parentGroup();
                if ( m_groupCache.contains( g ) ) {
                    ResourceGroupRequest *gr = m_groupCache[ g ];
                    if ( gr->units() + requestedResources( g ) > g->numResources() ) {
                        gr->setUnits( gr->units() - 1 );
                    }
                }
            } else {
                m_resourceCache[ res ]->setUnits( 0 );
            }
            QModelIndex idx = index( res->parentGroup() );
            emit dataChanged( index( idx.row(), 0, QModelIndex() ), index( idx.row(), columnCount() - 1, QModelIndex() ) );
            return true;
        }
    }
    return false;
}

bool ResourceAllocationItemModel::setAllocation( ResourceGroup *res, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            if ( ! m_groupCache.contains( res ) ) {
                m_groupCache[ res ] = new ResourceGroupRequest( res, 0 );
            }
            m_groupCache[ res ]->setUnits( value.toInt() );
            emit dataChanged( index( res ), index( res ) );
            return true;
    }
    return false;
}

QVariant ResourceAllocationItemModel::maximum( const ResourceGroup *res, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole: {
            int c = res->numResources() - requestedResources( res );
            if ( m_groupCache.contains( res ) ) {
                c -= m_groupCache[ res ]->units();
            }
            return i18nc( "1: free resources, 2: number of resources", "%1 of %2", c, res->numResources() );
        }
        case Qt::ToolTipRole:
            return i18ncp( "@info:tooltip", "There is %1 resource available in this group", "There are %1 resources available in this group", res->numResources() );
        default:
            return m_model.maximum( res, role );
    }
    return QVariant();
}

QVariant ResourceAllocationItemModel::required( const QModelIndex &idx, int role ) const
{
    if ( m_model.task() == 0 ) {
        return QVariant();
    }
    Resource *res = resource( idx );
    if ( res == 0 ) {
        return QVariant();
    }
    switch ( role ) {
        case Qt::DisplayRole: {
            if ( res->type() == Resource::Type_Work ) {
                QStringList lst;
                if ( m_requiredChecked[ res ] ) {
                    foreach ( const Resource *r, required( idx ) ) {
                        lst << r->name();
                    }
                }
                return lst.isEmpty() ? i18n( "None" ) : lst.join( "," );
            }
            break;
        }
        case Qt::EditRole: break;
        case Qt::ToolTipRole: 
            switch ( res->type() ) {
                case Resource::Type_Work: {
                    if ( ! hasMaterialResources() ) {
                        return i18nc( "@info:tooltip", "No material resources available" );
                    }
                    QStringList lst;
                    if ( m_requiredChecked[ res ] ) {
                        foreach ( const Resource *r, required( idx ) ) {
                            lst << r->name();
                        }
                    }
                    return lst.isEmpty() ? i18nc( "@info:tooltip", "No required resources" ) : lst.join( "\n" );
                }
                case Resource::Type_Material:
                    return i18nc( "@info:tooltip", "Material resources cannot have required resources" );
                case Resource::Type_Team:
                    return i18nc( "@info:tooltip", "Team resources cannot have required resources" );
            }
            break;
        case Qt::CheckStateRole:
            if ( res->type() == Resource::Type_Work ) {
                return m_requiredChecked[ res ];
            }
            break;
        default:
            return m_model.required( res, role );
    }
    return QVariant();
}

bool ResourceAllocationItemModel::setRequired( const QModelIndex &idx, const QVariant &value, int role )
{
    Resource *res = resource( idx );
    if ( res == 0 ) {
        return false;
    }
    switch ( role ) {
        case Qt::CheckStateRole:
            m_requiredChecked[ res ] = value.toInt();
            if ( value.toInt() == Qt::Unchecked ) {
                m_resourceCache[ res ]->setRequiredResources( QList<Resource*>() );
            }
            emit dataChanged( idx, idx );
            return true;
    }
    return false;
}

QVariant ResourceAllocationItemModel::notUsed( const ResourceGroup *, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
            return QString(" ");
        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;
        case Qt::EditRole:
        case Qt::ToolTipRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant ResourceAllocationItemModel::data( const QModelIndex &index, int role ) const
{
    QVariant result;
    QObject *obj = object( index );
    if ( obj == 0 ) {
        return QVariant();
    }
    if ( role == Qt::TextAlignmentRole ) {
        // use same alignment as in header (headers always horizontal)
        return headerData( index.column(), Qt::Horizontal, role );
    }
    Resource *r = qobject_cast<Resource*>( obj );
    if ( r ) {
        if ( index.column() == ResourceAllocationModel::RequestAllocation ) {
            return allocation( r->parentGroup(), r, role );
        }
        if ( index.column() == ResourceAllocationModel::RequestRequired ) {
            return required( index, role );
        }
        result = m_model.data( r->parentGroup(), r, index.column(), role );
    } else {
        ResourceGroup *g = qobject_cast<ResourceGroup*>( obj );
        if ( g ) {
            switch ( index.column() ) {
                case ResourceAllocationModel::RequestAllocation:
                    result = allocation( g, role );
                    break;
                case ResourceAllocationModel::RequestMaximum:
                    result = maximum( g, role );
                    break;
                default:
                    result = m_model.data( g, index.column(), role );
                    break;
            }
        }
    }
    if ( role == Qt::DisplayRole && ! result.isValid() ) {
        result = " "; // HACK to show focus in empty cells
    }
    return result;
}

bool ResourceAllocationItemModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
    if ( ! index.isValid() ) {
        return ItemModelBase::setData( index, value, role );
    }
    if ( ( flags( index ) & Qt::ItemIsEditable ) == 0 ) {
        return false;
    }
    QObject *obj = object( index );
    Resource *r = qobject_cast<Resource*>( obj );
    if ( r ) {
        switch (index.column()) {
            case ResourceAllocationModel::RequestAllocation:
                if ( setAllocation( r, value, role ) ) {
                    emit dataChanged( index, index );
                    QModelIndex idx = this->index( index.row(), ResourceAllocationModel::RequestAllocation, parent( parent( index ) ) );
                    emit dataChanged( idx, idx );
                    return true;
                }
                return false;
            case ResourceAllocationModel::RequestRequired:
                return setRequired( index, value, role );
            default:
                //qWarning("data: invalid display value column %d", index.column());
                return false;
        }
    }
    ResourceGroup *g = qobject_cast<ResourceGroup*>( obj );
    if ( g ) {
        switch (index.column()) {
            case ResourceAllocationModel::RequestAllocation:
                if ( setAllocation( g, value, role ) ) {
                    emit dataChanged( index, index );
                    return true;
                }
                return false;
            default:
                //qWarning("data: invalid display value column %d", index.column());
                return false;
        }
    }
    return false;
}

QVariant ResourceAllocationItemModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal ) {
        if ( role == Qt::DisplayRole ) {
            return m_model.headerData( section, role );
        }
        if ( role == Qt::TextAlignmentRole ) {
            switch (section) {
                case 0: return QVariant();
                default: return Qt::AlignCenter;
            }
            return Qt::AlignCenter;
        }
    }
    return m_model.headerData( section, role );
}

QAbstractItemDelegate *ResourceAllocationItemModel::createDelegate( int col, QWidget *parent ) const
{
    switch ( col ) {
        case ResourceAllocationModel::RequestAllocation: return new SpinBoxDelegate( parent );
        case ResourceAllocationModel::RequestRequired: return new RequieredResourceDelegate( parent );
        default: break;
    }
    return 0;
}

QObject *ResourceAllocationItemModel::object( const QModelIndex &index ) const
{
    QObject *o = 0;
    if ( index.isValid() ) {
        o = static_cast<QObject*>( index.internalPointer() );
        Q_ASSERT( o );
    }
    return o;
}

void ResourceAllocationItemModel::slotResourceChanged( Resource *res )
{
    ResourceGroup *g = res->parentGroup();
    if ( g ) {
        int row = g->indexOf( res );
        emit dataChanged( createIndex( row, 0, res ), createIndex( row, columnCount() - 1, res ) );
        return;
    }
}

void ResourceAllocationItemModel::slotResourceGroupChanged( ResourceGroup *res )
{
    Project *p = res->project();
    if ( p ) {
        int row = p->resourceGroups().indexOf( res );
        emit dataChanged( createIndex( row, 0, res ), createIndex( row, columnCount() - 1, res ) );
    }
}

Resource *ResourceAllocationItemModel::resource( const QModelIndex &idx ) const
{
    return qobject_cast<Resource*>( object( idx ) );
}

void ResourceAllocationItemModel::setRequired( const QModelIndex &idx, const QList<Resource*> &lst )
{
    Resource *r = resource( idx );
    Q_ASSERT( r );
    if ( m_resourceCache.contains( r ) ) {
        m_resourceCache[ r ]->setRequiredResources( lst );
        emit dataChanged( idx, idx );
    }
}

QList<Resource*> ResourceAllocationItemModel::required( const QModelIndex &idx ) const
{
    Resource *r = resource( idx );
    Q_ASSERT( r );
    if ( m_resourceCache.contains( r ) ) {
        ResourceRequest* request = m_resourceCache[ r ];
        return request->requiredResources();
    }
    return r->requiredResources();
}

} // namespace KPlato

#include "kptresourceallocationmodel.moc"
