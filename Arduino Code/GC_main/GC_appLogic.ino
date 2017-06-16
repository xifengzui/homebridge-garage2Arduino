/*****************************************************************************
* File    : GC_appLogic.ino
* Author  : 惜枫醉
* Date    : 2017/04/09
* Brief   : 车库控制的总逻辑 
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
#define TEXT_ALIGN_LEFT     0
#define TEXT_ALIGN_MIDDLE   1
#define TEXT_ALIGN_RIGHT    2

#define GARAGE_TIME_UP      0
#define GARAGE_TIME_DOWN    1

#define WIFI_MAX_NAME_LEN 16
#define WIFI_MAX_PASSWD_LEN 16


#define STAT_SELECT_CHAR 0
#define STAT_EDIT_CHAR 1
#define STAT_STRLEN_CHANGE 2



#define STAT_HK_CLOSED  0   //已关闭
#define STAT_HK_CLOSING 1   //关闭中
#define STAT_HK_CLOSE   2   //请求关闭

#define STAT_HK_OPENED  3   //已打开
#define STAT_HK_OPENING 4   //打开中
#define STAT_HK_OPEN    5   //请求打开

#define STAT_HK_IDEL 6
#define STAT_HK_BUSY 7

/*****************************************************************************
*                              数据类型定义
******************************************************************************/
struct APP_GarageParms
{
    Uint8 upTime;
    Uint8 *upTimeStr;
    Uint8 downTime;
    Uint8 *downTimeStr;
    volatile Uint8 isEnableHomeKit;
    volatile Uint8 status;
};

struct APP_WifiParms
{
    Uint8 isConnected;
    Uint8 *ssid;
    Uint8 ssidLen;
    Uint8 *password;
    Uint8 passwordLen;
    Uint8 *srvIp;
    Uint8 devId;
};


struct APP_serviceParms
{
    Uint8 *wifiStatus;
};

/*****************************************************************************
*                                函数声明
******************************************************************************/


/*****************************************************************************
*                                变量定义
******************************************************************************/


//主页菜单项  
static MENU_Handle      gMainMenu = NULL; 
static MENU_ItemHandle  gMainMenu_garage = NULL;
static MENU_ItemHandle  gMainMenu_wifi = NULL;

//车库菜单项
static MENU_Handle      gGarageMenu = NULL;
static MENU_ItemHandle  gGarageMenu_goBack = NULL;
static MENU_ItemHandle  gGarageMenu_upTimeMenu = NULL;
static MENU_ItemHandle  gGarageMenu_downTimeMenu = NULL;
static MENU_ItemHandle  gGarageMenu_enableHomekit = NULL;
static MENU_ItemHandle  gGarageMenu_disableHomekit = NULL;

//wifi菜单项
static MENU_Handle      gWifiMenu = NULL;
static MENU_ItemHandle  gWifiMenu_goBack = NULL;
static MENU_ItemHandle  gWifiMenu_ssid = NULL;
static MENU_ItemHandle  gWifiMenu_password = NULL;
static MENU_ItemHandle  gWifiMenu_connect = NULL;
static MENU_ItemHandle  gWifiMenu_srvIp = NULL;
static MENU_ItemHandle  gWifiMenu_devId = NULL;

//索引句柄，整个菜单的渲染逻辑通过索引句柄来进行的
static MENU_Handle *posHandle = NULL;
static MENU_ItemHandle *posItemHandle = NULL;

//服务器IP地址,设备会向这个ip上报车库门的状态 
static Uint8            gSrv_ip[4] = {192,168,1,200};

//车库门上和下的时间设置
static Uint8            gGarageMenu_downTimeDispMem[] =    "DOWN TIME: 15";
static Uint8            gGarageMenu_upTimeDispMem[]   =    "UP   TIME: 15";

//wifi字符串的内存
static Uint8            gWifiMenu_wifiNameMem[WIFI_MAX_NAME_LEN+1] = "";
static Uint8            gWifiMenu_passwordMem[WIFI_MAX_PASSWD_LEN+1] = "";


static volatile Uint8 gTimerDaemonFlag = 0;//由于定时器中断不建议做很多事情，所以用标志位，让她在主函数运行


static volatile Uint16 gClock = 0; //用于整个设备的计数时间戳

struct APP_GarageParms gAPP_garagePrms = 
{
    .upTime = 0,
    .upTimeStr = gGarageMenu_upTimeDispMem,
    .downTime = 0,
    .downTimeStr = gGarageMenu_downTimeDispMem,
    .isEnableHomeKit = 0,
    .status = STAT_HK_IDEL
};

struct APP_WifiParms gApp_wifiPrms = 
{
    .isConnected = 0,
    .ssid = gWifiMenu_wifiNameMem,
    .ssidLen = 4,
    .password = gWifiMenu_passwordMem,
    .passwordLen = 8,
    .srvIp = gSrv_ip,
    .devId = 66
};


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 从eeprom获取掉电的数据
* Return  : 
* Remarks : 
******************************************************************************/
Uint8 APP_dumpPrms(void)
{
    gAPP_garagePrms.upTime =  EEPROM_restore(EEPROM_UPTIME);
    gAPP_garagePrms.downTime =  EEPROM_restore(EEPROM_DOWNTIME);
    gApp_wifiPrms.srvIp[0] = EEPROM_restore(EEPROM_SRVIPSEG0);
    gApp_wifiPrms.srvIp[1] = EEPROM_restore(EEPROM_SRVIPSEG1);
    gApp_wifiPrms.srvIp[2] = EEPROM_restore(EEPROM_SRVIPSEG2);
    gApp_wifiPrms.srvIp[3] = EEPROM_restore(EEPROM_SRVIPSEG3);
    gApp_wifiPrms.devId = EEPROM_restore(EEPROM_DEVID);
    gAPP_garagePrms.isEnableHomeKit = EEPROM_restore(EEPROM_ISENHOMEKIT);
}
/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 将数据刷入eeprom
* Return  : 
* Remarks : 
******************************************************************************/
Uint8 APP_flushPrms(void)
{
    if(gAPP_garagePrms.upTime !=  EEPROM_restore(EEPROM_UPTIME))
    {
        EEPROM_store(EEPROM_UPTIME, gAPP_garagePrms.upTime);
    }
    
    if(gAPP_garagePrms.downTime !=  EEPROM_restore(EEPROM_DOWNTIME))
    {
        EEPROM_store(EEPROM_DOWNTIME, gAPP_garagePrms.downTime);
    }

    if(gApp_wifiPrms.srvIp[0] !=  EEPROM_restore(EEPROM_SRVIPSEG0))
    {
        EEPROM_store(EEPROM_SRVIPSEG0, gApp_wifiPrms.srvIp[0]);
    }

    if(gApp_wifiPrms.srvIp[1] !=  EEPROM_restore(EEPROM_SRVIPSEG1))
    {
        EEPROM_store(EEPROM_SRVIPSEG1, gApp_wifiPrms.srvIp[1]);
    }

    if(gApp_wifiPrms.srvIp[2] !=  EEPROM_restore(EEPROM_SRVIPSEG2))
    {
        EEPROM_store(EEPROM_SRVIPSEG2, gApp_wifiPrms.srvIp[2]);
    }

    if(gApp_wifiPrms.srvIp[3] !=  EEPROM_restore(EEPROM_SRVIPSEG3))
    {
        EEPROM_store(EEPROM_SRVIPSEG3, gApp_wifiPrms.srvIp[3]);
    }
    
    if(gApp_wifiPrms.devId !=  EEPROM_restore(EEPROM_DEVID))
    {
        EEPROM_store(EEPROM_DEVID, gApp_wifiPrms.devId);
    }

    if(gAPP_garagePrms.isEnableHomeKit != EEPROM_restore(EEPROM_ISENHOMEKIT))
    {
        EEPROM_store(EEPROM_ISENHOMEKIT, gAPP_garagePrms.isEnableHomeKit);
    }
}
/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/09
* Brief   : app的画一个字符串函数
* param line
* param margin
* param text
* param textlen
* param youtTextType
* Return  : 
* Remarks : 
******************************************************************************/
Uint8 APP_drawText(Uint16 line, Uint8 margin, Uint8 *text, Uint8 textlen, Uint8 *youtTextType)
{
    Uint8 cow = 0;
    Uint8 row = 0;
    Uint8 chrCnt = 0;
    
    if(textlen > MENU_MAX_TEXT_PER_LINE)
    {
        textlen = MENU_MAX_TEXT_PER_LINE;
    }
    
    row = line;

    if(margin == TEXT_ALIGN_MIDDLE)
    {
        cow = (MENU_MAX_TEXT_PER_LINE - textlen) / 2;
    }else if (margin == TEXT_ALIGN_RIGHT)
    {
        cow = MENU_MAX_TEXT_PER_LINE - textlen;
    }

    
    MENU_drawText(row, cow,text, textlen, 0,youtTextType);
}



/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/14
* Brief   : 全屏占用，进入到获取数字的一个对话框中
* param initVal 
* param maxVal
* param minVal
* param textTitle 标题
* param youtTextType 标题的存储方式
* Return  : 
* Remarks : 
******************************************************************************/
Uint8 APP_openGetNumberDialog(char *textTitle, Uint8 initVal,Uint8 minVal,Uint8 maxVal, Uint8 *youtTextType)
{
 
    Int8 errVal = 0;
    Uint8 tmpNumberBuf[4];
    Uint8 val = 0;
    Uint8 line = 0;
    APP_drawText(line, TEXT_ALIGN_MIDDLE, textTitle, MENU_strlen(textTitle, youtTextType), youtTextType);
    line+=2;
    APP_drawText(line++, TEXT_ALIGN_LEFT, PSTR("+/-: ROTRAY"), 11, TEXT_FLASH);
    line++;
    APP_drawText(line, TEXT_ALIGN_LEFT,   PSTR("OK : PRESS ROTRAY"), 17, TEXT_FLASH);
    line+=2;

    if((initVal > maxVal) || (initVal < minVal))
    {
        return 0;
    }
    val = initVal;
    
    sprintf(tmpNumberBuf, "%3d", val);
    APP_drawText(line, TEXT_ALIGN_LEFT,  tmpNumberBuf, 3, TEXT_RAM);
    
    while(1)
    {
        errVal = KEY_get(KEY_RE);

        if(errVal)
        {
            val += errVal;
            
            if(val > maxVal)
            {
                val = minVal;
            }

            if(val < minVal)
            {
                val = maxVal;
            }
            
            sprintf(tmpNumberBuf, "%3d", val);
            
            APP_drawText(line, TEXT_ALIGN_LEFT,  tmpNumberBuf, 3, TEXT_RAM);
        }

        if(!KEY_get(KEY_REBT))
        {
            KEY_wait(KEY_REBT, KEY_TIMEOUT_FOREVER);//wait key relese 

            return val;
        }
    }
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/21
* Brief   : 全屏，获取一个字符串的对话框 
* param maxLen
* param strLen
* param strMem
* param textTitle
* param youtTextType
* Return  : 
* Remarks : 
******************************************************************************/
Int8 APP_openGetStringDialog(char *textTitle, char* strMem, Uint8 strLen, Uint8 maxLen,  Uint8 *youtTextType)
{   
    Int8 ret = 0;
    Uint8 stat = 0;
    Int8 errVal = 0;
    Int8 charPos = 0;
    Uint8 line = 0;
    char tmpChr = 0;
    
    APP_drawText(line, TEXT_ALIGN_MIDDLE, textTitle, MENU_strlen(textTitle, youtTextType), youtTextType);
    line+=2;
    APP_drawText(line++, TEXT_ALIGN_LEFT, PSTR("+/-: ROTRAY"), 11, TEXT_FLASH);
    line++;
    APP_drawText(line, TEXT_ALIGN_LEFT,   PSTR("OK : PRESS ROTRAY"), 17, TEXT_FLASH);
    line+=2;
    
    if(strLen < maxLen)
    {
        strMem[strLen] = '*';
        strLen++;
    }
    APP_drawText(line, TEXT_ALIGN_LEFT,  strMem, strLen, TEXT_RAM);

    
    while(1)
    {
        switch(stat)
        {
            case STAT_SELECT_CHAR:
            {
                errVal = KEY_get(KEY_RE);
                
                if(errVal)
                {
                    charPos += errVal;
                    if(charPos >= strLen)
                    {
                        charPos = strLen-1;
                    }
                    else if(charPos < 0)
                    {
                        charPos = 0;
                    }
                    
                    tmpChr = strMem[charPos];
                    strMem[charPos] = '_';
                    APP_drawText(line, TEXT_ALIGN_LEFT,  strMem, strLen, TEXT_RAM);
                    strMem[charPos] = tmpChr;
                    
                }
                
                if(!KEY_get(KEY_REBT))
                {
                    ret = KEY_wait(KEY_REBT, 1500);//wait key relese 
                    if(!ret)
                    {
                        stat = STAT_EDIT_CHAR;
                    }
                    else
                    {   
                        if(strLen < maxLen)
                        {
                            strLen--;
                            strMem[strLen] = '\0';
                        }
                        MENU_clrScreen();
                        KEY_wait(KEY_REBT, KEY_TIMEOUT_FOREVER);//wait key relese 
                        return strLen;
                    }
                    
                }
                
                break;
            }
            case STAT_EDIT_CHAR:
            {
                errVal = KEY_get(KEY_RE);
                
                if(errVal)
                {
                    tmpChr = strMem[charPos];
                    strMem[charPos] += errVal;

                    if(strMem[charPos] >'z')
                    {
                        strMem[charPos] = '!';
                    }

                    if(strMem[charPos] <'!')
                    {
                        strMem[charPos] = 'z';
                    }
                    APP_drawText(line, TEXT_ALIGN_LEFT,  strMem, strLen, TEXT_RAM); 
                }

                if(!KEY_get(KEY_REBT))
                {
                    KEY_wait(KEY_REBT, KEY_TIMEOUT_FOREVER);//wait key relese 
                    stat = STAT_STRLEN_CHANGE;
                }
                break;
            }
            case STAT_STRLEN_CHANGE:
            {
                if((charPos+1) == strLen)
                {
                    if(strLen < maxLen)
                    {
                        strMem[strLen] = '*';
                        strLen++;
                        APP_drawText(line, TEXT_ALIGN_LEFT,  strMem, strLen, TEXT_RAM);
                    }
                }
                
                stat = STAT_SELECT_CHAR;
                break;
            }
        }
            
    }
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 全屏，获取一个ip的对话框
* param ipAddr
* param textTitle
* param youtTextType
* Return  : 
* Remarks : 
******************************************************************************/
Int8 APP_openGetIpDialog(char *textTitle, Uint8 ipAddr[4],  Uint8 *youtTextType)
{
    Int8 ret = 0;
    Uint8 line = 0;
    Uint8 hisVal = 0;
    Int8 errVal = STAT_SELECT_CHAR;
    Int8 stat = 0;
    Uint8 pos = 0;
    char ipBuf[17];
    APP_drawText(line, TEXT_ALIGN_MIDDLE, textTitle, MENU_strlen(textTitle, youtTextType), youtTextType);
    line+=2;
    APP_drawText(line++, TEXT_ALIGN_LEFT, PSTR("+/-: ROTRAY"), 11, TEXT_FLASH);
    line++;
    APP_drawText(line, TEXT_ALIGN_LEFT,   PSTR("OK : PRESS ROTRAY"), 17, TEXT_FLASH);
    line+=2;
    sprintf(ipBuf,"%3u.%3u.%3u.%3u",ipAddr[0],ipAddr[1],ipAddr[2],ipAddr[3]);
    APP_drawText(line, TEXT_ALIGN_LEFT,  ipBuf, strlen(ipBuf), TEXT_RAM);
    while(1)
    {
        switch(stat)
        {
            case STAT_SELECT_CHAR:
            {
                errVal = KEY_get(KEY_RE);
                
                if(errVal)
                {
                    pos+=errVal;
                    pos %=4;
                    sprintf(ipBuf,"%3u.%3u.%3u.%3u",ipAddr[0],ipAddr[1],ipAddr[2],ipAddr[3]);
                    ipBuf[pos*4+2] = '_';
                    APP_drawText(line, TEXT_ALIGN_LEFT,  ipBuf, strlen(ipBuf), TEXT_RAM);
                }
                
                if(!KEY_get(KEY_REBT))
                {
                    ret = KEY_wait(KEY_REBT, 1500);//wait key relese 
                    if(!ret)
                    {
                        stat = STAT_EDIT_CHAR;
                    }
                    else
                    {   
                        MENU_clrScreen();
                        KEY_wait(KEY_REBT, KEY_TIMEOUT_FOREVER);//wait key relese 
                        return 0;
                    }
                    
                }
                break;
            }
            case STAT_EDIT_CHAR:
            {
                errVal = KEY_get(KEY_RE);
                if(errVal)
                {
                    ipAddr[pos] += errVal;
                }
                sprintf(ipBuf,"%3u.%3u.%3u.%3u",ipAddr[0],ipAddr[1],ipAddr[2],ipAddr[3]);
                APP_drawText(line, TEXT_ALIGN_LEFT,  ipBuf, strlen(ipBuf), TEXT_RAM);
                if(!KEY_get(KEY_REBT))
                {
                    KEY_wait(KEY_REBT, KEY_TIMEOUT_FOREVER);//wait key relese 
                    stat = STAT_SELECT_CHAR;
                }
                break;
            }
            default:
            {
                stat = STAT_SELECT_CHAR;
            }
        }
    }
    
    
}
/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/09
* Brief   : 构造一个初始菜单,整个界面的核心逻辑在这里生成。
* Return  : 
* Remarks : 
******************************************************************************/
Int8 APP_menuGen(void)
{
    MENU_init();
    Int8 ret = 0;
    APP_dumpPrms();
	TIMER_register(4000, APP_timerDaemon);
	TIMER_register(1000,APP_clockService);
    //申请主页
    ret |= MENU_requestPage(&gMainMenu);
    ret |= MENU_setPageTitle(gMainMenu,PSTR("GARAGE CONTROLER"), TEXT_FLASH);

    if(ret)
    {
        dbgMsg("mainPage fail");
        goto ERR_OUT;
    }
    
    //申请车库页
    ret |= MENU_requestItemFromPage(gMainMenu, &gMainMenu_garage);
    ret |= MENU_setItemText(gMainMenu_garage, PSTR("GARAGE SETTING"), TEXT_FLASH);
    //申请车库子菜单
    ret |= MENU_requestPage(&gGarageMenu);
    ret |= MENU_setPageTitle(gGarageMenu,PSTR("GARAGE SETTING"), TEXT_FLASH);
    //为车库添加返回到主菜单条目
    ret |= MENU_requestItemFromPage(gGarageMenu, &gGarageMenu_goBack);
    ret |= MENU_setItemText(gGarageMenu_goBack, PSTR("GO BACK"), TEXT_FLASH);
    //将主菜单注册为返回条目的子菜单
    ret |= MENU_setItemSubPage(gGarageMenu_goBack, gMainMenu);
    
    //为车库添加上和下两个可变的内存
    ret |= MENU_requestItemFromPage(gGarageMenu, &gGarageMenu_upTimeMenu);
    sprintf(gAPP_garagePrms.upTimeStr,"UP TIME:%02u",gAPP_garagePrms.upTime);
    ret |= MENU_setItemText(gGarageMenu_upTimeMenu, gAPP_garagePrms.upTimeStr, TEXT_RAM);
    ret |= MENU_setItemEvent(gGarageMenu_upTimeMenu, APP_garageSetWorkingUpTime, NULL);
    ret |= MENU_requestItemFromPage(gGarageMenu, &gGarageMenu_downTimeMenu);
    sprintf(gAPP_garagePrms.downTimeStr,"DOWN TIME:%02u",gAPP_garagePrms.downTime);
    ret |= MENU_setItemText(gGarageMenu_downTimeMenu, gAPP_garagePrms.downTimeStr, TEXT_RAM);
    ret |= MENU_setItemEvent(gGarageMenu_downTimeMenu, APP_garageSetWorkingDownTime, NULL);

	ret |= MENU_requestItemFromPage(gGarageMenu, &gGarageMenu_enableHomekit);
	ret |= MENU_setItemText(gGarageMenu_enableHomekit, "ENABLE HOMTKIT", TEXT_RAM);
	ret |= MENU_setItemEvent(gGarageMenu_enableHomekit, APP_garageEnableHomekit, NULL);
	ret |= MENU_requestItemFromPage(gGarageMenu, &gGarageMenu_disableHomekit);
	ret |= MENU_setItemText(gGarageMenu_disableHomekit, "DISABLE HOMTKIT", TEXT_RAM);
	ret |= MENU_setItemEvent(gGarageMenu_disableHomekit, APP_garageDisalbeHomekit, NULL);
    
    //将车库申请的子菜单注册到主菜单的车库项中去
    ret |= MENU_setItemSubPage(gMainMenu_garage, gGarageMenu);
    
    if(ret)
    {
        dbgMsg("garage fail");
        goto ERR_OUT;
    }
    
    //申请wifi页
    ret |= MENU_requestItemFromPage(gMainMenu, &gMainMenu_wifi);
    ret |= MENU_setItemText(gMainMenu_wifi, PSTR("WIFI SETTING"), TEXT_FLASH);
    //申请wifi页子菜单
    ret |= MENU_requestPage(&gWifiMenu);
    ret |= MENU_setPageTitle(gWifiMenu,PSTR("WIFI SETTING"), TEXT_FLASH);
    //返回上一菜单
    ret |= MENU_requestItemFromPage(gWifiMenu, &gWifiMenu_goBack);
    ret |= MENU_setItemText(gWifiMenu_goBack, PSTR("GO BACK"), TEXT_FLASH);
    //将主菜单注册为返回条目的子菜单
    ret |= MENU_setItemSubPage(gWifiMenu_goBack, gMainMenu);
    //为wifi添加wifi ssid设置菜单
    ret |= MENU_requestItemFromPage(gWifiMenu, &gWifiMenu_connect);
    ret |= MENU_setItemText(gWifiMenu_connect, PSTR("CONNECT"), TEXT_FLASH);
    ret |= MENU_setItemEvent(gWifiMenu_connect, APP_wifiConnect, NULL);
    
    ret |= MENU_requestItemFromPage(gWifiMenu, &gWifiMenu_ssid);
    ret |= MENU_setItemText(gWifiMenu_ssid, PSTR("SSID"), TEXT_FLASH);
    ret |= MENU_setItemEvent(gWifiMenu_ssid, APP_wifiSetSSID, NULL);

    ret |= MENU_requestItemFromPage(gWifiMenu, &gWifiMenu_password);
    ret |= MENU_setItemText(gWifiMenu_password, PSTR("PASSWORD"), TEXT_FLASH);
    ret |= MENU_setItemEvent(gWifiMenu_password, APP_wifiSetPSWD, NULL);

    ret |= MENU_requestItemFromPage(gWifiMenu, &gWifiMenu_srvIp);
    ret |= MENU_setItemText(gWifiMenu_srvIp, PSTR("SERVER IP"), TEXT_FLASH);
    ret |= MENU_setItemEvent(gWifiMenu_srvIp, APP_wifiSetSrvIp, NULL);

    
    ret |= MENU_requestItemFromPage(gWifiMenu, &gWifiMenu_devId);
    ret |= MENU_setItemText(gWifiMenu_devId, PSTR("DEVICE ID"), TEXT_FLASH);
    ret |= MENU_setItemEvent(gWifiMenu_devId, APP_wifiSetDevID, NULL);

    
    //将wifi申请的子菜单注册到主菜单中去
    ret |= MENU_setItemSubPage(gMainMenu_wifi, gWifiMenu);
    if(ret)
    {
       dbgMsg("wifi fail");
       goto ERR_OUT;
    }
    
    posHandle = gMainMenu;
    ret = MENU_render(posHandle);

    if(ret)
    {
       dbgMsg("rendert fail");
       goto ERR_OUT;
    }

    APP_dumpPrms();
    ERR_OUT:
        return; 
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/09
* Brief   : 菜单及交互逻辑的展示工作,整个菜单与旋钮 旋钮的交互逻辑在这里完成
* Return  : 
* Remarks : 
******************************************************************************/
void APP_menuLogic(void)
{
    Int8 ret = 0;
    Int8 keyVal = 0;
    keyVal = KEY_get(KEY_RE);
    
    if(keyVal > 0)
    {
        MENU_nextItem(posHandle);
        MENU_render(posHandle);
    }
    else if(keyVal < 0)
    {
        MENU_prevItem(posHandle);
        MENU_render(posHandle);
    }

    if(!KEY_get(KEY_REBT))
    {
        KEY_wait(KEY_REBT, KEY_TIMEOUT_FOREVER);//wait key relese 
        
        ret = MENU_getItem(posHandle, &posItemHandle);
        if(ret)
        {
            dbgMsg("POS can't get itemHandle");
            goto RUN_END;
        }

        ret = MENU_getItemSubPage(posItemHandle, &posHandle);
        if(!ret)
        {
            MENU_clrScreen();
            ret = MENU_render(posHandle);
            if(ret)
            {
                dbgMsg("POS can't render subPage");
            }
            goto RUN_END;
        }
        MENU_clrScreen();
        ret = MENU_runItemEvent(posItemHandle);
        if(!ret)
        {
            MENU_clrScreen();
            ret = MENU_render(posHandle);
            if(ret)
            {
                dbgMsg("POS can't render subPage");
            }
            goto RUN_END;
        }

    }

    RUN_END:
        return;
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/27
* Brief   : 实际在中断执行4s一次
* Return  : 
* Remarks : 
******************************************************************************/
void APP_timerDaemon(void)
{
	gTimerDaemonFlag ++;
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 定时任务，每隔4s就运行一次timerService
* Return  : 
* Remarks : 
******************************************************************************/
void APP_timerServiceBody(void)
{
	static  Uint8 timerHisValue = 0;

	if(timerHisValue == gTimerDaemonFlag)
	{
		return 0;//没有发生跳变时，不动作
	}
	
	timerHisValue = gTimerDaemonFlag;

	APP_timerService();
	
}

void APP_clockService(void)
{
    gClock++;
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/21
* Brief   : 设备定时任务,每隔4s运行,其向服务器报告当前设备状态，同时获得服务器指令。
* Return  : 
* Remarks : 
******************************************************************************/
void APP_timerService(void)
{
    Int8 tmpBuf[20];
	Int8 ret = 0;
	Uint16 recvLen = 0;
	char *recvPtr = NULL;
	Uint8 id = 0;
	if(gAPP_garagePrms.isEnableHomeKit == 0)
	{
		return;
	}
    memset(tmpBuf,0,sizeof(tmpBuf));
    sprintf(tmpBuf,"STATUS:%u",gAPP_garagePrms.status);
	ret = WIFI_sendData(0, 0, tmpBuf, strlen(tmpBuf));
	if(ret)
	{
		//ret |= WIFI_disconnect(0);
		delay(250);
		//WIFI_debug(1);
		ret = WIFI_LinkServer(TCP,gApp_wifiPrms.srvIp,10470,0);
		if(ret)
		{
			dbgMsg("Link to server failed.\n");
		}
		//WIFI_debug(0);
	}

	ret = WIFI_readData(1,  &recvPtr, &id, &recvLen, 800);
	if(ret)
	{
        dbgMsg("\nrecv failed.\n");
        ret |= WIFI_disconnect(0);
		delay(250);
		//WIFI_debug(1);
		ret = WIFI_LinkServer(TCP,gApp_wifiPrms.srvIp,10470,0);
		if(ret)
		{
			dbgMsg("Link to server failed.\n");
		}
		//WIFI_debug(0);
        return;
	}


    sprintf(tmpBuf,"STATUS:%u",STAT_HK_CLOSE);
    if(strstr(recvPtr,tmpBuf) != NULL)
    {
        dbgMsg("opening\n");
        gAPP_garagePrms.status = STAT_HK_CLOSE;
        goto END;
    }

    sprintf(tmpBuf,"STATUS:%u",STAT_HK_OPEN);
    if(strstr(recvPtr,tmpBuf) != NULL)
    {
        dbgMsg("closing\n");
        gAPP_garagePrms.status = STAT_HK_OPEN;
        goto END;
    }

    sprintf(tmpBuf,"STATUS:%u",STAT_HK_IDEL);
    if(strstr(recvPtr,tmpBuf) != NULL)
    {
        dbgMsg("idel\n");
        goto END;
    }

    dbgMsg("\ncommand failed.\n");
    ret |= WIFI_disconnect(0);
    delay(250);
    ret = WIFI_LinkServer(TCP,gApp_wifiPrms.srvIp,10470,0);
    if(ret)
    {
        dbgMsg("Link to server failed.\n");
    }
    
    END:
    return;
    
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/21
* Brief   : 设备服务任务，实时运行 主要控制门的状态
* Return  : 
* Remarks : 
******************************************************************************/
void APP_backgroundService(void)
{
    static Uint16 curTime = 0;
    static Uint8 statusMark = 0;
    if(!KEY_get(KEY_DOWN) || (gAPP_garagePrms.status == STAT_HK_CLOSE))
    {
        statusMark = 1;
        curTime = gClock;
        gAPP_garagePrms.status = STAT_HK_BUSY;
        RELAY_set(RELAY_3, RELAY_ON);
        delay(1000);
        RELAY_set(RELAY_3, RELAY_OFF);
        
    }
    
    if(!KEY_get(KEY_STOP))
    {
        statusMark = 2;
        curTime = gClock;
        gAPP_garagePrms.status = STAT_HK_BUSY;
        RELAY_set(RELAY_2, RELAY_ON);
        delay(1000);
        RELAY_set(RELAY_2, RELAY_OFF);
    }

    if(!KEY_get(KEY_UP) || (gAPP_garagePrms.status == STAT_HK_OPEN))
    {
        statusMark = 3;
        curTime = gClock;
        gAPP_garagePrms.status = STAT_HK_BUSY;
        RELAY_set(RELAY_1, RELAY_ON);
        delay(1000);
        RELAY_set(RELAY_1, RELAY_OFF);
    }
    
    if(statusMark != 0)
    {
        if((statusMark == 1) && ((gClock-curTime) > gAPP_garagePrms.downTime))
        {
            gAPP_garagePrms.status = STAT_HK_IDEL;
            statusMark = 0;
        }
        else if((statusMark == 3) && ((gClock-curTime) > gAPP_garagePrms.upTime))
        {
            gAPP_garagePrms.status = STAT_HK_IDEL;
            statusMark = 0;
        }
        else if(statusMark == 2)
        {
            gAPP_garagePrms.status = STAT_HK_IDEL;
            statusMark = 0;
        }
    }

    
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 菜单回掉，用来设置上升时间
* param prms
* Return  : 
* Remarks : 
******************************************************************************/
Int8 APP_garageSetWorkingUpTime(void *prms)
{
    int ret = 0;
    ret = APP_dumpPrms();
    sprintf(gAPP_garagePrms.upTimeStr,"UP TIME:%02u",gAPP_garagePrms.upTime);
    ret = APP_openGetNumberDialog(PSTR("UP TIME"),gAPP_garagePrms.upTime,1,60,TEXT_FLASH);
    gAPP_garagePrms.upTime = ret;
    sprintf(gAPP_garagePrms.upTimeStr,"UP TIME:%02u",ret);
    ret = APP_flushPrms();
    return 0;
 
}
/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 菜单回调，用来设置下降时间
* param prms
* Return  : 
* Remarks : 
******************************************************************************/
Int8 APP_garageSetWorkingDownTime(void *prms)
{
    int ret = 0;
    ret = APP_dumpPrms();
    sprintf(gAPP_garagePrms.downTimeStr,"DOWN TIME:%02u",gAPP_garagePrms.downTime);
    ret = APP_openGetNumberDialog(PSTR("DOWN TIME"),gAPP_garagePrms.downTime,1,60,TEXT_FLASH);
    gAPP_garagePrms.downTime = ret;
    sprintf(gAPP_garagePrms.downTimeStr,"DOWN TIME:%02u",ret);
    ret = APP_flushPrms();
    return 0;
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 菜单回调，用来设置使用服务器上传
* param prms
* Return  : 
* Remarks : 
******************************************************************************/
Int8 APP_garageEnableHomekit(void * prms)
{
	
	APP_drawText(4, TEXT_ALIGN_MIDDLE, PSTR("ENABLE HOMEKIT"), 14, TEXT_FLASH);
	gAPP_garagePrms.isEnableHomeKit = 1;
    APP_flushPrms();
	delay(2000);
	return 0;
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 菜单回调用来设置失能服务器上传
* param prms
* Return  : 
* Remarks : 
******************************************************************************/
Int8 APP_garageDisalbeHomekit(void * prms)
{
	APP_drawText(4, TEXT_ALIGN_MIDDLE, PSTR("DISABLE HOMEKIT"), 14, TEXT_FLASH);
	gAPP_garagePrms.isEnableHomeKit = 0;
    APP_flushPrms();
	delay(2000);
	return 0;
}



/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 菜单回掉，用来设置SSID
* param prms
* Return  : 
* Remarks : 
******************************************************************************/
Int8 APP_wifiSetSSID(void *prms)
{
    int ret = 0;
    ret = APP_openGetStringDialog(PSTR("SSID"), gApp_wifiPrms.ssid, 
                                    0,WIFI_MAX_NAME_LEN, TEXT_FLASH);
    gApp_wifiPrms.ssidLen = ret;
    
    Serial.print(gApp_wifiPrms.ssidLen);
    return 0;
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 菜单回调 设置密码
* param prms
* Return  : 
* Remarks : 
******************************************************************************/
Int8 APP_wifiSetPSWD(void *prms)
{
    int ret = 0;
    ret = APP_openGetStringDialog(PSTR("PASSWORD"), gApp_wifiPrms.password, 
                                    0,WIFI_MAX_NAME_LEN, TEXT_FLASH);
    gApp_wifiPrms.passwordLen = ret;
    
    Serial.print(gApp_wifiPrms.passwordLen);
    return 0;
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 菜单回调，设置连接
* param prms
* Return  : 
* Remarks : 
******************************************************************************/
Int8 APP_wifiConnect(void *prms)
{
    int ret = 0;
    APP_drawText(4, TEXT_ALIGN_MIDDLE, PSTR("PLEASE WAIT..."), 14, TEXT_FLASH);
    ret = WIFI_connectToAP(gApp_wifiPrms.ssid, gApp_wifiPrms.password);
    if(ret)
    {
        APP_drawText(4, TEXT_ALIGN_MIDDLE, PSTR("CONNECT FAILED"), 14, TEXT_FLASH);
        gApp_wifiPrms.isConnected = 0;
    }
    else
    {
        APP_drawText(4, TEXT_ALIGN_MIDDLE, PSTR("CONNECT SUCCESS"), 15, TEXT_FLASH);
        gApp_wifiPrms.isConnected = 1;
        delay(500);
    }
    delay(1500);
    return 0;
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 回调，设置服务器IP
* param prms
* Return  : 
* Remarks : 
******************************************************************************/
Int8 APP_wifiSetSrvIp(void *prms)
{
    int ret = 0;
    APP_dumpPrms();
    ret = APP_openGetIpDialog(PSTR("SERVER IP"),gApp_wifiPrms.srvIp,TEXT_FLASH);
    if(!ret)
    {
        APP_flushPrms();   
    }
    return 0;
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/05/28
* Brief   : 回掉，设置本设备ID
* param prms
* Return  : 
* Remarks : 
******************************************************************************/
Int8 APP_wifiSetDevID(void *prms)
{
    int ret = 0;
    APP_dumpPrms();
    ret = APP_openGetNumberDialog(PSTR("DEVICE ID"), gApp_wifiPrms.devId, 0, 255, TEXT_FLASH);
    gApp_wifiPrms.devId = ret;
    APP_flushPrms();
    return 0;
}

