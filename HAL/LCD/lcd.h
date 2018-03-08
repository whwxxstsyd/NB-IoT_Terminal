#include "stm32f10x.h"
#include <stdio.h>
#include "stdlib.h"
#include <delay.h>
#include "sys.h"


#define LCD_WIDTH 240
#define LCD_HEIGHT 320


#define RED    0xF800		 //?����???��?3�ꨢ? 
#define GREEN  0x07E0
#define BLUE   0x001F
#define WHITE  0xFFFF
#define BLACK  0x0000
#define GRAY   0xEF5D	     //0x2410
#define GRAY75 0x39E7 
#define GRAY50 0x7BEF	
#define GRAY25 0xADB5	


#define CHAR_FONT_W8_H16	//??????¨º?¡Á?¡¤?¦Ì?¡Á?¨¬?¡ê?CHAR_FONT_W8_H16¡êo8*16¡ê?CHAR_FONT_W16_H21¡êo16*21
//#define CHAR_FONT_W16_H21
#ifdef  CHAR_FONT_W8_H16	
	#define  FONT_W  8
	#define  FONT_H  16
#endif
#ifdef  CHAR_FONT_W16_H21
	#define  FONT_W  16
	#define  FONT_H  21
#endif


#define LCD_BASE0        ((u32)0x60000000)
#define LCD_BASE1        ((u32)0x60020000)

//#define LCD_CtrlWrite(cmd)	  *(vu16*) (LCD_BASE0)= (cmd)
//#define LCD_DataWrite(data)   *(vu16*) (LCD_BASE1)= (data)
#define	LCD_StatusRead()	 *(vu16*) (LCD_BASE0)
#define	LCD_DataRead()   	 *(vu16*) (LCD_BASE1)

//#define MCU_DIR_Write GPIOA->BSRR = GPIO_Pin_0
//#define MCU_DIR_Read  GPIOA->BRR = GPIO_Pin_0

//#define RGB_DIR_Write GPIOC->BRR = GPIO_Pin_0
//#define RGB_DIR_Read GPIOC->BSRR = GPIO_Pin_0

//#define H_Reset GPIOD->BSRR = GPIO_Pin_12
//#define L_Reset GPIOD->BRR  = GPIO_Pin_12


//#define H_CS GPIOC->BSRR = GPIO_Pin_9
//#define L_CS GPIOC->BRR  = GPIO_Pin_9

//#define H_RS GPIOC->BSRR = GPIO_Pin_8
//#define L_RS GPIOC->BRR  = GPIO_Pin_8

//#define H_SCL GPIOB->BSRR = GPIO_Pin_13
//#define L_SCL GPIOB->BRR  = GPIO_Pin_13

//#define H_SDA GPIOB->BSRR = GPIO_Pin_15
//#define L_SDA GPIOB->BRR  = GPIO_Pin_15

#define	LCD_CS_SET  GPIOC->BSRR=1<<9    //片选端口  		PC9
#define	LCD_RS_SET	GPIOC->BSRR=1<<8    //数据/命令 		PC8	   
#define	LCD_WR_SET	GPIOC->BSRR=1<<7    //写数据			PC7
#define	LCD_RD_SET	GPIOC->BSRR=1<<6    //读数据			PC6
								    
#define	LCD_CS_CLR  GPIOC->BRR=1<<9     //片选端口  		PC9
#define	LCD_RS_CLR	GPIOC->BRR=1<<8     //数据/命令		PC8	   
#define	LCD_WR_CLR	GPIOC->BRR=1<<7     //写数据			PC7
#define	LCD_RD_CLR	GPIOC->BRR=1<<6     //读数据			PC6  

#define Data0out	PBout(8)
#define Data1out	PBout(9)
#define Data2out	PBout(10)
#define Data3out	PBout(11)
#define Data4out	PBout(12)
#define Data5out	PBout(13)
#define Data6out	PBout(14)
#define Data7out	PBout(15)


void SPI_Send(unsigned char _dat);

#define LCD_CtrlWrite(cmd)\
				LCD_RS_CLR;\
				LCD_CS_CLR;\
				SPI_Send(cmd);\
				LCD_WR_CLR;\
				LCD_WR_SET;\
				LCD_CS_SET;
				



#define LCD_DataWrite(data)\
				LCD_RS_SET;\
				LCD_CS_CLR;\
				SPI_Send(data);\
				LCD_WR_CLR;\
				LCD_WR_SET;\
				LCD_CS_SET;

void LCD_GPIO_Init();
void LCD_RST(void);
void LCD_Init(void);
void BlockWrite(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend);
void DispColor(unsigned int color);
void sd_showbmp(char * fname);
u16 SPI_Read_ID(void);
extern const unsigned char ascii[]; 
void DispStr(unsigned char *str,unsigned int Xstart,unsigned int Ystart,unsigned int TextColor,unsigned int BackColor);

//void SPI_CMD(unsigned int _cmd);
//void SPI_DAT(unsigned int _dat);


void SPI_8BIT(void);
void SPI_16BIT(void);
u16 ReadData();
u16 Long_Read(u8 adrress);
u16 Short_Read(u8 adrress);


