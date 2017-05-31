/*****************************************************************************
* File    : GC_wifi.ino
* Author  : 惜枫醉
* Date    : 2017/05/14
* Brief   : wifi driver 8266
*
* Copyright (c) 2000-2020 惜枫醉 All Rights Reserved.
* Remarks :要把模块速度设置为9600 Arduino在119200时，收数据不完整。
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
#define WIFI_RST  2
#define WIFI_CH_PD  3
#define CMD_RAM 1
#define CMD_FLASH 2
#define ACK_RAM 1
#define ACK_FLASH 2
#define TIME_SERIAL_QUERY_GAP 40
/*****************************************************************************
*                              数据类型定义
******************************************************************************/

struct WIFI_RecvFrame
{
    Uint8 isRecvData;//0 not recv 1 recv
    Uint8 id;
    Uint16 recvLen;
    char *recvBuf;
   
};

/*****************************************************************************
*                                函数声明
******************************************************************************/


/*****************************************************************************
*                                变量定义
******************************************************************************/
#define MAX_RBUF 600
#define MAX_CMD 100
#define MAX_ACK 25
unsigned char gBuf[MAX_RBUF];
unsigned char gCmd[MAX_CMD];
char ack1Buf[MAX_ACK];
char ack2Buf[MAX_ACK];
static volatile Int16 wifiIsrChrCnt = 0;
static volatile Int8  wifiIsrAtReadBegin = 0;
static volatile Int16  wifiIsrRecvCnt = 0;
static volatile Int8  wifiIsrRecvBegin = 0;
static volatile Int8  wifiIsrRecvIsMux = 0;
static volatile Uint8 wifi_debug_flg = 0;

struct WIFI_RecvFrame gRecvInfo = 
{
    .isRecvData = 0,
    .id = 0,
    .recvLen = 0,
    .recvBuf = 0
};


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 打开wifi debug后，wifi的输出会打印打串口上
* param status
* Return  : 
* Remarks : 
******************************************************************************/
Int8 WIFI_debug(Uint8 status)
{
    wifi_debug_flg = status;
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 串口等待有数据到来，目前这个用的不是很多
* param timeOut
* Return  : 
* Remarks : 
******************************************************************************/
int WIFI_serialWait(unsigned int timeOut)
{
    while (Serial.available() == 0)
   {
        timeOut--;
        delay(1);
        if(timeOut == 0)
        {
            break;
        }
   }
   if(timeOut  == 0)
   {
        return -1;
   }
   else
   {
        delay(16);
        return 0;
   }

}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 等待数据到来后，读掉数据
* Return  : 
* Remarks : 
******************************************************************************/
Uint16 WIFI_serialRead(void)
{
  Uint16 ii = 0;
  int jj = 16;
  memset(gBuf,0,MAX_RBUF);
  
  while (jj)
  {
    if(Serial.available() > 0)
    {
        gBuf[ii]= Serial.read();
        ii++;
        if(ii >= MAX_RBUF)
        {
          break;
        }
        continue;
    }
    
    jj--;
    delay(1);
  }

  return ii;
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 串口的定时器中断服务程序，因为arduino串口中断没法调用，所以委屈求全，用timer进行每隔40s一次数据查询
* Return  : 
* Remarks : 
******************************************************************************/
void WIFI_SerialISR(void)
{
    char *ptr = NULL;
    char *ptrf = NULL;
    if(wifiIsrAtReadBegin)
    {
        if(wifiIsrChrCnt >= MAX_RBUF)
        {  
            return;
        }
        while(Serial.available() >0)
        {
            gBuf[wifiIsrChrCnt] = Serial.read();
            wifiIsrChrCnt ++;
            
            if(wifiIsrChrCnt >= MAX_RBUF)
            {  
                return;
            }
        }
    }
    else if(wifiIsrRecvBegin)
    {
        if(wifiIsrRecvCnt >= MAX_RBUF)
        {  
            return;
        }
        while(Serial.available() >0)
        {
            gBuf[wifiIsrRecvCnt] = Serial.read();
            wifiIsrRecvCnt ++;
            
            if(wifiIsrRecvCnt >= MAX_RBUF)
            {  
                return;
            }
        }
        if(!gRecvInfo.isRecvData)
        {
            ptr = strstr(gBuf,"+IPD");
            ptrf = strstr(gBuf,":");
            
            if(ptr !=NULL && ptrf!= NULL)
            {
                ptr = strtok(gBuf,",");//recv head drop

                if(wifiIsrRecvIsMux)
                {
                    ptr = strtok(NULL,",");
                    if(ptr == NULL)
                    {
                        dbgMsg("ISR data failed1");
                        return;
                    }
                    gRecvInfo.id =  WIFI_stnou_my(ptr, '\0');//id
                }
                
                ptr = strtok(NULL,":");
                
                if(ptr == NULL)
                {
                    dbgMsg("ISR data failed2");
                    return;
                }
                gRecvInfo.recvLen = WIFI_stnou_my(ptr, '\0');//length
                
                
                gRecvInfo.recvBuf = ptr + strlen(ptr) +1 ;
                
                gRecvInfo.isRecvData = 1;
            }
            else
            {
                gRecvInfo.recvBuf = NULL;
                gRecvInfo.recvLen = 0;
                gRecvInfo.id = 0;
            }
            
        }
        
        
    }

    
    
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 这个基本就是等定时器中断数据读好了，拿过来用
* param waitTime
* Return  : 
* Remarks : 
******************************************************************************/
Uint16 WIFI_isrATRead(Uint16 waitTime)
{
    memset(gBuf,0,MAX_RBUF);
    wifiIsrChrCnt = 0;
    wifiIsrAtReadBegin = 1;
    delay(waitTime);
    wifiIsrAtReadBegin = 0;
    return wifiIsrChrCnt;
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 初始化wifi串口
* Return  : 
* Remarks : 
******************************************************************************/
int WIFI_pinInit(void)
{
    pinMode(WIFI_RST,OUTPUT);
    pinMode(WIFI_CH_PD,OUTPUT);  
    digitalWrite(WIFI_CH_PD,LOW);
    digitalWrite(WIFI_RST,HIGH);
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : wifi重制
* Return  : 
* Remarks : 
******************************************************************************/
void WIFI_reset(void)
{
  digitalWrite(WIFI_RST,LOW);
  delay(500);
  digitalWrite(WIFI_RST,HIGH);
  WIFI_serialRead();
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 使能wifi
* Return  : 
* Remarks : 
******************************************************************************/
void WIFI_enable(void)
{
  digitalWrite(WIFI_CH_PD,HIGH);
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 发送一个AT指令
* param ack1想要的结果1
* param ack2想要的结果2
* param ackType结果的存储类型，是flash还是ram
* param cmd 命令
* param cmdType命令是flash还是ram
* param timeMs等待时间
* Return  : 
* Remarks : 
******************************************************************************/
Int8 WIFI_sendATCmd(char *cmd,char *ack1,char *ack2,Uint16 timeMs,Uint16 cmdType,Uint16 ackType)
{
    Uint16 cnt = 0;
    char *tmpPtr = NULL;
    Uint16 ii=0;


    WIFI_serialRead();
    if(cmdType == CMD_RAM)
    {
	    Serial.println(cmd);
    }
    else
    {
        while(pgm_read_byte(cmd) != NULL)
        {
            Serial.write(pgm_read_byte(cmd));
            cmd++;
        }
        Serial.print("\r\n");
    }
    
	if(ack1==0&&ack2==0)
	{
		return 0;
	}
	
    cnt = WIFI_isrATRead(timeMs);
    
    if(wifi_debug_flg)
    {
        dbgMsg("----------begin-------------");
        for(ii=0;ii<cnt;ii++)
        {
            
            Serial.write(gBuf[ii]);
        }
        dbgMsg("-----------end--------------");
    }
    
    if(ackType == ACK_RAM)
	{
	    if(ack1 != NULL)
	    {
    	    tmpPtr = ack1Buf;
    	    while(ack1 != NULL)
    	    {
                *tmpPtr = *ack1;
                 tmpPtr++;
                ack1 ++;
    	    }
    	    
    	    *tmpPtr = '\0';
        }
        
        if(ack2 != NULL)
        {
    	    tmpPtr = ack2Buf;
    	    while(ack2 != NULL)
    	    {
                *tmpPtr = pgm_read_byte(ack2);
                tmpPtr ++;
                ack2++;
    	    }
    	    
    	    *tmpPtr = '\0';
	    }
	}
	else
	{
		if(ack1 != NULL)
	    {
            tmpPtr = ack1Buf;
            while(pgm_read_byte(ack1) != NULL)
            {
                *tmpPtr = pgm_read_byte(ack1);
                tmpPtr ++;
                ack1++;
            }
            
            *tmpPtr = '\0';
        }
        
        if(ack2 != NULL)
        {
            tmpPtr = ack2Buf;
            while(pgm_read_byte(ack2) != NULL)
            {
                *tmpPtr = pgm_read_byte(ack2);
                tmpPtr ++;
                ack2++;
            }
            
            *tmpPtr = '\0';
        }
	}

	
	if(ack1!=0 && ack2!=0)
	{
        if(strstr (gBuf, ack1Buf ) || strstr (gBuf, ack2Buf ) )
        {
            return 0;
        }
        else
        {
            return -1;
        }
	}
	else if( ack1 != 0 )
	{
		if(strstr(gBuf, ack1Buf ))
		{
            return 0;
		}
		else
		{
            return -1;
		}
    }
	else
	{
		if(strstr (gBuf, ack2Buf ))
		{
            return 0;
		}
		else
		{

            return -1;
		}
	}
	
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : at测试
* Return  : 
* Remarks : 
******************************************************************************/
void WIFI_ATTest(void)
{
    Uint8 cnt = 10;

    while(cnt--)
    {
        if(WIFI_sendATCmd(PSTR("AT"), PSTR("OK"), NULL, 500, CMD_FLASH, ACK_FLASH))
        {
            dbgMsg("WIFI FAILED");
        }
        else
        {
            return 0;
        }
        delay(500);
    }
    
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 模块的网络选择模式
* param mode
* Return  : 
* Remarks : 
******************************************************************************/
Int8 WIFI_netModeSelect(WIFI_Net_Mode mode)
{
    switch ( mode )
    {
        case STA:
            return WIFI_sendATCmd ( PSTR("AT+CWMODE=1"), PSTR("OK"), PSTR("no change"), 2500, CMD_FLASH, ACK_FLASH); 

        case AP:
            return WIFI_sendATCmd ( PSTR("AT+CWMODE=2"), PSTR("OK"), PSTR("no change"), 2500, CMD_FLASH, ACK_FLASH); 

        case STA_AP:
            return WIFI_sendATCmd ( PSTR("AT+CWMODE=3"), PSTR("OK"), PSTR("no change"), 2500, CMD_FLASH, ACK_FLASH); 

        default:
        return false;
    }	

}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : wifi初始化
* Return  : 
* Remarks : 
******************************************************************************/
Int8 WIFI_init(void)
{
    int ret = 0;
    Serial.begin(9600);
    TIMER_register(TIME_SERIAL_QUERY_GAP, WIFI_SerialISR);
    WIFI_reset();
    WIFI_enable();
    delay(1000);
    WIFI_ATTest();
    delay(500);
    ret |= WIFI_netModeSelect(STA);
	
    if(ret)
    {
        dbgMsg("WIFI init failed\n");
    }
    delay(500);
    ret = WIFI_enableMultipleId(1);
    if(ret)
    {
		dbgMsg("enable multId failed\n");
    }
    delay(500);
    return 0;
}



/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 连接到ap
* param password
* param SSID
* Return  : 
* Remarks : 
******************************************************************************/
Int8 WIFI_connectToAP(char *SSID,char *password)
{
    int ret = 0;
    sprintf(gCmd,"AT+CWJAP=\"%s\",\"%s\"",SSID,password);
    ret = WIFI_sendATCmd(gCmd, PSTR("OK"), PSTR("WIFI CONNECTED"), 6000,CMD_RAM,ACK_FLASH);
    
    return ret;
    
}
/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 使能多id传输
* param isEnableMuxId
* Return  : 
* Remarks : 
******************************************************************************/
Int8 WIFI_enableMultipleId (Uint8 isEnableMuxId )
{
	char cStr [20];
	if(isEnableMuxId)
	{
		return WIFI_sendATCmd ( PSTR("AT+CIPMUX=1"), PSTR("OK"), NULL, 500, CMD_FLASH, ACK_FLASH );
	}
	else
	{
		return WIFI_sendATCmd ( PSTR("AT+CIPMUX=0"), PSTR("OK"), NULL, 500, CMD_FLASH, ACK_FLASH );
	}
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 透传开关
* Return  : 
* Remarks : 
******************************************************************************/
Int8 WIFI_unvarnishSend(void)
{
	if(!WIFI_sendATCmd(PSTR("AT+CIPMODE=1"), PSTR("OK"), NULL, 500, CMD_FLASH, ACK_FLASH))
	{
		return -1;
	}

	return WIFI_sendATCmd( PSTR("AT+CIPSEND"), PSTR("OK"), PSTR(">"), 500, CMD_FLASH, ACK_FLASH );
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 退出透传
* Return  : 
* Remarks : 
******************************************************************************/
Int8 WIFI_exitUnvarnishSend(void)
{
		delay(1000);
		Serial.println( "+++" );
		delay( 500 ); 	
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 链接到服务器上
* param id
* param ip
* param linkMode
* param port
* Return  : 
* Remarks : 
******************************************************************************/
Int8 WIFI_LinkServer(WIFI_LinkMode linkMode, Uint8 ip[4],Uint16 port,Uint8 id)
{
	
    
  	if ( id < 5 )
  	{
    	sprintf ( gCmd, "AT+CIPSTART=%d,\"%s\",\"%u.%u.%u.%u\",%d",	id, 
    																(linkMode == TCP)?"TCP":"UDP",
    																ip[0],
    																ip[1],
    																ip[2],
    																ip[3],
    																port);
	}
  	else
  	{
    	sprintf ( gCmd, "AT+CIPSTART=\"%s\",\"%u.%u.%u.%u\",%d",    (linkMode == TCP)?"TCP":"UDP",
																	ip[0],
																	ip[1],
																	ip[2],
																	ip[3],
																	port);
	}
	return WIFI_sendATCmd ( gCmd, PSTR("OK"), PSTR("ALREADY CONNECTED"), 4000,CMD_RAM,ACK_FLASH );
	
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 断开连接
* param id
* Return  : 
* Remarks : 
******************************************************************************/
Int8 WIFI_disconnect(Uint8 id)
{
	sprintf(gCmd,"AT+CIPCLOSE=%u",id);
	return WIFI_sendATCmd ( gCmd, NULL, NULL, 100,CMD_RAM,ACK_FLASH );
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 发送数据
* param buf
* param id
* param isUnvarnishTx
* param sendLen
* Return  : 
* Remarks : 
******************************************************************************/
Int8 WIFI_sendData(Uint8 isUnvarnishTx,Uint8 id , char *buf,Uint16 sendLen)
{
	int ret = 0;
	int ii = 0;
	WIFI_debug(0);
	if ( isUnvarnishTx )
	{
		Serial.println(buf);
	}
	else
	{
		if ( id < 5 )
		{
			sprintf ( gCmd, "AT+CIPSEND=%d,%d", id, sendLen + 2 );
		}	
		else
		{
			sprintf ( gCmd, "AT+CIPSEND=%d", sendLen + 2 );
		}
		
		WIFI_sendATCmd ( gCmd, PSTR("> "), 0, 60,CMD_RAM,ACK_FLASH );
        //WIFI_debug(1);
		ret = WIFI_sendATCmd ( buf, PSTR("SEND OK"), PSTR("Recv"), 200,CMD_RAM,ACK_FLASH );
		//WIFI_debug(0);
	}
	
	return ret;
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 直接给出buf地址，为什么要这样？ 因为单片机内存不够 透传需要注意可能会返回错值
* param buf
* param recvLen
* param timeoutMs
* param isRecvThenOut 当收到一笔有效数据时立即退出
* Return  : 
* Remarks : 
******************************************************************************/
Int8 WIFI_readData(Uint8 isMux,char **buf,Uint8 *id,Uint16 *recvLen,Uint16 timeoutMs)
{
    char *posPtr = NULL;
    wifiIsrRecvCnt = 0;
    wifiIsrRecvIsMux = isMux;
    
    memset(gBuf,0,sizeof(gBuf));
    
    wifiIsrRecvBegin = 1;

    while(timeoutMs --)
    {
        if(gRecvInfo.isRecvData)
        {
            break;
        }
        delay(1);
    }

    if((gRecvInfo.recvLen == 0) || (gRecvInfo.isRecvData == 0))
    {
        return -1;
    }
    
    if(gRecvInfo.recvLen < TIME_SERIAL_QUERY_GAP)
    {
        delay(TIME_SERIAL_QUERY_GAP*3);
    }
    else
    {
        delay((gRecvInfo.recvLen/TIME_SERIAL_QUERY_GAP + 2)*TIME_SERIAL_QUERY_GAP );
    }
    
    
    wifiIsrRecvBegin = 0;
    
    gRecvInfo.isRecvData = 0;

    *buf = gRecvInfo.recvBuf;
    
    if(gRecvInfo.recvBuf == NULL)
    {
        return -1;
    }
    
    *id = gRecvInfo.id;
    *recvLen = gRecvInfo.recvLen;
    return 0;
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 自己的字符串转数函数
* param endmark
* param str
* Return  : 
* Remarks : 
******************************************************************************/
Uint8 WIFI_stnou_my(char *str,char endmark)
{
	int s=0;	
	while(*str==' ')
	{
		str++;
	}

	while(*str>='0'&&*str<='9' && *str != endmark)
	{
		s=s*10+*str-'0';
		str++;
	}
	
	return s;
}


