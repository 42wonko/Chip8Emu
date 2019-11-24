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
	void on(void)		{pixelState = PIXEL_ON;}
	void off(void)		{pixelState = PIXEL_OFF;}
	bool state(void)	{return pixelState;}

	QRectF	boundingRect() const override;
	void 	paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
	bool	pixelState;
	QRectF	pixel;
	QPen	whitePen;
	QPen	blackPen;
	QBrush	whiteBrush;
	QBrush	blackBrush;
};

#endif // CHIP8PIXELITEM_H
