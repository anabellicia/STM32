#include <stm32f30x_gpio.h>
#include <stm32f30x_rcc.h>
#include <math.h>

GPIO_InitTypeDef GPIO_InitStructure;

volatile double a;

void _delay(uint32_t i)
{
	uint32_t j = 0;
	i *= 7.2;
	for(j = 0; j <= i; j ++);
}

void PWM(uint32_t tau, uint32_t T)
{
	GPIO_SetBits(GPIOE, GPIO_Pin_9);
	_delay(tau);
	GPIO_ResetBits( GPIOE, GPIO_Pin_9);
	_delay(T - tau);
}

void GPIO_INIT()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}

int main(void)
{
	GPIO_INIT();
    while(1)
    {
    	PWM((1 + sinf(a))* 500, 1000);
    	a+=0.01;
    	if(a >= 6.28) a=0;
    }
}
