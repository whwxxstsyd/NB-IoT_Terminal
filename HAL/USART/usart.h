/*-----------------------------------------------------------------------------
File Name		:   usart.h
Author			:   zhaoji
Created Time	:   2018.01.12
Description		:   USART接口
-----------------------------------------------------------------------------*/


#ifndef __USART_H__
#define __USART_H__

/*---------------------------------------------------------------------------*
                               Dependencies                                  *
-----------------------------------------------------------------------------*/
#include "stm32f10x.h"


/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/
#define	UART_CLI_DEBUG          USART2    /* UART For FreeRTOS CLI and output Application Log */
#define	UART_M5310              USART3    /* UART is used to communicate with NB-IoT Module M5310 */
#define	UART_BLUETOOTH          USART1    /* UART is used to communicate with Bluetooth Module */


/*-----------------------------------------------------------------------------
Function Name	:	Usart_Init
Author			:	zhaoji
Created Time	:	2017.09
Description 	: 初始化指定USART接口
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_Uart_Init(USART_TypeDef *USARTx, uint32_t bandrate);


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_Uart_send
Author			:	zhaoji
Created Time	:	2017.09
Description 	: 指定UART口发送数据
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_Uart_send(USART_TypeDef *USARTx, uint8_t *senddata, uint32_t datalen);



#endif
