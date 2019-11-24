#include "keyboarddialog.h"
#include "ui_keyboarddialog.h"
#include "chip8.h"
#include "mainwindow.h"

/**

*/
KeyboardDialog::KeyboardDialog(Chip8Keyboard* aKeyboard, QWidget *parent)
: QDialog(parent), ui(new Ui::KeyboardDialog), keyboard(aKeyboard), modeChangeMaping(false)
{
	ui->setupUi(this);

	selectDialog = new KeyboardMappingDialog(keyboard, this);

	connect(dynamic_cast<Chip8MainWindow*>(parent)->get_emu(), &CHIP8::ButtonPress, this, &KeyboardDialog::ButtonPress);	// connect a signal from emulator to us to react on a button press event
	connect(dynamic_cast<Chip8MainWindow*>(parent)->get_emu(), &CHIP8::ButtonRelease, this, &KeyboardDialog::ButtonRelease);// connect a signal from emulator to us to react on a button release event
}
//-----------------------------------------------------------------------------

/**

*/
KeyboardDialog::~KeyboardDialog()
{
	delete selectDialog;
	delete ui;
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardDialog::on_changeMappingPushButton_clicked(bool checked)
{
	if(checked){
		modeChangeMaping = true;
	} else {
		modeChangeMaping = false;
	}
}
//-----------------------------------------------------------------------------

/**
	A private convenience function to reduce code.
*/
void KeyboardDialog::HandleKey(unsigned int aKey)
{
	if(true == modeChangeMaping){
		if(selectDialog){
			selectDialog->setModal(true);
			selectDialog->SetKey(aKey);
			selectDialog->open();
		}
	}
	SetLabel(MapKey(aKey));
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardDialog::on_pushButton_clicked()
{
		HandleKey(1);
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardDialog::on_pushButton_2_clicked()
{
	HandleKey(2);
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardDialog::on_pushButton_3_clicked()
{
	HandleKey(3);
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardDialog::on_pushButton_4_clicked()
{
	HandleKey(4);
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardDialog::on_pushButton_5_clicked()
{
	HandleKey(5);
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardDialog::on_pushButton_6_clicked()
{
	HandleKey(6);
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardDialog::on_pushButton_7_clicked()
{
	HandleKey(7);
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardDialog::on_pushButton_8_clicked()
{
	HandleKey(8);
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardDialog::on_pushButton_9_clicked()
{
	HandleKey(9);
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardDialog::on_pushButton_0_clicked()
{
	HandleKey(0);
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardDialog::on_pushButton_A_clicked()
{
	HandleKey(0xa);
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardDialog::on_pushButton_B_clicked()
{
	HandleKey(0xb);
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardDialog::on_pushButton_C_clicked()
{
	HandleKey(0xc);
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardDialog::on_pushButton_D_clicked()
{
	HandleKey(0xd);
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardDialog::on_pushButton_E_clicked()
{
	HandleKey(0xe);
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardDialog::on_pushButton_F_clicked()
{
	HandleKey(0xf);
}
//-----------------------------------------------------------------------------

/**

*/
QString KeyboardDialog::MapKey(int key)
{
	QString text;

	try {
		text = keyboard->GetMappedKey(key);
	} catch (...) {

	}

	return text;
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardDialog::SetLabel(QString text)
{
	ui->key_label->setText(text);
}
//-----------------------------------------------------------------------------

/**
	This is a public slot that is signaled by the keyboard device when a button
	on the PC keyboard is pressed to highlight the corresponding button on the GUI.

	\param	[in]	buttnId	Id of the button to highlight
	\return NONE
*/
void KeyboardDialog::ButtonPress(int buttonId)
{
	QPushButton*	button = 0;
	switch (buttonId){
		case 0:	button = ui->pushButton_0;
				break;
		case 1: button = ui->pushButton;
				break;
		case 2: button = ui->pushButton_2;
				break;
		case 3: button = ui->pushButton_3;
				break;
		case 4: button = ui->pushButton_4;
				break;
		case 5: button = ui->pushButton_5;
				break;
		case 6: button = ui->pushButton_6;
				break;
		case 7: button = ui->pushButton_7;
				break;
		case 8: button = ui->pushButton_8;
				break;
		case 9: button = ui->pushButton_9;
				break;
		case 0xa: button = ui->pushButton_A;
				break;
		case 0xb: button = ui->pushButton_B;
				break;
		case 0xc: button = ui->pushButton_C;
				break;
		case 0xd: button = ui->pushButton_D;
				break;
		case 0xe: button = ui->pushButton_E;
				break;
		case 0xf: button = ui->pushButton_F;
				break;
	}
	if(button){
		button->setDown(true);
	}
}
//-----------------------------------------------------------------------------

/**
	This is a public slot that is signaled by the keyboard device when a button
	on the PC keyboard is released to "un-highlight" the corresponding button on the GUI.

	\param	[in]	buttnId	Id of the button to released
	\return NONE
*/
void KeyboardDialog::ButtonRelease(int buttonId)
{
	QPushButton*	button = 0;
	switch (buttonId){
		case 0:	button = ui->pushButton_0;
				break;
		case 1: button = ui->pushButton;
				break;
		case 2: button = ui->pushButton_2;
				break;
		case 3: button = ui->pushButton_3;
				break;
		case 4: button = ui->pushButton_4;
				break;
		case 5: button = ui->pushButton_5;
				break;
		case 6: button = ui->pushButton_6;
				break;
		case 7: button = ui->pushButton_7;
				break;
		case 8: button = ui->pushButton_8;
				break;
		case 9: button = ui->pushButton_9;
				break;
		case 0xa: button = ui->pushButton_A;
				break;
		case 0xb: button = ui->pushButton_B;
				break;
		case 0xc: button = ui->pushButton_C;
				break;
		case 0xd: button = ui->pushButton_D;
				break;
		case 0xe: button = ui->pushButton_E;
				break;
		case 0xf: button = ui->pushButton_F;
				break;
	}
	if(button){
		button->setDown(false);
	}
}
//-----------------------------------------------------------------------------
