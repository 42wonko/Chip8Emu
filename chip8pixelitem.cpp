#include <QPainter>

#include "chip8pixelitem.h"

/**
	Default constructor (not used)
*/
Chip8PixelItem::Chip8PixelItem()
{

}
//-----------------------------------------------------------------------------

/**
	Constructor with pixel-object.

	\param	[in]	rect	Position and size of the new pixel.
*/
Chip8PixelItem::Chip8PixelItem(QRectF rect)
{
	pixel		= rect;
	pixelState	= false;
	whitePen	= QPen(Qt::white);
	blackPen	= QPen(Qt::black);
	whiteBrush	= QBrush(Qt::white);
	blackBrush	= QBrush(Qt::black);
	setPos(rect.x(), rect.y());
}
//-----------------------------------------------------------------------------

/**
	Default destructor.
*/
Chip8PixelItem::~Chip8PixelItem()
{
}
//-----------------------------------------------------------------------------

/**
	This method returns a bounding box of dimension (width x height) centered
	around the items origin (0, 0)
*/
QRectF Chip8PixelItem::boundingRect() const
{
	return QRectF(-(pixel.width() / 2), -(pixel.height() / 2), pixel.width(), pixel.height());
}
//-----------------------------------------------------------------------------

/**
	This method draws the pixel depending on the current state (ON or OFF). This method is called
	when the graphics scene receives an update event e.g. after calling update() for the graphics scene.

	\param	[in]	painter	Painter object.
	\param	[in]	option	Not used.
	\param	[in]	widget	Not used.
*/
void Chip8PixelItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if(PIXEL_ON == pixelState){
		painter->setPen(blackPen);
		painter->setBrush(blackBrush);
	} else {
		painter->setPen(whitePen);
		painter->setBrush(whiteBrush);
	}
	painter->drawRect(-(pixel.width() / 2), -(pixel.height() / 2), pixel.width(), pixel.height());
}
//-----------------------------------------------------------------------------
