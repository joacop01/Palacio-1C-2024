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
 * @author Joaquin Palacio (joaquin.palacio@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
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

/*==================[external functions definition]==========================*/
void app_main(void){
	
	uint32_t data = 500;
	uint8_t digits = 3;
	uint8_t bcd_number[digits];

	convertToBcdArray(data, digits, bcd_number);
	for(uint8_t i = digits; i > 0; i--)
	{
		printf("%d", bcd_number[i-1]);
	}
	
}
/*==================[end of file]============================================*/