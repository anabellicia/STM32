#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"

#include "stm32f30x.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "portmacro.h"

#include "stm32f30x_rcc.h"
#include "stm32f30x_gpio.h"

xQueueHandle queue;

void GPIO_LED_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Pin = 0xFF00;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}

void Task_1(void *pvParameters)
{
	while(1)
	{
		GPIOE->ODR ^= 1<<15;
		vTaskDelay(100);
	}
}

char pcWriteBuffer[1024];
void Task_2(void *pvParameters)
{
	unsigned int i = 0;
	unsigned char k = 0;
	while(1)
	{
		GPIOE->ODR ^= 1<<13;

		vTaskList(&pcWriteBuffer[0]);

		for(i = 0; pcWriteBuffer[i] !='\0'; i++)
		{
			USB_Send_Data(pcWriteBuffer[i]);
			vTaskDelay(10);
		}

		USB_Send_Data(0x0d);
		vTaskDelay(10);
		USB_Send_Data(0x0a);

		if(xQueueReceive(queue, &k, 0) == pdPASS)
		{
			GPIOE->ODR ^= 1<<(k-48);
		}

		vTaskDelay(1000);
	}
}

int main(void)
{
	Set_System();
	Set_USBClock();
	USB_Interrupts_Config();
	USB_Init();

	GPIO_LED_Init();
	queue = xQueueCreate(1, sizeof(unsigned char));
	xTaskCreate( Task_1, ( const char * ) "task_1", 1024, NULL, 2, (xTaskHandle *) NULL);
	xTaskCreate( Task_2, ( const char * ) "task_2", 1024, NULL, 2, (xTaskHandle *) NULL);

	vTaskStartScheduler();
    while(1){}
}