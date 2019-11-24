#include <QKeyEvent>

#include "kbddevice.h"

/**
	Constructor.
*/
KbdDevice::KbdDevice(QWidget *parent)
: QWidget(parent), keyPressed(false), currentKey(0), keyCount(0)
{

}
//-----------------------------------------------------------------------------

/**
	Destructor
*/
KbdDevice::~KbdDevice()
{
	access.release();
}
//-----------------------------------------------------------------------------

/**
	This method implements the non-blocking read function.

	\return	If a key is pressed, returns the ucrrent PC-keyboard key or
	\ref  KBD_DEVICE_NO_KEY otherwise
*/
int KbdDevice::ReadKey(void)
{
	if(keyPressed){
		return currentKey;
	} else {
		return -1;
	}
}
//-----------------------------------------------------------------------------

/**
	This method implements the blocking read function.
*/
int KbdDevice::GetKey(void)
{
	access.acquire();
	int key = currentKey;
	access.release();

	return key;
}
//-----------------------------------------------------------------------------

/**

*/
bool KbdDevice::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		qDebug("KbdDevice::eventFilter(): key press %d", keyEvent->key());
		++keyCount;												// increase reference count for pressed keys
		keyPressed = true;										// once we receive a kepress-event at least one key
																// ... is pressed until we receive the same amount of
																// ... key-release events
		currentKey = keyEvent->key();
		access.release();
		return true;
	} else if(event->type() == QEvent::KeyRelease) {
		--keyCount;
		if(0 == keyCount){
			keyPressed = false;
		}
		access.acquire();

		return true;
	} else {
		// standard event processing
		return QObject::eventFilter(obj, event);
	}
}
//-----------------------------------------------------------------------------
