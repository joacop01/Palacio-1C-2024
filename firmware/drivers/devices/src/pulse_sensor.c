
/*
   PulseSensor measurement manager.
   See https://www.pulsesensor.com to get started.

   Copyright World Famous Electronics LLC - see LICENSE
   Contributors:
     Joel Murphy, https://pulsesensor.com
     Yury Gitman, https://pulsesensor.com
     Bradford Needham, @bneedhamia, https://bluepapertech.com

   Licensed under the MIT License, a copy of which
   should have been included with this software.

   This software is not intended for medical use.
*/
/*
   Internal constants controlling the rate of fading for the FadePin.

   FADE_SCALE = FadeLevel / FADE_SCALE is the corresponding PWM value.
   FADE_LEVEL_PER_SAMPLE = amount to decrease FadeLevel per sample.
   MAX_FADE_LEVEL = maximum FadeLevel value.

   The time (milliseconds) to fade to black =
     (MAX_FADE_LEVEL / FADE_LEVEL_PER_SAMPLE) * sample time (2ms)
*/
//#define FADE_SCALE 10
//#define FADE_LEVEL_PER_SAMPLE 12
//#define MAX_FADE_LEVEL (255 * FADE_SCALE)
#include "pulse_sensor.h"
#include "led.h"



void initPulseSensor(HeartRateMonitor *hr_monitor)
{
	resetVariables(hr_monitor);
}

void resetVariables(HeartRateMonitor *hr_monitor){
	for (int i = 0; i < 10; ++i) {
    hr_monitor->rate[i] = 0;
  }
  hr_monitor->QS = false;
  hr_monitor->BPM = 0;
  hr_monitor->IBI = 750;                  // 750ms per beat = 80 Beats Per Minute (BPM)
  hr_monitor->Pulse = false;
  hr_monitor->sampleCounter = 0;
  hr_monitor->lastBeatTime = 0;
  hr_monitor->P = 1600;                    // peak at 1/2 the input range of 0..1023
  hr_monitor->T = 1600;                    // trough at 1/2 the input range.
  hr_monitor->N = 0;
  hr_monitor->thresh = hr_monitor->threshSetting;     // reset the thresh variable with user defined THRESHOLD
  hr_monitor->amp = 100;                  // beat amplitude 1/10 of input range.
  hr_monitor->firstBeat = true;           // looking for the first beat
  hr_monitor->secondBeat = false;         // not yet looking for the second beat in a row
  //hr_monitor->FadeLevel = 0; // LED is dark.
}


void setThreshold(HeartRateMonitor *hr_monitor, uint16_t threshold) {
  //DISABLE_PULSE_SENSOR_INTERRUPTS;
  hr_monitor->threshSetting = threshold; // this is the backup we get from the main .ino
  hr_monitor->thresh = threshold; // this is the one that updates in software
  //ENABLE_PULSE_SENSOR_INTERRUPTS;
}

uint16_t getLatestSample(HeartRateMonitor *hr_monitor) {
  return hr_monitor->Signal;
}

uint16_t getBeatsPerMinute(HeartRateMonitor *hr_monitor) {
  return hr_monitor->BPM;
}

uint16_t getInterBeatIntervalMs(HeartRateMonitor *hr_monitor) {
  return hr_monitor->IBI;
}

uint16_t getPulseAmplitude(HeartRateMonitor *hr_monitor) {
  return hr_monitor->amp;
}

uint16_t getLastBeatTime(HeartRateMonitor *hr_monitor) {
  return hr_monitor->lastBeatTime;
}

bool sawStartOfBeat(HeartRateMonitor *hr_monitor) {
  // Disable interrupts to avoid a race with the ISR.
  //DISABLE_PULSE_SENSOR_INTERRUPTS;
  bool started = hr_monitor->QS;
  hr_monitor->QS = false;
  //ENABLE_PULSE_SENSOR_INTERRUPTS;

  return started;
}

bool isInsideBeat(HeartRateMonitor *hr_monitor) {
  return hr_monitor->Pulse;
}

void readNextSample(HeartRateMonitor *hr_monitor) {
  // We assume assigning to an int is atomic.
  AnalogInputReadSingle(hr_monitor->ch, &hr_monitor->Signal);
}

void processLatestSample(HeartRateMonitor *hr_monitor) {
  hr_monitor->sampleCounter += hr_monitor->sampleIntervalMs;         // keep track of the time in mS with this variable
  hr_monitor->N = hr_monitor->sampleCounter - hr_monitor->lastBeatTime;      // monitor the time since the last beat to avoid noise
  // Fade the Fading LED
  /*hr_monitor->FadeLevel = hr_monitor->FadeLevel - FADE_LEVEL_PER_SAMPLE;
  hr_monitor->FadeLevel = constrain(hr_monitor->FadeLevel, 0, MAX_FADE_LEVEL);*/

  //  find the peak and trough of the pulse wave
  if (hr_monitor->Signal < hr_monitor->thresh && hr_monitor->N > (hr_monitor->IBI / 5) * 3) { // avoid dichrotic noise by waiting 3/5 of last IBI
    if (hr_monitor->Signal < hr_monitor->T) {                        // T is the trough
      hr_monitor->T = hr_monitor->Signal;                            // keep track of lowest point in pulse wave
    }
  }

  if (hr_monitor->Signal > hr_monitor->thresh && hr_monitor->Signal > hr_monitor->P) {       // thresh condition helps avoid noise
    hr_monitor->P = hr_monitor->Signal;                              // P is the peak
  }                                          // keep track of highest point in pulse wave

  //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
  // signal surges up in value every time there is a pulse
  if (hr_monitor->N > 250) {                             // avoid high frequency noise
    if ( (hr_monitor->Signal > hr_monitor->thresh) && (hr_monitor->Pulse == false) && (hr_monitor->N > (hr_monitor->IBI / 5) * 3) ) {
      hr_monitor->Pulse = true;             // set the Pulse flag when we think there is a pulse
      LedOn(LED_1);
      hr_monitor->IBI = hr_monitor->sampleCounter - hr_monitor->lastBeatTime;    // measure time between beats in mS
      hr_monitor->lastBeatTime = hr_monitor->sampleCounter;          // keep track of time for next pulse

      if (hr_monitor->secondBeat) {                      // if this is the second beat, if secondBeat == TRUE
        hr_monitor->secondBeat = false;                  // clear secondBeat flag
        for (int i = 0; i <= 9; i++) {       // seed the running total to get a realisitic BPM at startup
          hr_monitor->rate[i] = hr_monitor->IBI;
        }
      }

      if (hr_monitor->firstBeat) {                       // if it's the first time we found a beat, if firstBeat == TRUE
        hr_monitor->firstBeat = false;                   // clear firstBeat flag
        hr_monitor->secondBeat = true;                   // set the second beat flag
        // IBI value is unreliable so discard it
        return;
      }


      // keep a running total of the last 10 IBI values
      uint16_t runningTotal = 0;                  // clear the runningTotal variable

      for (int i = 0; i <= 8; i++) {          // shift data in the rate array
        hr_monitor->rate[i] = hr_monitor->rate[i + 1];                // and drop the oldest IBI value
        runningTotal += hr_monitor->rate[i];              // add up the 9 oldest IBI values
      }

      hr_monitor->rate[9] = hr_monitor->IBI;                          // add the latest IBI to the rate array
      runningTotal += hr_monitor->rate[9];                // add the latest IBI to runningTotal
      runningTotal /= 10;                     // average the last 10 IBI values
      hr_monitor->BPM = 60000 / runningTotal;             // how many beats can fit into a minute? that's BPM!
      hr_monitor->QS = true;                              // set Quantified Self flag (we detected a beat)
      //hr_monitor->FadeLevel = MAX_FADE_LEVEL;             // If we're fading, re-light that LED.
    }
  }

  if (hr_monitor->Signal < hr_monitor->thresh && hr_monitor->Pulse == true) {  // when the values are going down, the beat is over
    hr_monitor->Pulse = false;       // reset the Pulse flag so we can do it again
    LedOff(LED_1);                     
    hr_monitor->amp = hr_monitor->P - hr_monitor->T;                           // get amplitude of the pulse wave
    hr_monitor->thresh = hr_monitor->amp / 2 + hr_monitor->T;                  // set thresh at 50% of the amplitude
    hr_monitor->P = hr_monitor->thresh;                            // reset these for next time
    hr_monitor->T = hr_monitor->thresh;
  }

  if (hr_monitor->N > 1200) {                          // if 1.2 seconds go by without a beat
    hr_monitor->thresh = hr_monitor->threshSetting;                // set thresh default
    hr_monitor->P = 512;                               // set P default
    hr_monitor->T = 512;                               // set T default
    hr_monitor->lastBeatTime = hr_monitor->sampleCounter;          // bring the lastBeatTime up to date
    hr_monitor->firstBeat = true;                      // set these to avoid noise
    hr_monitor->secondBeat = false;                    // when we get the heartbeat back
    hr_monitor->QS = false;
    hr_monitor->BPM = 0;
    hr_monitor->IBI = 600;                  // 600ms per beat = 100 Beats Per Minute (BPM)
    hr_monitor->Pulse = false;
    hr_monitor->amp = 100;                  // beat amplitude 1/10 of input range.

  }
}

