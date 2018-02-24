/*-----------------------------------------------------------------------------
File Name   	  :   lcd.h
Author          :   zhaoji
Created Time    :   2018.02.24
Description     :   LCD接口
-----------------------------------------------------------------------------*/

#ifndef __LCD_H__
#define __LCD_H__

#include "stm32f10x.h"
#include "sys.h"
#include "stddef.h"


/*---------------------------------------------------------------------------*
                               Dependencies                                  *
-----------------------------------------------------------------------------*/
#define TFTLCD_R61509V

#define	Data0_OUT          PDout(14)
#define	Data1_OUT          PDout(15)
#define	Data2_OUT          PDout(0)
#define	Data3_OUT          PDout(1)
#define	Data4_OUT          PEout(7)
#define	Data5_OUT          PEout(8)
#define	Data6_OUT          PEout(9)
#define	Data7_OUT          PEout(10)
#define	Data8_OUT          PEout(11)
#define	Data9_OUT          PEout(12)
#define	Data10_OUT         PEout(13)
#define	Data11_OUT         PEout(14)
#define	Data12_OUT         PEout(15)
#define	Data13_OUT         PDout(8)
#define	Data14_OUT         PDout(9)
#define	Data15_OUT         PDout(10)


#define CS                 PGout(12)
#define RS                 PGout(0)
#define WR                 PDout(5)
#define RD                 PDout(4)


//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	   0x001F  
#define BRED             0XF81F
#define GRED 			       0XFFE0
#define GBLUE			       0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			     0XBC40 //棕色
#define BRRED 			     0XFC07 //棕红色
#define GRAY  			     0X8430 //灰色



typedef enum
{
	CM_DISABLE,
	CM_ENABLE
} CM_STATE;


//TFTLCD重要参数集
typedef struct  
{										    
	u16 width;			//LCD 宽度
	u16 height;			//LCD 高度
	u16 id;				  //LCD ID
	u8  dir;        //LCD 方向
}_tftlcd_data;


void TFTLCD_Init(void);

void LCD_ShowString(u16 x,u16 y,u16 width,u16 height,u8 size,u8 *p);

void LCD_Clear(u16 color);

#endif

