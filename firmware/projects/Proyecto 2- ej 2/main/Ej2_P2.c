/*! @mainpage Ej2_P2
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
#include "timer_mcu.h"
#include "lcditse0803.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_PERIOD_US 1000*1000


/*==================[internal data definition]===============================*/
uint16_t distance = 0;
bool toggle = false;
bool hold = false;
TaskHandle_t leds_task_handle = NULL;
TaskHandle_t measure_task_handle = NULL;
TaskHandle_t lcd_task_handle = NULL;
/*==================[internal functions declaration]=========================*/

/**
 * @brief Función invocada en la interrupción del timer measure
 */
void FuncTimerMeasure(void *param)
{
	vTaskNotifyGiveFromISR(measure_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada */
}
/**
 * @brief Función invocada en la interrupción del timer LCD
 */
void FuncTimerLCD(void *param)
{
	vTaskNotifyGiveFromISR(lcd_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada  */
	
	vTaskNotifyGiveFromISR(leds_task_handle, pdFALSE);
}
/**
 * @brief Función invocada en la interrupción del timer leds
 */
void FuncTimerLeds(void *param)
{
	vTaskNotifyGiveFromISR(leds_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada */
}

static void MeasureTask(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (toggle)
			distance = HcSr04ReadDistanceInCentimeters();
	}
}
static void LedsTask(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* La tarea espera en este punto hasta recibir una notificación */
		if (distance <= 10)
		{
			LedOff(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		}
		if (distance >= 10 && distance <= 20)
		{
			LedOn(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		}

		if (distance >= 20 && distance <= 30)
		{
			LedOn(LED_1);
			LedOn(LED_2);
			LedOff(LED_3);
		}
		if (distance >= 30)
		{
			LedOn(LED_1);
			LedOn(LED_2);
			LedOn(LED_3);
		}
	}
}
static void LCDTask(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (toggle)
		{
			if (!hold)
			{
				LcdItsE0803Write(distance);
			}
		}
		else
			LcdItsE0803Off();
	}
}
void Key1(void)
{
	toggle = !toggle;
}
void Key2(void)
{
	hold = !hold;
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	LedsInit();
	SwitchesInit();
	HcSr04Init(GPIO_3, GPIO_2);
	LcdItsE0803Init();
	/* Timer configuration */
	timer_config_t timer_measure = {
		.timer = TIMER_A,
		.period = CONFIG_PERIOD_US,
		.func_p = FuncTimerMeasure,
		.param_p = NULL};
	TimerInit(&timer_measure);

	timer_config_t timer_lcd = {
		.timer = TIMER_B,
		.period = CONFIG_PERIOD_US,
		.func_p = FuncTimerLCD,
		.param_p = NULL};
	TimerInit(&timer_lcd);

	timer_config_t timer_leds = {
		.timer = TIMER_C,
		.period = CONFIG_PERIOD_US,
		.func_p = FuncTimerLeds,
		.param_p = NULL
		};
	TimerInit(&timer_leds);
	TimerStart(timer_lcd.timer);
	TimerStart(timer_measure.timer);
	TimerStart(timer_leds.timer);

	SwitchActivInt(SWITCH_1, Key1, NULL);
	SwitchActivInt(SWITCH_2, Key2, NULL);
	xTaskCreate(&LedsTask, "LED_2", 512, NULL, 5, &leds_task_handle);
	xTaskCreate(&MeasureTask, "LED_3", 512, NULL, 5, &measure_task_handle);
	xTaskCreate(&LCDTask, "LED_3", 512, NULL, 5, &lcd_task_handle);
}
/*==================[end of file]============================================*/