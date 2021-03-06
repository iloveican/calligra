/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "exr_export.h"

#include <QCheckBox>
#include <QSlider>

#include <kapplication.h>
#include <kdialog.h>
#include <kpluginfactory.h>

#include <KoFilterChain.h>
#include <KoColorSpaceConstants.h>
#include <KoFilterManager.h>

#include <kis_properties_configuration.h>
#include <kis_config.h>
#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>

#include "exr_converter.h"

#include "ui_exr_export_widget.h"

class KisExternalLayer;

K_PLUGIN_FACTORY(ExportFactory, registerPlugin<exrExport>();)
K_EXPORT_PLUGIN(ExportFactory("calligrafilters"))

exrExport::exrExport(QObject *parent, const QVariantList &) : KoFilter(parent)
{
}

exrExport::~exrExport()
{
}

KoFilter::ConversionStatus exrExport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << "EXR export! From:" << from << ", To:" << to << "";

    if (from != "application/x-krita")
        return KoFilter::NotImplemented;

    KDialog dialog;
    dialog.setWindowTitle(i18n("OpenEXR Export Options"));
    dialog.setButtons(KDialog::Ok | KDialog::Cancel);
    Ui::ExrExportWidget widget;
    QWidget *page = new QWidget(&dialog);
    widget.setupUi(page);
    dialog.setMainWidget(page);
    dialog.resize(dialog.minimumSize());

    QString filterConfig = KisConfig().exportConfiguration("EXR");
    KisPropertiesConfiguration cfg;
    cfg.fromXML(filterConfig);

    widget.flatten->setChecked(cfg.getBool("flatten", false));

    if (!m_chain->manager()->getBatchMode() ) {
        kapp->restoreOverrideCursor();
        if (dialog.exec() == QDialog::Rejected) {
            return KoFilter::UserCancelled;
        }
    }

    cfg.setProperty("flatten", widget.flatten->isChecked());
    KisConfig().setExportConfiguration("EXR", cfg);

    KisDoc2 *output = dynamic_cast<KisDoc2*>(m_chain->inputDocument());
    QString filename = m_chain->outputFile();

    if (!output)
        return KoFilter::CreationError;


    if (filename.isEmpty()) return KoFilter::FileNotFound;

    KUrl url;
    url.setPath(filename);

    KisImageWSP image = output->image();
    Q_CHECK_PTR(image);

    exrConverter kpc(output);

    if (widget.flatten->isChecked()) {
        image->refreshGraph();
        image->lock();
        KisPaintDeviceSP pd = new KisPaintDevice(*image->projection());
        KisPaintLayerSP l = new KisPaintLayer(image, "projection", OPACITY_OPAQUE_U8, pd);
        image->unlock();

        KisImageBuilder_Result res;

        if ((res = kpc.buildFile(url, l)) == KisImageBuilder_RESULT_OK) {
            dbgFile << "success !";
            return KoFilter::OK;
        }

        dbgFile << " Result =" << res;
        return KoFilter::InternalError;
    } else {
        image->lock();

        KisImageBuilder_Result res = kpc.buildFile(url, image->rootLayer());
        image->unlock();

        if (res == KisImageBuilder_RESULT_OK) {
            dbgFile << "success !";
            return KoFilter::OK;
        }

        dbgFile << " Result =" << res;
        return KoFilter::InternalError;
    }
}

#include <exr_export.moc>

