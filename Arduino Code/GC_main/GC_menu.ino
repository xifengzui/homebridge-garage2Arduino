/*****************************************************************************
* File    : GC_menu.ino
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 菜单，该菜单实现了一个触发式文字菜单逻辑

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



//Do not edit
#define MENU_MAX_REQUEST_ITEM (MENU_MAX_TEXT_COW - 2)
/*****************************************************************************
*                              数据类型定义
******************************************************************************/
/*
一般页面组织形式如下，我们只要改变每个页的渲染行为，就可以跳转到不同页面，通过页面首指针即可

页下面是条目，条目下可以转到下一个页面或触发事件

page
 |-title
 |
 |-item1
 |   |-text
 |   |-event
 |   |-page
 |
 |-item2
 ...

*/

//菜单项内容,如果想了解菜单生成，第一部就是了解这个结构体
struct MENU_Item
{
    Uint8 *text;
    Uint8 textType;
    Uint8 isItemRequested;
    struct MENU_Page *subPage;
    void *eventArgs;
    Int8 (*event)(void *pageInfo);
};
//页内容,如果想了解菜单生成，第一部就是了解这个结构体
struct MENU_Page
{
    Uint8 isPageRequested;
    Uint8 index;
    Uint8 *titleText;
    Uint8 titleTextType;
    struct MENU_Item item[MENU_MAX_REQUEST_ITEM];
};

static struct MENU_Page gPage[MENU_MAX_REQUEST_PAGE];
/*****************************************************************************
*                                函数声明
******************************************************************************/


/*****************************************************************************
*                                变量定义
******************************************************************************/
char *gs1_ram = "                    ";


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 画一个字符串
* param cow 列
* param FontSize 字体大小
* param isSelect 是否选中
* param row 行
* param text  传入的字符串
* param yourType  你对字符串

* Return  : 
* Remarks : 
******************************************************************************/
void MENU_drawText(Uint8 row, Uint8 cow, Uint8 *text, Uint8 textLen,Uint8 isSelect,Uint8 yourTextType)
{
    char *s1 = PSTR("                      ");
    if(yourTextType == TEXT_FLASH)
    {
        OLED_fastDraw_F(cow, row, text, isSelect,0,textLen);
        if(textLen < MENU_MAX_TEXT_PER_LINE)
        {
            OLED_fastDraw_F(cow+textLen, row, s1, isSelect,0,MENU_MAX_TEXT_PER_LINE-textLen-cow);
        }
    }
    else  if(yourTextType == TEXT_RAM)
    {
        OLED_fastDraw(cow, row, text, isSelect,0,textLen);
        if(textLen < MENU_MAX_TEXT_PER_LINE)
        {
            OLED_fastDraw(cow+textLen, row, gs1_ram, isSelect,0,MENU_MAX_TEXT_PER_LINE-textLen-cow);
        }
    }
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 清除某一行某一段
* param chrCnt
* param cow
* param row
* Return  : 
* Remarks : 
******************************************************************************/
void MENU_clearText(Uint8 cow,Uint8 row, Uint8 chrCnt,Uint8 yourTextType)
{   
    char *clearText = PSTR("                                      ");
    OLED_fastDraw_F(cow, row,clearText,0,0,chrCnt);
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/09
* Brief   : 清屏
* Return  : 
* Remarks : 
******************************************************************************/
void MENU_clrScreen(void)
{
    OLED_clearScreen();
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 提醒字体
* param cow
* param row
* param text
* Return  : 
* Remarks : 
******************************************************************************/
void MENU_drawNotify(Uint8 cow,Uint8 row, Uint8 *text,Uint8 textLen,Uint8 yourTextType)
{
    char *s1 = PSTR("                      ");

    if(yourTextType == TEXT_FLASH)
    {
        OLED_fastDraw_F(cow, row, text,0,1,textLen);
        if(textLen < MENU_MAX_TEXT_PER_LINE)
        {
            OLED_fastDraw_F(cow+textLen, row, gs1_ram, 0,1,MENU_MAX_TEXT_PER_LINE-textLen-cow);
        }
    }
    else
    {
        OLED_fastDraw(cow, row, text,0,1,textLen);
        if(textLen < MENU_MAX_TEXT_PER_LINE)
        {
            OLED_fastDraw(cow+textLen, row, s1, 0,1,MENU_MAX_TEXT_PER_LINE-textLen-cow);
        }
    }
}


void MENU_clearNotify(Uint8 cow,Uint8 row, Uint8 chrCnt)
{
    char *clearText = PSTR("                                      ");
    
    OLED_fastDraw_F(cow, row,clearText,0,1,chrCnt);
}

Uint16 MENU_strlen(char *str, Uint8 yourTextType)
{
    Uint16 ii =0;
    if(yourTextType == TEXT_FLASH)
    {
        while(pgm_read_byte(str) != NULL)
        {
            ii++;
            str++;
        }
    }
    else if(yourTextType == TEXT_RAM)
    {
        ii = strlen(str);
    }
    return ii;
}


Uint8 MENU_init(void)
{
    memset((char *)&gPage,0,sizeof(gPage));
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 申请一个页面
* param handle
* Return  : 成功返回0 失败非0
* Remarks : 
******************************************************************************/
Int8 MENU_requestPage(MENU_Handle *handle)
{
    int ii=0;
    
    for(ii=0;ii<MENU_MAX_REQUEST_PAGE;ii++)
    {
        if(gPage[ii].isPageRequested == 0)
        {
            memset(&gPage[ii],0,sizeof(struct MENU_Page));
            gPage[ii].isPageRequested = 1;
            *handle = &(gPage[ii]);
            return 0;
        }
    }
    return -1;
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/08
* Brief   : 释放一个页面
* param handle
* Return  : 
* Remarks : 
******************************************************************************/
Int8 MENU_relesePage(MENU_Handle handle)
{
    int ii=0;
    
    for(ii=0;ii<MENU_MAX_REQUEST_PAGE;ii++)
    {
        if(&(gPage[ii]) == handle)
        {
            gPage[ii].isPageRequested = 0;
            return 0;
        }
    }
    
    return -1;
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 从已经申请的页面中获取一个item
* param handle
* param itemHandle
* Return  : 成功返回0 失败非0
* Remarks : 
******************************************************************************/
Int8 MENU_requestItemFromPage(MENU_Handle handle, MENU_ItemHandle *itemHandle)
{
    int ii = 0;
    struct MENU_Page *reqPage = NULL;
    struct MENU_Item *pItem = NULL;
    
    if(handle == NULL)
    {
        return -1;
    }
    
    reqPage = (struct MENU_Page *)handle;
    
    for(ii=0;ii<MENU_MAX_REQUEST_ITEM;ii++)
    {
        pItem = &reqPage->item[ii];
        if(pItem->isItemRequested == 0)
        {
            memset(pItem,0,sizeof(struct MENU_Item));
            pItem->isItemRequested = 1;
            *itemHandle = pItem;
            return 0;
        }
    }
    
    return -1;
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 从已经申请的页面中移除一个item
* param handle
* param itemHandle
* Return  :
* Remarks : 
******************************************************************************/
Int8 MENU_removeItemFromPage(MENU_Handle handle, MENU_ItemHandle itemHandle)
{
    int ii = 0;
    struct MENU_Page *reqPage = NULL;
    struct MENU_Item *pItem = NULL;
    
    if(handle == NULL)
    {
        return -1;
    }
    
    reqPage = (struct MENU_Page *)handle;
    
    for(ii=0;ii<MENU_MAX_REQUEST_ITEM;ii++)
    {
        pItem = &reqPage->item[ii];
        if(pItem == itemHandle)
        {
            pItem->isItemRequested = 0;
            return 0;
        }
    } 
    
    return 0;
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 设置一个item的子页面
* param itemHandle
* param subPageHandle
* Return  : 成功返回0 失败非0
* Remarks : 
******************************************************************************/
Int8 MENU_setItemSubPage(MENU_ItemHandle itemHandle,MENU_Handle subPageHandle)
{
    struct MENU_Item *reqItem = NULL;
    
    if((itemHandle == NULL) || (subPageHandle == NULL))
    {
        return -1;
    }
        
    if(reqItem->isItemRequested == 0)
    {
        return -1;
    }
    

    reqItem = (struct MENU_Item *)itemHandle;
    reqItem->subPage  = (struct MENU_Page *)subPageHandle;
    
    return 0;
}



/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 清除一个item的子页面
* param itemHandle
* param subPageHandle
* Return  : 成功返回0 失败非0
* Remarks : 
******************************************************************************/
Int8 MENU_clrSubPage(MENU_ItemHandle itemHandle,MENU_Handle *subPageHandle)
{
    struct MENU_Item *reqItem = NULL;
    
    if((itemHandle == NULL) || (subPageHandle == NULL))
    {
        return -1;
    }
    
    if(reqItem->isItemRequested == 0)
    {
        return -1;
    }
    

    reqItem = (struct MENU_Item *)itemHandle;
    reqItem->subPage  = NULL;
    
    return 0;
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 设置item的事件,一个item只支持一个事件
* param event
* param itemHandle
* Return  : 成功返回0 失败非0
* Remarks : 
******************************************************************************/
Int8 MENU_setItemEvent(MENU_ItemHandle itemHandle, void (*event)(void *eventArgs), void *eventArgs)
{
    struct MENU_Item *reqItem = NULL;
    
    if(itemHandle == NULL)
    {
        return -1;
    }

    reqItem = (struct MENU_Item *)itemHandle;
        
    if(reqItem->isItemRequested == 0)
    {
        return -1;
    }
    
    reqItem->event  = event;
    reqItem->eventArgs = eventArgs;
    
    return 0;
}
/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/08
* Brief   : 执行函数事件
* param handle
* Return  : 
* Remarks : 
******************************************************************************/
Int8 MENU_runItemEvent(MENU_ItemHandle itemHandle)
{
    Int8 ret = 0;
    struct MENU_Item *reqItem = NULL;
    if(itemHandle == NULL)
    {
        return -1;
    }
    reqItem = (struct MENU_Item *)itemHandle;
        
    if(reqItem->isItemRequested == 0)
    {
        return -1;
    }
    if(reqItem->event == NULL)
    {
        return -1;
    }

    ret = reqItem->event(reqItem->eventArgs);
    
    return ret;
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/08
* Brief   : 清除item的事件
* param itemHandle
* Return  : 
* Remarks : 
******************************************************************************/
Int8 MENU_clrItemEvent(MENU_ItemHandle itemHandle)
{
    struct MENU_Item *reqItem = NULL;
    
    if(itemHandle == NULL)
    {
        return -1;
    }

    reqItem = (struct MENU_Item *)itemHandle;
    
    if(reqItem->isItemRequested == 0)
    {
        return -1;
    }
    
    reqItem->event  = NULL;
    
    return 0;
}
/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 设置item的文字，注意！！！字符串必须是全局内存或静态字符串,因为单片机容量有限，
            不能再单独的为字符串分配内存了，内存会不够
* param itemHandle
* param yourString
* Return  : 成功返回0 失败非0
* Remarks : 
******************************************************************************/
Int8 MENU_setItemText(MENU_ItemHandle itemHandle, char *yourString, Uint8 yourTextType)
{
     struct MENU_Item *reqItem = NULL;
    
    if(itemHandle == NULL)
    {
        return -1;
    }

    reqItem = (struct MENU_Item *)itemHandle;

    if(reqItem->isItemRequested == 0)
    {
        return -1;
    }
    
    reqItem->text  = yourString;
    reqItem->textType = yourTextType;
    return 0;
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 清除item的文字，注意！！！字符串必须是全局内存或静态字符串,因为单片机容量有限，
            不能再单独的为字符串分配内存了，内存会不够
* param itemHandle
* param yourString
* Return  : 成功返回0 失败非0
* Remarks : 
******************************************************************************/
Int8 MENU_clrItemText(MENU_ItemHandle itemHandle)
{
     struct MENU_Item *reqItem = NULL;
    
    if(itemHandle == NULL)
    {
        return -1;
    }

    reqItem = (struct MENU_Item *)itemHandle;

    if(reqItem->isItemRequested == 0)
    {
        return -1;
    }
    
    reqItem->text  = NULL;
    reqItem->textType = NULL;
    return 0;
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 设置page的标题，注意！！！字符串必须是全局内存或静态字符串,因为单片机容量有限，
            不能再单独的为字符串分配内存了，字符串内存会不够
* param itemHandle
* param yourString
* Return  : 成功返回0 失败非0
* Remarks : 
******************************************************************************/
Int8 MENU_setPageTitle(MENU_Handle handle, char *yourString,Uint8 yourTextType)
{
     struct MENU_Page *reqPage = NULL;
    
    if(handle == NULL)
    {
        return -1;
    }

    reqPage = (struct MENU_Page *)handle;

    if(reqPage->isPageRequested == 0)
    {
        return -1;
    }
    
    reqPage->titleText  = yourString;
    reqPage->titleTextType = yourTextType;
    
    return 0;
}



/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/08
* Brief   : 渲染一个页面
* param handle
* Return  : 
* Remarks : 
******************************************************************************/
Int8 MENU_render(MENU_Handle handle)
{
    Uint8 ii = 0;
    Uint8 textRenderCnt = 0;
    Uint8 centerStart = 0;
    Uint8 line = 0;
    struct MENU_Page *reqPage = NULL;
    struct MENU_Item *reqItem = NULL;
    if(handle == NULL)
    {
        dbgMsg("handle\n");
        return -1;
    }

    reqPage = (struct MENU_Page *)handle;

    //draw title
    //title 长度不能超过一行长度
    if(reqPage->titleText == NULL)
    {
        dbgMsg("titleText null\n");
        return -1;
    }

    if(MENU_strlen(reqPage->titleText,reqPage->titleTextType) > MENU_MAX_TEXT_PER_LINE)
    {
        dbgMsg("titleText MENU_MAX_TEXT_PER_LINE\n");
        return -1;
    }

    centerStart = (MENU_MAX_TEXT_PER_LINE - MENU_strlen(reqPage->titleText,reqPage->titleTextType))  /2;
    MENU_drawText(line,centerStart,reqPage->titleText,MENU_strlen(reqPage->titleText,reqPage->titleTextType),0,reqPage->titleTextType);
    line+=2;//空一行更美观
    

    for(ii=0;ii<MENU_MAX_REQUEST_ITEM;ii++)
    {
        reqItem = &(reqPage->item[ii]);

        if(reqItem->isItemRequested == 0)
        {
            continue;
        }

        if(reqItem->text == NULL)
        {
            dbgMsg("item text null\n");
            return -1;
        }

        if(MENU_strlen(reqItem->text,reqItem->textType) > MENU_MAX_TEXT_PER_LINE)
        {
            
            textRenderCnt = MENU_MAX_TEXT_PER_LINE;
        }
        else
        {
            textRenderCnt = MENU_strlen(reqItem->text,reqItem->textType);
        }   
        
        MENU_drawText(line,0,reqItem->text,textRenderCnt,(reqPage->index == ii),reqItem->textType);
        line++;
    }
    return 0;
}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/08
* Brief   : 选中下一项目
* param handle
* Return  : 
* Remarks : 
******************************************************************************/
Int8 MENU_nextItem(MENU_Handle handle)
{
    Uint8 ii=0;
    struct MENU_Page *reqPage = NULL;
    struct MENU_Item *reqItem = NULL;
    if(handle == NULL)
    {
        dbgMsg("handle\n");
        return -1;
    }

    reqPage = (struct MENU_Page *)handle;
    if(reqPage->index >= MENU_MAX_REQUEST_ITEM)
    {
        reqPage->index = 0;
    }

    for(ii=reqPage->index+1;ii<MENU_MAX_REQUEST_ITEM;ii++)
    {
        reqItem = &(reqPage->item[ii]);

        if(reqItem->isItemRequested == 0)
        {
            continue;
        }
        reqPage->index = ii;
        return 0;
    }
    
    for(ii=0;ii<MENU_MAX_REQUEST_ITEM;ii++)
    {
        reqItem = &(reqPage->item[ii]);

        if(reqItem->isItemRequested == 0)
        {
            continue;
        }
        reqPage->index = ii;  
        return 0;
    }
      
    return -1;

}

/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/08
* Brief   : 选中前一项
* param handle
* Return  : 
* Remarks : 
******************************************************************************/
Int8 MENU_prevItem(MENU_Handle handle)
{
    Uint8 ii=0;
    struct MENU_Page *reqPage = NULL;
    struct MENU_Item *reqItem = NULL;
    if(handle == NULL)
    {
        dbgMsg("handle\n");
        return -1;
    }

    reqPage = (struct MENU_Page *)handle;
    if(reqPage->index >= MENU_MAX_REQUEST_ITEM)
    {
        reqPage->index = 0;
    }

    for(ii=reqPage->index-1;ii<MENU_MAX_REQUEST_ITEM;ii++)
    {
        reqItem = &(reqPage->item[ii]);

        if(reqItem->isItemRequested == 0)
        {
            continue;
        }
        reqPage->index = ii;
        return 0;
    }
    
    for(ii=MENU_MAX_REQUEST_ITEM;ii>0;ii--)
    {
        reqItem = &(reqPage->item[ii-1]);

        if(reqItem->isItemRequested == 0)
        {
            continue;
        }
        reqPage->index = ii-1;  
        return 0;
    }
      
    return -1;

}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/09
* Brief   : 获取当前选择item的句柄
* param handle
* param itemHandle
* Return  : 
* Remarks : 
******************************************************************************/
Int8 MENU_getItem(MENU_Handle handle, MENU_ItemHandle *itemHandle)
{
    struct MENU_Page *reqPage = NULL;
 
    if(handle == NULL)
    {
        return -1;
    }
    
    reqPage = (struct MENU_Page *)handle;

    if(reqPage->index >=MENU_MAX_REQUEST_ITEM)
    {
        return -1;
    }

    if(reqPage->item[reqPage->index].isItemRequested == 0)
    {
        return -1;
    }
    
    *itemHandle = &reqPage->item[reqPage->index];

    return 0;
}


/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/08
* Brief   : 获取子项菜单
* param handle
* Return  : 
* Remarks : 
******************************************************************************/
Int8 MENU_getItemSubPage(MENU_ItemHandle itemHandle,MENU_Handle *subPageHandle)
{
    struct MENU_Item *reqItem = NULL;  
    if(itemHandle == NULL)
    {
        return -1;
    }
    reqItem = (struct MENU_Item *)itemHandle;
    
    if(reqItem->isItemRequested == 0)
    {
        return -1;
    }

    if(reqItem->subPage == NULL)
    {
        return -1;
    }
    
    *subPageHandle  = reqItem->subPage;
    
    return 0;

}



/*****************************************************************************
* Author  : 惜枫醉
* Date    : 2017/04/07
* Brief   : 取得item的文字，注意！！！字符串必须是全局内存或静态字符串,因为单片机容量有限，
            不能再单独的为字符串分配内存了，内存会不够
* param itemHandle
* param yourString
* Return  : 成功返回0 失败非0
* Remarks : 
******************************************************************************/
Int8 MENU_getItemText(MENU_ItemHandle itemHandle, char **yourString,Uint8 *yourTextType)
{
     struct MENU_Item *reqItem = NULL;
    
    if(itemHandle == NULL)
    {
        return -1;
    }

    reqItem = (struct MENU_Item *)itemHandle;

    if(reqItem->isItemRequested == 0)
    {
        return -1;
    }
    
    *yourString = reqItem->text;
    *yourTextType = reqItem->textType;
    
    return 0;
}

