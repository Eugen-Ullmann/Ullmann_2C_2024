/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 *Dispositivo basado en la ESP-EDU que permita detectar eventos peligrosos para ciclistas.
 * <a href="https://drive.google.com/..."
 *
 * @section hardConn Hardware Connection
 *
 *  * |   EDU-ESP    | Periferico|
 * |:----------:|:-----------|
 *  | GND    | GND	| 
 * | GPIO_6    | Buzzer	| 
 * | GPIO_8    | 	ECHO   |
 * | GPIO_9    | 		TRIGGER   |
 * | GND  | GND	|
 * | CH0    | 		Acelerometro X   |
 * | CH1    | 		Acelerometro Y|
 * | CH   | 		Acelerometro Z|
  *|	BT (UART) TX	| 	GPIO_18	|
 *|	BT	(UART) RX	|	GPIO_19	|

 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 04/11/2024 | Document creation		                         |
 *
 * @author Eugen Ullmann (eugen.ullmann@ingenieria.uner.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"
#include "hc_sr04.h"
#include "led.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_medicion_US 500000
#define CONFIG_BLINK_PERIOD_caida_US 10000 // 100hz pasado a seg es 0.01seg que son 10ms -> 10000us
#define Buzzer GPIO_6
/*==================[internal data definition]===============================*/

uint16_t distancia_vehiculos;
TaskHandle_t deteccion_handle = NULL;
TaskHandle_t deteccion_caidas_handle = NULL;
uint16_t umbral_caida = 2850; // Este umbral se calcula de la sig manera 4*0.3V + 1.65V = 2,85V => El valor dado por el conversor DA es en mv entonces sera un umbral de valor 2850
uint16_t lectura_eje_X;
uint16_t lectura_eje_y;
uint16_t lectura_eje_z;
uint16_t sumatoria_ejes;

/*==================[internal functions declaration]=========================*/
/**
 * @brief Notifica a la tarea asociada 
 * @param param puntero a un parámetro que no se utiliza
 */
void FuncTimerDeteccion(void *param)
{
	vTaskNotifyGiveFromISR(deteccion_handle, pdFALSE);
}
/**
 * @brief Notifica a la tarea asociada 
 * @param param puntero a un parámetro que no se utiliza
 */
void FuncTimerCaidas(void *param)
{
	vTaskNotifyGiveFromISR(deteccion_caidas_handle, pdFALSE);
}
/**
 * @brief  Lee el valor del sensor de HcSr04 y lo compara con un valor establecido, si
 * se pasa de ese valor prendera leds de acuerdo a la proximidad. A su vez hara sonar un buzzer con distinta frecuencia 
 * depende que tan cerca se encuentre el vehiculo. Tambien enviara por uart a un dispositivo bluetooth un msj de alerta dependiendo la distancia 
 * a la q se encuentre el vehiculo
 * @param pvParameter puntero a un parámetro que no se utiliza
 */
static void medicion_vehiculos(void *pvParameters)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		distancia_vehiculos = HcSr04ReadDistanceInCentimeters();

		distancia_vehiculos = distancia_vehiculos * 100; //Convierto los valores a metros

		if (distancia_vehiculos > 5)
			LedOn(LED_1);

		else

			LedsOffAll();

		if (distancia_vehiculos < 5 && distancia_vehiculos > 3)
		{
			LedOn(LED_1);
			LedOn(LED_2);
			UartSendString(UART_CONNECTOR, "Precaución, vehículo cerca ");
			UartSendString(UART_CONNECTOR, "\r\n");
			while (distancia_vehiculos < 5 && distancia_vehiculos > 3)
			{
				GPIOOn(Buzzer);
				vTaskDelay(pdMS_TO_TICKS(1000)); // espera 1 segundo
				GPIOOff(Buzzer);
			}
		}
		else
			LedsOffAll();
		if (distancia_vehiculos < 3)
		{
			LedOn(LED_1);
			LedOn(LED_2);
			LedOn(LED_3);
			UartSendString(UART_CONNECTOR, "Peligro, vehículo cerca");
			UartSendString(UART_CONNECTOR, "\r\n");

			while (distancia_vehiculos < 3)
			{
				GPIOOn(Buzzer);
				vTaskDelay(pdMS_TO_TICKS(500)); // espera 0.5 segundos
				GPIOOff(Buzzer);
			}
		}
		else
			LedsOffAll();
	}
}
/**
 * @brief  Lee el valor del acelerometro en cada eje y calcula la aceleracion que va a tener en base a
 * una suma escalar. A su vez manda por uart al dispositivo bluetooth un msj de si se detecta una caida 
 * @param pvParameter puntero a un parámetro que no se utiliza
 */
static void detector_caidas(void *pvParameters)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		   AnalogInputReadSingle(CH0, &lectura_eje_X);
		   AnalogInputReadSingle(CH1, &lectura_eje_y);
		   AnalogInputReadSingle(CH2, &lectura_eje_z);
		sumatoria_ejes = lectura_eje_X + lectura_eje_y + lectura_eje_z;
		if(sumatoria_ejes > umbral_caida)
		{
			UartSendString(UART_CONNECTOR, "Caída detectada");
			UartSendString(UART_CONNECTOR, "\r\n");
		}

	}
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	/*Se inicializa el sensor de ultrasonido*/
	HcSr04Init(GPIO_8, GPIO_9);

	/*Se inicializan los leds*/
	LedsInit();

 /*Se inicializa el GPIO*/
	GPIOInit(Buzzer, GPIO_OUTPUT);

/*Se configura e inicializa la uart*/
	serial_config_t my_uart = {
		.port = UART_CONNECTOR,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL};

	UartInit(&my_uart);

	/*Configuracion ADC*/
	analog_input_config_t X = {
		.input = CH0,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = NULL,
	};
	AnalogInputInit(&X);

	analog_input_config_t Y = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = NULL,
	};
	AnalogInputInit(&Y);

	analog_input_config_t Z = {
		.input = CH2,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = NULL,
	};
	AnalogInputInit(&Z);


	/* Timer configuration */
	timer_config_t timer_deteccion = {
		.timer = TIMER_A,
		.period = CONFIG_BLINK_PERIOD_medicion_US,
		.func_p = FuncTimerDeteccion,
		.param_p = NULL};
	TimerInit(&timer_deteccion);
	TimerStart(timer_deteccion.timer);


	timer_config_t timer_caida = {
		.timer = TIMER_B,
		.period = CONFIG_BLINK_PERIOD_caida_US,
		.func_p = FuncTimerCaidas,
		.param_p = NULL};
	TimerInit(&timer_caida);
	TimerStart(timer_caida.timer);

	xTaskCreate(&medicion_vehiculos, "Sensado distancia vehiculos", 512, NULL, 5, &deteccion_handle);
	xTaskCreate(&detector_caidas, "Detecta si se cayo o no", 512, NULL, 5, &deteccion_caidas_handle);
}
/*==================[end of file]============================================*/
// 1Verde 2amarillo  3 rojo

