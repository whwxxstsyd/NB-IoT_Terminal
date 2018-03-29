/*
 * FreeRTOS Kernel V10.0.0
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software. If you wish to use our Amazon
 * FreeRTOS name, please do so in a fair use way that does not cause confusion.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* include timer.h for freertos runtime status */
#include "timer.h"

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE. 
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/



/***************************************************************************************************************/
/*                                        FreeRTOS基础配置配置选项                                             */
/***************************************************************************************************************/
#define configUSE_PREEMPTION											1	//1使用抢占式内核，0使用协程
#define configUSE_TIME_SLICING											1	//1使能时间片调度(默认式使能的)
#define configUSE_PORT_OPTIMISED_TASK_SELECTION							1	//1启用特殊方法来选择下一个要运行的任务
																			//一般是硬件计算前导零指令，如果所使用的
																			//MCU没有这些硬件指令的话此宏应该设置为0！
#define configUSE_TICKLESS_IDLE											0	//1启用低功耗tickless模式
#define configUSE_QUEUE_SETS											1	//为1时启用队列

#define configCPU_CLOCK_HZ					(( unsigned long ) 72000000)	//CPU频率
#define configTICK_RATE_HZ					(( TickType_t ) 1000)			//时钟节拍频率
#define configMAX_PRIORITIES				( 5 )							//可使用的最大优先级
#define configMINIMAL_STACK_SIZE			( ( unsigned short ) 128 )		//空闲任务使用的堆栈大小
#define configMAX_TASK_NAME_LEN				( 16 )							//任务名字字符串长度
#define configUSE_16_BIT_TICKS				0								//系统节拍计数器变量数据类型，1表示为16位无符号整形，0表示为32位无符号整形
#define configIDLE_SHOULD_YIELD				1								//为1时空闲任务放弃CPU使用权给其他同优先级的用户任务

#define configUSE_TASK_NOTIFICATIONS									1	//为1时开启任务通知功能，默认开启
#define configUSE_MUTEXES												1	//为1时使用互斥信号量
#define configQUEUE_REGISTRY_SIZE										8	//不为0时表示启用队列记录，具体的值是可以记录的队列和信号量最大数目
#define configCHECK_FOR_STACK_OVERFLOW									0	//大于0时启用堆栈溢出检测功能，如果使用此功能
																			//用户必须提供一个栈溢出钩子函数，如果使用的话
																			//此值可以为1或者2，因为有两种栈溢出检测方法。
#define configUSE_RECURSIVE_MUTEXES										1	//为1时使用递归互斥信号量
#define configUSE_MALLOC_FAILED_HOOK									0	//1使用内存申请失败钩子函数
#define configUSE_APPLICATION_TASK_TAG									0                       
#define configUSE_COUNTING_SEMAPHORES									1	//为1时使用计数信号量


/***************************************************************************************************************/
/*                                FreeRTOS与钩子函数有关的配置选项                                             */
/***************************************************************************************************************/
#define configUSE_IDLE_HOOK								0                          		//1，使用空闲钩子；0，不使用
#define configUSE_TICK_HOOK								0                          		//1，使用时间片钩子；0，不使用


/***************************************************************************************************************/
/*                                FreeRTOS与内存申请有关配置选项                                               */
/***************************************************************************************************************/
#define configSUPPORT_DYNAMIC_ALLOCATION        		1                       		//支持动态内存申请
#define configTOTAL_HEAP_SIZE							((size_t)(20*1024))     		//系统所有总的堆大小


/***************************************************************************************************************/
/*                                FreeRTOS与软件定时器有关的配置选项                                           */
/***************************************************************************************************************/
#define configUSE_TIMERS								1                               //为1时启用软件定时器
#define configTIMER_TASK_PRIORITY						(configMAX_PRIORITIES-1)        //软件定时器优先级
#define configTIMER_QUEUE_LENGTH						5                               //软件定时器队列长度
#define configTIMER_TASK_STACK_DEPTH					(configMINIMAL_STACK_SIZE*2)    //软件定时器任务堆栈大小


/***************************************************************************************************************/
/*                                FreeRTOS与协程有关的配置选项                                                 */
/***************************************************************************************************************/
#define configUSE_CO_ROUTINES							0								//为1时启用协程，启用协程以后必须添加文件croutine.c
#define configMAX_CO_ROUTINE_PRIORITIES					( 2 )                   		//协程的有效优先级数目


/***************************************************************************************************************/
/*                                FreeRTOS与运行时间和任务状态收集有关的配置选项                                */
/***************************************************************************************************************/
#define configGENERATE_RUN_TIME_STATS					1								//为1时启用运行时间统计功能
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()		ConfigureTimeForRunTimeStats()	//定时器3提供时间统计的时基
#define portGET_RUN_TIME_COUNTER_VALUE()				FreeRTOSRunTimeTicks			//获取时间统计时间值

#define configUSE_TRACE_FACILITY						0								//为1启用可视化跟踪调试
#define configUSE_STATS_FORMATTING_FUNCTIONS			0								//与宏configUSE_TRACE_FACILITY同时为1时会编译下面3个函数
																						//prvWriteNameToBuffer(),vTaskList(),
																						//vTaskGetRunTimeStats()


/***************************************************************************************************************/
/*                                FreeRTOS可选函数配置选项                                                     */
/***************************************************************************************************************/
#define INCLUDE_xTaskGetSchedulerState					1                       
#define INCLUDE_vTaskPrioritySet						1
#define INCLUDE_uxTaskPriorityGet						1
#define INCLUDE_vTaskDelete								1
#define INCLUDE_vTaskCleanUpResources					1
#define INCLUDE_vTaskSuspend							1
#define INCLUDE_vTaskDelayUntil							1
#define INCLUDE_vTaskDelay								1
#define INCLUDE_eTaskGetState							1
#define INCLUDE_xTimerPendFunctionCall					1
#define INCLUDE_uxTaskGetStackHighWaterMark				1


/***************************************************************************************************************/
/*                                FreeRTOS与中断有关的配置选项                                                 */
/***************************************************************************************************************/
/* This is the raw value as per the Cortex-M3 NVIC.  Values can be 255
(lowest) to 0 (1?) (highest). */
#define configKERNEL_INTERRUPT_PRIORITY 		255
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	191 /* equivalent to 0xb0, or priority 11. */


/* This is the value being used as per the ST library which permits 16
priority values, 0 to 15.  This must correspond to the
configKERNEL_INTERRUPT_PRIORITY setting.  Here 15 corresponds to the lowest
NVIC value of 255. */
#define configLIBRARY_KERNEL_INTERRUPT_PRIORITY	15


/***************************************************************************************************************/
/*                                FreeRTOS与中断服务函数有关的配置选项                                         */
/***************************************************************************************************************/
#define xPortPendSVHandler			PendSV_Handler
#define vPortSVCHandler				SVC_Handler




#define	configCOMMAND_INT_MAX_OUTPUT_SIZE       100


#endif /* FREERTOS_CONFIG_H */

