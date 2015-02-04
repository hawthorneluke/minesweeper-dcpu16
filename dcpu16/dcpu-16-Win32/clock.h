#pragma once

#include "computer.h"
#include "hardware.h"


class Clock : public Hardware
{

protected:
	unsigned short ticks;
	bool toInterrupt;
	unsigned short interruptMessage;


	

public:
	int run(); //need to impliment what happens when hardware is ran

	void interruptCompute(); //need to impliment what happens to hardware when it's interrupted

	//init
	Clock();
	
	
};