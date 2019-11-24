#include <QFileDialog>

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "configdialog.h"
#include "keyboarddialog.h"
#include "chip8display.h"

/**
	Constructor for the CHIP8 emulator main window
	\param	[in]	parent	???
*/
Chip8MainWindow::Chip8MainWindow(QWidget *parent)
: QMainWindow(parent), ui(new Ui::Chip8MainWindow), rtTrace(true), address(0x200)
{
	ui->setupUi(this);

	qRegisterMetaType<u_int16_t>("u_int16_t");
	qRegisterMetaType<u_int8_t>("u_int8_t");
	qRegisterMetaType<std::vector<std::vector<bool>> >("std::vector<std::vector<bool> >");

	kbdDevice		= new KbdDevice(this);												// create our handler for keyboard events
	this->installEventFilter(kbdDevice);												// ... and install it as a filter

	keyboard		= new Chip8Keyboard(kbdDevice);										// create keyboard wrapper for emulator and provide our event filter

	emu = new CHIP8(keyboard, this);													// create our emulator
	emu->moveToThread(&emuThread);														// move emulator into thread

	connect(&emuThread,	&QThread::finished, emu, &QObject::deleteLater);				// connect the destroy signal
	connect(this,		&Chip8MainWindow::Run,	emu, &CHIP8::Run);						// connect a signal to emulator to actually start emulating
	connect(emu,		&CHIP8::UpdateI,		this, &Chip8MainWindow::UpdateI);
	connect(emu,		&CHIP8::UpdateM,		this, &Chip8MainWindow::UpdateM);
	connect(emu,		&CHIP8::UpdatePC,		this, &Chip8MainWindow::UpdatePC);
	connect(emu,		&CHIP8::UpdateTd,		this, &Chip8MainWindow::UpdateTd);
	connect(emu,		&CHIP8::UpdateTs,		this, &Chip8MainWindow::UpdateTs);
	connect(emu,		&CHIP8::UpdateSP,		this, &Chip8MainWindow::UpdateSP);
	connect(emu,		&CHIP8::UpdateV,		this, &Chip8MainWindow::UpdateV);
	connect(emu,		&CHIP8::UpdateStack,	this, &Chip8MainWindow::UpdateStack);

	configDialog	= new ConfigDialog(this);											// Create our configuration dialog. This MUST be done after creating the emulator object.
	kbdDialog		= new KeyboardDialog(keyboard, this);								// This dialog MUST be created after the emulator object because it connects some signals to it
	list_model		= new QStringListModel(this);										// create a list_model for our list-view that displays the source code

	cgv = new Chip8GraphicsView(emu->width(), emu->height(), ui->graphicsView, this);	// install our api to draw on QtGraphicsView
	cgv->Resize(emu->width(), emu->height());
	cgv->Clear();

	emuThread.start();																	// start the tread
}
//-----------------------------------------------------------------------------

/**
	destructor of the CHIP8 emulator main window.
*/
Chip8MainWindow::~Chip8MainWindow()
{
	delete cgv;
	delete list_model;
	emuThread.terminate();
	emuThread.wait();
	delete emu;
	delete configDialog;
	delete kbdDialog;
	delete keyboard;
	delete kbdDevice;
	delete ui;
}
//-----------------------------------------------------------------------------

/**
	This is the callback for the clock-frequency slider widget.
	\param	[in]	value	The new slider value [0-100]
*/
void Chip8MainWindow::on_clockFreqSlider_valueChanged(int value)
{
	int freq = 500 + (30*value);
	emit Clock(freq);															// set new clock frequency for emulator
}
//-----------------------------------------------------------------------------


/**
	This callback is called when the Load-Button is clicked.
	Its purpose is to open a file dialog, let the user select a
	CHIP8 program and then load it into the emulator
*/
void Chip8MainWindow::on_loadButton_clicked()
{
	QString filename =  QFileDialog::getOpenFileName(this, tr("Open Chip8 Program"),QDir::homePath(), tr("Chip8 Programs (*.ch8)"));	// open file-dialog in users home dir
	if(! filename.isEmpty()){										// only do something if the user selected a file
		emu->load_file(filename.toStdString(), address);			// load CHIP8 program into emulator at default address
		std::vector<std::string> prog = emu->disassemble();			// try to disassemble the program ...
		code_list.clear();											// ... and put the assembler text in out code-view
		for(auto entry : prog){
			code_list.append(entry.c_str());
		}
		list_model->setStringList(code_list);
		ui->codeListView->setModel(list_model);
		ui->codeListView->setSelectionRectVisible(true);
		ui->codeListView->setSelectionMode(QAbstractItemView::SingleSelection);
		QModelIndex index = ui->codeListView->model()->index(code_list.indexOf(QRegExp(QString().sprintf("\\$%X.*", address))),0);
		ui->codeListView->setCurrentIndex(index);
		ui->codeListView->selectionModel()->select(index,QItemSelectionModel::Select);
	}
}
//-----------------------------------------------------------------------------

/**
	This signal is emitted to actually start the program on the emulator
*/
void Chip8MainWindow::on_runButton_clicked()
{
	emit Run(address);																	// signal to start emulating
}
//-----------------------------------------------------------------------------

/**
	This is the callback for the Config-Button.
*/
void Chip8MainWindow::on_toolButton_clicked()
{
	if(configDialog){
		configDialog->setModal(true);				// make sure we continue only when the dialog is closed again
		configDialog->open();
		ui->graphicsView->scene()->clear();			// since we may have changed the resolutions clear display
		unsigned int pixel_width	= static_cast<unsigned int>(ui->graphicsView->width()) / emu->width();
		unsigned int pixel_height	= static_cast<unsigned int>(ui->graphicsView->height()) / emu->height();
		for(unsigned int x = 0; x < emu->width(); ++x){
			for(unsigned int y = 0; y < emu->height(); ++y){
				QRectF rec(x*pixel_width, y*pixel_height, pixel_width, pixel_height);
				ui->graphicsView->scene()->addRect(rec, QPen(Qt::white), QBrush(Qt::white));

			}
		}
	}
}
//-----------------------------------------------------------------------------

/**

*/
void Chip8MainWindow::on_resetButton_clicked()
{
	emit Reset();
}
//-----------------------------------------------------------------------------

/**

*/
void Chip8MainWindow::on_stopExecutionButton_clicked()
{
	emit Stop();
}
//-----------------------------------------------------------------------------

/**

*/
void Chip8MainWindow::on_singleStepButton_clicked()
{
	emit Step();
}
//-----------------------------------------------------------------------------

/**

*/
void Chip8MainWindow::on_continueButton_clicked()
{
	emit Continue();
}
//-----------------------------------------------------------------------------

/**

*/
void Chip8MainWindow::on_rtTraceCheckBox_stateChanged(int arg1)
{
	if(ui->rtTraceCheckBox->isChecked()){
		rtTrace = false;
	} else {
		rtTrace = true;
	}
}
//-----------------------------------------------------------------------------

/**

*/
/*
void Chip8MainWindow::DrawScreen(std::vector<std::vector<bool>> dsp)
{
	ui->graphicsView->scene()->clear();			// since we may have changed the resolutions clear display
	unsigned int pixel_width	= static_cast<unsigned int>(ui->graphicsView->width()) / emu->width();
	unsigned int pixel_height	= static_cast<unsigned int>(ui->graphicsView->height()) / emu->height();

	for(unsigned int x = 0; x < emu->width(); ++x){
		for(unsigned int y = 0; y < emu->height(); ++y){
			QRectF rec(x*pixel_width, y*pixel_height, pixel_width, pixel_height);
			QColor color(dsp[x][y] ? Qt::black : Qt::white);
			ui->graphicsView->scene()->addRect(rec, QPen(color), QBrush(color));
		}
	}
}
*/
//-----------------------------------------------------------------------------

/**

*/
void Chip8MainWindow::ButtonPress(int button)
{

}
//-----------------------------------------------------------------------------

/**

*/
void Chip8MainWindow::ButtonRelease(int button)
{

}
//-----------------------------------------------------------------------------

/**

*/
void Chip8MainWindow::UpdateTs(u_int8_t const ts)
{
	if(rtTrace){
		ui->tsRegLabel->setText(QString().sprintf("0x%02X", ts));
	}
}
//-----------------------------------------------------------------------------

/**

*/
void Chip8MainWindow::UpdateTd(u_int8_t const td)
{
	if(rtTrace){
		ui->tsRegLabel->setText(QString().sprintf("0x%02X", td));
	}
}
//-----------------------------------------------------------------------------

/**

*/
void Chip8MainWindow::UpdateV(unsigned char const* regs)
{
	if(rtTrace){
		ui->V0_Label->setText(QString().sprintf("0x%02X", regs[0]));
		ui->V1_Label->setText(QString().sprintf("0x%02X", regs[1]));
		ui->V2_Label->setText(QString().sprintf("0x%02X", regs[2]));
		ui->V3_Label->setText(QString().sprintf("0x%02X", regs[3]));
		ui->V4_Label->setText(QString().sprintf("0x%02X", regs[4]));
		ui->V5_Label->setText(QString().sprintf("0x%02X", regs[5]));
		ui->V6_Label->setText(QString().sprintf("0x%02X", regs[6]));
		ui->V7_Label->setText(QString().sprintf("0x%02X", regs[7]));
		ui->V7_Label->setText(QString().sprintf("0x%02X", regs[8]));
		ui->V8_Label->setText(QString().sprintf("0x%02X", regs[9]));
		ui->VA_Label->setText(QString().sprintf("0x%02X", regs[10]));
		ui->VB_Label->setText(QString().sprintf("0x%02X", regs[11]));
		ui->VC_Label->setText(QString().sprintf("0x%02X", regs[12]));
		ui->VD_Label->setText(QString().sprintf("0x%02X", regs[13]));
		ui->VE_Label->setText(QString().sprintf("0x%02X", regs[14]));
		ui->VF_Label->setText(QString().sprintf("0x%02X", regs[15]));
	}
}
//-----------------------------------------------------------------------------

/**

*/
void Chip8MainWindow::UpdateM(u_int16_t const M)
{
	if(rtTrace){
		ui->memRegLabel->setText(QString().sprintf("0x%03X", M));
	}
}
//-----------------------------------------------------------------------------

/**

*/
void Chip8MainWindow::UpdateI(u_int16_t const I)
{
	if(rtTrace){
		ui->InstructionLabel->setText(QString().sprintf("0x%04X", I));
	}
}
//-----------------------------------------------------------------------------

/**

*/
void Chip8MainWindow::UpdatePC(u_int16_t const pc)
{
	if(rtTrace){
		QModelIndex index = ui->codeListView->model()->index(code_list.indexOf(QRegExp(QString().sprintf("\\$%X.*", pc))),0);	// update code view
		ui->codeListView->setCurrentIndex(index);
		ui->codeListView->selectionModel()->select(index,QItemSelectionModel::Select);

		ui->pcRegLabel->setText(QString().sprintf("0x%03X", pc));																// update PC widget
	}
}
//-----------------------------------------------------------------------------

/**

*/
void Chip8MainWindow::UpdateStack(u_int16_t const*  stack)
{
	if(rtTrace){
		// TBD
	}
}
//-----------------------------------------------------------------------------

/**

*/
void Chip8MainWindow::UpdateSP(u_int16_t const sp)
{
	if(rtTrace){
		ui->spRegLabel->setText(QString().sprintf("0x%03X", sp));
	}
}
//-----------------------------------------------------------------------------

void Chip8MainWindow::on_keyboardButton_clicked()
{
	kbdDialog->open();
}
