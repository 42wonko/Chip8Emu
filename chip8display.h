#ifndef CHIP8DISPLAY_H
#define CHIP8DISPLAY_H

#include "chip8.h"
#include "mainwindow.h"

/// TBD: derive from QObject

class Chip8Display : public QObject
{
	Q_OBJECT

	public:
		Chip8Display(void);
		~Chip8Display();
		void mode(CHIP8::EMULATION_MODE aMode);
		bool draw_pixel(unsigned int x, unsigned int y);	// draw one pixel
		bool draw_sprite(unsigned int x, unsigned int y, unsigned int size, unsigned char* ram);
		void resize(void);
		void clear(void);

	signals:
		void DrawSprite(std::vector<std::vector<bool>> display, unsigned int x, unsigned int y, unsigned int size);
		void Resize(unsigned int x, unsigned int y);
		void Clear(void);

	private:
		CHIP8::EMULATION_MODE			mMode;
		unsigned int					mWidth;
		unsigned int					mHeight;
		std::vector<std::vector<bool>>	mDsp;
};

#endif // CHIP8DISPLAY_H
