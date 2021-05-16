#include <T36PhaseShift.h>

T36DualShift::T36DualShift(){
	modulo = 1500; //defaut : 40000Hz, with un F_BUS = 60Mhz (case de la T3.6)
	prescaler = 0; //defaut cf up
	halfMod = modulo/2; //for 50% duty cycle
	//FTM0C2VVal = 0;
	//FTM0C3VVal = 0;
	//FTM0C4VVal = 0;
	//FTM0C5VVal = 0;
}

void T36DualShift::setDualFrequency(uint32_t frequency){
	if (frequency < 8){//Almost maxmod and prescaler=7
		frequency = 8;
	}
	if (frequency >F_BUS){ //Maybe not usefull
		frequency = F_BUS / 2;
	}
	uint32_t maxmod = 65535;
	uint32_t p = 1;
	modulo = 1500;
	prescaler = 0;
	for (uint8_t i = 0; i<=7;i++){
		modulo = F_BUS / (frequency * p)-1;
		halfMod = modulo / 2;
		if (modulo <= maxmod){
			break;
		}
		else{
			p *= 2;
			prescaler += 1; 
		}
	}
}

void T36DualShift::dualFTM0InitConfig() {
  FTM0_SC = 0;//halt timer
  FTM0_CONF = 0xC0; //set up BDM in 11
  FTM0_FMS = 0x00;//clear the WPEN so that WPDIS is set in FTM0_MODE register
  FTM0_MODE |= 0x05; //enable write the FTM CnV register
  FTM0_COMBINE = FTM_COMBINE_COMBINE1 | FTM_COMBINE_SYNCEN1 | FTM_COMBINE_DTEN1; // ch 2+3 = adjustable high/low points |enable update C2V+C3V| Enable deadtime insertion, ch #2 & #3
  FTM0_COMBINE |= FTM_COMBINE_COMBINE2 | FTM_COMBINE_SYNCEN2 | FTM_COMBINE_DTEN2; //Same for ch 4 + 5
  FTM0_DEADTIME = 0x00; //set deadtime to 0
  FTM0_C0V = halfMod;  // high at 50% of period 
  FTM0_C1V = halfMod; //high at 50% of period  complementary
  FTM0_C2V = 0; //initial value of c2+3 same as c0V
  FTM0_C3V = halfMod;
  FTM0_C4V = 0;
  FTM0_C5V = halfMod;
  FTM0_SYNC |= 0x01;
  FTM0_SYNCONF |= 0x80; //enable enhanced PWM synch
  FTM0_C1SC |= FTM_CSC_ELSA;//Complement on channel 1
  FTM0_C4SC |= FTM_CSC_ELSA;//Complement on channel 4
  //FTM0_C4SC |= FTM_CSC_ELSA;//Complement on channel 4+5
  //FTM0_C4SC &= ~FTM_CSC_ELSB;//Complement on channel 4+5
  FTM0_CNT = 0;   // reset timer
  FTM0_MOD = modulo;// set frequency thrue modulo
  FTM0_SC = FTM_SC_CLKS(1) | FTM_SC_PS(prescaler); // start timer and set the prescale
  CORE_PIN22_CONFIG =  PORT_PCR_MUX(4) | PORT_PCR_DSE | PORT_PCR_SRE; // ch0
  CORE_PIN23_CONFIG =  PORT_PCR_MUX(4) | PORT_PCR_DSE | PORT_PCR_SRE; // ch1
  CORE_PIN9_CONFIG = PORT_PCR_MUX(4) | PORT_PCR_DSE | PORT_PCR_SRE; // ch2+3
  CORE_PIN6_CONFIG = PORT_PCR_MUX(4) | PORT_PCR_DSE | PORT_PCR_SRE; // ch4+5
}

void T36DualShift::updateFTM0C2VToC5V() {//Update channels
  if (FTM0_SC & 0x80) {
    FTM0_SC &= 0x7F;
    FTM0_C2V = FTM0C2VVal;
    FTM0_C3V = FTM0C3VVal;
    FTM0_C4V = FTM0C4VVal;
    FTM0_C5V = FTM0C5VVal;
    FTM0_C2SC |= FTM_CSC_ELSB;//normal operation
    FTM0_C2SC &= ~FTM_CSC_ELSA;//for ch 2+3
    FTM0_C4SC |= FTM_CSC_ELSA;//Complement on ch4+5
    FTM0_C4SC &= ~FTM_CSC_ELSB;
    FTM0_PWMLOAD |= 0x200;
  }
}

void T36DualShift::updateFTM0C2VToC5VComp() {//Update channels complementary
  if (FTM0_SC & 0x80) {
    FTM0_SC &= 0x7F;
    FTM0_C2V = FTM0C2VVal;
    FTM0_C3V = FTM0C3VVal;
    FTM0_C4V = FTM0C4VVal;
    FTM0_C5V = FTM0C5VVal;
    FTM0_C2SC |= FTM_CSC_ELSA;//Complement operation
    FTM0_C2SC &= ~FTM_CSC_ELSB;//for ch 2+3
    FTM0_C4SC |= FTM_CSC_ELSB;//normal on ch4+5
    FTM0_C4SC &= ~FTM_CSC_ELSA;
    FTM0_PWMLOAD |= 0x200;
  }
}

void T36DualShift::updateFTM0(uint32_t phase) {//Function to call to update channels. phase must be between 0 and modulo
	if (phase<0){
		phase = 0;
	}
	if (phase >modulo){
		phase =modulo;
	}
  if (phase < halfMod) {
    FTM0C2VVal = phase;
    FTM0C3VVal = phase + halfMod;
    FTM0C4VVal = FTM0C2VVal;
    FTM0C5VVal = FTM0C3VVal;
    T36DualShift::updateFTM0C2VToC5V();
}
  else{
    FTM0C2VVal = phase - halfMod;
    FTM0C3VVal = phase;
    FTM0C4VVal = FTM0C2VVal;
    FTM0C5VVal = FTM0C3VVal;
    T36DualShift::updateFTM0C2VToC5VComp();
  }
  //Serial.println(phase);
}



