#include <stm32f30x.h>
#include <stm32f30x_gpio.h>
#include <stm32f30x_misc.h>
#include <stm32f30x_i2c.h>

#define I2C1_PORT	GPIOB
#define I2C1_SCL	GPIO_Pin_6
#define I2C1_SDA	GPIO_Pin_7

#define I2C2_PORT	GPIOA
#define I2C2_SCL	GPIO_Pin_9
#define I2C2_SDA	GPIO_Pin_10

#define MASTER_ADDR	0x08
#define SLAVE_ADDR	0b1000000

volatile uint8_t adres = 0;
volatile uint8_t data = 0;

volatile uint8_t flag_adres = 0;
volatile uint8_t flag_data = 0;

void I2C1_WriteByte(uint8_t HardSlaveAddr, uint8_tExtSlaveAddress, uint8_t Byte)
{
	while(I2C_GetFlagStatus(I2C1, I2C_ISR_BUSY) != RESET){};

	I2C_TransferHandling(I2C1, HardSlaveAddr, 1, I2C_Reload_Mode, I2C_Generate_Start_Write);
	while(I2C_GetFlagStatus(I2C1, I2C_ISR_TXE) == RESET){};

	I2C_TransferHandling(I2C1, HardSlaveAddr, 2, I2C_AutoEnd_Mode, I2C_NoStartStop );
	while(I2C_GetFlagStatus(I2C1, I2C_ISR_TXE) == RESET){};

	I2C_SendData(I2C1, ExtSlaveAddress);
	while(I2C_GetFlagStatus(I2C1, I2C_ISR_TXE) == RESET){};

	I2C_SendData(I2c1, Byte);
	while(I2C_GetFlagStatus(I2C1, I2C_ISR_TXE) == RESET){};

	I2C_GenerateSTOP(I2C1, ENABLE);
	while(I2C_GetFlagStatus(I2C1, I2C_ISR_TXE) == RESET){};
	I2C_ClearFlag(I2C1, I2C_ICR_STOPCF);
}

void I2C2_Init()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitTypeDef GPIO_I2C2;
	GPIO_I2C2.GPIO_Pin	=	I2C2_SCL | I2C2_SDA;
	GPIO_I2C2.GPIO_Speed	=	GPIO_Speed_Level_1;
	GPIO_I2C2.GPIO_Mode		=	GPIO_Mode_AF;
	GPIO_I2C2.GPIO_OType	= 	GPIO_OType_OD;
	GPIO_I2C2.GPIO_PuPd		=	GPIO_PuPd_UP;
	GPIO_Init(I2C2_PORT, &GPIO_I2C2);

	GPIO_PinAFConfig(I2C2_PORT, GPIO_PinSource9, GPIO_AF_4);
	GPIO_PinAFConfig(I2C2_PORT, GPIO_PinSource10, GPIO_AF_4);

	I2C_InitTypeDef I2C2_Struct;
	I2C_DeInit(I2C2);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
	RCC_I2CCLKConfig(RCC_I2C2CLK_HSI);
	I2C2_Struct.I2C_AnalogFilter	=	I2C_AnalogFilter_Enable;
	I2C2_Struct.I2C_Timing	=	0xB042181C;
	I2C2_Struct.I2C_OwnAddress1	=	SLAVE_ADDR;
	I2C2_Struct.I2C_Ack	=	I2C_Ack_Enable;
	I2C2_Struct.I2C_Mode =	I2C_Mode_I2C;
	I2C2_Struct.I2C_DigitalFilter	=	0;
	I2C2_Struct.I2C_AcknowledgedAddress	=	I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2C2, &I2C2_Struct);
	I2C_ITConfig(I2C2, I2C_IT_ADDR|I2C_IT_RXNE, ENABLE);
	I2C_Cmd(I2C2, ENABLE);
}

void I2C2_EV_IRQHandler()
{
	if(I2C_GetFladStatus(I2C2, I2CFLAG_ADDR))
	{
		I2C_ClearFlag(I2C2, I2C_FLAG_ADDR);
		return;
	}
	if(I2C_GetFladStatus(I2C2, I2C_FLAG_RXNE) && (flag_data == 0))
	{
		flag_data = 1;
		adres = I2C_ReceiveData(I2C2);
		I2C_ClearFlag(I2c2, I2C_Flag_RXNE);
		return;
	}
	if(I2C_GetFladStatus(I2C2, I2C_FLAG_RXNE) && (flag_data == 1))
	{
		flag_data = 0;
		data = I2C_ReceiveData(I2C2);
		if(data == 0x01) {GPIOE->ODR |= 1 << (adres +8);}
		else if(data == 0x00) {GPIOE->ODR &= ~(1<<(adres +8));}
		I2C_ClearFlag(I2C2, I2c_FLAG_RXNE);
		return;
	}
}

void GPIO_LED_Init()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = 0xFF00;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}

void I2C_Interrupt_Config()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel	=	I2C2_EV_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd 	=	ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void I2C2_Init()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitTypeDef GPIO_I2C1;
	GPIO_I2C1.GPIO_Pin	=	I2C1_SCL | I2C1_SDA;
	GPIO_I2C1.GPIO_Speed	=	GPIO_Speed_Level_1;
	GPIO_I2C1.GPIO_Mode		=	GPIO_Mode_AF;
	GPIO_I2C1.GPIO_OType	= 	GPIO_OType_OD;
	GPIO_I2C1.GPIO_PuPd		=	GPIO_PuPd_UP;
	GPIO_Init(I2C1_PORT, &GPIO_I2C1);

	GPIO_PinAFConfig(I2C1_PORT, GPIO_PinSource6, GPIO_AF_4);
	GPIO_PinAFConfig(I2C1_PORT, GPIO_PinSource7, GPIO_AF_4);

	I2C_InitTypeDef I2C1_Struct;
	I2C_DeInit(I2C2);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);
	I2C1_Struct.I2C_AnalogFilter	=	I2C_AnalogFilter_Enable;
	I2C1_Struct.I2C_Timing	=	0xB042181C;
	I2C1_Struct.I2C_OwnAddress1	=	MASTER_ADDR;
	I2C1_Struct.I2C_Ack	=	I2C_Ack_Enable;
	I2C1_Struct.I2C_Mode =	I2C_Mode_I2C;
	I2C1_Struct.I2C_DigitalFilter	=	0;
	I2C1_Struct.I2C_AcknowledgedAddress	=	I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2C1, &I2C1_Struct);
	I2C_Cmd(I2C1, ENABLE);
}
int main(void)
{
	I2C1_Init();
	I2C2_Init();
	I2C_Interrupt_Config();
	GPIO_LED_Init();
	I2C1_WriteBite(SLAVE_ADDR, 0x01, 0x01);
    while(1)
    {

    }
}
