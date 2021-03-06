/*
 * Copyright (c) 2008-2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "shivageneratorsplugin.h"

#include <QMutex>

#include <kis_debug.h>
#include <kpluginfactory.h>
#include <kstandarddirs.h>

#include <generator/kis_generator_registry.h>

#include <GTLCore/String.h>
#include <OpenShiva/SourcesCollection.h>

#include "shivagenerator.h"

#include <GTLCore/CompilationMessages.h>

QMutex* shivaMutex;

K_PLUGIN_FACTORY(ShivaPluginFactory, registerPlugin<ShivaPlugin>();)
K_EXPORT_PLUGIN(ShivaPluginFactory("krita"))

ShivaPlugin::ShivaPlugin(QObject *parent, const QVariantList &)
        : KParts::Plugin(parent)
{
    m_sourceCollection = new OpenShiva::SourcesCollection();

    QStringList kernelModulesDirs = KGlobal::mainComponent().dirs()->findDirs("data", "krita/shiva/kernels/");
    dbgPlugins << kernelModulesDirs;
    foreach(const QString & dir, kernelModulesDirs) {
        dbgPlugins << "Append : " << dir << " to the list of CTL modules";
        m_sourceCollection->addDirectory(dir.toAscii().data());
    }
    {
        KisGeneratorRegistry * manager = KisGeneratorRegistry::instance();
        Q_ASSERT(manager);
        std::list< OpenShiva::Source > kernels = m_sourceCollection->sources(OpenShiva::Source::GeneratorKernel);

        dbgPlugins << "Collection has " << kernels.size() << " generators";
        foreach(OpenShiva::Source kernel, kernels) {
            dbgPlugins << kernel.metadataCompilationMessages().toString().c_str();
            if (kernel.outputImageType() == OpenShiva::Source::Image || kernel.outputImageType() == OpenShiva::Source::Image4) {
                manager->add(new ShivaGenerator(new OpenShiva::Source(kernel)));
            }
        }
    }
    shivaMutex = new QMutex;
}

ShivaPlugin::~ShivaPlugin()
{
//     delete m_sourceCollection; // The plugin object get deleted right after creation, m_sourceCollection be staticly deleted
}
