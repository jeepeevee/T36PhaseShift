#ifndef T36PhaseShift_H_INCLUDED
#define T36PhaseShift_H_INCLUDED
#include <Arduino.h>

class T36DualShift
{
	public:
		T36DualShift();
		uint32_t modulo;
		uint8_t prescaler;
		uint32_t halfMod;
		uint32_t FTM0C2VVal, FTM0C3VVal,FTM0C4VVal,FTM0C5VVal;
		void setDualFrequency(uint32_t frequency);
		void dualFTM0InitConfig();
		void updateFTM0C2VToC5V();
		void updateFTM0C2VToC5VComp();
		void updateFTM0(uint32_t phase);
	private:

		


};





#endif