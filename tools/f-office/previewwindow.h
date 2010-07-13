/*
 * This file is part of Maemo 5 Office UI for KOffice
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Sugnan Prabhu S <sugnan.prabhu@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QLabel>
#include <QDialog>
#include <QScrollArea>
#include <KoPAPageBase.h>
#include <KoDocument.h>
#include <QThread>

#include "flowlayout.h"

class PreviewWindow : public QDialog
{
    Q_OBJECT

public:
    PreviewWindow(KoDocument *m_doc,KoView *m_view,int m_currentPage,QList <QPixmap> *thumbnailList);

public:
   /*!
    * Preview images of the slides will be shown on the pushbutton
    */
    QList<QPushButton *> previewScreenButton;
    QList <QPixmap> *thumbnailList;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    KoDocument *m_doc;
    KoView *m_view;
   /*!
    * Its a Layout created for showing the previews in a sequential manner
    */
    FlowLayout *flowLayout;
    QGridLayout *gridLayout;
   /*!
    * stores the number of preview its showing in the
    */
    int previewCount;
    int toBeMovedPage;
    bool moveFlag;
    int m_currentPage;
public slots:
   /*!
    * slot to service the click of any thumbnail
    */
    void screenThumbnailClicked();
   /*!
    * it adds thumbnail and shows it in the scroll area
    */
    void showThumbnail();
    /*!
     * it adds thumbnail and shows it in the scroll area
     */
    void addThumbnail(QPixmap pix);
signals:
   /*!
    * This signal is emitted whenever the thumbnail is clicked
    */
    void gotoPage(int page);
};

class StoreButtonPreview : public QObject
{
    Q_OBJECT
public:
    explicit StoreButtonPreview(KoDocument *m_doc,KoView *m_view,QObject *parent = 0);
public slots:
    void showDialog(int m_currentPage);
    void addThumbnail(long pageNumber);
public:
    QList<QPixmap> thumbnailList;
    KoDocument *m_doc;
    KoView *m_view;
    PreviewWindow *previewWindow;
    bool isPreviewDialogActive;
signals:
    void gotoPage(int page);
};

class ThumbnailRetriever : public QThread
{
    Q_OBJECT
public:
    explicit ThumbnailRetriever(long pageCount,QObject *parent = 0);

signals:
    void newThumbnail(long pageNumber);
private:
    long pageCount;
public:
    void run();
};

#endif
