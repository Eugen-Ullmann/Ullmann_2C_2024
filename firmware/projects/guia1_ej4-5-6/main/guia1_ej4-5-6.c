/*! @mainpage Blinking switch
 *
 * \section genDesc General Description
 *
 * This example makes LED_1 and LED_2 blink if SWITCH_1 or SWITCH_2 are pressed.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Eugen Ullmann (eu.ullmann@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
typedef struct
{
	gpio_t pin; /*!< GPIO pin number */
	io_t dir;	/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;
/*==================[internal functions declaration]=========================*/
/**
 * @brief Convierte un numero Bcd a decimal.
 * 
 * Esta función toma un numero decimal y lo convierte a bcd
 * @param data El número decimal el cual se convierte.
 * @param digits La cantidad de dígitos del número.
 * @param bcd_number El arreglo donde se almacena el numero bcd.
 * @return Ninguno.
 */
void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{

	for (int i = digits - 1; i >= 0; i--)
	{
		bcd_number[i] = data % 10;
		data = data / 10;
	}
}

/**
 * @brief Cuenta la cantidad de dígitos de un número entero dado.
 * 
 * Esta función toma un número entero como entrada, lo convierte a una cadena utilizando sprintf,
 * y devuelve la longitud de la cadena resultante, efectivamente contando la cantidad de dígitos
 * del número entero original.
 * 
 * @param num El número entero del cual se cuentan los dígitos.
 * @return La cantidad de dígitos del número entero de entrada.
 */
int contar_digitos(int num) 
{
	// Convertimos el número a cadena usando sprintf
	char str[50];			 // Buffer para almacenar el número como cadena
	sprintf(str, "%d", num); // Convertimos el entero a cadena

	// Devolvemos la longitud de la cadena (sin el signo si es negativo)
	return strlen(str);
}
/**
 * @brief Convierte un número dado a binario y configura los pines GPIO correspondientes.
 *
 * Esta función toma un entero de 8 bits sin signo y un puntero a una estructura gpioConf_t como entrada.
 * Luego, convierte el número a binario utilizando un bucle, y para cada bit, configura el correspondiente pin GPIO a ON o OFF.
 *
 * @param numero El entero de 8 bits sin signo a convertir a binario.
 * @param GPIOdigitos Un puntero a una estructura gpioConf_t que contiene las configuraciones de los pines GPIO.
 * @return Ninguno
 */
void convertir_a_binario(uint8_t numero, gpioConf_t *GPIOdigitos)
{

	// Convertimos el número a binario usando un bucle para imprimir cada bit
	for (int i = 0; i < 4; i++)
	{

		if ((numero >> i) & 1) // Desplazamos el bit y aplicamos un AND con 1 para obtener cada bit
			GPIOOn(GPIOdigitos[i].pin);
		else
			GPIOOff(GPIOdigitos[i].pin);
	};
}
/**
 * @brief Grafica un número en un display de 7 segmentos.
 * 
 * Esta función toma como entrada un número entero, lo convierte a un arreglo de dígitos BCD,
 * y luego grafica cada dígito en un display de 7 segmentos utilizando la configuración de GPIO proporcionada.
 * 
 * @param GPIOdigitos Puntero a la configuración de GPIO para los dígitos del display.
 * @param numero Número de dígitos a graficar.
 * @param GPIOmapa Puntero a la configuración de GPIO para el mapa de segmentos del display.
 * @param data Número entero a graficar.
 * @return None
 */
void graficarNumero(gpioConf_t *GPIOdigitos, uint8_t numero, gpioConf_t *GPIOmapa, uint32_t data)
{
	uint8_t digitosSeparados[3];
	convertToBcdArray(data, numero, digitosSeparados);
	for (int i = 0; i < 3; i++)
	{
		convertir_a_binario(digitosSeparados[i], GPIOdigitos);
		GPIOOn(GPIOmapa[i].pin);
		GPIOOff(GPIOmapa[i].pin);
	}
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	uint32_t data = 749;
	uint8_t digits;
	uint8_t bcd_number[32];
	digits = contar_digitos(data);
	printf("El número tiene %d dígitos.\n", digits);

	// convertToBcdArray(data, digits, bcd_number);
	// for (int i = 0; i < digits; i++)
	//	{
	//	printf("%d\n", bcd_number[i]);
	//}
	gpioConf_t GPIOdigitos[] =
		{
			{GPIO_20, GPIO_OUTPUT},
			{GPIO_21, GPIO_OUTPUT},
			{GPIO_22, GPIO_OUTPUT},
			{GPIO_23, GPIO_OUTPUT}};

	for (int i = 0; i < 4; i++)
	{
		GPIOInit(GPIOdigitos[i].pin, GPIOdigitos[i].dir);
	}
	// printf("%d", bcd_number[0]);
	// convertir_a_binario(bcd_number[0], GPIOdigitos);
	gpioConf_t GPIOmapa[] =
		{
			{GPIO_19, GPIO_OUTPUT},
			{GPIO_18, GPIO_OUTPUT},
			{GPIO_9, GPIO_OUTPUT}};

	for (int i = 0; i < 4; i++)
	{
		GPIOInit(GPIOmapa[i].pin, GPIOmapa[i].dir);
	}
	graficarNumero(GPIOdigitos, digits, GPIOmapa, data);
}
/*==================[end of file]============================================*/