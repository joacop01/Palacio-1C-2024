/*! @mainpage Guia2_ej3
	Resolución del ejercicio 3 de la guía 2 de electrónica programable 1C2024
 *	
 * @section genDesc General Description
 	Mediante un sensor de ultrasonido se mide la distancia a un objeto particular, la cual se muestra por 
 	la pantalla LCD, asi como por la cantidad de leds que se encienden de acuerdo a la distancia medida
 	Se realiza mediante el uso de tareas e interrupciones.
	Se implementa la funcion de poder mostrar la distancia por el puerto serie, asi como tambien la de
	poder enviar comandos para comenzar la medicion y mantenerla ('O' y 'H' respectivamente). 
 * 
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	  ECHO	 	| 	 GPIO_3		|
 * | 	TRIGGER	 	| 	 GPIO_2		|
 * | 	  +5V	 	| 	  +5V		|
 * | 	  GND	 	| 	  GND		|
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
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/
/**
 * @def CONFIG_SENSOR_TIMER_A
 * @brief Periodo del timerA
*/
#define CONFIG_SENSOR_TIMER_A 1000000

/** @def BASE
 * @brief base numérica del número que se va a convertir a ASCII
*/
#define BASE 10 //Base numerica

/** @def NBYTES
 * @brief numero de bytes que van a ser leídos por a UART
*/
#define NBYTES 8



/*==================[internal data definition]===============================*/
/**
 * @var distancia
 * @brief distancia calculada por el sensor hc-rs04
*/
uint16_t distancia;

/**
 * @var on
 * @brief booleano que indica si se realiza la medicion y acciones asociadas
*/
bool on = 0;

/**
 * @var hold
 * @brief booleano que indica si se debe manetener la pantalla LCD congelada
*/
bool hold = 0;

/**
 * @var data
 * @brief puntero a arreglo que almacena los datos que se reciben por la UART
*/
uint8_t* data;
TaskHandle_t calc_dist_task_handle = NULL;
TaskHandle_t manejo_leds_task_handle = NULL;
TaskHandle_t mostrar_dist_task_handle = NULL;
TaskHandle_t enviar_distancia_task_handle = NULL;

/**
 * 
 * @brief Calculo de la distancia hacia un objeto.
 * @param[in] pvParameter puntero tipo void.
 */
static void CalcDist(void *pvParameter);

/**
 * 
 * @brief Se encarga del control de los leds de acuerdo a la distancia medida.
 * @param[in] pvParameter puntero tipo void.
 */
static void ManejoLeds(void *pvParameter);

/**
 * 
 * @brief Muestra el valor de la distancia medida en la pantalla LCD.
 * @param[in] pvParameter puntero tipo void.
 */
static void MostrarDist(void *pvParameter);

/**
 *
 * @brief Se ejecuta al presionarse la tecla 1, cambia el estado del booleano on
 */
static void LecturaSwitch1();

/**
 *
 * @brief Se ejecuta al presionarse la tecla 2, cambia el estado del booleano hold
 */
static void LecturaSwitch2();

 /**
 * @fn void FuncTimerA(void *param)
 * @brief Notifica a las tareas de calcular distancia, manejo de leds y mostrar distancias para que se 
 * encuentren listas
 * @param[in] param puntero tipo void
 */
void FuncTimerA(void *param);

 /**
 * @fn void RecibirData(void *param)
 * @brief Se ejecuta al producirse la interrupcion por recepcion de datos en la UART
 * @param[in] param puntero tipo void
 */
void RecibirData(void *param);

 /**
 * @fn void EnviarDistancia(void *pvParameter)
 * @brief Envia el dato de la distancia medida por la UART
 * @param[in] pvParameter puntero tipo void
 */
static void EnviarDistancia(void *pvParameter);
/*==================[internal functions declaration]=========================*/

void FuncTimerA(void* param){
    vTaskNotifyGiveFromISR(calc_dist_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(manejo_leds_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(mostrar_dist_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(enviar_distancia_task_handle, pdFALSE);
}

void RecibirData(void* param){

	UartReadBuffer(UART_PC, data, NBYTES);
	switch (*data)
	{
	case 'O':
		on = !on;
		break;
	case 'H':
		hold = !hold;
		break;
	}
}

static void EnviarDistancia(void *pvParameter){

	while (true)
	{	
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if(on == 1)
		{
		uint8_t* msg = UartItoa(distancia, BASE);
		UartSendString(UART_PC,(const char*) msg);
		UartSendString(UART_PC, " cm\r\n");	
		}
	}
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
    }
}

static void LecturaSwitch1(){

	on = !on;
}

static void LecturaSwitch2(){

	hold = !hold;
}


/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
	HcSr04Init(3, 2);
	LcdItsE0803Init();
	SwitchesInit();

	timer_config_t timer_sensor = {
        .timer = TIMER_A,
        .period = CONFIG_SENSOR_TIMER_A,
        .func_p = FuncTimerA,
        .param_p = NULL
    };

	serial_config_t serial_port = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = &RecibirData,
		.param_p = NULL
	};

	UartInit(&serial_port);
	TimerInit(&timer_sensor);

	SwitchActivInt(SWITCH_1, &LecturaSwitch1, NULL );
	SwitchActivInt(SWITCH_2, &LecturaSwitch2, NULL );

    xTaskCreate(&CalcDist, "MedirDist", 512, NULL, 5, &calc_dist_task_handle);
    xTaskCreate(&ManejoLeds, "ManejoLeds", 512, NULL, 4, &manejo_leds_task_handle);
    xTaskCreate(&MostrarDist, "MostrarDist", 512, NULL, 4, &mostrar_dist_task_handle);
	xTaskCreate(&EnviarDistancia, "EnviarDist", 512, NULL, 4, &enviar_distancia_task_handle);

	TimerStart(timer_sensor.timer);


    

}
/*==================[end of file]============================================*/
//