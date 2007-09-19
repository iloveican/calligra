/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_POPUP_BUTTON_H_
#define _KIS_POPUP_BUTTON_H_

#include <QPushButton>

#include <krita_export.h>

/**
 * This class is a convenience class for a button that when clicked display
 * a popup.
 */
class KRITAUI_EXPORT KisPopupButton : public QPushButton {
    Q_OBJECT
    public:
        KisPopupButton(QWidget* parent);
        ~KisPopupButton();
        /**
         * Set the popup widget, the KisPopupButton becomes
         * the owner and parent of the widget.
         */
        void setPopupWidget(QWidget* widget);
    public slots:
        void showPopupWidget();
        void hidePopupWidget();
    private:
        struct Private;
        Private* const d;
};

#endif
