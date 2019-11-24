#include "chip8display.h"

/**

*/
Chip8Display::Chip8Display(void)
	: mMode(CHIP8::MODE_CLASSIC), mWidth(CHIP8::WIN_COLS), mHeight(CHIP8::WIN_ROWS)
{
	mDsp.resize(mWidth);
	for(unsigned int i = 0; i < mDsp.size(); ++i){
		mDsp[i].resize(mHeight);
	}
};
//-----------------------------------------------------------------------------

/**

*/
Chip8Display::~Chip8Display()
{

}
//-----------------------------------------------------------------------------

/**

*/
void Chip8Display::mode(CHIP8::EMULATION_MODE aMode)
{
	mMode = aMode;
	switch(mMode){
		case CHIP8::MODE_CLASSIC:	mWidth 	= CHIP8::WIN_COLS;
									mHeight	= CHIP8::WIN_ROWS;
									break;
		case CHIP8::MODE_SUPER:		mWidth	= CHIP8::WIN_S_COLS;
									mHeight	= CHIP8::WIN_S_ROWS;
									break;
	}
	resize();
}
//-----------------------------------------------------------------------------

/**

*/
void Chip8Display::resize(void)
{
	mDsp.resize(mWidth);
	for(unsigned int i = 0; i < mDsp.size(); ++i){
		mDsp[i].resize(mHeight);
	}
	for(unsigned int y = 0; y < mHeight; ++y){
		for(unsigned int x = 0; x < mWidth; ++x){
			mDsp[x][y] = false;
		}
	}
	emit Resize(mWidth, mHeight);	// signal main application to reset (the size of) the screen
}
//-----------------------------------------------------------------------------

/**

*/
void Chip8Display::clear(void)
{
	for(unsigned int y = 0; y < mHeight; ++y){
		for(unsigned int x = 0; x < mWidth; ++x){
			mDsp[x][y] = false;
		}
	}
	emit Clear();
}
//-----------------------------------------------------------------------------

/**

*/
bool Chip8Display::draw_sprite(unsigned int x, unsigned int y, unsigned int size, unsigned char* ram)
{
	bool		collision = false;

	if(0 == size ){															// draw an 16x16 sprite
//TBD
	} else {																// draw an 8xn sprite
		int idx=0;															// index for memory access
		for(unsigned int ly=0; ly < size; ++ly){
			for(unsigned int lx=0; lx<8; ++lx){
				bool pixel = ram[idx] & (0x80 >> lx);						// extract pixel from sprite
				bool screen_pixel = mDsp[(x+lx) % mWidth][(y+ly) % mHeight];
				if(pixel & screen_pixel){									// check collision
					collision |= true;
				}
				screen_pixel ^= pixel;
				mDsp[(x+lx) % mWidth][(y+ly) % mHeight] = screen_pixel;		// draw pixel into screen
			}
			++idx;
		}
		emit DrawSprite(mDsp, x, y, size);	// signal main application to redraw screen
	}

	return collision;
}
