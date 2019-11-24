#include "configdialog.h"
#include "ui_configdialog.h"

/**

*/
ConfigDialog::ConfigDialog(Chip8MainWindow* win, QWidget *parent)
: QDialog(parent)
, ui(new Ui::ConfigDialog)
, mainWin(win)
{
	ui->setupUi(this);
	emu = mainWin->get_emu();
}
//-----------------------------------------------------------------------------

/**

*/
ConfigDialog::~ConfigDialog()
{
	delete ui;
}
//-----------------------------------------------------------------------------

/**
	This method is called when the mainwindow calls dialog->open().
	Here we need to fill our widgets with the current settings of the
	emulator.
*/
void ConfigDialog::open()
{
	ui->addressLineEdit->setText(QString::number(mainWin->get_address()));
	ui->debugCheckBox->setChecked(emu->log());
	ui->traceCheckBox->setChecked(emu->trace());
	ui->ptraceCheckBox->setChecked(emu->ptrace());
	if(CHIP8::MODE_CLASSIC == emu->mode()){
		ui->classicRadioButton->setChecked(true);
	} else {
		ui->superRadioButton->setChecked(true);
	}
	QDialog::open();
}
//-----------------------------------------------------------------------------

/**
	This method is called when we leave the dialog by clicking the OK button.
	Now we have to write the current configuration selection back to the
	emulator object.
*/
void ConfigDialog::on_buttonBox_accepted()
{
	emu->set_address(ui->addressLineEdit->text().toInt());
	if(ui->debugCheckBox->isChecked()){
		emu->log_on();
	} else {
		emu->log_of();
	}
	if(ui->traceCheckBox->isChecked()){
		emu->trace_on();
	} else {
		emu->trace_of();
	}
	if(ui->ptraceCheckBox->isChecked()){
		emu->ptrace_on();
	} else {
		emu->ptrace_of();
	}
	if(ui->classicRadioButton->isChecked()){
		emu->mode(CHIP8::MODE_CLASSIC);
	} else {
		emu->mode(CHIP8::MODE_SUPER);
	}
	emu->set_logname(ui->filenameLineEdit->text().toStdString());
}
//-----------------------------------------------------------------------------

/**

*/
void ConfigDialog::on_buttonBox_rejected()
{

}
//-----------------------------------------------------------------------------
