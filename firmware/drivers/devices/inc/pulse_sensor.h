#ifndef PULSE_SENSOR_H
#define PULSE_SENSOR_H

/** \brief Pulse sensor driver
 *
 * @note 
 * 
 * @author Joaquin Palacio
 *
 * @section changelog
 *
 * |   Date	    | Description                                    						|
 * |:----------:|:----------------------------------------------------------------------|
 * | 09/06/2024 | Document creation		                         						|
 * 
 **/
/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "analog_io_mcu.h"
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
typedef struct 
{
    /// @brief variable donde se guarda la frecuencia cardiaca en latidos por minuto
    uint16_t BPM;    

    /// @brief variable donde se guarda la señal analógica digitalizada
    uint16_t Signal;

    /// @brief variable donde se guarda el intervalo entre latidos en ms 
    uint16_t IBI;    

    /// @brief true cuando la señal está dentro de un latido
    bool Pulse;       

    /// @brief true cuando se detecta el comienzo de un latido
    bool QS;       

    /// @brief used to seed and reset the thresh variable
    uint16_t threshSetting;

    /// @brief used to hold amplitude of pulse waveform, seeded (sample value)
    uint16_t amp;

    /// @brief time when the last beat occurs
    uint16_t lastBeatTime;   

    /// @brief expected time between calls to readSensor(), in milliseconds.
    uint16_t sampleIntervalMs;

    /// @brief array to hold last ten IBI values (ms)
    uint16_t rate[10];   

    /// @brief used to determine pulse timing. Milliseconds since we started.
    uint16_t sampleCounter; 

    /// @brief used to monitor duration between beats
    uint16_t N;              

    /// @brief used to find peak in pulse wave, seeded (sample value)
    uint16_t P;    
    
    /// @brief used to find trough in pulse wave, seeded (sample value)
    uint16_t T;   
                            
    /// @brief 
    uint16_t thresh;

    /// @brief 
    bool firstBeat;   

    /// @brief 
    bool secondBeat;  

    /// @brief 
    adc_ch_t ch;

} HeartRateMonitor;
/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/// @brief 
/// @param hr_monitor 
void initPulseSensor(HeartRateMonitor *hr_monitor);

/// @brief 
/// @param hr_monitor 
void resetVariables(HeartRateMonitor *hr_monitor);

/// @brief 
/// @param hr_monitor 
/// @return 
uint16_t getLatestSample(HeartRateMonitor *hr_monitor);
 
/// @brief Returns the latest beats-per-minute measurement on this PulseSensor.
/// @param hr_monitor 
/// @return 
uint16_t getBeatsPerMinute(HeartRateMonitor *hr_monitor);
 
/// @brief Returns the latest inter-beat interval (milliseconds) on this PulseSensor.
/// @param hr_monitor 
/// @return 
uint16_t getInterBeatIntervalMs(HeartRateMonitor *hr_monitor);

/// @brief Reads and clears the 'saw start of beat' flag, "QS".
/// @param hr_monitor 
/// @return 
bool sawStartOfBeat(HeartRateMonitor *hr_monitor);

/// @brief Returns true if this PulseSensor signal is inside a beat vs. outside.
/// @param hr_monitor 
/// @return 
bool isInsideBeat(HeartRateMonitor *hr_monitor);

/// @brief Returns the latest amp value.
/// @param hr_monitor 
/// @return 
uint16_t getPulseAmplitude(HeartRateMonitor *hr_monitor);

/// @brief Returns the sample number of the most recent detected pulse.
/// @param hr_monitor 
/// @return 
uint16_t getLastBeatTime(HeartRateMonitor *hr_monitor);

/// @brief 
/// @param hr_monitor 
void readNextSample(HeartRateMonitor *hr_monitor);

/// @brief 
/// @param hr_monitor 
void processLatestSample(HeartRateMonitor *hr_monitor);

/// @brief 
/// @param hr_monitor 
/// @param threshold 
void setThreshold(HeartRateMonitor *hr_monitor, uint16_t threshold);

#endif // PULSE_SENSOR_H
