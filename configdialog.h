#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include "mainwindow.h"
#include "chip8.h"

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ConfigDialog(Chip8MainWindow* win, QWidget *parent = nullptr);
	~ConfigDialog() override;
	void open() override;

private slots:
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();

private:
	Ui::ConfigDialog*	ui;
	Chip8MainWindow*	mainWin;
	CHIP8*				emu;
};

#endif // CONFIGDIALOG_H
