#include "lcd.h"
#include "stdlib.h"
#include "font.h" 
#include "usart.h"
#include "delay.h"	 
					 
//画笔颜色,背景颜色
u16 POINT_COLOR = 0x0000,BACK_COLOR = 0xFFFF;  
u16 DeviceCode;	 


#if LCD_FAST_IO==1 //快速IO
//写寄存器函数
void LCD_RESET(void)
{ 
	LCD_RST_SET;//高电平  
 	delay_ms(1);
	LCD_RST_CLR;//低电平
	delay_ms(20);
	LCD_RST_SET;//高电平  
	delay_ms(20);
}
#else//正常IO
//写寄存器函数
void LCD_RESET(void)
{ 
	LCD_RST=1;  
 	delay_ms(1);
	LCD_RST=0;//低电平
	delay_ms(20);
	LCD_RST=1;//高电平  
	delay_ms(20);
} 	
#endif

#if LCD_FAST_IO==1 //快速IO
//写寄存器函数
void LCD_WR_REG(u8 data)
{ 
	LCD_RS_CLR;//写地址  
 	LCD_CS_CLR; 
	DATAOUT(data<<8); 
	LCD_WR_CLR; 
	LCD_WR_SET; 
 	LCD_CS_SET;   
}
#else//正常IO
//写寄存器函数
void LCD_WR_REG(u8 data)
{ 
	LCD_RS=0;//写地址  
 	LCD_CS=0; 
	DATAOUT(data<<8); 
	LCD_WR=0; 
	LCD_WR=1; 
 	LCD_CS=1;   
} 	
#endif	

//写寄存器
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue)
{	
	LCD_WR_REG(LCD_Reg);  
	LCD_WR_DATA(LCD_RegValue);	    		 
}	   
//读寄存器
u16 LCD_ReadReg(u8 LCD_Reg)
{										   
	u16 t,tl;
	LCD_WR_REG(LCD_Reg);   //写入要读的寄存器号  
	GPIOC->CRL=0X88888888; //PC0-7  上拉输入
	GPIOC->CRH=0X88888888; //PC8-15 上拉输入
	GPIOC->ODR=0XFFFF;     //全部输出高
	LCD_RS=1;
	LCD_CS=0;
				   
	LCD_RD=0;					   
	LCD_RD=1;   // dummy read

	LCD_RD=0;					   
	LCD_RD=1;   // 读到0x00

	LCD_RD=0;					   
	LCD_RD=1;   // 读到0x00

	t=DATAIN;
	LCD_RD=0;					   
	LCD_RD=1;   //读到 0x93	

	tl=DATAIN;
	LCD_RD=0;					   
	LCD_RD=1;   //读到 0x41	

	tl = tl >> 8;
	tl = tl & 0x00ff;
	t  = t  & 0xff00;

	t |= tl;     // 合并数据
	  
	LCD_CS=1;   
	GPIOC->CRL=0X33333333; //PC0-7  上拉输出
	GPIOC->CRH=0X33333333; //PC8-15 上拉输出
	GPIOC->ODR=0XFFFF;     //全部输出高
	return t; 
}
//开始写GRAM
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(0x2c);
}	 
//LCD写GRAM
void LCD_WriteRAM(u16 RGB_Code)
{							    
	LCD_WR_DATA(RGB_Code);//写十六位GRAM
}
//从ILI93xx读出的数据为GBR格式，而我们写入的时候为RGB格式。
//通过该函数转换
//c:GBR格式的颜色值
//返回值：RGB格式的颜色值
u16 LCD_BGR2RGB(u16 c)
{
	u16  r,g,b,rgb;   
	b=(c>>0)&0x1f;
	g=(c>>5)&0x3f;
	r=(c>>11)&0x1f;	 
	rgb=(b<<11)+(g<<5)+(r<<0);		 
	return(rgb);
}		 

//LCD开启显示
void LCD_DisplayOn(void)
{					   
	LCD_WR_REG(0x29);
}	 
//LCD关闭显示
void LCD_DisplayOff(void)
{	   
    LCD_WR_REG(0x28);
}  
//设置光标位置
//Xpos:横坐标
//Ypos:纵坐标
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
//画点
//x:0~239
//y:0~319
//POINT_COLOR:此点的颜色
void LCD_DrawPoint(u16 x,u16 y)
{
	LCD_SetCursor(x,y);//设置光标位置 
	LCD_WriteRAM_Prepare();     //开始写入GRAM	
	LCD_WR_TDATA(POINT_COLOR); 
} 


void LCD_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE); //使能PORTB,C时钟和AFIO时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	   ///PORTC6~10复用推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure); //RS CS WR RD

	GPIO_SetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);

	// GPIO_Pin_7|GPIO_Pin_6|GPIO_Pin_5|GPIO_Pin_4|GPIO_Pin_3|GPIO_Pin_2|GPIO_Pin_1|GPIO_Pin_0
	// GPIO_Pin_15|GPIO_Pin_14|GPIO_Pin_13|GPIO_Pin_12|GPIO_Pin_11|GPIO_Pin_10|GPIO_Pin_9|GPIO_Pin_8
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_6|GPIO_Pin_5|GPIO_Pin_4|GPIO_Pin_3|GPIO_Pin_2|GPIO_Pin_1|GPIO_Pin_0;	//  PORTB推挽输出
	GPIO_Init(GPIOC, &GPIO_InitStructure); //D0 ~ D7
	
	GPIO_SetBits(GPIOC,GPIO_Pin_7|GPIO_Pin_6|GPIO_Pin_5|GPIO_Pin_4|GPIO_Pin_3|GPIO_Pin_2|GPIO_Pin_1|GPIO_Pin_0);
}


#define Delay delay_ms	 
//初始化lcd
//该初始化函数可以初始化各种ILI93XX液晶,但是其他函数是基于ILI9320的!!!
//在其他型号的驱动芯片上没有测试! 
void LCD_Init(void)
{ 
   	LCD_GPIO_Init();
	LCD_RD = 1; //不用读 RD拉高
	LCD_RESET();//复位
	
//---------------------------------------------------------------------------------------------------// 
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
}  	  
  
//清屏函数
//Color:要清屏的填充色
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
//在指定区域内填充指定颜色
//区域大小:
//  (xend-xsta)*(yend-ysta)
void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color)
{          
	u16 i,j;
	u16 xlen=0;
#if USE_HORIZONTAL==1
	xlen=yend-ysta+1;	   
	for(i=xsta;i<=xend;i++)
	{
	 	LCD_SetCursor(i,ysta);      //设置光标位置 
		LCD_WriteRAM_Prepare();     //开始写入GRAM	  
		for(j=0;j<xlen;j++)LCD_WR_DATA(color);//设置光标位置 	  
	}
#else
//	xlen=xend-xsta+1;	   
//	for(i=ysta;i<=yend;i++)
//	{
//	 	LCD_SetCursor(xsta,i);      //设置光标位置 
//		LCD_WriteRAM_Prepare();     //开始写入GRAM	  
//		for(j=0;j<xlen;j++)LCD_WR_DATA(color);//设置光标位置 	    
//	}
	
	xlen = xend - xsta + 1;
	for(i = ysta; i<= yend; i++)
	{
		for(j=0; j<xlen; j++)
		{
			LCD_SetCursor(xsta+j, i);//设置光标位置 
			LCD_WriteRAM_Prepare();     //开始写入GRAM	
			LCD_WR_TDATA(color);
		}
	}
#endif						  	    
}
//画线
//x1,y1:起点坐标
//x2,y2:终点坐标  
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		LCD_DrawPoint(uRow,uCol);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}    
//画矩形
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}
//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
void Draw_Circle(u16 x0,u16 y0,u8 r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //判断下个点位置的标志
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a);             //3           
		LCD_DrawPoint(x0+b,y0-a);             //0           
		LCD_DrawPoint(x0-a,y0+b);             //1       
		LCD_DrawPoint(x0-b,y0-a);             //7           
		LCD_DrawPoint(x0-a,y0-b);             //2             
		LCD_DrawPoint(x0+b,y0+a);             //4               
		LCD_DrawPoint(x0+a,y0-b);             //5
		LCD_DrawPoint(x0+a,y0+b);             //6 
		LCD_DrawPoint(x0-b,y0+a);             
		a++;
		//使用Bresenham算法画圆     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 
		LCD_DrawPoint(x0+a,y0+b);
	}
} 
//在指定位置显示一个字符
//x:0~234
//y:0~308
//num:要显示的字符:" "--->"~"
//size:字体大小 12/16
//mode:叠加方式(1)还是非叠加方式(0)
//在指定位置显示一个字符
//x:0~234
//y:0~308
//num:要显示的字符:" "--->"~"
//size:字体大小 12/16
//mode:叠加方式(1)还是非叠加方式(0)
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode)
{  
#if USE_HORIZONTAL==1
#define MAX_CHAR_POSX 312
#define MAX_CHAR_POSY 232 
#else     
#define MAX_CHAR_POSX 232
#define MAX_CHAR_POSY 312
#endif 
    u8 temp;
    u8 pos,t;
	u16 x0=x;
	u16 colortemp=POINT_COLOR;      
    if(x>MAX_CHAR_POSX||y>MAX_CHAR_POSY)return;	    
	//设置窗口		   
	num=num-' ';//得到偏移后的值
	if(!mode) //非叠加方式
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)
			{
				temp=asc2_1206[num][pos]; //调用1206字体
				for(t=0;t<size/2;t++)
				{                 
					if(temp&0x01)POINT_COLOR=colortemp;
					else POINT_COLOR=BACK_COLOR;
					LCD_DrawPoint(x,y);	
					temp>>=1; 
					x++;
				}
				x=x0;
				y++;
			}
			else if(size==16)
			{
				temp=asc2_1608[num][pos]; //调用1608字体
				for(t=0;t<size/2;t++)
				{
					if(temp&0x01)POINT_COLOR=colortemp;
					else POINT_COLOR=BACK_COLOR;
					LCD_DrawPoint(x,y);	
					temp>>=1; 
					x++;
				}
				x=x0;
				y++;
			}
			else
			{
				temp=ascii_3216[num][pos]; //调用3216字体
				for(t=0;t<size/2;t++)
				{
					if(temp&0x01)POINT_COLOR=colortemp;
					else POINT_COLOR=BACK_COLOR;
					LCD_DrawPoint(x,y);	
					temp>>=1; 
					x++;
				}
				x=x0;
				y++;
			}
		}
	}else//叠加方式
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)
			{
				temp=asc2_1206[num][pos]; //调用1206字体
			}
			else if(size==16)
			{
				temp=asc2_1608[num][pos]; //调用1608字体
			}
			else
			{
				temp=ascii_3216[num][pos]; //调用2412字体
			}
			for(t=0;t<size/2;t++)
		    {                 
		        if(temp&0x01)LCD_DrawPoint(x+t,y+pos);//画一个点     
		        temp>>=1; 
		    }
		}
	}
	POINT_COLOR=colortemp;	    	   	 	  
}   
//m^n函数
u32 mypow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}			 
//显示2个数字
//x,y :起点坐标	 
//len :数字的位数
//size:字体大小
//color:颜色
//num:数值(0~4294967295);	 
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size)
{         	
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+(size/2)*t,y,' ',size,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,0); 
	}
} 
//显示2个数字
//x,y:起点坐标
//size:字体大小
//mode:模式	0,填充模式;1,叠加模式
//num:数值(0~99);	 
void LCD_Show2Num(u16 x,u16 y,u16 num,u8 len,u8 size,u8 mode)
{         	
	u8 t,temp;						   
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,mode); 
	}
}
//显示字符串
//x,y:起点坐标  
//*p:字符串起始地址
//用16字体
void LCD_ShowString(u16 x,u16 y, u8 size, const u8 *p)
{
    while(*p!='\0')
    {
		if(*p == '\r')
		{
			p++;
			continue;
		}
		if(*p == '\n')
		{
			x=75;
			y+=16;
			p++;
			continue;
		}
        if(x>MAX_CHAR_POSX){x=75;y+=16;}
		if(y>MAX_CHAR_POSY){return;}
        // if(y>MAX_CHAR_POSY){y=x=0;LCD_Clear(WHITE);}
        LCD_ShowChar(x,y,*p,size,0);
        if(size == 12){
			x+=6;
		}
		else{
			x+=8;
		}
		
        p++;
    }
}

//送图片
void ILI9341_PIC(const unsigned char gImage[])	  // 全屏图片显示
{
    u32 i;
	u32  k = 0;
	u16 buff = 0;

	LCD_SetCursor(0,0);
	LCD_WriteRAM_Prepare();     //开始写入GRAM
	for (i = 0; i < 240 * 320; i ++)
	{
	    buff  = 	gImage[k];
        buff +=	    gImage[k+1] << 8;

	    LCD_WR_TDATA(buff);
	    k++;
		k++;
	}
}





void LCD_ShowFontHZ(u16 x, u16 y, u8 *cn, u8 size, u16 fontColor, u16 backColor)
{
	u8 i, j, wordNum;
	u16 color;
	u16 x0=x; 
	u16 y0=y; 
	while (*cn != '\0')
	{
		if(size == 24)
		{
			for (wordNum=0; wordNum<12; wordNum++)
			{	//wordNum扫描字库的字数
				if ((Arial24x27[wordNum].Index[0]==*cn)
					 &&(Arial24x27[wordNum].Index[1]==*(cn+1))&&(Arial24x27[wordNum].Index[2]==*(cn+2)))
				{
					for(i=0; i<81; i++) 
					{	//MSK的位数
						color=Arial24x27[wordNum].Msk[i];
						for(j=0;j<8;j++) 
						{
							if((color&0x80)==0x80)
							{
								POINT_COLOR = fontColor;
								LCD_DrawPoint(x,y);
								// LCD_DrawFRONT_COLOR(x,y,POINT_COLOR);
							} 						
							else
							{
								POINT_COLOR = backColor;
								LCD_DrawPoint(x,y);
								// LCD_DrawFRONT_COLOR(x,y,BACK_COLOR);
							} 
							color<<=1;
							x++;
							if((x-x0)==24)
							{
								x=x0;
								y++;
								if((y-y0)==27)
								{
									y=y0;
								}
							}
						}//for(j=0;j<8;j++)结束
					}	
				}
				
			}
			cn += 3;
			x += 24;
			x0=x;
		}
		else if(size == 16)
		{
			for (wordNum=0; wordNum<18; wordNum++)
			{	//wordNum扫描字库的字数
				if ((Yahei16x21[wordNum].Index[0]==*cn)
					 &&(Yahei16x21[wordNum].Index[1]==*(cn+1))&&(Yahei16x21[wordNum].Index[2]==*(cn+2)))
				{
					for(i=0; i<42; i++) 
					{	//MSK的位数
						color=Yahei16x21[wordNum].Msk[i];
						for(j=0;j<8;j++) 
						{
							if((color&0x80)==0x80)
							{
								POINT_COLOR = fontColor;
								LCD_DrawPoint(x,y);
								// LCD_DrawFRONT_COLOR(x,y,POINT_COLOR);
							} 						
							else
							{
								POINT_COLOR = backColor;
								LCD_DrawPoint(x,y);
								// LCD_DrawFRONT_COLOR(x,y,BACK_COLOR);
							} 
							color<<=1;
							x++;
							if((x-x0)==16)
							{
								x=x0;
								y++;
								if((y-y0)==21)
								{
									y=y0;
								}
							}
						}//for(j=0;j<8;j++)结束
					}	
				}
				
			}
			cn += 3;
			x += 16;
			x0=x;
		}
	}
}



void LCD_ShowFontHZ_Arial(u16 x, u16 y, u8 *cn, u8 size, u16 fontColor, u16 backColor)
{
	u8 i, j, wordNum;
	u16 color;
	u16 x0=x; 
	u16 y0=y; 
	while (*cn != '\0')
	{
		for (wordNum=0; wordNum<15; wordNum++)
		{	//wordNum扫描字库的字数
			if ((newArialCnChar16x16[wordNum].Index[0]==*cn)
				 &&(newArialCnChar16x16[wordNum].Index[1]==*(cn+1))&&(newArialCnChar16x16[wordNum].Index[2]==*(cn+2)))
			{
				for(i=0; i<32; i++) 
				{	//MSK的位数
					color=newArialCnChar16x16[wordNum].Msk[i];
					for(j=0;j<8;j++) 
					{
						if((color&0x80)==0x80)
						{
							POINT_COLOR = fontColor;
							LCD_DrawPoint(x,y);
							// LCD_DrawFRONT_COLOR(x,y,POINT_COLOR);
						} 						
						else
						{
							POINT_COLOR = backColor;
							LCD_DrawPoint(x,y);
							// LCD_DrawFRONT_COLOR(x,y,BACK_COLOR);
						} 
						color<<=1;
						x++;
						if((x-x0)==16)
						{
							x=x0;
							y++;
							if((y-y0)==16)
							{
								y=y0;
							}
						}
					}//for(j=0;j<8;j++)结束
				}	
			}
			
		}
		cn += 3;
		x += 16;
		x0=x;
	}
}




void LCD_ShowFontEN(u16 x, u16 y, u8 *cn, u8 size, u16 fontColor, u16 backColor)
{
	u8 i, j, wordNum;
	u16 color;
	u16 x0=x; 
	u16 y0=y; 
	while (*cn != '\0')
	{
		if(size == 24)
		{
			for (wordNum=0; wordNum<29; wordNum++)
			{	//wordNum扫描字库的字数
				if (EnChar24x24[wordNum].Index[0]==*cn)
				{
					for(i=0; i<72; i++) 
					{	//MSK的位数
						color=EnChar24x24[wordNum].Msk[i];
						for(j=0;j<8;j++) 
						{
							if((color&0x80)==0x80)
							{
								POINT_COLOR = fontColor;
								LCD_DrawPoint(x,y);
								// LCD_DrawFRONT_COLOR(x,y,POINT_COLOR);
							} 						
							else
							{
								POINT_COLOR = backColor;
								LCD_DrawPoint(x,y);
								// LCD_DrawFRONT_COLOR(x,y,BACK_COLOR);
							} 
							color<<=1;
							x++;
							if((x-x0)==24)
							{
								x=x0;
								y++;
								if((y-y0)==24)
								{
									y=y0;
								}
							}
						}//for(j=0;j<8;j++)结束
					}	
				}
				
			}
			cn += 1;
			x += 16;
			x0=x;
		}
	}
}



void LCD_ShowPicture(u16 x, u16 y, u16 wide, u16 high,u8 *pic)
{
	int i,j;
	u16 temp = 0;
	for(i=0; i<high; i++)
	{
		for(j=0; j<wide; j++)
		{
			temp = pic[(i*wide + j)*2 + 1];
			temp = temp << 8;
			temp = temp | pic[(i*wide + j)*2];
			POINT_COLOR = temp;
			LCD_DrawPoint(x+j, y+i);
		}
	}
}




void LCD_ShowChinese(u16 x, u16 y, CN_FONT font, u8 size, u8 *cn, u16 fontColor, u16 backColor)
{
	u8 i, j, wordNum;
	u16 color;
	u16 x0=x; 
	u16 y0=y; 
	if(font == Arial)
	{
		switch(size)
		{
			case 24:
			{
				while (*cn != '\0')
				{
					for (wordNum=0; wordNum<12; wordNum++)
					{	//wordNum扫描字库的字数
						if ((Arial24x27[wordNum].Index[0]==*cn)
							 &&(Arial24x27[wordNum].Index[1]==*(cn+1))&&(Arial24x27[wordNum].Index[2]==*(cn+2)))
						{
							for(i=0; i<81; i++) 
							{	//MSK的位数
								color=Arial24x27[wordNum].Msk[i];
								for(j=0;j<8;j++) 
								{
									if((color&0x80)==0x80)
									{
										POINT_COLOR = fontColor;
										LCD_DrawPoint(x,y);
									}
									else
									{
										POINT_COLOR = backColor;
										LCD_DrawPoint(x,y);
									}
									color<<=1;
									x++;
									if((x-x0)==24)
									{
										x=x0;
										y++;
										if((y-y0)==27)
										{
											y=y0;
										}
									}
								}//for(j=0;j<8;j++)结束
							}
						}
					}
					cn += 3;
					x += 24;
					x0=x;
				}
				break;
			}
			case 16:
			{
				while (*cn != '\0')
				{
					for (wordNum=0; wordNum<49; wordNum++)
					{	//wordNum扫描字库的字数
						if ((Arial16x18[wordNum].Index[0]==*cn)
							 &&(Arial16x18[wordNum].Index[1]==*(cn+1))&&(Arial16x18[wordNum].Index[2]==*(cn+2)))
						{
							for(i=0; i<36; i++) 
							{	//MSK的位数
								color=Arial16x18[wordNum].Msk[i];
								for(j=0;j<8;j++) 
								{
									if((color&0x80)==0x80)
									{
										POINT_COLOR = fontColor;
										LCD_DrawPoint(x,y);
									}
									else
									{
										POINT_COLOR = backColor;
										LCD_DrawPoint(x,y);
									}
									color<<=1;
									x++;
									if((x-x0)==16)
									{
										x=x0;
										y++;
										if((y-y0)==18)
										{
											y=y0;
										}
									}
								}//for(j=0;j<8;j++)结束
							}
						}
					}
					cn += 3;
					x += 16;
					x0=x;
				}
				break;
			}
			default:{ break; }
		}
	}
	else if(font == newArial)
	{
		switch(size)
		{
			case 24:
			{
				while (*cn != '\0')
				{
					for (wordNum=0; wordNum<6; wordNum++)
					{	//wordNum扫描字库的字数
						if ((newArialCnChar24x24[wordNum].Index[0]==*cn)
							 &&(newArialCnChar24x24[wordNum].Index[1]==*(cn+1))&&(newArialCnChar24x24[wordNum].Index[2]==*(cn+2)))
						{
							for(i=0; i<72; i++) 
							{	//MSK的位数
								color=newArialCnChar24x24[wordNum].Msk[i];
								for(j=0;j<8;j++) 
								{
									if((color&0x80)==0x80)
									{
										POINT_COLOR = fontColor;
										LCD_DrawPoint(x,y);
									} 						
									else
									{
										POINT_COLOR = backColor;
										LCD_DrawPoint(x,y);
									} 
									color<<=1;
									x++;
									if((x-x0)==24)
									{
										x=x0;
										y++;
										if((y-y0)==24)
										{
											y=y0;
										}
									}
								}//for(j=0;j<8;j++)结束
							}	
						}
						
					}
					cn += 3;
					x += 24;
					x0=x;
				}
				break;
			}
			case 16:
			{
				while (*cn != '\0')
				{
					for (wordNum=0; wordNum<44; wordNum++)
					{	//wordNum扫描字库的字数
						if ((newArialCnChar16x16[wordNum].Index[0]==*cn)
							 &&(newArialCnChar16x16[wordNum].Index[1]==*(cn+1))&&(newArialCnChar16x16[wordNum].Index[2]==*(cn+2)))
						{
							for(i=0; i<32; i++) 
							{	//MSK的位数
								color=newArialCnChar16x16[wordNum].Msk[i];
								for(j=0;j<8;j++)
								{
									if((color&0x80)==0x80)
									{
										POINT_COLOR = fontColor;
										LCD_DrawPoint(x,y);
									} 						
									else
									{
										POINT_COLOR = backColor;
										LCD_DrawPoint(x,y);
									} 
									color<<=1;
									x++;
									if((x-x0)==16)
									{
										x=x0;
										y++;
										if((y-y0)==16)
										{
											y=y0;
										}
									}
								}//for(j=0;j<8;j++)结束
							}	
						}
						
					}
					cn += 3;
					x += 16;
					x0=x;
				}
				break;
			}
			
			default: { break; }
		}
	}
	else if(font == Yahei)
	{
		switch(size){
			case 16:
			{
				while(*cn != '\0')
				{
					for (wordNum=0; wordNum<18; wordNum++)
					{	//wordNum扫描字库的字数
						if ((Yahei16x21[wordNum].Index[0]==*cn)
							 &&(Yahei16x21[wordNum].Index[1]==*(cn+1))&&(Yahei16x21[wordNum].Index[2]==*(cn+2)))
						{
							for(i=0; i<42; i++) 
							{	//MSK的位数
								color=Yahei16x21[wordNum].Msk[i];
								for(j=0;j<8;j++) 
								{
									if((color&0x80)==0x80)
									{
										POINT_COLOR = fontColor;
										LCD_DrawPoint(x,y);
									} 						
									else
									{
										POINT_COLOR = backColor;
										LCD_DrawPoint(x,y);
									} 
									color<<=1;
									x++;
									if((x-x0)==16)
									{
										x=x0;
										y++;
										if((y-y0)==21)
										{
											y=y0;
										}
									}
								}//for(j=0;j<8;j++)结束
							}	
						}
					}
					cn += 3;
					x += 16;
					x0=x;
				}
			}
		}
	}
}











