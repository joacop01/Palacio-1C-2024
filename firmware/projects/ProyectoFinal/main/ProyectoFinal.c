/*! @mainpage Guia2ej4
 *
 * @section genDesc General Description
	
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	Signal   	|	   CH1		|
 * | 	  Vcc     	|	   3.3V		|
 * | 	  GND      	|	   GND		|	
 * 
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 30/04/2024 | Document creation		                         |
 *
 * @author Joaquin Palacio
 *
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
#include "pulse_sensor.h"
#include "timer_mcu.h"
#include "buzzer.h"
#include "buzzer_melodies.h"
#include "uart_mcu.h"
#include "switch.h"

/*==================[macros and definitions]=================================*/
/** @def FS
 * @brief Frecuencia en Hz a la que se muestrea la sañal analógica
*/
#define FS 200 

/** @def CONFIG_TIMER_A
 * @brief tiempo en micro segundos del timer_A
*/
#define CONFIG_TIMER_A 1000*1000/FS

/** @def CONFIG_TIMER_B
 * @brief tiempo en micro segundos del timer_B
*/
#define CONFIG_TIMER_B 1000*1000*60

/** @def TRESHOLD
 * @brief umbral para la deteccion de latidos (Vcc/2)
*/
#define TRESHOLD 1650

/** @def MUESTRAS_1_MIN 
 * @brief cantidad de muestras que se toman en 1 minuto de la señal analógica
*/
#define MUESTRAS_1_MIN CONFIG_TIMER_B/ CONFIG_TIMER_A

/*==================[internal data declaration]==============================*/


/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
TaskHandle_t process_signal_task_handle = NULL;
TaskHandle_t read_signal_task_handle = NULL;
TaskHandle_t display_data_task_handle = NULL;
TaskHandle_t alarm_manage_task_handle = NULL;
TaskHandle_t detect_drowsines_task_handle = NULL;


/** @struct hr_monitor 
 * @brief monitor de frecuencia cardíaca
*/
HeartRateMonitor hr_monitor = 
{
    .ch = CH1,
    .sampleIntervalMs = (1000/FS),
    .threshSetting = TRESHOLD
};

/** @var on 
 * @brief determina si la aplicación se encuentra en funcionamiento
 * */
bool on = true;

/** @var somnolencia
 * @brief determina la presencia o no de somnolencia 
 * */
bool somnolencia = false;

/** @var somn
 * @brief caracter recibido por la UART que informa sobre el estado del sujeto ("0" = despierto, "1" = somnolencia) 
 * */
uint8_t somn;

/** @var count
 * @brief contador de la cantidad de muestras tomadas(se reinicia cada 1 minuto) 
 * */
uint16_t count = 0;

void FuncTimerA(void* param);

void FuncTimerB(void* param);

void serial_interrupt(void* param);

static void ProcessSignal(void *pvParameters);

static void DisplayData(void *pvParameters);

static void AlarmManage(void *pvParameters);

static void DetectDrowsines(void *pvParameters);

static void LecturaSwitch1();
/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
void FuncTimerA(void* param)
{

    vTaskNotifyGiveFromISR(process_signal_task_handle, pdFALSE);
}

void FuncTimerB(void* param)
{

	//vTaskNotifyGiveFromISR(display_data_task_handle, pdFALSE);
    //vTaskNotifyGiveFromISR(alarm_manage_task_handle, pdFALSE);

}

void serial_interrupt(void* param)
{
    UartReadByte(UART_PC, somn);
    vTaskNotifyGiveFromISR(detect_drowsines_task_handle, pdFALSE);

}

static void ProcessSignal(void *pvParameters) 
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        if(on)
        {
            AnalogInputReadSingle(hr_monitor.ch, &hr_monitor.Signal);
            processLatestSample(&hr_monitor);

            if (count == 0)
            {
                UartSendString(UART_PC, "S\r\n");   
            }
            
            uint8_t *msg =  UartItoa((uint32_t)getLatestSample(&hr_monitor), 10);
            UartSendString(UART_PC, (char*)msg);
            UartSendString(UART_PC, "\r\n");
            //if(sawStartOfBeat(&hr_monitor))
            //{
            //    uint8_t *msg =  UartItoa((uint32_t)hr_monitor.IBI, 10);
            //    UartSendString(UART_PC, (char*)msg);
            //    UartSendString(UART_PC, "\r\n");
            //}
            count++;

            if (count == MUESTRAS_1_MIN)
            {
                UartSendString(UART_PC, "E\r\n");
                count = 0;
            }            
        }
    }
}

static void DisplayData(void *pvParameters) 
{   
    while (true)
    {        
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        if(on)
        {
            //USAR DISPLAY
        }
    }
}

static void AlarmManage(void *pvParameters) 
{   
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(on)
        {
            if(somnolencia)
            {
            BuzzerPlayRtttl(songStarWars);
            }
        }
    }
}

static void DetectDrowsines(void *pvParameters) 
{   
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(on)
        {
            if(somn == '0')
            {

            somnolencia = 0;

            }
        else
            {
            somnolencia = 1;
            }
        }
    }
}

static void LecturaSwitch1()
{
    on = !on;
}
/*==================[external functions definition]==========================*/


void app_main(void) {
    timer_config_t timer_A = {
        .timer = TIMER_A,
        .period = CONFIG_TIMER_A,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
      timer_config_t timer_B = {
        .timer = TIMER_B,
        .period = CONFIG_TIMER_B,
        .func_p = FuncTimerB,
        .param_p = NULL
    };

    analog_input_config_t config_ADC = {
    .input = CH1,
    .mode = ADC_SINGLE,
    .func_p = NULL,
    .param_p = NULL,
    .sample_frec = 0
    };

    serial_config_t serial_port = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = serial_interrupt,
		.param_p = NULL
	};

    UartInit(&serial_port);
    initPulseSensor(&hr_monitor);
    AnalogInputInit(&config_ADC);
    TimerInit(&timer_A);
    TimerInit(&timer_B);
    BuzzerInit(GPIO_4);
    SwitchesInit();

    TimerStart(timer_A.timer);
    TimerStart(timer_B.timer);

    SwitchActivInt(SWITCH_1, &LecturaSwitch1, NULL );

    
    // Inicializar el monitor de frecuencia cardíaca
    printf("Iniciando sensor...\r\n");
    

    xTaskCreate(&ProcessSignal, "Process Signal", 2048, NULL, 4, &process_signal_task_handle);
    xTaskCreate(&DisplayData, "Display Data", 512, NULL, 4, &display_data_task_handle);
    xTaskCreate(&AlarmManage, "Alarm Manage", 512, NULL, 3, &alarm_manage_task_handle);
    xTaskCreate(&DetectDrowsines, "Detect Drowsiness", 512, NULL, 4, &detect_drowsines_task_handle);

}
/*==================[end of file]============================================*/






