#ifndef KEYBOARDDIALOG_H
#define KEYBOARDDIALOG_H

#include <QDialog>

#include "chip8keyboard.h"
#include "kbddevice.h"
#include "keyboardmappingdialog.h"

namespace Ui {
class KeyboardDialog;
}

class KeyboardDialog : public QDialog
{
	Q_OBJECT

public:
	explicit KeyboardDialog(Chip8Keyboard* aKeyboard, QWidget *parent = nullptr);
	~KeyboardDialog();

public slots:
	void ButtonPress(int button);							///< Set the GUI button corresponding to the associated key on the keyboard to pressed
	void ButtonRelease(int button);							///< Set the GUI button corresponding to the associated key on the keyboard to released

private slots:
	void on_changeMappingPushButton_clicked(bool checked);	///<
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();
	void on_pushButton_3_clicked();
	void on_pushButton_4_clicked();
	void on_pushButton_5_clicked();
	void on_pushButton_6_clicked();
	void on_pushButton_7_clicked();
	void on_pushButton_8_clicked();
	void on_pushButton_9_clicked();
	void on_pushButton_0_clicked();
	void on_pushButton_A_clicked();
	void on_pushButton_B_clicked();
	void on_pushButton_C_clicked();
	void on_pushButton_D_clicked();
	void on_pushButton_E_clicked();
	void on_pushButton_F_clicked();

private:
	void SetLabel(QString text);
	QString MapKey(int);
	void HandleKey(unsigned int aKey);

	Ui::KeyboardDialog *ui;
	KeyboardMappingDialog*	selectDialog;

	Chip8Keyboard*			keyboard;
	bool					modeChangeMaping;
};

#endif // KEYBOARDDIALOG_H
