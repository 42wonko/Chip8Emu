#include "chip8keyboard.h"

/**
	constructor of our abstract keyboard. The main purpose is to provide a keyboard to
	the catual emulator that looks like the original CHIP8 keyboard, i.e. the keys fom
	0 to 9 and A to F. It Does so by mapping the appropriate PC-keboard key to the
	orignal keys.  The mapping is configurable.

	\param [in]	device	The actual keyboard device of QT that handles the raw keyboard events.
	\return NONE
*/
Chip8Keyboard::Chip8Keyboard(KbdDevice* device)
{
	// initialize the original keymap (ASCII char -> number)
	keyMap['1'] = 1;
	keyMap['2'] = 2;
	keyMap['3'] = 3;
	keyMap['4'] = 4;
	keyMap['5'] = 5;
	keyMap['6'] = 6;
	keyMap['7'] = 7;
	keyMap['8'] = 8;
	keyMap['9'] = 9;
	keyMap['0'] = 0;
	keyMap['A'] = 0xA;
	keyMap['B'] = 0xB;
	keyMap['C'] = 0xC;
	keyMap['D'] = 0xD;
	keyMap['E'] = 0xE;
	keyMap['F'] = 0xF;

	// initialize the reverse map (number -> ASCII char)
	for(std::map<char,int>::iterator mapIter = keyMap.begin(); mapIter != keyMap.end(); ++mapIter){
		reverseKeyMap[mapIter->second] = mapIter->first;
	}


	kbdDevice = device;
}
//-----------------------------------------------------------------------------

/**
	Destructor to make everything nice again :-)
*/
Chip8Keyboard::~Chip8Keyboard()
{
	delete kbdDevice;
}
//-----------------------------------------------------------------------------

/**
	This method is the API that is called from the emulator to read one key.

	\param	[in]	mode	Read mode. can be one of \ref RD_MODE_BLOCKING or \ref RD_MODE_NON_BLOCKING.
	\return	Keyboard value between 0x00 and 0x0f. In case of mode = RD_MODE_NON_BLOCKING -1 is returned if no key was pressed.
*/
int Chip8Keyboard::ReadKey(READ_MODE mode)
{
	int key = KbdDevice::KBD_DEVICE_NO_KEY;

	if(RD_MODE_BLOCKING == mode){
		key = kbdDevice->GetKey();
	} else if(RD_MODE_NON_BLOCKING == mode){
		key = kbdDevice->ReadKey();
	}

	return GetKey(static_cast<char>(key));
}
//-----------------------------------------------------------------------------

/**
	This method maps the PC-key value to the original CHIP8 key-value.

	\param[in]	key	PC-Keyboard value
	\return		CHIP8 keyboard value or
*/
int Chip8Keyboard::GetKey(char key)
{
	if(KbdDevice::KBD_DEVICE_NO_KEY == key){
		return KbdDevice::KBD_DEVICE_NO_KEY;
	} else {
		return keyMap[key];
	}
}
//-----------------------------------------------------------------------------

/**

*/
bool Chip8Keyboard::MapKey(char source, int target)
{
	try{
		keyMap.at(source);	// check if the new character is already mapped to another character
		return false; 		// yes -> don't map and return an error
	} catch(...) {
							// key does not exists, so we can map the new key -> ignore exception
	}
	keyMap[source]			= target;		// add the new mapping to our key map
	keyMap.erase(reverseKeyMap[target]);	// remove the old mapping in key map
	reverseKeyMap[target]	= source;		// finally update the reverse mapping

	return true;
}
//-----------------------------------------------------------------------------

/**

*/
char Chip8Keyboard::GetMappedKey(int key)
{
	return reverseKeyMap.at(key);
}
//-----------------------------------------------------------------------------
