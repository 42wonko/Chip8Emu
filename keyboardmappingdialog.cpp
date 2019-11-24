#include "keyboardmappingdialog.h"
#include "ui_keyboardmappingdialog.h"

/**

*/
KeyboardMappingDialog::KeyboardMappingDialog(Chip8Keyboard* aKeyboard, QWidget *parent)
: QDialog(parent), ui(new Ui::KeyboardMappingDialog), keyboard(aKeyboard)
{
	ui->setupUi(this);
}
//-----------------------------------------------------------------------------

/**

*/
KeyboardMappingDialog::~KeyboardMappingDialog()
{
	delete ui;
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardMappingDialog::open()
{
	QString text;
	text = keyboard->GetMappedKey(selectedKey);
	ui->currentSettingLabel->setText(text);

	QDialog::open();
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardMappingDialog::on_buttonBox_accepted()
{
	QString text = ui->newSettingLineEdit->text();
	keyboard->MapKey(text.data()[0].toLatin1(), selectedKey);
}
//-----------------------------------------------------------------------------

/**

*/
void KeyboardMappingDialog::on_buttonBox_rejected()
{
}
//-----------------------------------------------------------------------------
