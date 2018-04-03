#ifndef __LCD_H
#define __LCD_H		
#include "sys.h"	 
#include "stdlib.h"

/////////////////////////////////////用户配置区///////////////////////////////////	 
//以下2个宏定义，定义屏幕的显示方式及IO速度
#define USE_HORIZONTAL  0	//定义是否使用横屏 		0,不使用.1,使用.
#define LCD_FAST_IO     1 	//定义是否使用快速IO	0,不实用.1,使用
//////////////////////////////////////////////////////////////////////////////////	 


//TFTLCD部分外要调用的函数		   
extern u16  POINT_COLOR;//默认红色    
extern u16  BACK_COLOR; //背景颜色.默认为白色
//定义LCD的尺寸
#if USE_HORIZONTAL==1	//使用横屏
#define LCD_W 320
#define LCD_H 240
#else
#define LCD_W 240
#define LCD_H 320
#endif
////////////////////////////////////////////////////////////////////
//-----------------LCD端口定义---------------- 
//如果使用快速IO，则定义下句，如果不使用，则去掉即可！
//使用快速IO，刷屏速率可以达到28帧每秒！
//普通IO，只能14帧每秒！   

//-----------------LCD端口定义---------------- 
#define	LCD_CS	PBout(15)  //片选端口
#define	LCD_RS	PBout(14)  //数据/命令
#define	LCD_WR	PBout(13)  //写数据
#define	LCD_RD	PBout(12)  //读数据
#define LCD_RST PBout(0)   //LCD复位


#define	LCD_CS_SET  GPIOB->BSRR=1<<15    //片选端口
#define	LCD_RS_SET	GPIOB->BSRR=1<<14    //数据/命令
#define	LCD_WR_SET	GPIOB->BSRR=1<<13    //写数据
#define	LCD_RD_SET	GPIOB->BSRR=1<<12    //读数据
#define	LCD_RST_SET	GPIOB->BSRR=1<<0
							    
#define	LCD_CS_CLR  GPIOB->BRR=1<<15     //片选端口
#define	LCD_RS_CLR	GPIOB->BRR=1<<14     //数据/命令
#define	LCD_WR_CLR	GPIOB->BRR=1<<13     //写数据
#define	LCD_RD_CLR	GPIOB->BRR=1<<12     //读数据
#define	LCD_RST_CLR	GPIOB->BRR=1<<0

//PC0-7,作为数据线
#define DATAOUT(x) GPIOC->ODR=(x>>8);  //数据输出
#define DATAIN     GPIOC->IDR;         //数据输入
//////////////////////////////////////////////////////////////////////
//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
#define DARKGREEN		 0X33C8

//GUI颜色

//#define DARKBLUE      	 0X01CF	//深蓝色
//#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE		0X5458 //灰蓝色
#define DARKBLUE		0x4398	//深蓝色
#define LIGHTBLUE		0x059E	//浅蓝色
#define TITLEBLUE		0x0419

#define PROGRESS_BAR_COLOR 0X1905

//以上三色为PANEL的颜色 

#define LIGHTGREEN     	 0X841F //浅绿色
//#define LIGHTGRAY        0XEF5B //浅灰色(PANNEL)
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色
#define DBROWN			0X2144

#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)
	    															  
extern u16 BACK_COLOR, POINT_COLOR ;  
void LCD_Init(void);
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_Clear(u16 Color);	 
void LCD_SetCursor(u16 Xpos, u16 Ypos);
void LCD_DrawPoint(u16 x,u16 y);//画点
u16  LCD_ReadPoint(u16 x,u16 y); //读点
void Draw_Circle(u16 x0,u16 y0,u8 r);
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);		   
void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color);
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode);//显示一个字符
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size);  //显示一个数字
void LCD_Show2Num(u16 x,u16 y,u16 num,u8 len,u8 size,u8 mode);//显示2个数字
void LCD_ShowString(u16 x,u16 y, u8 size, const u8 *p);		 //显示一个字符串
void ILI9341_PIC(const unsigned char gImage[]);
									    
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue);
u16 LCD_ReadReg(u8 LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(u16 RGB_Code);
u16 LCD_ReadRAM(void);		   
u16 LCD_BGR2RGB(u16 c);

void LCD_ShowFontHZ(u16 x, u16 y, u8 *cn, u8 size, u16 fontColor, u16 backColor);
void LCD_ShowFontEN(u16 x, u16 y, u8 *cn, u8 size, u16 fontColor, u16 backColor);
void LCD_ShowFontHZ_Arial(u16 x, u16 y, u8 *cn, u8 size, u16 fontColor, u16 backColor);
void LCD_ShowPicture(u16 x, u16 y, u16 wide, u16 high,u8 *pic);

//写8位数据函数
//用宏定义,提高速度.
#if LCD_FAST_IO==1 //快速IO
#define LCD_WR_DATA(data){\
LCD_RS_SET;\
LCD_CS_CLR;\
DATAOUT(data<<8);\
LCD_WR_CLR;\
LCD_WR_SET;\
LCD_CS_SET;\
} 
#else//正常IO
#define LCD_WR_DATA(data){\
LCD_RS=1;\
LCD_CS=0;\
DATAOUT(data<<8);\
LCD_WR=0;\
LCD_WR=1;\
LCD_CS=1;\
} 	
#endif	

#define LCD_WR_TDATA(data){\
LCD_RS=1;\
LCD_CS=0;\
DATAOUT(data);\
LCD_WR=0;\
LCD_WR=1;\
DATAOUT(data<<8);\
LCD_WR=0;\
LCD_WR=1;\
LCD_CS=1;\
}


typedef enum
{
	Arial = 0,
	Yahei,
	newArial,
}CN_FONT;


void LCD_ShowChinese(u16 x, u16 y, CN_FONT font, u8 size, u8 *cn, u16 fontColor, u16 backColor);












				  		 
#endif  
	 
	 



