/*! @mainpage Guia2_ej2
	Resolución del ejercicio 2 de la guía 2 de electrónica programable 1C2024
 *	
 * @section genDesc General Description
 	Mediante un sensor de ultrasonido se mide la distancia a un objeto particular, la cual se muestra por 
 	la pantalla LCD, asi como por la cantidad de leds que se encienden de acuerdo a la distancia medida
 	Se realiza mediante el uso de tareas e interrupciones
 * 
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO_3		|
 * | 	TRIGGER	 	| 	GPIO_2		|
 * | 	 +5V	 	| 	 +5V		|
 * | 	 GND	 	| 	 GND		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/04/2024 | Document creation		                         |
 *
 * @author Joaquin Palacio
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "switch.h"
#include "timer_mcu.h"

/*==================[macros and definitions]=================================*/
/**
 * @def CONFIG_SENSOR_TIMER
 * @brief Periodo del timerA
*/
#define CONFIG_SENSOR_TIMERA 1000000

/**
 * @def distancia
 * @brief distancia calculada por el sensor hc-rs04
*/
uint16_t distancia;

/**
 * @def on
 * @brief booleano que indica si se realiza la medicion y acciones asociadas
*/
bool on = 0;

/**
 * @def hold
 * @brief booleano que indica si se debe manetener la pantalla LCD congelada
*/
bool hold = 0;

/*==================[internal data definition]===============================*/
TaskHandle_t calc_dist_task_handle = NULL;
TaskHandle_t manejo_leds_task_handle = NULL;
TaskHandle_t mostrar_dist_task_handle = NULL;
TaskHandle_t lecuta_switches_task_handle = NULL;

/**
 * @fn static void CalcDsit
 * @brief El sensor hc-sr04 calcula la distancia a un objeto
 * @param[in] pvParameter
 * @return 
 */
static void CalcDist(void *pvParameter);

/**
 * @fn static void ManejoLeds
 * @brief Se encarga del control de los leds de acuerdo a la distancia medida
 * @param[in] pvParameter
 * @return 
 */
static void ManejoLeds(void *pvParameter);

/**
 * @fn static void MostrarDsit
 * @brief Muestra el valor de la distancia medida en la pantalla LCD
 * @param[in] pvParameter
 * @return 
 */
static void MostrarDist(void *pvParameter);

/**
 * @fn static void LecturaSwitch
 * @brief Funcion que se llama al producirse una interrupcion por una de las teclas, modifica el valor de 
 * on o de hold
 * @param[in] tecla indica la tecla que ocasiono la interrupcion
 * @return 
 */
static void LecturaSwitch(int *tecla);

/*==================[internal functions declaration]=========================*/
 /**
 * @fn void FuncTimerA
 * @brief Notifica a las tareas de calcular distancia, manejo de leds y mostrar distancias para que se 
 * encuentren listas
 * @param[in] param
 * @return 
 */
void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(calc_dist_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(manejo_leds_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(mostrar_dist_task_handle, pdFALSE);
}

static void CalcDist(void *pvParameter){
    while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
		if(on == 1)
		{
			distancia = HcSr04ReadDistanceInCentimeters();
		}
		else
		{

		}
        //vTaskDelay(CONFIG_DELAY / portTICK_PERIOD_MS);
    }
}

static void ManejoLeds(void *pvParameter){
    while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
		if(on == 1)
		{
			if(distancia < 10)
			{
				LedOff(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);
			}
			else if (distancia > 10 && distancia < 20)
			{
				LedOn(LED_1);
				LedOff(LED_2);
				LedOff(LED_3);

			}
			else if(distancia > 20 && distancia < 30)
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOff(LED_3);
			}
			else if(distancia > 30)
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
			}
		}
		else
		{
			LedOff(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		}
        //vTaskDelay(CONFIG_DELAY / portTICK_PERIOD_MS);
    }
}
static void MostrarDist(void *pvParameter){
    while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);    /* La tarea espera en este punto hasta recibir una notificación */
		if(on == 1)
		{
			if(hold == 1)
			{

			}
			else
			{
				LcdItsE0803Write(distancia);
			}
		}
		else
		{
			LcdItsE0803Off();
		}
		//vTaskDelay(CONFIG_DELAY / portTICK_PERIOD_MS);
    }
}

static void LecturaSwitch(int *tecla)
{
	switch (*tecla)
	{
	case SWITCH_1:
	on = !on;
		break;
	case SWITCH_2:
	hold = !hold;
		break;
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
	HcSr04Init(3, 2);
	LcdItsE0803Init();
	SwitchesInit();

	timer_config_t timer_sensor = {
        .timer = TIMER_A,
        .period = CONFIG_SENSOR_TIMERA,
        .func_p = FuncTimerA,
        .param_p = NULL
    };

	TimerInit(&timer_sensor);

	SwitchActivInt(SWITCH_1, &LecturaSwitch, SWITCH_1 );
	SwitchActivInt(SWITCH_2, &LecturaSwitch, SWITCH_2 );

    xTaskCreate(&CalcDist, "MedirDist", 512, NULL, 5, &calc_dist_task_handle);
    xTaskCreate(&ManejoLeds, "ManejoLeds", 512, NULL, 4, &manejo_leds_task_handle);
    xTaskCreate(&MostrarDist, "MostrarDist", 512, NULL, 4, &mostrar_dist_task_handle);
	//xTaskCreate(&LecturaSwitches, "LecturaSwitches", 512, NULL, 5, &lecuta_switches_task_handle);

	TimerStart(timer_sensor.timer);


    

}
/*==================[end of file]============================================*/