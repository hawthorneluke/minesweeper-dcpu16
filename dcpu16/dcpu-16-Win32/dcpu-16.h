#pragma once

#include "hardware.h"
#include <vector>





#define WORD_LENGTH 16

using namespace std;

//the CPU
class CPU : public Hardware
{
public:
	static enum Registers { regA, regB, regC, regX, regY, regZ, regI, regJ, regPC, regSP, regEX, regIA, regLit, regCount };
	static enum Ops { opsEXT = 0x00, SET, ADD, SUB, MUL, MLI, DIV, DVI, MOD, MDI, AND, BOR, XOR, SHR, ASR, SHL, IFB, IFC, IFE, IFN, IFG, IFA, IFL, IFU, ADX = 0x1a, SBX, STI = 0x1e, STD };
	static enum OpsSpecial { opsSpecialEXT = 0x00, JSR, HCF = 0x07, INT, IAG, IAS, RFI, IAQ, HWN = 0x10, HWQ, HWI };

	CPU();
	int run();
	void interruptCompute();

	void memoryDump();
	void memoryDump(int, int);
	void registerDump();

	void memoryWrite(unsigned short,unsigned short);
	unsigned short memoryRead(unsigned short);

	void registerWrite(Registers, unsigned short);
	void registerWrite(unsigned short, unsigned short);
	unsigned short registerRead(Registers);
	unsigned short registerRead(unsigned short);

	unsigned long long getCycle();

	void interruptSoftware(unsigned short message);

	void incCyclesTaken(int);

	unsigned short makeInstruction(unsigned short op, unsigned short b, unsigned short a);
	




	//break a ushort into bits
	static bool* ushortToBits(unsigned short ushort, int start, int length)
	{
		bool *bits = new bool[length]; //make sure to "delete []" this after using


		for(int i = 0; i < length; i++)
				bits[i] = ((ushort >> (i+start)) & 1);


		return bits;
	}

	//make a ushort from bits
	static unsigned short bitsToUshort(bool bits[], int start, int length) //try to use stuff like op = word&0xf;	a = ((word>>4)&0x3f);	b = ((word>>10)&0x3f); instead of this for loop
	{
		unsigned short r = 0;

			for(int i = 0; i < length; i++)
				r += (bits[start+i] << i);

			return r;
	}

	




private:


	
	bool halt;

	unsigned long long cycle;

	static const int memorySize = 0xffff;
	unsigned short memory[memorySize];

	unsigned short reg[regCount];



	static const unsigned short interruptQueueMaxSize = 256;
	bool interruptQueueInUse;

	vector<unsigned short> interruptQueue;



	

	int computeInstruction();
	void getOpValues(unsigned short, unsigned short&, unsigned short&, unsigned short&);
	int computeOp(unsigned short, unsigned short, unsigned short);
	unsigned short *computeValue(unsigned short, bool, bool);

	bool skipNextInstruction;
	bool skip();

	int cyclesTaken;

	void interruptSoftwareTrigger(unsigned short message);
	void interruptSoftwareTriggerQueue();

	void catchFire();
	
	

	

};