/*! @mainpage Guia1_ej6
	Resolución del ejercicio 6 de la guía 1 de electrónica programable 1C2024
 *	
 * @section genDesc General Description
 *Función que recibe un dato entero de 32 bits, lo convierte a BCD y lo muestra
  en un display 7 segmentos.
 * 
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	D1	 		| 	GPIO_20		|
 * | 	D2	 		| 	GPIO_21		|
 * | 	D3	 		| 	GPIO_22		|
 * | 	D4	 		| 	GPIO_23		|
 * | 	SEL_1		| 	GPIO_19		|
 * | 	SEL_2		| 	GPIO_18		|
 * | 	SEL_3		| 	GPIO_9		|
 * | 	+5V			| 	+5V			|
 * | 	GND	 		| 	GND			|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 27/03/24 	| Document creation		                         |
 *
 * @author Joaquin Palacio (joaquin.palacio@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>

/*==================[macros and definitions]=================================*/

/** @def N_BITS
 * @brief Numero de bits que tiene cada dígito.
*/
const uint8_t N_BITS = 4;

/*==================[internal data definition]===============================*/

/** @def gpioConfig_t
 * @brief Estrctura que contiene el pin y su configuración (INPUT/OUTPUT)
*/
struct gpioConfig_t
{
	gpio_t pin;
	io_t dir;
};

/** @fn void BCDto7seg()
 * @brief Recibe un numero en BCD y realiza su conversion a 7 segmentos
 * @param[uiuint32_t, uint8_t, struct, struct]
 * @return 
 */
void BCDto7seg(uint32_t data, uint8_t n_digits, struct gpioConfig_t *config_pines, struct gpioConfig_t *config_latch);

/** @fn void BCDto7seg()
 * @brief Recibe un numero de 32 bits y lo convierte a BCD, almacenandolo en un array.
 * @param[uint32_t, uint8_t, uint8_t*]
 * @return 
 */
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