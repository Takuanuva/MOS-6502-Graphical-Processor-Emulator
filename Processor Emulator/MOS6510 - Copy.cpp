#include "MOS6510.h"

namespace MOS6500Fam
{
	// Represents an emulator of a MOS6502 processor.
	class ProcMos6502
	{


		// Opcode method pointer signature.
		typedef void (ProcMos6502::*OpcodeMethod)(unsigned char *);

		// Implied mode only opcode method pointer signature.
		typedef void (ProcMos6502::*ImpliedOpcodeMethod)(void);



		// Template method for the Accumulator addressing mode.
		template<OpcodeMethod OP>
		bool AccumulatorMode()
		{
			//perform operation on accumulator
			*Op(Accumulator);
		}

		// Template method for the Immediate addressing mode.
		template<OpcodeMethod OP>
		bool ImmediateMode()
		{
			//get next byte
			char bep = getNextByte();

			//perform operation on retrieved value
			*Op(bep);
		}

		// Template method for the Implied addressing mode.
		template<ImpliedOpcodeMethod OP>
		bool ImpliedMode()
		{
			//perform operation
			*Op();
		}

		// Acts as Implied mode, but prints a warning message if called.
		template<ImpliedOpcodeMethod OP>
		bool ImpliedDebugMode()
		{
			TODO: ADD WARNING

			//perform operation
			*Op();
		}

		// Template method for the Relative addressing mode.
		template<OpcodeMethod OP>
		bool RelativeMode()
		{
			//get next byte
			char bep = getNextByte();

			//get current position
			unsigned short bop = getOpAddress();

			//modify position
			bop += bep;

			//perform operation on modified address
			*Op(&bop);
		}

		// Template method for the Absolute addressing mode.
		template<OpcodeMethod OP>
		bool AbsoluteMode()
		{
			//get next byte pair
			char a = getNextByte();
			char b = getNextByte();

			//convert to address
			unsigned short add = ((unsigned short)b) << 8 | a;

			//perform operation on address
			*Op(Memory + add);
		}

		// Template method for the Zero-Page addressing mode.
		template<OpcodeMethod OP>
		bool ZeroPageMode()
		{
			//get zero-page address
			unsigned char bep = getNextByte();

			//perform operation on retrieved address
			*Op(memory + bep);
		}

		// Template method for the Indirect addressing mode.
		template<OpcodeMethod OP>
		bool IndirectMode()
		{
			//get next byte pair
			char a = getNextByte();
			char b = getNextByte();

			//convert to preliminary address
			unsigned short add = ((unsigned short)b) << 8 | a;

			//retrieve the final address bytes
			a = Memory[add];
			b = Memory[add + 1];

			//convert to final address
			add = ((unsigned short)b) << 8 | a;

			//perform operation on address
			*Op(add);
		}

		// Template method for the Absolute X addressing mode.
		template<OpcodeMethod OP>
		bool AbsoluteXMode()
		{
			//get next byte pair
			char a = getNextByte();
			char b = getNextByte();

			//convert to address
			unsigned short add = ((unsigned short)b) << 8 | a;

			//perform operation on address
			*Op(Memory + add + &XRegister);
		}

		// Template method for the Absolute Y addressing mode.
		template<OpcodeMethod OP>
		bool AbsoluteYMode()
		{
			//get next byte pair
			char a = getNextByte();
			char b = getNextByte();

			//convert to address
			unsigned short add = ((unsigned short)b) << 8 | a;

			//perform operation on address
			*Op(Memory + add + &YRegister);
		}

		// Template method for the Zero-Page X addressing mode.
		template<OpcodeMethod OP>
		bool ZeroPageXMode()
		{
			//get zero-page address
			unsigned char add = getNextByte();

			//perform operation on modified address
			*Op(Memory + add + &XRegister);
		}

		// Template method for the Indirect X addressing mode.
		template<OpcodeMethod OP>
		bool IndirectXMode()
		{
			//get zero-page address
			unsigned char add = getNextByte();

			//modify address
			add += &XRegister;

			//retrieve the final address bytes
			a = Memory[add];
			b = Memory[add + 1];

			//convert to final address
			add = ((unsigned short)b) << 8 | a;

			//perform operation on modified address
			*Op(Memory + add);
		}

		// Template method for the Indirect Y addressing mode.
		template<OpcodeMethod OP>
		bool IndirectYMode()
		{
			//get zero-page address
			unsigned char add = getNextByte();

			//retrieve the final address bytes
			a = Memory[add];
			b = Memory[add + 1];

			//convert to final address
			add = ((unsigned short)b) << 8 | a;

			//modify address
			add += &XRegister;

			//perform operation on modified address
			*Op(Memory + add);
		}

		// Processor's addressable memory.
		unsigned char Memory[0x10000];

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
			Carry = 0,
			Zero = 1,
			Interrupt = 2,
			Decimal = 3,
			Break = 4,
			Overflow = 6,
			Sign = 7
		};

		// Gets the current state of the given flag.
		bool getFlag(ProcessorFlags flag)
		{
			return Register[Status] && (1 << flag);
		}

		// Sets the current state of the given flag.
		void setFlag(ProcessorFlags flag, bool value)
		{
			if (value)
				Register[Status] |= (1 << flag);
			else
				Register[Status] &= 0xff ^ (1 << flag);

		}




		// Program counter.
		unsigned short ProgramCounter;

		// Gets the next byte address and increments the program counter (with or without wraparound).
		unsigned char* getNextByte(bool wraparound)
		{
			//get byte address
			unsigned char* address = Memory + ProgramCounter;

			//increment program counter
			ProgramCounter++;

			//check if wraparound is enabled
			if (wraparound)
			{
				//check if second byte is 0 (implying overflow)
				if (ProgramCounter & 0xFF == 0)
				{
					//wraparound to the start of the page
					ProgramCounter -= 0x100;
				}
			}
		}

		// Stack pointer.
		unsigned char StackPointer;

		/*
		// Gets the next byte within memory, allows for page looping if desired.
		char getNextByte(bool pageLoop)
		{
			//TODO: IMPLEMENT!
		}

		// Gets the address of the current instruction.
		unsigned short getOpAddress()
		{
			//TODO: IMPLEMENT!
		}
		*/









		// Stores the amount of time required to wait before the operation should complete.
		char AdditionalProcessingTime = 0;



		// Performs a single processor cycle.
		void processCycle()
		{
			//check if the processor should wait
			if (AdditionalProcessingTime > 0)
			{
				//decrement the wait time
				AdditionalProcessingTime--;

				//stop cycle
				return;
			}

			//get next operation
			unsigned char opcode = Memory[ProgramCounter];

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
			bool pageCross = (*OpcodeFunctionTable[opcode])();

			//increment time by 1 if page bounds were crossed and time was negative
			if (negativeTime && pageCross) AdditionalProcessingTime++;

			//decrease the wait time by 1 (adjusting for current cycle)
			AdditionalProcessingTime--;
		}





		// Add with carry method.
		void op_ADC(unsigned char* arg)
		{
			//check if addition should be performed in BCD mode
			if (getFlag(Decimal))
			{
				TODO: IMPLEMENT!
			}
			else
			{
				//store argument value
				unsigned char val = *arg;

				//perform addition
				unsigned short bep = (unsigned short)Register[Accumulator] + (unsigned short)val;

				//add carry (if set)
				if (getFlag(Carry)) bep++;

				//set or clear the Sign flag
				if (bep > 127 && bep < 256)
					setFlag(Sign, true);
				else
					setFlag(Sign, false);

				//set or clear the Overflow flag
				if ((Register[Accumulator] < 128 && val < 128 && bep > 127) ||
					(Register[Accumulator] > 127 && val > 127 && bep - 0x100 < 128))
					setFlag(Overflow, true);
				else
					setFlag(Overflow, false);

				//set or clear the Zero flag
				if (bep == 0)
					setFlag(Zero, true);
				else
					setFlag(Zero, false);

				//set or clear the Carry flag
				if (bep > 0xFF)
					setFlag(Carry, true);
				else
					setFlag(Carry, false);

				//push calculated value to accumulator
				Register[Accumulator] = (unsigned char)bep;
			}
		}

		// Bitwise AND with accumulator method.
		void op_AND(unsigned char* arg)
		{
			//perform operation
			Register[Accumulator] &= *arg;

			//set or clear the Sign flag
			if (Accumulator && 0b10000000)
				setFlag(Sign, true);
			else
				setFlag(Sign, false);

			//set or clear the Zero flag
			if (Accumulator == 0)
				setFlag(Zero, true);
			else
				setFlag(Zero, false);
		}

		// Arithmetic shift left method.
		void op_ASL(unsigned char* arg)
		{

			//set or clear the Carry flag
			if (*arg && 0b10000000)
				setFlag(Carry, true);
			else
				setFlag(Carry, false);

			//perform operation
			*arg << 1;

			//set or clear the Sign flag
			if (*arg && 0b10000000)
				setFlag(Sign, true);
			else
				setFlag(Sign, false);

			//set or clear the Zero flag
			if (*arg == 0)
				setFlag(Zero, true);
			else
				setFlag(Zero, false);
		}

		// Bit test method.
		void op_BIT(unsigned char* arg)
		{
			//set or clear the Zero flag
			if (Register[Accumulator] & *arg == 0)
				setFlag(Zero, true);
			else
				setFlag(Zero, false);

			//copy bits 6 and 7 of the value at address into status register
			Register[Status] = (Register[Status] & 0b00111111) || (*arg & 0b11000000);
		}

		// Branch method.
		template <ProcessorFlags F, bool STATE>
		void branch()
		{
			//check flag
			if (getFlag(F) == STATE)
			{
				TODO: IMPLEMENT!
			}
		}

		// Branch opcode definitions.
		const ImpliedOpcodeMethod op_BPL = branch<Sign, false>;
		const ImpliedOpcodeMethod op_BMI = branch<Sign, true>;
		const ImpliedOpcodeMethod op_BVC = branch<Overflow, false>;
		const ImpliedOpcodeMethod op_BVS = branch<Overflow, true>;
		const ImpliedOpcodeMethod op_BCC = branch<Carry, false>;
		const ImpliedOpcodeMethod op_BCS = branch<Carry, true>;
		const ImpliedOpcodeMethod op_BNE = branch<Zero, false>;
		const ImpliedOpcodeMethod op_BEQ = branch<Zero, true>;

		// Break method.
		void op_BRK()
		{
			TODO: IMPLEMENT!
		}

		// Compare register template method.
		template<ProcessorRegisters R>
		void compareRegister(unsigned char* arg)
		{
		TODO: IMPLEMENT!
		}

		// Decrement memory method.
		void op_DEC(unsigned char* arg)
		{
		TODO: IMPLEMENT!
		}

		// Bitwise XOR method.
		void op_EOR(unsigned char* arg)
		{
		TODO: IMPLEMENT!
		}

		// Flag state modification method
		template <ProcessorFlags F, bool STATE>
		void flagState()
		{
			//set flag state
			setFlag(F, STATE);
		}

		// Increment memory method.
		void op_INC(unsigned char* arg)
		{
		TODO: IMPLEMENT!
		}

		// Jump method.
		void op_JMP(unsigned char* arg)
		{
		TODO: IMPLEMENT!
		}

		// Jump to subroutine method.
		void op_JSR(unsigned char* arg)
		{
		TODO: IMPLEMENT!
		}

		// Load to register template method.
		template<ProcessorRegisters R>
		void loadRegister(unsigned char* arg)
		{
		TODO: IMPLEMENT!
		}

		// Logical shift right method.
		void op_LSR(unsigned char* arg)
		{
		TODO: IMPLEMENT!
		}

		// No operation method.
		void op_NOP()
		{ }

		// Bitwise OR with accumulator method.
		void op_ORA(unsigned char* arg)
		{
			TODO: IMPLEMENT!
		}

		// Register transfer method.
		template <ProcessorRegisters SOURCE, ProcessorRegisters TARGET>
		void transferRegister()
		{
			//copy value
			Register[TRAGET] = Register[SOURCE];

			//set or clear the Sign flag
			if (Register[TRAGET] && 0b10000000)
				setFlag(Sign, true);
			else
				setFlag(Sign, false);

			//set or clear the Zero flag
			if (Register[TRAGET] == 0)
				setFlag(Zero, true);
			else
				setFlag(Zero, false);

			return 2;
		}

		// Register increment method.
		template <ProcessorRegisters R>
		void incrementRegister()
		{
			//increment value
			Register[R]++;

			//set or clear the Sign flag
			if (Register[R] && 0b10000000)
				setFlag(Sign, true);
			else
				setFlag(Sign, false);

			//set or clear the Zero flag
			if (Register[R] == 0)
				setFlag(Zero, true);
			else
				setFlag(Zero, false);

			return 2;
		}

		// Register decrement method.
		template <ProcessorRegisters R>
		void decrementRegister()
		{
			//decrement value
			Register[R]--;

			//set or clear the Sign flag
			if (Register[R] && 0b10000000)
				setFlag(Sign, true);
			else
				setFlag(Sign, false);

			//set or clear the Zero flag
			if (Register[R] == 0)
				setFlag(Zero, true);
			else
				setFlag(Zero, false);

			return 2;
		}

		// Rotate left method.
		void op_ROL(unsigned char* arg)
		{
		TODO: IMPLEMENT!
		}

		// Rotate right method.
		void op_ROR(unsigned char* arg)
		{
		TODO: IMPLEMENT!
		}

		// Return from interrupt method.
		void op_RTI()
		{
		TODO: IMPLEMENT!
		}

		// Return from subroutine method.
		void op_RTS()
		{
		TODO: IMPLEMENT!
		}

		// Substract with carry method.
		void op_SBC(unsigned char* arg)
		{
		TODO: IMPLEMENT!
		}

		// Transfer X register to stack pointer method.
		void op_TXS()
		{
			//copy value
			StackPointer = Register[X];
		}

		// Transfer stack pointer to X register method.
		void op_TSX()
		{
			//copy value
			Register[X] = StackPointer;

			return 2;
		}

		// Push Accumulator value method.
		void op_PHA()
		{
		TODO: IMPLEMENT!

			return 3;
		}

		// Pull Accumulator value method.
		void op_PLA()
		{
		TODO: IMPLEMENT!

		}

		// Push processor status method.
		void op_PHP()
		{
		TODO: IMPLEMENT!

			return 3;
		}

		// Pull processor status method.
		void op_PLP()
		{
		TODO: IMPLEMENT!

			return 4;
		}

		// Store register value template method.
		template<ProcessorRegisters R>
		void storeRegister(unsigned char* arg)
		{
			//copy value
			*arg = Register[R];
		}




		// Addressing mode template method pointer signature.
		typedef void (ProcMos6502::AddressedOpcodeMethod)(void);

		// Opcode function table, stores method pointers for all defined (and undefined) opcodes.
		AddressedOpcodeMethod* OpcodeFunctionTable[256] =
		{/*	0							1							2							3							4							5							6							7							8							9							A							B							C							D							E							F */
	/* 0 */	ImpliedMode<op_BRK>,		IndirectXMode<op_ORA>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ZeroPageMode<op_ORA>,		ZeroPageMode<op_ASL>,		ImpliedDebugMode<op_NOP>,	ImpliedMode<op_PHP>,		ImmediateMode<op_ORA>,		AccumulatorMode<op_ASL>,	ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	AbsoluteMode<op_ORA>,		AbsoluteMode<op_ASL>,		ImpliedDebugMode<op_NOP>,
	/* 1 */	ImpliedMode<op_BPL>,		IndirectYMode<op_ORA>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ZeroPageXmode<op_ORA>,		ZeroPageXMode<op_ASL>,		ImpliedDebugMode<op_NOP>,	ImpliedMode<op_CLC>,		AbsoluteYMode<op_ORA>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	AbsoluteXMode<op_ORA>,		AbsoluteXMode<op_ASL>,		ImpliedDebugMode<op_NOP>,
	/* 2 */	AbsoluteMode<op_JSR>,		IndirectXMode<op_AND>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ZeroPageMode<op_BIT>,		ZeroPageMode<op_AND>,		ZeroPageMode<op_ROL>,		ImpliedDebugMode<op_NOP>,	ImpliedMode<op_PLP>,		ImmediateMode<op_AND>,		AccumulatorMode<op_ROL>,	ImpliedDebugMode<op_NOP>,	AbsoluteMode<op_BIT>,		AbsoluteMode<op_AND>,		AbsoluteMode<op_ROL>,		ImpliedDebugMode<op_NOP>,
	/* 3 */	ImpliedMode<op_BMI>,		IndirectYMode<op_AND>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ZeroPageXMode<op_AND>,		ZeroPageXMode<op_ROL>,		ImpliedDebugMode<op_NOP>,	ImpliedMode<op_SEC>,		AbsoluteYMode<op_AND>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	AbsoluteXMode<op_AND>,		AbsoluteXMode<op_ROL>,		ImpliedDebugMode<op_NOP>,
	/* 4 */	ImpliedMode<op_RTI>,		IndirectXMode<op_EOR>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ZeroPageMode<op_EOR>,		ZeroPageMode<op_LSR>,		ImpliedDebugMode<op_NOP>,	ImpliedMode<op_PHA>,		ImmediateMode<op_EOR>,		AccumulatorMode<op_LSR>,	ImpliedDebugMode<op_NOP>,	AbsoluteMode<op_JMP>,		AbsoluteMode<op_EOR>,		AbsoluteMode<op_LSR>,		ImpliedDebugMode<op_NOP>,
	/* 5 */	ImpliedMode<op_BVC>,		IndirectYMode<op_EOR>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ZeroPageXMode<op_EOR>,		ZeroPageXMode<op_LSR>,		ImpliedDebugMode<op_NOP>,	ImpliedMode<op_CLI>,		AbsoluteYMode<op_EOR>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	AbsoluteXmode<op_EOR>,		AbsoluteXMode<op_LSR>,		ImpliedDebugMode<op_NOP>,
	/* 6 */	ImpliedMode<op_RTS>,		IndirectXMode<op_ADC>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ZeroPageMode<op_ADC>,		ZeroPageMode<op_ROR>,		ImpliedDebugMode<op_NOP>,	ImpliedMode<op_PLA>,		ImmediateMode<op_ADC>,		AccumulatorMode<op_ROR>,	ImpliedDebugMode<op_NOP>,	IndirectMode<op_JMP>,		AbsoluteMode<op_ADC>,		AbsoluteMode<op_ROR>,		ImpliedDebugMode<op_NOP>,
	/* 7 */	ImpliedMode<op_BVS>,		IndirectYMode<op_ADC>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ZeroPageXMode<op_ADC>,		ZeroPageXMode<op_ROR>,		ImpliedDebugMode<op_NOP>,	ImpliedMode<op_SEI>,		AbsoluteYMode<op_ADC>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	AbsoluteXMode<op_ADC>,		AbsoluteXMode<op_ROR>,		ImpliedDebugMode<op_NOP>,
	/* 8 */	ImpliedDebugMode<op_NOP>,	IndirectXMode<op_STA>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ZeroPageMode<op_STY>,		ZeroPageMode<op_STA>,		ZeroPageMode<op_STX>,		ImpliedDebugMode<op_NOP>,	ImpliedMode<op_DEY>,		ImpliedDebugMode<op_NOP>,	ImpliedMode<op_TXA>,		ImpliedDebugMode<op_NOP>,	AbsoluteMode<op_STY>,		AbsoluteMode<op_STA>,		AbsoluteMode<op_STX>,		ImpliedDebugMode<op_NOP>,
	/* 9 */	ImpliedMode<op_BCC>,		IndirectYMode<op_STA>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ZeroPageXMode<op_STY>,		ZeroPageXMode<op_STA>,		ZeroPageYMode<op_STX>,		ImpliedDebugMode<op_NOP>,	ImpliedMode<op_TYA>,		AbsoluteYMode<op_STA>,		ImpliedMode<op_TXS>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	AbsoluteXMode<op_STA>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,
	/* A */	ImmediateMode<op_LDY>,		IndirectXMode<op_LDA>,		ImmediateMode<op_LDX>,		ImpliedDebugMode<op_NOP>,	ZeroPageMode<op_LDY>,		ZeroPageMode<op_LDA>,		ZeroPageMode<op_LDX>,		ImpliedDebugMode<op_NOP>,	ImpliedMode<op_TAY>,		ImmediateMode<op_LDA>,		ImpliedMode<op_TAX>,		ImpliedDebugMode<op_NOP>,	AbsoluteMode<op_LDY>,		AbsoluteMode<op_LDA>,		AbsoluteMode<op_LDX>,		ImpliedDebugMode<op_NOP>,
	/* B */	ImpliedMode<op_BCS>,		IndirectYMode<op_LDA>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ZeroPageXMode<op_LDY>,		ZeroPageXMode<op_LDA>,		ZeroPageYMode<op_LDX>,		ImpliedDebugMode<op_NOP>,	ImpliedMode<op_CLV>,		AbsoluteYMode<op_LDA>,		ImpliedMode<op_TSX>,		ImpliedDebugMode<op_NOP>,	AbsoluteXMode<op_LDY>,		AbsoluteXMode<op_LDA>,		AbsoluteYMode<op_LDX>,		ImpliedDebugMode<op_NOP>,
	/* C */	ImmediateMode<op_CPY>,		IndirectXMode<op_CMP>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ZeroPageMode<op_CPY>,		ZeroPageMode<op_CMP>,		ZeroPageMode<op_DEC>,		ImpliedDebugMode<op_NOP>,	ImpliedMode<op_INY>,		ImmediateMode<op_CMP>,		ImpliedMode<op_DEX>,		ImpliedDebugMode<op_NOP>,	AbsoluteMode<op_CPY>,		AbsoluteMode<op_CMP>,		AbsoluteMode<op_DEC>,		ImpliedDebugMode<op_NOP>,
	/* D */	ImpliedMode<op_BNE>,		IndirectYMode<op_CMP>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ZeroPageXMode<op_CMP>,		ZeroPageXMode<op_DEC>,		ImpliedDebugMode<op_NOP>,	ImpliedMode<op_CLD>,		AbsoluteYMode<op_CMP>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	AbsoluteXMode<op_CMP>,		AbsoluteXMode<op_DEC>,		ImpliedDebugMode<op_NOP>,
	/* E */	ImmediateMode<op_CPX>,		IndirectXMode<op_SBC>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ZeroPageMode<op_CPX>,		ZeroPageMode<op_SBC>,		ZeroPageMode<op_INC>,		ImpliedDebugMode<op_NOP>,	ImpliedMode<op_INX>,		ImmediateMode<op_SBC>,		ImpliedMode<op_NOP>,		ImpliedDebugMode<op_NOP>,	AbsoluteMode<op_CPX>,		AbsoluteMode<op_SBC>,		AbsoluteMode<op_INC>,		ImpliedDebugMode<op_NOP>,
	/* F */	ImpliedMode<op_BEQ>,		IndirectYMode<op_SBC>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ZeroPageXMode<op_SBC>,		ZeroPageXMode<op_INC>,		ImpliedDebugMode<op_NOP>,	ImpliedMode<op_SED>,		AbsoluteYMode<op_SBC>,		ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	ImpliedDebugMode<op_NOP>,	AbsoluteXMode<op_SBC>,		AbsoluteXMode<op_INC>,		ImpliedDebugMode<op_NOP>,
		};

		TODO: ADJUST FOR BRANCH METHODS!;
		// Opcode length table, stores execution lengths of each of the opcodes. Negative times should be inverted to positive; they represents operations which times are incremented by 1 if crossing page boundaries.
		char OpcodeTimeTable[256] =
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
	};
}