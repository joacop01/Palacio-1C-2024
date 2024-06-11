/*! @mainpage Examen_parcial
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/06/2024 | Document creation		                         |
 *
 * @author Joaquin Palacio 
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
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
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
#define GPIO_SENSOR_HUM GPIO_20
#define GPIO_BOMBA_AGUA GPIO_0 
#define GPIO_BOMBA_SOL_BASICA GPIO_10
#define GPIO_BOMBA_SOL_ACIDA GPIO_11
#define CONFIG_TIMER_A (3*1000*1000) //periíodo de 3 segundos
#define CONFIG_TIMER_B (5*1000*1000) //periíodo de 5 segundos
#define BASE 10 //base numérica

/*==================[internal data definition]===============================*/
TaskHandle_t suministro_agua_task_handle = NULL;
TaskHandle_t control_ph_task_handle = NULL;
TaskHandle_t send_data_task_handle = NULL;

uint32_t pH; // variable donde se almacena el pH leído


static void SuministroAgua(void *pvParameters); // funcion que controla el suministro de agua
static void ControlpH(void *pvParameters); // funcion que controla el pH

void FuncTimerA(void *param); // funcion de callback del timer A
void FuncTimerB(void *param); // funcion de callback del timer B
/*==================[internal functions declaration]=========================*/
void FuncTimerA(void* param){

	vTaskNotifyGiveFromISR(suministro_agua_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(control_ph_task_handle, pdFALSE);
}

void FuncTimerB(void* param)
{

}



static void SuministroAgua(void *pvParameters)
{
	while(true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if(GPIORead(GPIO_SENSOR_HUM)) // si el GPIO del sensor de humedad esta en alto
		{
			GPIOOn(GPIO_BOMBA_AGUA); //prendemos la bomba
		}
		else
		{
			GPIOOn(GPIO_BOMBA_AGUA); //apagamos la bomba
		}
	}
}

static void ControlpH(void *pvParameters)
{
	while(true){

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);


		AnalogInputReadSingle(CH1, &pH); //realizamos la conversión de la tensión leída 
		pH = (pH * 14) / 3; //conversión del valor de tensión a pH
		
		if(pH < 6) //si el pH es menor a 6
		{
			GPIOOn(GPIO_BOMBA_SOL_BASICA); //prendemos la bomba de solucion básica
		}
		else // si el pH es mayor o igual a 6
		{
			GPIOOff(GPIO_BOMBA_SOL_BASICA); //apagamos la bomba de solucion básica
		}

		if(pH > 6.7) //si el pH es mayor a 6.7
		{
			GPIOOn(GPIO_BOMBA_SOL_ACIDA); //prendemos la bomba de solución ácida
		}
		else // si el pH es menor o igual a 6.7
		{
			GPIOOff(GPIO_BOMBA_SOL_ACIDA); //apagamos la bomba de solución ácida
		}
	}
}

static void SendData(void *pvParameters)
{
	while(true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		UartSendString(UART_PC, "pH: ");
		uint8_t *msg = UartItoa(pH, BASE);
		UartSendString(UART_PC, (char*)msg);
		UartSendString(UART_PC, ", ");
		if (pH >= 6 && pH <= 6.7)
		{
			UartSendString(UART_PC, "humedad correcta"); //si el pH esta en el rango correcto.
		}
		else
		{
			UartSendString(UART_PC, "humedad incorrecta"); // si el pH NO está en el rango correcto.
		}

		UartSendString(UART_PC, "\r\n");

		if(GPIORead(GPIO_BOMBA_AGUA)) //si la bomba de agua está encendida
		{
			UartSendString(UART_PC, "Bomba de agua encendida\r\n");
		}
		if(GPIORead(GPIO_BOMBA_SOL_BASICA)) //si la bomba de solución básica está encendida
		{
			UartSendString(UART_PC, "Bomba de pHB encendida\r\n");
		}
		if(GPIORead(GPIO_BOMBA_SOL_ACIDA)) //si la bomba de solución ácida está encendida
		{
			UartSendString(UART_PC, "Bomba de pHA encendida\r\n");
		}

	}
}
/*==================[external functions definition]==========================*/
void app_main(void){

	analog_input_config_t config_ADC = {
    .input = CH1,
    .mode = ADC_SINGLE,
    .func_p = NULL,
    .param_p = NULL,
    .sample_frec = 0
    };

	timer_config_t timer_sensores = {
	.timer = TIMER_A,
	.period = CONFIG_TIMER_A,
	.func_p = FuncTimerA,
	.param_p = NULL
    };

	timer_config_t timer_uart = {
	.timer = TIMER_B,
	.period = CONFIG_TIMER_B,
	.func_p = FuncTimerB,
	.param_p = NULL
    };

	AnalogInputInit(&config_ADC);
	GPIOInit(GPIO_SENSOR_HUM, GPIO_INPUT); // inicializamos el gpio del sensor de humedad como de entrada
	GPIOInit(GPIO_BOMBA_AGUA, GPIO_OUTPUT);
	TimerStart(timer_sensores.timer);
	TimerStart(timer_uart.timer);

	xTaskCreate(&SuministroAgua,"Suministro de agua", 1024, NULL, 4, &suministro_agua_task_handle);
	xTaskCreate(&ControlpH, "Control pH", 1024, NULL, 4, &control_ph_task_handle);
	xTaskCreate(&SendData, "Send Data", 1024, NULL, 4, &send_data_task_handle);
}
/*==================[end of file]============================================*/