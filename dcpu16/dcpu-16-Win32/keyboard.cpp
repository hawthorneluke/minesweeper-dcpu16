#include "keyboard.h"
#include "dcpu-16.h"

Keyboard::Keyboard()
{
	frequency = 1000;

	id = 0x30cf7406;
	version = 1;
	manufacturer = 0;

	HWICalledOnce = false;

	bufferIndex = 0;

	toInterrupt = false;
	interruptMessage = 0;

	memset(buffer,0,sizeof(buffer));
	memset(keysPressed,0,sizeof(keysPressed));
}

int Keyboard::run()
{
	while(SDL_PollEvent(&Event)) {
				OnEvent(&Event);
			}

	return 0;
}

void Keyboard::interruptCompute()
{
	switch(registerRead(CPU::regA))
	{
	case 0: //clear buffer
		for(unsigned short i = 0; i < bufferSize; i++)
		{
			buffer[i] = 0;
		}
		bufferIndex = 0;
		break;

	case 1: //put next key to reg C (or 0 if buffer empty)
		if (bufferIndex <= 0)
		{
			computer->cpu->registerWrite(CPU::regC,0);
		}
		else
		{
			computer->cpu->registerWrite(CPU::regC,buffer[0]);
			 for (unsigned int i = 1; i < bufferIndex; i++)
			 {
				 buffer[i-1] = buffer[i];
			 }
			 bufferIndex--;
		}
		break;

	case 2: //set reg C to 1 if key in reg B is pressed, 0 otherwise
		{
		unsigned short key = computer->cpu->registerRead(CPU::regB);
		if (key < keysPressedSize)
		{
			if (keysPressed[key] == true) computer->cpu->registerWrite(CPU::regC,1); else computer->cpu->registerWrite(CPU::regC,0);
		}
		else
		{
			computer->cpu->registerWrite(CPU::regC,0);
		}
		}
		break;

	case 3: //turn on interrupts with message in reg B (or turn off if reg B is 0)
		interruptMessage = computer->cpu->registerRead(CPU::regB);
		if (interruptMessage == 0) toInterrupt = false; else toInterrupt = true;
		break;
	}
}

void Keyboard::OnEvent(SDL_Event* Event)
{
	if(Event->type == SDL_QUIT) {
		computer->turnOff();
    }
	else
	{
		bool fKeyPressed = false;

		switch( Event->type )
		{
		  case SDL_KEYDOWN:
			  {
					int ascii = -1;

					switch (Event->key.keysym.sym)
					{

					
					case SDLK_F1:
					{
						fKeyPressed = true;
						if (!computer->step)
						{
							printf("Paused. Manual stepping turned on. Press F1 to step, F2 to return back to normal.\n");
							computer->step = true;
						}
						else
						{
							computer->cpu->run();
							computer->cpu->registerDump();
							computer->cpu->memoryDump(0x8180,64);
							//monitor->run();
						}
						break;
					}

					case SDLK_F2:
					{
						fKeyPressed = true;
						if (computer->step)
						{
							printf("Manual stepping off. Unpaused.\n");
							computer->step = false;
						}
						break;
					}

					case SDLK_F3:
					{
						fKeyPressed = true;
						if (computer->lockFreq)
						{
							printf("Frquency lock off.\n");
							computer->lockFreq = false;
						}
						else
						{
							printf("Frquency lock on (%iKHz).\n", computer->cpu->getFrequency()/1000);
							computer->lockFreq = true;
						}
						break;
					}						
						
						

					case SDLK_BACKSPACE:
						ascii = 0x10;
						break;
					
					case SDLK_RETURN:
						ascii = 0x11;
						break;

					case SDLK_INSERT:
						ascii = 0x12;
						break;

					case SDLK_DELETE:
						ascii = 0x13;
						break;

					case SDLK_LEFT:
						ascii = 0x82;
						//ascii = 1;
						break;

					case SDLK_RIGHT:
						ascii = 0x83;
						//ascii = 2;
						break;

					case SDLK_UP:
						ascii = 0x80;
						//ascii = 3;
						break;

					case SDLK_DOWN:
						ascii = 0x81;
						//ascii = 4;
						break;

					case SDLK_LSHIFT:
					case SDLK_RSHIFT:
						ascii = 0x90;
						break;

					case SDLK_LCTRL:
					case SDLK_RCTRL:
						ascii = 0x91;
						break;

					}

					if (ascii == -1)
					{

						if(!(Event->key.keysym.unicode & 0xff80))
						{
							ascii = Event->key.keysym.unicode;						
						}
					}


					if (ascii >= 0)
					{
						printf("Key pressed: %i\n",ascii);
						
						if (bufferIndex < bufferSize - 1)
						{
							buffer[bufferIndex++] = ascii;
						}

						if (ascii < keysPressedSize) keysPressed[ascii] = true;
					}

					if (toInterrupt)
					{
						computer->cpu->interruptSoftware(interruptMessage);
					}
			  }
			  break;

		  case SDL_KEYUP:
			  {
			  int ascii = -1;

					switch (Event->key.keysym.sym)
					{
						case SDLK_BACKSPACE:
						ascii = 0x10;
						break;
					
						case SDLK_RETURN:
							ascii = 0x11;
							break;

						case SDLK_INSERT:
							ascii = 0x12;
							break;

						case SDLK_DELETE:
							ascii = 0x13;
							break;

						case SDLK_LEFT:
							ascii = 0x82;
							//ascii = 1;
							break;

						case SDLK_RIGHT:
							ascii = 0x83;
							//ascii = 2;
							break;

						case SDLK_UP:
							ascii = 0x80;
							//ascii = 3;
							break;

						case SDLK_DOWN:
							ascii = 0x81;
							//ascii = 4;
							break;

						case SDLK_LSHIFT:
						case SDLK_RSHIFT:
							ascii = 0x90;
							break;

						case SDLK_LCTRL:
						case SDLK_RCTRL:
							ascii = 0x91;
							break;

					}

					if (ascii == -1)
					{

						if(!(Event->key.keysym.unicode & 0xff80))
						{
							ascii = Event->key.keysym.unicode;						
						}
					}


					if (ascii >= 0)
					{
						if (ascii < keysPressedSize) keysPressed[ascii] = false;
					}


			  if (toInterrupt)
					{
						computer->cpu->interruptSoftware(interruptMessage);
					}
			  }
			break;

		  default:
			break;
		}
    }
}