/*
 *  dlg_separate.cc - part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include "dlg_separate.h"

#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>

#include <klocale.h>
#include <knuminput.h>
#include <kis_debug.h>
DlgSeparate::DlgSeparate(const QString & imageCS,
                         const QString & layerCS,
                         QWidget *  parent,
                         const char * name)
        : KDialog(parent)
        , m_imageCS(imageCS)
        , m_layerCS(layerCS)
{
    setObjectName(name);
    setCaption(i18n("Separate Image"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    m_page = new WdgSeparations(this);
    Q_CHECK_PTR(m_page);
    setMainWidget(m_page);
    resize(m_page->sizeHint());

    m_page->lblColormodel->setText(layerCS);
    m_page->grpOutput->hide();
    connect(m_page->grpSource, SIGNAL(clicked(int)), this, SLOT(slotSetColorSpaceLabel(int)));
    connect(m_page->chkColors, SIGNAL(toggled(bool)), m_page->chkDownscale, SLOT(setDisabled(bool)));

    connect(this, SIGNAL(okClicked()),
            this, SLOT(okClicked()));
}

DlgSeparate::~DlgSeparate()
{
    delete m_page;
}
enumSepAlphaOptions DlgSeparate::getAlphaOptions()
{
    if (m_page->radioCopyAlpha) return COPY_ALPHA_TO_SEPARATIONS;
    if (m_page->radioDiscardAlpha) return DISCARD_ALPHA;
    if (m_page->radioSeparateAlpha) return CREATE_ALPHA_SEPARATION;

    return COPY_ALPHA_TO_SEPARATIONS;
}

enumSepSource DlgSeparate::getSource()
{
    if (m_page->radioCopyAlpha) return CURRENT_LAYER;
    if (m_page->radioAllLayers) return ALL_LAYERS;
    //if (XXX) return VISIBLE_LAYERS;

    return CURRENT_LAYER;
}

enumSepOutput DlgSeparate::getOutput()
{
    if (m_page->radioLayers) return TO_LAYERS;
    if (m_page->radioImages) return TO_IMAGES;

    return TO_LAYERS;
}


bool DlgSeparate::getDownscale()
{
    return m_page->chkDownscale->isChecked();
}

bool DlgSeparate::getToColor()
{
    return m_page->chkColors->isChecked();
}

void DlgSeparate::okClicked()
{
    accept();
}

void DlgSeparate::slotSetColorSpaceLabel(int buttonid)
{
    if (buttonid == 0) {
        m_page->lblColormodel->setText(m_layerCS);
    } else {
        m_page->lblColormodel->setText(m_imageCS);
    }
}
void DlgSeparate::enableDownscale(bool enable)
{
    m_page->chkDownscale->setEnabled(enable);
}

#include "dlg_separate.moc"
