#include <stm32f30x_gpio.h>
#include <stm32f30x_rcc.h>
#include <stm32f30x_tim.h>
#include <stm32f30x_misc.h>
#include <stm32f30x.h>
#include <stm32f30x_spi.h>

#define HIGH 	GPIOB->ODR |= (1 << 12);
#define LOW 	GPIOB->ODR &= ~(1 << 12);

GPIO_InitTypeDef GPIO_InitStruct;
TIM_OCInitTypeDef TIMPWM_InitStructure, TIMLED_InitStructure;
NVIC_InitTypeDef NVIC_LED_InitStructure, NVIC_SWITCHER_InitStructure;
SPI_InitTypeDef SPI_InitStruct;
PWM_InitTypeDef PWM_InitStructure;
	
void GPIO_INIT(void);
void NVIC_INIT(void);
void TIM_INIT(void);
void PWM_INIT(void);
void SPI_INIT(void);
void LED_Show(void);
uint16_t Switcher(void);
void PID_controller(void);

uint8_t mass[10] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };
volatile uint16_t Temp_flag = 0;
volatile double T_data = 0;
double temp = 0;

//ПИД-регулятор
//--------------------------------------------------------------
float Tst = 0;	  	//температура стабилизации
float T_disc = 0.5; //период дискретизации температуры
double tempDeviation = 0;
double deviationSumm = 0;
//коэффициенты ПИД-регулятора
uint8_t Kp = 3;
uint8_t Kip = 100;

void GPIO_INIT(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Pin = 0xFF00;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOE, &GPIO_InitStructure);

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = 0x000F;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void PWM_INIT(void)
{
	PWM_InitStructure.TIM_Pulse = 0;
	PWM_InitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	PWM_InitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	PWM_InitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
	PWM_InitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	PWM_InitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	PWM_InitStructure.TIM_OutputState = TIM_OutputState_Enable;
	PWM_InitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OC1Init(TIM1, &PWM_InitStructure);
}

void SPI_INIT(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_DataSize = SPI_Datasize_16b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_Init(SPI2, &SPI_InitStruct);
	SPI_cmd(SPI2, ENABLE);
}

void TIM_INIT(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
	
	TIMPWM_InitStructure.TIM_CounterMode= TIM_CounterMode_Up;
	TIMPWM_InitStructure.TIM_Prescaler= 36000;
	TIMPWM_InitStructure.TIM_Period= 1000;
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
	TIM_TimeBaseInit(TIM1, &TIMPWM_InitStructure);
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
	
	TIMLED_InitStructure.TIM_CounterMode= TIM_CounterMode_Up;
	TIMLED_InitStructure.TIM_Prescaler= 360;
	TIMLED_InitStructure.TIM_Period= 1000;
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
	TIM_TimeBaseInit(TIM1, &TIMLED_InitStructure);
}

void NVIC_INIT()
{

	NVIC_LED_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_LED_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_LED_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_LED_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_LED_InitStructure);
	
	NVIC_SWITCHER_InitStructure.NVIC_IRQChannel = TIM1_UP_TIM16_IRQn;
	NVIC_SWITCHER_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_SWITCHER_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_SWITCHER_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_SWITCHER_InitStructure);
}

void TIM1_UP_TIM16_IRQHandler(void)
{
	Temp_flag = 1;
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
}

void TIM7_IRQHandler(void)
{
	LED_Show(T_data);
	TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
}

uint16_t Switcher(void)
{
	uint16_t a = 0;
	uint8_t i = 0;

	for(i = 0; i <= 4; i++)
	{
		GPIOA->ODR &= 0xFF0F;
		GPIOA->ODR |= (1 << (7 - i));
		_delay_us(1000);
		a += ((uint8_t)(GPIOA -> IDR)&0x000F);
		a *= 10;
	}
	a /= 10;
	return a;
}

void PID_controller(void)
{
	if(Temp_flag)
	{
		T_data = (ReadRegister() >> 3)*0.25;
		Tst = Switcher();
		tempDeviation = (Tst - T_data);
		deviationSumm += tempDeviation;
			
		temp = Kp * (tempDeviation + Kip * T_disc * deviationSumm);
		if(temp < 0) temp = 0;
		if(temp > 1000) temp = 1000;
			
		TIM1->CCR2 = temp;
		Temp_flag = 0;
	}	
}

void LED_Show(double v)
{
	uint16_t be4P;
	be4P = (uint16_t)v;

	uint8_t v_mass[4] = {0, 0, 0, 0};
	uint8_t i = 0;
	v_mass[0]= (v - be4P)*10;

	for (i = 1; i < 4; i++)
	{
		v_mass[i]= be4P % 10;
		be4P /= 10;
	}

	for(i = 0; i < 4; i++)
	{
		GPIOC -> ODR |= 0x000F;
		GPIOE -> ODR &= 0x00FF;
		GPIOC -> ODR &= ~(1 << i);
		GPIOE -> ODR |= (~mass[v_mass[i]] << 8);
		if (i == 1)
		GPIOE -> ODR &= ~(1 << 15);
		_delay_ms(4);
	}
}

void _delay_ms(uint32_t j)
{
	j *= 7200;
	for (uint32_t i = 0; i < j; i++) {};
}

void _delay_us(uint32_t i)
{
	uint32_t j = 0;
	i*=7.2;
	for(j = 0; j <= i; j++){};
}

uint16_t ReadRegister()
{
	uint16_t data;
	HIGH
	LOW
	while (!(SPI2 ->SR &(SPI_I2S_FLAG_TXE)));
	SPI_I2S_SendData16(SPI2, 0x0000);
	while (!(SPI2 ->SR &(SPI_I2S_FLAG_TXE)));
	data= SPI2 -> DR;
	HIGH

	return data;
}

int main(void)
{
	GPIO_INIT();
	TIM_INIT();
	NVIC_INIT();
	
	SPI_INIT();
	PWM_INIT();
	
	TIM_Cmd(TIM1, ENABLE);
	TIM_Cmd(TIM7, ENABLE);

	while(1)
    {
		PID_controller();
    }
}

