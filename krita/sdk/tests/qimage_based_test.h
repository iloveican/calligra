/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __QIMAGE_BASED_TEST_H
#define __QIMAGE_BASED_TEST_H

#include "testutil.h"


#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_undo_stores.h"
#include "kis_image.h"
#include "kis_selection.h"
#include "kis_paint_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_transparency_mask.h"
#include "kis_clone_layer.h"

#include "filter/kis_filter.h"
#include "filter/kis_filter_registry.h"

#include "commands/kis_selection_commands.h"


namespace TestUtil
{

class QImageBasedTest
{
public:
    QImageBasedTest(const QString &directoryName)
        : m_directoryName(directoryName)
    {
    }

    // you need to declare your own test function
    // See KisProcessingTest for example

protected:
    /**
     * Creates a complex image connected to a surrogate undo store
     */
    KisImageSP createImage(KisSurrogateUndoStore *undoStore) {
        QImage sourceImage(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");

        QRect imageRect = QRect(QPoint(0,0), sourceImage.size());

        QRect transpRect(50,50,300,300);
        QRect blurRect(66,66,300,300);
        QPoint blurShift(34,34);
        QPoint cloneShift(75,75);

        const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
        KisImageSP image = new KisImage(undoStore, imageRect.width(), imageRect.height(), cs, "merge test");

        KisFilterSP filter = KisFilterRegistry::instance()->value("blur");
        Q_ASSERT(filter);
        KisFilterConfiguration *configuration = filter->defaultConfiguration(0);
        Q_ASSERT(configuration);

        KisAdjustmentLayerSP blur1 = new KisAdjustmentLayer(image, "blur1", configuration, 0);
        blur1->selection()->clear();
        blur1->selection()->getOrCreatePixelSelection()->select(blurRect);
        blur1->setX(blurShift.x());
        blur1->setY(blurShift.y());

        KisPaintLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
        paintLayer1->paintDevice()->convertFromQImage(sourceImage, 0, 0, 0);

        KisTransparencyMaskSP transparencyMask1 = new KisTransparencyMask();
        transparencyMask1->setName("tmask1");
        transparencyMask1->selection()->clear();
        transparencyMask1->selection()->getOrCreatePixelSelection()->select(transpRect);

        KisCloneLayerSP cloneLayer1 =
            new KisCloneLayer(paintLayer1, image, "clone1", OPACITY_OPAQUE_U8);
        cloneLayer1->setX(cloneShift.x());
        cloneLayer1->setY(cloneShift.y());

        image->addNode(cloneLayer1);
        image->addNode(blur1);
        image->addNode(paintLayer1);
        image->addNode(transparencyMask1, paintLayer1);

        return image;
    }

    void addGlobalSelection(KisImageSP image) {
        QRect selectionRect(40,40,300,300);

        KisSelectionSP selection = new KisSelection(new KisSelectionDefaultBounds(0, image));
        KisPixelSelectionSP pixelSelection = selection->getOrCreatePixelSelection();
        pixelSelection->select(selectionRect);

        KUndo2Command *cmd = new KisSetGlobalSelectionCommand(image, selection);
        image->undoAdapter()->addCommand(cmd);
    }

    /**
     * Checks the content of image's layers against the set of
     * QImages stored in @p prefix subfolder
     */
    bool checkLayers(KisImageWSP image, const QString &prefix) {
        QVector<QImage> images;
        QVector<QString> names;

        fillNamesImages(image->root(), image->bounds(), images, names);

        bool valid = true;

        const int stackSize = images.size();
        for(int i = 0; i < stackSize; i++) {
            if(!checkOneQImage(images[i], prefix, names[i])) {
                valid = false;
            }
        }

        return valid;
    }

    /**
     * Checks the content of one image's layer against the QImage
     * stored in @p prefix subfolder
     */
    bool checkOneLayer(KisImageWSP image, KisNodeSP node,  const QString &prefix) {
        QVector<QImage> images;
        QVector<QString> names;

        fillNamesImages(node, image->bounds(), images, names);

        return checkOneQImage(images.first(), prefix, names.first());
    }

    // add default bounds param
    bool checkOneDevice(KisPaintDeviceSP device,
                        const QString &prefix,
                        const QString &name)
    {
        QImage image = device->convertToQImage(0);
        return checkOneQImage(image, prefix, name);
    }

    KisNodeSP findNode(KisNodeSP root, const QString &name) {
        if(root->name() == name) return root;

        KisNodeSP child = root->firstChild();
        while (child) {
            if(root = findNode(child, name)) return root;
            child = child->nextSibling();
        }

        return 0;
    }

private:
    bool checkOneQImage(const QImage &image,
                        const QString &prefix,
                        const QString &name)
    {
        QString realName = prefix + "_" + name + ".png";
        QString expectedName = prefix + "_" + name + "_expected.png";

        bool valid = true;

        QImage ref(QString(FILES_DATA_DIR) + QDir::separator() +
                   m_directoryName + QDir::separator() +
                   prefix + QDir::separator() + realName);

        QPoint temp;
        int fuzzy = 0;

        {
            QStringList terms = name.split('_');
            if(terms[0] == "root" || terms[0] == "blur1") {
                fuzzy = 1;
            }
        }

        if(ref != image &&
           !TestUtil::compareQImages(temp, ref, image, fuzzy)) {

            qDebug() << "--- Wrong image:" << realName;
            valid = false;

            image.save(QString(FILES_OUTPUT_DIR) + QDir::separator() + realName);
            ref.save(QString(FILES_OUTPUT_DIR) + QDir::separator() + expectedName);
        }

        return valid;
    }

    void fillNamesImages(KisNodeSP node, const QRect &rc,
                         QVector<QImage> &images,
                         QVector<QString> &names) {

        while (node) {
            if(node->original()) {
                names.append(node->name() + "_original");
                images.append(node->original()->
                              convertToQImage(0, rc.x(), rc.y(),
                                              rc.width(), rc.height()));
            }

            if(node->projection()) {
                names.append(node->name() + "_projection");
                images.append(node->projection()->
                              convertToQImage(0, rc.x(), rc.y(),
                                              rc.width(), rc.height()));
            }

            if(node->paintDevice()) {
                names.append(node->name() + "_paintDevice");
                images.append(node->paintDevice()->
                              convertToQImage(0, rc.x(), rc.y(),
                                              rc.width(), rc.height()));
            }

            fillNamesImages(node->firstChild(), rc, images, names);
            node = node->nextSibling();
        }
    }

private:
    QString m_directoryName;

};

}

#endif /* __QIMAGE_BASED_TEST_H */
