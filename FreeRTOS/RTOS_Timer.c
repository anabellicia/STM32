#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "croutine.h"
#include "stm32f30x.h"
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include "semihosting.h"
#include "stm32f30x_misc.h"
#include "stm32f30x_exti.h"
#include "stm32f30x_syscfg.h"
int button_flag = 0;
int flag = 0;

void Port_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Pin = 0xFF00;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}

xTimerHandle RTOS_tim;
char pcWriteBuffer[1024];

uint32_t factorial(uint32_t n)
{
    if (n == 0 || n == 1) return 1;
    return n * factorial(n - 1);
}

void TimerFunction_reload(xTimerHandle RTOS_tim)
{
	vTaskList((char *)pcWriteBuffer);
	SH_SendString(pcWriteBuffer);
	GPIOE->ODR ^= 1<<13;
}
void Task_1(void *pvParameters)
{
	while(1)
	{
		vTaskDelay(230);
		GPIOE->ODR ^= 1<<10;
	}
}
void Task_2(void *pvParameters)
{
	uint8_t i = 0;
	while(1)
	{
		vTaskDelay(1000);
		GPIOE->ODR ^= 1<<9;
		factorial(i);
		i++;
	}
}

int main(void)
{
	Port_Init();

	RTOS_tim = xTimerCreate( "TIM_2", 1000,  pdTRUE, 0, TimerFunction_reload);
	xTaskCreate( Task_1, ( signed char * ) "task_1", 1024, NULL, 3, (xTaskHandle *) NULL);
	xTaskCreate( Task_2, ( signed char * ) "task_2", 1024, NULL, 3, (xTaskHandle *) NULL);
	xTimerStart(RTOS_tim, 0);
	vTaskStartScheduler();

    while(1)
    {}
}