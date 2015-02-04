#include "clock.h"
#include "dcpu-16.h"

Clock::Clock()
{
	frequency = 0;

	id = 0x12d0b402;
	version = 1;
	manufacturer = 0;

	HWICalledOnce = false;

	ticks = 0;
	toInterrupt = false;
	interruptMessage = 0;

}

int Clock::run()
{
	ticks++;

	if (toInterrupt) computer->cpu->interruptSoftware(interruptMessage);

	return 0;
}

void Clock::interruptCompute()
{
	switch(registerRead(CPU::regA))
	{
	case 0: //set tick freq to 60 / reg B (off if reg B is 0)
		{
		unsigned short freq = computer->cpu->registerRead(CPU::regB);
		if (freq == 0) frequency = 0; //turn off
		else frequency = 60 / freq;
		ticks = 0;
		}
		break;

	case 1: //set reg C to ticks since last call to case 0
		computer->cpu->registerWrite(CPU::regC, ticks);
		break;

	case 2: //turn on interrupts on tick with message in reg B (or turn off if reg B is 0)
		interruptMessage = computer->cpu->registerRead(CPU::regB);
		if (interruptMessage == 0) toInterrupt = false; else toInterrupt = true;
		break;
	}
}