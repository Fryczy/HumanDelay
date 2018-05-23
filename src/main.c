/***************************************************************************//**
 * @file
 * @brief FreeRTOS Tickless Demo for Energy Micro EFM32GG_STK3700 Starter Kit
 * @version 5.4.0
 *******************************************************************************
 * # License
 * <b>Copyright 2015 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#include "stdio.h"
#include <stdlib.h>
#include "retargetserial.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "croutine.h"
#include "em_gpio.h"
#include "em_chip.h"
#include "em_timer.h"
#include "em_system.h"
#include "em_device.h"
#include  "em_cmu.h"
#include "bsp_trace.h"
#include "segmentlcd.h"
#include  "timers.h"
#include "sleep.h"
#include "em_usart.h"
#include "em_device.h"

#define STACK_SIZE_FOR_TASK    (configMINIMAL_STACK_SIZE + 10)
#define TASK_PRIORITY          (tskIDLE_PRIORITY + 1)

/* Semaphores used by application to synchronize two tasks */
xSemaphoreHandle sem;
/* Text to display */
char text[8];
/* Counter start value*/
int count = 10;
volatile float ErrorTime = 0;
uint8_t SetTime = 10;
volatile uint32_t Timer0Value;
volatile float sec = 0;
//TaskHandle_t handleSend;
TaskHandle_t handleLCDPrint;
TaskHandle_t handleCount;


/***************************************************************************//**
 * @brief LcdPrint task which is showing numbers on the display
 * @param *pParameters pointer to parameters passed to the function
 ******************************************************************************/
static void LcdPrint(void *pParameters) {
	(void) pParameters; /* to quiet warnings */

	for (;;) {
		/* Wait for semaphore, then display next number */
		if (pdTRUE == xSemaphoreTake(sem, portMAX_DELAY)) {
			SegmentLCD_Write(text);
		}
	}
}

/***************************************************************************//**
 * @brief Count task which is preparing next number to display
 * @param *pParameters pointer to parameters passed to the function
 ******************************************************************************/
static void Count(void *pParameters) {
	(void) pParameters; /* to quiet warnings */

	for (;;) {
		if (count <= 4) {
			text[0] = 'W';
			text[1] = 'A';
			text[2] = 'I';
			text[4] = 'T';
			text[5] = '\0';
			xSemaphoreGive(sem);
			//vTaskSuspend(NULL);
		} else {

			count = (count - 1);
			text[0] = count >= 10 ? '1' : '0';
			text[1] = count % 10 + '0';
			text[2] = '\0';
			xSemaphoreGive(sem);
			vTaskDelay(pdMS_TO_TICKS(1000));
			//vTaskDelayUntil(&LastWakeTime,TikcFreq);
		}
	}
}
void Init_TIMER0(void) {
	CMU_ClockEnable(cmuClock_TIMER0, true);
	TIMER_IntEnable(TIMER0, true);
	TIMER_TopSet(TIMER0, 13672); // Count for 73,1us*13672=0,9994232~1s
	TIMER_Init_TypeDef timerInit = { .enable = true, .debugRun = true,
			.prescale = timerPrescale1024, .clkSel = timerClkSelHFPerClk,
			.fallAction = timerInputActionNone, .riseAction =
					timerInputActionNone, .mode = timerModeUp, .dmaClrAct =
			false, .quadModeX4 = false, .oneShot = false, .sync = false,

	};
	TIMER_Init(TIMER0, &timerInit);
	NVIC_EnableIRQ(TIMER0_IRQn);
	TIMER_IntEnable(TIMER0, TIMER_IF_OF);

}

void Init_GPIO(void) { /* Initialize the GPIO */

	CMU_ClockEnable(cmuClock_GPIO, true); // Enable GPIO clock
	GPIO_PinModeSet(gpioPortB, 9, gpioModeInput, 0); // Configure the PB0 button as input, it's connected to PB9
	GPIO_IntConfig(gpioPortB, 9, true, false, true); // Enable rising edge interrupt for PB0
	NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
	NVIC_EnableIRQ(GPIO_EVEN_IRQn); // Enable the IT for the GPIO pins
	// Enable interrupt in CPU for odd gpio ITs
	NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
	NVIC_EnableIRQ(GPIO_ODD_IRQn);
	NVIC_SetPriority(GPIO_ODD_IRQn, 0);

}

void TIMER0_IRQHandler(void) {
	sec++;
	TIMER_IntClear(TIMER0, TIMER_IF_OF);
}
void GPIO_ODD_IRQHandler(void) {
	Timer0Value = TIMER_CounterGet(TIMER0); // read the actual timer value
	ErrorTime=Timer0Value*0.0000000731;
	ErrorTime = 10-(ErrorTime+sec);

	//UARTSend();
	//vTaskResume(handleSend); // Resume Send Task
	printf("%f",ErrorTime);
	//vTaskResume(handleCount);
	GPIO_IntClear(1 << 9);
}
/***************************************************************************//**
 * @brief  Main function
 ******************************************************************************/
int main(void) {
	/* Chip errata */
	CHIP_Init();
	Init_GPIO();
	Init_TIMER0();
	RETARGET_SerialInit();
	RETARGET_SerialCrLf(1);
	/* If first word of user data page is non-zero, enable Energy Profiler trace */
	BSP_TraceProfilerSetup();

	/* Initialize SLEEP driver, no calbacks are used */
	SLEEP_Init(NULL, NULL);
#if (configSLEEP_MODE < 3)
	/* do not let to sleep deeper than define */
	SLEEP_SleepBlockBegin((SLEEP_EnergyMode_t) (configSLEEP_MODE + 1));
#endif

	/* Initialize the LCD driver */
	SegmentLCD_Init(false);

	/* Create standard binary semaphore */
	vSemaphoreCreateBinary(sem);

	/* Create two task to show numbers from 0 to 15 */
	xTaskCreate(Count, (const char *) "Count", STACK_SIZE_FOR_TASK, NULL,
	TASK_PRIORITY, NULL);
	xTaskCreate(LcdPrint, (const char *) "LcdPrint", STACK_SIZE_FOR_TASK, NULL,
	TASK_PRIORITY, NULL);

	/* Start FreeRTOS Scheduler */
	vTaskStartScheduler();

	return 0;
}
