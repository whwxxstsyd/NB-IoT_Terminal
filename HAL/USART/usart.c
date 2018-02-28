/*-----------------------------------------------------------------------------
File Name   	:   usart.c
Author          :   zhaoji
Created Time    :   2018.01.12
Description     :   USART接口
-----------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
                               Dependencies                                  *
-----------------------------------------------------------------------------*/
#include "usart.h"
#include "common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"

/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
**                             Global Vars                                    *
**----------------------------------------------------------------------------*/
/* USART3(UART_M5310)数据接收buffer */
extern uint8_t   UART_M5310_RxBuffer[128];
extern uint32_t  UART_M5310_RxBufferLen;

/* USART2(UART_CLI_DEBUG)数据接收buffer */
extern uint8_t   UART_CLI_RxBuffer[128];
extern uint32_t  UART_CLI_RxBufferLen;

extern TaskHandle_t cli_task;     /* CLI任务 */

/*----------------------------------------------------------------------------*
**                             Local Vars                                     *
**----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
Function Name	:	Usart_Init
Author			:	zhaoji
Created Time	:	2017.09
Description 	: 	初始化指定USART接口
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_Uart_Init(USART_TypeDef *USARTx, uint32_t bandrate)
{
	if(USARTx == USART1)
	{
		/* GPIO、USART、NVIC初始化结构体 */
		GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
		
		USART_DeInit(USART1);
		
		/* 开启串口和GPIO时钟 */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
		
		/* 配置串口TX */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		/* 配置串口RX */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		/* 配置串口参数，并使能串口 */
		USART_InitStructure.USART_BaudRate = bandrate;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
		USART_Init(USART1, &USART_InitStructure);
		USART_Cmd(USART1, ENABLE);
		
		/* 初始化NVIC */
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority =3; 
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		
		/* 配置接收数据中断 */
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	}
	else if(USARTx == USART2)
	{
		/* GPIO、USART、NVIC初始化结构体 */
		GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
		
		USART_DeInit(USART2);
		
		/* 开启串口和GPIO时钟 */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
		
		/* 配置串口TX */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		/* 配置串口RX */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		/* 配置串口参数，并使能串口 */
		USART_InitStructure.USART_BaudRate = bandrate;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
		USART_Init(USART2, &USART_InitStructure);
		USART_Cmd(USART2, ENABLE);
		
		/* 初始化NVIC */
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority =3; 
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		
		/* 配置接收数据中断 */
		USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	}
	else if(USARTx == USART3)
	{
		/* GPIO、USART、NVIC初始化结构体 */
		GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
		
		USART_DeInit(USART3);
		
		/* 开启串口和GPIO时钟 */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
		
		/* 配置串口TX */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		
		/* 配置串口RX */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		
		/* 配置串口参数，并使能串口 */
		USART_InitStructure.USART_BaudRate = bandrate;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
		USART_Init(USART3, &USART_InitStructure);
		USART_Cmd(USART3, ENABLE);
		
		/* 初始化NVIC */
		NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority =3; 
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		
		/* 配置接收数据中断 */
		USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	}
	else if(USARTx == UART4)
	{
		//
	}
	else if(USARTx == UART5)
	{
		//
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_Uart_send
Author			:	zhaoji
Created Time	:	2018.02.12
Description 	: 	指定UART口发送数据
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_Uart_send(USART_TypeDef *USARTx, uint8_t *senddata, uint32_t datalen)
{
	uint32_t i = 0;
	
	taskENTER_CRITICAL();   /* 进入临界区 */
	
	for(i=0; i<datalen; i++)
	{
		USART_ClearFlag(USARTx, USART_FLAG_TC);
		USART_SendData(USARTx, *(senddata + i));
		while( USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET );
	}
	
	taskEXIT_CRITICAL();   /* 退出临界区 */
}



/*-----------------------------------------------------------------------------
Function Name	:	USART3_IRQHandler
Author			:	zhaoji
Created Time	:	2018.02.13
Description 	: 	USART3中断处理函数(UART_M5310)
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void USART3_IRQHandler(void)
{
	uint8_t recvByte;
	
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		/* 接收数据并存储到UART3_RxBuffer中 */
		recvByte = USART_ReceiveData(USART3);
		if(UART_M5310_RxBufferLen < sizeof(UART_M5310_RxBuffer)-1)
		{
			UART_M5310_RxBuffer[UART_M5310_RxBufferLen] = recvByte;
			UART_M5310_RxBufferLen ++;
		}
		else
		{
			_CMIOT_Debug("UART3_RxBuffer(UART_M5310) is full!");
		}
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	USART1_IRQHandler
Author			:	zhaoji
Created Time	:	2018.02.13
Description 	: 	USART1接收中断处理函数(UART_BLUETOOTH)
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void USART1_IRQHandler(void)
{
	//Bluetooth COM Port
}


/*-----------------------------------------------------------------------------
Function Name	:	USART2_IRQHandler
Author			:	zhaoji
Created Time	:	2018.02.13
Description 	: 	USART2接收中断处理函数(UART_CLI_DEBUG)
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void USART2_IRQHandler(void)
{
	uint8_t recvByte;
	
	BaseType_t cliNotifyValue;
	
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		/* 接收数据并存储到UART_CLI_RxBufferLen中 */
		recvByte = USART_ReceiveData(USART2);
		
		if(recvByte == '\r' || recvByte == '\n')    /* 接收到CLI命令结束符 */
		{
			USART_SendData(UART_CLI_DEBUG, recvByte);
			vTaskNotifyGiveFromISR(cli_task, &cliNotifyValue);   /* 向CLI任务发送任务通知 */
			return;
		}
		
		if(recvByte == '\b')    /* 接收删除符 */
		{
			USART_SendData(UART_CLI_DEBUG, recvByte);
			if(UART_CLI_RxBufferLen > 0)
			{
				UART_CLI_RxBufferLen --;
				UART_CLI_RxBuffer[UART_CLI_RxBufferLen] = '\0';
			}
			return;
		}
		
		if(UART_CLI_RxBufferLen < sizeof(UART_CLI_RxBuffer)-1)
		{
			USART_SendData(UART_CLI_DEBUG, recvByte);
			UART_CLI_RxBuffer[UART_CLI_RxBufferLen] = recvByte;
			UART_CLI_RxBufferLen ++;
		}
	}
}

