/* $Id$ */

#include "KChart.h"
#include "KChartColorArray.h"
#include "KChartBarsPainter.h"
#include "KChartAreaPainter.h"
#include "KChartLinesPainter.h"
#include "KChartPointsPainter.h"
#include "KChartLinesPointsPainter.h"
#include "KChartPiePainter.h"
#include "KChartPie3DPainter.h"

#include <qapp.h> // fatal
#include <qstrlist.h> // QStrList

#include <kdebug.h>

#include "KChart.moc"

KChart::KChart( KChartType type ) :
  _charttype( type )
{
  // Create a chart painter object of the appropriate type
  switch( _charttype ) {
  case Bars:
	_cp = new KChartBarsPainter( this );
	break;
  case Lines:
	_cp = new KChartLinesPainter( this );
	break;
  case Points:
	_cp = new KChartPointsPainter( this );
	break;
  case LinesPoints:
	_cp = new KChartLinesPointsPainter( this );
	break;
  case Area:
	_cp = new KChartAreaPainter( this );
	break;
  case Pie:
	_cp = new KChartPiePainter( this );
	break;
  case Pie3D:
	_cp = new KChartPie3DPainter( this );
	break;
  default:
	KDEBUG( KDEBUG_WARN, 34001, "Unknown chart type selected, choosing bars" );
	_cp = new KChartBarsPainter( this );
  };
}


KChart::~KChart()
{
  delete _cp;
}


void KChart::setChartType( KChartType charttype )
{
  _charttype = charttype;
  debug( "Sorry, not implemented: KChart::setChartType\n" );
}

KChartType KChart::chartType() const
{
  return _charttype;
}


void KChart::setChartData( KChartData* chartdata )
{
  _chartdata = chartdata;
}


KChartData* KChart::chartData() const
{
  return _chartdata;
}


void KChart::repaintChart( QPaintDevice* paintdev )
{
  _cp->paintChart( paintdev );
}


void KChart::setTitle( const char* str )
{
  _title = str;
}


QString KChart::title() const
{
  return _title;
}


void KChart::setXLabel( const char* str )
{
  _xlabel = str;
}


QString KChart::xLabel() const
{
  return _xlabel;
}


void KChart::setYLabel( const char* str )
{
  _ylabel = str;
}


QString KChart::yLabel() const
{
  return _ylabel;
}

void KChart::setY1Label( const char* str )
{
  _y1label = str;
}

QString KChart::y1Label() const
{
  return _y1label;
}

void KChart::setY2Label( const char* str )
{
  _y2label = str;
}


QString KChart::y2Label() const
{
  return _y2label;
}




void KChart::setTextColor( QColor color  )
{
  _textcolor = color;
}

QColor KChart::textColor() const
{
  return _textcolor;
}


void KChart::setTitleFont( QFont font )
{
  _titlefont = font;
}

QFont KChart::titleFont() const
{
  return _titlefont;
}


void KChart::setLabelFont( QFont font )
{
  _xlabelfont = font;
  _ylabelfont = font;
}

QFont KChart::labelFont() const
{
  return _xlabelfont;
}


void KChart::setXLabelFont( QFont font  )
{
  _xlabelfont = font;
}

QFont KChart::xLabelFont() const
{
  return _xlabelfont;
}


void KChart::setYLabelFont( QFont font )
{
  _ylabelfont = font;
}

QFont KChart::yLabelFont() const
{
  return _ylabelfont;
}


void KChart::setValueFont( QFont font  )
{
  fatal( "Sorry, not implemented: KChart::setValueFont\n" );
}

QFont KChart::valueFont() const
{
  fatal( "Sorry, not implemented: KChart::valueFont\n" );
  return QFont();
}


void KChart::setXAxisFont( QFont font  )
{
  _xaxisfont = font;
}

QFont KChart::xAxisFont() const
{
  return _xaxisfont;
}


void KChart::setYAxisFont( QFont font )
{
  _yaxisfont = font;
}

QFont KChart::yAxisFont() const
{
  return _yaxisfont;
}


void KChart::setMargin( int margin )
{
  _topmargin = margin;
  _bottommargin = margin;
  _leftmargin = margin;
  _rightmargin = margin;
}

int KChart::margin() const
{
  fatal( "Sorry, not implemented: KChart::margin\n" );
  return 0;
}


void KChart::setDataColors( KChartColorArray* colors )
{
  _datacolors = *colors;
}

const KChartColorArray* KChart::dataColors() const
{
  return &_datacolors;
}


void KChart::setTickLength( int length )
{
  _ticklength = length;
}

int KChart::tickLength() const
{
  return _ticklength;
}


void KChart::setYTicksNum( int ticks )
{
  _yticksnum = ticks;
}


int KChart::yTicksNum() const
{
  return _yticksnum;
}


void KChart::setXLabelSkip( int skip )
{
  _xlabelskip = skip;
}

int KChart::xLabelSkip() const
{
  return _xlabelskip;
}


void KChart::setYLabelSkip( int skip  )
{
  _ylabelskip = skip;
}

int KChart::yLabelSkip() const
{
  return _ylabelskip;
}


void KChart::setXPlotValues( bool plot )
{
  _xplotvalues = plot;
}

bool KChart::xPlotValues() const
{
  return _xplotvalues;
}


void KChart::setYPlotValues( bool plot )
{
  _yplotvalues = plot;
}

bool KChart::yPlotValues() const
{
  return _yplotvalues;
}


void KChart::setYMaxValue( double value )
{
  _ymaxs[0] = value;
  _ymaxs[1] = value;
}

double KChart::yMaxValue() const
{
  return _ymaxs[0];
}


void KChart::setYMinValue( double value )
{
  _ymins[0] = value;
  _ymins[1] = value;
}

double KChart::yMinValue() const
{
  return _ymins[0];
}

void KChart::setY1MaxValue( double value )
{
  _ymaxs[0] = value;
}

double KChart::y1MaxValue() const
{
  return _ymaxs[0];
}

void KChart::setY1MinValue( double value )
{
  _ymins[0] = value;
}

double KChart::y1MinValue() const
{
  return _ymins[0];
}

void KChart::setY2MaxValue( double value )
{
  _ymaxs[1] = value;
}

double KChart::y2MaxValue() const
{
  return _ymaxs[1];
}

void KChart::setY2MinValue( double value )
{
  _ymins[1] = value;
}

double KChart::y2MinValue() const
{
  return _ymins[1];
}


void KChart::setAxisSpace( int space )
{
  _axisspace = space;
}

int KChart::axisSpace() const
{
  return _axisspace;
}


void KChart::setLineWidth( int width )
{
  _linewidth = width;
}

int KChart::lineWidth() const
{
  return _linewidth;
}


void KChart::setOverwrite( OverwriteMode over )
{
  _overwrite = over;
}


OverwriteMode KChart::overwrite() const
{
  return _overwrite;
}


void KChart::setTwoAxes( bool twoaxes )
{
  _twoaxes = twoaxes;
}

bool KChart::twoAxes() const
{
  return _twoaxes;
}


void KChart::setLongTicks( bool longticks )
{
  _longticks = longticks;
}

bool KChart::longTicks() const
{
  return _longticks;
}


void KChart::setLegends( QStrList legends )
{
  _legends = legends;
}


QStrList KChart::legends() const
{
  return _legends;
}


void KChart::setLegendPlacement( LegendPlacement placement )
{
  _legendplacement = placement;
}

LegendPlacement KChart::legendPlacement() const
{
  return _legendplacement;
}


void KChart::setZeroAxisOnly( bool only )
{
  _zeroaxisonly = only;
}

bool KChart::zeroAxisOnly() const
{
  return _zeroaxisonly;
}


void KChart::setAxisLabelColor( QColor color )
{
  _axislabelcolor = color;
}

QColor KChart::axisLabelColor() const
{
  return _axislabelcolor;
}


void KChart::setBoxAxis( bool boxaxis )
{
  _boxaxis = boxaxis;
}

bool KChart::boxAxis() const
{
  return _boxaxis;
}


void KChart::setXTicks( bool xticks )
{
  _xticks = xticks;
}

bool KChart::xTicks() const
{
  return _xticks;
}


void KChart::setLineTypes( PenStyle types[], int number )
{
  for( int i = 0; i < number; i++ ) {
	PenStyle* ps = new PenStyle(); // deleted via auto-delete of list
	*ps = types[i];
	_linetypes.append( ps );
  }
}

void KChart::lineTypes( PenStyle types[], int& number )
{
  uint i = 0;
  for( i =  0; i < _linetypes.count(); i++ )
	types[ i ] = *_linetypes.at( i );
  number = i;
}


void KChart::setLegendMarkerWidth( int width )
{
  _legendmarkerwidth = width;
}

int KChart::legendMarkerWidth() const
{
  return _legendmarkerwidth;
}


void KChart::setLegendMarkerHeight( int height )
{
  _legendmarkerheight = height;
}

int KChart::legendMarkerHeight() const
{
  return _legendmarkerheight;
}


void KChart::setLegendFont( QFont font )
{
  _legendfont = font;
}

QFont KChart::legendFont() const
{
  return _legendfont;
}


