#pragma once

#include "stdafx.h"
#include<iostream>
#include <math.h>
#include <fstream>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>

#include <SDL.h>


using namespace std;

class Hardware;
class CPU;

 
class Computer
{
private:
	static const long long frequency = 100000;

	static const bool dontIncludeCPUInHardwareCount = true;

	

    bool running;



	bool fileIsBigEndian;

	
    Hardware *keyboard;


	


	double PCFreq;
	__int64 CounterStart;

	void StartCounter()
	{
		LARGE_INTEGER li;
		if(!QueryPerformanceFrequency(&li))
			cout << "QueryPerformanceFrequency failed!\n";

		PCFreq = double(li.QuadPart)/1000000000.0; //nano seconds

		QueryPerformanceCounter(&li);
		CounterStart = li.QuadPart;
	}
	double GetCounter()
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		return double(li.QuadPart-CounterStart)/PCFreq;
	}

	struct timespec
	{
	  time_t tv_sec;
	  long int tv_nsec;
	};

	static void
	nanosleep (const struct timespec *requested_delay)
	{
	  if (requested_delay->tv_sec > 0)
		/* At least one second. Millisecond resolution is sufficient. */
		Sleep ((DWORD)(requested_delay->tv_sec * 1000 + requested_delay->tv_nsec / 1000000));
	  else
		{
		  /* Use Sleep for the largest part, and busy-loop for the rest. */
		  static double frequency;
		  if (frequency == 0)
			{
			  LARGE_INTEGER freq;
			  if (!QueryPerformanceFrequency (&freq))
				{
				  /* Cannot use QueryPerformanceCounter. */
				  Sleep (requested_delay->tv_nsec / 1000000);
				  return;
				}
			  frequency = (double) freq.QuadPart / 1000000000.0;
			}
		  long long expected_counter_difference = (long long)(requested_delay->tv_nsec * frequency);
		  int sleep_part = (int) requested_delay->tv_nsec / 1000000 - 10;
		  LARGE_INTEGER before;
		  QueryPerformanceCounter (&before);
		  long long expected_counter = before.QuadPart + expected_counter_difference;
		  if (sleep_part > 0)
			Sleep (sleep_part);
		  for (;;)
			{
			  LARGE_INTEGER after;
			  QueryPerformanceCounter (&after);
			  if (after.QuadPart >= expected_counter)
				break;
			}
		}
	}

	void sleep(long int time)
	{
		struct Computer::timespec t;
		t.tv_sec = 0;
		t.tv_nsec = time;
		nanosleep (&t);
	}








 
public:
    Computer();

	vector<Hardware*> hardware;
	CPU *cpu;
	
	bool lockFreq;
	bool step;

	int getHardwareCount();
	void getHardwareInfo(unsigned short i, unsigned short &A, unsigned short &B, unsigned short &C, unsigned short &X, unsigned short &Y);
	void interruptHardware(unsigned short i);
 
    int OnExecute(char*);
 
    bool OnInit(char*);
 
    void OnEvent(SDL_Event* Event);

    void OnCleanup();



	/*
	static bool isLittleEndian()
	{
		union {
			uint32_t i;
			char c[4];
		} bint = {0x01020304};

		return bint.c[0] != 1; 
	}

	unsigned short toLittleEndian(unsigned short data)
	{
		if (!isLittleEndian() || fileIsBigEndian)
		{
			unsigned char bytes[2];
			bytes[0] = (data >>  8) & 0xFF;
			bytes[1] =  data        & 0xFF;

			data = (unsigned short)bytes;
		}

		return data;
	}
	*/

	unsigned short toLittleEndian(unsigned short data)
	{
		if (fileIsBigEndian) //reverse byte order
		{
			unsigned short lower = data & 0x00ff;
			unsigned short upper = data & 0xff00;

			data = (lower << 8) | (upper >> 8);
		}

		return data;
	}

	void turnOff()
	{
		running = false;
	}
};
