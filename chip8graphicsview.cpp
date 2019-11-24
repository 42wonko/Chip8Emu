#include "chip8graphicsview.h"
#include "chip8display.h"
#include "mainwindow.h"

/**
	The constructor for our QtGraphicsView interface to draw the CHIP8 display.
	This object runs in the main-application context and reacts to signals from the
	emulator object that runs in its own thread.

	\param	[in]	aWidth	X-resolution of the CHIP8 display.
	\param	[in]	aHeight	Y-resolution of the CHIP8 display.
	\param	[in]	aGv		Pointer to the QtGraphicsView object that was created by QtCreator.
	\param	[in]	parent	Pointer to the main-window object (Chip8MainWindow)
*/
Chip8GraphicsView::Chip8GraphicsView(unsigned int aWidth, unsigned int aHeight, QGraphicsView* aGv, QObject* parent)
: QObject(parent), gv(aGv), width(aWidth), height(aHeight)
{
	gs = new QGraphicsScene(parent);				// initialize our graphicsView
	gv->setScene(gs);
	Resize(aWidth, aHeight);						//

	connect(dynamic_cast<Chip8MainWindow*>(parent)->get_emu()->display(), &Chip8Display::DrawSprite,	this, &Chip8GraphicsView::DrawSprite);	// receive signal from emulator display to draw a sprite
	connect(dynamic_cast<Chip8MainWindow*>(parent)->get_emu()->display(), &Chip8Display::Clear,			this, &Chip8GraphicsView::Clear);		// receive signal from emulator display to clear the screen
	connect(dynamic_cast<Chip8MainWindow*>(parent)->get_emu()->display(), &Chip8Display::Resize,		this, &Chip8GraphicsView::Resize);		// receive signal from emulator display to switch the display resolution
}
//-----------------------------------------------------------------------------

/**
	The destructor of our QtGraphicsView interface.
*/
Chip8GraphicsView::~Chip8GraphicsView()
{
	delete gs;
}
//-----------------------------------------------------------------------------

/**
	Public slot that receives the \ref Resize signal from the emulator display class.

	\param	[in]	aWidth	New X-resolution of the CHIP8 display.
	\param	[in]	aHeight	New Y-resolution of the CHIP8 display.
*/
void Chip8GraphicsView::Resize(unsigned int aWidth, unsigned int aHeight)
{
	width	= aWidth;
	height	= aHeight;

	gv->scene()->clear();																// delete all pixels in current scene

	display.clear();																	// remove all buffered pointer to the pixels
	display.resize(aWidth);																// set up a new pixel buffer with new width...
	for(unsigned int i = 0; i < display.size(); ++i){									// .. and height
		display[i].resize(aHeight);
	}

	unsigned int pixel_width	= static_cast<unsigned int>(gv->width()) / width;		// re-compute the physical size of a pixel on sceen ...
	unsigned int pixel_height	= static_cast<unsigned int>(gv->height()) / height;		// ... (depending on the actual window size)
	for(unsigned int x = 0; x < width; ++x){											// create width x height new pixel
		for(unsigned int y = 0; y < height; ++y){
			Chip8PixelItem* pixel =  new Chip8PixelItem(QRectF(x*pixel_width, y*pixel_height, pixel_width, pixel_height));
			gv->scene()->addItem(pixel);												// add pixel to the scene ...
			display[x][y] = pixel;														// and our pixel-buffer
		}
	}
}
//-----------------------------------------------------------------------------

/**
	Public slot that receives the \ref Clear signal from the emulator display class.
*/
void Chip8GraphicsView::Clear(void)
{
	for(unsigned int x = 0; x < width; ++x){		// loop over all pixel ...
		for(unsigned int y = 0; y < height; ++y){
			display[x][y]->off();					// .. and switch them off
		}
	}
}
//-----------------------------------------------------------------------------

/**
	Public slot that receives the \ref DrawSprite signal from the emulator display class.

	\param	[in]	dsp	The display that is to be drawn.
	\param	[in]	xs		X-psition of the sprite.
	\param	[in]	ys		Y-psition of the sprite.
	\param	[in]	size	Size of the sprite (i.e. the number of lines. The size in X-direction is always 8).

	NOTE: Right now the method only works for the CHIP8 Classic and not the SuperCHIP !
*/
void Chip8GraphicsView::DrawSprite(std::vector<std::vector<bool>> dsp, unsigned int xs, unsigned int ys, unsigned int size)
{
	for(unsigned int x = xs; x < xs+8; ++x){				// all sprites are 8 pixel wide
		unsigned int xm = x % width;						// wrap around at right edge of display
		for(unsigned int y = ys; y < ys+size; ++y){
			unsigned int ym = y % height;					// wrap around at bottom edge of display
			if(dsp[xm][ym] == display[xm][ym]->state()){
				continue;									// pixel didn't change -> don't bother
			} else if(true == dsp[xm][ym]){					// draw pixel
				display[xm][ym]->on();
			} else {
				display[xm][ym]->off();
			}
			display[xm][ym]->paint(&painter,nullptr,nullptr);	//
		}
	}
	gs->update();												// actually show the changes
}
//-----------------------------------------------------------------------------
