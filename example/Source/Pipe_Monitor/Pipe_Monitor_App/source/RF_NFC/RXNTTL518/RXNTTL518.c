/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\RF_NFC\RXNTTL518.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-31       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "RXNTTL518.h"
#include "hc32f460_gpio.h"
#include "hc32f460_exint_nmi_swi.h"
#include "hc32f460_interrupts.h"
#include "User_Data.h"
#include "drv_USART.h"
#include "hc32f460.h"
#include "hc32f460_utility.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
uint8_t guc_NFC_Card_ID[20][7] = 
{
    {0x04,0x05,0xCF,0x01,0x41,0x3B,0x03},
    {0x04,0x23,0x53,0xAB,0x46,0x59,0x80},
    {0}
};
/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static SystemPataSt *pst_RXNTTLSystemPara;

const uint16_t crc16_table[256]=
{
    0x0000,0xC0C1,0xC181,0x0140,0xC301,0x03C0,0x0280,0xC241,
    0xC601,0x06C0,0x0780,0xC741,0x0500,0xC5C1,0xC481,0x0440,
    0xCC01,0x0CC0,0x0D80,0xCD41,0x0F00,0xCFC1,0xCE81,0x0E40,
    0x0A00,0xCAC1,0xCB81,0x0B40,0xC901,0x09C0,0x0880,0xC841,
    0xD801,0x18C0,0x1980,0xD941,0x1B00,0xDBC1,0xDA81,0x1A40,
    0x1E00,0xDEC1,0xDF81,0x1F40,0xDD01,0x1DC0,0x1C80,0xDC41,
    0x1400,0xD4C1,0xD581,0x1540,0xD701,0x17C0,0x1680,0xD641,
    0xD201,0x12C0,0x1380,0xD341,0x1100,0xD1C1,0xD081,0x1040,
    0xF001,0x30C0,0x3180,0xF141,0x3300,0xF3C1,0xF281,0x3240,
    0x3600,0xF6C1,0xF781,0x3740,0xF501,0x35C0,0x3480,0xF441,
    0x3C00,0xFCC1,0xFD81,0x3D40,0xFF01,0x3FC0,0x3E80,0xFE41,
    0xFA01,0x3AC0,0x3B80,0xFB41,0x3900,0xF9C1,0xF881,0x3840,
    0x2800,0xE8C1,0xE981,0x2940,0xEB01,0x2BC0,0x2A80,0xEA41,
    0xEE01,0x2EC0,0x2F80,0xEF41,0x2D00,0xEDC1,0xEC81,0x2C40,
    0xE401,0x24C0,0x2580,0xE541,0x2700,0xE7C1,0xE681,0x2640,
    0x2200,0xE2C1,0xE381,0x2340,0xE101,0x21C0,0x2080,0xE041,
    0xA001,0x60C0,0x6180,0xA141,0x6300,0xA3C1,0xA281,0x6240,
    0x6600,0xA6C1,0xA781,0x6740,0xA501,0x65C0,0x6480,0xA441,
    0x6C00,0xACC1,0xAD81,0x6D40,0xAF01,0x6FC0,0x6E80,0xAE41,
    0xAA01,0x6AC0,0x6B80,0xAB41,0x6900,0xA9C1,0xA881,0x6840,
    0x7800,0xB8C1,0xB981,0x7940,0xBB01,0x7BC0,0x7A80,0xBA41,
    0xBE01,0x7EC0,0x7F80,0xBF41,0x7D00,0xBDC1,0xBC81,0x7C40,
    0xB401,0x74C0,0x7580,0xB541,0x7700,0xB7C1,0xB681,0x7640,
    0x7200,0xB2C1,0xB381,0x7340,0xB101,0x71C0,0x7080,0xB041,
    0x5000,0x90C1,0x9181,0x5140,0x9301,0x53C0,0x5280,0x9241,
    0x9601,0x56C0,0x5780,0x9741,0x5500,0x95C1,0x9481,0x5440,
    0x9C01,0x5CC0,0x5D80,0x9D41,0x5F00,0x9FC1,0x9E81,0x5E40,
    0x5A00,0x9AC1,0x9B81,0x5B40,0x9901,0x59C0,0x5880,0x9841,
    0x8801,0x48C0,0x4980,0x8941,0x4B00,0x8BC1,0x8A81,0x4A40,
    0x4E00,0x8EC1,0x8F81,0x4F40,0x8D01,0x4DC0,0x4C80,0x8C41,
    0x4400,0x84C1,0x8581,0x4540,0x8701,0x47C0,0x4680,0x8641,
    0x8201,0x42C0,0x4380,0x8341,0x4100,0x81C1,0x8081,0x4040
};
/****************************************************
函数名称：crc16
函数功能：16位crc校验函数(查表实现)
校验多项式：X^16 + X^15 + X^2 + 1
输入：
dt：数据首地址
len：数据长度
输出：
res：crc校验结果
若输入数据为全0,则返回0xFFFF
****************************************************/
uint16_t crc16(uint8_t *dt, uint16_t len)
{
    uint16_t i;
    uint16_t res;
    for (i=0; i<len; i++)
    {
        if (dt[i]!=0)
        {
            res=0;
            for (i=0; i<len; i++)
            {
                res=(crc16_table[dt[i]^(res&0xFF)]^(res/0x100));
            }
            res = (res>>8) |(res<<8); //交换crc16 校验码的高低字节
            return (res);
        }
    }
    return (0xFFFF);
}

//发送命令
unsigned char func_Send_CMD(unsigned char ucCMD)
{
    unsigned char ucRes = 1;
    uint8_t i;
    unsigned char ucData[10] = {0};
    unsigned short usCrc = 0xFFFF;
    unsigned char ucTimeOutCnt = 0;
    ucData[0] = 0x24;
    ucData[1] = 0x24;
    ucData[2] = 0x00;
    ucData[3] = 0x07;
    ucData[4] = ucCMD;
    usCrc = crc16(ucData, 5);
    
    ucData[5] = (unsigned char)((usCrc >> 8) & 0x00FF);
    ucData[6] = (unsigned char)(usCrc & 0x00FF);

    //while(1)
    for(i=0; i<3; i++)
    {
        drv_mcu_USART_SendData(MODULE_NFC_RFID, ucData, 7);
        ucRes = 0;
        while((pst_RXNTTLSystemPara->UsartData.ucUsartxRecvDataFlag[2] == 0) || (strlen((char*)pst_RXNTTLSystemPara->UsartData.ucUsartxRecvDataArr[2]) == 0))
        {
            //等待接收数据
            Ddl_Delay1ms(100);
            ucTimeOutCnt++;
            if(ucTimeOutCnt >= 10)
            {
                ucTimeOutCnt = 0;
                //return 1;
                ucRes = 1;
                break;
            }
        }
        if(ucRes == 0)
        {
            break;
        }
        //Ddl_Delay1ms(5000);
    }
    if(ucRes == 1)
    {
        return ucRes;
    }
    
    //接收数据
    if(pst_RXNTTLSystemPara->UsartData.usUsartxRecvDataLen[2] > 0)
    {
        usCrc = crc16((uint8_t *)pst_RXNTTLSystemPara->UsartData.ucUsartxRecvDataArr[2], pst_RXNTTLSystemPara->UsartData.usUsartxRecvDataLen[2]);
        if(usCrc == 0)
        {
            ucRes = 0;
        }
    }
    else
    {
        ucRes = 1;
    }
    return ucRes;
}

//读取RXNTTL518版本号；
//返回0：成功；1：失败
unsigned char func_Read_RXNTTL518_Version(void)
{
    unsigned char ucRes = 1;
    ucRes = func_Send_CMD(0x10);
    return ucRes;
}

//寻卡
//返回0：成功；1：失败
unsigned char func_Search_Card(void)
{
    unsigned char ucRes = 1;
    unsigned char ucData[20] = {0};
    unsigned char ucDataLen = 0x00;
    uint8_t i = 0;
    ucRes = func_Send_CMD(0x11);
    if(ucRes == 0)
    {
        if(pst_RXNTTLSystemPara->UsartData.ucUsartxRecvDataArr[2][6] == 0x02)   //有M1卡
        {
            ucDataLen = pst_RXNTTLSystemPara->UsartData.ucUsartxRecvDataArr[2][3] - 9; //获取卡片数据长度
            if(ucDataLen > 0)
            {
                //获取卡片数据
                memcpy(ucData, &pst_RXNTTLSystemPara->UsartData.ucUsartxRecvDataArr[2][7], ucDataLen);
                for(i=0; i<20; i++)
                {
                    if(strncmp((char*)ucData, (char*)guc_NFC_Card_ID[i],ucDataLen) == 0)
                    {
                        ucRes = 0; //成功
                        break;
                    }
                    else
                    {
                        ucRes = 1; //失败
                    }
                }
                
                
            }
            else
            {
                ucRes = 1; //失败
            }
        }
        else    //0x01->无卡; 0x03->有CPU卡
        {
            ucRes = 1;
        }
    }
    return ucRes;
}
//unsigned char  picc_atqa[2],picc_uid[15],picc_sak[3];
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
unsigned char func_RXNTTL518_Init(void)
{
    unsigned char ucRes = 1;
    pst_RXNTTLSystemPara = GetSystemPara();
    //开启供电电源
    PWRRF_PIN_CLOSE();
    Ddl_Delay1ms(100);
    PWRRF_PIN_OPEN();

    drv_mcu_ChangeUSART3_Source(MODULE_NFC_RFID);
    //Ddl_Delay1ms(2000);
    ucRes = func_Read_RXNTTL518_Version();
    //ucRes = func_Search_Card();
    return ucRes;
}



/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
