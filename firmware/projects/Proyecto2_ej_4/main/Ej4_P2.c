/*! @mainpage Ej3_P2
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
 * @author Eugen Ullmann (eugen.ullmann@ingenieria.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <analog_io_mcu.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
typedef struct {			
	adc_ch_t input;			/*!< Inputs: CH0, CH1, CH2, CH3 */
	adc_mode_t mode;		/*!< Mode: single read or continuous read */
	void *func_p;			/*!< Pointer to callback function for convertion end (only for continuous mode) */
	void *param_p;			/*!< Pointer to callback function parameters (only for continuous mode) */
	uint16_t sample_frec;	/*!< Sample frequency min: 20kHz - max: 2MHz (only for continuous mode)  */
} analog_input_config_t;	
/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	
	analog_input_config_t config;
	
	config.input = CH1;
	config.mode = ADC_SINGLE;
	config.func_p = NULL;
	config.param_p = NULL;
	config.sample_frec = NULL;
	
	AnalogInputInit(&config);
}
/*==================[end of file]============================================*/