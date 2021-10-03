#include "MOS6502.h"

#include <regex>

#pragma region Array Definitions

const char const ProcMos6502::Hexadecimal[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

// Allows for more concise syntax.
#define MTH &ProcMos6502

const ProcMos6502::AddressedOpcodeMethod const ProcMos6502::OpcodeFunctionTable[256] =
	{/*	0										1										2										3										4										5										6										7										8										9										A										B										C										D										E										F */
/* 0 */	MTH::ImpliedMode<MTH::op_BRK>,			MTH::IndirectXMode<MTH::op_ORA>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ZeroPageMode<MTH::op_ORA>,			MTH::ZeroPageMode<MTH::op_ASL>,			MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedMode<MTH::op_PHP>,			MTH::ImmediateMode<MTH::op_ORA>,		MTH::AccumulatorMode<MTH::op_ASL>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::AbsoluteMode<MTH::op_ORA>,			MTH::AbsoluteMode<MTH::op_ASL>,			MTH::ImpliedDebugMode<MTH::op_NOP>,
/* 1 */	MTH::RelativeMode<MTH::op_BPL>,			MTH::IndirectYMode<MTH::op_ORA>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ZeroPageXMode<MTH::op_ORA>,		MTH::ZeroPageXMode<MTH::op_ASL>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedMode<MTH::op_CLC>,			MTH::AbsoluteYMode<MTH::op_ORA>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::AbsoluteXMode<MTH::op_ORA>,		MTH::AbsoluteXMode<MTH::op_ASL>,		MTH::ImpliedDebugMode<MTH::op_NOP>,
/* 2 */	MTH::AbsoluteMode<MTH::op_JSR>,			MTH::IndirectXMode<MTH::op_AND>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ZeroPageMode<MTH::op_BIT>,			MTH::ZeroPageMode<MTH::op_AND>,			MTH::ZeroPageMode<MTH::op_ROL>,			MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedMode<MTH::op_PLP>,			MTH::ImmediateMode<MTH::op_AND>,		MTH::AccumulatorMode<MTH::op_ROL>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::AbsoluteMode<MTH::op_BIT>,			MTH::AbsoluteMode<MTH::op_AND>,			MTH::AbsoluteMode<MTH::op_ROL>,			MTH::ImpliedDebugMode<MTH::op_NOP>,
/* 3 */	MTH::RelativeMode<MTH::op_BMI>,			MTH::IndirectYMode<MTH::op_AND>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ZeroPageXMode<MTH::op_AND>,		MTH::ZeroPageXMode<MTH::op_ROL>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedMode<MTH::op_SEC>,			MTH::AbsoluteYMode<MTH::op_AND>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::AbsoluteXMode<MTH::op_AND>,		MTH::AbsoluteXMode<MTH::op_ROL>,		MTH::ImpliedDebugMode<MTH::op_NOP>,
/* 4 */	MTH::ImpliedMode<MTH::op_RTI>,			MTH::IndirectXMode<MTH::op_EOR>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ZeroPageMode<MTH::op_EOR>,			MTH::ZeroPageMode<MTH::op_LSR>,			MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedMode<MTH::op_PHA>,			MTH::ImmediateMode<MTH::op_EOR>,		MTH::AccumulatorMode<MTH::op_LSR>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::AbsoluteMode<MTH::op_JMP>,			MTH::AbsoluteMode<MTH::op_EOR>,			MTH::AbsoluteMode<MTH::op_LSR>,			MTH::ImpliedDebugMode<MTH::op_NOP>,
/* 5 */	MTH::RelativeMode<MTH::op_BVC>,			MTH::IndirectYMode<MTH::op_EOR>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ZeroPageXMode<MTH::op_EOR>,		MTH::ZeroPageXMode<MTH::op_LSR>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedMode<MTH::op_CLI>,			MTH::AbsoluteYMode<MTH::op_EOR>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::AbsoluteXMode<MTH::op_EOR>,		MTH::AbsoluteXMode<MTH::op_LSR>,		MTH::ImpliedDebugMode<MTH::op_NOP>,
/* 6 */	MTH::ImpliedMode<MTH::op_RTS>,			MTH::IndirectXMode<MTH::op_ADC>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ZeroPageMode<MTH::op_ADC>,			MTH::ZeroPageMode<MTH::op_ROR>,			MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedMode<MTH::op_PLA>,			MTH::ImmediateMode<MTH::op_ADC>,		MTH::AccumulatorMode<MTH::op_ROR>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::IndirectMode<MTH::op_JMP>,			MTH::AbsoluteMode<MTH::op_ADC>,			MTH::AbsoluteMode<MTH::op_ROR>,			MTH::ImpliedDebugMode<MTH::op_NOP>,
/* 7 */	MTH::RelativeMode<MTH::op_BVS>,			MTH::IndirectYMode<MTH::op_ADC>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ZeroPageXMode<MTH::op_ADC>,		MTH::ZeroPageXMode<MTH::op_ROR>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedMode<MTH::op_SEI>,			MTH::AbsoluteYMode<MTH::op_ADC>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::AbsoluteXMode<MTH::op_ADC>,		MTH::AbsoluteXMode<MTH::op_ROR>,		MTH::ImpliedDebugMode<MTH::op_NOP>,
/* 8 */	MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::IndirectXMode<MTH::op_STA>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ZeroPageMode<MTH::op_STY>,			MTH::ZeroPageMode<MTH::op_STA>,			MTH::ZeroPageMode<MTH::op_STX>,			MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedMode<MTH::op_DEY>,			MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedMode<MTH::op_TXA>,			MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::AbsoluteMode<MTH::op_STY>,			MTH::AbsoluteMode<MTH::op_STA>,			MTH::AbsoluteMode<MTH::op_STX>,			MTH::ImpliedDebugMode<MTH::op_NOP>,
/* 9 */	MTH::RelativeMode<MTH::op_BCC>,			MTH::IndirectYMode<MTH::op_STA>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ZeroPageXMode<MTH::op_STY>,		MTH::ZeroPageXMode<MTH::op_STA>,		MTH::ZeroPageYMode<MTH::op_STX>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedMode<MTH::op_TYA>,			MTH::AbsoluteYMode<MTH::op_STA>,		MTH::ImpliedMode<MTH::op_TXS>,			MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::AbsoluteXMode<MTH::op_STA>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,
/* A */	MTH::ImmediateMode<MTH::op_LDY>,		MTH::IndirectXMode<MTH::op_LDA>,		MTH::ImmediateMode<MTH::op_LDX>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ZeroPageMode<MTH::op_LDY>,			MTH::ZeroPageMode<MTH::op_LDA>,			MTH::ZeroPageMode<MTH::op_LDX>,			MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedMode<MTH::op_TAY>,			MTH::ImmediateMode<MTH::op_LDA>,		MTH::ImpliedMode<MTH::op_TAX>,			MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::AbsoluteMode<MTH::op_LDY>,			MTH::AbsoluteMode<MTH::op_LDA>,			MTH::AbsoluteMode<MTH::op_LDX>,			MTH::ImpliedDebugMode<MTH::op_NOP>,
/* B */	MTH::RelativeMode<MTH::op_BCS>,			MTH::IndirectYMode<MTH::op_LDA>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ZeroPageXMode<MTH::op_LDY>,		MTH::ZeroPageXMode<MTH::op_LDA>,		MTH::ZeroPageYMode<MTH::op_LDX>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedMode<MTH::op_CLV>,			MTH::AbsoluteYMode<MTH::op_LDA>,		MTH::ImpliedMode<MTH::op_TSX>,			MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::AbsoluteXMode<MTH::op_LDY>,		MTH::AbsoluteXMode<MTH::op_LDA>,		MTH::AbsoluteYMode<MTH::op_LDX>,		MTH::ImpliedDebugMode<MTH::op_NOP>,
/* C */	MTH::ImmediateMode<MTH::op_CPY>,		MTH::IndirectXMode<MTH::op_CMP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ZeroPageMode<MTH::op_CPY>,			MTH::ZeroPageMode<MTH::op_CMP>,			MTH::ZeroPageMode<MTH::op_DEC>,			MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedMode<MTH::op_INY>,			MTH::ImmediateMode<MTH::op_CMP>,		MTH::ImpliedMode<MTH::op_DEX>,			MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::AbsoluteMode<MTH::op_CPY>,			MTH::AbsoluteMode<MTH::op_CMP>,			MTH::AbsoluteMode<MTH::op_DEC>,			MTH::ImpliedDebugMode<MTH::op_NOP>,
/* D */	MTH::RelativeMode<MTH::op_BNE>,			MTH::IndirectYMode<MTH::op_CMP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ZeroPageXMode<MTH::op_CMP>,		MTH::ZeroPageXMode<MTH::op_DEC>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedMode<MTH::op_CLD>,			MTH::AbsoluteYMode<MTH::op_CMP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::AbsoluteXMode<MTH::op_CMP>,		MTH::AbsoluteXMode<MTH::op_DEC>,		MTH::ImpliedDebugMode<MTH::op_NOP>,
/* E */	MTH::ImmediateMode<MTH::op_CPX>,		MTH::IndirectXMode<MTH::op_SBC>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ZeroPageMode<MTH::op_CPX>,			MTH::ZeroPageMode<MTH::op_SBC>,			MTH::ZeroPageMode<MTH::op_INC>,			MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedMode<MTH::op_INX>,			MTH::ImmediateMode<MTH::op_SBC>,		MTH::ImpliedMode<MTH::op_NOP>,			MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::AbsoluteMode<MTH::op_CPX>,			MTH::AbsoluteMode<MTH::op_SBC>,			MTH::AbsoluteMode<MTH::op_INC>,			MTH::ImpliedDebugMode<MTH::op_NOP>,
/* F */	MTH::RelativeMode<MTH::op_BEQ>,			MTH::IndirectYMode<MTH::op_SBC>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ZeroPageXMode<MTH::op_SBC>,		MTH::ZeroPageXMode<MTH::op_INC>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedMode<MTH::op_SED>,			MTH::AbsoluteYMode<MTH::op_SBC>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::ImpliedDebugMode<MTH::op_NOP>,		MTH::AbsoluteXMode<MTH::op_SBC>,		MTH::AbsoluteXMode<MTH::op_INC>,		MTH::ImpliedDebugMode<MTH::op_NOP>
	};

const char const ProcMos6502::OpcodeTimeTable[256] =
	{/*	0	1	2	3	4	5	6	7	8	9	A	B	C	D	E	F */
/* 0 */	7,	6,	2,	2,	2,	3,	5,	2,	3,	2,	2,	2,	2,	4,	6,	2,
/* 1 */	2,	-5,	2,	2,	2,	4,	6,	2,	2,	-4,	2,	2,	2,	-4,	7,	2,
/* 2 */	6,	6,	2,	2,	2,	3,	5,	2,	4,	2,	2,	2,	2,	4,	6,	2,
/* 3 */	2,	-5,	2,	2,	2,	4,	6,	2,	2,	-4,	2,	2,	2,	-4,	7,	2,
/* 4 */	6,	6,	2,	2,	2,	3,	5,	2,	3,	2,	2,	2,	3,	4,	6,	2,
/* 5 */	2,	-5,	2,	2,	2,	4,	6,	2,	2,	-4,	2,	2,	2,	-4,	7,	2,
/* 6 */	6,	6,	2,	2,	2,	3,	5,	2,	4,	2,	2,	2,	5,	4,	6,	2,
/* 7 */	2,	-5,	2,	2,	2,	4,	6,	2,	2,	-4,	2,	2,	2,	-4,	7,	2,
/* 8 */	2,	6,	2,	2,	3,	3,	3,	2,	2,	2,	2,	2,	4,	4,	4,	2,
/* 9 */	2,	6,	2,	2,	4,	4,	4,	2,	2,	5,	2,	2,	2,	5,	2,	2,
/* A */	2,	6,	2,	2,	3,	3,	3,	2,	2,	2,	2,	2,	4,	4,	4,	2,
/* B */	2,	-5,	2,	2,	4,	4,	4,	2,	2,	-4,	2,	2,	-4,	-4,	-4,	2,
/* C */	2,	6,	2,	2,	3,	3,	5,	2,	2,	2,	2,	2,	4,	4,	6,	2,
/* D */	2,	-5,	2,	2,	2,	4,	6,	2,	2,	-4,	2,	2,	2,	-4,	7,	2,
/* E */	2,	6,	2,	2,	3,	3,	5,	2,	2,	2,	2,	2,	4,	4,	6,	2,
/* F */	2,	-5,	2,	2,	2,	4,	6,	2,	2,	-4,	2,	2,	2,	-4,	7,	2
	};

const std::string const ProcMos6502::OpcodeNameTable[256] =
	{/*	0		1		2		3		4		5		6		7		8		9		A		B		C		D		E		F */
/* 0 */	"BRK",	"ORA",	"NOP",	"NOP",	"NOP",	"ORA",	"ASL",	"NOP",	"PHP",	"ORA",	"ASL",	"NOP",	"NOP",	"ORA",	"ASL",	"NOP",
/* 1 */	"BPL",	"ORA",	"NOP",	"NOP",	"NOP",	"ORA",	"ASL",	"NOP",	"CLC",	"ORA",	"NOP",	"NOP",	"NOP",	"ORA",	"ASL",	"NOP",
/* 2 */	"JSR",	"AND",	"NOP",	"NOP",	"BIT",	"AND",	"ROL",	"NOP",	"PLP",	"AND",	"ROL",	"NOP",	"BIT",	"AND",	"ROL",	"NOP",
/* 3 */	"BMI",	"AND",	"NOP",	"NOP",	"NOP",	"AND",	"ROL",	"NOP",	"SEC",	"AND",	"NOP",	"NOP",	"NOP",	"AND",	"ROL",	"NOP",
/* 4 */	"RTI",	"EOR",	"NOP",	"NOP",	"NOP",	"EOR",	"LSR",	"NOP",	"PHA",	"EOR",	"LSR",	"NOP",	"JMP",	"EOR",	"LSR",	"NOP",
/* 5 */	"BVC",	"EOR",	"NOP",	"NOP",	"NOP",	"EOR",	"LSR",	"NOP",	"CLI",	"EOR",	"NOP",	"NOP",	"NOP",	"EOR",	"LSR",	"NOP",
/* 6 */	"RTS",	"ADC",	"NOP",	"NOP",	"NOP",	"ADC",	"ROR",	"NOP",	"PLA",	"ADC",	"ROR",	"NOP",	"JMP",	"ADC",	"ROR",	"NOP",
/* 7 */	"BVS",	"ADC",	"NOP",	"NOP",	"NOP",	"ADC",	"ROR",	"NOP",	"SEI",	"ADC",	"NOP",	"NOP",	"NOP",	"ADC",	"ROR",	"NOP",
/* 8 */	"NOP",	"STA",	"NOP",	"NOP",	"STY",	"STA",	"STX",	"NOP",	"DEY",	"NOP",	"TXA",	"NOP",	"STY",	"STA",	"STX",	"NOP",
/* 9 */	"BCC",	"STA",	"NOP",	"NOP",	"STY",	"STA",	"STX",	"NOP",	"TYA",	"STA",	"TXS",	"NOP",	"NOP",	"STA",	"NOP",	"NOP",
/* A */	"LDY",	"LDA",	"LDX",	"NOP",	"LDY",	"LDA",	"LDX",	"NOP",	"TAY",	"LDA",	"TAX",	"NOP",	"LDY",	"LDA",	"LDX",	"NOP",
/* B */	"BCS",	"LDA",	"NOP",	"NOP",	"LDY",	"LDA",	"LDX",	"NOP",	"CLV",	"LDA",	"TSX",	"NOP",	"LDY",	"LDA",	"LDX",	"NOP",
/* C */	"CPY",	"CMP",	"NOP",	"NOP",	"CPY",	"CMP",	"DEC",	"NOP",	"INY",	"CMP",	"DEX",	"NOP",	"CPY",	"CMP",	"DEC",	"NOP",
/* D */	"BNE",	"CMP",	"NOP",	"NOP",	"NOP",	"CMP",	"DEC",	"NOP",	"CLD",	"CMP",	"NOP",	"NOP",	"NOP",	"CMP",	"DEC",	"NOP",
/* E */	"CPX",	"SBC",	"NOP",	"NOP",	"CPX",	"SBC",	"INC",	"NOP",	"INX",	"SBC",	"NOP",	"NOP",	"CPX",	"SBC",	"INC",	"NOP",
/* F */	"BEQ",	"SBC",	"NOP",	"NOP",	"NOP",	"SBC",	"INC",	"NOP",	"SED",	"SBC",	"NOP",	"NOP",	"NOP",	"SBC",	"INC",	"NOP"
	};

#pragma endregion

bool ProcMos6502::getFlag(ProcessorFlags flag)
{
	if (Register[Status] & flag) return true;
	else return false;
}

void ProcMos6502::setFlag(ProcessorFlags flag, bool value)
{
	if (value)
		Register[Status] |= flag;
	else
		Register[Status] &= 0xff ^ flag;

}

unsigned char ProcMos6502::getNextByte()
{
	//save output
	unsigned char out = Memory[ProgramCounter];

	//increment program counter
	ProgramCounter = (ProgramCounter + 1) % 0x10000;

	return out;
}

bool ProcMos6502::processCycle(bool timingOn)
{
	//check if the processor should wait
	if (timingOn && AdditionalProcessingTime > 0)
	{
		//decrement the wait time
		AdditionalProcessingTime--;

		//stop cycle
		return false;
	}

	//save program counter
	LastProgramCounter = ProgramCounter;

	//reset opcode description
	LastOperation = "";

	//add location to description
	TWO_HEX_BYTES(LastProgramCounter);
	LastOperation.append(":\t");

	//get next operation
	unsigned char opcode = getNextByte();

	//add opcode name to description
	LastOperation.append(OpcodeNameTable[opcode]);
	
	//reset processing timer to match the retrieved opcode value
	AdditionalProcessingTime = OpcodeTimeTable[opcode];

	//initialize negative time flag
	bool negativeTime = false;

	//check whether the retrieved time is negative or not
	if (AdditionalProcessingTime < 0)
	{
		//adjust time
		AdditionalProcessingTime = 0 - AdditionalProcessingTime;

		//set flag value
		negativeTime = true;
	}

	//perform next operation and store whether the page bounds were crossed or not (automatically increments program counter)
	bool pageCross = (this->*(OpcodeFunctionTable[opcode]))();

	//increment time by 1 if page bounds were crossed and time was negative
	if (negativeTime && pageCross) AdditionalProcessingTime++;

	//decrease the wait time by 1 (adjusting for current cycle)
	AdditionalProcessingTime--;

	return true;
}

void ProcMos6502::reset()
{
	//set program counter to the reset vector
	ProgramCounter = ((int)Memory[RESET_HIGH] << 8) | Memory[RESET_LOW];
	//ProgramCounter = 0x600;

	//reset registers
	Register[0] = 0;
	Register[1] = 0;
	Register[2] = 0;
	Register[3] = 0b00110100; //sets I, B and unused, clears remaining

	//reset stack pointer
	StackPointer = 0xFF;

	//set additional wait time to 6 cycles (execution time of the reset command)
	AdditionalProcessingTime = 6;

	//reset the rendering helper variables
	LastTargetA = OP_INVALID_TARGET;
	LastTargetB = OP_INVALID_TARGET;
	LastProgramCounter = ProgramCounter;
	LastArgumentCount = 0;
}

void ProcMos6502::triggerIRQ()
{
	//check if Interrupt flag is clear
	if (!getFlag(Interrupt))
	{
		//push program counter to the stack
		pushProgramCounter();

		//push status word to the stack with Break flag set
		op_PHP();

		//set disable interrupt flag
		setFlag(Interrupt, true);

		//get program counter from appropriate memory location
		ProgramCounter = ((unsigned short)Memory[BRK_HIGH] << 8) | Memory[BRK_LOW];

		//add 6 cycles to program execution
		AdditionalProcessingTime += 6;
	}
}

void ProcMos6502::startNMI()
{
	//check if NMI flag is clear
	if (!IsNMIon)
	{
		//push program counter to the stack
		pushProgramCounter();

		//push status word to the stack with Break flag set
		op_PHP();

		//set disable interrupt flag
		setFlag(Interrupt, true);

		//get program counter from appropriate memory location
		ProgramCounter = ((unsigned short)Memory[NMI_HIGH] << 8) | Memory[NMI_LOW];

		//add 6 cycles to program execution
		AdditionalProcessingTime += 6;
	}
}

void ProcMos6502::endNMI()
{
	//reset NMI flag
	IsNMIon = false;
}

bool ProcMos6502::loadFromBinStream(std::istream & mem)
{
	//initialize counter
	int i = 0;

	//load memory from file
	for (; i < 0x10000 && !mem.eof(); i++)
		Memory[i] = mem.get();

	//return true if full memory has been loaded, otherwise fill rest with 0 and return false
	if (i >= 0x10000) return true;
	else
	{
		for (; i < 0x10000; i++)
			Memory[i] = 0;
		return false;
	}

	//reset processor
	reset();
}

bool ProcMos6502::loadFromHexStream(std::istream & mem)
{
	//clear memory
	for (int i = 0; i < 0x10000; i++)
		Memory[(unsigned short)i] = 0;

	//start whitespace cleanup parser
	std::regex cleanupParserWhitespace("[^\\:0123456789ABCDEFabcdef]+");

	//initialize variables
	unsigned int address;
	unsigned int valBuffer;
	unsigned char charBuffer;

	//create lambda function parsing hexadecimal characters into their proper values
	auto hexToVal = [](unsigned char& val)
	{
		//pull characters a-f down
		if (val >= 'a')
			val -= 'a' - 10;

		//pull characters A-F down
		else if (val >= 'A')
			val -= 'A' - 10;

		//pull characters 0-9 down
		else
			val -= '0';

		//remove dangling bits (if this ever happens it's probably due to an error, but at least I'll remember this place in code to check for that)
		val &= 0xF;
	};

	//initialize line buffer
	std::string lineBuffer;

	//get first line
	std::getline(mem, lineBuffer);

	//cleanup whitespace
	lineBuffer = std::regex_replace(lineBuffer, cleanupParserWhitespace, "");

	//check if file is in the Intel HEX format
	if (std::regex_search(lineBuffer, std::regex("^\\:[0123456789ABCDEFabcdef]+")))
	{
		//initialize variable
		unsigned int byteCount;

		//perform operations
		do
		{
			//get record type
			charBuffer = lineBuffer.at(7);
			hexToVal(charBuffer);
			valBuffer = charBuffer << 4;
			charBuffer = lineBuffer.at(8);
			hexToVal(charBuffer);
			valBuffer |= charBuffer;

			if (valBuffer != 0)
			{
				if (valBuffer == 1) return true;
				else return false;
			}

			//get address
			address = 0;
			for (int i = 3; i <= 6; i++)
			{
				//get character
				charBuffer = lineBuffer.at(i);

				//convert character into value
				hexToVal(charBuffer);

				//shift offset to the left
				address <<= 4;

				//add value
				address |= charBuffer;
			}

			//get byte count
			charBuffer = lineBuffer.at(1);
			hexToVal(charBuffer);
			byteCount = charBuffer << 4;
			charBuffer = lineBuffer.at(2);
			hexToVal(charBuffer);
			byteCount |= charBuffer;

			//add data byte values at appropriate memory locations
			for (int i = 0; i < byteCount; i++)
			{
				//convert characters to byte
				charBuffer = lineBuffer.at(9 + (i * 2));
				hexToVal(charBuffer);
				valBuffer = charBuffer << 4;
				charBuffer = lineBuffer.at(10 + (i * 2));
				hexToVal(charBuffer);
				valBuffer |= charBuffer;

				//write byte to memory
				Memory[(unsigned short)(address + i)] = (unsigned char)valBuffer;
			}
		}
		//get next line
		while (std::getline(mem, lineBuffer));

		//reset processor
		reset();

		return true;
	}
	//check if file is in the hexdump format
	else if (std::regex_search(lineBuffer, std::regex("^[0123456789ABCDEFabcdef]{4}\\:[0123456789ABCDEFabcdef]+")))
	{
		//initialize hexdump parser
		std::regex addressParser("^[0123456789ABCDEFabcdef]{4}\\:");
		
		//initialize variables
		std::string substringBuffer;
		std::smatch searchOutput;

		//parse lines while EOF wasn't reached
		do
		{
			//cleanup whitespace
			lineBuffer = std::regex_replace(lineBuffer, cleanupParserWhitespace, "");

			//reset address
			address = 0;

			//attempt to get address string, stop execution if one is not found
			if (!std::regex_search(lineBuffer, searchOutput, addressParser)) return false;
			substringBuffer = searchOutput[0].str();

			//parse address into value
			for (int i = 0; (charBuffer = substringBuffer.at(i)) != ':'; i++)
			{
				//convert character into value
				hexToVal(charBuffer);

				//shift address to the left
				address <<= 4;

				//add value
				address |= charBuffer;
			}

			//purge address from line buffer
			lineBuffer = std::regex_replace(lineBuffer, addressParser, "");

			//add data byte values at appropriate memory locations
			for (int i = 0; i < (int)(lineBuffer.length() / 2); i++)
			{
				//convert characters to byte
				charBuffer = lineBuffer.at(i * 2);
				hexToVal(charBuffer);
				valBuffer = charBuffer << 4;
				charBuffer = lineBuffer.at(1 + (i * 2));
				hexToVal(charBuffer);
				valBuffer |= charBuffer;

				//write byte to memory
				Memory[(unsigned short)(address + i)] = (unsigned char)valBuffer;
			}
		}
		//get next line
		while (std::getline(mem, lineBuffer));

		//reset processor
		reset();

		return true;
	}
	else //invalid file format, impossible to parse
		return false;
}

ProcMos6502::ProcMos6502()
{
	//fill memory with 0 values
	for (int i = 0; i < 0x10000; i++)
		Memory[i] = 0;

	//reset the processor
	reset();
}

/*
ProcMos6502::ProcMos6502(std::istream& mem)
{
	//perform default initialization
	ProcMos6502();

	//load binary stream into memory array
	loadFromBinStream(mem);

	//reset processor
	reset();
}
*/

void ProcMos6502::op_ADC(unsigned char * arg)
{
	//store argument value
	unsigned char val = *arg;

	//initialize output variable
	unsigned short result;

	//check if addition should be performed in BCD mode
	if (getFlag(Decimal))
	{
		//perform addition of low BCD digit
		unsigned char low = (Register[Accumulator] & 0x0F) + (val & 0x0F);

		//add carry (if set)
		if (getFlag(Carry)) low++;

		//check if low digit is over 9
		if (low > 9)
		{
			//normalize low digit value
			low = (low + 6) & 0x0F;

			//add high digit carry value
			low += 0x10;
		}

		//calculate result
		result = ((unsigned short)Register[Accumulator] & 0xF0) + (val & 0xF0) + low;

		//check if result exceeded 100 and set Carry appropriately
		if (result >= 0xA0)
		{
			//adjust result
			result += 0x60;

			//set Carry to true
			setFlag(Carry, true);
		}
		else
		{
			//set Carry to false
			setFlag(Carry, false);
		}
	}
	else
	{
		//perform addition
		result = (unsigned short)Register[Accumulator] + (unsigned short)val;

		//add carry (if set)
		if (getFlag(Carry)) result++;

		//set or clear the Carry flag
		setFlag(Carry, result > 0xFF);
	}

	//set or clear the Overflow flag
	if ((Register[Accumulator] ^ result) & (val ^ result) & 0b10000000)
		setFlag(Overflow, true);
	else
		setFlag(Overflow, false);

	//perform flag tests
	ZERO_FLAG_TEST(result & 0xFF);
	SIGN_FLAG_TEST(result & 0xFF);

	//put result in accumulator
	Register[Accumulator] = result;
}

void ProcMos6502::op_AND(unsigned char * arg)
{
	//perform operation
	Register[Accumulator] &= *arg;

	//perform flag tests
	ZERO_FLAG_TEST(Register[Accumulator]);
	SIGN_FLAG_TEST(Register[Accumulator]);
}

void ProcMos6502::op_ASL(unsigned char * arg)
{

	//set or clear the Carry flag
	if ((*arg) & 0b10000000)
		setFlag(Carry, true);
	else
		setFlag(Carry, false);

	//perform operation
	*arg <<= 1;

	//perform flag tests
	ZERO_FLAG_TEST(*arg);
	SIGN_FLAG_TEST(*arg);
}

void ProcMos6502::op_BIT(unsigned char * arg)
{
	//set or clear the Zero flag
	ZERO_FLAG_TEST(Register[Accumulator] & (*arg));

	//copy bits 6 and 7 of the value at address into status register
	Register[Status] = (Register[Status] & 0b00111111) | ((*arg) & 0b11000000);
}

void ProcMos6502::op_BRK()
{
	//increment program counter
	ProgramCounter++;

	//push program counter to the stack
	pushProgramCounter();

	//push status word to the stack with Break flag set
	op_PHP();

	//set disable interrupt flag
	setFlag(Interrupt, true);

	//get program counter from appropriate memory location
	ProgramCounter = ((unsigned short)Memory[BRK_HIGH] << 8) | Memory[BRK_LOW];
}

void ProcMos6502::op_DEC(unsigned char * arg)
{
	//decrement memory
	(*arg)--;

	//perform flag tests
	ZERO_FLAG_TEST(*arg);
	SIGN_FLAG_TEST(*arg);
}

void ProcMos6502::op_EOR(unsigned char * arg)
{
	//perform operation
	Register[Accumulator] ^= *arg;

	//perform flag tests
	ZERO_FLAG_TEST(Register[Accumulator]);
	SIGN_FLAG_TEST(Register[Accumulator]);
}

void ProcMos6502::op_INC(unsigned char * arg)
{
	//increment memory
	(*arg)++;

	//perform flag tests
	ZERO_FLAG_TEST(*arg);
	SIGN_FLAG_TEST(*arg);
}

void ProcMos6502::op_JMP(unsigned char * arg)
{
	//convert retrieved pointer into the new program counter
	ProgramCounter = arg - Memory;
}

void ProcMos6502::op_JSR(unsigned char * arg)
{
	//push Program Counter - 1 to the stack
	ProgramCounter--;
	pushProgramCounter();

	//replace program counter with the retrieved value
	ProgramCounter = arg - Memory;
}

void ProcMos6502::op_LSR(unsigned char * arg)
{
	//copy the value of bit 0 into Carry
	setFlag(Carry, (*arg) & 1);

	//shift value right
	*arg >>= 1;

	//perform flag tests
	ZERO_FLAG_TEST(*arg);
	SIGN_FLAG_TEST(*arg);
}

void ProcMos6502::op_NOP()
{
	//do nothing
}

void ProcMos6502::op_ORA(unsigned char * arg)
{
	//perform operation
	Register[Accumulator] |= *arg;

	//perform flag tests
	ZERO_FLAG_TEST(Register[Accumulator]);
	SIGN_FLAG_TEST(Register[Accumulator]);
}

void ProcMos6502::op_ROL(unsigned char * arg)
{
	//store current Carry value
	bool c = getFlag(Carry);

	//copy the value of bit 7 into Carry
	setFlag(Carry, (*arg) & 0b10000000);

	//shift value left
	*arg <<= 1;

	//adjust for previous Carry value
	if (c) *arg |= 1;

	//perform flag tests
	ZERO_FLAG_TEST(*arg);
	SIGN_FLAG_TEST(*arg);
}

void ProcMos6502::op_ROR(unsigned char * arg)
{
	//store current Carry value
	bool c = getFlag(Carry);

	//copy the value of bit 1 into Carry
	setFlag(Carry, (*arg) & 1);

	//shift value right
	*arg >>= 1;

	//adjust for previous Carry value
	if (c) *arg |= 0b10000000;

	//perform flag tests
	ZERO_FLAG_TEST(*arg);
	SIGN_FLAG_TEST(*arg);
}

void ProcMos6502::op_RTI()
{
	//get processor status from stack
	op_PLP();

	//get program counter from the stack
	pullProgramCounter();
}

void ProcMos6502::op_RTS()
{
	//get program counter from stack and increment it
	pullProgramCounter();
	ProgramCounter++;
}

void ProcMos6502::op_SBC(unsigned char * arg)
{
	//store argument value
	unsigned char val = *arg;

	//initialize output variable
	short result;

	//check if substraction should be performed in BCD mode
	if (getFlag(Decimal))
	{
		//perform substraction of low BCD digit
		char low = (Register[Accumulator] & 0x0F) - (val & 0x0F) - 1;

		//add carry (if set)
		if (getFlag(Carry)) low++;

		//check if low digit is under 0
		if (low < 0)
		{
			//normalize low digit value
			low = (low - 6) & 0x0F;

			//substract high digit carry value
			low -= 0x10;
		}

		//calculate result
		result = ((short)Register[Accumulator] & 0xF0) - (val & 0xF0) + low;

		//check if result is less than 0
		if (result < 0)
		{
			//adjust result
			result -= 0x60;
		}
	}
	else
	{
		//perform substraction
		result = (short)(-1) + (char)Register[Accumulator] - (char)val;

		//add carry (if set)
		if (getFlag(Carry)) result++;
	}

	//set or clear the Overflow flag
	if ((result < -128) || (result > 127))
		setFlag(Overflow, true);
	else
		setFlag(Overflow, false);

	//set or clear the Carry flag
	if (getFlag(Carry))
		setFlag(Carry, Register[Accumulator] >= val);
	else
		setFlag(Carry, Register[Accumulator] >= val + 1);

	//perform flag tests
	ZERO_FLAG_TEST((result & 0xFF));
	SIGN_FLAG_TEST((result & 0xFF));

	//put result in accumulator
	Register[Accumulator] = result;
}

void ProcMos6502::op_TXS()
{
	//transfer value
	StackPointer = Register[X];
}

void ProcMos6502::op_TSX()
{
	//transfer value
	Register[X] = StackPointer;

	//perform flag tests
	ZERO_FLAG_TEST(StackPointer);
	SIGN_FLAG_TEST(StackPointer);
}

void ProcMos6502::pushValue(unsigned char val)
{
	//put the provided value on the stack
	Memory[0x100 + StackPointer] = val;

	//move the pointer
	StackPointer--;
}

unsigned char ProcMos6502::pullValue()
{
	//move the pointer
	StackPointer++;

	return Memory[0x100 + StackPointer];
}

void ProcMos6502::pushProgramCounter()
{
	//push most significant byte
	pushValue(ProgramCounter >> 8);

	//push least significant byte
	pushValue(ProgramCounter & 0xFF);
}

void ProcMos6502::pullProgramCounter()
{
	//pull least significant byte
	unsigned char bep = pullValue();

	//pull most significant byte and put final value in the Program Counter register
	ProgramCounter = ((unsigned short)pullValue() << 8) | bep;
}

void ProcMos6502::op_PHA()
{
	//push the accumulator value to the top of the stack
	pushValue(Register[Accumulator]);
}

void ProcMos6502::op_PLA()
{
	//copy top of the stack into the accumulator
	Register[Accumulator] = pullValue();
}

void ProcMos6502::op_PHP()
{
	//push the processor status to the top of the stack with break flag set
	pushValue(Register[Status] | 0b00110000);
}

void ProcMos6502::op_PLP()
{
	//copy top of the stack into the processor status register (ignores bits 4 and 5)
	Register[Status] = pullValue() | 0b00110000;
}

std::string ProcMos6502::getLastOperation()
{
	return LastOperation;
}

int ProcMos6502::getLastTargetA()
{
	return LastTargetA;
}

int ProcMos6502::getLastTargetB()
{
	return LastTargetB;
}

unsigned short ProcMos6502::getLastProgramCounter()
{
	return LastProgramCounter;
}

unsigned char ProcMos6502::getLastArgumentCount()
{
	return LastArgumentCount;
}

unsigned char ProcMos6502::getMemoryCell(unsigned short index)
{
	return Memory[index];
}

void ProcMos6502::setMemoryCell(unsigned short index, unsigned char val)
{
	Memory[index] = val;
}

unsigned char ProcMos6502::getRegisterA()
{
	return Register[Accumulator];
}

void ProcMos6502::setRegisterA(unsigned char val)
{
	Register[Accumulator] = val;
}

unsigned char ProcMos6502::getRegisterX()
{
	return Register[X];
}

void ProcMos6502::setRegisterX(unsigned char val)
{
	Register[X] = val;
}

unsigned char ProcMos6502::getRegisterY()
{
	return Register[Y];
}

void ProcMos6502::setRegisterY(unsigned char val)
{
	Register[Y] = val;
}

unsigned char ProcMos6502::getStackPointer()
{
	return StackPointer;
}

void ProcMos6502::setStackPointer(unsigned char val)
{
	StackPointer = val;
}

unsigned short ProcMos6502::getProgramCounter()
{
	return ProgramCounter;
}

void ProcMos6502::setProgramCounter(unsigned short val)
{
	ProgramCounter = val;
}

unsigned short ProcMos6502::getVectorNMI()
{
	return (((unsigned short)(Memory[NMI_HIGH])) << 8) | Memory[NMI_LOW];
}

void ProcMos6502::setVectorNMI(unsigned short val)
{
	Memory[NMI_HIGH] = (unsigned char)(val >> 8);
	Memory[NMI_LOW] = (unsigned char)(val & 0xFF);
}

unsigned short ProcMos6502::getVectorReset()
{
	return (((unsigned short)(Memory[RESET_HIGH])) << 8) | Memory[RESET_LOW];
}

void ProcMos6502::setVectorReset(unsigned short val)
{
	Memory[RESET_HIGH] = (unsigned char)(val >> 8);
	Memory[RESET_LOW] = (unsigned char)(val & 0xFF);
}

unsigned short ProcMos6502::getVectorBRK()
{
	return (((unsigned short)(Memory[BRK_HIGH])) << 8) | Memory[BRK_LOW];
}

void ProcMos6502::setVectorBRK(unsigned short val)
{
	Memory[BRK_HIGH] = (unsigned char)(val >> 8);
	Memory[BRK_LOW] = (unsigned char)(val & 0xFF);
}

bool ProcMos6502::getFlagSign()
{
	if (Register[Status] & Sign) return true;
	else return false;
}

void ProcMos6502::setFlagSign(bool val)
{
	if (val) Register[Status] |= Sign;
	else Register[Status] &= ~Sign;

}

bool ProcMos6502::getFlagOverflow()
{
	if (Register[Status] & Overflow) return true;
	else return false;
}

void ProcMos6502::setFlagOverflow(bool val)
{
	if (val) Register[Status] |= Overflow;
	else Register[Status] &= ~Overflow;
}

bool ProcMos6502::getFlagBlank()
{
	//flag always has value 1
	return true;

	//if (Register[Status] & Blank) return true;
	//else return false;
}

void ProcMos6502::setFlagBlank(bool val)
{
	//flag can not be set
	return;

	//if (val) Register[Status] |= Blank;
	//else Register[Status] &= ~Blank;
}

bool ProcMos6502::getFlagBreak()
{
	//flag always has value 1
	return true;

	//if (Register[Status] & Break) return true;
	//else return false;
}

void ProcMos6502::setFlagBreak(bool val)
{
	//flag can not be set
	return;

	//if (val) Register[Status] |= Break;
	//else Register[Status] &= ~Break;
}

bool ProcMos6502::getFlagDecimal()
{
	if (Register[Status] & Decimal) return true;
	else return false;
}

void ProcMos6502::setFlagDecimal(bool val)
{
	if (val) Register[Status] |= Decimal;
	else Register[Status] &= ~Decimal;
}

bool ProcMos6502::getFlagInterrupt()
{
	if (Register[Status] & Interrupt) return true;
	else return false;
}

void ProcMos6502::setFlagInterrupt(bool val)
{
	if (val) Register[Status] |= Interrupt;
	else Register[Status] &= ~Interrupt;
}

bool ProcMos6502::getFlagZero()
{
	if (Register[Status] & Zero) return true;
	else return false;
}

void ProcMos6502::setFlagZero(bool val)
{
	if (val) Register[Status] |= Zero;
	else Register[Status] &= ~Zero;
}

bool ProcMos6502::getFlagCarry()
{
	if (Register[Status] & Carry) return true;
	else return false;
}

void ProcMos6502::setFlagCarry(bool val)
{
	if (val) Register[Status] |= Carry;
	else Register[Status] &= ~Carry;
}
