#include "dcpu-16.h"

#include "stdafx.h"
#include<iostream>
#include <math.h>
#include <fstream>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

CPU::CPU()
{
	frequency = 100000;

	halt = false;

	cycle = 0;
	cyclesTaken = 0;

	//set everything to 0
	//registers
	memset(reg,0,sizeof(reg));
	memset(memory,0,sizeof(memory));


	skipNextInstruction = false;


	interruptQueueInUse = false;


	/*


	//write current memory to "out" file
	ofstream outFile( "out", ios::binary);

	for (int i = 0; i< 100; i++)
	{
		unsigned short data = toLittleEndian(fromWord(memory[i]));
		outFile.write((char *)(&data), sizeof(data));
	}

	outFile.close();

	
	*/
}

int CPU::run()
{
	int error = -1;

	if (cyclesTaken > 1) //if last instruction took many cycles, let those cycles be used up
	{
		cyclesTaken--;
		cycle++;
	}
	else if (!halt) //when all the cycles of the last instruction are done, do next instruction
	{
		cyclesTaken = 0;

		if (!skip())
		{
			//read instruction from memory and compute it (this will also increase cyclesTaken)
			error = computeInstruction();

			interruptSoftwareTriggerQueue(); //if interrupts are set to be triggered and we have a queue of them, trigger the next one in the queue

		}


		if (cyclesTaken == 0) cyclesTaken = 1; //minimum cycle count has to be 1

		cycle++; //increase total cycle count

	}


	return error; //return error code from Read()
}

//read instruction and compute depending on opcode
int CPU::computeInstruction()
{
	int error = -1;

	//read word from memory at PC, then increase PC
	unsigned short word = memory[reg[regPC]++];

	//read opcode
	unsigned short op;

	//read value a and value b
	unsigned short A;
	unsigned short B;

	getOpValues(word, op, A, B);

	error = computeOp(op, A, B); //compute

	return error;
}

void CPU::getOpValues(unsigned short word, unsigned short &op, unsigned short &a, unsigned short &b)
{
	op = word&0x1f; //first 5 bits
	b = ((word>>5)&0x1f); //next 5 bits
	a = ((word>>10)&0x3f); //next (last) 6 bits

	//if (op | a| b != 0) printf("PC: %i, op: %x, b: %x, a: %x\n", reg[regPC], op, b, a);
}

unsigned short CPU::makeInstruction(unsigned short op, unsigned short b, unsigned short a)
{
	unsigned short instruction = op&0x1f;
	instruction += (b&0x1f)<<5;
	instruction += (a&0x3f)<<10;

	return instruction;
}

int CPU::computeOp(unsigned short op, unsigned short A, unsigned short B)
{
	int error = -1;

	unsigned short result;


	//extended opcodes
	if (op == opsEXT) // - | 0x00 | n/a
	{
		unsigned short *a = computeValue(A, false, true); //only use b for values as a is used for the extended opcode

		switch(B)
		{
			//reserved
			case opsSpecialEXT: // - | 0x00 | n/a   | reserved for future expansion
				halt = true;
				break;

			case JSR: // 3 | 0x01 | JSR a | pushes the address of the next instruction to the stack, then sets PC to a
				incCyclesTaken(3);
				memory[--reg[regSP]] = reg[regPC];
				reg[regPC] = *a;
				break;

			case HCF: //  9 | 0x07 | HCF a | use sparingly
				incCyclesTaken(9);
				catchFire();
				break;

			case INT: // 4 | 0x08 | INT a | triggers a software interrupt with message a
				incCyclesTaken(4);
				interruptSoftware(*a);
				break;

			case IAG: // 1 | 0x09 | IAG a | sets a to IA 
				incCyclesTaken(1);
				*a = reg[regIA];
				break;

			case IAS: // 1 | 0x0a | IAS a | sets IA to a
				incCyclesTaken(1);
				reg[regIA] = *a;
				break;

			case RFI: // 3 | 0x0b | RFI a | disables interrupt queueing, pops A from the stack, then pops PC from the stack
				incCyclesTaken(3);
				interruptQueueInUse = false;
				reg[regA] = memory[reg[regSP]++];
				reg[regPC] = memory[reg[regSP]++];
				break;

			case IAQ: // 2 | 0x0c | IAQ a | if a is nonzero, interrupts will be added to the queue instead of triggered. if a is zero, interrupts will be triggered as normal again
			{
				incCyclesTaken(2);
				interruptQueueInUse = *a != 0;
				break;
			}

			case HWN: // 2 | 0x10 | HWN a | sets a to number of connected hardware devices
				incCyclesTaken(2);
				*a = computer->getHardwareCount();
				break;

			case HWQ: // 4 | 0x11 | HWQ a | sets A, B, C, X, Y registers to information about hardware a. A+(B<<16) is a 32 bit word identifying the hardware id. C is the hardware version. X+(Y<<16) is a 32 bit word identifying the manufacturer
				incCyclesTaken(4);
				computer->getHardwareInfo(*a, reg[regA], reg[regB], reg[regC], reg[regX], reg[regY]);
				break;

			case HWI: // 4+| 0x12 | HWI a | sends an interrupt to hardware a
				incCyclesTaken(4);
				computer->interruptHardware(*a);
				break;
		}
	}

	else
	{
		unsigned short *a = computeValue(A, false, true);
		unsigned short *b = computeValue(B, false, false);

			switch(op)
			{
				
			case SET: // 1 | 0x01 | SET b, a | sets b to a
				incCyclesTaken(1);
				*b = *a;
				break;

				case ADD: // 2 | 0x02 | ADD b, a | sets b to b+a, sets EX to 0x0001 if there's an overflow, 0x0 otherwise
				{
					incCyclesTaken(2);
					result = *b + *a;
					reg[regEX] = (*b > 0xFFFF - *a) ? 0x0001 : 0x0000;
					*b = result;
					break;
				}

				case SUB: // 2 | 0x03 | SUB b, a | sets b to b-a, sets EX to 0xffff if there's an underflow, 0x0 otherwise
				{
					incCyclesTaken(2);
					result = *b - *a;
					reg[regEX] = (*b < *a) ? 0xFFFF : 0x0000;
					*b = result;
					break;
				}

				case MUL: //2 | 0x04 | MUL b, a | sets b to b*a, sets EX to ((b*a)>>16)&0xffff (treats b, a as unsigned)
					incCyclesTaken(2);
					result = (*b) * (*a);
					reg[regEX] = ((*b)>>WORD_LENGTH)&0xFFFF;
					*b = result;
					break;

				case MLI: //2 | 0x05 | MLI b, a | like MUL, but treat b, a as signed
					incCyclesTaken(2);
					result = (short)(*b) * (short)(*a);
					reg[regEX] = (((short)*b)>>WORD_LENGTH)&0xFFFF;
					*b = result;
					break;

				case DIV: // 3 | 0x06 | DIV b, a | sets b to b/a, sets EX to ((b<<16)/a)&0xffff. if a==0, sets b and EX to 0 instead. (treats b, a as unsigned)
				{
					incCyclesTaken(3);
					if(*a == 0)
					{
						*b = 0x0;
						reg[regEX] = 0x0;
					}
					else
					{
						result = *b / *a;
						reg[regEX] = (((*b)<<WORD_LENGTH)/(*a))&0xFFFF;
						*b = result;
					}
					break;
				}

				case DVI: // 3 | 0x07 | DVI b, a | like DIV, but treat b, a as signed. Rounds towards 0
				{
					incCyclesTaken(3);
					if((short)*a == 0)
					{
						*b = 0x0;
						reg[regEX] = 0x0;
					}
					else
					{
						result = (short)*b / (short)*a;
						reg[regEX] = ((((short)*b)<<WORD_LENGTH)/((short)*a))&0xFFFF;
						*b = result;
					}
					break;
				}

				case MOD: // 3 | 0x08 | MOD b, a | sets b to b%a. if a==0, sets b to 0 instead.
				{
					incCyclesTaken(3);
					if(*a == 0)
					{
						*b = 0;
					}
					else
					{
						*b = *b % *a;
					}
					break;
				}

				case MDI: // 3 | 0x09 | MDI b, a | like MOD, but treat b, a as signed. Rounds towards 0
				{
					incCyclesTaken(3);
					if((short)*a == 0)
					{
						*b = 0;
					}
					else
					{
						*b = (short)*b % (short)*a;
					}
					break;
				}

				case AND: //1 | 0x09 | AND b, a | sets b to b&a
					incCyclesTaken(1);
					*b = *b & *a;
					break;

				case BOR: // 1 | 0x0a | BOR b, a | sets b to b|a
					incCyclesTaken(1);
					*b = *b | *a;
					break;

				case XOR: // 1 | 0x0b | XOR b, a | sets b to b^a
					incCyclesTaken(1);
					*b = *b ^ *a;
					break;

				case SHR: // 1 | 0x0c | SHR b, a | sets b to b>>>a, sets EX to ((b<<16)>>a)&0xffff (logical shift)
					incCyclesTaken(1);
					result = *b >> *a;
					reg[regEX] = (((*b)<<WORD_LENGTH)>>(*a))&0xFFFF;
					*b = result;
					break;

				case ASR: //  1 | 0x0d | ASR b, a | sets b to b>>a, sets EX to ((b<<16)>>>a)&0xffff (arithmetic shift) (treats b as signed)
					incCyclesTaken(1);
					result = (short)*b >> *a;
					reg[regEX] = ((((short)*b)<<WORD_LENGTH)>>(*a))&0xFFFF;
					*b = result;
					break;

				case SHL: // 1 | 0x0e | SHL b, a | sets b to b<<a, sets EX to ((b<<a)>>16)&0xffff
					incCyclesTaken(1);
					result = *b << *a;
					reg[regEX] = (((*b)<<(*a))>>WORD_LENGTH)&0xFFFF;
					*b = result;
					break;

				case IFB: // 2+| 0x10 | IFB b, a | performs next instruction only if (b&a)!=0
					{
						incCyclesTaken(2);
						if(!((*b & *a) != 0))
						{
							skipNextInstruction = true;
						}
					}
					break;

				case IFC: // 2+| 0x11 | IFC b, a | performs next instruction only if (b&a)==0
					{
						incCyclesTaken(2);
						if(!((*b & *a) == 0))
						{
							skipNextInstruction = true;
						}
					}
					break;

				case IFE: // 2+| 0x12 | IFE b, a | performs next instruction only if b==a 
					{
						incCyclesTaken(2);
						if(!(*b == *a))
						{
							skipNextInstruction = true;
						}
					}
					break;

				case IFN: //  2+| 0x13 | IFN b, a | performs next instruction only if b!=a
					{
						incCyclesTaken(2);
						if(!(*b != *a))
						{
							skipNextInstruction = true;
						}
					}
					break;

				case IFG: // 2+| 0x14 | IFG b, a | performs next instruction only if b>a 
					{
						incCyclesTaken(2);
						if(!(*b > *a))
						{
							skipNextInstruction = true;
						}
					}
					break;

				case IFA: // 2+| 0x15 | IFA b, a | performs next instruction only if b>a (signed)
					{
						incCyclesTaken(2);
						if(!((short)*b > (short)*a))
						{
							skipNextInstruction = true;
						}
					}
					break;

				case IFL: // 2+| 0x16 | IFL b, a | performs next instruction only if b<a  
					{
						incCyclesTaken(2);
						if(!(*b < *a))
						{
							skipNextInstruction = true;
						}
					}
					break;

				case IFU: // 2+| 0x17 | IFU b, a | performs next instruction only if b<a (signed) 
					{
						incCyclesTaken(2);
						if(!((short)*b < (short)*a))
						{
							skipNextInstruction = true;
						}
					}
					break;

				case ADX: // 3 | 0x1a | ADX b, a | sets b to b+a+EX, sets EX to 0x0001 if there is an over-flow, 0x0 otherwise
				{
					incCyclesTaken(3);
					unsigned short temp = (*b > 0xFFFF - *a - reg[regEX]) ? 0x0001 : 0x0000;
					result = *b + *a + reg[regEX];
					reg[regEX] = temp;
					*b = result;
					break;
				}

				case SBX: // 3 | 0x1b | SBX b, a | sets b to b-a+EX, sets EX to 0xFFFF if there is an under-flow, 0x0 otherwise
				{
					incCyclesTaken(3);

					unsigned short temp = 0x0000;
					//if ((unsigned int)*b - (unsigned int)*a + (unsigned int)reg[regEX] > 0xffff) temp = 0x0001;
					/*else*/ if ((unsigned int)*b - (unsigned int)*a + (unsigned int)reg[regEX] < 0x0000) temp = 0xffff;

					result = *b - *a + reg[regEX];
					reg[regEX] = temp;
					*b = result;
					break;
				}

				case STI: // 2 | 0x1e | STI b, a | sets b to a, then increases I and J by 1
					incCyclesTaken(2);
					result = *a;
					reg[regI]++;
					reg[regJ]++;
					*b = result;
					break;

				case STD: // 2 | 0x1f | STD b, a | sets b to a, then decreases I and J by 1
					incCyclesTaken(2);
					result = *a;
					reg[regI]--;
					reg[regJ]--;
					*b = result;
					break;

			}

		}

		return error;
}

bool CPU::skip() //used to skip next instruction
{
	if (!skipNextInstruction)
	{
		return false;
	}
	else
	{
		incCyclesTaken(1);

		skipNextInstruction = false;

		unsigned short word = memory[reg[regPC]++];

		unsigned short op;
		unsigned short A;
		unsigned short B;

		


		getOpValues(word, op, A, B);

		//make sure to skip the next word for values that state to use the next word too
		if((op) == opsEXT)
		{
			computeValue(B, true, false);
		}
		else
		{
			computeValue(A, true, true);
			computeValue(B, true, false);

			if (op >= IFB && op <= IFU)
			{
				skipNextInstruction = true;
			}
		}

		return true;
	}

}

//increase cycles taken
void CPU::incCyclesTaken(int count)
{
	cyclesTaken += count;
}


//get memory location of a value (whether it's pointing to a register/memory etc location or just to a literal number)
unsigned short *CPU::computeValue(unsigned short value, bool skip, bool inA)
{
	if (value < 0x08) // 0 | 0x00-0x07 | register (A, B, C, X, Y, Z, I or J, in that order)
	{		
		return &reg[value];
	}
	else if (value < 0x10) // 0 | 0x08-0x0f | [register]
	{
		return &memory[reg[value-0x8]];
	}
	else if (value < 0x18) // 1 | 0x10-0x17 | [register + next word]
	{
		if (!skip) incCyclesTaken(1);
		unsigned short temp = memory[reg[regPC]++]+reg[value-0x10];
		return &memory[temp];
	}
	else if (value > 0x1f) // 0 | 0x20-0x3f | literal value 0xffff-0x1e (-1..30) (literal) (only for a)
	{
		reg[regLit] = value - 0x21;
		return &reg[regLit];
	}
	else 
	{
		switch (value)
		{
			case 0x18: // 0 |      0x18 | (PUSH / [--SP]) if in b, or (POP / [SP++]) if in a
				{
					if (inA)
					{
						if (!skip) {
							return &memory[reg[regSP]++];
						}
						else {
							return &memory[reg[regSP] + 1];
						}
					}
					else
					{
						if (!skip) {
							return &memory[--reg[regSP]];
						}
						else {
							return &memory[reg[regSP] - 1];
						}
					}
				break;
				}

			case 0x19: // 0 |      0x19 | [SP] / PEEK
				return &memory[reg[regSP]];
				break;

			case 0x1a: // 1 |      0x1a | [SP + next word] / PICK n
			{
				if (!skip) incCyclesTaken(1);
				unsigned short temp = memory[reg[regPC]++]+reg[regSP];
				return &memory[temp];
				break;
			}

			case 0x1b: //0 |      0x1b | SP
				return &reg[regSP];
				break;

			case 0x1c: // 0 |      0x1c | PC
				return &reg[regPC];
				break;

			case 0x1d: // 0 |      0x1d | EX
				return &reg[regEX];
				break;

			case 0x1e: // 1 |      0x1e | [next word]
			{
				if (!skip) incCyclesTaken(1);
				unsigned short temp = memory[reg[regPC]++];
				return &memory[temp];
				break;
			}

			case 0x1f: // 1 |      0x1f | next word (literal)
				if (!skip) incCyclesTaken(1);
				return &memory[reg[regPC]++];
				break;
		}
	}

		return 0;
}


//output memory (from start with length)
void CPU::memoryDump(int start, int length)
{
	if (start + length > memorySize) {
		length = memorySize - start + 1;
	}

	printf("\nMemory Dump (%04x to %04x):\n", start, (length + start));
	for (int i = start; i < start + length; i++)
	{
		if ((i % 8) == 0)
		{
			printf("\n%04x: ", i);
		}
		printf("%04x ", memory[i]);
	}

	printf("\n");
}

//output memory (all)
void CPU::memoryDump()
{
	memoryDump(0, memorySize);
}

//output registers
void CPU::registerDump()
{
	printf("\nRegister Dump:\n\n");

	printf("A: %04x ", (reg[regA]));
	printf("B: %04x ", (reg[regB]));
	printf("C: %04x ", (reg[regC]));
	printf("X: %04x ", (reg[regX]));
	printf("Y: %04x ", (reg[regY]));
	printf("Z: %04x ", (reg[regZ]));
	printf("I: %04x ", (reg[regI]));
	printf("J: %04x ", (reg[regJ]));

	printf("PC: %04x ", (reg[regPC]));
	printf("SP: %04x ", (reg[regSP]));
	printf("EX: %04x ", (reg[regEX]));
	printf("IA: %04x ", (reg[regIA]));

	printf("\n");
}

//public read memory
unsigned short CPU::memoryRead(unsigned short i)
{
	return memory[i];
}

//public write memory
void CPU::memoryWrite(unsigned short i, unsigned short data)
{
	memory[i] = data;
}

//public read register
unsigned short CPU::registerRead(Registers r)
{
	return reg[r];
}

unsigned short CPU::registerRead(unsigned short r)
{
	return reg[r];
}

//public write register
void CPU::registerWrite(Registers r, unsigned short data)
{
	reg[r] = data;
}

void CPU::registerWrite(unsigned short r, unsigned short data)
{
	reg[r] = data;
}

//public get cycle count
unsigned long long CPU::getCycle()
{
	return cycle;
}

void CPU::interruptCompute()
{
}

void CPU::interruptSoftware(unsigned short message)
{
	if (interruptQueue.size() >= interruptQueueMaxSize)
	{
		catchFire();
	}
	else
	{
		interruptQueue.push_back(message);
	}
}

void CPU::interruptSoftwareTrigger(unsigned short message)
{
	if (reg[regIA] != 0)
	{
		interruptQueueInUse = true;

		memory[--reg[regSP]] = reg[regPC];
		memory[--reg[regSP]] = reg[regA];
		reg[regPC] = reg[regIA];
		reg[regA] = message;
	}
}

void CPU::interruptSoftwareTriggerQueue()
{
	if (interruptQueue.size() > 0 && !interruptQueueInUse)
	{
		unsigned short message = interruptQueue.front();

		interruptQueue.erase(interruptQueue.begin());

		interruptSoftwareTrigger(message);
	}
}

void CPU::catchFire()
{
	printf("FIRE!");
}




