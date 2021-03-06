/* This file is part of the KDE project

   Copyright (C) 2011 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB. If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "CompareView.h"

#include <QDebug>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QTabBar>
#include <QStackedWidget>

CompareView::CompareView(QWidget *parent)
: QWidget(parent)
{
    init();
}

CompareView::CompareView(const QImage &image1, const QImage &image2, const QString &name1, const QString &name2, QWidget *parent)
: QWidget(parent)
, m_image1(image1)
, m_image2(image2)
{
    init();
    update(image1, image2, name1, name2, QImage());
}

CompareView::~CompareView()
{
}

void CompareView::init()
{
    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_tabBar = new QTabBar();
    m_tabBar->addTab("Image 1");
    m_tabBar->addTab("Image 2");
    layout->addWidget(m_tabBar, 0, 0, 1, 2);
    connect(m_tabBar, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));
    m_image1Label = new QLabel();
    m_image2Label = new QLabel();
    m_stack = new QStackedWidget();
    m_stack->addWidget(m_image1Label);
    m_stack->addWidget(m_image2Label);
    layout->addWidget(m_stack, 1, 0);

    m_diffLabel = new QLabel(this);
    layout->addWidget(m_diffLabel, 1, 1, Qt::AlignCenter);

    setLayout(layout);
}

void CompareView::update(const QImage &image1, const QImage &image2, const QString &name1, const QString &name2, const QImage &forcedDeltaView)
{
    m_image1 = image1;
    m_image2 = image2;
    if (forcedDeltaView.isNull()) {
        m_diff = difference(image1, image2);
    } else {
        m_diff = forcedDeltaView;
    }
    m_tabBar->setTabText(0, name1);
    m_tabBar->setTabText(1, name2);
    m_image1Label->setPixmap(QPixmap::fromImage(image1));
    m_image2Label->setPixmap(QPixmap::fromImage(image2));
    m_diffLabel->setPixmap(QPixmap::fromImage(m_diff));
    m_diffLabel->setMinimumWidth(image1.width());
    m_diffLabel->setAlignment(Qt::AlignCenter);
}

int lighten(int value)
{
    return 255 - (255 - value) / 2;
}

QImage CompareView::difference(const QImage &image1, const QImage &image2)
{
    QSize size(image1.size());
    if (size != image2.size()) {
        return QImage();
    }

    quint32 width = size.width();
    quint32 height = size.height();
    QImage result(width, height, QImage::Format_ARGB32);
    for (quint32 y = 0; y < height; ++y) {
        QRgb *scanLineRes = (QRgb *)result.scanLine(y);
        QRgb *scanLineIn1 = (QRgb *)image1.scanLine(y);
        QRgb *scanLineIn2 = (QRgb *)image2.scanLine(y);
        for (quint32 x = 0; x < width; ++x) {
            if (scanLineIn1[x] == scanLineIn2[x]) {
                scanLineRes[x] = qRgb(lighten(qRed(scanLineIn1[x])), lighten(qGreen(scanLineIn1[x])), lighten(qBlue(scanLineIn1[x])));
            }
            else {
                scanLineRes[x] = qRgb(255, 0, 0);
            }
        }
    }

    return result;
}

void CompareView::keyPressEvent(QKeyEvent * event)
{
    switch (event->key()) {
    case Qt::Key_Space:
        m_tabBar->setCurrentIndex((m_tabBar->currentIndex() + 1) % 2);
        m_stack->setCurrentIndex(m_tabBar->currentIndex());
        break;
    default:
        event->setAccepted(false);
        break;
    }
}

void CompareView::currentChanged(int currentIndex)
{
    m_tabBar->setCurrentIndex(currentIndex);
    m_stack->setCurrentIndex(currentIndex);
}
