/*
  ******************************************************************************
  * @file    LCD
  * @author  LCD
  * @version V1.0
  * @date    20170101
  * @brief   LCD
  ******************************************************************************
*/

#include "lcd.h"

#include "stm32f10x_gpio.h"


#define SPIT_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define SPIT_LONG_TIMEOUT         ((uint32_t)(10 * SPIT_FLAG_TIMEOUT))
static __IO uint32_t  SPITimeout = SPIT_LONG_TIMEOUT;    

void SPI_Send(unsigned char _dat)
{
	Data0out = (_dat>>0)&0x01;
	Data1out = (_dat>>1)&0x01;
	Data2out = (_dat>>2)&0x01;
	Data3out = (_dat>>3)&0x01;
	Data4out = (_dat>>4)&0x01;
	Data5out = (_dat>>5)&0x01;
	Data6out = (_dat>>6)&0x01;
	Data7out = (_dat>>7)&0x01;
}

void SPI_8BIT(void)
{
	SPI_InitTypeDef  SPI_InitStructure;

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//串行同步时钟的空闲状态为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;//SPI_BaudRatePrescaler_2;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI2, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
 
	SPI_Cmd(SPI2, ENABLE); //使能SPI外设
}	
void SPI_16BIT(void)
{
	SPI_InitTypeDef  SPI_InitStructure;

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;		//设置SPI的数据大小:SPI发送接收16位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//串行同步时钟的空闲状态为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;//SPI_BaudRatePrescaler_2;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI2, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
 
	//SPI_Cmd(SPI2, ENABLE); //使能SPI外设
}	

u16 id=0;

u8 SPI_FLASH_SendByte(u8 byte)
{
	 SPITimeout = SPIT_FLAG_TIMEOUT;
  /* 等待发送缓冲区为空，TXE事件 */
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
	{
    if((SPITimeout--) == 0) return 0;
   }

  /* 写入数据寄存器，把要写入的数据写入发送缓冲区 */
  SPI_I2S_SendData(SPI2 , byte);

	SPITimeout = SPIT_FLAG_TIMEOUT;
  /* 等待接收缓冲区非空，RXNE事件 */
  while (SPI_I2S_GetFlagStatus(SPI2 , SPI_I2S_FLAG_RXNE) == RESET)
  {
    if((SPITimeout--) == 0) return 0;
   }

  /* 读取数据寄存器，获取接收缓冲区数据 */
  return SPI_I2S_ReceiveData(SPI2 );
}




void LCD_RST(void)
{
//	H_Reset;
//	delay_ms(20);
//	L_Reset;
//	delay_ms(20);
//	H_Reset;
//	delay_ms(120);
}

void BlockWrite(unsigned int Xstart,unsigned int Xend,unsigned int Ystart,unsigned int Yend)
{
	LCD_CtrlWrite(0x2a);   
	LCD_DataWrite(Xstart>>8);
	LCD_DataWrite(Xstart&0xff);
	LCD_DataWrite(Xend>>8);
	LCD_DataWrite(Xend&0xff);

	LCD_CtrlWrite(0x2b);   
	LCD_DataWrite(Ystart>>8);
	LCD_DataWrite(Ystart&0xff);
	LCD_DataWrite(Yend>>8);
	LCD_DataWrite(Yend&0xff);

	LCD_CtrlWrite(0x2c);
}

#if 1
void DispColor(unsigned int color)
{
	unsigned int i,j;

	BlockWrite(0,LCD_WIDTH-1,0,LCD_HEIGHT-1);

	for(i=0;i<LCD_WIDTH;i++)
	{
	    for(j=0;j<LCD_HEIGHT;j++)
		{    
			LCD_DataWrite((color>>16)&0xFF);  ////RGB666 mode
			LCD_DataWrite((color>>8)&0xFF);
			LCD_DataWrite(color&0xFF);
		}
	}

}	
#else
void DispColor(unsigned int color)
{
	unsigned int i,j;

	BlockWrite(0,LCD_WIDTH-1,0,LCD_HEIGHT-1);

	for(i=0;i<LCD_WIDTH;i++)
	{
	    for(j=0;j<LCD_HEIGHT;j++)
		{    
			 LCD_DataWrite(color>>8);    /////RGB565 mode
			 LCD_DataWrite(color&0xFF);
		}
	}

}	
#endif
u16 SPI_Read_ID()
{
	unsigned char lcd_id_h=0,lcd_id_l=0;
    u16 lcd_id=0;

	LCD_CtrlWrite(0xdb);
	lcd_id_h = ReadData(); 

	LCD_CtrlWrite(0xdc);
	lcd_id_l = ReadData(); 
	
	lcd_id=lcd_id_h<<8|lcd_id_l;
	return lcd_id;
}

u16 ReadData()
{
//	u8 i=0,dat=0;

//	for(i=0;i<8;i++)
//	{
//		L_SCL;
//		MCU_DIR_Read;
//		RGB_DIR_Read;
//		__NOP;__NOP;__NOP;
//		dat=(dat<<1)|GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15);

//		MCU_DIR_Write;
//		RGB_DIR_Write;
//		__NOP;__NOP;__NOP;
//		H_SCL;
//		__NOP;__NOP;__NOP;
//	}
//	return dat;
}

u16 Short_Read(u8 adrress)
{
//	u8 dat=0;
//	L_CS;
//	L_RS;
//	SPI_Send(adrress);
//	H_RS;
//	dat=ReadData();
//	H_CS;
//	return dat;
}

u16 Long_Read(u8 adrress)
{
//	u16 data=0x00,Read_1,Read_2,Read_3,Read_4;
//	
//	L_CS;
//	L_RS;
//	SPI_Send(adrress);
//	H_RS;
//	
//	L_SCL;
//	H_SCL; ////dummy read 
//	
//	Read_1=ReadData();
//	Read_2=ReadData();
//	Read_3=ReadData();
////	Read_4=ReadData();

//	H_CS;
//	data=Read_2<<8|Read_3;
//	return data;
}

void GC9304_Init()
{
	LCD_CtrlWrite(0xfe);// internal reg enable	
	LCD_CtrlWrite(0xef);//// internal reg enable	
	LCD_CtrlWrite(0x36);
	LCD_DataWrite(0x48);
	LCD_CtrlWrite(0x3a);
	LCD_DataWrite(0x06);
	LCD_CtrlWrite(0xe8);
	LCD_DataWrite(0x12);
	LCD_DataWrite(0x22);
		//=============		

	LCD_CtrlWrite(0xe3);
	LCD_DataWrite(0x01);
	LCD_DataWrite(0x04);
	LCD_CtrlWrite(0xa5);
	LCD_DataWrite(0x40);
	LCD_DataWrite(0x40);
	LCD_CtrlWrite(0xa4);
	LCD_DataWrite(0x44);
	LCD_DataWrite(0x44);
	LCD_CtrlWrite(0xab);
	LCD_DataWrite(0x08);
	LCD_CtrlWrite(0xaa);
	LCD_DataWrite(0x88);
	LCD_DataWrite(0x88);
	LCD_CtrlWrite(0xae);
	LCD_DataWrite(0x0b);//0x1c  20151013 update
	LCD_CtrlWrite(0xac);
	LCD_DataWrite(0x00);
	LCD_CtrlWrite(0xaf);

	LCD_DataWrite(0x77);
	LCD_CtrlWrite(0xad);
	LCD_DataWrite(0x77);

		
	//===============gamma==========		
	LCD_CtrlWrite(0xF0);
	LCD_DataWrite(0x02);
	LCD_DataWrite(0x02);

	LCD_DataWrite(0x00);
	LCD_DataWrite(0x09);
	LCD_DataWrite(0x13);
	LCD_DataWrite(0x0E);
	LCD_CtrlWrite(0xF1);
	LCD_DataWrite(0x01);
	LCD_DataWrite(0x02);
	LCD_DataWrite(0x00);
	LCD_DataWrite(0x0F);
	LCD_DataWrite(0x1C);
	LCD_DataWrite(0x10);
	LCD_CtrlWrite(0xF2);
	LCD_DataWrite(0x0F);
	LCD_DataWrite(0x08);
	LCD_DataWrite(0x39);
	LCD_DataWrite(0x04);
	LCD_DataWrite(0x05);
	LCD_DataWrite(0x49);
	LCD_CtrlWrite(0xF3);
	LCD_DataWrite(0x11);
	LCD_DataWrite(0x0A);

	LCD_DataWrite(0x41);
	LCD_DataWrite(0x03);
	LCD_DataWrite(0x03);
	LCD_DataWrite(0x4F);
	LCD_CtrlWrite(0xF4);
	LCD_DataWrite(0x0F);
	LCD_DataWrite(0x18);
	LCD_DataWrite(0x16);
	LCD_DataWrite(0x1D);
	LCD_DataWrite(0x20);
	LCD_DataWrite(0x0F);
	LCD_CtrlWrite(0xF5);
	LCD_DataWrite(0x05);
	LCD_DataWrite(0x11);
	LCD_DataWrite(0x11);
	LCD_DataWrite(0x1C);
	LCD_DataWrite(0x1F);
	LCD_DataWrite(0x0F);

	LCD_CtrlWrite(0x35);
	LCD_DataWrite(0x00);


	LCD_CtrlWrite(0x44);
	LCD_DataWrite(0x00);
	LCD_DataWrite(0x0a);

	LCD_CtrlWrite(0x11);
	delay_ms(120);

	LCD_CtrlWrite(0x29);
	LCD_CtrlWrite(0x2c);
}

void GC9305_Init()
{
	LCD_CtrlWrite(0xfe);
	LCD_CtrlWrite(0xef);
	LCD_CtrlWrite(0x36);
	LCD_DataWrite(0x48);
	LCD_CtrlWrite(0x3a);
	LCD_DataWrite(0x06);
	//------end display control setting----//
	//------Power Control Registers Initial----//
	LCD_CtrlWrite(0xa4);
	LCD_DataWrite(0x44);
	LCD_DataWrite(0x44);
	LCD_CtrlWrite(0xa5);
	LCD_DataWrite(0x42);
	LCD_DataWrite(0x42);
	LCD_CtrlWrite(0xaa);
	LCD_DataWrite(0x88);
	LCD_DataWrite(0x88);
	LCD_CtrlWrite(0xe8);
	LCD_DataWrite(0x11);
	LCD_DataWrite(0x0b);
	LCD_CtrlWrite(0xe3);
	LCD_DataWrite(0x01);
	LCD_DataWrite(0x10);
	LCD_CtrlWrite(0xff);
	LCD_DataWrite(0x61);
	LCD_CtrlWrite(0xAC);
	LCD_DataWrite(0x00);
	LCD_CtrlWrite(0xAd);//ldo enable
	LCD_DataWrite(0x33);
	
	LCD_CtrlWrite(0xae);
	LCD_DataWrite(0x2b);
	
	LCD_CtrlWrite(0xAf);//DIG_VREFAD_VRDD[2]
	LCD_DataWrite(0x55);
	
	LCD_CtrlWrite(0xa6);
	LCD_DataWrite(0x2a);
	LCD_DataWrite(0x2a);
	LCD_CtrlWrite(0xa7);
	LCD_DataWrite(0x2b);
	LCD_DataWrite(0x2b);
	LCD_CtrlWrite(0xa8);
	LCD_DataWrite(0x18);
	LCD_DataWrite(0x18);
	LCD_CtrlWrite(0xa9);
	LCD_DataWrite(0x2a);
	LCD_DataWrite(0x2a);
	
	//-----display window 240X320---------//
	LCD_CtrlWrite(0x2a);
	LCD_DataWrite(0x00);
	LCD_DataWrite(0x00);
	LCD_DataWrite(0x00);
	LCD_DataWrite(0xef);
	LCD_CtrlWrite(0x2b);
	LCD_DataWrite(0x00);
	LCD_DataWrite(0x00);
	LCD_DataWrite(0x01);
	LCD_DataWrite(0x3f);
	LCD_CtrlWrite(0x2c);
	//--------end display window --------------//
	//------------gamma setting------------------//
	LCD_CtrlWrite(0xf0);
	LCD_DataWrite(0x2);
	LCD_DataWrite(0x0);
	LCD_DataWrite(0x0);
	LCD_DataWrite(0x25);
	LCD_DataWrite(0x26);
	LCD_DataWrite(0x5);
	LCD_CtrlWrite(0xf1);
	LCD_DataWrite(0x1);
	LCD_DataWrite(0x3);
	LCD_DataWrite(0x0);
	LCD_DataWrite(0x33);
	LCD_DataWrite(0x33);
	LCD_DataWrite(0xA);
	LCD_CtrlWrite(0xf2);
	LCD_DataWrite(0x5);
	LCD_DataWrite(0x3);
	LCD_DataWrite(0x3C);
	LCD_DataWrite(0x4);
	LCD_DataWrite(0x4);
	LCD_DataWrite(0x4C);
	LCD_CtrlWrite(0xf3);
	LCD_DataWrite(0x9);
	LCD_DataWrite(0x4);
	LCD_DataWrite(0x47);
	LCD_DataWrite(0x3);
	LCD_DataWrite(0x2);
	LCD_DataWrite(0x52);
	LCD_CtrlWrite(0xf4);
	LCD_DataWrite(0xA);
	LCD_DataWrite(0x17);
	LCD_DataWrite(0x16);
	LCD_DataWrite(0x1F);
	LCD_DataWrite(0x21);
	LCD_DataWrite(0xF);
	LCD_CtrlWrite(0xf5);
	LCD_DataWrite(0x7);
	LCD_DataWrite(0x12);
	LCD_DataWrite(0x12);
	LCD_DataWrite(0x1F);
	LCD_DataWrite(0x20);
	LCD_DataWrite(0xF);
	//--------end gamma setting--------------//
	LCD_CtrlWrite(0x11);
	delay_ms(120);
	LCD_CtrlWrite(0x29);
	LCD_CtrlWrite(0x2c);
}

void LCD_Init(void)
{
	LCD_RST();
	GC9304_Init();

}



void WriteOneDot(unsigned int color)
{ 
	
	LCD_DataWrite(color>>8);
    LCD_DataWrite(color&0xFF);

}

#if 1//def STRING_FUNCTION

//ascii 32~90(!~Z), (32~47)空格~/,(48~57)0~9,(58~64):~@,(65~126)A~~
//ord 0~95, (48~57)0~9,(65~126)A~z,(33~47)!~/,(58~64):~@
unsigned char ToOrd(unsigned char ch)
{
	if(ch<32)
	{
		ch=95;
	}
	else if((ch>=32)&&(ch<=47)) //(32~47)空格~/
	{
		ch=(ch-32)+10+62;
	}
	else if((ch>=48)&&(ch<=57))//(48~57)0~9
	{
		ch=ch-48;
	}
	else if((ch>=58)&&(ch<=64))//(58~64):~@
	{
		ch=(ch-58)+10+62+16;
	}
	else if((ch>=65)&&(ch<=126))//(65~126)A~~
	{
		ch=(ch-65)+10;
	}
	else if(ch>126)
	{		
		ch=95;
	}

	return ch;
}

void  DispOneChar(unsigned char ord,unsigned int Xstart,unsigned int Ystart,unsigned int TextColor,unsigned int BackColor)	 // ord:0~95
{													  
   unsigned char i,j;
   unsigned char  *p;
   unsigned char dat;
   unsigned int index;

   BlockWrite(Xstart,Xstart+(FONT_W-1),Ystart,Ystart+(FONT_H-1));

   index = ord;

   if(index>95)	   //95:ASCII CHAR NUM
   		index=95;

   index = index*((FONT_W/8)*FONT_H);	 

   p = (uint8_t *)ascii;
   p = p+index;

   for(i=0;i<(FONT_W/8*FONT_H);i++)
    {
       dat=*p++;
       for(j=0;j<8;j++)
        {
           if((dat<<j)&0x80)
             {
                WriteOneDot(TextColor);
             }      
           else 
             {
                WriteOneDot(BackColor);	  
             }
         }
     }
}

void DispStr(unsigned char *str,unsigned int Xstart,unsigned int Ystart,unsigned int TextColor,unsigned int BackColor)
{

	while(!(*str=='\0'))
	{
		DispOneChar(ToOrd(*str++),Xstart,Ystart,TextColor,BackColor);

		if(Xstart>((LCD_WIDTH-1)-FONT_W))
		{
			Xstart=0;
		    Ystart=Ystart+FONT_H;
		}
		else
		{
			Xstart=Xstart+FONT_W;
		}

		if(Ystart>((LCD_HEIGHT-1)-FONT_H))
		{
			Ystart=0;
		}
	}	
	
	BlockWrite(0,LCD_WIDTH-1,0,LCD_HEIGHT-1);
}

void DispInt(unsigned int i,unsigned int Xstart,unsigned int Ystart,unsigned int TextColor,unsigned int BackColor)
{
	if(Xstart>((LCD_WIDTH-1)-FONT_W*4))
	{
		Xstart=(LCD_WIDTH-1)-FONT_W*4;
	}
	if(Ystart>((LCD_HEIGHT-1)-FONT_H))
	{
		Ystart=(Ystart-1)-FONT_H;
	}
			
	DispOneChar((i>>12)%16,Xstart,Ystart,TextColor,BackColor); //ID value
	DispOneChar((i>>8)%16,Xstart+FONT_W,Ystart,TextColor,BackColor);
	DispOneChar((i>>4)%16,Xstart+FONT_W*2,Ystart,TextColor,BackColor);
	DispOneChar(i%16,Xstart+FONT_W*3,Ystart,TextColor,BackColor); 

	BlockWrite(0,LCD_WIDTH-1,0,LCD_HEIGHT-1);
}



void DispRegValue_TEST(u8 RegIndex,u16 ParNum)
{
//	u8  i,a,b,c,d;
//	u16  ID=0;
//	unsigned int reg_data[20];	

//		c=Short_Read(0xdb);
//		d=Short_Read(0xdc);

//		MCU_DIR_Write;
//		ID=c<<8|d;
//		DispStr("READ ID REG04= ",0,0,BLACK,WHITE);

//	//DispStr("READ REG:0X",0,0,BLACK,WHITE);
//	//DispInt(i,FONT_W*11,0,BLACK,WHITE);
//    for(i=0;i<ParNum;i++)
//	{
//		DispStr("0X",0,(FONT_H+1)*(i+1),BLUE,WHITE);
//		DispInt(ID,FONT_W*2,(FONT_H+1)*(i+1),BLUE,WHITE);
//	}

}

#endif

void LCD_GPIO_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE); //浣胯兘PORTB,C鏃堕挓鍜孉FIO鏃堕挓
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);//寮�鍚疭WD锛屽け鑳絁TAG
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_9|GPIO_Pin_8|GPIO_Pin_7|GPIO_Pin_6;	   ///PORTC6~10澶嶇敤鎺ㄦ尳杈撳嚭
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure); //GPIOC	

	GPIO_SetBits(GPIOC,GPIO_Pin_10|GPIO_Pin_9|GPIO_Pin_8|GPIO_Pin_7|GPIO_Pin_6);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_14|GPIO_Pin_13|GPIO_Pin_12|GPIO_Pin_11|GPIO_Pin_10|GPIO_Pin_9|GPIO_Pin_8;	//  PORTB鎺ㄦ尳杈撳嚭
	GPIO_Init(GPIOB, &GPIO_InitStructure); //GPIOB
	
	GPIO_SetBits(GPIOB,GPIO_Pin_15|GPIO_Pin_14|GPIO_Pin_13|GPIO_Pin_12|GPIO_Pin_11|GPIO_Pin_10|GPIO_Pin_9|GPIO_Pin_8);
}


#ifdef CHAR_FONT_W8_H16
//ascii 32~90(!~Z), (32~47)空格~/,(48~57)0~9,(58~64):~@,(65~126)A~~
//ord 0~95, (48~57)0~9,(65~126)A~z,(33~47)!~/,(58~64):~@
unsigned char const ascii[]=
{
//宋体12,点阵为：宽x高=8x16 
//0(ord:0)
0x00,0x00,0x00,0x18,0x24,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x24,0x18,0x00,0x00,

//(ord:1)~1
0x00,0x00,0x00,0x10,0x70,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x7C,0x00,0x00,

//(ord:2)~2
0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x04,0x04,0x08,0x10,0x20,0x42,0x7E,0x00,0x00,

//(ord:3)~3
0x00,0x00,0x00,0x3C,0x42,0x42,0x04,0x18,0x04,0x02,0x02,0x42,0x44,0x38,0x00,0x00,

//(ord:4)~4
0x00,0x00,0x00,0x04,0x0C,0x14,0x24,0x24,0x44,0x44,0x7E,0x04,0x04,0x1E,0x00,0x00,

//(ord:5)~5
0x00,0x00,0x00,0x7E,0x40,0x40,0x40,0x58,0x64,0x02,0x02,0x42,0x44,0x38,0x00,0x00,

//(ord:0)~6
0x00,0x00,0x00,0x1C,0x24,0x40,0x40,0x58,0x64,0x42,0x42,0x42,0x24,0x18,0x00,0x00,

//(ord:0)~7
0x00,0x00,0x00,0x7E,0x44,0x44,0x08,0x08,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,

//(ord:0)~8
0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x24,0x18,0x24,0x42,0x42,0x42,0x3C,0x00,0x00,

//(ord:0)~9
0x00,0x00,0x00,0x18,0x24,0x42,0x42,0x42,0x26,0x1A,0x02,0x02,0x24,0x38,0x00,0x00,

// A~Z,宋体12;  此字体下对应的点阵为：宽x高=8x16 
//(ord:10)~A 
0x00,0x00,0x00,0x10,0x10,0x18,0x28,0x28,0x24,0x3C,0x44,0x42,0x42,0xE7,0x00,0x00,

//(ord:11)~B  
0x00,0x00,0x00,0xF8,0x44,0x44,0x44,0x78,0x44,0x42,0x42,0x42,0x44,0xF8,0x00,0x00,

//(ord:12)~C  
0x00,0x00,0x00,0x3E,0x42,0x42,0x80,0x80,0x80,0x80,0x80,0x42,0x44,0x38,0x00,0x00,

//(ord:13)~D 
0x00,0x00,0x00,0xF8,0x44,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x44,0xF8,0x00,0x00,

//(ord:14)~E  
0x00,0x00,0x00,0xFC,0x42,0x48,0x48,0x78,0x48,0x48,0x40,0x42,0x42,0xFC,0x00,0x00,

//(ord:15)~F 
0x00,0x00,0x00,0xFC,0x42,0x48,0x48,0x78,0x48,0x48,0x40,0x40,0x40,0xE0,0x00,0x00,

//(ord:16)~G  
0x00,0x00,0x00,0x3C,0x44,0x44,0x80,0x80,0x80,0x8E,0x84,0x44,0x44,0x38,0x00,0x00,

//(ord:17)~H 
0x00,0x00,0x00,0xE7,0x42,0x42,0x42,0x42,0x7E,0x42,0x42,0x42,0x42,0xE7,0x00,0x00,

//(ord:18)~I 
0x00,0x00,0x00,0x7C,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x7C,0x00,0x00,

//(ord:19)~J  
0x00,0x00,0x00,0x3E,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x88,0xF0,

//(ord:20)~K 
0x00,0x00,0x00,0xEE,0x44,0x48,0x50,0x70,0x50,0x48,0x48,0x44,0x44,0xEE,0x00,0x00,

//(ord:21)~L  
0x00,0x00,0x00,0xE0,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x42,0xFE,0x00,0x00,

//(ord:22)~M  
0x00,0x00,0x00,0xEE,0x6C,0x6C,0x6C,0x6C,0x54,0x54,0x54,0x54,0x54,0xD6,0x00,0x00,

//(ord:23)~N 
0x00,0x00,0x00,0xC7,0x62,0x62,0x52,0x52,0x4A,0x4A,0x4A,0x46,0x46,0xE2,0x00,0x00,

//(ord:24)~O  
0x00,0x00,0x00,0x38,0x44,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x44,0x38,0x00,0x00,

//(ord:25)~P  
0x00,0x00,0x00,0xFC,0x42,0x42,0x42,0x42,0x7C,0x40,0x40,0x40,0x40,0xE0,0x00,0x00,

//(ord:26)~Q 
0x00,0x00,0x00,0x38,0x44,0x82,0x82,0x82,0x82,0x82,0xB2,0xCA,0x4C,0x38,0x06,0x00,

//(ord:27)~R  
0x00,0x00,0x00,0xFC,0x42,0x42,0x42,0x7C,0x48,0x48,0x44,0x44,0x42,0xE3,0x00,0x00,

//(ord:28)~S  
0x00,0x00,0x00,0x3E,0x42,0x42,0x40,0x20,0x18,0x04,0x02,0x42,0x42,0x7C,0x00,0x00,

//(ord:29)~T  
0x00,0x00,0x00,0xFE,0x92,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x38,0x00,0x00,

//(ord:30)~U 
0x00,0x00,0x00,0xE7,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,

//(ord:31)~V  
0x00,0x00,0x00,0xE7,0x42,0x42,0x44,0x24,0x24,0x28,0x28,0x18,0x10,0x10,0x00,0x00,

//(ord:32)~W  
0x00,0x00,0x00,0xD6,0x92,0x92,0x92,0x92,0xAA,0xAA,0x6C,0x44,0x44,0x44,0x00,0x00,

//(ord:33)~X 
0x00,0x00,0x00,0xE7,0x42,0x24,0x24,0x18,0x18,0x18,0x24,0x24,0x42,0xE7,0x00,0x00,

//(ord:34)~Y  
0x00,0x00,0x00,0xEE,0x44,0x44,0x28,0x28,0x10,0x10,0x10,0x10,0x10,0x38,0x00,0x00,

//(ord:35)~Z 
0x00,0x00,0x00,0x7E,0x84,0x04,0x08,0x08,0x10,0x20,0x20,0x42,0x42,0xFC,0x00,0x00,

//(ord:36)~ [  
0x00,0x1E,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x1E,0x00,

//(ord:37)~ '\'
0x00,0x00,0x40,0x40,0x20,0x20,0x10,0x10,0x10,0x08,0x08,0x04,0x04,0x04,0x02,0x02,

//(ord:38)~ ] 
0x00,0x78,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x78,0x00,

//(ord:39)~ ^ 
0x00,0x1C,0x22,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

//(ord:40)~ _ 
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,

//(ord:41)~ `
0x00,0x60,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

//(ord:42)~ a  
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x1E,0x22,0x42,0x42,0x3F,0x00,0x00,

//(ord:43)~ b 
0x00,0x00,0x00,0xC0,0x40,0x40,0x40,0x58,0x64,0x42,0x42,0x42,0x64,0x58,0x00,0x00,

//(ord:44)~ c 
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x22,0x40,0x40,0x40,0x22,0x1C,0x00,0x00,

//(ord:45)~ d  
0x00,0x00,0x00,0x06,0x02,0x02,0x02,0x1E,0x22,0x42,0x42,0x42,0x26,0x1B,0x00,0x00,

//(ord:46)~ e 
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x7E,0x40,0x40,0x42,0x3C,0x00,0x00,

//(ord:47)~ f 
0x00,0x00,0x00,0x0F,0x11,0x10,0x10,0x7E,0x10,0x10,0x10,0x10,0x10,0x7C,0x00,0x00,

//(ord:48)~ g
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3E,0x44,0x44,0x38,0x40,0x3C,0x42,0x42,0x3C,

//(ord:49)~ h 
0x00,0x00,0x00,0xC0,0x40,0x40,0x40,0x5C,0x62,0x42,0x42,0x42,0x42,0xE7,0x00,0x00,

//(ord:50)~ i  
0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x70,0x10,0x10,0x10,0x10,0x10,0x7C,0x00,0x00,

//(ord:51)~ j  
0x00,0x00,0x00,0x0C,0x0C,0x00,0x00,0x1C,0x04,0x04,0x04,0x04,0x04,0x04,0x44,0x78,

//(ord:52)~ k 
0x00,0x00,0x00,0xC0,0x40,0x40,0x40,0x4E,0x48,0x50,0x68,0x48,0x44,0xEE,0x00,0x00,

//(ord:53)~ l  
0x00,0x00,0x00,0x70,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x7C,0x00,0x00,

//(ord:54)~ m  
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x49,0x49,0x49,0x49,0x49,0xED,0x00,0x00,

//(ord:55)~ n  
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xDC,0x62,0x42,0x42,0x42,0x42,0xE7,0x00,0x00,

//(ord:56)~ o  
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0x00,0x00,

//(ord:57)~ p 
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xD8,0x64,0x42,0x42,0x42,0x44,0x78,0x40,0xE0,

//(ord:58)~ q  
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1E,0x22,0x42,0x42,0x42,0x22,0x1E,0x02,0x07,

//(ord:59)~ r 
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xEE,0x32,0x20,0x20,0x20,0x20,0xF8,0x00,0x00,

//(ord:60)~ s  
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3E,0x42,0x40,0x3C,0x02,0x42,0x7C,0x00,0x00,

//(ord:61)~ t 
0x00,0x00,0x00,0x00,0x00,0x10,0x10,0x7C,0x10,0x10,0x10,0x10,0x10,0x0C,0x00,0x00,

//(ord:62)~ u 
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC6,0x42,0x42,0x42,0x42,0x46,0x3B,0x00,0x00,

//(ord:63)~ v 
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE7,0x42,0x24,0x24,0x28,0x10,0x10,0x00,0x00,

//(ord:64)~ w 
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xD7,0x92,0x92,0xAA,0xAA,0x44,0x44,0x00,0x00,

//(ord:65)~ x 
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6E,0x24,0x18,0x18,0x18,0x24,0x76,0x00,0x00,

//(ord:66)~ y 
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE7,0x42,0x24,0x24,0x28,0x18,0x10,0x10,0xE0,

//(ord:67)~ z 
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x44,0x08,0x10,0x10,0x22,0x7E,0x00,0x00,

//(ord:68)~ { 
0x00,0x03,0x04,0x04,0x04,0x04,0x04,0x08,0x04,0x04,0x04,0x04,0x04,0x04,0x03,0x00,

//(ord:69)~ |  
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,

//(ord:70)~ } 
0x00,0x60,0x10,0x10,0x10,0x10,0x10,0x08,0x10,0x10,0x10,0x10,0x10,0x10,0x60,0x00,

//(ord:71)~ ~  
0x30,0x4C,0x43,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,


//(ord:72)~ 空格 
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

//(ord:73)~ !  
0x00,0x00,0x00,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,0x18,0x18,0x00,0x00,

//(ord:74)~ " 
0x00,0x12,0x36,0x24,0x48,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

//(ord:75)~ #  
0x00,0x00,0x00,0x24,0x24,0x24,0xFE,0x48,0x48,0x48,0xFE,0x48,0x48,0x48,0x00,0x00,

//(ord:76)~ $  
0x00,0x00,0x10,0x38,0x54,0x54,0x50,0x30,0x18,0x14,0x14,0x54,0x54,0x38,0x10,0x10,

//(ord:77)~ % 
0x00,0x00,0x00,0x44,0xA4,0xA8,0xA8,0xA8,0x54,0x1A,0x2A,0x2A,0x2A,0x44,0x00,0x00,

//(ord:78)~ & 
0x00,0x00,0x00,0x30,0x48,0x48,0x48,0x50,0x6E,0xA4,0x94,0x88,0x89,0x76,0x00,0x00,

//(ord:79)~ '  
0x00,0x60,0x60,0x20,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

//(ord:80)~ ( 
0x00,0x02,0x04,0x08,0x08,0x10,0x10,0x10,0x10,0x10,0x10,0x08,0x08,0x04,0x02,0x00,

//(ord:81)~ ) 
0x00,0x40,0x20,0x10,0x10,0x08,0x08,0x08,0x08,0x08,0x08,0x10,0x10,0x20,0x40,0x00,

//(ord:82)~ *  
0x00,0x00,0x00,0x00,0x10,0x10,0xD6,0x38,0x38,0xD6,0x10,0x10,0x00,0x00,0x00,0x00,

//(ord:83)~ +  
0x00,0x00,0x00,0x00,0x10,0x10,0x10,0x10,0xFE,0x10,0x10,0x10,0x10,0x00,0x00,0x00,

//(ord:84)~ , 
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x60,0x20,0xC0,

//(ord:85)~ - 
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

//(ord:86)~ . 
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x60,0x00,0x00,

//(ord:87)~ /  
0x00,0x00,0x01,0x02,0x02,0x04,0x04,0x08,0x08,0x10,0x10,0x20,0x20,0x40,0x40,0x00,

//(ord:88)~ : 
0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,

//(ord:89)~ ;  
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x10,0x10,0x20,

//(ord:90)~ < 
0x00,0x00,0x00,0x02,0x04,0x08,0x10,0x20,0x40,0x20,0x10,0x08,0x04,0x02,0x00,0x00,

//(ord:91)~ = 
0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0x00,0x00,

//(ord:92)~ >  
0x00,0x00,0x00,0x40,0x20,0x10,0x08,0x04,0x02,0x04,0x08,0x10,0x20,0x40,0x00,0x00,

//(ord:93)~ ?
0x00,0x00,0x00,0x3C,0x42,0x42,0x62,0x02,0x04,0x08,0x08,0x00,0x18,0x18,0x00,0x00,

//(ord:94)~ @ 
0x00,0x00,0x00,0x38,0x44,0x5A,0xAA,0xAA,0xAA,0xAA,0xB4,0x42,0x44,0x38,0x00,0x00,

//(ord:95)~ 0xff 
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,

};
#endif


