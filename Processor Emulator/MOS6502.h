#pragma once

#include<istream>
#include"interface.h"
#include<string>

// Represents an emulator of a MOS6502 processor.
class ProcMos6502
{
private:

	bool IsNMIon = false;

#pragma region Processor Memory

	// Processor's addressable memory.
	unsigned char Memory[0x10000];

	// Static program counter vectors.
#define NMI_LOW 0xFFFA
#define NMI_HIGH 0xFFFB

#define RESET_LOW 0xFFFC
#define RESET_HIGH 0xFFFD

#define BRK_LOW 0xFFFE
#define BRK_HIGH 0xFFFF

	// Processor registers.
	enum ProcessorRegisters
	{
		Accumulator = 0,
		X = 1,
		Y = 2,
		Status = 3
	};

	// Register array.
	unsigned char Register[4] = { 0, 0, 0, 0xFF };

	// Processor flags.
	enum ProcessorFlags
	{
		Carry		= 0b1,
		Zero		= 0b10,
		Interrupt	= 0b100,
		Decimal		= 0b1000,
		Break		= 0b10000,
		Blank		= 0b100000,
		Overflow	= 0b1000000,
		Sign		= 0b10000000
	};

	// Gets the current state of the given flag.
	bool getFlag(ProcessorFlags flag);

	// Sets the current state of the given flag.
	void setFlag(ProcessorFlags flag, bool value);

	// Program counter.
	unsigned short ProgramCounter;

	// Gets the next byte address and increments the program counter.
	unsigned char getNextByte();

	// Stack pointer.
	unsigned char StackPointer = 0xFF;

#pragma endregion

#pragma region Typedefs

	// Opcode method pointer signature.
	typedef void (ProcMos6502::*OpcodeMethod)(unsigned char *);

	// Implied mode only opcode method pointer signature.
	typedef void (ProcMos6502::*ImpliedOpcodeMethod)(void);

	// Addressed opcode method pointer signature.
	typedef bool (ProcMos6502::*AddressedOpcodeMethod)(void);

	// Error code signifying invalid target.
	#define OP_INVALID_TARGET -1

#pragma endregion

#pragma region Addressing Modes

#define ONE_HEX_BYTE(VAR) LastOperation += Hexadecimal[((VAR) >> 4) & 0xF]; LastOperation += Hexadecimal[(VAR) & 0xF]
#define TWO_HEX_BYTES(VAR) LastOperation += Hexadecimal[((VAR) >> 12) & 0xF]; LastOperation += Hexadecimal[((VAR) >> 8) & 0xF]; LastOperation += Hexadecimal[((VAR) >> 4) & 0xF]; LastOperation += Hexadecimal[(VAR) & 0xF]
#define VALUE_SUFIX(VAR) LastOperation.append(" [$"); ONE_HEX_BYTE((*(VAR))); LastOperation.append("]")

	// Template method for the Accumulator addressing mode.
	template<OpcodeMethod OP>
	bool AccumulatorMode()
	{
		//add operation sufix to string
		LastOperation.append(" A");

		//perform operation on accumulator
		(*this.*OP)(Register + Accumulator);

		//set operation target helpers
		LastTargetA = OP_INVALID_TARGET;
		LastTargetB = OP_INVALID_TARGET;
		LastArgumentCount = 0;

		//add value sufix to string
		VALUE_SUFIX(&(Register[Accumulator]));

		return false;
	}

	// Template method for the Immediate addressing mode.
	template<OpcodeMethod OP>
	bool ImmediateMode()
	{
		//get next byte
		unsigned char bep = getNextByte();

		//perform operation on retrieved value
		(*this.*OP)(&bep);

		//add operation sufix to string
		LastOperation.append(" #$");
		ONE_HEX_BYTE(bep);

		//set operation target helpers
		LastTargetA = ProgramCounter - 1;
		LastTargetB = OP_INVALID_TARGET;
		LastArgumentCount = 1;

		return false;
	}

	// Template method for the Implied addressing mode.
	template<ImpliedOpcodeMethod OP>
	bool ImpliedMode()
	{
		//perform operation
		(*this.*OP)();

		//set operation target helpers
		LastTargetA = OP_INVALID_TARGET;
		LastTargetB = OP_INVALID_TARGET;
		LastArgumentCount = 0;

		return false;
	}

	// Acts as Implied mode, but prints a warning message if called.
	template<ImpliedOpcodeMethod OP>
	bool ImpliedDebugMode()
	{
		//TODO: replace with warning!
		//throw "WRONG OPCODE PERFORMED!";

		//perform operation
		(*this.*OP)();

		//add operation sufix to string
		LastOperation.append(" [!!!]");

		//set operation target helpers
		LastTargetA = OP_INVALID_TARGET;
		LastTargetB = OP_INVALID_TARGET;
		LastArgumentCount = 0;

		return false;
	}

	// Template method for the Relative addressing mode.
	template<OpcodeMethod OP>
	bool RelativeMode()
	{
		//get next byte
		char bep = getNextByte();

		//perform operation on the address
		(*this.*OP)(Memory + ((ProgramCounter + bep) % 0x10000));

		//add operation sufix to string
		LastOperation.append(" ");
		LastOperation.append(std::to_string(bep));

		//set operation target helpers
		LastTargetA = (ProgramCounter + bep) % 0x10000;
		LastTargetB = OP_INVALID_TARGET;
		LastArgumentCount = 1;

		//perform page boundary cross test
		if ((((ProgramCounter + bep) % 0x10000) & 0xFF00) == (ProgramCounter & 0xFF00)) return false; //TODO: CHECK IF CORRECT!
		else return true;
	}

	// Template method for the Absolute addressing mode.
	template<OpcodeMethod OP>
	bool AbsoluteMode()
	{
		//get next byte pair
		unsigned char a = getNextByte();
		unsigned char b = getNextByte();

		//convert to address
		unsigned short add = ((unsigned short)b) << 8 | a;

		//perform operation on address
		(*this.*OP)(Memory + add);

		//add operation sufix to string
		LastOperation.append(" $");
		TWO_HEX_BYTES(add);

		//add value sufix to string
		VALUE_SUFIX(Memory + add);

		//set operation target helpers
		LastTargetA = add;
		LastTargetB = OP_INVALID_TARGET;
		LastArgumentCount = 2;

		//perform page boundary cross test
		if ((add & 0xFF00) == (ProgramCounter & 0xFF00)) return false;
		else return true;
	}

	// Template method for the Zero-Page addressing mode.
	template<OpcodeMethod OP>
	bool ZeroPageMode()
	{
		//get zero-page address
		unsigned char bep = getNextByte();

		//perform operation on retrieved address
		(*this.*OP)(Memory + bep);

		//add operation sufix to string
		LastOperation.append(" $");
		ONE_HEX_BYTE(bep);

		//add value sufix to string
		VALUE_SUFIX(Memory + bep);

		//set operation target helpers
		LastTargetA = bep;
		LastTargetB = OP_INVALID_TARGET;
		LastArgumentCount = 0;

		return false;
	}

	// Template method for the Indirect addressing mode.
	template<OpcodeMethod OP>
	bool IndirectMode()
	{
		//get next byte pair
		unsigned char a = getNextByte();
		unsigned char b = getNextByte();

		//convert to preliminary address
		unsigned short addt = ((unsigned short)b) << 8 | a;

		//retrieve the final address bytes
		a = Memory[addt];
		b = Memory[addt + 1];

		//convert to final address
		unsigned short add = ((unsigned short)b) << 8 | a;

		//perform operation on address
		(*this.*OP)(Memory + add);

		//add operation sufix to string
		LastOperation.append(" ($");
		TWO_HEX_BYTES(addt);
		LastOperation.append(")");

		//add value sufix to string
		LastOperation.append(" [$");
		TWO_HEX_BYTES(add);
		LastOperation.append("]");

		//set operation target helpers
		LastTargetA = add;
		LastTargetB = OP_INVALID_TARGET;
		LastArgumentCount = 2;

		return false;
	}

	// Template method for the Absolute X addressing mode.
	template<OpcodeMethod OP>
	bool AbsoluteXMode()
	{
		//get next byte pair
		unsigned char a = getNextByte();
		unsigned char b = getNextByte();

		//convert to address
		unsigned short add = ((unsigned short)b) << 8 | a;

		//perform operation on address
		(*this.*OP)(Memory + ((add + Register[X]) % 0x10000));

		//add operation sufix to string
		LastOperation.append(" $");
		TWO_HEX_BYTES(add);
		LastOperation.append(",X");

		//add value sufix to string
		VALUE_SUFIX(Memory + ((add + Register[X]) % 0x10000));

		//set operation target helpers
		LastTargetA = (add + Register[X]) % 0x10000;
		LastTargetB = OP_INVALID_TARGET;
		LastArgumentCount = 2;

		//perform page boundary cross test
		if ((((add + Register[X]) % 0x10000) & 0xFF00) == (add & 0xFF00)) return false;
		else return true;
	}

	// Template method for the Absolute Y addressing mode.
	template<OpcodeMethod OP>
	bool AbsoluteYMode()
	{
		//get next byte pair
		unsigned char a = getNextByte();
		unsigned char b = getNextByte();

		//convert to address
		unsigned short add = ((unsigned short)b) << 8 | a;

		//perform operation on address
		(*this.*OP)(Memory + ((add + Register[Y]) % 0x10000));

		//add operation sufix to string
		LastOperation.append(" $");
		TWO_HEX_BYTES(add);
		LastOperation.append(",Y");

		//add value sufix to string
		VALUE_SUFIX(Memory + ((add + Register[Y]) % 0x10000));

		//set operation target helpers
		LastTargetA = (add + Register[Y]) % 0x10000;
		LastTargetB = OP_INVALID_TARGET;
		LastArgumentCount = 2;

		//perform page boundary cross test
		if ((((add + Register[Y]) % 0x10000) & 0xFF00) == (add & 0xFF00)) return false;
		else return true;
	}

	// Template method for the Zero-Page X addressing mode.
	template<OpcodeMethod OP>
	bool ZeroPageXMode()
	{
		//get zero-page address
		unsigned char add = getNextByte();

		//perform operation on modified address
		(*this.*OP)(Memory + ((add + Register[X]) % 0x100));

		//add operation sufix to string
		LastOperation.append(" $");
		ONE_HEX_BYTE(add);
		LastOperation.append(",X");

		//add value sufix to string
		VALUE_SUFIX(Memory + ((add + Register[X]) % 0x100));

		//set operation target helpers
		LastTargetA = (add + Register[X]) % 0x100;
		LastTargetB = OP_INVALID_TARGET;
		LastArgumentCount = 1;

		return false;
	}

	// Template method for the Zero-Page Y addressing mode.
	template<OpcodeMethod OP>
	bool ZeroPageYMode()
	{
		//get zero-page address
		unsigned char add = getNextByte();

		//perform operation on modified address
		(*this.*OP)(Memory + ((add + Register[Y]) % 0x100));

		//add operation sufix to string
		LastOperation.append(" $");
		ONE_HEX_BYTE(add);
		LastOperation.append(",Y");

		//add value sufix to string
		LastOperation.append(" [");
		LastOperation += Hexadecimal[(*(Memory + ((add + Register[Y]) % 0x100))) & 0xF];
		LastOperation += Hexadecimal[(*(Memory + ((add + Register[Y]) % 0x100))) & 0xF];
		LastOperation.append("]");

		//set operation target helpers
		LastTargetA = (add + Register[Y]) % 0x100;
		LastTargetB = OP_INVALID_TARGET;
		LastArgumentCount = 1;

		return false;
	}

	// Template method for the Indirect X addressing mode.
	template<OpcodeMethod OP>
	bool IndirectXMode()
	{
		//get zero-page address
		unsigned char addt = getNextByte();
		unsigned char addx = (addt + Register[X]);

		//retrieve the final address bytes
		unsigned char a = Memory[addx];
		unsigned char b = Memory[addx + 1];

		//convert to final address
		unsigned short add = ((unsigned short)b << 8) | a;

		//perform operation on modified address
		(*this.*OP)(Memory + add);

		//add operation sufix to string
		LastOperation.append(" ($");
		ONE_HEX_BYTE(addt);
		LastOperation.append(",X)");

		//add value sufix to string
		VALUE_SUFIX(Memory + add);

		//set operation target helpers
		LastTargetA = add;
		LastTargetB = OP_INVALID_TARGET;
		LastArgumentCount = 1;

		//perform page boundary cross test
		if ((add & 0xFF00) == 0) return false;
		else return true;
	}

	// Template method for the Indirect Y addressing mode.
	template<OpcodeMethod OP>
	bool IndirectYMode()
	{
		//get zero-page address
		unsigned char addt = getNextByte();
		
		//retrieve the final address bytes
		unsigned char a = Memory[addt];
		unsigned char b = Memory[(addt + 1) % 0x100];

		//convert to final address
		unsigned short adda = ((unsigned short)b) << 8 | a;

		//modify address
		unsigned short addb = (adda + Register[Y]) % 0x10000;

		//perform operation on modified address
		(*this.*OP)(Memory + addb);

		//add operation sufix to string
		LastOperation.append(" ($");
		ONE_HEX_BYTE(addt);
		LastOperation.append("),Y");

		//add value sufix to string
		VALUE_SUFIX(Memory + addb);

		//set operation target helpers
		LastTargetA = addb;
		LastTargetB = OP_INVALID_TARGET;
		LastArgumentCount = 1;

		//perform page boundary cross test
		if ((adda & 0xFF00) == (addb & 0xFF00)) return false;
		else return true;
	}

#pragma endregion

#pragma region Operations

	// Sign flag test condition
	#define SIGN_FLAG_TEST(X) setFlag(Sign, (X) & 0b10000000)

	// Zero flag test condition
	#define ZERO_FLAG_TEST(X) setFlag(Zero, (X) == 0)

	// Add with carry method.
	void op_ADC(unsigned char* arg);

	// Bitwise AND with accumulator method.
	void op_AND(unsigned char* arg);

	// Arithmetic shift left method.
	void op_ASL(unsigned char* arg);

	// Bit test method.
	void op_BIT(unsigned char* arg);

	// Branch method.
	template <ProcessorFlags F, bool STATE>
	void branch(unsigned char* arg)
	{
		//check if flag state matches expected value
		if (getFlag(F) == STATE)
		{
			//check if branch crossing would occur and adjust appropriately
			if (ProgramCounter >> 8 == (arg - Memory) >> 8)
				AdditionalProcessingTime += 1;
			else
				AdditionalProcessingTime += 2;

			//adjust program counter
			ProgramCounter = arg - Memory;
		}
	}

	// Branch opcode definitions.
	#define op_BPL branch<Sign, false>
	#define op_BMI branch<Sign, true>
	#define op_BVC branch<Overflow, false>
	#define op_BVS branch<Overflow, true>
	#define op_BCC branch<Carry, false>
	#define op_BCS branch<Carry, true>
	#define op_BNE branch<Zero, false>
	#define op_BEQ branch<Zero, true>

	// Break method.
	void op_BRK();

	// Compare register template method.
	template<ProcessorRegisters R>
	void compareRegister(unsigned char* arg)
	{
		//get compared value
		unsigned char val = *arg;

		//set or clear Carry flag
		setFlag(Carry, Register[R] >= val);

		//set or clear Zero flag
		setFlag(Zero, Register[R] == val);

		//set or clear Sign flag
		setFlag(Sign, (Register[R] - val) & 0b10000000); //NOTE: MIGHT BE INCORRECT, GOTTA CHECK LATER.
	}

	// Compare opcode definitions.
	#define op_CMP compareRegister<Accumulator>
	#define op_CPX compareRegister<X>
	#define op_CPY compareRegister<Y>

	// Decrement memory method.
	void op_DEC(unsigned char* arg);

	// Bitwise XOR method.
	void op_EOR(unsigned char* arg);

	// Flag state modification method.
	template <ProcessorFlags F, bool STATE>
	void setFlagState()
	{
		//set flag state
		setFlag(F, STATE);
	}

	//Flag state modification definitions.
	#define op_CLC setFlagState<Carry, false>
	#define op_SEC setFlagState<Carry, true>
	#define op_CLI setFlagState<Interrupt, false>
	#define op_SEI setFlagState<Interrupt, true>
	#define op_CLV setFlagState<Overflow, false>
	#define op_CLD setFlagState<Decimal, false>
	#define op_SED setFlagState<Decimal, true>

	// Increment memory method.
	void op_INC(unsigned char* arg);

	// Jump method.
	void op_JMP(unsigned char* arg);

	// Jump to subroutine method.
	void op_JSR(unsigned char* arg);

	// Load to register template method.
	template<ProcessorRegisters R>
	void loadRegister(unsigned char* arg)
	{
		//load value to register
		Register[R] = *arg;

		//perform flag tests
		ZERO_FLAG_TEST(*arg);
		SIGN_FLAG_TEST(*arg);
	}

	// Load to register definitions.
	#define op_LDA loadRegister<Accumulator>
	#define op_LDX loadRegister<X>
	#define op_LDY loadRegister<Y>

	// Logical shift right method.
	void op_LSR(unsigned char* arg);

	// No operation method.
	void op_NOP();

	// Bitwise OR with accumulator method.
	void op_ORA(unsigned char* arg);

	// Register transfer method.
	template <ProcessorRegisters SOURCE, ProcessorRegisters TARGET>
	void transferRegister()
	{
		//copy value
		Register[TARGET] = Register[SOURCE];

		//perform flag tests
		ZERO_FLAG_TEST(Register[TARGET]);
		SIGN_FLAG_TEST(Register[TARGET]);
	}

	// Register transfer definitions.
	#define op_TAX transferRegister<Accumulator, X>
	#define op_TXA transferRegister<X, Accumulator>
	#define op_TAY transferRegister<Accumulator, Y>
	#define op_TYA transferRegister<Y, Accumulator>

	// Register increment method.
	template <ProcessorRegisters R>
	void incrementRegister()
	{
		//increment value
		Register[R]++;

		//perform flag tests
		ZERO_FLAG_TEST(Register[R]);
		SIGN_FLAG_TEST(Register[R]);
	}

	// Register increment definitions.
	#define op_INX incrementRegister<X>
	#define op_INY incrementRegister<Y>

	// Register decrement method.
	template <ProcessorRegisters R>
	void decrementRegister()
	{
		//decrement value
		Register[R]--;

		//perform flag tests
		ZERO_FLAG_TEST(Register[R]);
		SIGN_FLAG_TEST(Register[R]);
	}

	// Register decrement definitions.
	#define op_DEX decrementRegister<X>
	#define op_DEY decrementRegister<Y>

	// Rotate left method.
	void op_ROL(unsigned char* arg);

	// Rotate right method.
	void op_ROR(unsigned char* arg);

	// Return from interrupt method.
	void op_RTI();

	// Return from subroutine method.
	void op_RTS();

	// Substract with carry method.
	void op_SBC(unsigned char* arg);

	// Transfer X register to stack pointer method.
	void op_TXS();

	// Transfer stack pointer to X register method.
	void op_TSX();

	// Pushes specified value onto the stack.
	void pushValue(unsigned char val);

	// Pulls and returns the topmost value of the stack.
	unsigned char pullValue();

	// Pushes the program counter value onto the stack.
	void pushProgramCounter();

	// Pulls two bytes off the stack and pushes them into the program counter.
	void pullProgramCounter();

	// Push Accumulator value method.
	void op_PHA();

	// Pull Accumulator value method.
	void op_PLA();

	// Push processor status method.
	void op_PHP();

	// Pull processor status method.
	void op_PLP();

	// Store register value template method.
	template<ProcessorRegisters R>
	void storeRegister(unsigned char* arg)
	{
		//copy value
		*arg = Register[R];
	}

	// Store register value definitions.
	#define op_STA storeRegister<Accumulator>
	#define op_STX storeRegister<X>
	#define op_STY storeRegister<Y>

#pragma endregion

#pragma region Opcode Tables

	// Opcode function table, stores method pointers for all defined (and undefined) opcodes.
	static const AddressedOpcodeMethod const OpcodeFunctionTable[256];

	// Opcode length table, stores execution lengths of each of the opcodes. Negative times should be inverted to positive; they represents operations which times are incremented by 1 if crossing page boundaries.
	static const char OpcodeTimeTable[256];

	// Opcode name table, stores each of the opcode names.
	static const std::string const OpcodeNameTable[256];

#pragma endregion

#pragma region Rendering Helpers

	// Hexadecimal character translation table.
	static const char Hexadecimal[16];

	// String representing the last operation's notation.
	std::string LastOperation = "";

	// Last performed OP's location.
	unsigned short LastProgramCounter = 0;

	// Last performed OP's argument byte count.
	unsigned char LastArgumentCount = 0;

	// Last performed OP's first target.
	int LastTargetA;

	// Last performed OP's second target.
	int LastTargetB;

#pragma endregion

	// Stores the number of additional cycles required to wait before the operation should complete.
	char AdditionalProcessingTime = 0;

public:

#pragma region Getters/Setters

	// Returns an std::string representation of the previously executed command.
	std::string getLastOperation();

	int getLastTargetA();
	int getLastTargetB();
	unsigned short getLastProgramCounter();
	unsigned char getLastArgumentCount();




	unsigned char getMemoryCell(unsigned short index);
	void setMemoryCell(unsigned short index, unsigned char val);

	unsigned char getRegisterA();
	void setRegisterA(unsigned char val);

	unsigned char getRegisterX();
	void setRegisterX(unsigned char val);

	unsigned char getRegisterY();
	void setRegisterY(unsigned char val);

	unsigned char getStackPointer();
	void setStackPointer(unsigned char val);

	unsigned short getProgramCounter();
	void setProgramCounter(unsigned short val);

	unsigned short getVectorNMI();
	void setVectorNMI(unsigned short val);

	unsigned short getVectorReset();
	void setVectorReset(unsigned short val);

	unsigned short getVectorBRK();
	void setVectorBRK(unsigned short val);

	bool getFlagSign();
	void setFlagSign(bool val);

	bool getFlagOverflow();
	void setFlagOverflow(bool val);

	bool getFlagBlank();
	void setFlagBlank(bool val);

	bool getFlagBreak();
	void setFlagBreak(bool val);

	bool getFlagDecimal();
	void setFlagDecimal(bool val);

	bool getFlagInterrupt();
	void setFlagInterrupt(bool val);

	bool getFlagZero();
	void setFlagZero(bool val);

	bool getFlagCarry();
	void setFlagCarry(bool val);

#pragma endregion

	// Performs a single processor cycle, skipping any empty timing cycles imposed by the previous command if timingOn is set to 'false'. Returns true if an operation was performed, false otherwise.
	bool processCycle(bool timingOn = true);

	// Resets the processor.
	void reset();

	// Triggers a maskable interrupt (IRQ).
	void triggerIRQ();

	// Starts a non-maskable interrupt signal (NMI).
	void startNMI();

	// Ends a non-maskable interrupt signal (NMI).
	void endNMI();

	// Loads provided std::istream into the processor memory as a raw binary file.
	bool loadFromBinStream(std::istream& mem);

	// Loads provided std::istream into the processor memory as a hexadecimal program file.
	bool loadFromHexStream(std::istream& mem);

	// Default constructor, initializes the emulator.
	ProcMos6502();

	// Constructor, initializes the emulator and loads in memory contents from the provided std::istream.
	//ProcMos6502(std::istream& mem);
};