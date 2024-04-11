/*! @mainpage Template
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
 * | 12/09/2023 | Document creation		                         |
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

/*==================[macros and definitions]=================================*/
#define CONFIG_DELAY 1000
#define CONFIG_DELAY_SWITCHES 100

uint16_t distancia;
bool on = 0;
bool hold = 0;

/*==================[internal data definition]===============================*/
TaskHandle_t calc_dist_task_handle = NULL;
TaskHandle_t manejo_leds_task_handle = NULL;
TaskHandle_t mostrar_dist_task_handle = NULL;
TaskHandle_t lecuta_switches_task_handle = NULL;
/*==================[internal functions declaration]=========================*/
static void CalcDist(void *pvParameter){
    while(true){
		if(on == 1)
		{
			distancia = HcSr04ReadDistanceInCentimeters();
		}
		else
		{

		}
        vTaskDelay(CONFIG_DELAY / portTICK_PERIOD_MS);
    }
}
static void ManejoLeds(void *pvParameter){
    while(true){
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
        vTaskDelay(CONFIG_DELAY / portTICK_PERIOD_MS);
    }
}

static void MostrarDist(void *pvParameter){
    while(true){
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
		vTaskDelay(CONFIG_DELAY / portTICK_PERIOD_MS);
    }
}

static void LecturaSwitches(void *pvParameter){
    while(true){
		uint8_t switches = SwitchesRead();
		switch(switches)
		{
			case SWITCH_1:
				on = !on;
			break;
			
			case SWITCH_2:
				hold = !hold;
			break;

			case SWITCH_1 | SWITCH_2:
				on =! on;
				hold =! hold;
			break;
		}
		
		vTaskDelay(CONFIG_DELAY_SWITCHES/ portTICK_PERIOD_MS);
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
	HcSr04Init(3, 2);
	LcdItsE0803Init();
	SwitchesInit();

    xTaskCreate(&CalcDist, "MedirDist", 512, NULL, 5, &calc_dist_task_handle);
    xTaskCreate(&ManejoLeds, "ManejoLeds", 512, NULL, 4, &manejo_leds_task_handle);
    xTaskCreate(&MostrarDist, "MostrarDist", 512, NULL, 4, &mostrar_dist_task_handle);
	xTaskCreate(&LecturaSwitches, "LecturaSwitches", 512, NULL, 5, &lecuta_switches_task_handle);

}
/*==================[end of file]============================================*/