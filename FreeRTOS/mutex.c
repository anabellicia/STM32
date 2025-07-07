/* Lesson8Classwork1
 * Продемонстрируем программу, имитирующую опрос датчиков температуры (Т) и давления (Р) и
 * передающую измеренные данные по USB в терминал компьютера.
 * Для работы с каждым из датчиков создается своя задача, каждая из которых в своем теле обращается
 * к единому ресурсу передачи данных — USB. В задачах доступ к интерфейсу должен регулироваться мьютексом.
 */

#include <hw_config.h>
#include <usb_lib.h>
#include <usb_desc.h>
#include <usb_pwr.h>

#include "stm32f30x.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "stm32f30x_rcc.h"
#include "stm32f30x_gpio.h"

xSemaphoreHandle mutex;

void Task_1(void *pvParameters)
{
	char str[40] = "Temperature = 23 C.\0";
	unsigned char i = 0;
	while(1)
	{
		xSemaphoreTake(mutex, portMax_DELAY);
		for(i=0; str[i]!='\0'; i++)
		{
			USB_Send_Data(str[i]);
			vTaskDelay(10);
		}
		USB_Send_Data(0x0d);
		vTaskDelay(10);
		USB_Send_Data(0x0a);
		xSemaphoreGive(mutex);
		vTaskDelay(1000);
	}
}

void Task_2(void *pvParameters)
{
	char str[40] = "Pressure = 736 mm\0";
	unsigned char i = 0;
	while(1)
	{
		xSemaphoreTake(mutex, portMax_DELAY);
		for(i=0; str[i]!='\0'; i++)
		{
			USB_Send_Data(str[i]);
			vTaskDelay(10);
		}
		USB_Send_Data(0x0d);
		vTaskDelay(10);
		USB_Send_Data(0x0a);
		xSemaphoreGive(mutex);
		vTaskDelay(1000);
	}
}

int main(void)
{
	Set_System();
	Set_USBClock();
	USB_Interrupts_Config();
	USB_Init();

	Mutex = xSemaphoreCreateMutex();
	xTaskCreate(Task_1, (char *) "task_1", 1024, NULL,2, (xTaskHandle *) NULL);
	xTaskCreate(Task_2, (char *) "task_2", 1024, NULL,2, (xTaskHandle *) NULL);
	vTaskStartScheduler();
	while(1){}
}

//Заглушки
void vApplicationIdleHook ( void ){}
void vApplicationMallocFailedHook ( void ){for ( ;; );}
void vApplicationStackOverflowHook ( xTaskHandle pxTask, char * pcTaskName )
{( void ) pcTaskName;
( void ) pxTask;
for ( ;; );}
void vApplicationTickHook ( void ){}