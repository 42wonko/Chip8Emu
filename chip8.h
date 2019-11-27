#ifndef CHIP8_H
#define CHIP8_H

#include <QObject>
#include <QTimer>
#include <iostream>
#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>

#include "chip8keyboard.h"

#define VM_SIZE	8192
#define CHAR_SIZE	5

class Chip8Display;

class CHIP8 : public QObject
{
	Q_OBJECT

	public:
		enum EMULATION_MODE {
			MODE_CLASSIC    = 0,
			MODE_SUPER      = 1
		};

		enum EXECUTION_MODE {
			MODE_RUNNING	= 0,
			MODE_STEP		= 1
		};

		enum MEMORY_MAP {
			MAP_INTPRT_START	= 0x000,
			MAP_CHAR_TBL_START	= 0x100,
			MAP_INTPRT_END		= 0x1ff,
			MAP_RAM_START		= 0x200,
			MAP_ETI_RAM_SART	= 0x600,
			MAP_RAM_END			= 0xfff
		};

		enum WIN_SIZE {
			WIN_ROWS	= 32,	///< Original CHIP8 rows (32)
			WIN_COLS	= 64,	///< Original CHIP8 columns (64)
			WIN_S_ROWS	= 64,	///< S-CHIP8 rows
			WIN_S_COLS	= 128	///< S-CHIP8 columns
		};

		enum OP_MASK{
			MSK_OP_CODE	= 0xf000,
			MSK_ADDR	= 0x0fff,
			MSK_REG_X	= 0x0f00,
			MSK_REG_Y	= 0x00f0,
			MSK_CONST	= 0x00ff
		};

		enum OP_CODE{
			OC_CALL		= 0x0000,	///< 0NNN - call RCA 1802 program at address NNN
			OC_DSP_CLR	= 0x00e0,	///< 00E0 - clear screen
			OC_RET		= 0x00ee,	///< 00ee - retun from subroutine
			OC_JMP		= 0x1000,	///< 1NNN - Jump to address NNN
			OC_JSR		= 0x2000,	///< 2NNN - Jump to subroutine at address NNN (call subroutine)
			OC_SKP_EQ	= 0x3000,	///< 3XNN - Skip next instruction if VX == NN
			OC_SKP_NEQ	= 0x4000,	///< 4XNN - Skip next instruction if VX != NN
			OC_SKP_EREG	= 0x5000,	///< 5XY0 - Skip next instruction if VX == VY
			OC_SET_VX	= 0x6000,	///< 6XNN - Set Vx = NN
			OC_ADD_K	= 0x7000,	///< 7XNN - Add NN to Vx, Carry flag is not changed
			OC_ASS_VXY	= 0x8000,	///< 8XY0 - Set Vx = Vy
			OC_OR_VXY	= 0x8001,	///< 8XY1 - Bitwise OR, Vx = Vx | Vy
			OC_AND_VXY	= 0x8002,	///< 8XY2 - Bitwise AND, VX = Vx & Vy
			OC_XOR_VXY	= 0x8003,	///< 8XY3 - Bitwise XOR, Vx = Vx ^ Vy
			OC_ADD_REG	= 0x8004,	///< 8XY4 - Add Vy to Vx, Vx = Vx + Vy, Set Vf=1 if carry - Vf=0 if no carry
			OC_SUB_REG	= 0x8005,	///< 8XY5 - Subtract Vy from Vx, Vx = Vx - Vy, Vf=0 if borrow, Vf=1 if no borrow
			OC_ASR		= 0x8006,	///< 8XY6 - Vf=LSB(Vx) and Vx >> 1 (??? Was soll das Y ???)
			OC_SUB_NREG	= 0x8007,	///< 8XY7 - Vx=Vy-Vx, Vf=0 if borrow, Vf=1 if no borrow
			OC_ASL		= 0x800e,	///< 8XYe - Vf=MSB(Vx), Vx << 1 (??? Was soll das Y ???)
			OC_SKP_NREG	= 0x9000,	///< 9XY0 - Skip next instruction if VX != VY
			OC_LD_ADD	= 0xa000,	///< aNNN - I = NNN
			OC_JMP_IDX	= 0xb000,	///< bNNN - Jump to address NNN+V0
			OC_RND		= 0xc000,	///< cXNN - Vx=Rnd()&NN, Set Vx to bitwise and of a random number (0-255) and NN
			OC_DRAW		= 0xd000,	///< dXYN - Draw a sprite at (Vx,Vy) Width=8 pixel, height=N, sprite starts (bitcoded) at address I
			OC_SKP_KEY	= 0xe09e,	///< eX9e - Skip next instruction if key == Vx
			OC_SKP_NKEY	= 0xe0a1,	///< eXa1 - Skip next instruction if key != Vx
			OC_GET_TD	= 0xf007,	///< fX07 - Set Vx to value of delay timer
			OC_GET_KEY	= 0xf00a,	///< fX0a - Wait for keypress and store to Vx
			OC_SET_TD	= 0xf015,	///< fX15 - Set delay timer to Vx
			OC_SET_TS	= 0xf018,	///< fX18 - Set sound timer to Vx
			OC_INC_ADD	= 0xf01e,	///< fX1e - Add Vx to address register, I=I+Vx
			OC_SET_SPT	= 0xf029,	///< fX29 - Set I to address of sprite for character in Vx (0-f) Fonts are 4x5
			OC_STO_BCD	= 0xf033,	///< fX33 - Store Vx as BCD at Address I
			OC_DMP_REG	= 0xf055,	///< fX55 - Store V0 to Vx in memory starting at address I. I is not modified.
			OC_FIL_REG	= 0xf065	///< fX65 - Load registers V0 to Vx with values starting at address I. I is not modified.
		};

		explicit CHIP8(Chip8Keyboard* aKeyboard, QObject* aParent = nullptr);
		~CHIP8();
		void mode(EMULATION_MODE mode);
		EMULATION_MODE mode(void){return emuMode;}
		unsigned int width(void){return dsp_width;}
		unsigned int height(void){return dsp_height;}
		int load(std::string program, u_int16_t address);
		int load_file(std::string filename, u_int16_t address);
		void set_address(u_int16_t address){PC = address;}
		std::vector<std::string> disassemble(void);
		bool log(void){return f_log;}
		bool trace(void){return f_trace;}
		bool ptrace(void){return f_ptrace;}
		int open_log(void);
		void close_log(void);
		void set_logname(std::string filename);
		void trace_on(void){f_trace = true;}
		void trace_of(void){f_trace = false;}
		void log_on(void){f_log = true;}
		void log_of(void){f_log = false;}
		void ptrace_on(void){f_ptrace = true;}
		void ptrace_of(void){f_ptrace = false;}
		Chip8Display* display(void){return mDsp;}

	signals:
		void ButtonPress(int button);					///< Signal a button press to the main window for possible display.
		void ButtonRelease(int button);					///< Signal a button release to the main window for possible display.
		void UpdateTs(u_int8_t const ts);				///< Send new value of sound timer to main window for display.
		void UpdateTd(u_int8_t const td);				///< Send new value of delay timer to main window for display.
		void UpdateV(unsigned char const* regs);		///< Send new values of registers to main window for display.
		void UpdateM(u_int16_t const M);				///< Send new value of memory pointer to main window for display.
		void UpdateI(u_int16_t const I);				///< Send current instruction to main window for display.
		void UpdatePC(u_int16_t const pc);				///< Send new value of program counter to main window for display.
		void UpdateStack(u_int16_t const*  stack);		///< Send current stack contents to main window for display.
		void UpdateSP(u_int16_t const sp);				///< Send current stack pointer to main windows for display.

	public slots:
		void Run(u_int16_t address);					///< This slot executes the program at \ref address in a new thread.
		void Stop(void);								///< This slot interrupts the running thread (but keeps it alive)
		void Step(void);								///< This slot single-steps the program.
		void Continue(void);							///< This slot continues after an interrupt.
		void Clock(int time){sleep_time = time;}		///< This slot changes the emulation speed.
		void Reset(void);								///< This slot stops the current program and terminates the thread.

	private:
		void start_timers(void);									///< Start the 60Hz CHIP8 timers
		void log_msg(char const* msg);								///< Write log-messages if enabled.
		void trace_msg(char const* msg);							///< Write trace-messages if enabled.
		void p_trace_msg(char const* msg);							///< Write program-trace-messages if enabled.
		int	 run(u_int16_t address, std::future<void> exitRequest);	///< The main emulation routine.
		void handle_timers(void);									///< Handler for Chip8 timers.
		std::string parse_op_code(u_int16_t op_code, u_int16_t pc);

		Chip8Display*			mDsp;						///< Our display object.
		std::string				log_filename;				///< Name of the logfile.
		FILE*					log_file;					///< File handle for the logfile.
		unsigned char*			ram;						///< The memory of the CHIP8 emulation.
		u_int16_t				program_size;				///< The size of the memory of the CHIP8 emulation.
		EMULATION_MODE			emuMode;					///< Indicates if we are emulation the classic CHIP8 or the SuperCHIP.
		EXECUTION_MODE			execMode;
		bool					emulatorRunning;
		unsigned char			V[16];						///< Registers  V0 - Vf.
		u_int16_t				M;							///< Memory register.
		u_int16_t				PC;							///< Program counter.
		u_int16_t				Stack[16];					///< Our 16 level deep stack.
		u_int16_t				I;							///< Instruction register.
		u_int16_t				SP;							///< Stack pointer.
		u_int8_t				TD;							///< Delay timer.
		u_int8_t				TS;							///< Sound timer.
		int						sleep_time; 				///< Constant to adjust emulation speed.
		unsigned int			dsp_width;					///< Current width of the display.
		unsigned int			dsp_height;					///< Current height of the display.
		bool					f_trace;					///< Indicates whether we are writing a fuction trace or not.
		bool					f_log;						///< Indicates whether we are writing genaral log info or not.
		bool					f_ptrace;					///< Indicates whether we are writing a program trace or not.
		Chip8Keyboard*			keyboard;					///< Our emulation of the CHIP( keyboard.
		QTimer*             	emuTimer;					///< Timer to handle the CHIP8 sound- and delay-timers.
		std::thread*			runMethod;
		std::future<void>		exitThread;					///< Future object to cancel the run method
		std::promise<void>*		exitSignal;					///<
		std::mutex				mtx;						///< Synchronize access to condition variable to control exec mode
		std::condition_variable	cond_var;					///< Used to control the execution mode (halt, step, continue)
		bool 					do_step;

		static unsigned char CHAR_0[];
		static unsigned char CHAR_1[];
		static unsigned char CHAR_2[];
		static unsigned char CHAR_3[];
		static unsigned char CHAR_4[];
		static unsigned char CHAR_5[];
		static unsigned char CHAR_6[];
		static unsigned char CHAR_7[];
		static unsigned char CHAR_8[];
		static unsigned char CHAR_9[];
		static unsigned char CHAR_a[];
		static unsigned char CHAR_b[];
		static unsigned char CHAR_c[];
		static unsigned char CHAR_d[];
		static unsigned char CHAR_e[];
		static unsigned char CHAR_f[];
};

#endif // CHIP8_H
