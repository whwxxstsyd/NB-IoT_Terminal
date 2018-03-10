/*-----------------------------------------------------------------------------
File Name		:   st7789s.h
Author			:   zhaoji
Created Time	:   2018.01.12
Description		:   LCD 接口(驱动IC: ST7789S)
-----------------------------------------------------------------------------*/

#ifndef __TFT_LCD__
#define __TFT_LCD__


/*---------------------------------------------------------------------------*
                               Dependencies                                  *
-----------------------------------------------------------------------------*/	 



/*----------------------------------------------------------------------------*
**                             Mcaro Definitions                              *
**----------------------------------------------------------------------------*/
#define USE_HORIZONTAL  1	/* 定义是否使用横屏 		0:不使用, 1:使用 */

#if USE_HORIZONTAL==1	    /* 使用横屏 */
#define LCD_W 320
#define LCD_H 240
#else
#define LCD_W 240
#define LCD_H 320
#endif

/*---------------------------------LCD端口定义---------------------------------*/
#define	LCD_LED		PCout(10) 			/* LCD背光    	    PC10 */

#define	LCD_CS_SET  GPIOC->BSRR=1<<9    /* 片选端口  		PC9  */
#define	LCD_RS_SET	GPIOC->BSRR=1<<8    /* 数据/命令 		PC8	 */
#define	LCD_WR_SET	GPIOC->BSRR=1<<7    /* 写数据			PC7  */
#define	LCD_RD_SET	GPIOC->BSRR=1<<6    /* 读数据			PC6  */
#define LCD_RST_SET GPIOC->BSRR=1<<5    /* 复位				PC5  */

#define	LCD_CS_CLR  GPIOC->BRR=1<<9     /* 片选端口  		PC9  */
#define	LCD_RS_CLR	GPIOC->BRR=1<<8     /* 数据/命令		PC8	 */ 
#define	LCD_WR_CLR	GPIOC->BRR=1<<7     /* 写数据			PC7  */
#define	LCD_RD_CLR	GPIOC->BRR=1<<6     /* 读数据			PC6  */
#define LCD_RST_CLR GPIOC->BRR=1<<5     /* 复位				PC5  */

/* PB0~15,作为数据线 */
#define DATAOUT(x) GPIOB->ODR=x; //数据输出


/* 写数据函数 */
#define LCD_WR_DATA(data){\
LCD_RS_SET;\
LCD_CS_CLR;\
DATAOUT(data);\
LCD_WR_CLR;\
LCD_WR_SET;\
LCD_CS_SET;\
}


#define LCD_WR_TDATA(data){\
LCD_RS_SET;\
LCD_CS_CLR;\
DATAOUT(data);\
LCD_WR_CLR;\
LCD_WR_SET;\
DATAOUT(data<<8);\
LCD_WR_CLR;\
LCD_WR_SET;\
LCD_CS_SET;\
}


/* 写命令函数 */
#define LCD_WR_REG(data){\
LCD_RS_CLR;\
LCD_CS_CLR;\
DATAOUT(data);\
LCD_WR_CLR;\
LCD_WR_SET;\
LCD_CS_SET;\
}


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
//GUI颜色

#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 

#define LIGHTGREEN     	 0X841F //浅绿色
#define LIGHTGRAY        0XEF5B //浅灰色(PANNEL)
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)

#endif


