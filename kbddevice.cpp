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
		return KBD_DEVICE_NO_KEY;
	}
}
//-----------------------------------------------------------------------------

/**
	This method implements the blocking read function.

	The event filter will block access until the next key-press event occurrs.
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
	This method implements the event filter to handle the keypress and key-release
	events.

	If we receive a key-press event we set the member \ref keyPressed to true and
	maintain a reference count of the number of incoming key-press events. If we
	receive a key-release event we decrement the counter for the number of pressed
	keys and set \ref keyPressed to false when we reach a count of 0.

	When the CHIP8 emulator calls the non-blocking read function we simply check the
	state of the \ref keyPressed member and return the proper kay value.

	The blocking read  uses a semaphore to block access to the keyboard device after the
	last key has been released (keyCount = 0). The blocking read will now block until
	the next key-press event arrives and releases the semaphore.

	NOTE: The calling object has to be running in a different thread.

	All events that are not QEvent::KeyPress or QEvent::KeyRelease are passed on the
	the normal event handler.


	\param	[in]	obj	???
	\param	[in]	event	The incoming event.
*/
bool KbdDevice::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
//		qDebug("KbdDevice::eventFilter(): key press %d", keyEvent->key());
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
