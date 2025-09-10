/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\driver\drv_device\drv_LKT4202\drv_LKT4202.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-21       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "drv_LKT4202.h"
#include "stdio.h"
#include "string.h"
#include "drv_IIC.h"

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

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/


uint8_t drv_LKT4202_Create_MFFile(void)
{
	uint8_t ucRes = 0;
	//char ucSendBuf[30] = "80E03F000D38FFFFF0F0FFFFFFFFFFFFFFFF";
	char ucSendBuf[30] = {0x00,0x14,0x80,0xE0,0x3F,0x00,0x0D,0x38,0xFF,0xFF,0xF0,0xF0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	char ucRecvBuf[10] = {0};
	char cCheckBuf[2] = {0x90,0x00};
	uint16_t usRecvLen = 0;
	unsigned short usPosition = 0;

	drv_SM4_IIC_Reset();
	ucRes = drv_SM4_IIC_SendApdu(20,ucSendBuf,ucRecvBuf,&usRecvLen);

	if(ucRes == 0)
	{
		ucRes = func_Array_Find_Str(ucRecvBuf+2, usRecvLen, cCheckBuf, 2, &usPosition);
	}
	return ucRes;
}

uint8_t drv_LKT4202_Create_KeyFile(void)
{
	uint8_t ucRes = 0;
	//char ucSendBuf[30] = "80E03F000D38FFFFF0F0FFFFFFFFFFFFFFFF";
	char ucSendBuf[30] = {0x00,0x0C,0x80,0xE0,0x00,0x00,0x07,0x3F,0x00,0x50,0xFF,0xF0,0xFF,0xFF};
	char ucRecvBuf[10] = {0};
	char cCheckBuf[2] = {0x90,0x00};
	uint16_t usRecvLen = 0;
	unsigned short usPosition = 0;

	drv_SM4_IIC_Reset();	
	ucRes = drv_SM4_IIC_SendApdu(14,ucSendBuf,ucRecvBuf,&usRecvLen);

	if(ucRes == 0)
	{
		ucRes = func_Array_Find_Str(ucRecvBuf+2, usRecvLen, cCheckBuf, 2, &usPosition);
	}
	return ucRes;
}
/**
*******************************************************************************
** \brief LKT4202 IIC Init function
**
** \param [in]  None
**
** \retval 0: init success; 1: init failed.
**
** \note 
******************************************************************************/
uint8_t drv_LKT4202_Init(void)
{
	uint8_t ucRes = 0;

	ucRes = drv_mcu_SM4_IIC_Init();

	ucRes = drv_LKT4202_Create_MFFile();

	ucRes = drv_LKT4202_Create_KeyFile();

	return ucRes;
}



/**
*******************************************************************************
** \brief send 16 bytes key to lkt4202
**
** \param [in]  None
**
** \retval 0: init success; 1: init failed.
**
** \note 写入加密秘钥
******************************************************************************/
uint8_t drv_LKT4202_Send_EncryKEY(uint8_t* cKeyBuf)
{
	uint8_t ucRes = 0;
	char ucSendBuf[30] = {0};
	char ucRecvBuf[10] = {0};
	char ucCmdBuf[10] = {0x80,0xD4,0x01,0x02,0x15,0x30,0xF0,0xFA,0x01,0x04};
	char cCheckBuf[2] = {0x90,0x00};
	uint16_t usRecvLen = 0;
	unsigned short usPosition = 0;

	memcpy(ucSendBuf,"\x00\x1C",2);
	memcpy(ucSendBuf+2,ucCmdBuf,10);
	memcpy(ucSendBuf+12,cKeyBuf,16);

	drv_SM4_IIC_Reset();
	ucRes = drv_SM4_IIC_SendApdu(28,ucSendBuf,ucRecvBuf,&usRecvLen);

	if(ucRes == 0)
	{
		ucRes = func_Array_Find_Str(ucRecvBuf+2, usRecvLen, cCheckBuf, 2, &usPosition);
	}
	return ucRes;
}

/**
*******************************************************************************
** \brief send 16 bytes key to lkt4202
**
** \param [in]  None
**
** \retval 0: init success; 1: init failed.
**
** \note 写入解密秘钥
******************************************************************************/
uint8_t drv_LKT4202_Send_DecryKEY(uint8_t* cKeyBuf)
{
	uint8_t ucRes = 0;
	char ucSendBuf[30] = {0};
	char ucRecvBuf[10] = {0};
	char ucCmdBuf[10] = {0x80,0xD4,0x01,0x02,0x15,0x31,0xF0,0xFA,0x01,0x04};
	char cCheckBuf[2] = {0x90,0x00};
	uint16_t usRecvLen = 0;
	unsigned short usPosition = 0;

	memcpy(ucSendBuf,"\x00\x1C",2);
	memcpy(ucSendBuf+2,ucCmdBuf,10);
	memcpy(ucSendBuf+12,cKeyBuf,16);

	drv_SM4_IIC_Reset();
	ucRes = drv_SM4_IIC_SendApdu(28,ucSendBuf,ucRecvBuf,&usRecvLen);

	if(ucRes == 0)
	{
		ucRes = func_Array_Find_Str(ucRecvBuf+2, usRecvLen, cCheckBuf, 2, &usPosition);
	}
	return ucRes;
}

//发送加密数据
uint8_t drv_LKT4202_SendData_Encry(uint8_t *uBaseDataArr, char *uEnDataArr)
{
	uint8_t ucRes = 0;
	char ucSendBuf[30] = {0};
	char cCMDBuf[5] = {0x00,0x88,0x00,0x02,0x10};
	uint16_t usRecvLen = 0;
	char cCheckBuf[2] = {0x90,0x00};
	char ucRecvBuf[20] = {0};
	unsigned short usPosition = 0;

	memcpy(ucSendBuf,"\x00\x15",2);
	memcpy(ucSendBuf+2,cCMDBuf,5);
	memcpy(ucSendBuf+7,uBaseDataArr,16);

	drv_SM4_IIC_Reset();
	ucRes = drv_SM4_IIC_SendApdu(23,ucSendBuf,ucRecvBuf,&usRecvLen);

	if(ucRes == 0)
	{
		ucRes = func_Array_Find_Str(ucRecvBuf+2, usRecvLen, cCheckBuf, 2, &usPosition);
		if(ucRes == 0)
		{
			memcpy(uEnDataArr, ucRecvBuf+2, 16);
		}
	}
	return ucRes;
}	

//获取解密数据
uint8_t drv_LKT4202_SendData_Decry(uint8_t *uBaseDataArr, char *uDeDataArr)
{
	uint8_t ucRes = 0;
	char ucSendBuf[30] = {0};
	char cCMDBuf[5] = {0x00,0x88,0x01,0x02,0x10};
	uint16_t usRecvLen = 0;
	char ucRecvBuf[20] = {0};
	char cCheckBuf[2] = {0x90,0x00};
	unsigned short usPosition = 0;

	memcpy(ucSendBuf,"\x00\x15",2);
	memcpy(ucSendBuf+2,cCMDBuf,5);
	memcpy(ucSendBuf+7,uBaseDataArr,16);

	drv_SM4_IIC_Reset();
	ucRes = drv_SM4_IIC_SendApdu(23,ucSendBuf,ucRecvBuf,&usRecvLen);

	if(ucRes == 0)
	{
		ucRes = func_Array_Find_Str(ucRecvBuf+2, usRecvLen, cCheckBuf, 2, &usPosition);
		if(ucRes == 0)
		{
			memcpy(uDeDataArr, ucRecvBuf+2, 16);
		}
	}
	return ucRes;
}	

uint8_t drv_LKT4202_Random_Test(void)
{
	uint8_t ucRes = 0;
	char ucSendBuf[30] = {0};
	char ucRecvBuf[20] = {0};
	uint16_t usRecvLen = 0;
	char ucCheckBuf[2] = {0};
	unsigned short usPosition = 0;

	memcpy(ucSendBuf,"\x00\x05\x00\x84\x00\x00\x08",0x07); 
	memcpy(ucCheckBuf,"\x90\x00",0x02);

	drv_SM4_IIC_Reset();
	ucRes = drv_SM4_IIC_SendApdu(0x07,ucSendBuf,ucRecvBuf,&usRecvLen);

	if(ucRes == 0)
	{
		ucRes = func_Array_Find_Str(&ucRecvBuf[2], usRecvLen, ucCheckBuf, 2, &usPosition);
	}
	return ucRes;
}

/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
