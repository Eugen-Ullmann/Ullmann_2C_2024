/*! @mainpage Ej1_P2
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
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
#include "hc_sr04.h"
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 1000
#define CONFIG_BLINK_PERIOD_3 1500
#define CONFIG_BLINK_PERIOD_2 500

/*==================[internal data definition]===============================*/
uint16_t distance = 0;
bool toggle = false;
bool hold = false;
TaskHandle_t key_task_handle = NULL;
TaskHandle_t leds_task_handle = NULL;
TaskHandle_t measure_task_handle = NULL;
TaskHandle_t lcd_task_handle = NULL;
/*==================[internal functions declaration]=========================*/
static void KeyTask(void *pvParameter)
{
	while (true)
	{
		uint8_t teclas;
		teclas = SwitchesRead();
		if (teclas == SWITCH_1)
			toggle = !toggle;
		if (teclas == SWITCH_2)
			hold = !hold;
	}
	vTaskDelay(CONFIG_BLINK_PERIOD_2 / portTICK_PERIOD_MS); // usar otro delay
}

static void MeasureTask(void *pvParameter)
{
	while (true)
	{
		if (toggle)
			distance = HcSr04ReadDistanceInCentimeters();
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
static void LedsTask(void *pvParameter)
{
	while (true)
	{
		if (distance <= 10)
		{
			LedOff(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		}
		if (distance >= 10 && distance  <= 20)
			LedOn(LED_1);
		if (distance >=  20 && distance  <= 30)
		{
			LedOn(LED_1)
				LedOn(LED_2);
		}
		if (distance >= 30)
		{
			LedOn(LED_1)
				LedOn(LED_2)
					LedOn(LED_3);
		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
static void LCDTask(void *pvParameter)
{
	while (true)
	{
		if (toggle)
		{
			if (!hold)
			{
				LcdItsE0803Write(distance);
			}
		}
		else
			LcdItsE0803Off();
		vtaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	LedsInit();
	SwitchesInit();
	HcSr04Init(GPIO_3, GPIO_2);
	LcdItsE0803Init();
	xTaskCreate(&KeyTask, "Tarea_Teclas", 512, NULL, 5, &key_task_handle);
	xTaskCreate(&LedsTask, "LED_2", 512, NULL, 5, &leds_task_handle);
	xTaskCreate(&MeasureTask, "LED_3", 512, NULL, 5, &measure_task_handle);
	xTaskCreate(&LCDTask, "LED_3", 512, NULL, 5, &lcd_task_handle);
}
/*==================[end of file]============================================*/