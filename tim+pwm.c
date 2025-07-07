#include <stm32f30x_gpio.h>
#include <stm32f30x_rcc.h>
#include <stm32f30x_tim.h>
#include <stm32f30x_misc.h>

TIM_OCInitTypeDef PWM_InitStructure;
TIM_TimeBaseInitTypeDef TIMPWM_InitStructure;
NVIC_InitTypeDef NVICPWM_InitStructure;
GPIO_InitTypeDef GPIOPWM_InitStructure;

volatile double a = 0;

void _delay(uint32_t i)
{
	uint32_t j = 0;
	i *= 7.2;
	for(j = 0; j <= i; j ++);
}
void TIMPWM_INIT()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	TIMPWM_InitStructure.TIM_CounterMode= TIM_CounterMode_Up;
	TIMPWM_InitStructure.TIM_Prescaler= 7200;
	TIMPWM_InitStructure.TIM_Period= 100;
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
	TIM_TimeBaseInit(TIM1, &TIMPWM_InitStructure);

	TIM_Cmd(TIM1, ENABLE);
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
}
void GPIOPWM_INIT()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	GPIOPWM_InitStructure.GPIO_Pin= GPIO_Pin_0;
	GPIOPWM_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIOPWM_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIOPWM_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIOPWM_InitStructure.GPIO_OType = GPIO_OType_PP;

	GPIO_Init(GPIOB, &GPIOPWM_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_2);
}
void PWM_INIT()
{
	PWM_InitStructure.TIM_Pulse= 11;
	PWM_InitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	PWM_InitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	PWM_InitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
	PWM_InitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	PWM_InitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	PWM_InitStructure.TIM_OutputState = TIM_OutputState_Enable;
	PWM_InitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OC1Init(TIM1, &PWM_InitStructure);
}

void NVICPWM_INIT()
{
	NVICPWM_InitStructure.NVIC_IRQChannel = TIM1_UP_TIM16_IRQn;
	NVICPWM_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVICPWM_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVICPWM_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVICPWM_InitStructure);
}

void PWM(uint32_t tau, uint32_t T)
{
	GPIO_SetBits(GPIOE, GPIO_Pin_9);
	_delay(tau);
	GPIO_ResetBits( GPIOE, GPIO_Pin_9);
	_delay(T - tau);
}

void TIM1_UP_TIM16_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM1,TIM_IT_Update);
	PWM((1 + sinf(a))* 500, 1000);
	a+=0.01;
	if(a >= 6.28) a=0;
	TIM1 -> CCR1 = a;
}
int main(void)
{

    while(1)
    {
    }
}
