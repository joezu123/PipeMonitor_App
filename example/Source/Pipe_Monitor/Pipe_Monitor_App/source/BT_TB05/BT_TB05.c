/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\BT_TB05\BT_TB05.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-04-03       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "BT_TB05.h"
#include "hc32f460_utility.h"
#include "ModbusRTU.h"
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
static SystemPataSt *pst_BTSystemPara;
char g_Ble_SSID[20] = "SK_";              //蓝牙ssid前缀
static char g_AT_SSID_CMD[35] = "AT+BLENAME=";     //蓝牙ssid配置AT指令
static char g_AT_RESET_CMD[30] = "AT+RST";        //蓝牙软件复位配置AT指令   
static char g_AT_CMD[4] = "AT";             //蓝牙AT指令
static char g_ATE_CMD[6] = "ATE0";           //蓝牙AT指令
//static char g_AT_BLEMTU[13] = "AT+BLEMTU=200"; //蓝牙MTU查询AT指令
//static char g_AT_MTU_CMD[10] = "AT+BLEMTU?"; //蓝牙MTU配置AT指令
//static char g_AT_GETUUID[14] = "AT+BLESERUUID?"; //蓝牙UUID查询AT指令
//static char g_AT_GETNAME_CMD[30] = "AT+BLENAME?";        //蓝牙ssid查询AT指令
/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
//通过串口将数据发送到蓝牙模块
void drv_BT05_SendATCMD(uint8_t *ucDataArr, uint16_t usDataLen)
{
	int len = strlen( (char*)ucDataArr );
    uint8_t cTemp[50];
    
    memcpy( cTemp, ucDataArr,len );

    cTemp[len] = '\r';
    cTemp[len+1] = '\n';

	memset(pst_BTSystemPara->UsartData.ucUsartxRecvDataArr[1], 0, USART_DATA_LEN_MAX);
	drv_mcu_USART_SendData(MODULE_BT, cTemp, usDataLen+2);
}

void drv_BT05_SendData(uint8_t *ucDataArr, uint16_t usDataLen)
{
	drv_mcu_USART_SendData(MODULE_BT, ucDataArr, usDataLen);
}


uint8_t HexToChar(uint8_t temp)
{
    uint8_t dst;
    if (temp < 10)
	{
        dst = temp + '0';
    }
	else
	{
        dst = temp -10 +'A';
    }
    return dst;
}

//蓝牙模块初始化AT指令配置
void drv_BT05_InitATCMD(void)
{
	uint8_t i = 0;

	//Ddl_Delay1ms(500);
	//drv_BT05_SendATCMD((uint8_t *)g_AT_RESTORE_CMD,10);
	drv_BT05_SendATCMD((uint8_t *)g_AT_RESET_CMD,6);
	Ddl_Delay1ms(1500);
	//memset(pst_BTSystemPara->UsartData.ucUsartxRecvDataArr[1], 0, USART_DATA_LEN_MAX);

	//drv_BT05_SendATCMD((uint8_t *)g_AT_CMD,2);
	//Ddl_Delay1ms(500);
	drv_BT05_SendATCMD((uint8_t *)g_AT_CMD,2);
	Ddl_Delay1ms(500);
	//memset(pst_BTSystemPara->UsartData.ucUsartxRecvDataArr[1], 0, USART_DATA_LEN_MAX);
	drv_BT05_SendATCMD((uint8_t *)g_ATE_CMD,4);
	Ddl_Delay1ms(500);
	//memset(pst_BTSystemPara->UsartData.ucUsartxRecvDataArr[1], 0, USART_DATA_LEN_MAX);
	//drv_BT05_SendATCMD((uint8_t *)g_AT_MTU_CMD,10);
	//Ddl_Delay1ms(500);
	//drv_BT05_SendATCMD((uint8_t *)g_AT_BLEMTU,13);
	//Ddl_Delay1ms(500);
	//drv_BT05_SendATCMD((uint8_t *)g_AT_MTU_CMD,10);
	//Ddl_Delay1ms(500);

	//drv_BT05_SendATCMD((uint8_t *)g_AT_GETNAME1_CMD,8);
	//Ddl_Delay1ms(500);
	//drv_BT05_SendATCMD((uint8_t *)g_AT_GETNAME_CMD,11);
	//Ddl_Delay1ms(500);
	//memset(pst_BTSystemPara->UsartData.ucUsartxRecvDataArr[1], 0, USART_DATA_LEN_MAX);
	if((pst_BTSystemPara->DevicePara.cDeviceID[0] == 0x00) && (pst_BTSystemPara->DevicePara.cDeviceID[1] == 0x00))
	{
		memcpy(pst_BTSystemPara->DevicePara.cDeviceID, "1234567890ABCDEF", 16);
	}
	/* 设置蓝牙SSID */
	for(i=2; i<18; i++)
	{
		g_Ble_SSID[i+1] = pst_BTSystemPara->DevicePara.cDeviceID[i-2];
	}
	g_Ble_SSID[19] = 0;
	
	strcat( g_AT_SSID_CMD, g_Ble_SSID );//"SK_3241ab"
	drv_BT05_SendATCMD((uint8_t *) g_AT_SSID_CMD ,30);
	//Ddl_Delay1ms(500);
	//memset(pst_BTSystemPara->UsartData.ucUsartxRecvDataArr[1], 0, USART_DATA_LEN_MAX);
	//drv_BT05_SendATCMD((uint8_t *)g_AT_GETUUID,14);
	//Ddl_Delay1ms(500);
	//memset(pst_BTSystemPara->UsartData.ucUsartxRecvDataArr[1], 0, USART_DATA_LEN_MAX);
}

//蓝牙模块初始化
uint8_t drv_BT05_Module_Init(void)
{
	stc_port_init_t stcPortInit;

	MEM_ZERO_STRUCT(stcPortInit);

	stcPortInit.enPinMode = Pin_Mode_Out; 

    
	PORT_Init(BTRST_GPIO_PORT, BTRST_GPIO_PIN, &stcPortInit);

	//开启供电
	PWRBLE_PIN_OPEN();
	Ddl_Delay1ms(100);

	//复位模块
	BTRST_PIN_RESET;
	Ddl_Delay1ms(100);
	BTRST_PIN_SET;
	Ddl_Delay1ms(100);

	//Ddl_Delay1ms(10000);

	pst_BTSystemPara = GetSystemPara();

	drv_BT05_InitATCMD();
	
	return 0;
}

//蓝牙模块上电及复位操作
void func_BT05_PowerUp_Init(void)
{
	uint8_t i = 0;
	stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Out;
    PORT_Init(BTRST_GPIO_PORT, BTRST_GPIO_PIN, &stcPortInit);

	//开启供电
	PWRBLE_PIN_OPEN();
	Ddl_Delay1ms(100);

	//复位模块
	BTRST_PIN_RESET;
	Ddl_Delay1ms(100);
	BTRST_PIN_SET;
	Ddl_Delay1ms(100);
	pst_BTSystemPara->DeviceRunPara.cDeviceBTPowerOnFlag =  1;

	drv_BT05_SendATCMD((uint8_t *)g_AT_RESET_CMD,6);
	Ddl_Delay1ms(1500);
	//drv_BT05_SendATCMD((uint8_t *)g_AT_CMD,2);
	//Ddl_Delay1ms(500);
	//drv_BT05_SendATCMD((uint8_t *)g_AT_CMD,2);
	//Ddl_Delay1ms(200);
	drv_BT05_SendATCMD((uint8_t *)g_ATE_CMD,4);
	Ddl_Delay1ms(200);
	if((pst_BTSystemPara->DevicePara.cDeviceID[0] == 0x00) && (pst_BTSystemPara->DevicePara.cDeviceID[1] == 0x00))
	{
		memcpy(pst_BTSystemPara->DevicePara.cDeviceID, "1234567890ABCDEF", 16);
	}
	/* 设置蓝牙SSID */
	for(i=2; i<18; i++)
	{
		g_Ble_SSID[i+1] = pst_BTSystemPara->DevicePara.cDeviceID[i-2];
	}
	g_Ble_SSID[19] = 0;
	
	strcat( g_AT_SSID_CMD, g_Ble_SSID );//"SK_3241ab"
	drv_BT05_SendATCMD((uint8_t *) g_AT_SSID_CMD ,30);
	//Ddl_Delay1ms(500);
}

void func_BT05_PowerDown_DeInit(void)
{
	stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Ana;
	PORT_Init(BTRST_GPIO_PORT, BTRST_GPIO_PIN, &stcPortInit);
	pst_BTSystemPara->DeviceRunPara.cDeviceBTPowerOnFlag = 0;
	PWRBLE_PIN_CLOSE();	//关闭蓝牙模块电源
}

unsigned char guc_ERR13DelayCnt = 0;
//BT通信处理程序
void func_BT_Dispose(void)
{
	static unsigned short usSendLen = 0;	//发送数据长度
	static unsigned short usSendMaxNum = 0;
	unsigned short usPosi = 0;
	uint8_t ucValueArr[200] = {0};	//发送数据数组

	if(pst_BTSystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_BT] == 1)  //蓝牙模块收到数据
	{
		if(strstr(pst_BTSystemPara->UsartData.ucUsartxRecvDataArr[1],"BLE_CONNECT") != NULL)
		{
			pst_BTSystemPara->DeviceRunPara.cDeviceBTConnectFlag = 1;
			pst_BTSystemPara->DeviceRunPara.usDeviceBTWaitCnt = 0;
			if(pst_BTSystemPara->DeviceRunPara.cTimer4StartFlag == 0)
			{
				pst_BTSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_DIAG;
				//drv_mcu_Timer4_Start();
			}
		}
		else if(strstr(pst_BTSystemPara->UsartData.ucUsartxRecvDataArr[1],"BLE_DISCONNECT") != NULL)
		{
			pst_BTSystemPara->DeviceRunPara.cDeviceBTConnectFlag = 0;
		}
		else
		{
			if(pst_BTSystemPara->DeviceRunPara.cDeviceBTConnectFlag == 1)
			{
				//drv_BT05_SendData((uint8_t *)"OK",2);
				#if 0
				pst_MainloopSystemPara->DeviceRunPara.usBTRecValue = (pst_MainloopSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BT][0] - 0x30) * 100 \
					+ (pst_MainloopSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BT][1] - 0x30) * 10 \
					+ (pst_MainloopSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BT][2] - 0x30);
				#else
				#if 0
				pst_MainloopSystemPara->DeviceRunPara.usBTRecValue = pst_MainloopSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BT][0];
				#else
				usSendMaxNum = 0;
				//memcpy(ucTestArr+ucPosi, pst_MainloopSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BT], pst_MainloopSystemPara->UsartData.usUsartxRecvDataLen[MODULE_BT]);
				//ucPosi += pst_MainloopSystemPara->UsartData.usUsartxRecvDataLen[MODULE_BT];
				//if(ucPosi >= 100)
				//{
				//	ucPosi = 0;
				//}
				eMBErrorCode eStatus = eMBDMASendRevic((unsigned char*)pst_BTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BT], 
					ucValueArr,
					pst_BTSystemPara->UsartData.usUsartxRecvDataLen[MODULE_BT],
					&usSendMaxNum);
				if(eStatus != MB_ENOERR)
				{
					guc_ERR13DelayCnt++;
					if(guc_ERR13DelayCnt >= 10)
					{
						pst_BTSystemPara->DeviceRunPara.usDevStatus |= 0x0040;	//BT通讯异常
					}
				}
				else
				{
					guc_ERR13DelayCnt = 0;
					pst_BTSystemPara->DeviceRunPara.usDevStatus &= 0xfb;
				}
				#endif
				#endif
			}
			else
			{
				pst_BTSystemPara->DeviceRunPara.usBTRecValue = 0;
			}
		}
		
		memset(pst_BTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BT], 0, USART_DATA_LEN_MAX);
		pst_BTSystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_BT] = 0;
	} 

	if(usSendMaxNum > 0)
	{
		//ucPosi = 0;
		//memset(ucTestArr, 0, sizeof(ucTestArr));
		//每次蓝牙发送的最大字节数为20，。超过20个字节，小程序接收异常。所以当数据量大时，应分包发送

		//sprintf((char*)ucValueArr,"%d",pst_MainloopSystemPara->DeviceRunPara.usBTRecValue+10);
		//ucValueArr[0] = 0x01;
		//ucValueArr[1] = 0x02;
		//ucValueArr[2] = 0x03;
		#if 0
		//memcpy((char*)&ucValueArr,pst_MainloopSystemPara->DevicePara.cDeviceID,strlen((char*)pst_MainloopSystemPara->DevicePara.cDeviceID));
		ucValueArr[0] = 0xF0;
		ucValueArr[1] = 0x03;
		ucValueArr[2] = 0x10;
		ucValueArr[3] = 0x4D;
		ucValueArr[4] = 0x51;
		ucValueArr[5] = 0x30;
		ucValueArr[6] = 0x31;
		ucValueArr[7] = 0x30;
		ucValueArr[8] = 0x30;
		ucValueArr[9] = 0x30;
		ucValueArr[10] = 0x30;
		ucValueArr[11] = 0x30;
		ucValueArr[12] = 0x34;
		ucValueArr[13] = 0x30;
		ucValueArr[14] = 0x30;
		ucValueArr[15] = 0x30;
		ucValueArr[16] = 0x31;
		ucValueArr[17] = 0x31;
		ucValueArr[18] = 0x31;
		ucValueArr[19] = 0xEF;
		ucValueArr[20] = 0XBE;
		drv_BT05_SendData(ucValueArr,21);
		#else
		#if 0
		if(usSendMaxNum > 20)
		{
			usSendLen = 20;
		}
		else
		{
			usSendLen = usSendMaxNum;
		}
		usPosi = usSendLen;
		usSendMaxNum -= usSendLen;
		drv_BT05_SendData(ucValueArr,usSendLen);
		while(usSendMaxNum > 0)
		{
			Ddl_Delay1ms(100);
			if(usSendMaxNum > 20)
			{
				usSendLen = 20;
			}
			else
			{
				usSendLen = usSendMaxNum;
			}
			usSendMaxNum -= usSendLen;
			drv_BT05_SendData(ucValueArr+usPosi,usSendLen);
			usPosi += usSendLen;
		}
		#else
		drv_BT05_SendData(ucValueArr,usSendMaxNum);
		usSendMaxNum = 0;
		#endif
		#endif
	}
}

/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
