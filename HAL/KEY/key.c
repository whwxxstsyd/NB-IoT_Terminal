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

#define KEY0	PCin(5)

/*----------------------------------------------------------------------------*
**                             Global Vars                                    *
**----------------------------------------------------------------------------*/
extern TaskHandle_t m5310_task;     /* M5310任务  */
extern uint8_t menuNewIndex;
extern uint8_t menuOldIndex;


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
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//使能时钟
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;//选择IO口   PC5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//设置成上拉输入
	GPIO_Init(GPIOC, &GPIO_InitStructure);//使用结构体信息进行初始化IO口
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource5);
	
	EXTI_InitStructure.EXTI_Line=EXTI_Line5; //中断线的标号 取值范围为EXTI_Line0~EXTI_Line15
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断模式，可选值为中断 EXTI_Mode_Interrupt 和事件 EXTI_Mode_Event。
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//触发方式，可以是下降沿触发 EXTI_Trigger_Falling，上升沿触发 EXTI_Trigger_Rising，或者任意电平（上升沿和下降沿）触发EXTI_Trigger_Rising_Falling
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);//根据结构体信息进行初始化

	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn; //使能外部中断所在的通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; //抢占优先级 2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02; //子优先级 2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); //根据结构体信息进行优先级初始化 
}





void EXTI9_5_IRQHandler(void)
{
	BaseType_t cliNotifyValue;
	
	if (EXTI_GetITStatus(EXTI_Line5) == RESET)
	{
		return;
	}
	_CMIOT_Debug("%s(KEY0 Down)\r\n", __func__);
	delay_ms(10);//延时消抖
	if(KEY0==0)    //按键真的被按下
	{
		if(menuNewIndex == 4)
		{
			menuNewIndex = 0;
			menuOldIndex = 4;
		}
		else
		{
			menuOldIndex = menuNewIndex;
			menuNewIndex ++;
		}
		
		while(KEY0!=0);//等待松手
		vTaskNotifyGiveFromISR(m5310_task, &cliNotifyValue);   /* 向任务发送任务通知 */
	}
	EXTI_ClearITPendingBit(EXTI_Line5); //清楚中断标志位 
}





