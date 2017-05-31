/*****************************************************************************
* File    : GC_oled.ino
* Author  : 惜枫醉
* Date    : 2017/04/06
* Brief   : oled 驱动层级，因为OLED操作很慢，我考虑将128x64个点映射到128x64/8个bit中
*
* Copyright (c) 2000-2020 惜枫醉 All Rights Reserved.
* Remarks :修改日志
******************************************************************************/


/*****************************************************************************
*                               头文件引用
******************************************************************************/


/*****************************************************************************
*                                常量定义
******************************************************************************/


/*****************************************************************************
*                                 宏定义
******************************************************************************/
    
#define HORIZEN_PIX     128
#define VERTICAL_PIX    64

#if HW_IIC 

#else
#define OLED_IIC_sw_clockHigh()    digitalWrite(19,HIGH)
#define OLED_IIC_sw_clockLow()     digitalWrite(19,LOW)
#define OLED_IIC_sw_dataHigh()     digitalWrite(18,HIGH)
#define OLED_IIC_sw_dataLow()      digitalWrite(18,LOW)
#define OLED_IIC_sw_dataInput()    pinMode(18,INPUT)
#define OLED_IIC_sw_dataOutput()   pinMode(18,OUTPUT)
#define OLED_IIC_sw_clockOutput()  pinMode(19, OUTPUT);//CLOCK


#define OLED_IIC_sw_start()\
    do\
    {\
        OLED_IIC_sw_clockHigh();\
        OLED_IIC_sw_dataHigh();\
        OLED_IIC_sw_dataLow();\
        OLED_IIC_sw_clockLow();\
    }\
    while(0)

#define OLED_IIC_sw_stop()\
    do\
    {\
        OLED_IIC_sw_clockLow();\
        OLED_IIC_sw_dataLow();\
        OLED_IIC_sw_clockHigh();\
        OLED_IIC_sw_dataHigh();\
    }\
    while(0)
#endif


/*****************************************************************************
*                              数据类型定义
******************************************************************************/


/*****************************************************************************
*                                函数声明
******************************************************************************/


/*****************************************************************************
*                                变量定义
******************************************************************************/

void OLED_init(void)
{
    OLED_IIC_init();
    OLED_clearScreen();
}


void OLED_IIC_init(void)
{
    pinMode(7, OUTPUT);//RES
    pinMode(8, OUTPUT);//CS
    digitalWrite(7,HIGH); //res keep high for iic communication
    digitalWrite(8,LOW); //cs keep low to enable oled

#if HW_IIC
    OLED_IIC_hw_init();
#else
    OLED_IIC_sw_init();
#endif
    delay(200);
    OLED_IIC(0xAE, OLED_CMD);   //display off
    OLED_IIC(0x20, OLED_CMD);	//Set Memory Addressing Mode	
    OLED_IIC(0x10, OLED_CMD);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    OLED_IIC(0xb0, OLED_CMD);	//Set Page Start Address for Page Addressing Mode,0-7
    OLED_IIC(0xc8, OLED_CMD);	//Set COM Output Scan Direction
    OLED_IIC(0x00, OLED_CMD);//---set low column address
    OLED_IIC(0x10, OLED_CMD);//---set high column address
    OLED_IIC(0x40, OLED_CMD);//--set start line address
    OLED_IIC(0x81, OLED_CMD);//--set contrast control register
    OLED_IIC(0xdf, OLED_CMD);
    OLED_IIC(0xa1, OLED_CMD);//--set segment re-map 0 to 127
    OLED_IIC(0xa6, OLED_CMD);//--set normal display
    OLED_IIC(0xa8, OLED_CMD);//--set multiplex ratio(1 to 64)
    OLED_IIC(0x3F, OLED_CMD);//
    OLED_IIC(0xa4, OLED_CMD);//0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    OLED_IIC(0xd3, OLED_CMD);//-set display offset
    OLED_IIC(0x00, OLED_CMD);//-not offset
    OLED_IIC(0xd5, OLED_CMD);//--set display clock divide ratio/oscillator frequency
    OLED_IIC(0xf0, OLED_CMD);//--set divide ratio
    OLED_IIC(0xd9, OLED_CMD);//--set pre-charge period
    OLED_IIC(0x22, OLED_CMD); //
    OLED_IIC(0xda, OLED_CMD);//--set com pins hardware configuration
    OLED_IIC(0x12, OLED_CMD);
    OLED_IIC(0xdb, OLED_CMD);//--set vcomh
    OLED_IIC(0x20, OLED_CMD);//0x20,0.77xVcc
    OLED_IIC(0x8d, OLED_CMD);//--set DC-DC enable
    OLED_IIC(0x14, OLED_CMD);//
    OLED_IIC(0xaf, OLED_CMD);//--turn on oled panel 

}

#if HW_IIC //hardware iic function


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 硬件IIC初始化
* Return  : 
* Remarks : 
******************************************************************************/
void OLED_IIC_hw_init(void)
{
    Wire.begin();
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 硬件iic写一个字节，该函数提供给上层iic的通讯结构软硬都一样
* param dataType
* param iicByte
* Return  : 
* Remarks : 
******************************************************************************/
void OLED_IIC(Uint8 iicByte,Uint8 dataType)
{
    if(dataType == OLED_DATA)
    {
        Wire.beginTransmission(0x78);
        Wire.write(0x40);
        Wire.write((int)(iicByte&0xff));
        Wire.endTransmission();
        
    }else if(dataType == OLED_CMD)
    {
        Wire.beginTransmission(0x78);
        Wire.write(0x00);
        Wire.write((int)(iicByte&0xff));
        Wire.endTransmission();
    }
    delay(10);

}

#else//software iic fucntion


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 软件IIC初始化
* Return  : 
* Remarks : 
******************************************************************************/
void OLED_IIC_sw_init(void)
{
    OLED_IIC_sw_dataOutput();
    OLED_IIC_sw_clockOutput();
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 软件往IIC里写一个字节
* param iicByte
* Return  : 
* Remarks : 
******************************************************************************/
void OLED_IIC_sw_write_byte(Uint8 iicByte)
{
    unsigned char i;
    for(i=0;i<8;i++)
    {
        if(iicByte &0x80)
        {
            OLED_IIC_sw_dataHigh();
        }
        else
        {
            OLED_IIC_sw_dataLow();
        }
        OLED_IIC_sw_clockHigh();
        OLED_IIC_sw_clockLow();
        iicByte <<=1;
    }
    OLED_IIC_sw_dataHigh();
    OLED_IIC_sw_clockHigh();
    OLED_IIC_sw_clockLow();
}



/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 软件iic写一个字节，该函数提供给上层iic的通讯结构软硬都一样
* param dataType
* param iicByte
* Return  : 
* Remarks : 
******************************************************************************/
void OLED_IIC(Uint8 iicByte,Uint8 dataType)
{
    if(dataType == OLED_DATA)
    {
        OLED_IIC_sw_start();
        OLED_IIC_sw_write_byte(0x78);
        OLED_IIC_sw_write_byte(0x40);
        OLED_IIC_sw_write_byte(iicByte);
        OLED_IIC_sw_stop();
        
    }else if(dataType == OLED_CMD)
    {
        OLED_IIC_sw_start();
        OLED_IIC_sw_write_byte(0x78);
        OLED_IIC_sw_write_byte(0x00);
        OLED_IIC_sw_write_byte(iicByte);
        OLED_IIC_sw_stop();

    }
}

#endif


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 清屏
* Return  : 
* Remarks : 
******************************************************************************/
void OLED_clearScreen(void)
{
    Uint8 *ramClr = 0;
    Uint8 ii = 0;
    Uint8 jj = 0;
    for(ii=0;ii<8;ii++)
    {
        OLED_IIC (0xb0+ii, OLED_CMD);
        OLED_IIC (0x00, OLED_CMD);
        OLED_IIC (0x10, OLED_CMD);
        for(jj=0;jj<128;jj++)
        {  
          OLED_IIC(0x00,OLED_DATA);
        }
    }
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 填充全满
* Return  : 
* Remarks : 
******************************************************************************/
void OLED_fillScreen(void)
{
    Uint8 *ramClr = 0;
    Uint8 ii = 0;
    Uint8 jj = 0;
    for(ii=0;ii<8;ii++)
    {
        OLED_IIC (0xb0+ii, OLED_CMD);
        OLED_IIC (0x00, OLED_CMD);
        OLED_IIC (0x10, OLED_CMD);
        for(jj=0;jj<128;jj++)
        {  
          OLED_IIC(0xFF,OLED_DATA);
        }
    }
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 快速设置字符位置
* param x
* param y
* Return  : 
* Remarks : 
******************************************************************************/
void OLED_fastSetDrawCharPos(unsigned char x,unsigned y)
{
    OLED_IIC(0xb0+y, OLED_CMD);
    OLED_IIC(((x&0xf0)>>4)|0x10, OLED_CMD);
    OLED_IIC((x&0x0f), OLED_CMD); 
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 
* param cow
* param isColorReverse
* param row
* param singleChar
* Return  : 
* Remarks : 
******************************************************************************/
void OLED_fastDraw6x8Char(Uint8 row,Uint8 cow,Uint8 singleChar,Uint8 isColorReverse)
{
    Uint8 fGet = 0;
    Uint8 ii = 0;
    //checek if row over screen
    if(row >= (VERTICAL_PIX/8))
    {
        return;
    }
    if(cow >= (HORIZEN_PIX/6))
    {
        return;
    }
    OLED_fastSetDrawCharPos(cow*6,row);
    
    for(ii=0;ii<6;ii++)
    {
        fGet = pgm_read_byte(&F6x8[(singleChar-' ')][ii]);
        OLED_IIC(isColorReverse? ~fGet: fGet,OLED_DATA);
    }
    
}

void OLED_fastDraw8x16Char(Uint8 row, Uint8 cow, Uint8 singleChar, Uint8 isColorReverse)
{
    Uint8 ii = 0;
    Uint8 fGet = 0;
    //checek if row over screen
    if(row >= (VERTICAL_PIX/16))
    {
        return;
    }
    if(cow >= (HORIZEN_PIX/8))
    {
        return;
    }
    OLED_fastSetDrawCharPos(cow*8,row);
    for(ii=0;ii<8;ii++)
    {
        fGet = pgm_read_byte(&F8X16[(singleChar-' ')*16+ii]);
        
        OLED_IIC(isColorReverse? ~fGet : fGet,OLED_DATA);
    }
    OLED_fastSetDrawCharPos(cow*8,row+1);
    for(ii=0;ii<8;ii++)
    {
        fGet = pgm_read_byte(&F8X16[(singleChar-' ')*16+ii+8]);
        OLED_IIC(isColorReverse? ~fGet : fGet,OLED_DATA);
    }
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 这里稍微借用一下C++语法。。。
            只支持flash型字符串
* param isColorReverse
* param isFontX2
* param startCow
* param startRow
* param text
* Return  : 
* Remarks : 
******************************************************************************/
Uint8 OLED_fastDraw_F(Uint8 startCow,Uint8 startRow,Uint8 *text,Uint8 isColorReverse = 0, Uint8 isFontX2 = 0, Uint8 chrCnt=255)
{
    char text_r = 0;
    Uint8 lineCnt = 1;
    Uint8 cow = 0;
    Uint8 row = 0;
    cow = startCow;
    row = startRow;
    if(isFontX2)
    {
        if(row >= (VERTICAL_PIX/16))
        {
            return;
        }
        if(cow >= (HORIZEN_PIX/8))
        {
            return;
        }
        
        while( ((text_r = pgm_read_byte(text)) != '\0') && (chrCnt != 0))
        {
            OLED_fastDraw8x16Char(row,cow,text_r,isColorReverse);
            text++;
            cow ++;
            if(cow >= (HORIZEN_PIX/8))
            {
                cow = 0;
                row ++;
                lineCnt++;
            }
            chrCnt--;
        }
    }
    else
    {
        if(row >= (VERTICAL_PIX/8))
        {
            return;
        }
        if(cow >= (HORIZEN_PIX/6))
        {
            return;
        }

        while(((text_r = pgm_read_byte(text)) != '\0') && (chrCnt != 0))
        {
            OLED_fastDraw6x8Char(row,cow,text_r,isColorReverse);
            text++;
            cow ++;
            if(cow >= (HORIZEN_PIX/6))
            {
                cow = 0;
                row ++;
                lineCnt++;
            }
            chrCnt--;
        }
    }
    return lineCnt;
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 这里稍微借用一下C++语法。。。
            只支持普通型字符串
* param isColorReverse
* param isFontX2
* param startCow
* param startRow
* param text
* Return  : 
* Remarks : 
******************************************************************************/
Uint8 OLED_fastDraw(Uint8 startCow,Uint8 startRow,Uint8 *text,Uint8 isColorReverse = 0, Uint8 isFontX2 = 0, Uint8 chrCnt=255)
{
    Uint8 lineCnt = 1;
    Uint8 cow = 0;
    Uint8 row = 0;
    cow = startCow;
    row = startRow;
    if(isFontX2)
    {
        if(row >= (VERTICAL_PIX/16))
        {
            return;
        }
        if(cow >= (HORIZEN_PIX/8))
        {
            return;
        }
        
        while( ((*text) != '\0') && (chrCnt != 0))
        {
            OLED_fastDraw8x16Char(row,cow,*text,isColorReverse);
            text++;
            cow ++;
            if(cow >= (HORIZEN_PIX/8))
            {
                cow = 0;
                row ++;
                lineCnt++;
            }
            chrCnt --;
        }
    }
    else
    {
        if(row >= (VERTICAL_PIX/8))
        {
            return;
        }
        if(cow >= (HORIZEN_PIX/6))
        {
            return;
        }

        while( ((*text) != '\0') && (chrCnt != 0))
        {
            OLED_fastDraw6x8Char(row,cow,*text,isColorReverse);
            text++;
            cow ++;
            if(cow >= (HORIZEN_PIX/6))
            {
                cow = 0;
                row ++;
                lineCnt++;
            }
            chrCnt --;
        }
    }
    return lineCnt;
}


