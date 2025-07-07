#include <stm32f30x_gpio.h>
#include <stm32f30x_rcc.h>
#include <stm32f30x_misc.h>
#include <stm32f30x_tim.h>
#include <stm32f30x_spi.h>

uint8_t sendData;
uint8_t receiveData[2];
uint8_t tempByte;
uint16_t xResult;
uint8_t xSign;

double xPosition;

void GPIO_INIT();
void NVIC_INIT();
void TIM_INIT();
void SPI_INIT();
void writeData(uint8_t address, uint8_t dataToWrite);
uint8_t sendByte(uint8_t byteToSend);

int main(void)
{
GPIO_INIT();
SPI_INIT();
NVIC_INIT();
TIM_INIT();

xPosition = 0;

writeData(0x20, 0x0A);
writeData(0x23, 0x30);
	while(1)
		{

		}
}

void GPIO_INIT()
{
RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);
RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

GPIO_InitTypeDef GPIO_InitStruct;

GPIO_InitStruct.GPIO_Pin = 0xFF00;
GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_1;
GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
GPIO_Init(GPIOE, &GPIO_InitStruct);

GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_Init(GPIOA, &GPIO_InitStruct);

GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_5);
GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_5);
GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_5);

GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_Init(GPIOE, &GPIO_InitStruct);
}

void NVIC_INIT()
{
NVIC_InitTypeDef NVIC_IniT;

NVIC_IniT.NVIC_IRQChannel = TIM7_IRQn;
NVIC_IniT.NVIC_IRQChannelPreemptionPriority = 0;
NVIC_IniT.NVIC_IRQChannelSubPriority = 0;
NVIC_IniT.NVIC_IRQChannelCmd = ENABLE;
NVIC_Init(&NVIC_IniT);
}

void SPI_INIT()
{
RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
SPI_InitTypeDef SPI_InitStruct;

SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;
SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;
SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
SPI_InitStruct.SPI_CRCPolynomial = 7;
SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
SPI_Init(SPI1, &SPI_InitStruct);

SPI_Cmd(SPI1, ENABLE);

SPI_RxFIFOThresholdConfig(SPI1, SPI_RxFIFOThreshold_QF);
}
void TIM_INIT()
{
TIM_TimeBaseInitTypeDef TIM_InitStructure;
RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
TIM_InitStructure.TIM_Prescaler = 36000;
TIM_InitStructure.TIM_Period = 40;
TIM_TimeBaseInit(TIM7,&TIM_InitStructure);
TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
TIM_Cmd(TIM7, ENABLE);
}

void TIM7_IRQHandler(void)
{
GPIO_ResetBits(GPIOE, GPIO_Pin_3);
sendByte(0xE8);
receiveData[0] = sendByte(0x00);
receiveData[1] = sendByte(0x00);
GPIO_SetBits(GPIOE, GPIO_Pin_3);

xResult = (receiveData[0] | (receiveData[1] << 8)) - 10;
if ((xResult & 0x8000) == 0){xSign = 0;}
else
{
xSign = 1;
xResult &= 0x7FFF;
xResult = 0x8000 - xResult;
}
if (xResult < 0x20){xResult = 0;}
if (xSign == 0){xPosition += 0.07 * xResult * 0.025;}
else{xPosition -= 0.07 * xResult * 0.025;}
GPIO_Write(GPIOE, 0x0000);
if((xPosition>-105) && (xPosition<-75)) GPIO_SetBits(GPIOE, GPIO_Pin_8);
if((xPosition>-75) && (xPosition<-45)) GPIO_SetBits(GPIOE, GPIO_Pin_9);
if((xPosition>-45) && (xPosition<-15)) GPIO_SetBits(GPIOE, GPIO_Pin_10);
if((xPosition>-15) && (xPosition<15)) GPIO_SetBits(GPIOE, GPIO_Pin_11);
if((xPosition>15) && (xPosition<45)) GPIO_SetBits(GPIOE, GPIO_Pin_12);
if((xPosition>45) && (xPosition<75)) GPIO_SetBits(GPIOE, GPIO_Pin_13);
if((xPosition>75) && (xPosition<105)) GPIO_SetBits(GPIOE, GPIO_Pin_14);

TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
}
uint8_t sendByte(uint8_t byteToSend)
{
while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET){}
SPI_SendData8(SPI1, byteToSend);
while ((SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)){}
return (uint8_t)SPI_ReceiveData8(SPI1);
}

void writeData(uint8_t address, uint8_t dataToWrite)
{
GPIO_ResetBits(GPIOE, GPIO_Pin_3);
sendByte(address);
sendByte(dataToWrite);
GPIO_SetBits(GPIOE, GPIO_Pin_3);
}
