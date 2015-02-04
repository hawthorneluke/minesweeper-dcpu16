#pragma once

#include "computer.h"


//only copy over run, interruptComputer, const and deconst
class Hardware //: public Hardware
{

protected:
	unsigned int id;
	unsigned short version;
	unsigned int manufacturer;

	unsigned long frequency;

	bool HWICalledOnce; //needed because hardware can't modify any memory or registers before its been interrupted with a HWI called once



	
	


	unsigned short memoryRead(unsigned short i);

	void memoryWrite(unsigned short i, unsigned short data);

	unsigned short registerRead(unsigned short r);

	void registerWrite(unsigned short r, unsigned short data);
	

public:
	Computer *computer;

	virtual int run() =0; //need to impliment what happens when hardware is ran. change to none virtual when copying this for new hardware

	//public get hardware info (this needs to be set in any hardware)
	void getInfo(unsigned int &id_, unsigned short &version_, unsigned int &manufacturer_)
	{
		id_ = id;
		version_ = version;
		manufacturer_ = manufacturer;
	}

	virtual void interruptCompute() =0; //need to impliment what happens to hardware when it's interrupted. change to none virtual when copying this for new hardware

	//public interrupt this hardware
	virtual void interrupt()
	{
		HWICalledOnce = true;

		interruptCompute();
	}

	//public get hardware run frequency
	unsigned long getFrequency()
	{
		return frequency;
	}

	//public set hardware run frequency
	void setFrequency(unsigned long n)
	{
		frequency = n;
	}

	//init
	Hardware();

	
	//used to attach new hardware
	template <class Class>
	static Class* Attach(Computer *comp) //need to pass computer to it
	{
		Class *hard = new Class();

		hard->computer = comp;

		comp->hardware.push_back(hard);

		return hard;
	}

	
	
};