/* This file is part of the KDE project
Copyright (C) 2002, 2003 Laurent Montel <lmontel@mandrakesoft.com>
Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>

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

#include "KarbonConfigInterfacePage.h"

#include "KarbonView.h"
#include "KarbonPart.h"
#include "KarbonFactory.h"

#include <KoUnitDoubleSpinBox.h>
#include <KoConfigGridPage.h>
#include <KoConfigDocumentPage.h>
#include <KoConfigMiscPage.h>

#include <KLocale>
#include <KComponentData>
#include <KConfig>
#include <KConfigGroup>
#include <KNumInput>
#include <KColorButton>

#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QGridLayout>

KarbonConfigInterfacePage::KarbonConfigInterfacePage(KarbonView* view, char* name)
{
    setObjectName(name);

    m_view = view;
    m_config = KarbonFactory::componentData().config();

    m_oldRecentFiles = 10;
    m_oldDockerFontSize = 8;
    m_oldCanvasColor = QColor(Qt::white);
    bool oldShowStatusBar = true;

    QGroupBox* tmpQGroupBox = new QGroupBox(i18n("Interface"), this);

    KConfigGroup emptyGroup = m_config->group("GUI");
    m_oldDockerFontSize = emptyGroup.readEntry("palettefontsize", m_oldDockerFontSize);

    if (m_config->hasGroup("Interface")) {
        KConfigGroup interfaceGroup = m_config->group("Interface");

        m_oldRecentFiles = interfaceGroup.readEntry("NbRecentFile", m_oldRecentFiles);
        oldShowStatusBar = interfaceGroup.readEntry("ShowStatusBar", true);
        m_oldCanvasColor = interfaceGroup.readEntry("CanvasColor", m_oldCanvasColor);
    }

    QGridLayout *grpLayout = new QGridLayout(tmpQGroupBox);

    grpLayout->addWidget(new QLabel(i18n("Show status bar:"), tmpQGroupBox), 0, 0);
    m_showStatusBar = new QCheckBox("", tmpQGroupBox);
    m_showStatusBar->setChecked(oldShowStatusBar);
    grpLayout->addWidget(m_showStatusBar, 0, 1);

    grpLayout->addWidget(new QLabel(i18n("Number of recent files:"), tmpQGroupBox), 1, 0);
    m_recentFiles = new KIntNumInput(tmpQGroupBox);
    m_recentFiles->setRange(1, 20, 1);
    m_recentFiles->setValue(m_oldRecentFiles);
    grpLayout->addWidget(m_recentFiles, 1, 1);

    grpLayout->addWidget(new QLabel(i18n("Palette font size:"), tmpQGroupBox), 2, 0);
    m_dockerFontSize = new KIntNumInput(tmpQGroupBox);
    m_dockerFontSize->setRange(5, 20, 1);
    m_dockerFontSize->setValue(m_oldDockerFontSize);
    grpLayout->addWidget(m_dockerFontSize, 2, 1);

    grpLayout->addWidget(new QLabel(i18n("Canvas color:"), tmpQGroupBox), 3, 0);
    m_canvasColor = new KColorButton(m_oldCanvasColor, tmpQGroupBox);
    grpLayout->addWidget(m_canvasColor, 3, 1);

    grpLayout->setRowStretch(4, 1);
}

void KarbonConfigInterfacePage::apply()
{
    bool showStatusBar = m_showStatusBar->isChecked();

    KarbonPart* part = m_view->part();

    KConfigGroup interfaceGroup = m_config->group("Interface");

    int recent = m_recentFiles->value();

    if (recent != m_oldRecentFiles) {
        interfaceGroup.writeEntry("NbRecentFile", recent);
        m_view->setNumberOfRecentFiles(recent);
        m_oldRecentFiles = recent;
    }

    bool refreshGUI = false;

    if (showStatusBar != part->showStatusBar()) {
        interfaceGroup.writeEntry("ShowStatusBar", showStatusBar);
        part->setShowStatusBar(showStatusBar);
        refreshGUI = true;
    }

    int dockerFontSize = m_dockerFontSize->value();

    if (dockerFontSize != m_oldDockerFontSize) {
        m_config->group("GUI").writeEntry("palettefontsize", dockerFontSize);
        m_oldDockerFontSize = dockerFontSize;
        refreshGUI = true;
    }

    QColor canvasColor = m_canvasColor->color();
    if (canvasColor != m_oldCanvasColor) {
        interfaceGroup.writeEntry("CanvasColor", canvasColor);
        refreshGUI = true;
    }

    if (refreshGUI)
        part->reorganizeGUI();
}

void KarbonConfigInterfacePage::slotDefault()
{
    m_recentFiles->setValue(10);
    m_dockerFontSize->setValue(8);
    m_showStatusBar->setChecked(true);
}

#include "KarbonConfigInterfacePage.moc"
