#ifndef CHIP8MAINWINDOW_H
#define CHIP8MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QDialog>
#include <QStringListModel>
//#include <QGraphicsScene>

#include "chip8.h"
#include "kbddevice.h"
#include "chip8graphicsview.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Chip8MainWindow; }
QT_END_NAMESPACE

class Chip8MainWindow : public QMainWindow
{
	Q_OBJECT
	QThread emuThread;

	public:
		Chip8MainWindow(QWidget *parent = nullptr);
		~Chip8MainWindow();
		CHIP8* get_emu(void){return emu;}
		u_int16_t get_address(void){return address;}
		void update_display(std::vector<std::vector<bool>> dsp);

	signals:
		void Run(u_int16_t address);
		void Stop(void);
		void Step(void);
		void Continue(void);
		void Clock(int freq);

	public slots:
//		void DrawScreen(std::vector<std::vector<bool>> dsp);
		void ButtonPress(int button);
		void ButtonRelease(int button);
		void UpdateTs(u_int8_t const ts);
		void UpdateTd(u_int8_t const td);
		void UpdateV(unsigned char const* regs);
		void UpdateM(u_int16_t const M);
		void UpdateI(u_int16_t const I);
		void UpdatePC(u_int16_t const pc);
		void UpdateStack(u_int16_t const*  stack);
		void UpdateSP(u_int16_t const sp);

	private slots:
		void on_clockFreqSlider_valueChanged(int value);
		void on_loadButton_clicked();
		void on_runButton_clicked();
		void on_toolButton_clicked();
		void on_stopExecutionButton_clicked();
		void on_singleStepButton_clicked();
		void on_continueButton_clicked();
		void on_rtTraceCheckBox_stateChanged(int arg1);
		void on_resetButton_clicked();

		void on_keyboardButton_clicked();

private:
		Ui::Chip8MainWindow *ui;
		bool					rtTrace;
		Chip8Keyboard*			keyboard;
		KbdDevice*				kbdDevice;
		QDialog*				kbdDialog;
		QDialog*				configDialog;
		QStringListModel*		list_model;
		QStringList				code_list;
		CHIP8*                  emu;
		u_int16_t				address;
//		QGraphicsScene*			gs;
		Chip8GraphicsView*		cgv;
};
#endif // MAINWINDOW_H
