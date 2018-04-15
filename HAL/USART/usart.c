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
#include "timers.h"
#include "ui.h"

/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
**                             Global Vars                                    *
**----------------------------------------------------------------------------*/
/* USART3(UART_M5310)数据接收buffer */
extern uint8_t   UART_M5310_RxBuffer[512];
extern uint32_t  UART_M5310_RxBufferLen;

/* USART2(UART_CLI_DEBUG)数据接收buffer */
extern uint8_t   UART_CLI_RxBuffer[128];
extern uint32_t  UART_CLI_RxBufferLen;

/* USART1(UART_BLUETOOTH)数据接收buffer */
extern uint8_t   UART_BLE_RxBuffer[512];
extern uint32_t  UART_BLE_RxBufferLen;

extern TaskHandle_t cli_task;			/* CLI任务   */
extern TaskHandle_t bluetooth_task;		/* 蓝牙任务  */

bool	NB_DEBUG_FLAG  = false;		/* NB模组调试开关标志 为true时将串口接收到的模组数据转发至单片机调试串口 */
bool	BLE_DEBUG_FLAG = false;		/* BLE蓝牙模组调试标志位，为true时将串口接收到的模组数据转发至单片机调试串口 */

/* 蓝牙执行AT指令标志位 */
extern bool BLE_AT_EXE_FLAG;

extern TimerHandle_t bleAtExeTimer;	/* 蓝牙执行AT指令时的M5310串口定时器，定时器到达时将M5310串口接收buffer数据通过蓝牙发送到APP */

extern CM_MENU_POSITION menuPosition;

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
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority =1; 
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
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority =2; 
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
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority =0; 
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
	// BaseType_t taskWoken;
	
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		/* 接收数据并存储到UART3_RxBuffer中 */
		recvByte = USART_ReceiveData(USART3);
		
		/* 当开启NB模组调试模式时，将接收到的数据发送到调试串口 */
		if(NB_DEBUG_FLAG)
		{
			USART_ClearFlag(UART_CLI_DEBUG, USART_FLAG_TC);
			USART_SendData(UART_CLI_DEBUG, recvByte);
			while( USART_GetFlagStatus(UART_CLI_DEBUG, USART_FLAG_TC) == RESET );
		}
		
		if(recvByte == '\0')
		{
			return;
		}
		
		if(UART_M5310_RxBufferLen < sizeof(UART_M5310_RxBuffer)-1)
		{
			UART_M5310_RxBuffer[UART_M5310_RxBufferLen] = recvByte;
			UART_M5310_RxBufferLen ++;
		}
		else
		{
			_CMIOT_Debug("UART3_RxBuffer(UART_M5310) is full!");
		}
		
		/* M5310串口接收超时定时器 */
		if(BLE_AT_EXE_FLAG && (menuPosition.xPosition + menuPosition.yPosition * 3) == 5)
		{
			// xTimerStopFromISR(bleAtExeTimer, &taskWoken);
			xTimerStartFromISR(bleAtExeTimer, 0);
			xTimerResetFromISR(bleAtExeTimer, 0);
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
	// 
	uint8_t recvByte;
	BaseType_t bleNotifyValue;
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		/* 接收数据并存储到UART3_RxBuffer中 */
		recvByte = USART_ReceiveData(USART1);
		
		/* 当开启BLE蓝牙模组调试模式时，将接收到的数据发送到调试串口 */
		if(BLE_DEBUG_FLAG)
		{
			USART_ClearFlag(UART_CLI_DEBUG, USART_FLAG_TC);
			USART_SendData(UART_CLI_DEBUG, recvByte);
			while( USART_GetFlagStatus(UART_CLI_DEBUG, USART_FLAG_TC) == RESET );
		}
		/* 将数据存储到缓冲区 */
		if(UART_BLE_RxBufferLen < sizeof(UART_BLE_RxBuffer)-1)
		{
			UART_BLE_RxBuffer[UART_BLE_RxBufferLen] = recvByte;
			UART_BLE_RxBufferLen ++;
		}
		else
		{
			/* 如果溢出则清空缓存 */
			memset(UART_BLE_RxBuffer, 0, sizeof(UART_BLE_RxBuffer));
			UART_BLE_RxBufferLen = 0;
			
			/* 重头开始放入接收到数据 */
			UART_BLE_RxBuffer[UART_BLE_RxBufferLen] = recvByte;
			UART_BLE_RxBufferLen ++;
		}
		/* 接收到完整的请求内容后，向BLE蓝牙线程发送通知 */
		// if(strstr((const char*)UART_BLE_RxBuffer, "</Request>") != NULL && (menuPosition.xPosition + menuPosition.yPosition * 3) == 5)
		if(_CMIOT_Str_EndWith(UART_BLE_RxBuffer, (uint8_t *)"</Request>") && (menuPosition.xPosition + menuPosition.yPosition * 3) == 5)
		{
			vTaskNotifyGiveFromISR(bluetooth_task, &bleNotifyValue);   /* 向CLI任务发送任务通知 */
		}
	}
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
		/* 存储数据到CLI串口接收缓冲区 */
		if(UART_CLI_RxBufferLen < sizeof(UART_CLI_RxBuffer)-1)
		{
			USART_SendData(UART_CLI_DEBUG, recvByte);
			UART_CLI_RxBuffer[UART_CLI_RxBufferLen] = recvByte;
			UART_CLI_RxBufferLen ++;
		}
	}
}

