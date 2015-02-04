#include "hardware.h" //change
#include "dcpu-16.h"

//change each Hardware::
Hardware::Hardware()
{
	frequency = 1; //change

	id = 0; //change
	version = 0; //change
	manufacturer = 0; //change

	HWICalledOnce = false;
}

unsigned short Hardware::memoryRead(unsigned short i)
{
	return computer->cpu->memoryRead(i);
}

void Hardware::memoryWrite(unsigned short i, unsigned short data)
{
	if (HWICalledOnce) computer->cpu->memoryWrite(i, data);
}

unsigned short Hardware::registerRead(unsigned short r)
{
	return computer->cpu->registerRead(r);
}

void Hardware::registerWrite(unsigned short r, unsigned short data)
{
	if (HWICalledOnce) computer->cpu->registerWrite(r, data);
}

/*
int Hardware::run()
{
	return 0;
}

void Hardware::interruptCompute()
{
	switch(registerRead(CPU::regA))
	{
	}
}
*/
