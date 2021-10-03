#include "Interface.h"
#include <fstream>
#include <thread>

IMPLEMENT_APP(ProcApp) // Sets ProcApp as the main app class.

bool ProcApp::OnInit()
{
	// Create an instance of our frame, or window
	MainWin = new ProcFrame(_T("MOS 6502 Emulator"), wxPoint(1, 1), wxSize(300, 200));
	MainWin->Show(TRUE);
	SetTopWindow(MainWin);

	return TRUE;
}



ProcFrame::ProcFrame(const wxString & title, const wxPoint & pos, const wxSize & size) : wxFrame(0, -1, title, pos, size)
{
	//initialize processor emulator
	Processor = new ProcMos6502();

	//initialize bitmaps and bitmap interfaces
	MemDumpBitmap = new wxBitmap(256 * MEMDUMP_PX_PER_CELL, 256 * MEMDUMP_PX_PER_CELL, 24);
	AsmScreenBitmap = new wxBitmap(32 * ASM_SCREEN_PX_PER_CELL, 32 * ASM_SCREEN_PX_PER_CELL, 24);
	MemDumpInterface = new wxNativePixelData(*MemDumpBitmap);
	AsmScreenInterface = new wxNativePixelData(*AsmScreenBitmap);



	//initialize tracked interface elements
	LoadButton = new wxButton(this, ID_LoadButtonClicked, _T("Load Memory State"), wxDefaultPosition, wxDefaultSize, 0);
	SingleOperationButton = new wxButton(this, ID_SingleOperationButtonClicked, _T("Run Single Operation"), wxDefaultPosition, wxDefaultSize, 0);
	PlayPauseButton = new wxButton(this, ID_PlayPauseButtonClicked, _T("Play"), wxDefaultPosition, wxDefaultSize, 0);
	ResetButton = new wxButton(this, ID_ResetButtonClicked, _T("Reset Signal"), wxDefaultPosition, wxDefaultSize, 0);
	IrqButton = new wxButton(this, ID_IrqButtonClicked, _T("IRQ Signal"), wxDefaultPosition, wxDefaultSize, 0);
	NmiButton = new wxButton(this, ID_NmiButtonClicked, _T("NMI Signal"), wxDefaultPosition, wxDefaultSize, 0);

	MemDumpPanel = new wxImagePanel(this, MemDumpBitmap);
	AsmScreenPanel = new wxImagePanel(this, AsmScreenBitmap);
	
	CommandHistory = new wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 500), wxTE_MULTILINE | wxTE_READONLY); //new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 500), wxTE_MULTILINE | wxTE_READONLY);
	CommandHistory->SetUseHorizontalScrollBar(false);
	
	InputRegisterA = new wxSpinCtrl(this, ID_InputRegisterAChanged, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxSP_WRAP | wxSP_ARROW_KEYS, 0, 0xFF);
	InputRegisterX = new wxSpinCtrl(this, ID_InputRegisterXChanged, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxSP_WRAP | wxSP_ARROW_KEYS, 0, 0xFF);
	InputRegisterY = new wxSpinCtrl(this, ID_InputRegisterYChanged, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxSP_WRAP | wxSP_ARROW_KEYS, 0, 0xFF);
	InputStackPointer = new wxSpinCtrl(this, ID_InputStackPointerChanged, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxSP_WRAP | wxSP_ARROW_KEYS, 0, 0xFF);
	InputProgramCounter = new wxSpinCtrl(this, ID_InputProgramCounterChanged, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxSP_WRAP | wxSP_ARROW_KEYS, 0, 0xFFFF);
	InputRegisterA->SetBase(16);
	InputRegisterX->SetBase(16);
	InputRegisterY->SetBase(16);
	InputStackPointer->SetBase(16);
	InputProgramCounter->SetBase(16);
	
	InputFlagNegative = new wxSpinCtrl(this, ID_InputFlagNegativeChanged, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxSP_WRAP | wxSP_ARROW_KEYS, 0, 1);
	InputFlagOverflow = new wxSpinCtrl(this, ID_InputFlagOverflowChanged, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxSP_WRAP | wxSP_ARROW_KEYS, 0, 1);
	InputFlagBlank = new wxSpinCtrl(this, ID_InputFlagBlankChanged, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxSP_WRAP | wxSP_ARROW_KEYS, 0, 1);
	InputFlagBreak = new wxSpinCtrl(this, ID_InputFlagBreakChanged, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxSP_WRAP | wxSP_ARROW_KEYS, 0, 1);
	InputFlagDecimal = new wxSpinCtrl(this, ID_InputFlagDecimalChanged, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxSP_WRAP | wxSP_ARROW_KEYS, 0, 1);
	InputFlagInterrupt = new wxSpinCtrl(this, ID_InputFlagInterruptChanged, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxSP_WRAP | wxSP_ARROW_KEYS, 0, 1);
	InputFlagZero = new wxSpinCtrl(this, ID_InputFlagZeroChanged, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxSP_WRAP | wxSP_ARROW_KEYS, 0, 1);
	InputFlagCarry = new wxSpinCtrl(this, ID_InputFlagCarryChanged, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxSP_WRAP | wxSP_ARROW_KEYS, 0, 1);
	
	InputVectorNMI = new wxSpinCtrl(this, ID_InputVectorNMIChanged, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxSP_WRAP | wxSP_ARROW_KEYS, 0, 0xFFFF);
	InputVectorReset = new wxSpinCtrl(this, ID_InputVectorResetChanged, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxSP_WRAP | wxSP_ARROW_KEYS, 0, 0xFFFF);
	InputVectorBRK = new wxSpinCtrl(this, ID_InputVectorBRKChanged, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxSP_WRAP | wxSP_ARROW_KEYS, 0, 0xFFFF);
	InputVectorNMI->SetBase(16);
	InputVectorReset->SetBase(16);
	InputVectorBRK->SetBase(16);
	
	InputClockFrequency = new wxSpinCtrlDouble(this, ID_InputClockFrequencyChanged, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxSP_ARROW_KEYS,
		(double)(EMULATOR_CLOCK_FREQUENCY_MIN) / (double)(EMULATOR_CLOCK_DISPLAY_MULTIPLIER),
		(double)(EMULATOR_CLOCK_FREQUENCY_MAX) / (double)(EMULATOR_CLOCK_DISPLAY_MULTIPLIER),
		(double)(EMULATOR_CLOCK_FREQUENCY_START) / (double)(EMULATOR_CLOCK_DISPLAY_MULTIPLIER),
		(double)(EMULATOR_CLOCK_FREQUENCY_DELTA) / (double)(EMULATOR_CLOCK_DISPLAY_MULTIPLIER));

	#define CONTROL_WIDTH_A 65
	#define CONTROL_WIDTH_B 35

	InputRegisterA->SetMaxSize(wxSize(CONTROL_WIDTH_A, 200));
	InputRegisterX->SetMaxSize(wxSize(CONTROL_WIDTH_A, 200));
	InputRegisterY->SetMaxSize(wxSize(CONTROL_WIDTH_A, 200));
	InputStackPointer->SetMaxSize(wxSize(CONTROL_WIDTH_A, 200));
	InputProgramCounter->SetMaxSize(wxSize(CONTROL_WIDTH_A, 200));
	InputVectorNMI->SetMaxSize(wxSize(CONTROL_WIDTH_A, 200));
	InputVectorReset->SetMaxSize(wxSize(CONTROL_WIDTH_A, 200));
	InputVectorBRK->SetMaxSize(wxSize(CONTROL_WIDTH_A, 200));

	InputFlagNegative->SetMaxSize(wxSize(CONTROL_WIDTH_B, 200));
	InputFlagOverflow->SetMaxSize(wxSize(CONTROL_WIDTH_B, 200));
	InputFlagBlank->SetMaxSize(wxSize(CONTROL_WIDTH_B, 200));
	InputFlagBreak->SetMaxSize(wxSize(CONTROL_WIDTH_B, 200));
	InputFlagDecimal->SetMaxSize(wxSize(CONTROL_WIDTH_B, 200));
	InputFlagInterrupt->SetMaxSize(wxSize(CONTROL_WIDTH_B, 200));
	InputFlagZero->SetMaxSize(wxSize(CONTROL_WIDTH_B, 200));
	InputFlagCarry->SetMaxSize(wxSize(CONTROL_WIDTH_B, 200));
	InputFlagBlank->Disable();
	InputFlagBreak->Disable();



	//build interface
	wxSizer* SizerRegisterA = new wxBoxSizer(wxHORIZONTAL);
	SizerRegisterA->Add(new wxStaticText(this, -1, "Accumulator", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL), 1, wxEXPAND | wxALL, 0);
	SizerRegisterA->Add(InputRegisterA, 0, wxALL, 0);

	wxSizer* SizerRegisterX = new wxBoxSizer(wxHORIZONTAL);
	SizerRegisterX->Add(new wxStaticText(this, -1, "X", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL), 1, wxEXPAND | wxALL, 0);
	SizerRegisterX->Add(InputRegisterX, 0, wxALL, 0);

	wxSizer* SizerRegisterY = new wxBoxSizer(wxHORIZONTAL);
	SizerRegisterY->Add(new wxStaticText(this, -1, "Y", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL), 1, wxEXPAND | wxALL, 0);
	SizerRegisterY->Add(InputRegisterY, 0, wxALL, 0);

	wxSizer* SizerStackPointer = new wxBoxSizer(wxHORIZONTAL);
	SizerStackPointer->Add(new wxStaticText(this, -1, "Stack Pointer", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL), 1, wxEXPAND | wxALL, 0);
	SizerStackPointer->Add(InputStackPointer, 0, wxALL, 0);

	wxSizer* SizerProgramCounter = new wxBoxSizer(wxHORIZONTAL);
	SizerProgramCounter->Add(new wxStaticText(this, -1, "Program Counter", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL), 1, wxEXPAND | wxALL, 0);
	SizerProgramCounter->Add(InputProgramCounter, 0, wxALL, 0);

	#define FLAG_LABEL_HEIGHT 15

	wxSizer* SizerFlagLabels = new wxBoxSizer(wxHORIZONTAL);
	SizerFlagLabels->Add(new wxStaticText(this, -1, "S", wxDefaultPosition, wxSize(CONTROL_WIDTH_B, FLAG_LABEL_HEIGHT), wxALIGN_CENTRE_HORIZONTAL | wxST_NO_AUTORESIZE), 1, wxEXPAND | wxALL, 0);
	SizerFlagLabels->Add(new wxStaticText(this, -1, "V", wxDefaultPosition, wxSize(CONTROL_WIDTH_B, FLAG_LABEL_HEIGHT), wxALIGN_CENTRE_HORIZONTAL | wxST_NO_AUTORESIZE), 1, wxEXPAND | wxALL, 0);
	SizerFlagLabels->Add(new wxStaticText(this, -1, "-", wxDefaultPosition, wxSize(CONTROL_WIDTH_B, FLAG_LABEL_HEIGHT), wxALIGN_CENTRE_HORIZONTAL | wxST_NO_AUTORESIZE), 1, wxEXPAND | wxALL, 0);
	SizerFlagLabels->Add(new wxStaticText(this, -1, "B", wxDefaultPosition, wxSize(CONTROL_WIDTH_B, FLAG_LABEL_HEIGHT), wxALIGN_CENTRE_HORIZONTAL | wxST_NO_AUTORESIZE), 1, wxEXPAND | wxALL, 0);
	SizerFlagLabels->Add(new wxStaticText(this, -1, "D", wxDefaultPosition, wxSize(CONTROL_WIDTH_B, FLAG_LABEL_HEIGHT), wxALIGN_CENTRE_HORIZONTAL | wxST_NO_AUTORESIZE), 1, wxEXPAND | wxALL, 0);
	SizerFlagLabels->Add(new wxStaticText(this, -1, "I", wxDefaultPosition, wxSize(CONTROL_WIDTH_B, FLAG_LABEL_HEIGHT), wxALIGN_CENTRE_HORIZONTAL | wxST_NO_AUTORESIZE), 1, wxEXPAND | wxALL, 0);
	SizerFlagLabels->Add(new wxStaticText(this, -1, "Z", wxDefaultPosition, wxSize(CONTROL_WIDTH_B, FLAG_LABEL_HEIGHT), wxALIGN_CENTRE_HORIZONTAL | wxST_NO_AUTORESIZE), 1, wxEXPAND | wxALL, 0);
	SizerFlagLabels->Add(new wxStaticText(this, -1, "C", wxDefaultPosition, wxSize(CONTROL_WIDTH_B, FLAG_LABEL_HEIGHT), wxALIGN_CENTRE_HORIZONTAL | wxST_NO_AUTORESIZE), 1, wxEXPAND | wxALL, 0);

	wxSizer* SizerFlagValues = new wxBoxSizer(wxHORIZONTAL);
	SizerFlagValues->Add(InputFlagNegative, 1, wxEXPAND | wxALL, 0);
	SizerFlagValues->Add(InputFlagOverflow, 1, wxEXPAND | wxALL, 0);
	SizerFlagValues->Add(InputFlagBlank, 1, wxEXPAND | wxALL, 0);
	SizerFlagValues->Add(InputFlagBreak, 1, wxEXPAND | wxALL, 0);
	SizerFlagValues->Add(InputFlagDecimal, 1, wxEXPAND | wxALL, 0);
	SizerFlagValues->Add(InputFlagInterrupt, 1, wxEXPAND | wxALL, 0);
	SizerFlagValues->Add(InputFlagZero, 1, wxEXPAND | wxALL, 0);
	SizerFlagValues->Add(InputFlagCarry, 1, wxEXPAND | wxALL, 0);

	wxSizer* SizerVectorNMI = new wxBoxSizer(wxHORIZONTAL);
	SizerVectorNMI->Add(new wxStaticText(this, -1, "NMI Vector", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL), 1, wxEXPAND | wxALL, 0);
	SizerVectorNMI->Add(InputVectorNMI, 0, wxALL, 0);

	wxSizer* SizerVectorReset = new wxBoxSizer(wxHORIZONTAL);
	SizerVectorReset->Add(new wxStaticText(this, -1, "Reset Vector", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL), 1, wxEXPAND | wxALL, 0);
	SizerVectorReset->Add(InputVectorReset, 0, wxALL, 0);

	wxSizer* SizerVectorBRK = new wxBoxSizer(wxHORIZONTAL);
	SizerVectorBRK->Add(new wxStaticText(this, -1, "IRQ/BRK Vector", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL), 1, wxEXPAND | wxALL, 0);
	SizerVectorBRK->Add(InputVectorBRK, 0, wxALL, 0);

	wxSizer* SizerPlayPause = new wxBoxSizer(wxHORIZONTAL);
	SizerPlayPause->Add(PlayPauseButton, 1, wxEXPAND | wxALL, 0);
	SizerPlayPause->Add(SingleOperationButton, 1, wxEXPAND | wxALL, 0);



	wxStaticBoxSizer* BoxSizerMemDisplay = new wxStaticBoxSizer(wxVERTICAL, this, "Memory Display");
	BoxSizerMemDisplay->Add(MemDumpPanel, 0, wxALL, 5);
	BoxSizerMemDisplay->Add(LoadButton, 1, wxEXPAND | wxALL, 0);



	wxStaticBoxSizer* BoxSizerScreen = new wxStaticBoxSizer(wxVERTICAL, this, "Screen");
	BoxSizerScreen->Add(AsmScreenPanel, 0, wxALL, 5);

	wxStaticBoxSizer* BoxSizerCommandHistory = new wxStaticBoxSizer(wxVERTICAL, this, "Command History");
	BoxSizerCommandHistory->Add(CommandHistory, 1, wxEXPAND | wxALL, 5);



	wxStaticBoxSizer* BoxSizerRegisters = new wxStaticBoxSizer(wxVERTICAL, this, "Registers");
	BoxSizerRegisters->Add(SizerRegisterA, 0, wxEXPAND | wxALL, 5);
	BoxSizerRegisters->Add(SizerRegisterX, 0, wxEXPAND | wxALL, 5);
	BoxSizerRegisters->Add(SizerRegisterY, 0, wxEXPAND | wxALL, 5);
	BoxSizerRegisters->Add(SizerStackPointer, 0, wxEXPAND | wxALL, 5);
	BoxSizerRegisters->Add(SizerProgramCounter, 0, wxEXPAND | wxALL, 5);

	wxStaticBoxSizer* BoxSizerFlags = new wxStaticBoxSizer(wxVERTICAL, this, "Flags");
	BoxSizerFlags->Add(SizerFlagLabels, 0, wxEXPAND | wxALL, 5);
	BoxSizerFlags->Add(SizerFlagValues, 0, wxEXPAND | wxALL, 5);

	wxStaticBoxSizer* BoxSizerVectors = new wxStaticBoxSizer(wxVERTICAL, this, "Memory Vectors");
	BoxSizerVectors->Add(SizerVectorNMI, 0, wxEXPAND | wxALL, 5);
	BoxSizerVectors->Add(SizerVectorReset, 0, wxEXPAND | wxALL, 5);
	BoxSizerVectors->Add(SizerVectorBRK, 0, wxEXPAND | wxALL, 5);

	wxStaticBoxSizer* BoxSizerClockFrequency = new wxStaticBoxSizer(wxVERTICAL, this, "Clock Frequency (KHz)");
	BoxSizerClockFrequency->Add(InputClockFrequency, 1, wxEXPAND | wxALL, 5);
	//BoxSizerClockFrequency->Add(new wxStaticText(this, -1, EMULATOR_CLOCK_DISPLAY_UNIT, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL), 0, wxALL, 0);
	//BoxSizerClockFrequency->Add(PlayButton, 1, wxEXPAND | wxALL, 5);
	//BoxSizerClockFrequency->Add(PauseButton, 1, wxEXPAND | wxALL, 5);
	BoxSizerClockFrequency->Add(SizerPlayPause, 0, wxEXPAND | wxALL, 5);

	wxStaticBoxSizer* BoxSizerSignalControls = new wxStaticBoxSizer(wxVERTICAL, this, "Signal Controls");
	BoxSizerSignalControls->Add(ResetButton, 0, wxEXPAND | wxALL, 5);
	BoxSizerSignalControls->Add(IrqButton, 0, wxEXPAND | wxALL, 5);
	BoxSizerSignalControls->Add(NmiButton, 0, wxEXPAND | wxALL, 5);



	wxSizer* SizerLeft = new wxBoxSizer(wxVERTICAL);
	SizerLeft->Add(BoxSizerMemDisplay, 0, wxEXPAND | wxALL, 5);
	
	wxSizer* SizerMiddle = new wxBoxSizer(wxVERTICAL);
	SizerMiddle->Add(BoxSizerScreen, 0, wxEXPAND | wxALL, 5);
	SizerMiddle->Add(BoxSizerCommandHistory, 1, wxEXPAND | wxALL, 5);

	wxSizer* SizerRight = new wxBoxSizer(wxVERTICAL);
	SizerRight->Add(BoxSizerRegisters, 0, wxEXPAND | wxALL, 5);
	SizerRight->Add(BoxSizerFlags, 0, wxEXPAND | wxALL, 5);
	SizerRight->Add(BoxSizerVectors, 0, wxEXPAND | wxALL, 5);
	SizerRight->Add(BoxSizerClockFrequency, 0, wxEXPAND | wxALL, 5);
	SizerRight->Add(BoxSizerSignalControls, 0, wxEXPAND | wxALL, 5);
	
	wxSizer* SizerMain = new wxBoxSizer(wxHORIZONTAL);
	SizerMain->Add(SizerLeft, 0, wxALL, 0);
	SizerMain->Add(SizerMiddle, 0, wxALL, 0);
	SizerMain->Add(SizerRight, 0, wxALL, 0);

	//set sizer
	SetSizerAndFit(SizerMain);

	//invalidate the cell data buffer
	for (int i = 0; i < MEMORY_DISPLAY_TRACE_FADE_TIME; i++)
		PreviousMemoryCells[i].ArgumentCount = 255;

	//get initial processor cycle duration
	EmulatorCycleDuration = std::chrono::duration<double, std::ratio<1, 1>>(1 / EMULATOR_CLOCK_FREQUENCY_START);

	//refresh interface
	updateInterface();

	//start emulator timing thread
	startEmulatorThread();
}

void ProcFrame::updateMemDumpValue(unsigned short address, unsigned char value)
{
	//initialize iterators
	wxNativePixelData::Iterator p(*MemDumpInterface);
	wxNativePixelData::Iterator rowStart;

	//set cell location
	p.MoveTo(*MemDumpInterface, (address & 0xFF) * MEMDUMP_PX_PER_CELL, ((address >> 8) & 0xFF) * MEMDUMP_PX_PER_CELL);

	//render changed value
	for (int i = 0; i < MEMDUMP_PX_PER_CELL; i++)
	{
		//store row position
		rowStart = p;

		//iterate over row
		for (int j = 0; j < MEMDUMP_PX_PER_CELL; j++, p++)
		{
			//set color
			p.Blue() = value;
		}

		//increment row
		p = rowStart;
		p.OffsetY(*MemDumpInterface, 1);
	}
}

void ProcFrame::drawMemDumpTrace()
{
	//initialize iterators
	wxNativePixelData::Iterator p(*MemDumpInterface);
	wxNativePixelData::Iterator rowStart;

	//initialize variable
	unsigned char proportion;

	//render cells
	for (int i = 0; i < MEMORY_DISPLAY_TRACE_FADE_TIME; i++)
	{
		//check if the cell is valid
		if (PreviousMemoryCells[i].ArgumentCount <= 2)
		{
			//calculate proportion value
			proportion = (unsigned char)(256.0f * ((float)((i - PreviousMemoryPointer - 1) % MEMORY_DISPLAY_TRACE_FADE_TIME) / (float)MEMORY_DISPLAY_TRACE_FADE_TIME));

			//check if the operation's A target was a memory cell
			if (PreviousMemoryCells[i].TargetA >= 0 && PreviousMemoryCells[i].TargetA <= 0xFFFF)
			{
				//set cell location
				p.MoveTo(*MemDumpInterface, (PreviousMemoryCells[i].TargetA % 0x100) * MEMDUMP_PX_PER_CELL, (PreviousMemoryCells[i].TargetA >> 8) * MEMDUMP_PX_PER_CELL);

				//render changed value
				for (int i = 0; i < MEMDUMP_PX_PER_CELL; i++)
				{
					//store row position
					rowStart = p;

					//iterate over row
					for (int j = 0; j < MEMDUMP_PX_PER_CELL; j++, p++)
					{
						//set color
						p.Green() = proportion;
					}

					//increment row
					p = rowStart;
					p.OffsetY(*MemDumpInterface, 1);
				}
			}

			//check if the operation's B target was a memory cell
			if (PreviousMemoryCells[i].TargetB >= 0 && PreviousMemoryCells[i].TargetB <= 0xFFFF)
			{
				//set cell location
				p.MoveTo(*MemDumpInterface, (PreviousMemoryCells[i].TargetB % 0x100) * MEMDUMP_PX_PER_CELL, (PreviousMemoryCells[i].TargetB >> 8) * MEMDUMP_PX_PER_CELL);

				//render changed value
				for (int i = 0; i < MEMDUMP_PX_PER_CELL; i++)
				{
					//store row position
					rowStart = p;

					//iterate over row
					for (int j = 0; j < MEMDUMP_PX_PER_CELL; j++, p++)
					{
						//set color
						p.Green() = proportion;
					}

					//increment row
					p = rowStart;
					p.OffsetY(*MemDumpInterface, 1);
				}
			}

			//set cell location
			p.MoveTo(*MemDumpInterface, (PreviousMemoryCells[i].ProgramCounter % 0x100) * MEMDUMP_PX_PER_CELL, (PreviousMemoryCells[i].ProgramCounter >> 8) * MEMDUMP_PX_PER_CELL);

			//render program counter position
			for (int i = 0; i < MEMDUMP_PX_PER_CELL; i++)
			{
				//store row position
				rowStart = p;

				//iterate over row
				for (int j = 0; j < MEMDUMP_PX_PER_CELL; j++, p++)
				{
					//set color
					p.Red() = proportion;
				}

				//increment row
				p = rowStart;
				p.OffsetY(*MemDumpInterface, 1);
			}

			//TODO:

			//render argument positions
		}
	}
}

void ProcFrame::clearMemDumpTrace()
{
	//initialize iterators
	wxNativePixelData::Iterator p(*MemDumpInterface);
	wxNativePixelData::Iterator rowStart;

	//iterate over pixel rows
	for (int y = 0; y < MemDumpBitmap->GetHeight(); y++)
	{
		//store initial position
		rowStart = p;

		//iterate over pixel columns
		for (int x = 0; x < MemDumpBitmap->GetWidth(); x++)
		{
			//set color
			p.Red() = 0;
			p.Green() = 0;

			p++;
		}

		//retrieve initial position
		p = rowStart;

		//increment pixel row
		p.OffsetY(*MemDumpInterface, 1);
	}

	/*
	//clear cells
	for (int i = 0; i < MEMORY_DISPLAY_TRACE_FADE_TIME; i++)
	{
		//check if the cell is valid
		if (PreviousMemoryCells[i].ArgumentCount <= 2)
		{
			//check if the operation's A target was a memory cell
			if (PreviousMemoryCells[i].TargetA >= 0 && PreviousMemoryCells[i].TargetA <= 0xFFFF)
			{
				//set cell location
				p.MoveTo(*MemDumpInterface, (PreviousMemoryCells[i].TargetA % 0x100) * MEMDUMP_PX_PER_CELL, (PreviousMemoryCells[i].TargetA >> 8) * MEMDUMP_PX_PER_CELL);

				//clear changed value
				for (int i = 0; i < MEMDUMP_PX_PER_CELL; i++)
				{
					//store row position
					rowStart = p;

					//iterate over row
					for (int j = 0; j < MEMDUMP_PX_PER_CELL; j++, p++)
					{
						//set color
						p.Green() = 0;
					}

					//increment row
					p = rowStart;
					p.OffsetY(*MemDumpInterface, 1);
				}
			}

			//check if the operation's B target was a memory cell
			if (PreviousMemoryCells[i].TargetB >= 0 && PreviousMemoryCells[i].TargetB <= 0xFFFF)
			{
				//set cell location
				p.MoveTo(*MemDumpInterface, (PreviousMemoryCells[i].TargetB % 0x100) * MEMDUMP_PX_PER_CELL, (PreviousMemoryCells[i].TargetB >> 8) * MEMDUMP_PX_PER_CELL);

				//clear changed value
				for (int i = 0; i < MEMDUMP_PX_PER_CELL; i++)
				{
					//store row position
					rowStart = p;

					//iterate over row
					for (int j = 0; j < MEMDUMP_PX_PER_CELL; j++, p++)
					{
						//set color
						p.Green() = 0;
					}

					//increment row
					p = rowStart;
					p.OffsetY(*MemDumpInterface, 1);
				}
			}

			//set cell location
			p.MoveTo(*MemDumpInterface, (PreviousMemoryCells[i].ProgramCounter % 0x100) * MEMDUMP_PX_PER_CELL, (PreviousMemoryCells[i].ProgramCounter >> 8) * MEMDUMP_PX_PER_CELL);

			//clear program counter position
			for (int i = 0; i < MEMDUMP_PX_PER_CELL; i++)
			{
				//store row position
				rowStart = p;

				//iterate over row
				for (int j = 0; j < MEMDUMP_PX_PER_CELL; j++, p++)
				{
					//set color
					p.Red() = 0;
				}

				//increment row
				p = rowStart;
				p.OffsetY(*MemDumpInterface, 1);
			}

			//TODO:

			//clear argument positions
		}
	}
	*/
}

void ProcFrame::resetMemDump()
{
	//initialize rendering iterators
	wxNativePixelData::Iterator p(*MemDumpInterface);
	wxNativePixelData::Iterator q(*MemDumpInterface);
	wxNativePixelData::Iterator r(*MemDumpInterface);
	wxNativePixelData::Iterator s(*MemDumpInterface);
	
	//initialize value variable
	unsigned char val;

	//iterate over memory rows
	for (int y = 0; y < 256; y++)
	{
		//store initial position
		q = p;

		//iterate over memory columns
		for (int x = 0; x < 256; x++)
		{
			//store initial position
			r = p;

			//get memory cell value
			val = Processor->getMemoryCell((y << 8) | x);

			//iterate over cell rows
			for (int i = 0; i < MEMDUMP_PX_PER_CELL; i++)
			{
				//store row position
				s = p;

				//iterate over cell columns
				for (int j = 0; j < MEMDUMP_PX_PER_CELL; j++, p++)
				{
					//set color
					p.Blue() = val;
					p.Red() = 0;
					p.Green() = 0;
				}

				//increment row
				p = s;
				p.OffsetY(*MemDumpInterface, 1);
			}

			//restore position
			p = r;

			//increment row
			p.OffsetX(*MemDumpInterface, MEMDUMP_PX_PER_CELL);
		}

		//restore position
		p = q;

		//increment memory row
		p.OffsetY(*MemDumpInterface, MEMDUMP_PX_PER_CELL);
	}
}

#pragma optimize("",off)

void ProcFrame::onLoadButtonClick(wxCommandEvent & event)
{
	//open file dialog
	wxFileDialog openFileDialog(this, _("Open compiled memory file"), "", "", "Binary files (*.bin)|*.bin|Hex files (*.hex)|*.hex|All files|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	
	//return if file select cancelled
	if (openFileDialog.ShowModal() == wxID_CANCEL) return;

	//open file as filestream
	std::ifstream file(openFileDialog.GetPath().ToStdString(), std::ios::binary);

	//stop emulator
	emulatorPause();
	
	//wait until LockFlag is freed up
	while (LockFlag) {}

	//lock out emulation updates
	LockFlag = true;

	//load file into emulated processor memory using filetype-approriate method
	if (openFileDialog.GetFilterIndex() == 1)
	{
		Processor->loadFromHexStream(file);
	}
	else
	{
		Processor->loadFromBinStream(file);
	}

	//reset processor
	Processor->reset();

	//invalidate the cell data buffer
	for (int i = 0; i < MEMORY_DISPLAY_TRACE_FADE_TIME; i++)
		PreviousMemoryCells[i].ArgumentCount = 255;

	//reset command list
	CommandAccumulator = "";
	CommandHistory->ClearAll();

	//reset memory dump
	resetMemDump();

	//reset screen
	resetAsmScreen();

	//re-enable emulation updates
	LockFlag = false;

	//update interface
	updateInterface();
}

#pragma optimize("",on)

void ProcFrame::onSingleOperationButtonClick(wxCommandEvent & event)
{
	//perform operation
	updateEmulator(false);

	//update interface
	updateInterface();
}

void ProcFrame::emulatorPlay()
{
	//change play/pause button label
	PlayPauseButton->SetLabel("Pause");

	//disable single operation button
	SingleOperationButton->Disable();

	//toggle flag value
	RunEmulator = true;
}

void ProcFrame::emulatorPause()
{
	//change play/pause button label
	PlayPauseButton->SetLabel("Play");

	//enable single operation button
	SingleOperationButton->Enable();

	//toggle flag value
	RunEmulator = false;

	//update interface
	updateInterface();
}

void ProcFrame::onPlayPauseButtonClick(wxCommandEvent & event)
{
	//perform appropriate action
	if (RunEmulator) emulatorPause();
	else emulatorPlay();
}

#pragma optimize("",off)

void ProcFrame::onResetButtonClick(wxCommandEvent & event)
{
	//wait until LockFlag is freed up
	while (LockFlag) {}

	//lock out emulation updates
	LockFlag = true;

	//reset processor
	Processor->reset();

	//invalidate the cell data buffer
	for (int i = 0; i < MEMORY_DISPLAY_TRACE_FADE_TIME; i++)
		PreviousMemoryCells[i].ArgumentCount = 255;

	//reset command list
	CommandAccumulator = "";
	CommandHistory->ClearAll();

	//reset memory dump
	resetMemDump();

	//reset screen
	resetAsmScreen();

	//re-enable emulation updates
	LockFlag = false;

	//update interface
	updateInterface();
}

#pragma optimize("",on)

#pragma optimize("",off)

void ProcFrame::onIrqButtonClick(wxCommandEvent & event)
{
	//wait for cycle end
	while (LockFlag) {}

	//lock emulator updates
	LockFlag = true;

	//send IRQ signal
	Processor->triggerIRQ();

	//unlock emulator updates
	LockFlag = false;
}

#pragma optimize("",on)

#pragma optimize("",off)

void ProcFrame::onNmiButtonClick(wxCommandEvent & event)
{
	//wait for cycle end
	while (LockFlag) {}

	//lock emulator updates
	LockFlag = true;

	//send NMI signal
	Processor->startNMI();
	Processor->endNMI();

	//unlock emulator updates
	LockFlag = false;
}

#pragma optimize("",on)

void ProcFrame::updateAsmScreenValue(unsigned short address, unsigned char value)
{
	//check if the operation wrote to the AsmScreen address space
	if ((address >= 0x0200) && (address <= 0x05FF))
	{
		//initialize iterators
		wxNativePixelData::Iterator asmI(*AsmScreenInterface);
		wxNativePixelData::Iterator asmRowStart;

		//get address value
		unsigned short add = (address - 0x200) % 1024;

		//set cell location
		asmI.MoveTo(*AsmScreenInterface, (add & 0b11111) * ASM_SCREEN_PX_PER_CELL, (add >> 5) * ASM_SCREEN_PX_PER_CELL);

		//get color
		wxColour bep = AsmScreenColor[value & 0xF];

		//render changed cell
		for (int i = 0; i < ASM_SCREEN_PX_PER_CELL; i++)
		{
			//store row position
			asmRowStart = asmI;

			//iterate over row
			for (int j = 0; j < ASM_SCREEN_PX_PER_CELL; j++, asmI++)
			{
				//set color
				asmI.Red() = bep.Red();
				asmI.Green() = bep.Green();
				asmI.Blue() = bep.Blue();
			}

			//increment row
			asmI = asmRowStart;
			asmI.OffsetY(*AsmScreenInterface, 1);
		}

		//refresh screen
		//AsmScreenPanel->Refresh();
	}
}

void ProcFrame::resetAsmScreen()
{
	//NOTE: removed due to issues with multithreading.
	/*
	//initialize iterators
	wxNativePixelData::Iterator p = wxNativePixelData::Iterator(*AsmScreenInterface);
	wxNativePixelData::Iterator q = wxNativePixelData::Iterator(*AsmScreenInterface);

	//iterate over pixel rows
	for (int y = 0; y < AsmScreenBitmap->GetHeight(); y++)
	{
		//store initial position
		q = p;

		//iterate over pixel columns
		for (int x = 0; x < AsmScreenBitmap->GetWidth(); x++)
		{
			//set color
			p.Red() = 0;
			p.Green() = 0;
			p.Blue() = 0;

			p++;
		}

		//retrieve initial position
		p = q;

		//increment pixel row
		p.OffsetY(*AsmScreenInterface, 1);
	}
	*/

	//initialize iterators
	wxNativePixelData::Iterator asmI(*AsmScreenInterface);
	wxNativePixelData::Iterator asmRowStart;

	//loop over screen address values
	for (int add = 0x0000; add < 0x0400; add++)
	{
		//set cell location
		asmI.MoveTo(*AsmScreenInterface, (add & 0b11111) * ASM_SCREEN_PX_PER_CELL, (add >> 5) * ASM_SCREEN_PX_PER_CELL);

		//get cell color
		wxColour bep = AsmScreenColor[Processor->getMemoryCell(add + 0x0200) & 0xF];

		//render changed cell
		for (int i = 0; i < ASM_SCREEN_PX_PER_CELL; i++)
		{
			//store row position
			asmRowStart = asmI;

			//iterate over row
			for (int j = 0; j < ASM_SCREEN_PX_PER_CELL; j++, asmI++)
			{
				//set color
				asmI.Red() = bep.Red();
				asmI.Green() = bep.Green();
				asmI.Blue() = bep.Blue();
			}

			//increment row
			asmI = asmRowStart;
			asmI.OffsetY(*AsmScreenInterface, 1);
		}
	}
}

void ProcFrame::onInputRegisterAChange(wxSpinEvent& event)
{
	//change register value
	Processor->setRegisterA(event.GetValue());
}

void ProcFrame::onInputRegisterXChange(wxSpinEvent& event)
{
	//change register value
	Processor->setRegisterX(event.GetValue());
}

void ProcFrame::onInputRegisterYChange(wxSpinEvent& event)
{
	//change register value
	Processor->setRegisterY(event.GetValue());
}

void ProcFrame::onInputStackPointerChange(wxSpinEvent & event)
{
	//change stack pointer value
	Processor->setStackPointer(event.GetValue());
}

void ProcFrame::onInputProgramCounterChange(wxSpinEvent& event)
{
	//change program counter value
	Processor->setProgramCounter(event.GetValue());
}

void ProcFrame::onInputVectorNMIChange(wxSpinEvent& event)
{
	//change vector value
	Processor->setVectorNMI(event.GetValue());
}

void ProcFrame::onInputVectorResetChange(wxSpinEvent& event)
{
	//change vector value
	Processor->setVectorReset(event.GetValue());
}

void ProcFrame::onInputVectorBRKChange(wxSpinEvent& event)
{
	//change vector value
	Processor->setVectorBRK(event.GetValue());
}

void ProcFrame::onClockFrequencyChanged(wxSpinDoubleEvent & event)
{
	//get new clock frequency
	double freq = event.GetValue();

	//convert from displayed unit back into Hz
	freq *= EMULATOR_CLOCK_DISPLAY_MULTIPLIER;

	//convert into emulator cycle duration
	EmulatorCycleDuration = std::chrono::duration<double, std::ratio<1, 1>>(1 / freq);
}

void ProcFrame::onInputFlagNegativeChange(wxSpinEvent& event)
{
	//change flag value
	Processor->setFlagSign(event.GetValue());
}

void ProcFrame::onInputFlagOverflowChange(wxSpinEvent& event)
{
	//change flag value
	Processor->setFlagOverflow(event.GetValue());
}

void ProcFrame::onInputFlagBlankChange(wxSpinEvent& event)
{
	//change flag value
	Processor->setFlagBlank(event.GetValue());
}

void ProcFrame::onInputFlagBreakChange(wxSpinEvent& event)
{
	//change flag value
	Processor->setFlagBreak(event.GetValue());
}

void ProcFrame::onInputFlagDecimalChange(wxSpinEvent& event)
{
	//change flag value
	Processor->setFlagDecimal(event.GetValue());
}

void ProcFrame::onInputFlagInterruptChange(wxSpinEvent& event)
{
	//change flag value
	Processor->setFlagInterrupt(event.GetValue());
}

void ProcFrame::onInputFlagZeroChange(wxSpinEvent& event)
{
	//change flag value
	Processor->setFlagZero(event.GetValue());
}

void ProcFrame::onInputFlagCarryChange(wxSpinEvent& event)
{
	//change flag value
	Processor->setFlagCarry(event.GetValue());
}

#pragma optimize("",off)

void ProcFrame::updateInterface()
{
	//wait until LockFlag is freed up
	while (LockFlag) {}

	//lock out emulation updates
	LockFlag = true;

	//NOTE: removed due to issues with multithreading.
	/*
	//clear previous PC trace and cell updates from the MemDump bitmap
	clearMemDumpTrace();
	*/

	//redraw memory contents from scratch (bypasses multithreading issues)
	resetMemDump();

	//redraw screen contents from scratch (bypasses multithreading issues)
	resetAsmScreen();

	//draw current PC trace and cell updates to the MemDump bitmap
	drawMemDumpTrace();



	//append accumulated command strings to history
	//CommandHistory->SetText((CommandHistory->GetText() + CommandAccumulator).substr(std::max(((int)(CommandHistory->GetLength() + CommandAccumulator.length()) - 1000000), 0)));
	CommandHistory->AppendText(CommandAccumulator);
	CommandHistory->ScrollToEnd();

	//clear string accumulator
	CommandAccumulator = "";

	//update parameter inputs
	InputRegisterA->SetValue(Processor->getRegisterA());
	InputRegisterX->SetValue(Processor->getRegisterX());
	InputRegisterY->SetValue(Processor->getRegisterY());
	InputStackPointer->SetValue(Processor->getStackPointer());
	InputProgramCounter->SetValue(Processor->getProgramCounter());

	InputFlagNegative->SetValue(Processor->getFlagSign());
	InputFlagOverflow->SetValue(Processor->getFlagOverflow());
	InputFlagBlank->SetValue(Processor->getFlagBlank());
	InputFlagBreak->SetValue(Processor->getFlagBreak());
	InputFlagDecimal->SetValue(Processor->getFlagDecimal());
	InputFlagInterrupt->SetValue(Processor->getFlagInterrupt());
	InputFlagZero->SetValue(Processor->getFlagZero());
	InputFlagCarry->SetValue(Processor->getFlagCarry());

	InputVectorNMI->SetValue(Processor->getVectorNMI());
	InputVectorReset->SetValue(Processor->getVectorReset());
	InputVectorBRK->SetValue(Processor->getVectorBRK());

	//unlock emulation updates
	LockFlag = false;

	//refresh GUI
	MemDumpPanel->Refresh();
	AsmScreenPanel->Refresh();

	CommandHistory->Refresh();
	
	InputRegisterA->Refresh();
	InputRegisterX->Refresh();
	InputRegisterY->Refresh();
	InputStackPointer->Refresh();
	InputProgramCounter->Refresh();

	InputFlagNegative->Refresh();
	InputFlagOverflow->Refresh();
	InputFlagBlank->Refresh();
	InputFlagBreak->Refresh();
	InputFlagDecimal->Refresh();
	InputFlagInterrupt->Refresh();
	InputFlagZero->Refresh();
	InputFlagCarry->Refresh();

	InputVectorNMI->Refresh();
	InputVectorReset->Refresh();
	InputVectorBRK->Refresh();
}

#pragma optimize("",on)

void ProcFrame::startEmulatorThread()
{
	//attempt to create emulator thread
	if (CreateThread(wxTHREAD_JOINABLE) != wxTHREAD_NO_ERROR)
		return;

	//attempt to run emulator thread
	if (GetThread()->Run() != wxTHREAD_NO_ERROR)
		return;
}

wxThread::ExitCode ProcFrame::Entry()
{
	//initialize clock
	std::chrono::steady_clock clock = std::chrono::steady_clock();

	//initialize timepoints
	std::chrono::time_point<std::chrono::steady_clock> cycleStart;
	std::chrono::time_point<std::chrono::steady_clock> cycleEnd = cycleStart;
	std::chrono::time_point<std::chrono::steady_clock> nextInterfaceUpdate;

	//initialize cycle counter
	unsigned int cycleCount = 1;

	//continue timing loops until thread is destroyed
	while (!GetThread()->TestDestroy())
	{
		//low clock frequency loop behavior, performs an interface update immediately after each emulator update
		while (RunEmulator && MinimumFrameDelay <= EmulatorCycleDuration && !GetThread()->TestDestroy())
		{
			//get current time
			cycleStart = clock.now();

			//reset cycle counter
			cycleCount = 1;

			//perform emulation updates until an operation is performed
			while (!updateEmulator()) cycleCount++;

			//generate incremented time
			cycleEnd = cycleStart + (std::chrono::duration_cast<std::chrono::steady_clock::duration>(EmulatorCycleDuration) * cycleCount);

			//wait until incremented time
			std::this_thread::sleep_until(cycleEnd);

			//perform interface update
			wxQueueEvent(this, new wxThreadEvent());
		}

		//initialize next interface update timepoint
		nextInterfaceUpdate = clock.now() + std::chrono::duration_cast<std::chrono::steady_clock::duration>(MinimumFrameDelay);

		//generate incremented time
		cycleEnd = cycleStart + std::chrono::duration_cast<std::chrono::steady_clock::duration>(EmulatorCycleDuration);

		//default loop behavior, performs emulator and interface updates semi-asynchronously
		while (RunEmulator && MinimumFrameDelay > EmulatorCycleDuration && !GetThread()->TestDestroy())
		{
			
			//perform emulator updates until an interface update timepoint is passed
			while (nextInterfaceUpdate >= cycleEnd)
			{
				//wait until incremented time
				std::this_thread::sleep_until(cycleEnd);

				//get current time
				cycleStart = clock.now();

				//reset cycle counter
				cycleCount = 1;

				//perform emulation updates until an operation is performed
				while (!updateEmulator()) cycleCount++;

				//generate incremented time
				cycleEnd = cycleStart + std::chrono::duration_cast<std::chrono::steady_clock::duration>(EmulatorCycleDuration);
			}

			//perform interface update
			wxQueueEvent(this, new wxThreadEvent());
			
			//queue next interface update
			nextInterfaceUpdate = clock.now() + std::chrono::duration_cast<std::chrono::steady_clock::duration>(MinimumFrameDelay);
		}
	}

	return (wxThread::ExitCode)0;
}

void ProcFrame::threadUpdateMethod(wxThreadEvent & WXUNUSED)
{
	//update interface
	updateInterface();
}

#pragma optimize("",off)

void ProcFrame::onExit(wxCloseEvent& WXUNUSED(event))
{
	//reset lock flag
	LockFlag = false;

	//stop emulator thread
	if (GetThread() && GetThread()->IsRunning())
		GetThread()->Delete();
	
	Destroy();
}

#pragma optimize("",on)

#pragma optimize("",off)

bool ProcFrame::updateEmulator(bool timingOn)
{
	//wait until LockFlag is freed up
	while (LockFlag) {}

	//lock out interface updates
	LockFlag = true;

	//perform processor cycle and check if an operation was performed
	if (Processor->processCycle(timingOn))
	{
		//store parameters of last operation
		PreviousMemoryPointer = (PreviousMemoryPointer + 1) % MEMORY_DISPLAY_TRACE_FADE_TIME;
		PreviousMemoryCells[PreviousMemoryPointer].ProgramCounter = Processor->getLastProgramCounter();
		PreviousMemoryCells[PreviousMemoryPointer].ArgumentCount = Processor->getLastArgumentCount();
		PreviousMemoryCells[PreviousMemoryPointer].TargetA = Processor->getLastTargetA();
		PreviousMemoryCells[PreviousMemoryPointer].TargetB = Processor->getLastTargetB();

		//add command string to accumulator
		CommandAccumulator += '\n';
		CommandAccumulator.append(Processor->getLastOperation());

		//NOTE: removed due to issues with multithreading.
		/*
		//check if a memory cell was addressed
		if (PreviousMemoryCells[PreviousMemoryPointer].TargetA >= 0 && PreviousMemoryCells[PreviousMemoryPointer].TargetA <= 0xFFFF)
		{
			//get cell value
			unsigned char val = Processor->getMemoryCell((unsigned short)(PreviousMemoryCells[PreviousMemoryPointer].TargetA));

			//update value within memory dump display
			updateMemDumpValue(PreviousMemoryCells[PreviousMemoryPointer].TargetA, val);

			//attempt to update screen display cell
			updateAsmScreenValue(PreviousMemoryCells[PreviousMemoryPointer].TargetA, val);
		}
		*/

		//unlock interface updates
		LockFlag = false;

		return true;
	}

	//unlock interface updates
	LockFlag = false;

	return false;
}

#pragma optimize("",on)

const wxColour ProcFrame::AsmScreenColor[16] =
{
	wxColour(0, 0, 0),			//Black($0)
	wxColour(255, 255, 255),	//White($1)
	wxColour(255, 0, 0),		//Red($2)
	wxColour(0, 255, 255),		//Cyan($3)
	wxColour(255, 0, 255),		//Purple($4)
	wxColour(0, 255, 0),		//Green($5)
	wxColour(0, 0, 255),		//Blue($6)
	wxColour(255, 255, 0),		//Yellow($7)
	wxColour(255, 127, 0),		//Orange($8)
	wxColour(131, 92, 59),		//Brown($9)
	wxColour(255, 127, 127),	//Light red($a)
	wxColour(63, 63, 63),		//Dark gray($b)
	wxColour(127, 127, 127),	//Gray($c)
	wxColour(127, 255, 127),	//Light green($d)
	wxColour(127, 127, 255),	//Light blue($e)
	wxColour(191, 191, 191)		//Light gray($f)
};

BEGIN_EVENT_TABLE(ProcFrame, wxFrame)
EVT_BUTTON(ID_LoadButtonClicked, ProcFrame::onLoadButtonClick)
EVT_BUTTON(ID_SingleOperationButtonClicked, ProcFrame::onSingleOperationButtonClick)
EVT_BUTTON(ID_PlayPauseButtonClicked, ProcFrame::onPlayPauseButtonClick)
EVT_BUTTON(ID_ResetButtonClicked, ProcFrame::onResetButtonClick)
EVT_BUTTON(ID_IrqButtonClicked, ProcFrame::onIrqButtonClick)
EVT_BUTTON(ID_NmiButtonClicked, ProcFrame::onNmiButtonClick)
EVT_SPINCTRL(ID_InputRegisterAChanged, ProcFrame::onInputRegisterAChange)
EVT_SPINCTRL(ID_InputRegisterXChanged, ProcFrame::onInputRegisterXChange)
EVT_SPINCTRL(ID_InputRegisterYChanged, ProcFrame::onInputRegisterYChange)
EVT_SPINCTRL(ID_InputStackPointerChanged, ProcFrame::onInputStackPointerChange)
EVT_SPINCTRL(ID_InputProgramCounterChanged, ProcFrame::onInputProgramCounterChange)
EVT_SPINCTRL(ID_InputFlagNegativeChanged, ProcFrame::onInputFlagNegativeChange)
EVT_SPINCTRL(ID_InputFlagOverflowChanged, ProcFrame::onInputFlagOverflowChange)
EVT_SPINCTRL(ID_InputFlagBlankChanged, ProcFrame::onInputFlagBlankChange)
EVT_SPINCTRL(ID_InputFlagBreakChanged, ProcFrame::onInputFlagBreakChange)
EVT_SPINCTRL(ID_InputFlagDecimalChanged, ProcFrame::onInputFlagDecimalChange)
EVT_SPINCTRL(ID_InputFlagInterruptChanged, ProcFrame::onInputFlagInterruptChange)
EVT_SPINCTRL(ID_InputFlagZeroChanged, ProcFrame::onInputFlagZeroChange)
EVT_SPINCTRL(ID_InputFlagCarryChanged, ProcFrame::onInputFlagCarryChange)
EVT_SPINCTRL(ID_InputVectorNMIChanged, ProcFrame::onInputVectorNMIChange)
EVT_SPINCTRL(ID_InputVectorResetChanged, ProcFrame::onInputVectorResetChange)
EVT_SPINCTRL(ID_InputVectorBRKChanged, ProcFrame::onInputVectorBRKChange)
EVT_SPINCTRLDOUBLE(ID_InputClockFrequencyChanged, ProcFrame::onClockFrequencyChanged)
EVT_THREAD(wxID_ANY, ProcFrame::threadUpdateMethod)
EVT_CLOSE(ProcFrame::onExit)
END_EVENT_TABLE()


wxImagePanel::wxImagePanel(wxFrame* parent, wxBitmap* img) : wxPanel(parent, wxID_ANY, wxDefaultPosition, img->GetSize())
{
	//store bitmap pointer
	image = img;
}

void wxImagePanel::paintEvent(wxPaintEvent & evt)
{
	//render bitmap
	wxPaintDC dc(this);
	dc.DrawBitmap(*image, 0, 0, false);
}

void wxImagePanel::eraseEvent(wxEraseEvent & event)
{
}

BEGIN_EVENT_TABLE(wxImagePanel, wxPanel)
EVT_PAINT(wxImagePanel::paintEvent)
EVT_ERASE_BACKGROUND(wxImagePanel::eraseEvent)
END_EVENT_TABLE()