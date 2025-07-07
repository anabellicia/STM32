#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "queue.h"
#include "stm32f30x.h"
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"


void Port_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = 0xFF00;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = 0x0001;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

xQueueHandle xQueue1;
unsigned char i = 1;
unsigned char f = 0;
void LED(xCoRoutineHandle xHandle, unsigned portBASE_TYPE uxIndex )
{
	portBASE_TYPE xResult;
	unsigned char j = 0;
	crSTART( xHandle );
	while(1)
	{
		crQUEUE_RECEIVE(xHandle, xQueue1, &j, portMAX_DELAY, &xResult);
		if (xResult == pdTRUE)
		{
			if(j) {GPIOE->ODR |= 1<<9;}
			else {GPIOE->ODR &= ~(1<<9);}
		}
	}
	crEND();
}

void BUTTON(xCoRoutineHandle xHandle, unsigned portBASE_TYPE uxIndex )
{
	portBASE_TYPE xResult;
	crSTART( xHandle );
	while(1)
	{
		if( ((GPIOA->IDR & (1<<0)) == 1) && (f == 0))
		{
			crDELAY(xHandle, 50);
			if( ((GPIOA->IDR & (1<<0)) == 1) && (f == 0))
			{
				crQUEUE_SEND(xHandle, xQueue1, &i, portMAX_DELAY, &xResult);
				i ^= 1;
				f = 1;
			}
		}
		if( ((GPIOA->IDR & (1<<0)) == 0) && (f == 1))
		{
			f = 0;
		}
		crDELAY(xHandle, 50);
	}
	crEND();
}

int main(void)
{

	Port_Init();
	xQueue1 = xQueueCreate(1, sizeof(unsigned char));

	xCoRoutineCreate(LED,	 1, 0);
	xCoRoutineCreate(BUTTON, 1, 0);

	vTaskStartScheduler();

    while(1)
    {}
}

void vApplicationIdleHook(void)
{
	while(1)
	{
		vCoRoutineSchedule();
	}
}