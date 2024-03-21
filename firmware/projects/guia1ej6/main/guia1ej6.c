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
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/
const uint8_t N_BITS = 4;
/*==================[internal data definition]===============================*/

struct gpioConfig_t
{
	gpio_t pin;
	io_t dir;
};
void BCDto7seg(uint32_t data, uint8_t n_digits, struct gpioConfig_t *config_pines, struct gpioConfig_t *config_latch);
void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number);

/*==================[internal functions declaration]=========================*/
void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{
	for(uint8_t i = 0; i < digits; i++)
	{
		bcd_number[i] = data % 10; 
		data /= 10; 
	}
}
void BCDto7seg(uint32_t data, uint8_t digits, struct gpioConfig_t *config_pines, struct gpioConfig_t *config_latch)
{
		for(uint8_t i = 0; i < N_BITS; i++)
	{
		GPIOInit(config_pines[i].pin, config_pines[i].dir);
	}
		for(uint8_t i = 0; i < digits; i++)
	{
		GPIOInit(config_latch[i].pin, config_latch[i].dir);
	}

	uint8_t BCD [digits];
	convertToBcdArray(data, digits, BCD);


for(uint8_t j = digits; j > 0; j--)
{
	for(uint8_t i = 0; i < N_BITS; i++)
	{
		if((BCD[j-1] & 1 << i) == 0)
		{
			GPIOOff(config_pines[i].pin);			
		}
		else
		{
			GPIOOn(config_pines[i].pin);
		}
	}
	GPIOOn(config_latch[j-1].pin);
	GPIOOff(config_latch[j-1].pin);
}
}
/*==================[external functions definition]==========================*/
void app_main(void){

	uint8_t digits = 3;
	uint32_t data = 456;

	struct gpioConfig_t config_pines[N_BITS];
	struct gpioConfig_t config_latch[digits];

	config_pines[0].pin = GPIO_20;
	config_pines[1].pin = GPIO_21;
	config_pines[2].pin = GPIO_22;
	config_pines[3].pin = GPIO_23;
	
	for(uint8_t i = 0; i < N_BITS; i++)
	{
		config_pines[i].dir = 1;
	}

	config_latch[0].pin = GPIO_9;
	config_latch[1].pin = GPIO_18;
	config_latch[2].pin = GPIO_19;

	for(uint8_t i = 0; i < digits; i++)
	{
		config_latch[i].dir = 1;
	}

	BCDto7seg(data, digits, config_pines, config_latch);

}
/*==================[end of file]============================================*/