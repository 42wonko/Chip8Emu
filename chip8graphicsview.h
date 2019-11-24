#ifndef CHIP8GRAPHICSVIEW_H
#define CHIP8GRAPHICSVIEW_H

#include <QObject>
#include <QGraphicsView>
#include "chip8pixelitem.h"

class Chip8GraphicsView : public QObject
{
	Q_OBJECT

	public:
		explicit Chip8GraphicsView(unsigned int aWidth, unsigned int aHeight, QGraphicsView* aGv, QObject *parent = nullptr);	///< Constructor
		~Chip8GraphicsView();																									///< Destructor

	signals:

	public slots:
		void Resize(unsigned int width, unsigned int heigt);														///< Changed display resolution.
		void Clear(void);																							///< Clear the display.
		void DrawSprite(std::vector<std::vector<bool>> dsp, unsigned int x, unsigned int y, unsigned int size);		///< Draw a sprite.

	private:
		QGraphicsView*								gv;			///< The QtGraphicsView that display the CHIP8 display.
		QGraphicsScene*								gs;			///< The scene for the graphics view.
		QPainter									painter;	///< Painter that does the drawing.
		unsigned int								width;		///< Logical X-resolution of the CHIP8 display.
		unsigned int								height;		///< Logical Y-resolution of the CHIP8 display.
		std::vector<std::vector<Chip8PixelItem*>>	display;	///< Local pixel buffer for faster access to items in scene.
};

#endif // CHIP8GRAPHICSVIEW_H
