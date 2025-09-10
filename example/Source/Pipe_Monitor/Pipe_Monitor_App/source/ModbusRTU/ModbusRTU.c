/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\ModbusRTU\ModbusRTU.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-05-16       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "ModbusRTU.h"
#include "WatchDog.h"

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

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
SystemPataSt *pst_MBSystemPara;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
static const unsigned char aucCRCHi[] = {
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40
};

static const unsigned char aucCRCLo[] = {
  0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
  0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
  0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
  0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
  0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
  0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
  0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
  0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 
  0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
  0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
  0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
  0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
  0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 
  0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
  0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
  0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
  0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
  0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
  0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
  0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
  0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
  0x41, 0x81, 0x80, 0x40
};

//unsigned char ucValue1 = 0;
//unsigned char ucValue2 = 0;
unsigned short func_Get_MBCRC16( unsigned char * pucFrame, unsigned short usLen )
{
  	unsigned char           ucCRCHi = 0xFF;
  	unsigned char           ucCRCLo = 0xFF;
  	int             iIndex;
    //unsigned char ucLen = 0;
    //unsigned char ucArr[20] = {0};

  	while( usLen-- )
  	{
        //ucArr[ucLen++] = *pucFrame;
      	iIndex = ucCRCLo ^ *( pucFrame++ );
      	ucCRCLo = ( unsigned char )( ucCRCHi ^ aucCRCHi[iIndex] );
      	ucCRCHi = aucCRCLo[iIndex];
  	}
      //ucValue1 = ucCRCHi;
      //ucValue2 = ucCRCLo;
      //if((ucValue1 != 0) && (ucValue2 != 0))
      //{
      //  ucValue1 = 1;
      //}
      //else
      //{
      //  ucValue2 = 1;
      //}
  	return ( unsigned short )( ucCRCHi << 8 | ucCRCLo );
}

//Crc16计算函数
const unsigned short  crc_ta[256]={ /* CRC余式表 */

    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
    
};

//CRC校验，适用于要校验的数据是连续的
unsigned short  Crc16(unsigned char *ptr, unsigned short len)
{
    unsigned short crc;
    unsigned char  da;

    crc=0;
    while(len--!=0) 
    {
        da=(unsigned char) (crc/256); // 以8位二进制数的形式暂存CRC的高8位 
        crc<<=8; //左移8位，相当于CRC的低8位乘以 
        crc^=crc_ta[da^*ptr]; // 高8位和当前字节相加后再查表求CRC ，再加上以前的CRC 
        ptr++;
    }
    return(crc);
}


//适用用数据不连续的校验
unsigned short  Crc16_New(unsigned short old_crc,unsigned char *ptr, unsigned short len)
{
    unsigned short crc;
    unsigned char  da;

    crc=old_crc;
    while(len--!=0) 
    {
        da=(unsigned char) (crc/256); // 以8位二进制数的形式暂存CRC的高8位 
        crc<<=8; //左移8位，相当于CRC的低8位乘以 
        crc^=crc_ta[da^*ptr]; // 高8位和当前字节相加后再查表求CRC ，再加上以前的CRC 
        ptr++;
    }
    return(crc);
}


void drv_ModbusRTU_Init(void)
{
	pst_MBSystemPara = GetSystemPara();

    stc_port_init_t stcPortInit;

	MEM_ZERO_STRUCT(stcPortInit);

	stcPortInit.enPinMode = Pin_Mode_Out;
    PORT_Init(RTS1_GPIO_PORT, RTS1_GPIO_PIN, &stcPortInit);
    PORT_Init(RTS2_GPIO_PORT, RTS2_GPIO_PIN, &stcPortInit);
    stcPortInit.enPullUp = Enable;
    PORT_Init(USART4_TX_485_1_PORT, USART4_TX_485_1_PIN, &stcPortInit);
    PORT_Init(USART4_TX_485_2_PORT, USART4_TX_485_2_PIN, &stcPortInit);

    //PORT_SetBits(USART4_TX_485_1_PORT,USART4_TX_485_1_PIN);
    //PORT_ResetBits(USART4_TX_485_1_PORT,USART4_TX_485_1_PIN);
    //PORT_SetBits(RTS1_GPIO_PORT,RTS1_GPIO_PIN);
    //PORT_SetBits(PortB,Pin10);
}

/*************************************************
Function   : RegData_HL_Swap_func
Description: //数据，高低位转换。
Input : 获得新的字符串 OScbuf ，ScData输入字符串,字符串总长，HL高低位 1高低对调。Typelong 对调大小
// float : [0x00,0x00,0xA0,0x40] --> 5.0
Return: 
*************************************************/
void RegData_HL_Swap_func(unsigned char * OScbuf,const unsigned char * ScData,unsigned char IDataNum, unsigned char HLflag, unsigned char Typelong)
{
    unsigned char i,j;
    unsigned char   SwapNum = IDataNum/Typelong;
    unsigned char bufData[256];
    memcpy(bufData,ScData,IDataNum);
    
    for(i=0; i<SwapNum; i++)
    {
        for(j=0; j<Typelong; j++)
        {
            if(HLflag == 0)
            {   
                OScbuf[i*Typelong + j ] = bufData[i*Typelong + j];
            }
            else    //返回的数据是一个字符串数组
            {    
                OScbuf[i*Typelong + Typelong -1 - j] = bufData[i*Typelong + j];
            }
        }
    }
}

/*************************************************
Function   : char_reg_rw_mode
Description: char类型参数，寄存器读写
Input : UCHAR CVlaue, USHORT inOpCommand
Return: eMBErrorCode
*************************************************/
eMBErrorCode char_reg_rw_mode( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode, unsigned char CVlaue, en_SaveParaCMD inOpCommand)
{
    unsigned short usAddrP = 2*usAddressP;
    unsigned char *cData;
	unsigned short sValue;
    unsigned char wData[2];
    if(eMode == MB_REG_READ)
    { 
        unsigned short usAddrP = 2*usAddressP;
        if(eMode == MB_REG_READ)
        {  
            sValue = (unsigned short)CVlaue;
            cData = (unsigned char*)&sValue;
            RegData_HL_Swap_func(pucRegBuffer, cData + usAddrP,2*usNRegs - usAddrP, 1, sizeof(sValue));   
        }
    }
    else
    {  
        if(inOpCommand == DEV_BEGIN_PARA)
        {
            return MB_ENOERR;//不写
        }
        if(usAddrP != 0)
        {
            return MB_ENOREG;
        }
        RegData_HL_Swap_func((unsigned char *)&wData[0], pucRegBuffer,2*usNRegs , 1, sizeof(sValue));   
        if(1 !=  func_Save_Device_Parameter(inOpCommand, (unsigned char *)&wData[0]) )
        {
          return MB_EINVAL;
        }
    }
    return MB_ENOERR;
}

/*************************************************
Function   : Short_reg_rw_mode
Description: Short类型参数，寄存器读写
Input : UCHAR CVlaue, USHORT inOpCommand
Return: eMBErrorCode
*************************************************/
eMBErrorCode Short_reg_rw_mode( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode, unsigned short SVlaue, en_SaveParaCMD inOpCommand)
{
    unsigned short usAddrP = 2*usAddressP;
    unsigned char *cData;
    unsigned char wData[2];
    if(eMode == MB_REG_READ)
    { 
        unsigned short usAddrP = 2*usAddressP;
        if(eMode == MB_REG_READ)
        {  
            cData = (unsigned char*)&SVlaue;
            RegData_HL_Swap_func(pucRegBuffer, cData + usAddrP,2*usNRegs - usAddrP, 1, sizeof(SVlaue));   
        }
    }
    else
    {  
        if(inOpCommand == DEV_BEGIN_PARA)
        {
            return MB_ENOERR;//不写
        }
        if(usAddrP != 0)
        {
            return MB_ENOREG;
        }
        RegData_HL_Swap_func((unsigned char *)&wData[0], pucRegBuffer,2*usNRegs , 1, sizeof(SVlaue));   
        if(1 !=  func_Save_Device_Parameter(inOpCommand, (unsigned char *)&wData[0]) )
        {
          return MB_EINVAL;
        }
    }
    return MB_ENOERR;
}

/*************************************************
Function   : Float_reg_rw_mode
Description: //float类型参数，寄存器读写
Input : 
Return: eMBErrorCode
*************************************************/
eMBErrorCode Float_reg_rw_mode( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode, float FVlaue, en_SaveParaCMD inOpCommand)
{
    unsigned short usAddrP = 2*usAddressP;
    unsigned char *cData;
    unsigned char wData[4];
    if(eMode == MB_REG_READ)
    { 
        cData = (unsigned char*)&FVlaue;
        RegData_HL_Swap_func(pucRegBuffer, cData + usAddrP,2*usNRegs - usAddrP, 1, sizeof(FVlaue));
    }
    else
    {
        if(inOpCommand == DEV_BEGIN_PARA)
        {
            return MB_ENOERR;//不写
        }
        if(usAddrP != 0)
        {
            return MB_ENOREG;
        }
        RegData_HL_Swap_func((unsigned char *)&wData[0], pucRegBuffer,2*usNRegs , 1, 2*usNRegs);   
        if(1 !=  func_Save_Device_Parameter(inOpCommand, (unsigned char *)&wData[0]) )
        {
          return MB_EINVAL;
        }
    }
    return MB_ENOERR;
}

/*************************************************
Function   : Int_reg_rw_mode
Description: //float类型参数，寄存器读写
Input : 
Return: eMBErrorCode
*************************************************/
eMBErrorCode Int_reg_rw_mode( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode, int IVlaue, en_SaveParaCMD inOpCommand)
{
    unsigned short usAddrP = 2*usAddressP;
    unsigned char *cData;
    unsigned long ulValue;
    unsigned char wData[4];
    if(eMode == MB_REG_READ)
    { 
        ulValue = (unsigned long)IVlaue;
        cData = (unsigned char*)&ulValue;
        RegData_HL_Swap_func(pucRegBuffer, cData + usAddrP,2*usNRegs - usAddrP, 1, sizeof(ulValue));
    }
    else
    {
        if(inOpCommand == DEV_BEGIN_PARA)
        {
            return MB_ENOERR;//不写
        }
        if(usAddrP != 0)
        {
            return MB_ENOREG;
        }
        RegData_HL_Swap_func((unsigned char *)&wData[0], pucRegBuffer,2*usNRegs , 1, 2*usNRegs);   
        if(1 !=  func_Save_Device_Parameter(inOpCommand, (unsigned char *)&wData[0]) )
        {
          return MB_EINVAL;
        }
    }
    return MB_ENOERR;
}

//等待接收传感器返回数据及对返回的数据进行CRC校验
//return: -1.0:等待超时; -2.0: CRC校验异常; -3.0:接收数据长度异常
unsigned char* drv_Wait_RecvData_And_CrcCheck(enPara_Type enPaType, unsigned short usRegNum)
{
	unsigned short usRecvDataLength = 0;

	//数据校验及获取
	usRecvDataLength = usRegNum * 2 + 5; //寄存器个数*2 + 地址(1Byte)+功能码(1Byte:03/06/10)+数据长度(1Byte)+CRC校验码(2Bytes)
    Ddl_Delay1ms(10); //延时10ms，等待数据接收完成
	//判断接收数据个数是否正确
	//if(pst_MBSystemPara->UsartData.usUsartxRecvDataLen[3] == usRecvDataLength)
	{
		//判断CRC校验码
		if(func_Get_MBCRC16((unsigned char*)pst_MBSystemPara->UsartData.ucUsart4RecvDataArr,usRecvDataLength) != 0)
		{
            pst_MBSystemPara->DeviceRunPara.cModbusErrCnt++;
            if(pst_MBSystemPara->DeviceRunPara.cModbusErrCnt > 20)
            {
                pst_MBSystemPara->DeviceRunPara.cModbusErrCnt = 0;
                pst_MBSystemPara->DeviceRunPara.usDevStatus |= 0x0020; //设置设备异常状态
            }
			//pst_MBSystemPara->DeviceRunPara.usDevStatus |= 0x0020;
			return NULL;
		}
        else
        {
            pst_MBSystemPara->DeviceRunPara.cModbusErrCnt = 0; //清除错误计数
        }
	}
	//else
	{
	//	pst_MBSystemPara->DeviceRunPara.usDevStatus |= 0x0020;
	//	return NULL;
	}
	memset(guc_RTURecvArr,0,sizeof(guc_RTURecvArr));
	pst_MBSystemPara->DeviceRunPara.usDevStatus &= 0xFFDF;
	if(enPaType == Type_Float)
	{
		RegData_HL_Swap_func(guc_RTURecvArr,(unsigned char*)&pst_MBSystemPara->UsartData.ucUsart4RecvDataArr[3],usRegNum*2,1,usRegNum*2);
	}
    else if(enPaType == Type_Short)
    {
        RegData_HL_Swap_func(guc_RTURecvArr,(unsigned char*)&pst_MBSystemPara->UsartData.ucUsart4RecvDataArr[3],usRegNum*2,1,usRegNum*2);
    }
    else if(enPaType == Type_Long)
    {
        memcpy(guc_RTURecvArr, (unsigned char*)&pst_MBSystemPara->UsartData.ucUsart4RecvDataArr[3], 4);
    }
    else if(enPaType == Type_Float_Low) //浮点数四字节，低位在前，不用对换位置
    {
        RegData_HL_Swap_func(guc_RTURecvArr,(unsigned char*)&pst_MBSystemPara->UsartData.ucUsart4RecvDataArr[3],usRegNum*2,0,usRegNum*2);
    }
	
	//fValue = *((float*)ucRecvDataValueArr);
	return &guc_RTURecvArr[0];
}

//RTS信号引脚控制及发送命令，返回值：0->发送成功; 1->发送失败(超时未接收到设备返回数据)
unsigned char func_Rts_Control_And_SendData(en_usart_device_t ucDeviceType,unsigned char* ucSendData, unsigned short usDataLen, unsigned short usDelayCnt)
{
    unsigned short usRecvTimeOutCnt = 0;

    if(ucDeviceType == MODULE_MEAS_SENSOR2)
    {
        PORT_SetBits(USART4_TX_485_2_PORT,USART4_TX_485_2_PIN);
        RST2_PIN_OPEN();
    }
    else
    {
        PORT_SetBits(USART4_TX_485_1_PORT,USART4_TX_485_1_PIN);
        RST1_PIN_OPEN();
    }

    pst_MBSystemPara->UsartData.ucUsartxRecvDataFlag[3] = 0;
    drv_mcu_USART_SendData(ucDeviceType,ucSendData,usDataLen);
    
    usRecvTimeOutCnt = 0;
    while(pst_MBSystemPara->UsartData.ucUsartxRecvDataFlag[3] == 0)
    {
        //最长延时等待1s，等待串口接收到设备返回的数据
        Ddl_Delay1ms(10);
        usRecvTimeOutCnt++;
        if(usRecvTimeOutCnt >= usDelayCnt)
        {
            pst_MBSystemPara->DeviceRunPara.cModbusErrCnt++;
            if(pst_MBSystemPara->DeviceRunPara.cModbusErrCnt > 20)
            {
                pst_MBSystemPara->DeviceRunPara.cModbusErrCnt = 0;
                pst_MBSystemPara->DeviceRunPara.usDevStatus |= 0x0020; //设置设备异常状态
            }
            //pst_MBSystemPara->DeviceRunPara.usDevStatus |= 0x0020;
            return 1;
            //break;
        }
    }
    return 0;
}


//获取一体式电导率传感器设备测量数值
//返回值：0->发送成功; 1->发送失败(超时未接收到设备返回数据)
unsigned char func_Get_Meas_BY_Integrated_Conductivity_Sensor_Value(en_usart_device_t ucDeviceType)
{
	unsigned char ucSendBuf[10] = {0};
	float *pfMeasValue = NULL;
	unsigned short usCrc = 0xFFFF;
	unsigned char i = 0;
    unsigned char j = 0;
	char *pcRecvData = NULL;
    char cFloat[4] = {0}; //接收数据缓冲区
    unsigned char ucResult = 1;
    unsigned short usCMD = 0;
	
    for(j = 0; j < 4; j++)
    {
        i = 0;
        switch (j)
        {
        case 0:
            usCMD = 0x0002; //获取电导率值
            pfMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esBY_IntegratedConductivityData.fConductivityValue;
            break;
        case 1: //获取温度值
            usCMD = 0x0004; //获取温度值
            pfMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esBY_IntegratedConductivityData.fTemperatureValue;
            break;
        case 2: //获取盐度值
            usCMD = 0x0006; //获取电导率值
            pfMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esBY_IntegratedConductivityData.fSalinityValue;
            break;
        case 3: //获取TDS值
            usCMD = 0x0008; //获取TDS值
            pfMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esBY_IntegratedConductivityData.fTDSValue;
            break;
        default:
            break;
        }
        ucSendBuf[i++] = BY_INTEGRATED_CONDUCTIVITY_SENSOR_ADDR;
        ucSendBuf[i++] = 0x04;
        ucSendBuf[i++] = usCMD >> 8;    //高字节
        ucSendBuf[i++] = usCMD & 0xFF; //低字节
        ucSendBuf[i++] = 0x00;
        ucSendBuf[i++] = 0x02;
        usCrc = func_Get_MBCRC16(ucSendBuf,i);
        ucSendBuf[i++] = usCrc & 0xFF;
        ucSendBuf[i++] = usCrc >> 8;

        ucResult = func_Rts_Control_And_SendData(ucDeviceType,ucSendBuf,i, 50);
        if(ucResult == 0)
        {
            pcRecvData = (char *)drv_Wait_RecvData_And_CrcCheck(Type_Float , 2);
            if(pcRecvData != NULL)
            {
                cFloat[0] = pcRecvData[2];
                cFloat[1] = pcRecvData[3];
                cFloat[2] = pcRecvData[0];
                cFloat[3] = pcRecvData[1];
                *pfMeasValue = *((float*)cFloat);
                ucResult = 0;
            }
            else
            {
                *pfMeasValue = 0.0;/* code */
                ucResult = 1;
            }
            
        }
    }
	
	return ucResult;
}

//获取流量计数据
float func_Get_Meas_Flowermeter_Sensor_Value(en_usart_device_t ucDeviceType)
{
    unsigned char ucSendBuf[10] = {0};
    float fMeasValue = 0.0;
	unsigned short usCrc = 0xFFFF;
	unsigned char i = 0;
	char *pcRecvData = NULL;
    unsigned char ucResult = 1;

    ucSendBuf[i++] = FLOWERMETER_SENSOR_ADDR;
	ucSendBuf[i++] = 0x04;
	ucSendBuf[i++] = 0x00;
	ucSendBuf[i++] = 0x00;
	ucSendBuf[i++] = 0x00;
	ucSendBuf[i++] = 0x02;
	usCrc = func_Get_MBCRC16(ucSendBuf,i);
	ucSendBuf[i++] = usCrc & 0xFF;
	ucSendBuf[i++] = usCrc >> 8;

    ucResult = func_Rts_Control_And_SendData(ucDeviceType,ucSendBuf,i, 50);
    if(ucResult == 0)
    {
        pcRecvData = (char *)drv_Wait_RecvData_And_CrcCheck(Type_Float , 2);
        if(pcRecvData != NULL)
        {
            fMeasValue = *((float*)pcRecvData);
            ucResult = 0;
        }
        else
        {
            ucResult = 1;
        }
    }
	return fMeasValue;
}

//获取雷达液位传感器设备测量数值
//返回值：0->发送成功; 1->发送失败(超时未接收到设备返回数据)
unsigned char func_Get_Meas_BY_Radar_Level_Sensor_Value(en_usart_device_t ucDeviceType)
{
    unsigned char ucSendBuf[10] = {0};
    float *fMeasValue = NULL;
	unsigned short usCrc = 0xFFFF;
	unsigned char i = 0;
	char *pcRecvData = NULL;
    unsigned short usValue = 0;
    unsigned char ucResult = 1;

    fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esBY_LevelData.fRadarEmptyHeightValue;

    ucSendBuf[i++] = BY_RADAR_WATER_LEVEL_SENSOR_ADDR;
	ucSendBuf[i++] = 0x03;
	ucSendBuf[i++] = 0x00;
	ucSendBuf[i++] = 0x01;
	ucSendBuf[i++] = 0x00;
	ucSendBuf[i++] = 0x01;
	usCrc = func_Get_MBCRC16(ucSendBuf,i);
	ucSendBuf[i++] = usCrc & 0xFF;
	ucSendBuf[i++] = usCrc >> 8;

    ucResult = func_Rts_Control_And_SendData(ucDeviceType,ucSendBuf,i, 50);
    if(ucResult == 0)
    {
        pcRecvData = (char *)drv_Wait_RecvData_And_CrcCheck(Type_Short , 1);
        if(pcRecvData != NULL)
        {
            usValue = *((unsigned short*)pcRecvData);
            *fMeasValue = (float)((double)usValue / 1000.0);
            ucResult = 0;
        }
        else
        {
            *fMeasValue = 0.0;
            ucResult = 1;
        }
    }

    if(pst_MBSystemPara->DeviceRunPara.esMeasData.esBY_LevelData.fRadarEmptyHeightValue >= 0.01)
    {
        pst_MBSystemPara->DeviceRunPara.esMeasData.esBY_LevelData.fRadarWaterLevelValue = pst_MBSystemPara->DevicePara.fInit_Height - pst_MBSystemPara->DeviceRunPara.esMeasData.esBY_LevelData.fRadarEmptyHeightValue;
    }
    else
    {
        pst_MBSystemPara->DeviceRunPara.esMeasData.esBY_LevelData.fRadarWaterLevelValue = 0.0;
    }
	return ucResult;
}


//获取压力液位传感器设备测量数值
//返回值：0->发送成功; 1->发送失败(超时未接收到设备返回数据)
unsigned char func_Get_Meas_BY_Pressure_Level_Sensor_Value(en_usart_device_t ucDeviceType)
{
    unsigned char ucSendBuf[10] = {0};
    float *fMeasValue = NULL;
	unsigned short usCrc = 0xFFFF;
	unsigned char i = 0;
	char *pcRecvData = NULL;
    unsigned short usValue = 0;
    unsigned char ucResult = 1;

    fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esBY_LevelData.fPressureWaterLevelValue;

    ucSendBuf[i++] = BY_PRESSURE_WATER_LEVEL_SENSOR_ADDR;
	ucSendBuf[i++] = 0x03;
	ucSendBuf[i++] = 0x00;
	ucSendBuf[i++] = 0x00;
	ucSendBuf[i++] = 0x00;
	ucSendBuf[i++] = 0x01;
	usCrc = func_Get_MBCRC16(ucSendBuf,i);
	ucSendBuf[i++] = usCrc & 0xFF;
	ucSendBuf[i++] = usCrc >> 8;

    ucResult = func_Rts_Control_And_SendData(ucDeviceType,ucSendBuf,i, 50);
    if(ucResult == 0)
    {
        pcRecvData = (char *)drv_Wait_RecvData_And_CrcCheck(Type_Short , 1);
        if(pcRecvData != NULL)
        {
            usValue = *((unsigned short*)pcRecvData);
            *fMeasValue = (float)(((double)usValue - 1007) / 100.0) ; //压力传感器返回值为0.1MPa，转换为水位需要减去1.007m
            ucResult = 0;
        }
        else
        {
            *fMeasValue = 0.0;
            ucResult = 1;
        }
    }
	return ucResult;
}

//获取航征雷达超声流量计瞬时流量数据
unsigned char func_Get_Meas_HZ_Radar_Ultrasonic_Flow_Sensor_Value(en_usart_device_t ucDeviceType, EMeasSensorType eMeasSensorType)
{
    unsigned char ucSendBuf[10] = {0};
    float *fMeasValue = NULL;
	unsigned short usCrc = 0xFFFF;
	unsigned char i = 0;
    unsigned char j = 0;
	char *pcRecvData = NULL;
    unsigned short usCMD = 0x0001; 
    unsigned short usRegNum = 1;
    unsigned short usValue = 0;
    unsigned char ucResult = 1;
    signed long slValue = 0;
    uint64_t ulValue = 0;
    signed short sValue = 0;
    uint32_t ulValue1 = 0;
    
    enPara_Type ePType = Type_Char;
    //float fValue = 0.0;

    for(j=0; j<12; j++)
    {
        i = 0;
        switch (j)
        {
        case 0: //获取瞬时流量值
            usCMD = 0x0001; //获取瞬时流量值
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHZ_Radar_Ultrasonic_FlowData.fFlowValue;
            usRegNum = 2;
            ePType = Type_Long;
            break;
        case 1: //获取正向累计流量1---高位
            usCMD = 0x0003; //获取流速
            usRegNum = 1;
            ePType = Type_Short;
            break;
        case 2: //获取正向累计流量2
            usCMD = 0x0004; //获取正向累计流量
            usRegNum = 1;
            ePType = Type_Short;
            break;
        case 3: //获取正向累计流量3---低位
            usCMD = 0x0005; //获取正向累计流量
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHZ_Radar_Ultrasonic_FlowData.fPositiveCumulativeTraffic;
            usRegNum = 1;
            ePType = Type_Short;
            break;
        case 4: //获取过水面积
            usCMD = 0x0009;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHZ_Radar_Ultrasonic_FlowData.fDischargeArea;
            usRegNum = 2;
            ePType = Type_Long; 
            break;
        case 5: //获取表面流速
            usCMD = 0x000B; //获取表面流速
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHZ_Radar_Ultrasonic_FlowData.fSurfaceVelocity;
            usRegNum = 1;
            ePType = Type_Short; 
            break;
        case 6: //获取断面流速
            usCMD = 0x000C; 
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHZ_Radar_Ultrasonic_FlowData.fCross_SectionVelocity;
            usRegNum = 1;
            ePType = Type_Short; 
            break;
        case 7: //获取水深
            usCMD = 0x000D; //获取水深
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHZ_Radar_Ultrasonic_FlowData.fWaterLevel;
            usRegNum = 2;
            ePType = Type_Long;
            break;
        case 8:
            if(eMeasSensorType == Meas_HZ_Radar_Ultrasonic_Flow)    //雷达超声波流量计含有获取空高命令
            {
                usCMD = 0x000F;
                fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHZ_Radar_Ultrasonic_FlowData.fEmptyHeight;
                usRegNum = 2;
                ePType = Type_Long; //获取空高值
            }
            else
            {
                continue;
            }
            break;
        case 9: //横滚角
            if(eMeasSensorType == Meas_HZ_Radar_Ultrasonic_Flow)
            {
                usCMD = 0x0013; //获取横滚角
            }
            else
            {
                usCMD = 0x0093; //获取横滚角
            }
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHZ_Radar_Ultrasonic_FlowData.fRadarRoll_Angle;
            usRegNum = 1;
            ePType = Type_Short; //获取横滚角
            break;
        case 10: //获取垂直角
            if(eMeasSensorType == Meas_HZ_Radar_Ultrasonic_Flow)
            {
                usCMD = 0x0014; //获取横滚角
            }
            else
            {
                usCMD = 0x0094; //获取横滚角
            }
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHZ_Radar_Ultrasonic_FlowData.fRadarVertical_Angle;
            usRegNum = 1;
            ePType = Type_Short; //获取横滚角
            break;
        case 12: //获取安装高度
            usCMD = 0x0308; 
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHZ_Radar_Ultrasonic_FlowData.fRadarInstallHeight;
            usRegNum = 2;
            ePType = Type_Long;
            break;
        default:
            break;
        }
        ucSendBuf[i++] = HZ_RADAR_ULTRASONIC_FLOW_SENSOR_ADDR;
        ucSendBuf[i++] = 0x03;
        ucSendBuf[i++] = usCMD >> 8;    //高字节
        ucSendBuf[i++] = usCMD & 0xFF; //低字节
        ucSendBuf[i++] = usRegNum >> 8; //高字节
        ucSendBuf[i++] = usRegNum & 0xFF; //低字节
        usCrc = func_Get_MBCRC16(ucSendBuf,i);
        ucSendBuf[i++] = usCrc & 0xFF;
        ucSendBuf[i++] = usCrc >> 8;
    
        ucResult = func_Rts_Control_And_SendData(ucDeviceType,ucSendBuf,i, 50);
        if(ucResult == 0)
        {
            pcRecvData = (char *)drv_Wait_RecvData_And_CrcCheck(ePType , usRegNum);
            if(pcRecvData != NULL)
            {
                switch (j)
                {
                case 0: //瞬时流量
                    slValue = (pcRecvData[0] << 24) | (pcRecvData[1] << 16) | (pcRecvData[2] << 8) | pcRecvData[3];
                    *fMeasValue = ((float)slValue) / 10000.0f ; //
                    break;
                case 1: //正向累计流量1:
                    usValue = (pcRecvData[0] << 8) | pcRecvData[1];
                    ulValue = (uint64_t)usValue << 32;
                    break;
                case 2: //正向累计流量2:
                    usValue = (pcRecvData[0] << 8) | pcRecvData[1];
                    ulValue |= (uint64_t)usValue << 16;
                    break;
                case 3: //正向累计流量3:
                    usValue = (pcRecvData[0] << 8) | pcRecvData[1];
                    ulValue |= (uint64_t)usValue;
                    *fMeasValue = (float)(((double)ulValue) / 100.0); //正向累计流量返回值为m^3，转换为m^3
                    break;
                case 4: //过水面积
                    ulValue1 = (pcRecvData[0] << 24) | (pcRecvData[1] << 16) | (pcRecvData[2] << 8) | pcRecvData[3];
                    *fMeasValue = ((float)ulValue1) / 10000.0f ; //
                    break;
                case 5: //表面流速:
                case 6: //断面流速
                    sValue = (pcRecvData[0] << 8) | pcRecvData[1];
                    *fMeasValue = ((float)sValue) / 1000.0f; 
                    break;
                case 7: //水深
                case 8: //雷达超声流量计-空高
                case 12:    //安装高度
                        ulValue1 = (pcRecvData[0] << 24) | (pcRecvData[1] << 16) | (pcRecvData[2] << 8) | pcRecvData[3];
                        *fMeasValue = ((float)ulValue1) / 1000.0f ; //
                        break;
                case 9: //横滚角
                case 10: //垂直角
                    sValue = (pcRecvData[0] << 8) | pcRecvData[1];
                    *fMeasValue = ((float)sValue) / 10.0f; //横滚角返回值为0.1度，转换为度
                    break;
                default:
                    *fMeasValue = 0.0f;
                    break;
                }
                
                ucResult = 0;
            }
            else
            {
                *fMeasValue = 0.0f;
                ucResult = 1;
                break;
            }
        }
    }
	return ucResult;
}

//获取航征雷达液位计数据
//返回值：0->发送成功; 1->发送失败(超时未接收到设备返回数据)
unsigned char func_Get_Meas_HZ_Radar_Level_Sensor_Value(en_usart_device_t ucDeviceType)
{
    unsigned char ucSendBuf[10] = {0};
    float *fMeasValue = NULL;
	unsigned short usCrc = 0xFFFF;
    unsigned short usCMD = 0x0001; //获取液位值
    unsigned short usRegNum = 0;
	unsigned char i = 0;
    unsigned char j = 0;
	char *pcRecvData = NULL;
    unsigned short usValue = 0;
    unsigned char ucResult = 1;

    for(j=0; j<4; j++)
    {
        i = 0;
        switch (j)
        {
        case 0:
            usCMD = 0x0006; //获取空高值
            usRegNum = 1;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHZ_LevelData.fRadarEmptyHeightValue;
            break;
        case 1:
            usCMD = 0x0009; //获取横滚角
            usRegNum = 1;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHZ_LevelData.fRadarRoll_Angle;
            break;
        case 2:
            usCMD = 0x000A; //获取垂直角
            usRegNum = 1;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHZ_LevelData.fRadarVertical_Angle;
            break;
        case 3:
            usCMD = 0x0049; //获取安装高度
            usRegNum = 2;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHZ_LevelData.fRadarInstallHeight;
            break;
        default:
            break;
        }
        ucSendBuf[i++] = HZ_RADAR_WATER_LEVEL_SENSOR_ADDR;
        ucSendBuf[i++] = 0x03;
        ucSendBuf[i++] = usCMD >> 8;    //高字节
        ucSendBuf[i++] = usCMD & 0xFF; //低字节
        ucSendBuf[i++] = usRegNum >> 8; //高字节
        ucSendBuf[i++] = usRegNum & 0xFF; //低字节
        usCrc = func_Get_MBCRC16(ucSendBuf,i);
        ucSendBuf[i++] = usCrc & 0xFF;
        ucSendBuf[i++] = usCrc >> 8;

        ucResult = func_Rts_Control_And_SendData(ucDeviceType,ucSendBuf,i, 50);
        if(ucResult == 0)
        {
            if(j != 3)
            {
                pcRecvData = (char *)drv_Wait_RecvData_And_CrcCheck(Type_Short , 1);
            }
            else
            {
                pcRecvData = (char *)drv_Wait_RecvData_And_CrcCheck(Type_Float_Low , 2);
            }
            
            if(pcRecvData != NULL)
            {
                if(j != 3)
                {
                    usValue = *((unsigned short*)pcRecvData);
                    *fMeasValue = ((float)usValue / 1000.0f); //雷达液位计返回值为mm，转换为m
                }
                else
                {
                    *fMeasValue = *((float*)pcRecvData); //安装高度返回值为m
                }
                ucResult = 0;
            }
            else
            {
                *fMeasValue = 0.0f;
                
                ucResult = 1;
                break;
            }
        }
        if(j == 0)
        {
            pst_MBSystemPara->DeviceRunPara.esMeasData.esHZ_LevelData.fRadarWaterLevelValue = pst_MBSystemPara->DevicePara.fInit_Height - (pst_MBSystemPara->DeviceRunPara.esMeasData.esHZ_LevelData.fRadarEmptyHeightValue / 1000.0f);
        }
        else if((j== 1) || (j == 2))
        {
            *fMeasValue /= 10.0f;
        }
    }
    
	return ucResult;
}

//获取恒星雷达超声流量计瞬时流量数据
unsigned char func_Get_Meas_HX_Radar_Ultrasonic_Flow_Sensor_Value(en_usart_device_t ucDeviceType)
{
    unsigned char ucSendBuf[10] = {0};
    float *fMeasValue = NULL;
	unsigned short usCrc = 0xFFFF;
    unsigned short usCMD = 0x0001; //获取瞬时流量值
    unsigned short usRegNum = 2;
	unsigned char i = 0;
    unsigned char j = 0;
	char *pcRecvData = NULL;
    //unsigned short usValue = 0;
    unsigned char ucResult = 1;
    //signed long slValue = 0;
    //float fValue = 0.0;
    enPara_Type ePType = Type_Float_Low;
    for(j=0; j<10; j++)
    {
        i = 0;
        switch (j)
        {
        case 0: //获取断面流速
            usCMD = 0x0000;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_FlowmeterData.fCross_SectionVelocity;
            break;
        case 1: //获取液位值
            usCMD = 0x0002;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_FlowmeterData.fWaterLevel;
            break;
        case 2: //获取温度值
            usCMD = 0x0004;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_FlowmeterData.fTemperature;
            break;
        case 3: //获取表面流速
            usCMD = 0x0006;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_FlowmeterData.fSurfaceVelocity;
            break;
        case 4: //获取超声波流速
            usCMD = 0x0008;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_FlowmeterData.fUltraFlowVelocity;
            break;
        case 5: //获取俯仰角
            usCMD = 0x000A;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_FlowmeterData.fRadarVertical_Angle;
            break;
        case 6: //获取翻滚角
            usCMD = 0x000C;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_FlowmeterData.fRadarRoll_Angle;
            break;
        case 7: //获取空高
            usCMD = 0x0012;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_FlowmeterData.fEmptyHeight;
            break;
        case 8: //获取瞬时流量
            usCMD = 0x0016;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_FlowmeterData.fFlowValue;
            break;
        case 9: //获取累计流量
            usCMD = 0x0018;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_FlowmeterData.fCumulativeTraffic;
            break;
        default:
            break;
        }
        ucSendBuf[i++] = HX_RADAR_ULTRASONIC_FLOW_SENSOR_ADDR;
        ucSendBuf[i++] = 0x03;
        ucSendBuf[i++] = usCMD >> 8;    //高字节
        ucSendBuf[i++] = usCMD & 0xFF; //低字节
        ucSendBuf[i++] = usRegNum >> 8; //高字节
        ucSendBuf[i++] = usRegNum & 0xFF; //低字节
        usCrc = func_Get_MBCRC16(ucSendBuf,i);
        ucSendBuf[i++] = usCrc & 0xFF;
        ucSendBuf[i++] = usCrc >> 8;

        ucResult = func_Rts_Control_And_SendData(ucDeviceType,ucSendBuf,i, 50);
        if(ucResult == 0)
        {
            pcRecvData = (char *)drv_Wait_RecvData_And_CrcCheck(ePType , usRegNum);
            if(pcRecvData != NULL)
            {
                *fMeasValue = *((float*)pcRecvData);
                ucResult = 0;
            }
            else
            {
                *fMeasValue = 0.0f;
                ucResult = 1;
                break;
            }
        }
    }
    
	return ucResult;
}

//获取恒星-截污流量计瞬时流量数据
unsigned char func_Get_Meas_HX_Flow_Sensor_Value(en_usart_device_t ucDeviceType)
{
    unsigned char ucSendBuf[10] = {0};
    float *fMeasValue = NULL;
	unsigned short usCrc = 0xFFFF;
    unsigned short usCMD = 0x0001; //获取瞬时流量值
    unsigned short usRegNum = 2;
	unsigned char i = 0;
    unsigned char j = 0;
	char *pcRecvData = NULL;
    unsigned char ucResult = 1;

    for(j=0; j<6; j++)
    {
        i = 0;
        switch (j)
        {
        case 0: //获取流速
            usCMD = 0x0000;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_FlowmeterData.fSurfaceVelocity;
            break;
        case 1: //获取液位值
            usCMD = 0x0002;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_FlowmeterData.fWaterLevel;
            break;
        case 2: //获取温度值
            usCMD = 0x0004;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_FlowmeterData.fTemperature;
            break;
        case 3: //获取电压值
            usCMD = 0x0006;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_FlowmeterData.fVoltage;
            break;
        case 4: //获取瞬时流量
            usCMD = 0x0008;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_FlowmeterData.fFlowValue;
            break;
        case 5: //获取累计流量
            usCMD = 0x000A;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_FlowmeterData.fCumulativeTraffic;
            break;
        default:
            break;
        }
        ucSendBuf[i++] = HX_FLOWMETER_SENSOR_ADDR;
        ucSendBuf[i++] = 0x03;
        ucSendBuf[i++] = usCMD >> 8;    //高字节
        ucSendBuf[i++] = usCMD & 0xFF; //低字节
        ucSendBuf[i++] = usRegNum >> 8; //高字节
        ucSendBuf[i++] = usRegNum & 0xFF; //低字节
        usCrc = func_Get_MBCRC16(ucSendBuf,i);
        ucSendBuf[i++] = usCrc & 0xFF;
        ucSendBuf[i++] = usCrc >> 8;

        ucResult = func_Rts_Control_And_SendData(ucDeviceType,ucSendBuf,i, 50);
        if(ucResult == 0)
        {
            pcRecvData = (char *)drv_Wait_RecvData_And_CrcCheck(Type_Float_Low , 2);
            if(pcRecvData != NULL)
            {
                *fMeasValue = *((float*)pcRecvData);
                ucResult = 0;
            }
            else
            {
                *fMeasValue = 0.0f;
                ucResult = 1;
                break;
            }
        }
    }
    
	return ucResult;
}

//获取恒星-COD传感器测量数值
unsigned char func_Get_Meas_HX_WaterQuality_COD_Sensor_Value(en_usart_device_t ucDeviceType, float* fValue)
{
    unsigned char ucSendBuf[10] = {0};
    float *fMeasValue = NULL;
    unsigned char *ucMeasValue = NULL;
	unsigned short usCrc = 0xFFFF;
	unsigned short usCMD = 0x0001; //获取瞬时流量值
    unsigned short usRegNum = 2;
	unsigned char i = 0;
    unsigned char j = 0;
	char *pcRecvData = NULL;
    //unsigned short usValue = 0;
    unsigned char ucResult = 1;
    //signed long slValue = 0;
    enPara_Type ePType = Type_Float_Low;

    for(j=0; j<6; j++)
    {
        i = 0;
        switch (j)
        {
        case 0: //获取传感器状态
            usCMD = 0x0000;
            ucMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_WaterQuality_CODData.ucSensorStatus;
            break;
        case 1: //获取COD测量值
            usCMD = 0x0002;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_WaterQuality_CODData.fCODValue;
            break;
        case 2: //获取浊度测量值
            usCMD = 0x0004;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_WaterQuality_CODData.fTurbidity;
            break;
        case 3: //获取COD信号值
            usCMD = 0x0006;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_WaterQuality_CODData.fCODSignalValue;
            break;
        case 4: //获取浊度信号值
            usCMD = 0x0008;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_WaterQuality_CODData.fTurbiditySignalValue;
            break;
        case 5: //获取温度值
            usCMD = 0x000A;
            fMeasValue = &pst_MBSystemPara->DeviceRunPara.esMeasData.esHX_WaterQuality_CODData.fTemperature;
            break;
        default:
            break;
        }
        if(j == 0)
        {
            ePType = Type_Char;
            usRegNum = 1;
        }
        else
        {
            ePType = Type_Float_Low;
            usRegNum = 2;
        }
        ucSendBuf[i++] = HX_WATER_QUALITY_COD_SENSOR_ADDR;
        ucSendBuf[i++] = 0x03;
        ucSendBuf[i++] = usCMD >> 8;    //高字节
        ucSendBuf[i++] = usCMD & 0xFF; //低字节
        ucSendBuf[i++] = usRegNum >> 8; //高字节
        ucSendBuf[i++] = usRegNum & 0xFF; //低字节
        usCrc = func_Get_MBCRC16(ucSendBuf,i);
        ucSendBuf[i++] = usCrc & 0xFF;
        ucSendBuf[i++] = usCrc >> 8;

        ucResult = func_Rts_Control_And_SendData(ucDeviceType,ucSendBuf,i, 50);
        if(ucResult == 0)
        {
            pcRecvData = (char *)drv_Wait_RecvData_And_CrcCheck(ePType , usRegNum);
            if(pcRecvData != NULL)
            {
                if(j == 0)
                {
                    *ucMeasValue = *((unsigned char*)pcRecvData); //传感器状态
                }
                else
                {
                    *fMeasValue = *((float*)pcRecvData);
                }
                ucResult = 0;
            }
            else
            {
                if(j == 0)
                {
                    *ucMeasValue = 100; //传感器状态
                }
                else
                {
                    *fValue = 0.0f;
                }
                ucResult = 1;
                break;
            }
        }
    }
    
	return ucResult;
}

//发送黑光图像站雨刮器动作指令
unsigned char func_Set_BlackLight_Move_AT_CMD(en_usart_device_t ucDeviceType)
{
    unsigned char ucResult = 1;
    unsigned char ucSendBuf[12] = {0};
    unsigned char ucCnt = 0;

    ucSendBuf[0] = 'A';
    ucSendBuf[1] = 'T';
    ucSendBuf[2] = '+';
    ucSendBuf[3] = 'S';
    ucSendBuf[4] = 'W';
    ucSendBuf[5] = 'A';
    ucSendBuf[6] = 'Y';
    ucSendBuf[7] = '=';
    ucSendBuf[8] = '3';
    ucSendBuf[9] = '\r';
    ucSendBuf[10] = '\n';

    ucResult = func_Rts_Control_And_SendData(ucDeviceType,ucSendBuf,11, 500);
    if(ucResult == 0)   //发送成功
    {
        while(strlen(pst_MBSystemPara->UsartData.ucUsart4RecvDataArr) == 0)
        {
            Ddl_Delay1ms(100);
            ucCnt++;
            if(ucCnt >= 50) //等待500ms
            {
                ucResult = 1; //返回异常
                break;
            }
        }
        if(strstr(pst_MBSystemPara->UsartData.ucUsart4RecvDataArr, "OK") != NULL)
        {
            ucResult = 0; //返回OK
        }
        else
        {
            ucResult = 1; //返回异常
        }
    }

    return ucResult;
}

//测试摄像机通讯协议
unsigned char func_BY_BlackLight_Camera_Test_CMD(en_usart_device_t ucDeviceType)
{
    unsigned char ucResult = 1;
    unsigned char ucSendBuf[12] = {0};
    unsigned char ucRetCheckArr[] = {0x90,0xEB,0x01,0x01,0x03,0x00,0x00,0xAA,0x55,0xF6,0xEB};

    //drv_mcu_ChangeUSART4_Baud(115200); //切换波特率到115200
    

    ucSendBuf[0] = 0x90;
    ucSendBuf[1] = 0xEB;
    ucSendBuf[2] = 0x01;
    ucSendBuf[3] = 0x01;
    ucSendBuf[4] = 0x02;
    ucSendBuf[5] = 0x00;
    ucSendBuf[6] = 0x55;
    ucSendBuf[7] = 0xAA;
    ucSendBuf[8] = 0xC1;
    ucSendBuf[9] = 0xC2;

    ucResult = func_Rts_Control_And_SendData(ucDeviceType,ucSendBuf,10, 50);
    if(ucResult == 0)   //发送成功
    {
        if(strcmp(pst_MBSystemPara->UsartData.ucUsart4RecvDataArr, (char*)ucRetCheckArr) == 0)
        {
            ucResult = 0; //返回OK
        }
        else
        {
            ucResult = 1; //返回异常
        }
    }
    return  ucResult;
}

//发送拍照命令
unsigned char func_Set_Camera_Photo_CMD(en_usart_device_t ucDeviceType)
{
    unsigned char ucResult = 1;
    unsigned char ucSendBuf[12] = {0x90,0xEB,0x01,0x40,0x04,0x00,0x00,0x02,0x05,0x01,0xC1,0xC2};
    unsigned long ulValue = 0;
    //unsigned char ucTestArr[] = {0x90,0xEB,0x01,0x01,0x03,0x00,0x00,0xAA,0x55,0xF6,0xEB};
    unsigned short usCrc = 0;
    unsigned short usCrcBase = 0;
    //usCrc = Crc16(&ucTestArr[2],9);

    pst_MBSystemPara->DeviceRunPara.st_BlackLightData.cResolution = 0x05;
    pst_MBSystemPara->DeviceRunPara.st_BlackLightData.cCompression_Radio = 0x01;

    ucResult = func_Rts_Control_And_SendData(ucDeviceType,ucSendBuf,12, 1000);
    if(ucResult == 0)   //发送成功
    {
        Ddl_Delay1ms(10);
        usCrcBase = Crc16((unsigned char*)&pst_MBSystemPara->UsartData.ucUsart4RecvDataArr[2],15);
        usCrc = (pst_MBSystemPara->UsartData.ucUsart4RecvDataArr[18] << 8) 
                | (pst_MBSystemPara->UsartData.ucUsart4RecvDataArr[17]);
        if(usCrc == usCrcBase)
        {
            ucResult = 0; //返回OK  
            ulValue = (unsigned long)(pst_MBSystemPara->UsartData.ucUsart4RecvDataArr[10] << 24
                    | (unsigned long)pst_MBSystemPara->UsartData.ucUsart4RecvDataArr[9] << 16
                    | (unsigned long)pst_MBSystemPara->UsartData.ucUsart4RecvDataArr[8] << 8
                    | (unsigned long)pst_MBSystemPara->UsartData.ucUsart4RecvDataArr[7]);
            pst_MBSystemPara->DeviceRunPara.st_BlackLightData.ulData_Size = ulValue; //获取拍照数据大小
            pst_MBSystemPara->DeviceRunPara.st_BlackLightData.ulCurGetDataSize = 0; //当前获取数据大小清零
        }
        else
        {
            ucResult = 1; //返回异常
        }
    }

    return ucResult;
}

//获取具体拍照数据
unsigned char func_Get_Photo_Data(en_usart_device_t ucDeviceType, unsigned long ulStartAddr)
{
    unsigned char ucResult = 1;
    unsigned char ucSendBuf[14] = {0x90,0xEB,0x01,0x48,0x06,0x00};
    //unsigned long ulValue = 0;
    //unsigned char ucTestArr[] = {0x90,0xEB,0x01,0x01,0x03,0x00,0x00,0xAA,0x55,0xF6,0xEB};
    unsigned short usCrc = 0;
    unsigned short usCrcBase = 0;
    unsigned short usGetDataLen = 0;

    ucSendBuf[6] = ulStartAddr & 0xFF; //低位
    ucSendBuf[7] = (ulStartAddr >> 8) & 0xFF; //次低位
    ucSendBuf[8] = (ulStartAddr >> 16) & 0xFF; //次高位
    ucSendBuf[9] = (ulStartAddr >> 24) & 0xFF; //高位
    
    if(pst_MBSystemPara->DeviceRunPara.st_BlackLightData.ulData_Size >= (1024 + pst_MBSystemPara->DeviceRunPara.st_BlackLightData.ulCurGetDataSize))
    {
        usGetDataLen = 1024; //每次获取1024字节数据
    }
    else
    {
        usGetDataLen = pst_MBSystemPara->DeviceRunPara.st_BlackLightData.ulData_Size - pst_MBSystemPara->DeviceRunPara.st_BlackLightData.ulCurGetDataSize; //获取剩余数据
    }
    //每次取1024字节数据
    ucSendBuf[10] = usGetDataLen & 0xFF; //数据长度低位
    ucSendBuf[11] = (usGetDataLen >> 8) & 0xFF; //数据长度高位
    
    ucSendBuf[12] = 0xC1; //CRC校验低位
    ucSendBuf[13] = 0xC2; //CRC校验高位

    ucResult = func_Rts_Control_And_SendData(ucDeviceType,ucSendBuf,14, 1000);
    if(ucResult == 0)   //发送成功
    {
        Ddl_Delay1ms(10);
        usCrcBase = Crc16((unsigned char*)&pst_MBSystemPara->UsartData.ucUsart4RecvDataArr[2],usGetDataLen+4);
        usCrc = (pst_MBSystemPara->UsartData.ucUsart4RecvDataArr[usGetDataLen+7] << 8) 
                | (pst_MBSystemPara->UsartData.ucUsart4RecvDataArr[usGetDataLen+6]);
        if(usCrc == usCrcBase)
        {
            ucResult = 0;
            pst_MBSystemPara->DeviceRunPara.st_BlackLightData.ulCurGetDataSize += usGetDataLen; //获取拍照数据大小
        }
    }

    return ucResult;
}

//设备外挂测量传感器(电导率，液位，流量等)通信及数据处理
void func_Meas_Sensor_Dispose(void)
{
	unsigned char i = 0;
    unsigned char j = 0;
    unsigned char l = 0;
    unsigned char k = 0;
    unsigned char ucRes = 0;
    en_usart_device_t enType = MODULE_MEAS_SENSOR1;
    MeasSensorParaSt st_MeasSensorPara[2][10];
    uint8_t ucCnt = 0;

    for(l=0; l<2; l++)
    {
        for(i=0; i<pst_MBSystemPara->DevicePara.cMeasSensorCount[l]; i++)
        {
            st_MeasSensorPara[l][i].eMeasSensor = pst_MBSystemPara->DevicePara.eMeasSensor[l][i];
            st_MeasSensorPara[l][i].cGetDataFlag = 0;
        }
    }
    
    for(j=0; j<2; j++)
    {
        //如果是第二个模块，则切换到485-2
        if(j == 1)
        {
            //enType = MODULE_MEAS_SENSOR2;
            //drv_mcu_ChangeUSART4_Source(MODULE_MEAS_SENSOR2);
            //enType = MODULE_MEAS_SENSOR1;
            //drv_mcu_ChangeUSART4_Source(MODULE_MEAS_SENSOR1);
            if((pst_MBSystemPara->DevicePara.cMeasSensorEnableFlag[1] == 1) && (pst_MBSystemPara->DevicePara.cMeasSensorCount[1] > 0))
            {
                enType = MODULE_MEAS_SENSOR2;
                drv_mcu_ChangeUSART4_Source(MODULE_MEAS_SENSOR2, 9600);
                 l = 1;
            }
            else
            {
                break; //如果第二个模块没有启用，则跳过
            }
        }
        else
        {
            if((pst_MBSystemPara->DevicePara.cMeasSensorEnableFlag[0] == 1) && (pst_MBSystemPara->DevicePara.cMeasSensorCount[0] > 0))
            {
                enType = MODULE_MEAS_SENSOR1;
                drv_mcu_ChangeUSART4_Source(MODULE_MEAS_SENSOR1, 9600);
                l = 0;
            }
            else
            {
                continue; //如果第一个模块没有启用，则跳过
            }
            //enType = MODULE_MEAS_SENSOR2;
            //drv_mcu_ChangeUSART4_Source(MODULE_MEAS_SENSOR2);
            ///enType = MODULE_MEAS_SENSOR1;
            ///drv_mcu_ChangeUSART4_Source(MODULE_MEAS_SENSOR1, 9600);
            //enType = MODULE_MEAS_SENSOR2;
           // drv_mcu_ChangeUSART4_Source(MODULE_MEAS_SENSOR2);
            //enType = MODULE_MEAS_SENSOR1;
            //drv_mcu_ChangeUSART4_Source(MODULE_MEAS_SENSOR1);
        }
        //Ddl_Delay1ms(500);
        //根据当前配置的外接设备数量，进行设备型号查询及处理
        //for(l=0; l<2; l++)
        {
            for(i=0; i<pst_MBSystemPara->DevicePara.cMeasSensorCount[l]; i++)
            {
                pst_MBSystemPara->UsartData.ucUsartxSendDataFlag[3] = 0;
                
                if(st_MeasSensorPara[l][i].cGetDataFlag == 1)
                {
                    continue; //如果已经获取过数据，则跳过
                } 
                //pst_MBSystemPara->DeviceRunPara.cGetSensorCnt++;
                //本轮采样不需要上传时，下方采集次数可为20-5=15次，需要上传时，因4G耗时，下方采样次数为3次
                //for(k=0; k<2; k++)  //返回数据错误时，最多重发三次
                {
                    switch(pst_MBSystemPara->DevicePara.eMeasSensor[l][i])
                    {
                    case Meas_BY_Integrated_Conductivity:
                        ucRes = func_Get_Meas_BY_Integrated_Conductivity_Sensor_Value(enType);
                        break;
                    case Meas_BY_Radar_Level:
                        ucRes = func_Get_Meas_BY_Radar_Level_Sensor_Value(enType);
                        break;
                    case Meas_BY_Pressure_Level:
                        ucRes = func_Get_Meas_BY_Pressure_Level_Sensor_Value(enType);
                        break;
                    case Meas_HZ_Radar_Level:
                        ucRes = func_Get_Meas_HZ_Radar_Level_Sensor_Value(enType);
                        break; 
                    case Meas_Flowmeter:
                    case Meas_HZ_Radar_Ultrasonic_Flow:
                    case Meas_HZ_Ultrasonic_Flow:
                        ucRes = func_Get_Meas_HZ_Radar_Ultrasonic_Flow_Sensor_Value(enType,pst_MBSystemPara->DevicePara.eMeasSensor[l][i] );
                    break;
                    case Meas_HX_WaterQuality_COD:
                        ucRes = func_Get_Meas_HX_WaterQuality_COD_Sensor_Value(enType,&pst_MBSystemPara->DeviceRunPara.esMeasData.fWaterQuality_CODValue);
                        break;
                    case Meas_HX_Radar_Ultrasonic_Flow:
                        ucRes = func_Get_Meas_HX_Radar_Ultrasonic_Flow_Sensor_Value(enType);
                    break;
                    case Meas_HX_Flowmeter:
                        ucRes = func_Get_Meas_HX_Flow_Sensor_Value(enType);
                        break;
                    default:
                        break;
                    }
                    memset(pst_MBSystemPara->UsartData.ucUsart4RecvDataArr,0,sizeof(pst_MBSystemPara->UsartData.ucUsart4RecvDataArr));
                    pst_MBSystemPara->UsartData.usUsartxRecvDataLen[3] = 0;
                    //drv_mcu_ChangeUSART4_Baud(9600); //切换波特率到9600
                    if(ucRes == 0)
                    {
                        st_MeasSensorPara[l][i].cGetDataFlag = 1; //获取数据成功
                        
                        ucCnt++;
                        if(ucCnt >= pst_MBSystemPara->DevicePara.cMeasSensorCount[l])
                        {
                            pst_MBSystemPara->DeviceRunPara.cGetMeasSensorValueSuccFlag = 1;
                            //增加下面这条语句会导致4G无法启动？？？？？
                            pst_MBSystemPara->DeviceRunPara.cGetMeasSensorValueFlag = 1;
                            break;
                        }
                        //k = 3; //跳出重试循环
                    }
                }
                
            }
            pst_MBSystemPara->DeviceRunPara.cGetMeasSensorValueFlag = 1;
        }
    }
}

//黑光图像站处理
unsigned char func_BlackLight_Sensor_Dispose(void)
{
    unsigned char i = 0;
    unsigned char j = 0;
    unsigned char ucRes = 0;
    static en_usart_device_t enCameraType = MODULE_MEAS_SENSOR1;
    static en_usart_device_t enMoveType = MODULE_MEAS_SENSOR2;

    //针对黑光图像站，通讯需要做特殊处理:
    //1. 先发测试命令，确认摄像机通讯正常及摄像机对应的串口
    drv_mcu_ChangeUSART4_Source(enCameraType, 115200);
    for(i=0; i<3; i++)
    {
        ucRes = func_BY_BlackLight_Camera_Test_CMD(enCameraType);
        if(ucRes == 0)
        {
            break;
        }
    }   
    if(ucRes == 1)  //说明摄像机通讯异常或者摄像机对应的串口是串口2
    {
        //如果是串口1，则切换到串口2
        if(enCameraType == MODULE_MEAS_SENSOR1)
        {
            enCameraType = MODULE_MEAS_SENSOR2;   
        }
        else
        {
            enCameraType = MODULE_MEAS_SENSOR1;
        }
        drv_mcu_ChangeUSART4_Source(enCameraType, 115200);
        for(i=0; i<3; i++)
        {
            ucRes = func_BY_BlackLight_Camera_Test_CMD(enCameraType);
            if(ucRes == 0)
            {
                break;
            }
        }
        if(ucRes == 0) //说明摄像机通讯正常
        {
            pst_MBSystemPara->DeviceRunPara.usDevStatus &= 0xFFDF; //清除异常状态
        }
        else
        {
            pst_MBSystemPara->DeviceRunPara.cModbusErrCnt++;
            if(pst_MBSystemPara->DeviceRunPara.cModbusErrCnt > 20)
            {
                pst_MBSystemPara->DeviceRunPara.cModbusErrCnt = 0;
                pst_MBSystemPara->DeviceRunPara.usDevStatus |= 0x0020; //设置设备异常状态
            }
            //pst_MBSystemPara->DeviceRunPara.usDevStatus |= 0x0020; //设置异常状态
            return 1;
        }
    }

    if(enCameraType == MODULE_MEAS_SENSOR1)
    {
        enMoveType = MODULE_MEAS_SENSOR2; //设置移动指令对应的串口为串口2
    }
    else 
    {
        enMoveType = MODULE_MEAS_SENSOR1; //设置移动指令对应的串口为串口1
    }
    pst_MBSystemPara->UsartData.ucUsartxRecvDataFlag[3] = 0;
    memset(pst_MBSystemPara->UsartData.ucUsart4RecvDataArr,0,sizeof(pst_MBSystemPara->UsartData.ucUsart4RecvDataArr));
    //发送移动指令,避免第一次操作无反应，操作2次
    drv_mcu_ChangeUSART4_Source(enMoveType, 9600);
    for(j=0; j<2; j++)
    {
        ucRes = func_Set_BlackLight_Move_AT_CMD(enMoveType);
        if(ucRes == 0)
        {
            break;
        }
    }
    pst_MBSystemPara->UsartData.ucUsartxRecvDataFlag[3] = 0;
    memset(pst_MBSystemPara->UsartData.ucUsart4RecvDataArr,0,sizeof(pst_MBSystemPara->UsartData.ucUsart4RecvDataArr));

    //发送拍照命令
    drv_mcu_ChangeUSART4_Source(enCameraType, 115200);
    ucRes = func_Set_Camera_Photo_CMD(enCameraType);
    pst_MBSystemPara->UsartData.ucUsartxRecvDataFlag[3] = 0;
    memset(pst_MBSystemPara->UsartData.ucUsart4RecvDataArr,0,sizeof(pst_MBSystemPara->UsartData.ucUsart4RecvDataArr));
    if(ucRes == 0)  //接收设备返回的拍照结束及照片大小数据
    {
        //分批读取设备拍照数据
        if(pst_MBSystemPara->DeviceRunPara.st_BlackLightData.ulData_Size > 0)
        {
            //pst_MBSystemPara->DeviceRunPara.st_BlackLightData.ulCurGetDataSize = 0; //清除当前获取数据大小
            while(pst_MBSystemPara->DeviceRunPara.st_BlackLightData.ulCurGetDataSize < pst_MBSystemPara->DeviceRunPara.st_BlackLightData.ulData_Size)
            {
                pst_MBSystemPara->UsartData.ucUsartxRecvDataFlag[3] = 0;
                memset(pst_MBSystemPara->UsartData.ucUsart4RecvDataArr,0,sizeof(pst_MBSystemPara->UsartData.ucUsart4RecvDataArr));
                ucRes = func_Get_Photo_Data(enCameraType, pst_MBSystemPara->DeviceRunPara.st_BlackLightData.ulCurGetDataSize);
                if(ucRes != 0) //如果获取数据异常，则退出
                {
                    break;
                }
            }
        }
    }
    return ucRes;
}

/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
