/*-----------------------------------------------------------------------------
File Name   	:   key.c
Author          :   zhaoji
Created Time    :   2018.03.16
Description     :   按键接口
-----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*
                               Dependencies                                   *
------------------------------------------------------------------------------*/
#include "key.h"
#include "stm32f10x.h"
#include "delay.h"
#include "sys.h"
#include "FreeRTOS.h"
#include "task.h"
#include "common.h"
#include "ui.h"
#include "usart.h"
#include "string.h"

#define KEY_LEFT	PBin(4)
#define KEY_RIGHT	PCin(12)
#define KEY_UP		PBin(3)
#define KEY_DOWN	PDin(2)
#define KEY_ENTER	PCin(11)

/*----------------------------------------------------------------------------*
**                             Global Vars                                    *
**----------------------------------------------------------------------------*/
extern	TaskHandle_t m5310_task;     	/* M5310任务  */
extern	CM_MENU_POSITION menuPosition;	/* 菜单坐标信息 */

extern	bool CM_UI_BUSY;	/* UI界面切换状态忙标识（此时不响应按键请求） */

extern bool FACTORY_MODE_FLAG;		/* 工厂生产测试模式标志位 */

/*-----------------------------------------------------------------------------
Function Name	:	cm_key_init
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	初始化按键
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void cm_key_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;//定义结构体
	EXTI_InitTypeDef EXTI_InitStructure;//定义初始化结构体
	NVIC_InitTypeDef NVIC_InitStructure;//定义结构体
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//使能时钟
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 
	DBGMCU_Config(((uint32_t)0x00000020),DISABLE);/* 解决仿真时JTAG复用口无法输出的问题 */
	
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_3;//选择IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//设置成上拉输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);//使用结构体信息进行初始化IO口
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource3);

	EXTI_InitStructure.EXTI_Line=EXTI_Line3; //中断线的标号 取值范围为EXTI_Line0~EXTI_Line15
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断模式，可选值为中断 EXTI_Mode_Interrupt 和事件 EXTI_Mode_Event。
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//触发方式，可以是下降沿触发 EXTI_Trigger_Falling，上升沿触发 EXTI_Trigger_Rising，或者任意电平（上升沿和下降沿）触发EXTI_Trigger_Rising_Falling
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);//根据结构体信息进行初始化

	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn; //使能外部中断所在的通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4; //子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); //根据结构体信息进行优先级初始化
	
	
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;//选择IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//设置成上拉输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);//使用结构体信息进行初始化IO口
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource4);

	EXTI_InitStructure.EXTI_Line=EXTI_Line4; //中断线的标号 取值范围为EXTI_Line0~EXTI_Line15
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断模式，可选值为中断 EXTI_Mode_Interrupt 和事件 EXTI_Mode_Event。
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//触发方式，可以是下降沿触发 EXTI_Trigger_Falling，上升沿触发 EXTI_Trigger_Rising，或者任意电平（上升沿和下降沿）触发EXTI_Trigger_Rising_Falling
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);//根据结构体信息进行初始化

	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn; //使能外部中断所在的通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5; //子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); //根据结构体信息进行优先级初始化
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//使能时钟
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11;//选择IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//设置成上拉输入
	GPIO_Init(GPIOC, &GPIO_InitStructure);//使用结构体信息进行初始化IO口
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource11);

	EXTI_InitStructure.EXTI_Line=EXTI_Line11; //中断线的标号 取值范围为EXTI_Line0~EXTI_Line15
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断模式，可选值为中断 EXTI_Mode_Interrupt 和事件 EXTI_Mode_Event。
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//触发方式，可以是下降沿触发 EXTI_Trigger_Falling，上升沿触发 EXTI_Trigger_Rising，或者任意电平（上升沿和下降沿）触发EXTI_Trigger_Rising_Falling
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);//根据结构体信息进行初始化
	

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12;//选择IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//设置成上拉输入
	GPIO_Init(GPIOC, &GPIO_InitStructure);//使用结构体信息进行初始化IO口
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource12);

	EXTI_InitStructure.EXTI_Line=EXTI_Line12; //中断线的标号 取值范围为EXTI_Line0~EXTI_Line15
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断模式，可选值为中断 EXTI_Mode_Interrupt 和事件 EXTI_Mode_Event。
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//触发方式，可以是下降沿触发 EXTI_Trigger_Falling，上升沿触发 EXTI_Trigger_Rising，或者任意电平（上升沿和下降沿）触发EXTI_Trigger_Rising_Falling
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);//根据结构体信息进行初始化
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn; //使能外部中断所在的通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //抢占优先级 2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7; //子优先级 2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); //根据结构体信息进行优先级初始化
	
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);//使能时钟
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2;//选择IO口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//设置成上拉输入
	GPIO_Init(GPIOD, &GPIO_InitStructure);//使用结构体信息进行初始化IO口
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD,GPIO_PinSource2);

	EXTI_InitStructure.EXTI_Line=EXTI_Line2; //中断线的标号 取值范围为EXTI_Line0~EXTI_Line15
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断模式，可选值为中断 EXTI_Mode_Interrupt 和事件 EXTI_Mode_Event。
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//触发方式，可以是下降沿触发 EXTI_Trigger_Falling，上升沿触发 EXTI_Trigger_Rising，或者任意电平（上升沿和下降沿）触发EXTI_Trigger_Rising_Falling
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);//根据结构体信息进行初始化
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn; //使能外部中断所在的通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6; //子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); //根据结构体信息进行优先级初始化
}



/*-----------------------------------------------------------------------------
Function Name	:	EXTI2_IRQHandler
Author			:	zhaoji
Created Time	:	2018.03.30
Description 	: 	EXTI_Line2中断处理函数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void EXTI2_IRQHandler(void)
{
	BaseType_t cliNotifyValue;

	if(EXTI_GetITStatus(EXTI_Line2) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line2); //清楚中断标志位
		delay_xms(10);	//延时消抖
		if(KEY_DOWN == 0)
		{
			_CMIOT_Debug("%s(KEY_DOWN Pressed Down)\r\n", __func__);
			/* 打印工厂测试按键信息 */
			if(FACTORY_MODE_FLAG)
			{
				_CMIOT_Uart_send(UART_CLI_DEBUG, (uint8_t *)"\r\nKEYDOWN PRESSED\r\n", strlen((const char*)"\r\nKEYDOWN_PRESSED\r\n"));
				return;
			}
			
			if(((menuPosition.yPosition + 1)*3 + menuPosition.xPosition < 7) && menuPosition.subMenu == 0 && !CM_UI_BUSY)
			{
				menuPosition.yPosition++;
				menuPosition.pressKey = KEYPAD_DOWN;
				CM_UI_BUSY = true;
				_CMIOT_Debug("%s(sendTaskMsg)\r\n", __func__);
				vTaskNotifyGiveFromISR(m5310_task, &cliNotifyValue);   /* 发送任务通知 */
				delay_ms(1);
			}
			else
			{
				_CMIOT_Debug("%s(invalid operation)\r\n", __func__);
			}
		}
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	EXTI3_IRQHandler
Author			:	zhaoji
Created Time	:	2018.03.30
Description 	: 	EXTI_Line3中断处理函数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void EXTI3_IRQHandler(void)
{
	BaseType_t cliNotifyValue;

	if(EXTI_GetITStatus(EXTI_Line3) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line3); //清楚中断标志位
		delay_xms(10);	//延时消抖
		if(KEY_UP==0)
		{
			_CMIOT_Debug("%s(KEY_UP Pressed Down)\r\n", __func__);
			/* 打印工厂测试按键信息 */
			if(FACTORY_MODE_FLAG)
			{
				_CMIOT_Uart_send(UART_CLI_DEBUG, (uint8_t *)"\r\nKEYUP PRESSED\r\n", strlen((const char*)"\r\nKEYUP PRESSED\r\n"));
				return;
			}
			
			if(menuPosition.yPosition > 0 && menuPosition.subMenu == 0 && !CM_UI_BUSY)
			{
				menuPosition.yPosition--;
				menuPosition.pressKey = KEYPAD_UP;
				CM_UI_BUSY = true;
				_CMIOT_Debug("%s(sendTaskMsg)\r\n", __func__);
				vTaskNotifyGiveFromISR(m5310_task, &cliNotifyValue);   /* 发送任务通知 */
				delay_ms(1);
			}
			else
			{
				_CMIOT_Debug("%s(invalid operation)\r\n", __func__);
			}
		}
	}
}



/*-----------------------------------------------------------------------------
Function Name	:	EXTI4_IRQHandler
Author			:	zhaoji
Created Time	:	2018.03.30
Description 	: 	EXTI_Line4中断处理函数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void EXTI4_IRQHandler(void)
{
	BaseType_t cliNotifyValue;
	
	if(EXTI_GetITStatus(EXTI_Line4) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line4); //清楚中断标志位 
		
		delay_xms(10);	//延时消抖
		if(KEY_LEFT==0)
		{
			_CMIOT_Debug("%s(KEY_LEFT Pressed Down)\r\n", __func__);
			/* 打印工厂测试按键信息 */
			if(FACTORY_MODE_FLAG)
			{
				_CMIOT_Uart_send(UART_CLI_DEBUG, (uint8_t *)"\r\nKEYLEFT PRESSED\r\n", strlen((const char*)"\r\nKEYLEFT PRESSED\r\n"));
				return;
			}
			
			if(menuPosition.xPosition > 0 && menuPosition.subMenu == 0 && !CM_UI_BUSY)
			{
				menuPosition.xPosition--;
				menuPosition.pressKey = KEYPAD_LEFT;
				CM_UI_BUSY = true;
				_CMIOT_Debug("%s(sendTaskMsg)\r\n", __func__);
				vTaskNotifyGiveFromISR(m5310_task, &cliNotifyValue);   /* 发送任务通知 */
				delay_ms(1);
			}
			else
			{
				_CMIOT_Debug("%s(invalid operation)\r\n", __func__);
			}
		}
	}
}



/*-----------------------------------------------------------------------------
Function Name	:	EXTI15_10_IRQHandler
Author			:	zhaoji
Created Time	:	2018.03.30
Description 	: 	EXTI_Line10~15中断处理函数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void EXTI15_10_IRQHandler(void)
{
	BaseType_t cliNotifyValue;
	
	if (EXTI_GetITStatus(EXTI_Line12) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line12); //清楚中断标志位 
		
		delay_xms(10);//延时消抖
		if(KEY_RIGHT==0)
		{
			_CMIOT_Debug("%s(KEY_RIGHT Pressed Down)\r\n", __func__);
			/* 打印工厂测试按键信息 */
			if(FACTORY_MODE_FLAG)
			{
				_CMIOT_Uart_send(UART_CLI_DEBUG, (uint8_t *)"\r\nKEYRIGHT PRESSED\r\n", strlen((const char*)"\r\nKEYRIGHT PRESSED\r\n"));
				return;
			}
			
			if(menuPosition.yPosition < 2 && menuPosition.xPosition < 2 && menuPosition.subMenu == 0 && !CM_UI_BUSY)
			{
				menuPosition.xPosition++;
				menuPosition.pressKey = KEYPAD_RIGHT;
				CM_UI_BUSY = true;
				_CMIOT_Debug("%s(sendTaskMsg)\r\n", __func__);
				vTaskNotifyGiveFromISR(m5310_task, &cliNotifyValue);   /* 发送任务通知 */
				delay_ms(1);
			}
			else
			{
				_CMIOT_Debug("%s(invalid operation)\r\n", __func__);
			}
		}
	}
	
	if (EXTI_GetITStatus(EXTI_Line11) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line11); //清楚中断标志位 
		delay_xms(10);//延时消抖
		if(KEY_ENTER==0 && !CM_UI_BUSY)
		{
			_CMIOT_Debug("%s(KEY_ENTER Pressed Down)\r\n", __func__);
			/* 打印工厂测试按键信息 */
			if(FACTORY_MODE_FLAG)
			{
				_CMIOT_Uart_send(UART_CLI_DEBUG, (uint8_t *)"\r\nKEYENTER PRESSED\r\n", strlen((const char*)"\r\nKEYENTER PRESSED\r\n"));
				return;
			}
			
			if(menuPosition.subMenu == 0)
			{
				menuPosition.subMenu = 1;
			}
			else
			{
				menuPosition.subMenu = 0;
			}
			menuPosition.pressKey = KEYPAD_ENTER;
			CM_UI_BUSY = true;
			_CMIOT_Debug("%s(sendTaskMsg)\r\n", __func__);
			vTaskNotifyGiveFromISR(m5310_task, &cliNotifyValue);   /* 向发送任务通知 */
			delay_ms(1);
		}
	}
}




