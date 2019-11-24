#include <iostream>
#include <fstream>
#include <chrono>

#include <arpa/inet.h>		// htons()...
#include <unistd.h>			// usleep()

#include "chip8.h"
#include "chip8display.h"

/**
	Define font for hex characters.
*/
unsigned char CHIP8::CHAR_0[] = {0xf0, 0x90, 0x90, 0x90, 0xf0};
unsigned char CHIP8::CHAR_1[] = {0x20, 0x60, 0x20, 0x20, 0x70};
unsigned char CHIP8::CHAR_2[] = {0xf0, 0x10, 0xf0, 0x80, 0xf0};
unsigned char CHIP8::CHAR_3[] = {0xf0, 0x10, 0xf0, 0x10, 0xf0};
unsigned char CHIP8::CHAR_4[] = {0x90, 0x90, 0xf0, 0x10, 0x10};
unsigned char CHIP8::CHAR_5[] = {0xf0, 0x80, 0xf0, 0x10, 0xf0};
unsigned char CHIP8::CHAR_6[] = {0xf0, 0x80, 0xf0, 0x90, 0xf0};
unsigned char CHIP8::CHAR_7[] = {0xf0, 0x10, 0x20, 0x40, 0x40};
unsigned char CHIP8::CHAR_8[] = {0xf0, 0x90, 0xf0, 0x90, 0xf0};
unsigned char CHIP8::CHAR_9[] = {0xf0, 0x90, 0xf0, 0x10, 0xf0};
unsigned char CHIP8::CHAR_a[] = {0xf0, 0x90, 0xf0, 0x90, 0x90};
unsigned char CHIP8::CHAR_b[] = {0xe0, 0x90, 0xe0, 0x90, 0xe0};
unsigned char CHIP8::CHAR_c[] = {0xf0, 0x80, 0x80, 0x80, 0xf0};
unsigned char CHIP8::CHAR_d[] = {0xe0, 0x90, 0x90, 0x90, 0xe0};
unsigned char CHIP8::CHAR_e[] = {0xf0, 0x80, 0xf0, 0x80, 0xf0};
unsigned char CHIP8::CHAR_f[] = {0xf0, 0x80, 0xf0, 0x80, 0x80};

/**
	This is the constructor of the CHIP8 emulator. It initializes all
	resisters to 0, installs a font for the HEX numbers to memory location
	\ref MAP_CHAR_TBL_START (currently 0x100).

	\param	[in]	aKeyboard	Pointer to the Chip8 keyboard emulation.
	\param	[in]	aParent		Pointer to the QtMainwindow.
*/
CHIP8::CHIP8(Chip8Keyboard* aKeyboard, QObject* aParent)
: log_file(nullptr)
, ram(nullptr), program_size(0), emuMode(MODE_CLASSIC), execMode(MODE_RUNNING), emulatorRunning(false), PC(0x200)
, I(0), SP(0x0f), TD(0), TS(0), sleep_time(1000), dsp_width(WIN_COLS), dsp_height(WIN_ROWS)
, f_trace(false), f_log(false), f_ptrace(false), keyboard(aKeyboard), runMethod(nullptr), do_step(true)
{
	Q_UNUSED(aParent)

	ram = new unsigned char[VM_SIZE];
	memset(ram, 0, VM_SIZE);
	for(int i = 0; i < 16; ++i){
		V[i] = 0;
	}
	int offset = MAP_CHAR_TBL_START;
	memcpy(ram+offset, CHAR_0,5), offset+=5;
	memcpy(ram+offset, CHAR_1,5), offset+=5;
	memcpy(ram+offset, CHAR_2,5), offset+=5;
	memcpy(ram+offset, CHAR_3,5), offset+=5;
	memcpy(ram+offset, CHAR_4,5), offset+=5;
	memcpy(ram+offset, CHAR_5,5), offset+=5;
	memcpy(ram+offset, CHAR_6,5), offset+=5;
	memcpy(ram+offset, CHAR_7,5), offset+=5;
	memcpy(ram+offset, CHAR_8,5), offset+=5;
	memcpy(ram+offset, CHAR_9,5), offset+=5;
	memcpy(ram+offset, CHAR_a,5), offset+=5;
	memcpy(ram+offset, CHAR_b,5), offset+=5;
	memcpy(ram+offset, CHAR_c,5), offset+=5;
	memcpy(ram+offset, CHAR_d,5), offset+=5;
	memcpy(ram+offset, CHAR_e,5), offset+=5;
	memcpy(ram+offset, CHAR_f,5), offset+=5;

	mDsp = new Chip8Display();
	emuTimer = new QTimer(this);																				// create timer for emulating the sound and delay timers
	connect(emuTimer, &QTimer::timeout, this, &CHIP8::handle_timers);											// connect callback to timer
	connect(dynamic_cast<Chip8MainWindow*>(aParent), &Chip8MainWindow::Clock,		this, &CHIP8::Clock);		// Let the user change the emulation speed
	connect(dynamic_cast<Chip8MainWindow*>(aParent), &Chip8MainWindow::Stop,		this, &CHIP8::Stop);		// Interrupt the current program
	connect(dynamic_cast<Chip8MainWindow*>(aParent), &Chip8MainWindow::Step,		this, &CHIP8::Step);		// Single-step the current program
	connect(dynamic_cast<Chip8MainWindow*>(aParent), &Chip8MainWindow::Continue,	this, &CHIP8::Continue);	// Continue current program
	connect(dynamic_cast<Chip8MainWindow*>(aParent), &Chip8MainWindow::Reset,		this, &CHIP8::Reset);		// Terminate current program

	exitThread = exitSignal.get_future();
}
//-----------------------------------------------------------------------------

/**
	Destructor to clean everything up again.
*/
CHIP8::~CHIP8()
{
	trace_msg("-T- CHIP8::~CHIP8() start");

	std::cerr << "CHIP8::~CHIP8(): quitting" << std::endl;

	emuTimer->stop();
	if(runMethod){
		exitSignal.set_value();
		runMethod->join();
	}
	delete [] ram;
	if(log_file){
		fclose(log_file);
	}
	trace_msg("-T- CHIP8::~CHIP8() end");
}
//-----------------------------------------------------------------------------

/**
	This method starts the CHIP8 60Hz timers
*/
void CHIP8::start_timers(void)
{
	emuTimer->start(16);							// start timer with 60Hz
}
//-----------------------------------------------------------------------------

/**
	This is the callback that handles the 60Hz timers (delay and soud).
*/
void CHIP8::handle_timers(void)
{
	if(TS > 0){
		--TS;
	}
	if(TD > 0){
		--TD;
	}
}
//-----------------------------------------------------------------------------


/**
	This method load a program provided as string into memory at address.
	\param [in]	program	the program code.
	\param [in]	address	Startaddress of the program.
	\return Always 0
*/
int CHIP8::load(std::string program, u_int16_t address)
{
	trace_msg("-T- CHIP8::load() start");

	memcpy(ram+address, program.data(), program.size());
	program_size = static_cast<u_int16_t>(program.size()/2);
	trace_msg("-T- CHIP8::load() end");
	return 0;
}
//-----------------------------------------------------------------------------

/**
	This method load a program provided as string into memory at address.
	\param [in]	filename Filename of the progrram to load.
	\param [in]	address	Startaddress of the program.
	\return - 0 on success
			- 1 file read failed
*/
int CHIP8::load_file(std::string filename, u_int16_t address)
{
	trace_msg("-T- CHIP8::log_file() start");

	std::string program;
	std::streampos	size;
	char* 		inbuf;

	try{
		std::ifstream file(filename, std::ios::in|std::ios::binary|std::ios::ate);
		if(file.is_open()){
			size = file.tellg();
			inbuf = new char [size];
			file.seekg(0, std::ios::beg);
			file.read(inbuf,size);
			file.close();
			program.assign(inbuf, static_cast<size_t>(size));
			delete [] inbuf;
		} else {
			std::string l_msg("-E- Couldn't read file <"+filename+">");
			log_msg(l_msg.c_str());
			trace_msg("-T- CHIP8::load_file() end 1");
			return 1;
		}
	} catch (...){
	}
	trace_msg("-T- CHIP8::load_file() end 2");

	return load(program, address);
}
//-----------------------------------------------------------------------------

/**
	This method sets the name of the file that is used to log all
	debug, trace and program-trace information.

	\param	[in]	filename	Name of the logfilen incl. the path.
	\retirn NONE
*/
void CHIP8::set_logname(std::string filename)
{
	log_filename = filename;
}
//-----------------------------------------------------------------------------

/**

*/
void CHIP8::log_msg(char const* msg)
{
	if(f_log){
		open_log();
		if(log_file){
			fprintf(log_file, "%s\n", msg), fflush(log_file);
			close_log();
		}
	}
}
//-----------------------------------------------------------------------------

/**

*/
void CHIP8::trace_msg(char const* msg)
{
	if(f_trace){
		open_log();
		if(log_file){
			fprintf(log_file, "%s\n", msg), fflush(log_file);
			close_log();
		}
	}
}
//-----------------------------------------------------------------------------

/**

*/
void CHIP8::p_trace_msg(char const* msg)
{
	if(f_ptrace){
		open_log();
		if(log_file){
			fprintf(log_file, "%s\n", msg), fflush(log_file);
			close_log();
		}
	}
}
//-----------------------------------------------------------------------------

/**

*/
int CHIP8::open_log(void)
{
	if(log_filename.empty()){
		std::cerr << "CHIP8::open_log(): no file name" << std::endl;
		return 0;
	}
	if((log_file = fopen(log_filename.c_str(), "a+"))==NULL){
		std::cerr << "CHIP8::open_log(): file open..." << std::endl;
		return 1;
	} else {
		std::cerr << "CHIP8::open_log(): file not open..." << std::endl;
		return 0;
	}
}
//-----------------------------------------------------------------------------

/**

*/
void CHIP8::close_log(void)
{
	if(log_file){
		fclose(log_file);
	}
	log_file = NULL;
}
//-----------------------------------------------------------------------------

/**
	This is the main emulation routine.
	\return Always 0
*/
int CHIP8::run(u_int16_t address, std::future<void> exitRequest)
{
	trace_msg("-T- CHIP8::run() start");

	u_int8_t 	reg_x	= 0;		// index of register X
	u_int8_t 	reg_y	= 0;		// index of register Y
	u_int8_t 	k		= 0;		// 8-bit constant
	u_int16_t	old_pc	= 0;
	u_int8_t	vx		= 0;
	u_int16_t	i_val	= 0;		// temporary int value
	u_int8_t	hun		= 0;
	u_int8_t	ten		= 0;
	u_int8_t	one		= 0;
	char		dbg_msg[80];
	PC 					= address;	// start program at this address

	while(emulatorRunning){
		if(exitRequest.wait_for(std::chrono::microseconds(1)) == std::future_status::ready){
			break;
		}
		usleep((unsigned int)sleep_time);
		I = htons(*(u_int16_t*)(ram+PC));		// read next instruction
		old_pc=PC;								// copy of current PC for disassembler
		PC+=2;									// increment program counter
		emit UpdateI(I);
		emit UpdatePC(PC);

		switch((I & MSK_OP_CODE) >> 12){
			case 0:	if(OC_CALL == I){
//						thread_active = false;
						sprintf(dbg_msg, "$%03X:   SYS, addr (not implemented -> HALT)", old_pc);
						p_trace_msg(dbg_msg);
					} else if(OC_DSP_CLR == I){
						mDsp->clear();
						sprintf(dbg_msg, "$%03X:   CLS             (I=%04X:)", old_pc, I);
						p_trace_msg(dbg_msg);
					} else if(OC_RET == I){
						PC = Stack[++SP];
						sprintf(dbg_msg, "$%03X:   RET             (I=%04X: PC=$%03X, SP=$%03X)",old_pc, I, PC, SP);
						p_trace_msg(dbg_msg);
						emit UpdatePC(PC);
						emit UpdateSP(SP);
						emit UpdateStack(Stack);
					}
					break;
			case 1:	PC = (I & MSK_ADDR);		// JMP to address
					sprintf(dbg_msg, "$%03X:   JMP $%03X        (I=%04X:)", old_pc, PC, I);
					p_trace_msg(dbg_msg);
					emit UpdatePC(PC);
					break;
			case 2:	Stack[SP--] = PC;			// save return address
					PC = (I & MSK_ADDR);		// JSR
					sprintf(dbg_msg, "$%03X:   CALL $%03X       (I=%04X:)", old_pc, PC, I);
					p_trace_msg(dbg_msg);
					emit UpdatePC(PC);
					emit UpdateSP(SP);
					emit UpdateStack(Stack);
					break;
			case 3:	reg_x	= (I & MSK_REG_X) >> 8;
					k		= (I & MSK_CONST);
					if(V[reg_x] == k){
						PC += 2;
						emit UpdatePC(PC);
					}
					sprintf(dbg_msg, "$%03X:   SE V%X #$%02X      (I=%04X: V%X=$%02X)", old_pc, reg_x, k, I, reg_x, V[reg_x]);
					p_trace_msg(dbg_msg);
					break;
			case 4:	reg_x	= (I & MSK_REG_X) >> 8;
					k		= (I & MSK_CONST);
					if(V[reg_x] != k){
						PC += 2;
						emit UpdatePC(PC);
					}
					sprintf(dbg_msg, "$%03X:   SNE V%X, #$%02X    (I=%04X: V%X=$%02X)", old_pc, reg_x, k, I, reg_x, V[reg_x]);
					p_trace_msg(dbg_msg);
					break;
			case 5:	reg_x	= (I & MSK_REG_X) >> 8;
					reg_y	= (I & MSK_REG_Y) >> 4;
					if(V[reg_x] == V[reg_y]){
						PC += 2;
						emit UpdatePC(PC);
					}
					sprintf(dbg_msg, "$%03X:   SE V%X, V%X       (I=%04X V%X=$%02X, V%X=$%02X)", old_pc, reg_x, reg_y, I, reg_x, V[reg_x], reg_y, V[reg_y]);
					p_trace_msg(dbg_msg);
					break;
			case 6:	reg_x		= (I & MSK_REG_X) >> 8;
					k			= (I & MSK_CONST);
					V[reg_x]	= k;
					sprintf(dbg_msg, "$%03X:   LD V%X, #$%02X     (I=%04X:)", old_pc, reg_x, k, I);
					p_trace_msg(dbg_msg);
					emit UpdateV(V);
					break;
			case 7:	reg_x		= (I & MSK_REG_X) >> 8;
					k			= (I & MSK_CONST);
					V[reg_x]	+= k;
					sprintf(dbg_msg, "$%03X:   ADD V%X, #$%02X    (I=%04X:)", old_pc, reg_x, k, I);
					p_trace_msg(dbg_msg);
					emit UpdateV(V);
					break;
			case 8:	switch(I & 0x000f){
						case 0:	reg_x		= (I & MSK_REG_X) >> 8;
								reg_y		= (I & MSK_REG_Y) >> 4;
								sprintf(dbg_msg, "$%03X:   LD V%X, V%X       (I=%04X:)", old_pc, reg_x, reg_y, I);
								p_trace_msg(dbg_msg);
								V[reg_x]	= V[reg_y];
								emit UpdateV(V);
								break;
						case 1:	reg_x		= (I & MSK_REG_X) >> 8;
								reg_y		= (I & MSK_REG_Y) >> 4;
								sprintf(dbg_msg, "$%03X:   OR V%X, V%X       (I=%04X:)", old_pc, reg_x, reg_y, I);
								p_trace_msg(dbg_msg);
								V[reg_x]	|= V[reg_y];
								emit UpdateV(V);
								break;
						case 2:	reg_x		= (I & MSK_REG_X) >> 8;
								reg_y		= (I & MSK_REG_Y) >> 4;
								sprintf(dbg_msg, "$%03X:   AND V%X, V%X      (I=%04X:)", old_pc, reg_x, reg_y, I);
								p_trace_msg(dbg_msg);
								V[reg_x]	&= V[reg_y];
								emit UpdateV(V);
								break;
						case 3:	reg_x		= (I & MSK_REG_X) >> 8;
								reg_y		= (I & MSK_REG_Y) >> 4;
								sprintf(dbg_msg, "$%03X:   XOR V%X, V%X      (I=%04X:)", old_pc, reg_x, reg_y, I);
								p_trace_msg(dbg_msg);
								V[reg_x]	^= V[reg_y];
								emit UpdateV(V);
								break;
						case 4:	reg_x		= (I & MSK_REG_X) >> 8;
								reg_y		= (I & MSK_REG_Y) >> 4;
								i_val		= V[reg_x] + V[reg_y];
								if(i_val > 255){		// set carry
									V[0xf]	= 1;
								} else {
									V[0xf]	= 0;
								}
								sprintf(dbg_msg, "$%03X:   ADC V%X, V%X      (I=%04X: VF=%02X)", old_pc, reg_x, reg_y, I, V[0xf]);
								p_trace_msg(dbg_msg);
								V[reg_x] = (u_int8_t)(i_val & 0x00ff);
								emit UpdateV(V);
								break;
						case 5:	reg_x		= (I & MSK_REG_X) >> 8;
								reg_y		= (I & MSK_REG_Y) >> 4;
								if(V[reg_x] > V[reg_y]){		// set carry
									V[0xf]	= 1;
								} else {
									V[0xf]	= 0;
								}
								sprintf(dbg_msg, "$%03X:   SBC V%X, V%X      (I=%04X: VF=%02X)", old_pc, reg_x, reg_y, I, V[0xf]);
								p_trace_msg(dbg_msg);
								V[reg_x]	= V[reg_x] - V[reg_y];
								emit UpdateV(V);
								break;
						case 6:	reg_x		= (I & MSK_REG_X) >> 8;
								reg_y		= (I & MSK_REG_Y) >> 4;
								V[0xf]		= (V[reg_x] & 0x01);
								sprintf(dbg_msg, "$%03X:   SHR V%X{, V%X}    (I=%04X: VF=%02X)", old_pc, reg_x, reg_y, I, V[0xf]);
								p_trace_msg(dbg_msg);
								V[reg_x]	= V[reg_x] >> 1;
								emit UpdateV(V);
								break;
						case 7:	reg_x		= (I & MSK_REG_X) >> 8;
								reg_y		= (I & MSK_REG_Y) >> 4;
								if(V[reg_y] > V[reg_x]){		// set carry
									V[0xf]	= 1;
								} else {
									V[0xf]	= 0;
								}
								sprintf(dbg_msg, "$%03X:   SUBN V%X, V%X     (I=%04X: VF=%02X)", old_pc, reg_x, reg_y, I, V[0xf]);
								p_trace_msg(dbg_msg);
								V[reg_x]	= V[reg_y] - V[reg_x];
								emit UpdateV(V);
								break;
						case 0x0e:	reg_x		= (I & MSK_REG_X) >> 8;
									reg_y		= (I & MSK_REG_Y) >> 4;
									V[0xf]		= (V[reg_x] & 0x80)? 1:0;
									sprintf(dbg_msg, "$%03X:   SHL V%X{, V%X}  (I=%04X: V%X=$%02X, VF=%02X)", old_pc, reg_x, reg_y, I, reg_x, V[reg_x], V[0xf]);
									p_trace_msg(dbg_msg);
									V[reg_x]	= V[reg_x] << 1;
									emit UpdateV(V);
									break;
					}
					break;
			case 9:	reg_x	= (I & MSK_REG_X) >> 8;
					reg_y	= (I & MSK_REG_Y) >> 4;
					if(V[reg_x] != V[reg_y]){
						PC += 2;
						emit UpdatePC(PC);
					}
					sprintf(dbg_msg, "$%03X:   SNE V%X, V%X    (I=%04X: V%X=$%02X, V%X=%02X)", old_pc, reg_x, reg_y, I, reg_x, V[reg_x], reg_y, V[reg_y]);
					p_trace_msg(dbg_msg);
					break;
			case 0xa:	M = (I & MSK_ADDR);		// Load new address
						sprintf(dbg_msg, "$%03X:   LD M, #$%03X     (I=%04X:)", old_pc, M, I);
						p_trace_msg(dbg_msg);
						emit UpdateM(M);
						break;
			case 0xb:	PC = (I & MSK_ADDR) + V[0];
						sprintf(dbg_msg, "$%03X:   JMP V0, #$%03X    (I=%04X: PC(new)=%03X, V0=%02X)", old_pc, M, I, PC, V[0]);
						p_trace_msg(dbg_msg);
						emit UpdatePC(PC);
						break;
			case 0xc:	reg_x		= (I & MSK_REG_X) >> 8;
						k			= (I & MSK_CONST);
						vx			= V[reg_x];
						V[reg_x]	= (rand()%256) & k;
						sprintf(dbg_msg, "$%03X:   RND V%X, #$%02X    (I=%04X: V%X(old)=$%02X,V%X(new)=$%02X)", old_pc, reg_x, k, I, reg_x, vx, reg_x, V[reg_x]);
						p_trace_msg(dbg_msg);
						emit UpdateV(V);
						break;
			case 0xd:	reg_x	= (I & MSK_REG_X) >> 8;
						reg_y	= (I & MSK_REG_Y) >> 4;
						i_val	= (I & 0x000f);
						sprintf(dbg_msg, "$%03X:   DRW V%X, V%X, #$%X (I=%04X: M=%03X, V%X=$%02X, V%X=%02X)", old_pc, reg_x, reg_y, i_val, I, M, reg_x, V[reg_x], reg_y, V[reg_y]);
						p_trace_msg(dbg_msg);
						V[0xf]=mDsp->draw_sprite(V[reg_x], V[reg_y], i_val, ram+M);
						if(V[0xf] == 1){
							log_msg("-D- Draw -> Collision");
						}
						emit UpdateV(V);
						break;
			case 0xe: switch(I & 0x00ff){
							case 0x9e:	reg_x		= (I & MSK_REG_X) >> 8;
										if(keyboard->ReadKey(Chip8Keyboard::RD_MODE_NON_BLOCKING) == V[reg_x]){
											PC += 2;
											emit UpdatePC(PC);
										}
										sprintf(dbg_msg, "$%03X:   SKP V%X          (I=%04X: PC=$%03X, V%X=$%02X)", old_pc, reg_x, I, PC, reg_x, V[reg_x]);
										p_trace_msg(dbg_msg);
										break;
							case 0xa1:	reg_x		= (I & MSK_REG_X) >> 8;
										if(keyboard->ReadKey(Chip8Keyboard::RD_MODE_NON_BLOCKING) != V[reg_x]){
											PC += 2;
											emit UpdatePC(PC);
										}
										sprintf(dbg_msg, "$%03X:   SKNP V%X         (I=%04X: PC=$%03X, V%X=$%02X)", old_pc, reg_x, I, PC, reg_x, V[reg_x]);
										p_trace_msg(dbg_msg);
										break;
							default:	sprintf(dbg_msg,"-E- Unknown OP-code %04X",I);
										p_trace_msg(dbg_msg);
										break;
						}
						break;
			case 0xf:	switch(I & 0x00ff){
							case 0x07:	reg_x		= (I & MSK_REG_X) >> 8;
										V[reg_x]	= TD;
										sprintf(dbg_msg, "$%03X:   LD V%X, TD       (I=%04X: V%X=$%02X)", old_pc, reg_x, I, reg_x, V[reg_x]);
										p_trace_msg(dbg_msg);
										emit UpdateV(V);
										break;
							case 0x0a:	reg_x		= (I & MSK_REG_X) >> 8;
										V[reg_x]	= keyboard->ReadKey(Chip8Keyboard::RD_MODE_BLOCKING);
										sprintf(dbg_msg, "$%03X:   LD V%X, K        (I=%04X: V%X=$%02X)", old_pc, reg_x, I, reg_x, V[reg_x]);
										p_trace_msg(dbg_msg);
										emit UpdateV(V);
										break;
							case 0x15:	reg_x		= (I & MSK_REG_X) >> 8;
										TD			= V[reg_x];
										sprintf(dbg_msg, "$%03X:   LD TD, V%X       (I=%04X: V%X=$%02X)", old_pc, reg_x, I, reg_x, V[reg_x]);
										p_trace_msg(dbg_msg);
										emit UpdateTd(TD);
										break;
							case 0x18:	reg_x		= (I & MSK_REG_X) >> 8;
										TS			= V[reg_x];
										sprintf(dbg_msg, "$%03X:   LD TS, V%X       (I=%04X: V%X=$%02X)", old_pc, reg_x, I, reg_x, V[reg_x]);
										emit UpdateTs(TS);
										p_trace_msg(dbg_msg);
										break;
							case 0x1e:	reg_x		= (I & MSK_REG_X) >> 8;
										vx			= M;						// mis-use vx to store old M
										M 			+= V[reg_x];
										sprintf(dbg_msg, "$%03X:   ADD M, V%X       (I=%04X: M(old)=$%03X, V%X=$%02X)", old_pc, reg_x, I, vx, reg_x, V[reg_x]);
										p_trace_msg(dbg_msg);
										emit UpdateM(M);
										break;
							case 0x29:	reg_x		= (I & MSK_REG_X) >> 8;
										M			= MAP_CHAR_TBL_START + (V[reg_x] * CHAR_SIZE);
										sprintf(dbg_msg, "$%03X:   LD F, V%X        (I=%04X: M=$%03X, V%X=$%02X)", old_pc, reg_x, I, M, reg_x, V[reg_x]);
										p_trace_msg(dbg_msg);
										emit UpdateM(M);
										break;
							case 0x33:	reg_x		= (I & MSK_REG_X) >> 8;			// store BCD representation of VX at memory loc. M
										hun			= V[reg_x]/100;
										ten			= (V[reg_x]-(hun*100))/10;
										one			= V[reg_x] % 10;
										ram[M]		= hun;
										ram[M+1]	= ten;
										ram[M+2]	= one;
										sprintf(dbg_msg, "$%03X:   STO B, V%X       (I=%04X: M=$%03X, V%X=$%03i)", old_pc, reg_x, I, M, reg_x, V[reg_x]);
										p_trace_msg(dbg_msg);
										emit UpdateM(M);
										break;
							case 0x55:	reg_x		= (I & MSK_REG_X) >> 8;
										for(int offset = 0; offset <= reg_x; ++offset){
											ram[M+offset] = V[offset];
										}
										sprintf(dbg_msg, "$%03X:   STO [M], V%X     (I=%04X: M=$%03X)", old_pc, reg_x, I, M);
										p_trace_msg(dbg_msg);
										emit UpdateM(M);
										break;
							case 0x65:	reg_x		= (I & MSK_REG_X) >> 8;
										for(int offset = 0; offset <= reg_x; ++offset){
											V[offset] = ram[M+offset];
										}
										sprintf(dbg_msg, "$%03X:   RSTO [M], V%X    (I=%04X: M=$%03X)", old_pc, reg_x, I, M);
										p_trace_msg(dbg_msg);
										emit UpdateV(V);
										break;
							case 0x75:
									break;
						}
						break;
			default:	std::cerr << "-W- Invalid OP-Code <" << I << ">" << std::endl;
						break;
		}

		if(MODE_STEP == execMode){
			std::unique_lock<std::mutex> mlock(mtx);
			cond_var.wait(mlock, [this]{return do_step;});
			do_step=false;
		}
	}
	trace_msg("-T- CHIP8::run() end");
	std::cerr << "CHIP8::run(): stopped running" << std::endl;
	emulatorRunning=false;

	return 0;
}
//-----------------------------------------------------------------------------

/**

*/
std::string CHIP8::parse_op_code(u_int16_t op_code, u_int16_t pc)
{
	std::string	command;
	char		buf[100];
	u_int8_t 	reg_x	= 0;		// index of register X
	u_int8_t 	reg_y	= 0;		// index of register Y
	u_int8_t 	k		= 0;		// 8-bit constant
	u_int16_t	i_val	= 0;

	switch((op_code & MSK_OP_CODE) >> 12){
	case 0:	if(OC_CALL == op_code){
				sprintf(buf, "$%03X:   SYS, addr (not implemented -> HALT)", pc);
				command = buf;
			} else if(OC_DSP_CLR == op_code){
				sprintf(buf, "$%03X:   CLS", pc);
				command	= buf;
			} else if(OC_RET == op_code){
				sprintf(buf, "$%03X:   RET",pc);
				command	= buf;
			}
			break;
	case 1:	PC = (op_code & MSK_ADDR);		// JMP to address
			sprintf(buf, "$%03X:   JMP $%03X", pc, PC);
			command = buf;
			break;
	case 2: PC = (op_code & MSK_ADDR);		// JSR
			sprintf(buf, "$%03X:   CALL $%03X", pc, PC);
			command = buf;
			break;
	case 3:	reg_x	= (op_code & MSK_REG_X) >> 8;
			k		= (op_code & MSK_CONST);
			sprintf(buf, "$%03X:   SE V%X #$%02X", pc, reg_x, k);
			command = buf;
			break;
	case 4:	reg_x	= (op_code & MSK_REG_X) >> 8;
			k		= (op_code & MSK_CONST);
			sprintf(buf, "$%03X:   SNE V%X, #$%02X", pc, reg_x, k);
			command = buf;
			break;
	case 5:	reg_x	= (op_code & MSK_REG_X) >> 8;
			reg_y	= (op_code & MSK_REG_Y) >> 4;
			sprintf(buf, "$%03X:   SE V%X, V%X", pc, reg_x, reg_y);
			command = buf;
			break;
	case 6:	reg_x		= (op_code & MSK_REG_X) >> 8;
			k			= (op_code & MSK_CONST);
			sprintf(buf, "$%03X:   LD V%X, #$%02X", pc, reg_x, k);
			command = buf;
			break;
	case 7:	reg_x		= (op_code & MSK_REG_X) >> 8;
			k			= (op_code & MSK_CONST);
			sprintf(buf, "$%03X:   ADD V%X, #$%02X", pc, reg_x, k);
			command = buf;
			break;
	case 8:	switch(op_code & 0x000f){
				case 0:	reg_x		= (op_code & MSK_REG_X) >> 8;
						reg_y		= (op_code & MSK_REG_Y) >> 4;
						sprintf(buf, "$%03X:   LD V%X, V%X", pc, reg_x, reg_y);
						command = buf;
						V[reg_x]	= V[reg_y];
						break;
				case 1:	reg_x		= (op_code & MSK_REG_X) >> 8;
						reg_y		= (op_code & MSK_REG_Y) >> 4;
						sprintf(buf, "$%03X:   OR V%X, V%X", pc, reg_x, reg_y);
						command = buf;
						break;
				case 2:	reg_x		= (op_code & MSK_REG_X) >> 8;
						reg_y		= (op_code & MSK_REG_Y) >> 4;
						sprintf(buf, "$%03X:   AND V%X, V%X", pc, reg_x, reg_y);
						command = buf;
						break;
				case 3:	reg_x		= (op_code & MSK_REG_X) >> 8;
						reg_y		= (op_code & MSK_REG_Y) >> 4;
						sprintf(buf, "$%03X:   XOR V%X, V%X", pc, reg_x, reg_y);
						command = buf;
						break;
				case 4:	reg_x		= (op_code & MSK_REG_X) >> 8;
						reg_y		= (op_code & MSK_REG_Y) >> 4;
						sprintf(buf, "$%03X:   ADC V%X, V%X", pc, reg_x, reg_y);
						command = buf;
						break;
				case 5:	reg_x		= (op_code & MSK_REG_X) >> 8;
						reg_y		= (op_code & MSK_REG_Y) >> 4;
						sprintf(buf, "$%03X:   SBC V%X, V%X", pc, reg_x, reg_y);
						command = buf;
						break;
				case 6:	reg_x		= (op_code & MSK_REG_X) >> 8;
						reg_y		= (op_code & MSK_REG_Y) >> 4;
						sprintf(buf, "$%03X:   SHR V%X{, V%X}", pc, reg_x, reg_y);
						command = buf;
						break;
				case 7:	reg_x		= (op_code & MSK_REG_X) >> 8;
						reg_y		= (op_code & MSK_REG_Y) >> 4;
						sprintf(buf, "$%03X:   SUBN V%X, V%X", pc, reg_x, reg_y);
						command = buf;
						break;
				case 0x0e:	reg_x		= (op_code & MSK_REG_X) >> 8;
							reg_y		= (op_code & MSK_REG_Y) >> 4;
							sprintf(buf, "$%03X:   SHL V%X{, V%X}", pc, reg_x, reg_y);
							command = buf;
							break;
			}
			break;
	case 9:		reg_x	= (op_code & MSK_REG_X) >> 8;
				reg_y	= (op_code & MSK_REG_Y) >> 4;
				sprintf(buf, "$%03X:   SNE V%X, V%X", pc, reg_x, reg_y);
				command = buf;
				break;
	case 0xa:	M = (op_code & MSK_ADDR);		// Load new address
				sprintf(buf, "$%03X:   LD M, #$%03X", pc, M);
				command = buf;
				break;
	case 0xb:	PC = (op_code & MSK_ADDR) + V[0];
				sprintf(buf, "$%03X:   JMP V0, #$%03X", pc, M);
				command = buf;
				break;
	case 0xc:	reg_x	= (op_code & MSK_REG_X) >> 8;
				k		= (op_code & MSK_CONST);
				sprintf(buf, "$%03X:   RND V%X, #$%02X", pc, reg_x, k);
				command = buf;
				break;
	case 0xd:	reg_x	= (op_code & MSK_REG_X) >> 8;
				reg_y	= (op_code & MSK_REG_Y) >> 4;
				i_val	= (op_code & 0x000f);
				sprintf(buf, "$%03X:   DRW V%X, V%X, #$%X", pc, reg_x, reg_y, i_val);
				command = buf;
				break;
	case 0xe: switch(op_code & 0x00ff){
			case 0x9e:	reg_x		= (op_code & MSK_REG_X) >> 8;
						sprintf(buf, "$%03X:   SKP V%X", pc, reg_x);
						command = buf;
						break;
			case 0xa1:	reg_x		= (op_code & MSK_REG_X) >> 8;
						sprintf(buf, "$%03X:   SKNP V%X", pc, reg_x);
						command = buf;
						break;
			default:	sprintf(buf,"-E- Unknown OP-code %04X", op_code);
						command = buf;
						break;
			}
				break;
	case 0xf:	switch(op_code & 0x00ff){
			case 0x07:	reg_x		= (op_code & MSK_REG_X) >> 8;
						sprintf(buf, "$%03X:   LD V%X, TD", pc, reg_x);
						command = buf;
						break;
			case 0x0a:	reg_x		= (op_code & MSK_REG_X) >> 8;
						sprintf(buf, "$%03X:   LD V%X, K", pc, reg_x);
						command = buf;
						break;
			case 0x15:	reg_x		= (op_code & MSK_REG_X) >> 8;
					sprintf(buf, "$%03X:   LD TD, V%X", pc, reg_x);
					command = buf;
					break;
			case 0x18:	reg_x		= (op_code & MSK_REG_X) >> 8;
					sprintf(buf, "$%03X:   LD TS, V%X", pc, reg_x);
					command = buf;
					break;
			case 0x1e:	reg_x		= (op_code & MSK_REG_X) >> 8;
					sprintf(buf, "$%03X:   ADD M, V%X", pc, reg_x);
					command = buf;
					break;
			case 0x29:	reg_x		= (op_code & MSK_REG_X) >> 8;
					sprintf(buf, "$%03X:   LD F, V%X", pc, reg_x);
					command = buf;
					break;
			case 0x33:	reg_x		= (op_code & MSK_REG_X) >> 8;			// store BCD representation of VX at memory loc. M
					sprintf(buf, "$%03X:   STO B, V%X", pc, reg_x);
					command = buf;
					break;
			case 0x55:	reg_x		= (op_code & MSK_REG_X) >> 8;
					sprintf(buf, "$%03X:   STO [M], V%X", pc, reg_x);
					command = buf;
					break;
			case 0x65:	reg_x		= (op_code & MSK_REG_X) >> 8;
					sprintf(buf, "$%03X:   RSTO [M], V%X", pc, reg_x);
					command = buf;
					break;
			case 0x75:
					break;
			}
	}
	return command;
}
//-----------------------------------------------------------------------------

/**

*/
std::vector<std::string> CHIP8::disassemble(void)
{
	std::vector<std::string>	prog;
	u_int16_t					base = PC;

	for(unsigned int index = 0; index < program_size; index+=2){
		prog.push_back(parse_op_code(htons(*(u_int16_t*)(ram+base+index)), base+index));
	}
	return prog;
}
//-----------------------------------------------------------------------------

/**


*/
void CHIP8::mode(EMULATION_MODE mode)
{
	emuMode = mode;

	if(MODE_CLASSIC == emuMode){
		dsp_height	= WIN_ROWS;
		dsp_width	= WIN_COLS;
	} else {
		dsp_height	= WIN_S_ROWS;
		dsp_width	= WIN_S_COLS;
	}
	mDsp->mode(emuMode);			// notify the display about our new display-resolution
}
//-----------------------------------------------------------------------------


/* Pupblic slots */

/**
	This method interrupts the  execution of the current program but keeps the
	emulation thread running.

	We are setting execMode to MODE_STEP. The emulator main loop ( \ref run()) checks
	for this value and halts execution untill we send a notification via a
	condition variable in methods \ref step() or \ref continue().
*/
void CHIP8::Stop(void)
{
	execMode = MODE_STEP;
	do_step  = true;
}
//-----------------------------------------------------------------------------

/**
	This method single-steps through the program. This method only has an effect
	if the programm was Stopped prior to calling this method.
*/
void CHIP8::Step(void)
{
	if(MODE_STEP == execMode){						// only do the stepping if we are already stopped
		std::lock_guard<std::mutex> guard(mtx);		// acquire mutex for condition var.
		do_step = true;								// do one step
		cond_var.notify_one();						// send the notification
	}
}
//-----------------------------------------------------------------------------

/**
	This method resumes execution after the program was interrupted with \ref Stop().
*/
void CHIP8::Continue(void)
{
	if(MODE_STEP == execMode){						// only resume execution if we were stopped
		execMode = MODE_RUNNING;					// resume execution
		std::lock_guard<std::mutex> guard(mtx);		// acquire mutex for condition var ...
		cond_var.notify_one();						// ... and tell the thread to stop waiting for the condition var
		do_step		= true;							//
	}
}
//-----------------------------------------------------------------------------


/**
	This method starts the actual emulation in a separate thread.

	\param	[in]	address	Starting address of the program.
*/
void CHIP8::Run(u_int16_t address)
{
	emulatorRunning = true;
	execMode = MODE_RUNNING;
	start_timers();																		// start the CHIP8 60 Hz timers
	runMethod =  new std::thread(&CHIP8::run, this, address, std::move(exitThread));	// "this" needs to be passed as a dummy-object!
}
//-----------------------------------------------------------------------------

/**

*/
void CHIP8::Reset(void)
{
	emuTimer->stop();
	if(runMethod){
		exitSignal.set_value();			// tell run() thread to stop

		if(MODE_STEP == execMode){		// make sure we are not stuck in a wait for a condition var
			Continue();
		}

		runMethod->join();
	}
	runMethod = nullptr;
	emulatorRunning = false;
	mDsp->clear();
	memset(ram, 0, VM_SIZE);			// clear memory
}
//-----------------------------------------------------------------------------
