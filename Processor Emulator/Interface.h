#pragma once

//#define wxNO_EXCEPTIONS

// Interface library headers.
#include<wx/wx.h>
#include<wx/rawbmp.h>
#include<wx/sizer.h>
#include<wx/timer.h>
#include<wx/spinctrl.h>
#include<wx/stc/stc.h>

// Processor emulator header.
#include"MOS6502.h"

// Additional headers.
#include<chrono>



// Allows for drawing bitmaps within the app.
class wxImagePanel : public wxPanel
{
private:

	// Stored bitmap pointer.
	wxBitmap* image;

public:

	// Default constructor.
	wxImagePanel(wxFrame* parent, wxBitmap* img);

	// Draws the bitmap contents onto the panel (can be called via Refresh()/Update()).
	void paintEvent(wxPaintEvent & evt);

	// Empty function, prevents background redraw, limiting flicker on rendering.
	void eraseEvent(wxEraseEvent& event);

	DECLARE_EVENT_TABLE()
};



// Forward declaration.
class ProcFrame;

// Stores interface.
class ProcApp : public wxApp
{
private:
	ProcFrame *MainWin;

public:
	virtual bool OnInit();
};



// Forward declaration of the pointer, removes circular dependency issues.
class ProcMos6502;

// Represents the main window of the program.
class ProcFrame : public wxFrame, public wxThreadHelper
{
public:
	// Default constructor.
	ProcFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

private:

	// Processor emulator instance.
	ProcMos6502* Processor;

	// ID numbers of the event methods.
	enum commandIdNums
	{
		ID_InputRegisterAChanged = wxID_HIGHEST + 1,
		ID_InputRegisterXChanged,
		ID_InputRegisterYChanged,
		ID_InputStackPointerChanged,
		ID_InputProgramCounterChanged,
		
		ID_InputFlagNegativeChanged,
		ID_InputFlagOverflowChanged,
		ID_InputFlagBlankChanged,
		ID_InputFlagBreakChanged,
		ID_InputFlagDecimalChanged,
		ID_InputFlagInterruptChanged,
		ID_InputFlagZeroChanged,
		ID_InputFlagCarryChanged,
		
		ID_InputVectorNMIChanged,
		ID_InputVectorResetChanged,
		ID_InputVectorBRKChanged,

		ID_InputClockFrequencyChanged,
		
		ID_LoadButtonClicked,
		ID_PlayPauseButtonClicked,
		ID_SingleOperationButtonClicked,
		ID_ResetButtonClicked,
		ID_IrqButtonClicked,
		ID_NmiButtonClicked
	};



	// Size of each cell of the memory dump view.
	#define MEMDUMP_PX_PER_CELL 3

	// Memory dump bitmap display panel.
	wxImagePanel* MemDumpPanel;
	wxBitmap* MemDumpBitmap;
	wxNativePixelData* MemDumpInterface;

	// Updates value of given cell on the memory display bitmap (unused due to multithreading issues).
	void updateMemDumpValue(unsigned short address, unsigned char value);
	
	// Draws PC and addressed cell traces to the memory display.
	void drawMemDumpTrace();

	// Clears PC and addressed cell traces from the memory display (unused due to multithreading issues).
	void clearMemDumpTrace();

	// Redraws memory dump bitmap from processor emulator memory.
	void resetMemDump();

	// Memory image load button. Opens a dialog allowing the user to select a file to load into the processor memory.
	wxButton* LoadButton;
	void onLoadButtonClick(wxCommandEvent& event);



	// Size of each cell of the 6502asm.com compatible screen.
	#define ASM_SCREEN_PX_PER_CELL 8

	// 6502asm screen bitmap display panel.
	wxImagePanel* AsmScreenPanel;
	wxBitmap* AsmScreenBitmap;
	wxNativePixelData* AsmScreenInterface;

	// Fills appropriate cell with color according to the value if the provided address is within the range of the screen (unused due to multithreading issues).
	void updateAsmScreenValue(unsigned short address, unsigned char value);

	// Reloads the screen contents from memory.
	void resetAsmScreen();

	// Table translating values to AsmScreen colors.
	static const wxColour AsmScreenColor[16];

	// Command history display.
	wxStyledTextCtrl* CommandHistory;



	// Numeric inputs containing (and allowing for the changing of) the current values of various registers.
	wxSpinCtrl* InputRegisterA;
	void onInputRegisterAChange(wxSpinEvent& event);
	wxSpinCtrl* InputRegisterX;
	void onInputRegisterXChange(wxSpinEvent& event);
	wxSpinCtrl* InputRegisterY;
	void onInputRegisterYChange(wxSpinEvent& event);
	wxSpinCtrl* InputStackPointer;
	void onInputStackPointerChange(wxSpinEvent& event);
	wxSpinCtrl* InputProgramCounter;
	void onInputProgramCounterChange(wxSpinEvent& event);

	wxSpinCtrl* InputFlagNegative;
	void onInputFlagNegativeChange(wxSpinEvent& event);
	wxSpinCtrl* InputFlagOverflow;
	void onInputFlagOverflowChange(wxSpinEvent& event);
	wxSpinCtrl* InputFlagBlank;
	void onInputFlagBlankChange(wxSpinEvent& event);
	wxSpinCtrl* InputFlagBreak;
	void onInputFlagBreakChange(wxSpinEvent& event);
	wxSpinCtrl* InputFlagDecimal;
	void onInputFlagDecimalChange(wxSpinEvent& event);
	wxSpinCtrl* InputFlagInterrupt;
	void onInputFlagInterruptChange(wxSpinEvent& event);
	wxSpinCtrl* InputFlagZero;
	void onInputFlagZeroChange(wxSpinEvent& event);
	wxSpinCtrl* InputFlagCarry;
	void onInputFlagCarryChange(wxSpinEvent& event);

	wxSpinCtrl* InputVectorNMI;
	void onInputVectorNMIChange(wxSpinEvent& event);
	wxSpinCtrl* InputVectorReset;
	void onInputVectorResetChange(wxSpinEvent& event);
	wxSpinCtrl* InputVectorBRK;
	void onInputVectorBRKChange(wxSpinEvent& event);



	// Minimum duration expected between interface updates (results in, at most, approximately 12 frames per second refresh rate).
	const std::chrono::duration<double, std::milli> MinimumFrameDelay = std::chrono::milliseconds(1000) / 12;

	// Minimum, maximum and starting clock frequencies (in Hz).
	#define EMULATOR_CLOCK_FREQUENCY_MIN 1
	#define EMULATOR_CLOCK_FREQUENCY_MAX 1000000000ull
	#define EMULATOR_CLOCK_FREQUENCY_START 1

	// Minimum change between clock frequencies (in Hz).
	#define EMULATOR_CLOCK_FREQUENCY_DELTA 1

	// Additional values allowing for adjusting the units on the displayed clock frequency.
	#define EMULATOR_CLOCK_DISPLAY_MULTIPLIER 1000
	//#define EMULATOR_CLOCK_DISPLAY_UNIT "KHz"

	// Minimum duration expected between processor cycles, determined by the clock frequency.
	std::chrono::duration<double, std::ratio<1, EMULATOR_CLOCK_FREQUENCY_MAX>> EmulatorCycleDuration;

	// Numeric input allowing for control over the emulated processor's clock frequency.
	wxSpinCtrlDouble* InputClockFrequency;
	void onClockFrequencyChanged(wxSpinDoubleEvent& event);



	// Emulator run flag, prevents emulator cycles if set to false.
	bool RunEmulator = false;

	// Helper methods, perform required interface cleanup when toggling between running and stopping the emulator cycle clock.
	void emulatorPlay();
	void emulatorPause();

	// Play/Pause button. Toggles auto-execution of emulation cycles on/off.
	wxButton* PlayPauseButton;
	void onPlayPauseButtonClick(wxCommandEvent& event);

	// Single operation button. Performs a single processor operation, skipping any 'empty' timing cycles.	
	wxButton* SingleOperationButton;
	void onSingleOperationButtonClick(wxCommandEvent& event);



	// Reset button. Performs the basic reset operation (does not reload processor memory).
	wxButton* ResetButton;
	void onResetButtonClick(wxCommandEvent& event);

	// IRQ button. Sends the IRQ signal to the emulated processor.
	wxButton* IrqButton;
	void onIrqButtonClick(wxCommandEvent& event);

	// NMI button. Sends the NMI signal to the emulated processor.
	wxButton* NmiButton;
	void onNmiButtonClick(wxCommandEvent& event);



	// Command history queue, accumulates strings representing processor commands between interface updates.
	std::string CommandAccumulator = "";

	// Time over which the displayed data should fade.
	#define MEMORY_DISPLAY_TRACE_FADE_TIME 256

	// Rolling buffer storing previously accessed cell data.
	struct {
		unsigned short ProgramCounter = 0;
		unsigned char ArgumentCount = 255; //signifies uninitialized cell
		int TargetA = -1;
		int TargetB = -1;
	} PreviousMemoryCells[MEMORY_DISPLAY_TRACE_FADE_TIME];

	// Pointer to the current buffered cell data block.
	unsigned int PreviousMemoryPointer = 0;



	// Processing lock flag, prevents processor cycles and interface updates from occuring at the same time.
	bool LockFlag = false;

	// Emulator update method, performs a single processor cycle and stores all information relevant to interface updates. Skips timing cycles if value of false is provided as a parameter. Returns true if an operation was performed, false on timing cycle.
	bool updateEmulator(bool timingOn = true);

	// Interface update method, writes the current processor parameters into the appropriate interface elements and performs a refresh.
	void updateInterface();


	
	// Thread initializer method, starts the emulator thread.
	void startEmulatorThread();

	// Thread method, performs interface and emulation updates based on the defined clock frequency.
	wxThread::ExitCode Entry();

	// Update event method, required for multithreaded interface updates.
	void threadUpdateMethod(wxThreadEvent& WXUNUSED(event));

	
		
	// Performs any required cleanup on program exit.
	void onExit(wxCloseEvent& WXUNUSED(event));



	DECLARE_EVENT_TABLE();
};