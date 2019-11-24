#ifndef KEYBOARDMAPPINGDIALOG_H
#define KEYBOARDMAPPINGDIALOG_H

#include <QDialog>
#include <chip8keyboard.h>

namespace Ui {
class KeyboardMappingDialog;
}

class KeyboardMappingDialog : public QDialog
{
	Q_OBJECT

public:
	explicit KeyboardMappingDialog(Chip8Keyboard* aKeyboard, QWidget *parent = nullptr);
	~KeyboardMappingDialog() override;
	void open() override;
	void SetKey(int key){selectedKey = key;}

private slots:
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();

private:
	Ui::KeyboardMappingDialog*	ui;
	int							selectedKey;
	Chip8Keyboard*				keyboard;
};

#endif // KEYBOARDMAPPINGDIALOG_H
