/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef KIS_COLORSPACE_WET_STICKY_H_
#define KIS_COLORSPACE_WET_STICKY_H_

#include <qcolor.h>

#include "kis_global.h"
#include "kis_strategy_colorspace.h"
#include "kis_pixel.h"

namespace WetAndSticky {

	/**
         * A color is specified as a vector in HLS space.  Hue is a value
	 * in the range 0..360 degrees with 0 degrees being red.  Saturation
         * and Lightness are both in the range [0,1].  A lightness of 0 means
	 * black, with 1 being white.  A totally saturated color has saturation
	 * of 1.
	 */

	typedef struct hls_color {float hue, saturation, lightness; }
		HLS_COLOR;

	typedef struct rgb_color {Q_UINT8 red; Q_UINT8 green; Q_UINT8 blue;}
		RGB_COLOR;


	enum enumDirection {
		NORTH,
		EAST,
		SOUTH,
		WEST
	};


	typedef struct paint {
		HLS_COLOR color;
		Q_UINT8        liquid_content;  /*  [0,100].  */
		Q_UINT8        drying_rate;     /*  [0,100].  */
		Q_UINT8        miscibility;     /*  [0,inf].  */
	} PAINT, *PAINT_PTR;


	typedef struct rgba {
		RGB_COLOR color;
		Q_UINT8 alpha;
	} RGBA, *RGBA_PTR;


	/**
	 * Defines the strength and direction of gravity for a cell.
	 */
	typedef struct gravity {
		enumDirection  direction;
		Q_UINT8    strength;     /*  [0,Infinity). */
	} GRAVITY, *GRAVITY_PTR;



	/**
	 * Defines the contents and attributes of a cell on the canvas.
	 */
	typedef struct cell {
		RGBA     representation; /* The on-screen representation of this cell. */
		PAINT    contents;    /* The paint in this cell. */
		GRAVITY  gravity;     /* This cell's gravity.  */
		Q_UINT8  absorbancy;  /* How much paint can this cell hold? */
		Q_UINT8  volume;      /* The volume of paint. */
	} CELL, *CELL_PTR;


}



class KisColorSpaceWetSticky : public KisStrategyColorSpace {
public:
	KisColorSpaceWetSticky();
	virtual ~KisColorSpaceWetSticky();

public:



	virtual void nativeColor(const QColor& c, Q_UINT8 *dst, KisProfileSP profile = 0);
	virtual void nativeColor(const QColor& c, QUANTUM opacity, Q_UINT8 *dst, KisProfileSP profile = 0);

	virtual void toQColor(const Q_UINT8 *src, QColor *c, KisProfileSP profile = 0);
	virtual void toQColor(const Q_UINT8 *src, QColor *c, QUANTUM *opacity, KisProfileSP profile = 0);

	virtual KisPixelRO toKisPixelRO(const Q_UINT8 *src, KisProfileSP profile = 0);

	virtual KisPixel toKisPixel(Q_UINT8 *src, KisProfileSP profile = 0);


	virtual vKisChannelInfoSP channels() const;
	virtual bool alpha() const;
	virtual Q_INT32 nChannels() const;
	virtual Q_INT32 nColorChannels() const;
	virtual Q_INT32 nSubstanceChannels() const;
	virtual Q_INT32 pixelSize() const;

	virtual QImage convertToQImage(const Q_UINT8 *data, Q_INT32 width, Q_INT32 height,
				       KisProfileSP srcProfile, KisProfileSP dstProfile,
				       Q_INT32 renderingIntent = INTENT_PERCEPTUAL);


	virtual void adjustBrightness(Q_UINT8 *src1, Q_INT8 adjust) const;
	virtual void adjustBrightnessContrast(const Q_UINT8 *src, Q_UINT8 *dst, Q_INT8 brightness, Q_INT8 contrast, Q_INT32 nPixels) const {};;
	virtual void mixColors(const Q_UINT8 **colors, const Q_UINT8 *weights, Q_UINT32 nColors, Q_UINT8 *dst) const;
	
	virtual KisCompositeOpList userVisiblecompositeOps() const;

protected:

	virtual void bitBlt(Q_UINT8 *dst,
			    Q_INT32 dstRowSize,
			    const Q_UINT8 *src,
			    Q_INT32 srcRowStride,
			    const Q_UINT8 *srcAlphaMask,
			    Q_INT32 maskRowStride,
			    QUANTUM opacity,
			    Q_INT32 rows,
			    Q_INT32 cols,
			    const KisCompositeOp& op);


	virtual bool convertPixelsTo(const Q_UINT8 * src, KisProfileSP srcProfile,
				     Q_UINT8 * dst, KisStrategyColorSpaceSP dstColorStrategy, KisProfileSP dstProfile,
				     Q_UINT32 numPixels,
				     Q_INT32 renderingIntent = INTENT_PERCEPTUAL);


private:

	void compositeOver(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeClear(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);
	void compositeCopy(Q_UINT8 *dst, Q_INT32 dstRowStride, const Q_UINT8 *src, Q_INT32 srcRowStride, const Q_UINT8 *mask, Q_INT32 maskRowStride, Q_INT32 rows, Q_INT32 columns, QUANTUM opacity);

private:
	vKisChannelInfoSP m_channels;
};

#endif // KIS_COLORSPACE_WET_STICKY_H_
