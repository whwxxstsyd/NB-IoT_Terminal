/*-----------------------------------------------------------------------------
File Name   	:   ui.c
Author          :   zhaoji
Created Time    :   2018.03.12
Description     :   UI接口
-----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*
                               Dependencies                                   *
------------------------------------------------------------------------------*/
#include "ui.h"
#include "lcd.h"
#include "common.h"
#include "delay.h"
#include "m5310.h"
#include "stdio.h"
#include "image.h"
#include "math.h"
#include "adc.h"
#include "bluetooth.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"


/*----------------------------------------------------------------------------*
**                             Global Vars                                    *
**----------------------------------------------------------------------------*/
static uint8_t msg[128];


/* USART1(UART_BLUETOOTH)数据接收buffer */
extern uint8_t   UART_BLE_RxBuffer[1024];
extern uint32_t  UART_BLE_RxBufferLen;

extern TaskHandle_t bleCmdProcess_task;	/* 蓝牙命令处理任务 */

/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_UI_BootPage
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	开机欢迎界面
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_UI_BootPage(void)
{
	LCD_ShowPicture(70,50,100,100,(u8 *)BootImage);
	// LCD_ShowFontHZ(75, 170, (u8 *)"中国移动", 24, TITLEBLUE, WHITE);
	LCD_ShowChinese(75, 170, Arial, 24, (u8 *)"中国移动", TITLEBLUE, WHITE);
	POINT_COLOR = TITLEBLUE;
	LCD_ShowFontEN(15, 210, (u8 *)"NB-IoT", 24, TITLEBLUE, WHITE);
	LCD_ShowFontHZ(110, 210, (u8 *)"网络测试仪", 24, TITLEBLUE, WHITE);
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_TabIndex
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	菜单跳转
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_TabIndex(CM_MENU_POSITION *position)
{
	_CMIOT_Debug("%s(Position: %d,%d,%d,%d,%d,%d)\r\n", __func__,\
		position->xPosition, position->yPosition, position->xSubPosition, position->ySubPosition, position->subMenu, position->pressKey);
	
	switch(position->pressKey)
	{
		/* 确认/返回 按键被按下 */
		case KEYPAD_ENTER:
		{
			if(position->subMenu == 0)
			{
				_CMIOT_GUI_Init(position->xPosition, position->yPosition);
			}
			else if(position->subMenu == 1)
			{
				_CMIOT_NBIOT_Measurement(position->yPosition * 3 + position->xPosition);
			}
			else
			{
				_CMIOT_Debug("%s(error subMenu value)\r\n", __func__);
			}
			break;
		}
		/* 上 按键被按下 */
		case KEYPAD_UP:
		{
			if(position->subMenu == 0)
			{
				if((position->yPosition + 1)*3 + position->xPosition == 0)
				{LCD_ShowChinese(8, 110, newArial, 16, (u8 *)"设备信息", BLACK, WHITE); }
				else if((position->yPosition + 1)*3 + position->xPosition == 1)
					{LCD_ShowChinese(88, 110, newArial, 16, (u8 *)"驻网测试", BLACK, WHITE); }
				else if((position->yPosition + 1)*3 + position->xPosition == 2)
					{LCD_ShowChinese(168, 110, newArial, 16, (u8 *)"信号参数", BLACK, WHITE); }
				else if((position->yPosition + 1)*3 + position->xPosition == 3)
					{LCD_ShowChinese(8, 200, newArial, 16, (u8 *)"网络延时", BLACK, WHITE); }
				else if((position->yPosition + 1)*3 + position->xPosition == 4)
					{LCD_ShowChinese(88, 200, newArial, 16, (u8 *)"速率测试", BLACK, WHITE); }
				else if((position->yPosition + 1)*3 + position->xPosition == 5)
					{LCD_ShowChinese(168, 200, newArial, 16, (u8 *)"蓝牙直连", BLACK, WHITE); }
				else if((position->yPosition + 1)*3 + position->xPosition == 6)
					{LCD_ShowChinese(8, 290, newArial, 16, (u8 *)"使用说明", BLACK, WHITE); }

				if((position->yPosition)*3 + position->xPosition == 0)
					{LCD_ShowChinese(8, 110, newArial, 16, (u8 *)"设备信息", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 1)
					{LCD_ShowChinese(88, 110, newArial, 16, (u8 *)"驻网测试", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 2)
					{LCD_ShowChinese(168, 110, newArial, 16, (u8 *)"信号参数", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 3)
					{LCD_ShowChinese(8, 200, newArial, 16, (u8 *)"网络延时", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 4)
					{LCD_ShowChinese(88, 200, newArial, 16, (u8 *)"速率测试", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 5)
					{LCD_ShowChinese(168, 200, newArial, 16, (u8 *)"蓝牙直连", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 6)
					{LCD_ShowChinese(8, 290, newArial, 16, (u8 *)"使用说明", BLACK, LIGHTBLUE); }
			}
			else
			{
				_CMIOT_Debug("%s(prevent key up)\r\n", __func__);
			}
			break;
		}
		/* 下 按键被按下 */
		case KEYPAD_DOWN:
		{
			if(position->subMenu == 0)
			{
				if((position->yPosition - 1)*3 + position->xPosition == 0)
				{LCD_ShowChinese(8, 110, newArial, 16, (u8 *)"设备信息", BLACK, WHITE); }
				else if((position->yPosition - 1)*3 + position->xPosition == 1)
					{LCD_ShowChinese(88, 110, newArial, 16, (u8 *)"驻网测试", BLACK, WHITE); }
				else if((position->yPosition - 1)*3 + position->xPosition == 2)
					{LCD_ShowChinese(168, 110, newArial, 16, (u8 *)"信号参数", BLACK, WHITE); }
				else if((position->yPosition - 1)*3 + position->xPosition == 3)
					{LCD_ShowChinese(8, 200, newArial, 16, (u8 *)"网络延时", BLACK, WHITE); }
				else if((position->yPosition - 1)*3 + position->xPosition == 4)
					{LCD_ShowChinese(88, 200, newArial, 16, (u8 *)"速率测试", BLACK, WHITE); }
				else if((position->yPosition - 1)*3 + position->xPosition == 5)
					{LCD_ShowChinese(168, 200, newArial, 16, (u8 *)"蓝牙直连", BLACK, WHITE); }
				else if((position->yPosition - 1)*3 + position->xPosition == 6)
					{LCD_ShowChinese(8, 290, newArial, 16, (u8 *)"使用说明", BLACK, WHITE); }

				if((position->yPosition)*3 + position->xPosition == 0)
					{LCD_ShowChinese(8, 110, newArial, 16, (u8 *)"设备信息", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 1)
					{LCD_ShowChinese(88, 110, newArial, 16, (u8 *)"驻网测试", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 2)
					{LCD_ShowChinese(168, 110, newArial, 16, (u8 *)"信号参数", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 3)
					{LCD_ShowChinese(8, 200, newArial, 16, (u8 *)"网络延时", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 4)
					{LCD_ShowChinese(88, 200, newArial, 16, (u8 *)"速率测试", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 5)
					{LCD_ShowChinese(168, 200, newArial, 16, (u8 *)"蓝牙直连", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 6)
					{LCD_ShowChinese(8, 290, newArial, 16, (u8 *)"使用说明", BLACK, LIGHTBLUE); }
			}
			break;
		}
		/* 左 按键被按下 */
		case KEYPAD_LEFT:
		{
			if(position->subMenu == 0)
			{
				if((position->yPosition)*3 + position->xPosition + 1 == 0)
				{LCD_ShowChinese(8, 110, newArial, 16, (u8 *)"设备信息", BLACK, WHITE); }
				else if((position->yPosition)*3 + position->xPosition + 1 == 1)
					{LCD_ShowChinese(88, 110, newArial, 16, (u8 *)"驻网测试", BLACK, WHITE); }
				else if((position->yPosition)*3 + position->xPosition + 1 == 2)
					{LCD_ShowChinese(168, 110, newArial, 16, (u8 *)"信号参数", BLACK, WHITE); }
				else if((position->yPosition)*3 + position->xPosition + 1 == 3)
					{LCD_ShowChinese(8, 200, newArial, 16, (u8 *)"网络延时", BLACK, WHITE); }
				else if((position->yPosition)*3 + position->xPosition + 1 == 4)
					{LCD_ShowChinese(88, 200, newArial, 16, (u8 *)"速率测试", BLACK, WHITE); }
				else if((position->yPosition)*3 + position->xPosition + 1 == 5)
					{LCD_ShowChinese(168, 200, newArial, 16, (u8 *)"蓝牙直连", BLACK, WHITE); }
				else if((position->yPosition)*3 + position->xPosition + 1 == 6)
					{LCD_ShowChinese(8, 290, newArial, 16, (u8 *)"使用说明", BLACK, WHITE); }

				if((position->yPosition)*3 + position->xPosition == 0)
					{LCD_ShowChinese(8, 110, newArial, 16, (u8 *)"设备信息", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 1)
					{LCD_ShowChinese(88, 110, newArial, 16, (u8 *)"驻网测试", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 2)
					{LCD_ShowChinese(168, 110, newArial, 16, (u8 *)"信号参数", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 3)
					{LCD_ShowChinese(8, 200, newArial, 16, (u8 *)"网络延时", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 4)
					{LCD_ShowChinese(88, 200, newArial, 16, (u8 *)"速率测试", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 5)
					{LCD_ShowChinese(168, 200, newArial, 16, (u8 *)"蓝牙直连", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 6)
					{LCD_ShowChinese(8, 290, newArial, 16, (u8 *)"使用说明", BLACK, LIGHTBLUE); }
			}
			break;
		}
		/* 右 按键被按下 */
		case KEYPAD_RIGHT:
		{
			if(position->subMenu == 0)
			{
				if((position->yPosition)*3 + position->xPosition - 1 == 0)
				{LCD_ShowChinese(8, 110, newArial, 16, (u8 *)"设备信息", BLACK, WHITE); }
				else if((position->yPosition)*3 + position->xPosition - 1 == 1)
					{LCD_ShowChinese(88, 110, newArial, 16, (u8 *)"驻网测试", BLACK, WHITE); }
				else if((position->yPosition)*3 + position->xPosition - 1 == 2)
					{LCD_ShowChinese(168, 110, newArial, 16, (u8 *)"信号参数", BLACK, WHITE); }
				else if((position->yPosition)*3 + position->xPosition - 1 == 3)
					{LCD_ShowChinese(8, 200, newArial, 16, (u8 *)"网络延时", BLACK, WHITE); }
				else if((position->yPosition)*3 + position->xPosition - 1 == 4)
					{LCD_ShowChinese(88, 200, newArial, 16, (u8 *)"速率测试", BLACK, WHITE); }
				else if((position->yPosition)*3 + position->xPosition - 1 == 5)
					{LCD_ShowChinese(168, 200, newArial, 16, (u8 *)"蓝牙直连", BLACK, WHITE); }
				else if((position->yPosition)*3 + position->xPosition - 1 == 6)
					{LCD_ShowChinese(8, 290, newArial, 16, (u8 *)"使用说明", BLACK, WHITE); }

				if((position->yPosition)*3 + position->xPosition == 0)
					{LCD_ShowChinese(8, 110, newArial, 16, (u8 *)"设备信息", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 1)
					{LCD_ShowChinese(88, 110, newArial, 16, (u8 *)"驻网测试", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 2)
					{LCD_ShowChinese(168, 110, newArial, 16, (u8 *)"信号参数", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 3)
					{LCD_ShowChinese(8, 200, newArial, 16, (u8 *)"网络延时", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 4)
					{LCD_ShowChinese(88, 200, newArial, 16, (u8 *)"速率测试", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 5)
					{LCD_ShowChinese(168, 200, newArial, 16, (u8 *)"蓝牙直连", BLACK, LIGHTBLUE); }
				else if((position->yPosition)*3 + position->xPosition == 6)
					{LCD_ShowChinese(8, 290, newArial, 16, (u8 *)"使用说明", BLACK, LIGHTBLUE); }
			}
			break;
		}
		
		default: {
			_CMIOT_Debug("%s(error pressKey)\r\n", __func__);
		}
	}
	while(1)
	{
		/* 显示信号强度状态 */
		_CMIOT_ShowSignalStrength(_CMIOT_M5310_GetSignalstrength());
		/* 显示电池状态 */
		_CMIOT_ShowBatteryTips();
		delay_ms(2000);
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_ShowDeviceInfo
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	获取设备信息展示到LCD屏幕
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_ShowDeviceInfo()
{
	/* 左侧蓝条 */
	LCD_Fill(0,40,77,319, LIGHTBLUE);
	
	/* 模组型号 */
	LCD_ShowChinese(3, 50, newArial, 16, (u8 *)"模组型号", BLACK, LIGHTBLUE);
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTBLUE;
	LCD_ShowString(68, 50, 16,  (u8 *)":");
	
	/* IMEI */
	LCD_ShowString(3, 80, 16, (u8 *)"IMEI");
	LCD_ShowString(68, 80, 16,  (u8 *)":");
	
	/* ICCID */
	LCD_ShowString(3, 110, 16, (u8 *)"ICCID");
	LCD_ShowString(68, 110, 16,  (u8 *)":");
	
	/* IMSI */
	LCD_ShowString(3, 140, 16, (u8 *)"IMSI");
	LCD_ShowString(68, 140, 16,  (u8 *)":");
	
	/* 电池电压 */
	LCD_ShowChinese(3, 170, newArial, 16, (u8 *)"电池电压", BLACK, LIGHTBLUE);
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTBLUE;
	LCD_ShowString(68, 170, 16,  (u8 *)":");
	
	/* 蓝牙状态 */
	LCD_ShowChinese(3, 200, newArial, 16, (u8 *)"蓝牙状态", BLACK, LIGHTBLUE);
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTBLUE;
	LCD_ShowString(68, 200, 16,  (u8 *)":");
	
	/* 模组版本 */
	LCD_ShowChinese(3, 230, newArial, 16, (u8 *)"模组版本", BLACK, LIGHTBLUE);
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTBLUE;
	LCD_ShowString(68, 230, 16,  (u8 *)":");
	
	/* MCU版本 */
	LCD_ShowChinese(3, 275, newArial, 16, (u8 *)"软件版本", BLACK, LIGHTBLUE);
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTBLUE;
	LCD_ShowString(68, 275, 16,  (u8 *)":");
	
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	/* 模组型号 */
	_CMIOT_GetModuleName(msg, sizeof(msg));
	LCD_ShowString(80, 50, 16, (u8 *)msg);
	/* IMEI */
	_CMIOT_GetIMEI(msg, sizeof(msg));
	LCD_ShowString(80, 80, 16, (u8 *)msg);
	/* ICCID */
	_CMIOT_GetICCID(msg, sizeof(msg));
	LCD_ShowString(80, 110, 16, (u8 *)msg);
	/* IMSI */
	_CMIOT_GetIMSI(msg, sizeof(msg));
	LCD_ShowString(80, 140, 16, (u8 *)msg);
	/* 模组版本 */
	_CMIOT_GetModuleVersion(msg, sizeof(msg));
	LCD_ShowString(80, 230, 16, (u8 *)msg);
	/* 软件版本 */
	cm_getbuildVersion(msg, sizeof(msg));
	LCD_ShowString(80, 275, 16, (u8 *)msg);

	while(1)
	{
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		/* 电池电压 */
		sprintf((char *)msg, "%-3.1f V", cm_getBatteryVol());
		LCD_ShowString(80, 170, 16, (u8 *)msg);
	
		/* 蓝牙状态 */
		if(_CMIOT_GetBleConnectedState())
		{
			LCD_ShowChinese(80, 200, newArial, 16, (u8 *)"已连接", BLACK, WHITE);
		}
		else
		{
			LCD_ShowChinese(80, 200, newArial, 16, (u8 *)"未连接", BLACK, WHITE);
		}
		_CMIOT_ShowSignalStrength(_CMIOT_M5310_GetSignalstrength());
		delay_ms(1000);
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_ShowRadioInfo
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	获取模组射频信息、信息信息展示到LCD屏幕
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_ShowRadioInfo()
{
	CMIOT_UE_STATE ue_state;
	CMIOT_UE_STATE_CELL ue_state_cell;
	uint8_t CsqValue = 0;
	uint8_t t3412[32] = {0};
	uint8_t t3324[32] = {0};
	
	LCD_ShowChinese(5, 45, newArial, 16, (u8 *)"信号质量评估：正在获取", BLACK, WHITE);
	POINT_COLOR = BLACK;
	LCD_DrawLine(0, 69, 239, 69);
	/* 左侧蓝条 */
	LCD_Fill(0,70,79,319, LIGHTBLUE);
	
	/* 显示参数名 */
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTBLUE;
	LCD_ShowString(5, 75,  16, (u8 *)"CSQ    :");
	LCD_ShowString(5, 95,  16, (u8 *)"SNR    :");
	LCD_ShowString(5, 115, 16, (u8 *)"RSRQ   :");
	LCD_ShowString(5, 135, 16, (u8 *)"RSRP   :");
	LCD_ShowString(5, 155, 16, (u8 *)"EARFCN :");
	LCD_ShowString(5, 175, 16, (u8 *)"ECL    :");
	LCD_ShowString(5, 195, 16, (u8 *)"PLMN   :");
	LCD_ShowString(5, 215, 16, (u8 *)"APN    :");
	LCD_ShowString(5, 235, 16, (u8 *)"BAND   :");
	LCD_ShowString(5, 255, 16, (u8 *)"T3324  :");
	LCD_ShowString(5, 275, 16, (u8 *)"T3412  :");
	LCD_ShowString(5, 295, 16, (u8 *)"CELLID :");
	
	while(1)
	{
		/* Ping测试防止模组进入PSM模式后射频参数不准确 */
		_CMIOT_GetNetworkDelay((uint8_t *)PING_ADDR, 100, 5000);
		
		/* RSRP */
		ue_state_cell = _CMIOT_GetUeCellStats();
		sprintf((char *)msg, "%-5.1f", (float)ue_state_cell.rsrp/10);
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		LCD_ShowString(95, 135, 16,  (u8 *)msg);
		
		/* 刷新信号评估结果 */
		if(ue_state_cell.rsrp == 0)
		{
			LCD_Fill(117,45,239,65,WHITE);
			LCD_ShowChinese(117, 45, newArial, 16, (u8 *)"无信号", RED, WHITE);
		}
		else if(ue_state_cell.rsrp >= -950)
		{
			LCD_Fill(117,45,239,65,WHITE);
			LCD_ShowChinese(117, 45, newArial, 16, (u8 *)"优", GREEN, WHITE);
		}
		else if(ue_state_cell.rsrp >= -1050)
		{
			LCD_Fill(117,45,239,65,WHITE);
			LCD_ShowChinese(117, 45, newArial, 16, (u8 *)"良", DARKGREEN, WHITE);
		}
		else if(ue_state_cell.rsrp >= -1150)
		{
			LCD_Fill(117,45,239,65,WHITE);
			LCD_ShowChinese(117, 45, newArial, 16, (u8 *)"中", DARKGREEN, WHITE);
		}
		else
		{
			LCD_Fill(117,45,239,65,WHITE);
			LCD_ShowChinese(117, 45, newArial, 16, (u8 *)"差", RED, WHITE);
		}
		
		/* 刷新信号强度 */
		CsqValue = _CMIOT_M5310_GetSignalstrength();
		sprintf((char *)msg, "%-3d", CsqValue);
		_CMIOT_Debug("%s(csqMsg: %s)\r\n", __func__, msg);
		/* 更新信号强度图标 */
		_CMIOT_ShowSignalStrength(CsqValue);
		
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		LCD_ShowString(95, 75, 16,  (u8 *)msg);
		
		/* SNR */
		ue_state = _CMIOT_M5310_GetUeState();
		_CMIOT_Debug("%s(%d,%d,%d,%d,%s)\r\n", __func__, ue_state.snr, ue_state.rsrq, ue_state.earfcn, ue_state.ecl, ue_state.cellid);
		sprintf((char *)msg, "%-10d", ue_state.snr);
		LCD_ShowString(95, 95, 16,  (u8 *)msg);
		/* RSRQ */
		sprintf((char *)msg, "%-10d", ue_state.rsrq);
		LCD_ShowString(95, 115, 16,  (u8 *)msg);
		/* EARFCN */
		sprintf((char *)msg, "%-10d", ue_state.earfcn);
		LCD_ShowString(95, 155, 16,  (u8 *)msg);
		/* ECL */
		sprintf((char *)msg, "%-10d", ue_state.ecl);
		LCD_ShowString(95, 175, 16,  (u8 *)msg);
		/* CELLID */
		LCD_Fill(95, 295, 239, 315, WHITE);
		LCD_ShowString(95, 295, 16,  ue_state.cellid);
		
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		/* 刷新PLMN号 */
		_CMIOT_GetPLMN(msg, sizeof(msg));
		LCD_Fill(95, 195, 239, 215, WHITE);
		LCD_ShowString(95, 195, 16,  msg);
		
		/* 刷新BAND频段号 */
		sprintf((char *)msg, "%-10d", _CMIOT_GetNB_Band());
		LCD_ShowString(95, 235, 16, msg);
		
		/* 刷新T3412 T3324定时器值 */
		_CMIOT_Get_PSM_TIMER_Value(t3324, t3412, sizeof(t3324));
		LCD_Fill(95, 255, 239, 275, WHITE);
		LCD_ShowString(95, 255, 16,  t3324);
		
		LCD_Fill(95, 275, 239, 295, WHITE);
		LCD_ShowString(95, 275, 16,  t3412);
		
		/* apn */
		cm_getAPN(msg, sizeof(msg));
		LCD_Fill(95, 215, 239, 235, WHITE);
		LCD_ShowString(95, 215, 16,  msg);
		
		delay_ms(5000);
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_ShowNetworkRegTime
Author			:	zhaoji
Created Time	:	2018.03.16
Description 	: 	测试驻网时间，并展示到LCD屏幕
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_ShowNetworkRegTime()
{
	uint32_t TestCount = 0;
	uint32_t FailCount = 0;
	uint32_t MaxRegisterTime = 0;
	uint32_t AvrRegisterTime = 0;
	uint32_t MinRegisterTime = 0;
	uint32_t result;
	
	LCD_ShowChinese(75, 50, Arial, 24, (u8 *)"驻网时间", LIGHTBLUE, WHITE);
	LCD_ShowFontEN(65, 80, (u8 *)"---.---", 24, BRRED, WHITE);
	/* 分割线 */
	POINT_COLOR = BLACK;
	LCD_DrawLine(0,129,239,129);
	/* 左侧蓝条 */
	LCD_Fill(0,130,79,319,LIGHTBLUE);
	
	LCD_ShowChinese(5, 140, newArial, 16, (u8 *)"测试次数", BLACK, LIGHTBLUE);
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTBLUE;
	LCD_ShowString(70, 140, 16,  (u8 *)":");
	LCD_ShowChinese(5, 170, newArial, 16, (u8 *)"失败次数", BLACK, LIGHTBLUE);
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTBLUE;
	LCD_ShowString(70, 170, 16,  (u8 *)":");
	LCD_ShowChinese(5, 200, newArial, 16, (u8 *)"成功率", BLACK, LIGHTBLUE);
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTBLUE;
	LCD_ShowString(70, 200, 16,  (u8 *)":");
	LCD_ShowChinese(5, 230, newArial, 16, (u8 *)"平均时长", BLACK, LIGHTBLUE);
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTBLUE;
	LCD_ShowString(70, 230, 16,  (u8 *)":");
	LCD_ShowChinese(5, 260, newArial, 16, (u8 *)"最大时长", BLACK, LIGHTBLUE);
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTBLUE;
	LCD_ShowString(70, 260, 16,  (u8 *)":");
	LCD_ShowChinese(5, 290, newArial, 16, (u8 *)"最短时长", BLACK, LIGHTBLUE);
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTBLUE;
	LCD_ShowString(70, 290, 16,  (u8 *)":");
	while(1)
	{
		LCD_ShowChinese(90, 109, newArial, 16, (u8 *)"正在驻网", BLACK, WHITE);
		result = _CMIOT_M5310_GetRegisterTime();
		delay_ms(50);
		
		if(result > 1)
		{
			LCD_ShowChinese(90, 109, newArial, 16, (u8 *)"驻网成功", DARKGREEN, WHITE);
			TestCount++;	/* 测试次数+1 */
			sprintf((char *)msg, "%07.3fs", (float)result/1000);
			LCD_ShowFontEN(65, 80, (u8 *)msg, 24, BRRED, WHITE);
		}
		else if(result == 1)
		{
			LCD_ShowChinese(90, 109, newArial, 16, (u8 *)"驻网失败", RED, WHITE);
			LCD_ShowFontEN(65, 80, (u8 *)"timeout ", 24, RED, WHITE);
			TestCount++;	/* 测试次数+1 */
			FailCount++;	/* 失败次数+1 */
		}
		else
		{
			LCD_ShowFontEN(65, 80, (u8 *)"booterr ", 24, RED, WHITE);
		}
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		/* 显示测试次数 */
		sprintf((char *)msg, "%-5d", TestCount);
		// LCD_Fill(165, 130, 239, 150, WHITE);
		LCD_ShowString(95, 140, 16,  msg);
		/* 显示失败次数 */
		sprintf((char *)msg, "%-5d", FailCount);
		// LCD_Fill(165, 155, 239, 150, WHITE);
		LCD_ShowString(95, 170, 16,  msg);
		/* 显示成功率 */
		if(TestCount > 0)
		{
			sprintf((char *)msg, "%-7.2f%%", ((float)(TestCount - FailCount)/(float)TestCount)*100);
			// LCD_Fill(85, 180, 239, 150, WHITE);
			LCD_ShowString(95, 200, 16,  msg);
		}
		/* 平均时长 */
		if(result > 1)
		{
			AvrRegisterTime = (AvrRegisterTime * (TestCount - FailCount - 1) + result)/(TestCount - FailCount);
		}
		sprintf((char *)msg, "%-7.3fs", (float)AvrRegisterTime/1000);
		// LCD_Fill(165, 205, 239, 150, WHITE);
		LCD_ShowString(95, 230, 16,  msg);
		/* 最大时长 */
		if(result > MaxRegisterTime && result > 1) { MaxRegisterTime = result; }
		sprintf((char *)msg, "%-7.3fs", (float)MaxRegisterTime/1000);
		// LCD_Fill(165, 230, 239, 150, WHITE);
		LCD_ShowString(95, 260, 16,  msg);
		/* 最短时长 */
		if(((result < MinRegisterTime) || (MinRegisterTime == 0)) && (result > 1)) { MinRegisterTime = result; }
		sprintf((char *)msg, "%-7.3fs", (float)MinRegisterTime/1000);
		// LCD_Fill(165, 255, 239, 150, WHITE);
		LCD_ShowString(95, 290, 16,  msg);
		
		_CMIOT_ShowSignalStrength(_CMIOT_M5310_GetSignalstrength());
		delay_ms(10000);
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_ShowPingResult
Author			:	zhaoji
Created Time	:	2018.03.27
Description 	: 	展示Ping测试结果到LCD
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_ShowPingResult(void)
{
	uint32_t TestCount = 0;
	uint32_t FailCount = 0;
	uint32_t MaxPingDelay = 0;
	uint32_t AvrPingDelay = 0;
	uint32_t MinPingDelay = 0;
	uint32_t result;
	
	LCD_ShowChinese(75, 50, Arial, 24, (u8 *)"网络延时", LIGHTBLUE, WHITE);
	LCD_ShowFontEN(65, 80, (u8 *)"-----ms", 24, BRRED, WHITE);
	/* 分割线 */
	POINT_COLOR = BLACK;
	LCD_DrawLine(0,129,239,129);
	/* 左侧蓝条 */
	LCD_Fill(0,130,79,319,LIGHTBLUE);
	
	LCD_ShowChinese(5, 140, newArial, 16, (u8 *)"测试次数", BLACK, LIGHTBLUE);
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTBLUE;
	LCD_ShowString(70, 140, 16,  (u8 *)":");
	LCD_ShowChinese(5, 170, newArial, 16, (u8 *)"失败次数", BLACK, LIGHTBLUE);
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTBLUE;
	LCD_ShowString(70, 170, 16,  (u8 *)":");
	LCD_ShowChinese(5, 200, newArial, 16, (u8 *)"丢包率", BLACK, LIGHTBLUE);
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTBLUE;
	LCD_ShowString(70, 200, 16,  (u8 *)":");
	LCD_ShowChinese(5, 230, newArial, 16, (u8 *)"平均延时", BLACK, LIGHTBLUE);
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTBLUE;
	LCD_ShowString(70, 230, 16,  (u8 *)":");
	LCD_ShowChinese(5, 260, newArial, 16, (u8 *)"最大延时", BLACK, LIGHTBLUE);
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTBLUE;
	LCD_ShowString(70, 260, 16,  (u8 *)":");
	LCD_ShowChinese(5, 290, newArial, 16, (u8 *)"最短延时", BLACK, LIGHTBLUE);
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTBLUE;
	LCD_ShowString(70, 290, 16,  (u8 *)":");
	
	/* 显示当前测试服务器地址 */
	sprintf((char *)msg, "ping %s", PING_ADDR);
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	LCD_ShowString(40, 109, 16,  msg);
	
	while(1)
	{
		result = _CMIOT_GetNetworkDelay((uint8_t *)PING_ADDR, 100, 10000);
		delay_ms(50);
		
		if(result > 0)
		{
			TestCount++;	/* 测试次数+1 */
			sprintf((char *)msg, "%05dms", result);
			LCD_ShowFontEN(65, 80, (u8 *)msg, 24, BRRED, WHITE);
		}
		else
		{
			TestCount++;	/* 测试次数+1 */
			FailCount++;	/* 失败次数+1 */
			LCD_ShowFontEN(65, 80, (u8 *)"timeout ", 24, RED, WHITE);
		}
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		/* 显示测试次数 */
		sprintf((char *)msg, "%-5d", TestCount);
		LCD_ShowString(95, 140, 16,  msg);
		/* 显示失败次数 */
		sprintf((char *)msg, "%-5d", FailCount);
		LCD_ShowString(95, 170, 16,  msg);
		/* 显示丢包率 */
		sprintf((char *)msg, "%-6.2f%%", ((float)FailCount/(float)TestCount)*100);
		LCD_ShowString(95, 200, 16,  msg);

		/* 平均时长 */
		if(result > 0)
		{
			AvrPingDelay = (AvrPingDelay * (TestCount - FailCount - 1) + result)/(TestCount - FailCount);
		}
		sprintf((char *)msg, "%-5dms", AvrPingDelay);
		LCD_ShowString(95, 230, 16,  msg);
		/* 最大时长 */
		if(result > MaxPingDelay && result > 0) { MaxPingDelay = result; }
		sprintf((char *)msg, "%-5dms", MaxPingDelay);
		LCD_ShowString(95, 260, 16,  msg);
		/* 最短时长 */
		if(((result < MinPingDelay) || (MinPingDelay == 0)) && (result > 0)) { MinPingDelay = result; }
		sprintf((char *)msg, "%-5dms", MinPingDelay);
		LCD_ShowString(95, 290, 16,  msg);
		
		_CMIOT_ShowSignalStrength(_CMIOT_M5310_GetSignalstrength());
		delay_ms(5000);
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_ComprehensiveTest
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	综合测试，并展示结果到LCD屏幕
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_ComprehensiveTest(uint32_t times)
{
	uint32_t i = 0;
	uint32_t result = 0;
	uint32_t networkRegCount = 0;
	uint32_t networkRegFailCount = 0;
	uint32_t networkRegAvrtime = 0;
	uint32_t newworkRegMaxtime = 0;
	
	uint32_t csqtestCount = 0;
	uint32_t csqMaxValue = 0;
	uint32_t csqAvrValue = 0;
	uint32_t csqMinValue = 0;
	
	uint32_t pingCount = 0;
	uint32_t pingFailCount = 0;
	uint32_t pingAvrValue = 0;
	uint32_t pingMaxValue = 0;
	uint32_t pingMinValue = 0;
	
	/* 左侧蓝条 */
	LCD_Fill(0,40,79,319,LIGHTBLUE);
	
	LCD_ShowChinese(5, 50, newArial, 16, (u8 *)"测试进度", BLACK, LIGHTBLUE);
	LCD_ShowChinese(5, 110, newArial, 16, (u8 *)"驻网时间", BLACK, LIGHTBLUE);
	LCD_ShowChinese(5, 190, newArial, 16, (u8 *)"信号质量", BLACK, LIGHTBLUE);
	LCD_ShowChinese(5, 270, newArial, 16, (u8 *)"网络延时", BLACK, LIGHTBLUE);

	/* 分割线 */
	POINT_COLOR = DARKBLUE;
	LCD_DrawLine(0,79,239,79);
	LCD_DrawLine(0,159,239,159);
	LCD_DrawLine(0,239,239,239);
	LCD_DrawLine(80,40,80,319);
	
	POINT_COLOR = PROGRESS_BAR_COLOR;
	LCD_DrawRectangle(125, 49, 236 ,67);
	
	LCD_ShowChinese(85, 86, newArial, 16, (u8 *)"平均值：", BLACK, WHITE);
	LCD_ShowChinese(85, 109, newArial, 16, (u8 *)"最大值：", BLACK, WHITE);
	LCD_ShowChinese(85, 132, newArial, 16, (u8 *)"成功率：", BLACK, WHITE);
	
	LCD_ShowChinese(85, 166, newArial, 16, (u8 *)"平均值：", BLACK, WHITE);
	LCD_ShowChinese(85, 189, newArial, 16, (u8 *)"最小值：", BLACK, WHITE);
	LCD_ShowChinese(85, 212, newArial, 16, (u8 *)"最大值：", BLACK, WHITE);
	
	LCD_ShowChinese(85, 246, newArial, 16, (u8 *)"平均值：", BLACK, WHITE);
	LCD_ShowChinese(85, 269, newArial, 16, (u8 *)"最大值：", BLACK, WHITE);
	LCD_ShowChinese(85, 292, newArial, 16, (u8 *)"丢包率：", BLACK, WHITE);
	
	LCD_Fill(126,50, 125 + 2, 66, LIGHTBLUE);	/* 刷新一点进度条，提示进入测试过程 */
	while(i<times)
	{
		i++;
		/* 显示测试进度 */
		sprintf((char *)msg, "%d/%d", i, times);
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		LCD_ShowString(85,50,16,msg);
		
		result = _CMIOT_M5310_GetRegisterTime();
		delay_ms(50);
		
		if(result > 1)
		{
			networkRegAvrtime = (networkRegAvrtime * (networkRegCount - networkRegFailCount) + result)/(networkRegCount - networkRegFailCount + 1);
			if((networkRegCount - networkRegFailCount) == 0 || result > newworkRegMaxtime)
			{
				newworkRegMaxtime = result;
			}
			networkRegCount++;	/* 测试次数+1 */
		}
		else
		{
			networkRegCount++;	/* 测试次数+1 */
			networkRegFailCount++;	/* 失败次数+1 */
		}
		
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		
		sprintf((char *)msg, "%-5.1fs", (float)networkRegAvrtime/1000);   /* 平均时长 */
		LCD_ShowString(150,86,16,msg);
		sprintf((char *)msg, "%-5.1fs", (float)newworkRegMaxtime/1000);   /* 最大时长 */
		LCD_ShowString(150,109,16,msg);
		sprintf((char *)msg, "%-5.1f%%", ((float)(networkRegCount - networkRegFailCount)/(float)networkRegCount)*100); /* 成功率 */
		LCD_ShowString(150,132,16,msg);
		
		result = _CMIOT_M5310_GetSignalstrength();
		if(result > 0 && result <= 31)
		{
			csqAvrValue = (csqAvrValue * csqtestCount + result)/(csqtestCount + 1);
			if(csqtestCount == 0 || result < csqMinValue)
			{
				csqMinValue = result;
			}
			if(csqtestCount == 0 || result > csqMaxValue)
			{
				csqMaxValue = result;
			}
			csqtestCount ++;
		}
		sprintf((char *)msg, "%-2d", csqAvrValue);   /* CSQ平均值 */
		LCD_ShowString(150,166,16,msg);
		sprintf((char *)msg, "%-2d", csqMinValue); /* CSQ最小值 */
		LCD_ShowString(150,189,16,msg);
		sprintf((char *)msg, "%-2d", csqMaxValue); /* CSQ最大值 */
		LCD_ShowString(150,212,16,msg);
		
		result = _CMIOT_GetNetworkDelay((uint8_t *)"114.114.114.114", 100, 10000);
		if(result > 0)
		{
			pingAvrValue = (pingAvrValue * (pingCount - pingFailCount) + result)/(pingCount - pingFailCount + 1);
			if(pingCount == 0 || result > pingMaxValue)
			{
				pingMaxValue = result;
			}
			if(pingCount == 0 || result < pingMinValue)
			{
				pingMinValue = result;
			}
			pingCount++;	/* 测试次数+1 */
		}
		else
		{
			pingCount++;
			pingFailCount++;
		}
		sprintf((char *)msg, "%-5dms", pingAvrValue);   /* ping平均值 */
		LCD_ShowString(150,246,16,msg);
		sprintf((char *)msg, "%-5dms", pingMaxValue); /* ping最大值 */
		LCD_ShowString(150,269,16,msg);
		sprintf((char *)msg, "%-5.1f%%", ((float)(pingFailCount)/(float)pingCount)*100); /* 丢包率 */
		LCD_ShowString(150,292,16,msg);
		
		LCD_Fill(126,50, 126 + floor(((float)i/(float)times)*110), 66, LIGHTBLUE);
		delay_ms(2000);
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_ShowInstructionMsg
Author			:	zhaoji
Created Time	:	2018.03.30
Description 	: 	使用说明界面
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_ShowInstructionMsg(void)
{
	LCD_ShowChinese(5, 45, Yahei, 16, (u8 *)"驻网测试", LIGHTBLUE, WHITE);
	LCD_ShowChinese(5, 70, newArial, 16, (u8 *)"通过反复驻网测试，获取设备入", BLACK, WHITE);
	LCD_ShowChinese(5, 90, newArial, 16, (u8 *)"网成功率、入网时间等参数", BLACK, WHITE);
	
	LCD_ShowChinese(5, 115, Yahei, 16, (u8 *)"信号参数", LIGHTBLUE, WHITE);
	LCD_ShowChinese(5, 140, newArial, 16, (u8 *)"查看详细网络参数，包括信号电", BLACK, WHITE);
	LCD_ShowChinese(5, 160, newArial, 16, (u8 *)"平、信噪比、频点、定时器参数", BLACK, WHITE);
	
	LCD_ShowChinese(5, 185, Yahei, 16, (u8 *)"网络延时", LIGHTBLUE, WHITE);
	LCD_ShowChinese(5, 210, newArial, 16, (u8 *)"持续测试与服务器的网络延迟情", BLACK, WHITE);
	LCD_ShowChinese(5, 230, newArial, 16, (u8 *)"况，统计丢包率、平时延时信息", BLACK, WHITE);
	
	LCD_ShowChinese(5, 255, Yahei, 16, (u8 *)"速率测试", LIGHTBLUE, WHITE);
	LCD_ShowChinese(5, 280, newArial, 16, (u8 *)"获取无线速率参数", BLACK, WHITE);
	
	while(1)
	{
		/* 显示信号强度状态 */
		_CMIOT_ShowSignalStrength(_CMIOT_M5310_GetSignalstrength());
		delay_ms(5000);
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_BluetoothMode
Author			:	zhaoji
Created Time	:	2018.05.16
Description 	: 	蓝牙直连模式
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_BluetoothMode(void)
{
	/* 清空BLE接收Buffer */
	memset(UART_BLE_RxBuffer, 0, sizeof(UART_BLE_RxBuffer));
	UART_BLE_RxBufferLen = 0;

	LCD_ShowChinese(20, 220, newArial, 16, (u8 *)"请使用微信小程序进行测试", BLACK, WHITE);
	LCD_ShowPicture(70,100,100,100,(u8 *)gImage_qrCode);
	
	/* 进入数据透传模式 */
	_CMIOT_BLE_EnterPassthroughMode();
	
	while(1);
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_ShowSpeedInfo
Author			:	zhaoji
Created Time	:	2018.05.16
Description 	: 	速率查看显示
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_ShowSpeedInfo(void)
{
	CMIOT_UE_STATE_THP ue_state_thp = {0,0,0,0};
	
	/* 左侧蓝条 */
	LCD_Fill(0,40,77,319, LIGHTBLUE);
	/* 分割线 */
	POINT_COLOR = BLACK;
	BACK_COLOR = LIGHTBLUE;
	LCD_DrawLine(79,109,239,109);
	LCD_DrawLine(79,179,239,179);
	LCD_DrawLine(79,249,239,249);
	/* 左侧关键字 */
	LCD_ShowString(5, 65,  16,  (uint8_t *)"MAC UL:");
	LCD_ShowString(5, 135, 16,  (uint8_t *)"MAC DL:");
	LCD_ShowString(5, 205, 16,  (uint8_t *)"RLC UL:");
	LCD_ShowString(5, 275, 16,  (uint8_t *)"RLC DL:");

	while(1)
	{
		/* Ping测试防止模组进入PSM模式后射频参数不准确 */
		_CMIOT_GetNetworkDelay((uint8_t *)"117.135.252.141", 1000, 10000);
		/* 获取速率信息 */
		ue_state_thp = _CMIOT_GetUeTHPStats();
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		/* MAC上行 */
		sprintf((char *)msg, "%-8.2f kb/s", (float)ue_state_thp.MAC_UL/100);
		LCD_ShowString(100, 65, 16,  msg);
		/* MAC下行 */
		sprintf((char *)msg, "%-8.2f kb/s", (float)ue_state_thp.MAC_DL/100);
		LCD_ShowString(100, 135, 16,  msg);
		/* RLC上行 */
		sprintf((char *)msg, "%-8.2f kb/s", (float)ue_state_thp.RLC_UL/100);
		LCD_ShowString(100, 205, 16,  msg);
		/* RLC下行 */
		sprintf((char *)msg, "%-8.2f kb/s", (float)ue_state_thp.RLC_DL/100);
		LCD_ShowString(100, 275, 16,  msg);
		/* 更新信息图标 */
		_CMIOT_ShowSignalStrength(_CMIOT_M5310_GetSignalstrength());
		
		delay_ms(5000);
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_NBIOT_Measurement
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	根据当前选中菜单进行对应的测试，并展示数据到LCD
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_NBIOT_Measurement(uint8_t index)
{
	_CMIOT_Debug("%s(index:%d)\r\n", __func__, index);
	LCD_Fill(0,40,239,319,WHITE);
	LCD_Fill(210,10,235,30,WHITE);
	
	switch(index)
	{
		case 0:
		{
			_CMIOT_ShowDeviceInfo();
			break;
		}
		
		case 1:
		{
			_CMIOT_ShowNetworkRegTime();
			break;
		}
		
		case 2:
		{
			_CMIOT_ShowRadioInfo();
			break;
		}
		
		case 3:
		{
			_CMIOT_ShowPingResult();
			break;
		}
		
		case 4:
		{
			// _CMIOT_ComprehensiveTest(5);
			_CMIOT_ShowSpeedInfo();
			break;
		}
		
		case 5:
		{
			_CMIOT_BluetoothMode();
			break;
		}
		
		case 6:
		{
			_CMIOT_ShowInstructionMsg();
			break;
		}
		default: { break; }
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_GUI_Init
Author			:	zhaoji
Created Time	:	2018.03.14
Description 	: 	GUI界面初始化
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_GUI_Init(int8_t xIndex, int8_t yIndex)
{
	uint8_t index;
	
	_CMIOT_Debug("%s...\r\n", __func__);
	
	index = yIndex * 3 + xIndex;
	
	/* 清屏 */
	LCD_Clear(WHITE);
	
	POINT_COLOR = BLACK;
	LCD_DrawLine(0, 39, 239, 39);
	
	/* 显示logo */
	LCD_ShowPicture(5,5,30,30,(u8 *)LogoImage);
	LCD_ShowChinese(35, 6, Arial, 24, (u8 *)"中国移动", TITLEBLUE, WHITE);

	/* 显示APP图标和功能名 */
	LCD_ShowPicture(15,60,50,50,(u8 *)gImage_settingIcon);
	if(index == 0){ LCD_ShowChinese(8, 110, newArial, 16, (u8 *)"设备信息", BLACK, LIGHTBLUE); }
	else { LCD_ShowChinese(8, 110, newArial, 16, (u8 *)"设备信息", BLACK, WHITE); }
	
	LCD_ShowPicture(95,60,50,50,(u8 *)gImage_cellIcon);
	if(index ==1){ LCD_ShowChinese(88, 110, newArial, 16, (u8 *)"驻网测试", BLACK, LIGHTBLUE); }
	else { LCD_ShowChinese(88, 110, newArial, 16, (u8 *)"驻网测试", BLACK, WHITE); }
	
	LCD_ShowPicture(175,60,50,50,(u8 *)gImage_radioIcon);
	if(index ==2) { LCD_ShowChinese(168, 110, newArial, 16, (u8 *)"信号参数", BLACK, LIGHTBLUE); }
	else{ LCD_ShowChinese(168, 110, newArial, 16, (u8 *)"信号参数", BLACK, WHITE); }
	
	LCD_ShowPicture(15,150,50,50,(u8 *)gImage_pingIcon);
	if(index ==3){ LCD_ShowChinese(8, 200, newArial, 16, (u8 *)"网络延时", BLACK, LIGHTBLUE); }
	else { LCD_ShowChinese(8, 200, newArial, 16, (u8 *)"网络延时", BLACK, WHITE); }
	
	LCD_ShowPicture(95,150,50,50,(u8 *)gImage_speedIcon);
	if(index ==4){ LCD_ShowChinese(88, 200, newArial, 16, (u8 *)"速率测试", BLACK, LIGHTBLUE); }
	else { LCD_ShowChinese(88, 200, newArial, 16, (u8 *)"速率测试", BLACK, WHITE); }
	
	LCD_ShowPicture(175,150,50,50,(u8 *)gImage_bluetoothIcon);
	if(index ==5) { LCD_ShowChinese(168, 200, newArial, 16, (u8 *)"蓝牙直连", BLACK, LIGHTBLUE); }
	else { LCD_ShowChinese(168, 200, newArial, 16, (u8 *)"蓝牙直连", BLACK, WHITE); }
	
	LCD_ShowPicture(15,240,50,50,(u8 *)gImage_instructionsIcon);
	if(index ==6) { LCD_ShowChinese(8, 290, newArial, 16, (u8 *)"使用说明", BLACK, LIGHTBLUE); }
	else { LCD_ShowChinese(8, 290, newArial, 16, (u8 *)"使用说明", BLACK, WHITE); }
	
	/* 显示基站信号标志符号 */
	LCD_ShowPicture(160,10,15,20,(u8 *)gImage_signalSign);
	/* 显示信号强度状态 */
	_CMIOT_ShowSignalStrength(_CMIOT_M5310_GetSignalstrength());
	
	/* 显示电池状态 */
	_CMIOT_ShowBatteryTips();
	
	/* 从蓝牙模式退出来 */
	if(index == 5)
	{
		_CMIOT_Debug("%s(exit form blemode)\r\n", __func__);
		/* 退出透传模式 */
		_CMIOT_BLE_ExitPassthroughMode();
		/* 清空BLE接收Buffer */
		memset(UART_BLE_RxBuffer, 0, sizeof(UART_BLE_RxBuffer));
		UART_BLE_RxBufferLen = 0;
		/* 删除蓝牙命令处理任务 */
		/* 删除创建线程 */
		if(bleCmdProcess_task != NULL)
		{
			vTaskDelete(bleCmdProcess_task);   /* 删除任务 */
			bleCmdProcess_task = NULL;
			_CMIOT_Debug("%s(Delete bleCmdProcess task)\r\n", __func__);
		}
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_ShowSignalStrength
Author			:	zhaoji
Created Time	:	2018.03.23
Description 	: 	显示信号强度图标
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_ShowSignalStrength(uint8_t csqValue)
{
	if(_CMIOT_IsPdpAttached())	/* 当前为附着状态 */
	{
		if(csqValue > 20 && csqValue <= 31)
		{
			LCD_Fill(175,26,178,30,DEEPBLUE);
			LCD_Fill(181,22,184,30,DEEPBLUE);
			LCD_Fill(187,18,190,30,DEEPBLUE);
			LCD_Fill(193,14,196,30,DEEPBLUE);
			LCD_Fill(199,10,202,30,DEEPBLUE);
		}
		else if(csqValue > 15 && csqValue <= 20)
		{
			LCD_Fill(175,26,178,30,DEEPBLUE);
			LCD_Fill(181,22,184,30,DEEPBLUE);
			LCD_Fill(187,18,190,30,DEEPBLUE);
			LCD_Fill(193,14,196,30,DEEPBLUE);
			LCD_Fill(199,10,202,30,WHITE);
		}
		else if(csqValue > 10 && csqValue <= 15)
		{
			LCD_Fill(175,26,178,30,DEEPBLUE);
			LCD_Fill(181,22,184,30,DEEPBLUE);
			LCD_Fill(187,18,190,30,DEEPBLUE);
			LCD_Fill(193,14,196,30,WHITE);
			LCD_Fill(199,10,202,30,WHITE);
		}
		else if(csqValue > 5 && csqValue <= 10)
		{
			LCD_Fill(175,26,178,30,DEEPBLUE);
			LCD_Fill(181,22,184,30,DEEPBLUE);
			LCD_Fill(187,18,190,30,WHITE);
			LCD_Fill(193,14,196,30,WHITE);
			LCD_Fill(199,10,202,30,WHITE);
		}
		else if(csqValue > 0 && csqValue <= 5)
		{
			LCD_Fill(175,26,178,30,DEEPBLUE);
			LCD_Fill(181,22,184,30,WHITE);
			LCD_Fill(187,18,190,30,WHITE);
			LCD_Fill(193,14,196,30,WHITE);
			LCD_Fill(199,10,202,30,WHITE);
		}
		else
		{
			LCD_Fill(175,26,178,30,WHITE);
			LCD_Fill(181,22,184,30,WHITE);
			LCD_Fill(187,18,190,30,WHITE);
			LCD_Fill(193,14,196,30,WHITE);
			LCD_Fill(199,10,202,30,WHITE);
		}
	}
	else
	{
		if(csqValue > 20 && csqValue <= 31)
		{
			LCD_Fill(175,26,178,30,DARKBLUE);
			LCD_Fill(181,22,184,30,DARKBLUE);
			LCD_Fill(187,18,190,30,DARKBLUE);
			LCD_Fill(193,14,196,30,DARKBLUE);
			LCD_Fill(199,10,202,30,DARKBLUE);
		}
		else if(csqValue > 15 && csqValue <= 20)
		{
			LCD_Fill(175,26,178,30,DARKBLUE);
			LCD_Fill(181,22,184,30,DARKBLUE);
			LCD_Fill(187,18,190,30,DARKBLUE);
			LCD_Fill(193,14,196,30,DARKBLUE);
			LCD_Fill(199,10,202,30,WHITE);
		}
		else if(csqValue > 10 && csqValue <= 15)
		{
			LCD_Fill(175,26,178,30,DARKBLUE);
			LCD_Fill(181,22,184,30,DARKBLUE);
			LCD_Fill(187,18,190,30,DARKBLUE);
			LCD_Fill(193,14,196,30,WHITE);
			LCD_Fill(199,10,202,30,WHITE);
		}
		else if(csqValue > 5 && csqValue <= 10)
		{
			LCD_Fill(175,26,178,30,DARKBLUE);
			LCD_Fill(181,22,184,30,DARKBLUE);
			LCD_Fill(187,18,190,30,WHITE);
			LCD_Fill(193,14,196,30,WHITE);
			LCD_Fill(199,10,202,30,WHITE);
		}
		else if(csqValue > 0 && csqValue <= 5)
		{
			LCD_Fill(175,26,178,30,DARKBLUE);
			LCD_Fill(181,22,184,30,WHITE);
			LCD_Fill(187,18,190,30,WHITE);
			LCD_Fill(193,14,196,30,WHITE);
			LCD_Fill(199,10,202,30,WHITE);
		}
		else
		{
			LCD_Fill(175,26,178,30,WHITE);
			LCD_Fill(181,22,184,30,WHITE);
			LCD_Fill(187,18,190,30,WHITE);
			LCD_Fill(193,14,196,30,WHITE);
			LCD_Fill(199,10,202,30,WHITE);
		}
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_ShowBatteryLevel
Author			:	zhaoji
Created Time	:	2018.03.29
Description 	: 	绘制电池电量
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_ShowBatteryLevel(uint8_t percentValue)
{
	if(percentValue > 30)
	{
		POINT_COLOR = BLACK;
		LCD_DrawRectangle(210,12,220,30);
		LCD_Fill(213,10,217,12,BLACK);
		LCD_Fill(211,30-(17*percentValue/100),219,29,GREEN);
		LCD_Fill(211,13,219,29-17*percentValue/100,WHITE);
	}
	else if(percentValue > 20)
	{
		POINT_COLOR = BLACK;
		LCD_DrawRectangle(210,12,220,30);
		LCD_Fill(213,10,217,12,BLACK);
		LCD_Fill(211,30-(17*percentValue/100),219,29,YELLOW);
		LCD_Fill(211,13,219,29-17*percentValue/100,WHITE);
	}
	else
	{
		POINT_COLOR = RED;
		LCD_DrawRectangle(210,12,220,30);
		LCD_Fill(213,10,217,12,RED);
		LCD_Fill(211,30-(17*percentValue/100),219,29,RED);
		LCD_Fill(211,13,219,29-17*percentValue/100,WHITE);
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	_CMIOT_ShowBatteryTips
Author			:	zhaoji
Created Time	:	2018.05.19
Description 	: 	显示电池信息
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void _CMIOT_ShowBatteryTips(void)
{
	float batteryVol = 0;
	
	/* 获取电池电压 */
	batteryVol = cm_getBatteryVol();
	if(batteryVol >= 4.2) { _CMIOT_ShowBatteryLevel(100); }
	else if(batteryVol >= 4.08) { _CMIOT_ShowBatteryLevel(90); }
	else if(batteryVol >= 4) { _CMIOT_ShowBatteryLevel(80); }
	else if(batteryVol >= 3.93) { _CMIOT_ShowBatteryLevel(70); }
	else if(batteryVol >= 3.87) { _CMIOT_ShowBatteryLevel(60); }
	else if(batteryVol >= 3.82) { _CMIOT_ShowBatteryLevel(50); }
	else if(batteryVol >= 3.79) { _CMIOT_ShowBatteryLevel(40); }
	else if(batteryVol >= 3.77) { _CMIOT_ShowBatteryLevel(30); }
	else if(batteryVol >= 3.73) { _CMIOT_ShowBatteryLevel(20); }
	else if(batteryVol >= 3.7) { _CMIOT_ShowBatteryLevel(15); }
	else if(batteryVol >= 3.68) { _CMIOT_ShowBatteryLevel(10); }
	else if(batteryVol >= 3.5) { _CMIOT_ShowBatteryLevel(5); }
	
	if(BATTERY_CHRG || BATTERY_STDBY)
	{
		LCD_ShowPicture(225,10,10,20,(u8 *)gImage_chargePowerIcon);
	}
	else
	{
		LCD_Fill(225, 10, 235, 30, WHITE);
	}
}

