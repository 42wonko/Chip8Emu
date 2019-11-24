#ifndef CHIP8KEYBOARD_H
#define CHIP8KEYBOARD_H

#include <QObject>
#include "kbddevice.h"

class Chip8Keyboard
{
public:
	enum	READ_MODE {
		RD_MODE_BLOCKING		= 0,
		RD_MODE_NON_BLOCKING	= 1
	};

	enum	KEY_CODE {
		NO_KEY		= -1,
		KEY_0		= 0,
		KEY_1		= 1,
		KEY_2		= 2,
		KEY_3		= 3,
		KEY_4		= 4,
		KEY_5		= 5,
		KEY_6		= 6,
		KEY_7		= 7,
		KEY_8		= 8,
		KEY_9		= 9,
		KEY_A		= 10,
		KEY_B		= 11,
		KEY_C		= 12,
		KEY_D		= 13,
		KEY_E		= 14,
		KEY_F		= 15,
	};

	Chip8Keyboard(KbdDevice* device);
	~Chip8Keyboard();

	int		ReadKey(READ_MODE mode);			///< Blocking or non-blocking keyboard read (API).
	int		GetKey(char key);					///< Do the actual key translation.
	char	GetMappedKey(int key);				///< Do a reverse lookup of the key mappping.
	bool	MapKey(char source, int target);	///< Install a new key mapping.

private:
	std::map<char, int>	keyMap;
	std::map<int, char> reverseKeyMap;
	KbdDevice*			kbdDevice;
};

#endif // CHIP8KEYBOARD_H
