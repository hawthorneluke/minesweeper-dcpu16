#pragma once

#include "computer.h"
#include "hardware.h"


class Keyboard : public Hardware
{

protected:
	SDL_Event Event;


	void OnEvent(SDL_Event* Event);

	static const unsigned short bufferSize = 16;
	unsigned short buffer[bufferSize];

	unsigned short bufferIndex;
	bool toInterrupt;
	unsigned short interruptMessage;

	static const unsigned short keysPressedSize = 128;
	bool keysPressed[keysPressedSize];
	

public:
	int run(); //need to impliment what happens when hardware is ran

	void interruptCompute(); //need to impliment what happens to hardware when it's interrupted

	//init
	Keyboard();
	
	
};