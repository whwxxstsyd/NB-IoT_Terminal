/*-----------------------------------------------------------------------------
File Name   	  :   lcd.c
Author          :   zhaoji
Created Time    :   2018.02.24
Description     :   LCD接口
-----------------------------------------------------------------------------*/
#include "lcd.h"
#include "font.h"
#include "common.h"
#include "delay.h"

/*----------------------------------------------------------------------------*
                               Dependencies                                   *
------------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
**                             Global Vars                                    *
**----------------------------------------------------------------------------*/
_tftlcd_data  tftlcd_data;

//LCD的画笔颜色和背景色	   
u16 FRONT_COLOR=BLACK;	//画笔颜色
u16 BACK_COLOR=WHITE;  //背景色 

/*----------------------------------------------------------------------------*
**                             Local Vars                                     *
**----------------------------------------------------------------------------*/


void _CMIOT_Ctrl_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG,ENABLE);    

	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;       
	GPIO_Init(GPIOD,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;       
	GPIO_Init(GPIOG,&GPIO_InitStructure);
}


void Write_config(FunctionalState NewState)
{
	GPIO_InitTypeDef GPIO_InitStructure;            
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);   
	if(NewState!=DISABLE)
	{
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_8|
									GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_14|GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;      
		GPIO_Init(GPIOD,&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|
									GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|
									GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;      
		GPIO_Init(GPIOE,&GPIO_InitStructure);
	}
	else
	{
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_8|
									GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_14|GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;       
		GPIO_Init(GPIOD,&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|
									GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|
									GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;        
		GPIO_Init(GPIOE,&GPIO_InitStructure);
	}
}


void Write_data(short unsigned int Data)
{
	short unsigned int temp=0x0000;
	temp=Data;
	Data0_OUT=(temp>>0)&0x0001;
	Data1_OUT=(temp>>1)&0x0001;
	Data2_OUT=(temp>>2)&0x0001;
	Data3_OUT=(temp>>3)&0x0001;
	Data4_OUT=(temp>>4)&0x0001;
	Data5_OUT=(temp>>5)&0x0001;
	Data6_OUT=(temp>>6)&0x0001;
	Data7_OUT=(temp>>7)&0x0001;
	Data8_OUT=(temp>>8)&0x0001;
	Data9_OUT=(temp>>9)&0x0001;
	Data10_OUT=(temp>>10)&0x0001;
	Data11_OUT=(temp>>11)&0x0001;
	Data12_OUT=(temp>>12)&0x0001;
	Data13_OUT=(temp>>13)&0x0001;
	Data14_OUT=(temp>>14)&0x0001;
	Data15_OUT=(temp>>15)&0x0001;
}




/**
 *LCD写数据
 *
 *
 */
void LCD_WriteData(short unsigned int Data)
{
	Write_config(ENABLE);
	CS=0;
	RS=1;
	RD=1;
	Write_data(Data);
	WR=0;
	WR=1;
	CS=1;
}


/**
 *LCD写命令
 *
 *·
 */
void LCD_WriteCmd(short unsigned int Cmd)
{
	Write_config(ENABLE);
	CS=0;
	RS=0;
	RD=1;
	Write_data(Cmd);
	WR=0;
	WR=1;
	CS=1;
}



//设置窗口,并自动设置画点坐标到窗口左上角(sx,sy).
//sx,sy:窗口起始坐标(左上角)
//width,height:窗口宽度和高度,必须大于0!!
//窗体大小:width*height. 
void LCD_Set_Window(u16 sx,u16 sy,u16 width,u16 height)
{
	if(tftlcd_data.dir==0)
	{
		LCD_WriteCmd(0x0210);   
	    LCD_WriteData(sx);
		LCD_WriteCmd(0x0211);  
	    LCD_WriteData(width);
		LCD_WriteCmd(0x0212);   
	    LCD_WriteData(sy);
		LCD_WriteCmd(0x0213);   
	    LCD_WriteData(height);
	
		LCD_WriteCmd(0x0200);   
	    LCD_WriteData(sx);
		LCD_WriteCmd(0x0201);   
	    LCD_WriteData(sy);	
	}   	
	else
	{
		LCD_WriteCmd(0x0212);   
	    LCD_WriteData(sx);
		LCD_WriteCmd(0x0213);  
	    LCD_WriteData(width);
		LCD_WriteCmd(0x0210);   
	    LCD_WriteData(sy);
		LCD_WriteCmd(0x0211);   
	    LCD_WriteData(height);
	
		LCD_WriteCmd(0x0201);   
	    LCD_WriteData(sx);
		LCD_WriteCmd(0x0200);   
	    LCD_WriteData(sy);	
	}
	LCD_WriteCmd(0x0202);
}



void LCD_WriteData_Color(u16 color)
{
	LCD_WriteData(color);
}



//快速画点
//x,y:坐标
//color:颜色
void LCD_DrawFRONT_COLOR(u16 x,u16 y,u16 color)
{	   
	LCD_Set_Window(x, y, x, y);
	LCD_WriteData_Color(color);	
}


//在指定位置显示一个字符
//x,y:起始坐标
//num:要显示的字符:" "--->"~"
//size:字体大小 12/16/24
//mode:叠加方式(1)还是非叠加方式(0)
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode)
{  							  
    u8 temp,t1,t;
	u16 y0=y;
	u8 csize=(size/8+((size%8)?1:0))*(size/2);		//得到字体一个字符对应点阵集所占的字节数	
 	num=num-' ';//得到偏移后的值（ASCII字库是从空格开始取模，所以-' '就是对应字符的字库）
	for(t=0;t<csize;t++)
	{   
		if(size==12)temp=ascii_1206[num][t]; 	 	//调用1206字体
		else if(size==16)temp=ascii_1608[num][t];	//调用1608字体
		else if(size==24)temp=ascii_2412[num][t];	//调用2412字体
		else return;								//没有的字库
		for(t1=0;t1<8;t1++)
		{			    
			if(temp&0x80)LCD_DrawFRONT_COLOR(x,y,FRONT_COLOR);
			else if(mode==0)LCD_DrawFRONT_COLOR(x,y,BACK_COLOR);
			temp<<=1;
			y++;
			if(y>=tftlcd_data.height)return;		//超区域了
			if((y-y0)==size)
			{
				y=y0;
				x++;
				if(x>=tftlcd_data.width)return;	//超区域了
				break;
			}
		}  	 
	}  	    	   	 	  
} 


//显示字符串
//x,y:起点坐标
//width,height:区域大小  
//size:字体大小
//*p:字符串起始地址		  
void LCD_ShowString(u16 x,u16 y,u16 width,u16 height,u8 size,u8 *p)
{         
	u8 x0=x;
	width+=x;
	height+=y;
    while((*p<='~')&&(*p>=' '))//判断是不是非法字符!
    {       
        if(x>=width){x=x0;y+=size;}
        if(y>=height)break;//退出
        LCD_ShowChar(x,y,*p,size,0);
        x+=size/2;
        p++;
    }  
}


//设置LCD显示方向
//dir:0,竖屏；1,横屏
void LCD_Display_Dir(u8 dir)
{
	if(dir==0)  //默认竖屏方向
	{	
		LCD_WriteCmd(0x0001);   
		LCD_WriteData(0x0100);
		LCD_WriteCmd(0x0003);
		LCD_WriteData(0x1030);
		tftlcd_data.height=400;
		tftlcd_data.width=240;

		tftlcd_data.dir=0;
	}
	else
	{	
		LCD_WriteCmd(0x0001);   
		LCD_WriteData(0x0000);
		LCD_WriteCmd(0x0003);
		LCD_WriteData(0x1038);
		tftlcd_data.height=240;
		tftlcd_data.width=400;

		tftlcd_data.dir=1;
	}
}


//清屏函数
//color:要清屏的填充色
void LCD_Clear(u16 color)
{
	uint16_t i, j ;

	LCD_Set_Window(0, 0, tftlcd_data.width-1, tftlcd_data.height-1);	 //作用区域
  	for(i=0; i<tftlcd_data.width; i++)
	{
		for (j=0; j<tftlcd_data.height; j++)
		{
			LCD_WriteData_Color(color);
		}
	} 
}



void TFTLCD_Init(void)
{
	u16 i;
	_CMIOT_Ctrl_GPIO_Init();

	delay_ms(50); 

	LCD_WriteCmd(0x0000);LCD_WriteData(0x0000);
	LCD_WriteCmd(0x0000);LCD_WriteData(0x0000);
	LCD_WriteCmd(0x0000);LCD_WriteData(0x0000);

	LCD_WriteCmd(0x0400);LCD_WriteData(0x6200); 
	LCD_WriteCmd(0x0008);LCD_WriteData(0x0808); 


	LCD_WriteCmd(0x0001);LCD_WriteData(0x0100);
	LCD_WriteCmd(0x0002);LCD_WriteData(0x0100);
	LCD_WriteCmd(0x0003);LCD_WriteData(0x1030);
	LCD_WriteCmd(0x0008);LCD_WriteData(0x0808);
	/*gamma set 20090423 BY LYJ*/

	LCD_WriteCmd(0x0300);LCD_WriteData(0x000c); 
	LCD_WriteCmd(0x0301);LCD_WriteData(0x5A0B); 
	LCD_WriteCmd(0x0302);LCD_WriteData(0x0906); 
	LCD_WriteCmd(0x0303);LCD_WriteData(0x1017); 
	LCD_WriteCmd(0x0304);LCD_WriteData(0x2300); 
	LCD_WriteCmd(0x0305);LCD_WriteData(0x1700); 
	LCD_WriteCmd(0x0306);LCD_WriteData(0x6309); 
	LCD_WriteCmd(0x0307);LCD_WriteData(0x0c09); 
	LCD_WriteCmd(0x0308);LCD_WriteData(0x100c); 
	LCD_WriteCmd(0x0309);LCD_WriteData(0x2232); 
	/*power set*/
	LCD_WriteCmd(0x0010);LCD_WriteData(0x0014);
	LCD_WriteCmd(0x0011);LCD_WriteData(0x0101);  
	LCD_WriteCmd(0x0100);LCD_WriteData(0x0230);//
	//for(i=500; i>0; i--);
	delay_ms(50);
	LCD_WriteCmd(0x0101);LCD_WriteData(0x0247);//
	for(i=500; i>0; i--);
	delay_ms(50);
	LCD_WriteCmd(0x0103);LCD_WriteData(0x0a00);//Starts VLOUT3,Sets the VREG1OUT.
	for(i=500; i>0; i--);
	delay_ms(10);
	LCD_WriteCmd(0x0280);LCD_WriteData(0xf0ff);//VCOMH voltage   //0xcEff
	delay_ms(50);
	for(i=500; i>0; i--);
	LCD_WriteCmd(0x0102);LCD_WriteData(0xB1b0);//Starts VLOUT3,Sets the VREG1OUT.
	for(i=500; i>0; i--);
	delay_ms(50);
	/*window set*/
	LCD_WriteCmd(0x0210);LCD_WriteData(0x0000);//Window Horizontal RAM Address Start
	LCD_WriteCmd(0x0211);LCD_WriteData(0x00ef);//Window Horizontal RAM Address End
	LCD_WriteCmd(0x0212);LCD_WriteData(0x0000);//Window Vertical RAM Address Start
	LCD_WriteCmd(0x0213);LCD_WriteData(0x018f);//Window Vertical RAM Address End
	LCD_WriteCmd(0x0200);LCD_WriteData(0x0000);//RAM Address Set (Horizontal Address)
	LCD_WriteCmd(0x0201);LCD_WriteData(0x0000);//RAM Address Set (Vertical Address)
	LCD_WriteCmd(0x0401);LCD_WriteData(0x0000);//Base Image Display
	LCD_WriteCmd(0x0007);LCD_WriteData(0x0100);//Display Control 1
	for(i=500; i>0; i--);
	delay_ms(50);
	LCD_WriteCmd(0x0202);

	LCD_Display_Dir(0);		//0：竖屏  1：横屏  默认竖屏
	LCD_Clear(WHITE);
}


