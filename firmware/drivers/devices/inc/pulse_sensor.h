#ifndef PULSE_SENSOR_H
#define PULSE_SENSOR_H



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
/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "analog_io_mcu.h"

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

typedef struct 
{
    uint16_t BPM;                // int that holds raw Analog in 0. updated every call to readSensor()
    uint16_t Signal;             // holds the latest incoming raw data (0..1023)
    uint16_t IBI;                // int that holds the time interval (ms) between beats! Must be seeded!
    bool Pulse;          // "True" when User's live heartbeat is detected. "False" when not a "live beat".
    bool QS;             // The start of beat has been detected and not read by the Sketch.
    //uint16_t FadeLevel;          // brightness of the FadePin, in scaled PWM units. See FADE_SCALE
    uint16_t threshSetting;      // used to seed and reset the thresh variable
    uint16_t amp;                         // used to hold amplitude of pulse waveform, seeded (sample value)
    uint16_t lastBeatTime;   //time when the last beat occurs
    uint16_t sampleIntervalMs;  // expected time between calls to readSensor(), in milliseconds.
    uint16_t rate[10];                    // array to hold last ten IBI values (ms)
    uint16_t sampleCounter;     // used to determine pulse timing. Milliseconds since we started.
    uint16_t N;                           // used to monitor duration between beats
    uint16_t P;                           // used to find peak in pulse wave, seeded (sample value)
    uint16_t T;                           // used to find trough in pulse wave, seeded (sample value)
    uint16_t thresh;                      // used to find instant moment of heart beat, seeded (sample value)
    bool firstBeat;               // used to seed rate array so we startup with reasonable BPM
    bool secondBeat;  
    adc_ch_t ch;

} HeartRateMonitor;

void initPulseSensor(HeartRateMonitor *hr_monitor);

void resetVariables(HeartRateMonitor *hr_monitor);

uint16_t getLatestSample(HeartRateMonitor *hr_monitor);

    // Returns the latest beats-per-minute measurement on this PulseSensor.
uint16_t getBeatsPerMinute(HeartRateMonitor *hr_monitor);

    // Returns the latest inter-beat interval (milliseconds) on this PulseSensor.
uint16_t getInterBeatIntervalMs(HeartRateMonitor *hr_monitor);

    // Reads and clears the 'saw start of beat' flag, "QS".
bool sawStartOfBeat(HeartRateMonitor *hr_monitor);

    // Returns true if this PulseSensor signal is inside a beat vs. outside.
bool isInsideBeat(HeartRateMonitor *hr_monitor);

    // Returns the latest amp value.
uint16_t getPulseAmplitude(HeartRateMonitor *hr_monitor);

    // Returns the sample number of the most recent detected pulse.
uint16_t getLastBeatTime(HeartRateMonitor *hr_monitor);

    //COULD move these to private by having a single public function the ISR calls.
    // (internal to the library) Read a sample from this PulseSensor.
void readNextSample(HeartRateMonitor *hr_monitor);

    // (internal to the library) Process the latest sample.
void processLatestSample(HeartRateMonitor *hr_monitor);


void setThreshold(HeartRateMonitor *hr_monitor, uint16_t threshold);

#endif // PULSE_SENSOR_H
