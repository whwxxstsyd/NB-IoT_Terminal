/*-----------------------------------------------------------------------------
File Name   	:   tft_lcd.c
Author          :   zhaoji
Created Time    :   2018.03.09
Description     :   LCD接口(适用于ST7789S)
-----------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
                               Dependencies                                  *
-----------------------------------------------------------------------------*/
#include "tft_lcd.h"
#include "delay.h"
#include "sys.h"
#include "font.h"

/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
**                             Global Vars                                    *
**----------------------------------------------------------------------------*/
//画笔颜色,背景颜色
u16 POINT_COLOR = 0x0000,BACK_COLOR = 0xFFFF;




/*-----------------------------------------------------------------------------
Function Name	:	LCD_SetCursor
Author			:	zhaoji
Created Time	:	2018.03.08
Description 	:	设置光标位置
Input Argv		:	Xpos:横坐标   Ypos:纵坐标
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
__inline void LCD_SetCursor(u16 Xpos, u16 Ypos)
{
#if USE_HORIZONTAL==1
	LCD_WR_REG(0x2a);
	LCD_WR_DATA(Ypos >> 8);
	LCD_WR_DATA(Ypos);
	LCD_WR_REG(0x2b);
	LCD_WR_DATA(Xpos >> 8);
	LCD_WR_DATA(Xpos);
#else
	LCD_WR_REG(0x2a);
	LCD_WR_DATA(Xpos >> 8);
	LCD_WR_DATA(Xpos);
	LCD_WR_REG(0x2b);
	LCD_WR_DATA(Ypos >> 8);
	LCD_WR_DATA(Ypos);
#endif
}


/*-----------------------------------------------------------------------------
Function Name	:	LCD_WriteRAM_Prepare
Author			:	zhaoji
Created Time	:	2018.03.08
Description 	:	开始写入GRAM
Input Argv		:	
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(0x2c);
}


/*-----------------------------------------------------------------------------
Function Name	:	LCD_DrawPoint
Author			:	zhaoji
Created Time	:	2018.03.08
Description 	:	LCD 画点函数
Input Argv		:	x 0~239	y 0~319
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void LCD_DrawPoint(u16 x,u16 y)
{
	LCD_SetCursor(x,y);	/* 设置光标位置 */
	LCD_WriteRAM_Prepare();	/* 开始写入GRAM */	
	LCD_WR_TDATA(POINT_COLOR); 
}


/*-----------------------------------------------------------------------------
Function Name	:	LCD_Clear
Author			:	zhaoji
Created Time	:	2018.03.08
Description 	:	LCD 清屏函数
Input Argv		:	Color 清屏的填充色
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void LCD_Clear(u16 Color)
{
	u32 index=0;      
	LCD_SetCursor(0x00,0x0000);//设置光标位置 
	LCD_WriteRAM_Prepare();     //开始写入GRAM	 	  
	for(index=0;index<76800;index++)
	{
		LCD_WR_TDATA(Color);    
	}
}


/*-----------------------------------------------------------------------------
Function Name	:	LCD_RESET
Author			:	zhaoji
Created Time	:	2018.03.08
Description 	:	LCD 复位函数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void LCD_RESET(void)
{ 
	LCD_RST_SET;//高电平  
 	delay_ms(1);
	LCD_RST_CLR;//低电平
	delay_ms(20);
	LCD_RST_SET;//高电平  
	delay_ms(20);
}


/*-----------------------------------------------------------------------------
Function Name	:	LCD_GPIO_Init
Author			:	zhaoji
Created Time	:	2018.03.08
Description 	:	LCD GPIO初始化函数
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void LCD_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE); //使能PORTB,C时钟和AFIO时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_9|GPIO_Pin_8|GPIO_Pin_7|GPIO_Pin_6|GPIO_Pin_5;	   ///PORTC6~10复用推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure); //GPIOC	

	GPIO_SetBits(GPIOC,GPIO_Pin_10|GPIO_Pin_9|GPIO_Pin_8|GPIO_Pin_7|GPIO_Pin_6|GPIO_Pin_5);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_14|GPIO_Pin_13|GPIO_Pin_12|GPIO_Pin_11|GPIO_Pin_10|GPIO_Pin_9|GPIO_Pin_8;	//  PORTB推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure); //GPIOB
	
	GPIO_SetBits(GPIOB,GPIO_Pin_15|GPIO_Pin_14|GPIO_Pin_13|GPIO_Pin_12|GPIO_Pin_11|GPIO_Pin_10|GPIO_Pin_9|GPIO_Pin_8);
}


/*-----------------------------------------------------------------------------
Function Name	:	LCD_Init
Author			:	zhaoji
Created Time	:	2018.03.08
Description 	:	LCD初始化函数(适用于ST7789S)
Input Argv		:
Output Argv 	:
Return Value	:
-----------------------------------------------------------------------------*/
void LCD_Init(void)
{
	LCD_GPIO_Init();
	LCD_RESET();//复位
	
	//-----------------------------------------------------------------------------------------------// 
	LCD_WR_REG(0x11); 
	delay_ms(120);         //Delay 120ms 
	//--------------------------------------Display Setting------------------------------------------// 
	LCD_WR_REG(0x36); 
	LCD_WR_DATA(0x00); 
	LCD_WR_REG(0x3a); 
	LCD_WR_DATA(0x05); 
	//--------------------------------ST7789S Frame rate setting----------------------------------// 
	LCD_WR_REG(0xb2); 
	LCD_WR_DATA(0x0c); 
	LCD_WR_DATA(0x0c); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x33); 
	LCD_WR_DATA(0x33); 
	LCD_WR_REG(0xb7); 
	LCD_WR_DATA(0x35); 
	//---------------------------------ST7789S Power setting--------------------------------------// 
	LCD_WR_REG(0xbb); 
	LCD_WR_DATA(0x0e); 
	LCD_WR_REG(0xc0); 
	LCD_WR_DATA(0x2c); 
	LCD_WR_REG(0xc2); 
	LCD_WR_DATA(0x01); 
	LCD_WR_REG(0xc3); 
	LCD_WR_DATA(0x12); 
	LCD_WR_REG(0xc4); 
	LCD_WR_DATA(0x20); 
	LCD_WR_REG(0xc6); 
	LCD_WR_DATA(0x0f); 
	LCD_WR_REG(0xd0); 
	LCD_WR_DATA(0xa4); 
	LCD_WR_DATA(0xa1); 
	//--------------------------------ST7789S gamma setting---------------------------------------// 
	LCD_WR_REG(0xe0); 
	LCD_WR_DATA(0xd0); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x05); 
	LCD_WR_DATA(0x0d); 
	LCD_WR_DATA(0x0f); 
	LCD_WR_DATA(0x2b); 
	LCD_WR_DATA(0x3b); 
	LCD_WR_DATA(0x44); 
	LCD_WR_DATA(0x4a); 
	LCD_WR_DATA(0x18); 
	LCD_WR_DATA(0x0f); 
	LCD_WR_DATA(0x0b); 
	LCD_WR_DATA(0x1f); 
	LCD_WR_DATA(0x22); 
	LCD_WR_REG(0xe1); 
	LCD_WR_DATA(0xd0); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x05); 
	LCD_WR_DATA(0x0d); 
	LCD_WR_DATA(0x10); 
	LCD_WR_DATA(0x2b); 
	LCD_WR_DATA(0x3d); 
	LCD_WR_DATA(0x54); 
	LCD_WR_DATA(0x50); 
	LCD_WR_DATA(0x2f); 
	LCD_WR_DATA(0x1f); 
	LCD_WR_DATA(0x1f); 
	LCD_WR_DATA(0x1f); 
	LCD_WR_DATA(0x23); 
	LCD_WR_REG(0x29); 
	LCD_WR_REG(0x2C);
	delay_ms(20);
	
	LCD_Clear(WHITE);
	LCD_LED=1;//点亮背光
}

