/* This file is part of the KDE project
   Copyright (C) 2001-2002 Beno�t Vautrin <benoit.vautrin@free.fr>
   Copyright (C) 2002 Rob Buis <buis@kde.org>
   Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>

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

#ifndef _VGRADIENTTABWIDGET_H_
#define _VGRADIENTTABWIDGET_H_

#include <karbonui_export.h>

#include <QWidget>
#include <QTabWidget>
#include <QtGui/QTableWidgetItem>

#include <KoAbstractGradient.h>
#include <KoCheckerBoardPainter.h>

class KComboBox;
class VGradientWidget;
class KListWidget;
class QPushButton;
class KarbonGradientChooser;
class KoSliderCombo;
class KoResourceItem;
class KColorButton;

/// A widget to preview a gradient
class KARBONUI_EXPORT VGradientPreview : public QWidget
{
public:
    /// Constructs a gradient preview with the givne parent
    explicit VGradientPreview( QWidget* parent = 0L );

    /// Destroys the gradient preview
    ~VGradientPreview();

    /**
     * Sets the gradient to preview.
     * @param gradient the gradient to preview
     */
    void setGradient( const QGradient * gradient );

protected:
    virtual void paintEvent( QPaintEvent* );
private:
    QGradient * m_gradient; ///< the gradient to preview
};

/**
 * A tab widget for managing gradients.
 *
 * It has one tab to edit a selected gradients type, spread method and color stops.
 * Another tab contains a list with predefined gradients to choose from.
 */
class KARBONUI_EXPORT VGradientTabWidget : public QTabWidget
{
Q_OBJECT

public:
    enum VGradientTarget {
        StrokeGradient,
        FillGradient
    };

    /**
     * Creates a new gradient tab widget with the given parent.
     *
     * The predefined gradients are retrived from the given resource server.
     *
     * @param server the resource server to retrieve predefined gradients from
     * @param parent the widgets parent
     * @param name the widgets name
     */
    explicit VGradientTabWidget( QWidget* parent = 0L );

    /// Destroys the widget
    ~VGradientTabWidget();

    /**
     * Returns the actual selected gradient.
     * @return the actual gradient
     */
    const QGradient* gradient();

    /**
     * Sets a new gradient to edit.
     * @param gradient the gradient to edit
     */
    void setGradient( const QGradient* gradient );

    /// Returns the gradient target (fill/stroke)
    VGradientTarget target();

    /// Sets a new gradient target
    void setTarget( VGradientTarget target );

    /// Returns the gradient opacity
    double opacity() const;

    /// Sets the gradients opacity to @p opacity
    void setOpacity( double opacity );

    /// Sets the index of the stop to edit
    void setStopIndex( int index );

Q_SIGNALS:
    /// Is emmited a soon as the gradient changes
    void changed();

protected Q_SLOTS:
    void combosChange( int );
    void addGradientToPredefs();
    void changeToPredef( QTableWidgetItem* );
    void opacityChanged( double value, bool final );
    void stopsChanged();
    void stopChanged();
protected:
    void setupUI();
    void updateUI();
    void updatePredefGradients();
    void setupConnections();
    void blockChildSignals( bool block );

private:
    QWidget          *m_editTab;
    VGradientWidget  *m_gradientWidget;
    KComboBox        *m_gradientTarget;
    KComboBox        *m_gradientRepeat;
    KComboBox        *m_gradientType;
    KarbonGradientChooser *m_predefGradientsView;
    QPushButton      *m_addToPredefs;
    KoSliderCombo * m_opacity;
    KColorButton * m_stopColor;
    KoSliderCombo * m_stopOpacity;
    QGradient * m_gradient; /// the actual edited gradient
    double m_gradOpacity;    ///< the gradient opacity
    int m_stopIndex; ///< the index of the selected gradient stop
    KoCheckerBoardPainter m_checkerPainter;
};

#endif /* _VGRADIENTTABWIDGET_H_ */
