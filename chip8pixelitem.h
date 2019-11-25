#ifndef CHIP8PIXELITEM_H
#define CHIP8PIXELITEM_H

#include <QGraphicsItem>
#include <QPen>


class Chip8PixelItem : public QGraphicsItem
{
public:
	enum PIXEL_STATE {
		PIXEL_OFF	= false,
		PIXEL_ON	= true
	};

	Chip8PixelItem();
	Chip8PixelItem(QRectF rect);
	~Chip8PixelItem() override;
	void on(void)		{pixelState = PIXEL_ON;}	///< Enable pixel.
	void off(void)		{pixelState = PIXEL_OFF;}	///< Disable pixel.
	bool state(void)	{return pixelState;}		///< Return current state of pixel.

	QRectF	boundingRect() const override;
	void 	paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
	bool	pixelState;		///< The current pixel state (\ref PIXEL_OFF or \ref PIXEL_ON).
	QRectF	pixel;			///<
	QPen	whitePen;		///< Default pen for an OFF-pixel.
	QPen	blackPen;		///< Default pen for an ON-pixel.
	QBrush	whiteBrush;		///< Default brush for an OFF-pixel.
	QBrush	blackBrush;		///< Default brush for an ON-pixel.
};

#endif // CHIP8PIXELITEM_H
