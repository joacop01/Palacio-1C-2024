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
    /** @var BPM
    * @brief variable donde se guarda la frecuencia cardiaca en latidos por minuto
    */
    uint16_t BPM;    

     /** @var Signal
    * @brief variable donde se guarda la señal analógica digitalizada
    */
    uint16_t Signal;

     /** @var IBI
    * @brief variable donde se guarda el intervalo entre latidos en ms
    */
    uint16_t IBI;    

    /** @var Pulse
    * @brief true cuando la señal está dentro de un latido
    */
    bool Pulse;       

    /** @var QS
    * @brief true cuando se detecta el comienzo de un latido
    */
    bool QS;       

    /** @var threshSetting
    * @brief used to seed and reset the thresh variable
    */
    uint16_t threshSetting;

    /** @var amp
    * @brief used to hold amplitude of pulse waveform, seeded (sample value)
    */
    uint16_t amp;

    /** @var lastBeatTime
    * @brief time when the last beat occurs
    */
    uint16_t lastBeatTime;   

    /** @var sampleIntervalMs
    * @brief expected time between calls to readSensor(), in milliseconds.
    */
    uint16_t sampleIntervalMs;

    /** @var rate[10]
    * @brief array to hold last ten IBI values (ms)
    */
    uint16_t rate[10];   

    /** @var sampleCounter
    * @brief used to determine pulse timing. Milliseconds since we started.
    */
    uint16_t sampleCounter; 

    /** @var N
    * @brief used to monitor duration between beats
    */
    uint16_t N;              

    /** @var P
    * @brief used to find peak in pulse wave, seeded (sample value)
    */
    uint16_t P;    
    
    /** @var T
    * @brief used to find trough in pulse wave, seeded (sample value)
    */
    uint16_t T;   
                            
    /** @var thresh
    * @brief umbral con el que se compara la señal
    */
    uint16_t thresh;

    /** @var firstBeat
    * @brief true si se encuentra en el primer latido
    */
    bool firstBeat;   

    /** @var secondBeat
    * @brief true si se encuentra en el segundo latido
    */
    bool secondBeat;  

    /** @var ch
    * @brief canal del que se lee la señal de pulso
    */
    adc_ch_t ch;

} HeartRateMonitor;
/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @fn void initPulseSensor(HeartRateMonitor *hr_monitor)
 * @brief inicializacion del sensor
 * @param[in] hr_monitor puntero a struct HearRateMonitor
 */
void initPulseSensor(HeartRateMonitor *hr_monitor);

/**
 * @fn void resetVariables(HeartRateMonitor *hr_monitor);
 * @brief reseteo de las variables del monitor de frecuencia cardíaca
 * @param[in] hr_monitor puntero a struct HearRateMonitor
 */
void resetVariables(HeartRateMonitor *hr_monitor);

/**
 * @fn uint16_t getLatestSample(HeartRateMonitor *hr_monitor)
 * @brief devuelve la ultima muestra de la señal de PPG
 * @param[in] hr_monitor puntero a struct HearRateMonitor
 * @return uint16_t ultima muestra de la señal
 */
uint16_t getLatestSample(HeartRateMonitor *hr_monitor);
 
/**
 * @fn uint16_t getBeatsPerMinute(HeartRateMonitor *hr_monitor)
 * @brief devuelve la frecuencia cardiaca actual
 * @param[in] hr_monitor puntero a struct HearRateMonitor
 * @return uint16_t BPM actual
 */
uint16_t getBeatsPerMinute(HeartRateMonitor *hr_monitor);
 
/**
 * @fn uint16_t getInterBeatIntervalMs(HeartRateMonitor *hr_monitor)
 * @brief devuelve el intervalo entre latidos actual
 * @param[in] hr_monitor puntero a struct HearRateMonitor
 * @return uint16_t IBI actual
 */
uint16_t getInterBeatIntervalMs(HeartRateMonitor *hr_monitor);

/**
 * @fn bool sawStartOfBeat(HeartRateMonitor *hr_monitor)
 * @brief devuelve true si detecta el inicio de un latido
 * @param[in] hr_monitor puntero a struct HearRateMonitor
 * @return bool determina si se encuentra ante el inicio de un latido o no
 */
bool sawStartOfBeat(HeartRateMonitor *hr_monitor);

/**
 * @fn bool isInsideBeat(HeartRateMonitor *hr_monitor)
 * @brief devuelve true si se encuentra dentro de un latido
 * @param[in] hr_monitor puntero a struct HearRateMonitor
 * @return bool determina si se encuentra dentro de un latido
 */
bool isInsideBeat(HeartRateMonitor *hr_monitor);

/**
 * @fn uint16_t getPulseAmplitude(HeartRateMonitor *hr_monitor)
 * @brief devuelve el ultimo valor de amplitud
 * @param[in] hr_monitor puntero a struct HearRateMonitor
 * @return uint16_t amplitud de la señal de pulso
 */
uint16_t getPulseAmplitude(HeartRateMonitor *hr_monitor);

/**
 * @fn uint16_t getLastBeatTime(HeartRateMonitor *hr_monitor)
 * @brief devuelve el numero de muestras que paso desde el ultimo latido detectado
 * @param[in] hr_monitor puntero a struct HearRateMonitor
 * @return uint16_t numero de muestras desde el ultimo latido detectado
 */
uint16_t getLastBeatTime(HeartRateMonitor *hr_monitor);

/**
 * @fn void readNextSample(HeartRateMonitor *hr_monitor)
 * @brief lee la señal analogica del sensor y realiza su conversion ADC
 * @param[in] hr_monitor puntero a struct HearRateMonitor
 */
void readNextSample(HeartRateMonitor *hr_monitor);

/**
 * @fn void processLatestSample(HeartRateMonitor *hr_monitor)
 * @brief realiza el procesamiento de la ultima muestra(calculo de BPM, IBI, ect)
 * @param[in] hr_monitor puntero a struct HearRateMonitor
 */
void processLatestSample(HeartRateMonitor *hr_monitor);

/**
 * @fn void setThreshold(HeartRateMonitor *hr_monitor, uint16_t threshold)
 * @brief seteo del treshold mediante el cual se detectan latidos
 * @param[in] hr_monitor puntero a struct HearRateMonitor
 * @param[in] treshold entero de 16 bits que determina el nuevo umbral
 */
void setThreshold(HeartRateMonitor *hr_monitor, uint16_t threshold);

#endif // PULSE_SENSOR_H
