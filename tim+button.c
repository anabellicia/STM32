#include <stm32f30x_gpio.h>
#include <stm32f30x_rcc.h>
#include <stm32f30x_tim.h>
#include <stm32f30x_misc.h>

GPIO_InitTypeDef GPIO_InitStructure;
NVIC_InitTypeDef NVIC_LED_InitStructure;
TIM_TimeBaseInitTypeDef TIM_LED_InitStructure;

void GPIO_INIT()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin= GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin= GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void NVIC_INIT()
{
	NVIC_LED_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_LED_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_LED_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_LED_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_LED_InitStructure);
}

void TIM_INIT()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
	TIM_LED_InitStructure.TIM_CounterMode= TIM_CounterMode_Up;
	TIM_LED_InitStructure.TIM_Prescaler= 36000;
	TIM_LED_InitStructure.TIM_Period= 1000;
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
	TIM_TimeBaseInit(TIM7, &TIM_LED_InitStructure);

	TIM_Cmd(TIM7, ENABLE);
}

void button()
{
	if((GPIOA-> IDR & (1<<0) == 1) && (f==0))
	{
		_delay(50000);
		if(((GPIOA->IDR & (1<<0)) == 1) && (f==0))
		{
			GPIOE->IDR & (1<<0) ^= (1<<8);
			f=1;
		}
		if(((GPIOA->IDR & (1<<0)) == 0) && (f == 1))
		{
			f=0;
		}
	}
}

void TIM7_IRQHandler(void)
{
	button();
	TIM_ClearITPendingBit(TIM7,TIM_IT_Update);
}

int main(void)
{
    while(1)
    {
    }
}
