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

#include <stdio.h>
#include <stdlib.h>

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
#define GPIOREAD_PRIORITY 		(TASK_PRIORITY +1)
/*-------------------------------------------------------
 VARIABLES
 --------------------------------------------------------*/
/* Semaphores used by application to synchronize two tasks */
xSemaphoreHandle sem;
/* Text to display */
char text[8];
/* Counter start value*/
int count = 10;
int ErrorTime = 0;
uint8_t SetTime = 10;
uint32_t Timer0Value;

/* Variable for the Timer */
TimerHandle_t timer;
/***************************************************************************//**
 * @brief LcdPrint task which is showing numbers on the display
 * @param *pParameters pointer to parameters passed to the function
 ******************************************************************************/
/*----------------------------------------------------------
 FUNCTION DEFINITIONS
 */
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
			vTaskDelay(pdMS_TO_TICKS(1000));
		} else {

			count = (count - 1);
			text[0] = count >= 10 ? '1' : '0';
			text[1] = count % 10 + '0';
			text[2] = '\0';
			xSemaphoreGive(sem);
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}
}
/*static void GPIORead(void *pParameters) {

	if (GPIO_PinInGet(gpioPortB, 8)&& (pdTRUE == xSemaphoreTake(sem, portMAX_DELAY))) {
		ErrorTime = SetTime - Timer0Value; // Calculate the Delay
		SegmentLCD_Write(ErrorTime);
	}
	else {
		vTaskDelay(pdMS_TO_TICKS(1000));
	}

}*/
void Init_TIMER0(void) {
	CMU_ClockEnable(cmuClock_TIMER0, true);
	TIMER_IntEnable(TIMER0,true);
	TIMER_TopSet(TIMER0,39062); // Count for 512us*39062=19999744us ~20s
	TIMER_Init_TypeDef timerInit = { .enable = true, .debugRun = true,
			.prescale = timerPrescale1024, .clkSel = timerClkSelHFPerClk,
			.fallAction = timerInputActionNone, .riseAction =
					timerInputActionNone, .mode = timerModeUp, .dmaClrAct =
					false, .quadModeX4 = false, .oneShot = false, .sync = false,

	};
	TIMER_Init(TIMER0, &timerInit);

}
void Init_GPIO (void){ 			/* Initialize the GPIO */

		CMU_ClockEnable(cmuClock_GPIO, true); // Enable GPIO clock
		GPIO_PinModeSet(gpioPortB, 9, gpioModeInput, 0); // Configure the PB0 button as input
		NVIC_EnableIRQ(GPIO_EVEN_IRQn); // Enable the IT for the GPIO pins

}
void Init_UART(void){
	GPIO->P[5].MODEL |= GPIO_P_MODEL_MODE7_PUSHPULL; // Set PF7 high
	GPIO->P[5].DOUTSET = 1 << 7;
	CMU_ClockEnable(cmuClock_UART0,true); // Enable UART0 clock
	USART_InitAsync_TypeDef uart0_init = USART_INITASYNC_DEFAULT; // configure UART0 115200 baudrate, 8N1 format
		uart0_init.baudrate = 115200;
		uart0_init.refFreq = 0;
		uart0_init.databits = usartDatabits8;
		uart0_init.parity = usartNoParity;
		uart0_init.stopbits = usartStopbits1;
		uart0_init.mvdis = false;
		uart0_init.oversampling = usartOVS16;
		uart0_init.prsRxEnable = false;
		uart0_init.prsRxCh = 0;
		uart0_init.enable = usartEnable;
		USART_InitAsync(UART0, &uart0_init);
		GPIO_PinModeSet(gpioPortE, 0, gpioModePushPull, 1);
		UART0->ROUTE |= USART_ROUTE_TXPEN | USART_ROUTE_RXPEN;
			UART0->ROUTE |= USART_ROUTE_LOCATION_LOC1;
}
void GPIO_EVEN_IRQHandler (void){
	Timer0Value = TIMER_CounterGet(TIMER0);// read the actual timer value
	UARTSend ();
}
void UARTSend (void)
{
ErrorTime= (19531-Timer0Value)*512; // The real 0 moment is when the counter reach 19531, here I calculate the difference
USART_Tx(UART0,ErrorTime); // Send ErrorTime through UART0
}

/***************************************************************************//**
 * @brief  Main function
 ******************************************************************************/

int main(void) {
	/* Chip errata */
	CHIP_Init();
	CMU_HFRCOBandSet(cmuAUXHFRCOBand_14MHz); // Set High Freq. RC Oscilaator to 1 MHz

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
	/* Initialize TIMER0 */
	Init_TIMER0();
	Init_GPIO(); // Initialize GPIO Pin PB8 (Push-Button0 )
	Init_UART(); // Initialize UART0
	/* Create standard binary semaphore */

	vSemaphoreCreateBinary(sem);

	/* Create two task to show numbers from 10 to 4 */
	//xTaskCreate(GPIORead,(const char *)"GPIORead", STACK_SIZE_FOR_TASK, NULL,TASK_PRIORITY ,NULL);
	xTaskCreate(Count, (const char *) "Count", STACK_SIZE_FOR_TASK, NULL,
	TASK_PRIORITY, NULL);
	xTaskCreate(LcdPrint, (const char *) "LcdPrint", STACK_SIZE_FOR_TASK,
	NULL, TASK_PRIORITY, NULL);

	/* // Create the timer of the Delay Counter
	 timer = xTimerCreate("Delay",pdMS_TO_TICKS(20000),pdFALSE,(void*)NULL,GPIORead); */

	/* Start FreeRTOS Scheduler */
	vTaskStartScheduler();

	return 0;
}
