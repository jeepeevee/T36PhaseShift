/*read before use:
 * T36PhaseShift is use to provide phase shifing between  tow pairs of square wave with 50% duty cycle
 * frequency is configurable
 * phase can be changed on the fly
 * wiring : first pair of waves are on 23 and 22 pins. the output signals are "fixed" (not affected with phase).
 * 23 and 22 are complementary : while 23 is high, 22 is low and vice-versa. It is usefull to use with drv8871 for example
 * https://www.adafruit.com/product/3190
 * second pair of waves are pins 6 and 9. the outputs signals are phase shifted with 23 and 22. when phase = 0 (no phase shift)
 * signal on pin 6 follows exactly signal on pin 23 and signal on pin 9 the one on pin 22.
 * With this programme 2 motors drivers (like drv8871) can be used.
 * driver 1 : in1->23 in2->22
 * driver 2 : in1 ->6 in2 -> 9
 * This library only use FTM0 timers.
 * Exact desired frequency may not be obtained. Use scope do verify.
 * Compatible with Intervaltimer, ADC lib (used on ADC0)
 * Purpose of the script:
 * read value of a potentiometer, convert it into a phase (phase drive 6-9 signals between 0-360)
 * phase is expressed in numerical unit. minimum value is 0 (0°) maximum value is modulo (360).
 * modulo is computed after setting frequency et can be used (attribute).
 */

#include <T36PhaseShift.h>
#include <ADC.h>

ADC *adc = new ADC();
IntervalTimer theTimer;
T36DualShift monShift;

uint32_t Teus = 10000; //sampling period
uint32_t Tf = 100000; //filter constant time for potentiometer (used for smoothing potentiometer signal)
uint32_t valPot, valPotPrec, valPotScaled;
uint32_t nP = 1; //number of periods covered in one rotation of pot.
const int pinpwm = 2;//reference pin to see if they is decay in time (belongs to FTM3, independant of FTM0)
uint32_t freq =500;


uint32_t o1Filter(uint32_t x, uint32_t y) {
  float xx = (float) x;
  float yy = (float) y;
  float out = (((float) Tf) * xx + ((float) Teus) * yy) / ((float) Tf + Teus);
  return (uint32_t) out;
}

void myISR() {
  valPot = adc->analogReadContinuous(ADC_0);
  valPot = o1Filter(valPotPrec, valPot);
  valPotScaled = (uint32_t)((float)monShift.modulo/1023 * (float)valPot);//scaled to match with 0-modulo amplitude
  valPotScaled = (valPotScaled * nP) % monShift.modulo;//optionnal : in one turn of pot, make np phase shift between 0-360
  monShift.updateFTM0(valPotScaled);//set phase here
  valPotPrec = valPot;//uptade for next call (used for 1rst order filter)
}

void setup() {
  Serial.begin(115200);
  delay(100);
  monShift.setDualFrequency(freq);
  monShift.dualFTM0InitConfig();
  //Serial.println(monShift.modulo);
  //Serial.println(monShift.prescaler);
  //Serial.println(monShift.halfMod);
  analogWriteRes(12);
  analogWriteFrequency(pinpwm,freq);
  analogWrite(pinpwm,2048);
  adc->adc0->setAveraging(2);
  adc->adc0->setResolution(10);
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED :: VERY_HIGH_SPEED);
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED :: VERY_HIGH_SPEED);
  adc->adc0->startContinuous(A14);
  valPot = adc->analogReadContinuous(ADC_0);
  valPotPrec = valPot;
  theTimer.begin(myISR, Teus);
}

void loop() {
}
