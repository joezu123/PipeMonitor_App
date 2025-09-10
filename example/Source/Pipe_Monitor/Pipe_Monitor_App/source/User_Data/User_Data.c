/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\User_Data\User_Data.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-13       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "User_Data.h"
#include "string.h"
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
static SystemPataSt gSt_systemPara;
unsigned char guc_OLED_Buf[128][8] = {0};	//OLED显示数据缓存; 8*8=64行128列
unsigned char  picc_atqa[2],picc_uid[15],picc_sak[3];
unsigned char guc_TextBDData[20] = {0};	//北斗模块接收数据缓存
unsigned char guc_NFCPWRInitFlag = 0;	//NFC模块开启电源及初始化配置标志位
unsigned char guc_SystemPowerInitFlag = 0;	//板级各模块电源开启及复位标志位
//unsigned char guc_RTC_TimeOutFlag = 0;	//RTC 定时中断标志位，1min置位一次
unsigned char guc_LcdDipRevesAttr = 0 ;	//显示字符反白显示标志位: 1->反白显示； 0->正常显示
unsigned char guc_NFC_Card_Flag = 0;	//NFC识别正确标志位: 1->正确; 0->非法
unsigned char guc_RTURecvArr[20] = {0};	//Modbus RTU接收数据缓存
char gc_SystemPosi = 0;
DevMeasRecordDataSt gSt_DevMeasRecordData = {0};	//设备测量数据记录结构体

SysDeviceParaSt gs_DeviceDefaultPara = 
{
	//char cDeviceID[16];	//设备ID:MQ+4位厂商代码+4位客户端类型+6位自定义字符串
	{'M','Q','0','1','0','0','0','0','0','4','0','0','0','0','0','3',0},			
	{0},						//char cDeviceIdenFlag;	//设备是否经过NFC认证标志位	0->未验证
	{0},						//char cDeviceRegisterFlag;	//设备是否在服务器通讯上注册标志位	0->未注册
	{0,0,0},					//char cDeviceIMSI[15];	//设备IMSI号；是一个用于在全球范围内唯一地识别移动用户的标识符。IMSI存储在手机的SIM卡中，用于在蜂窝网络中进行用户身份验证和位置管理。
	{0,0,0},					//char cDeviceIMEI[15];	//设备IMEI号；通常所说的手机序列号、手机“串号”，用于在移动电话网络中识别每一部独立的手机等移动通信设备，相当于移动电话的身份证。序列号共有15~17位数字
	{'1','.','0','.','1'},	//char cDeviceHWVersion[10];	//设备硬件版本号
	{'1','.','0','.','3','S','K'},	//char cDeviceSWVersion[10];	//设备软件版本号
	{'2','5','-','0','7','-','1','0'},	//char cDevicePDDate[10];		//设备生产日期: YY-MM-DD
	{5},						//int nDeviceSampleGapCnt;	//设备设置的采样间隔时间，单位Min，注意通讯协议上的单位为s,要注意时间转换
	{5},						//int nDeviceSaveRecordCnt;	//设备设置的保存记录时间，单位Min，注意通讯协议上的单位为s,要注意时间转换
	{15},						//int nDeviceUploadCnt;		//设备设置的上传间隔时间，单位Min，注意通讯协议上的单位为s,要注意时间转换
	{0},						//int nDeviceRecordCnt;	//设备历史数据记录计数
	//{
	//	{0},					//int nChannelID;	//通道序号
	//	{0,0,0},				//char cIPAddr[20];	//IP字符串，支持域名
	//	{0},					//int nPort;			//端口
	//	{'M','Q','T','T'},		//char cProcArr[10];	//UPD/TCP/COAP/MQTT
	//	{1},					//int nAppProc;		//1:本协议
	//	{5},					//int nDeviceUploadRecordCnt;	//设备设置的上传记录时间，单位Min，注意通讯协议上的单位为s,要注意时间转换
	//	{1},					//int nDelayTime;		//延时时间，单位min					
	//},							//ServerParaSt esServerPara;	//服务器上发参数
	{0,0,0},					//char cMQTT_UserNameArr[20];	//登录用户名
	{0,0,0},					//char cMQTT_UserPWDArr[20];	//登录密码
	{60},						//char cHeartGap;				//心跳间隔，单位s
	{1},						//char cDps;					//离散估长	单位s
	//{0,0,0},					//char cAPNArr[20];			//接入点名称
	//{0,0,0},					//char cVPDNUserNameArr[20];	//VPDN用户名
	//{0,0,0},					//char cVPDNUserPWDArr[20];	//VPDN密码
	//{
	//	{1},					//char cAutoIP;	//0->关闭自动获取IP; 1->自动获取IP
	//	{0,0,0},				//char cLocAddrArr[20];	//本地地址
	//	{0,0,0},				//char cMaskArr[20];		//子网掩码
	//	{0,0,0},				//char cGatewayArr[20];	//网关地址
	//	{0,0,0},				//char cDNSArr[20];		//DNS服务器
	//},							//EthParaSt esEthPara;		//以太网参数
	{1},						//char cParaVersion;			//设备参数版本号
	{0,0},						//char cMeasSensorEnableFlag[2];	//两路485使能开关
	{0,0},						//char cMeasSensorCount;		//设备外接测量传感器数量
	{
		{
			{Meas_NULL},
			{Meas_NULL},
			{Meas_NULL},
		},
		{
			{Meas_NULL},
			{Meas_NULL},
			{Meas_NULL},
		}
	},							//EMeasSensorType eMeasSensor[4];	//设备外接测量传感器设备型号,可配置
	{0.0},						//float fTotal_Volume;	//设备工作累计流量，单位m3
	{10.0},						//float fInit_Height;		//设备安装时井深，用于计算液位高度，单位m，该值减去雷达液位计的空高值即为液位高度
	{24},						//unsigned char ucUploadStatusGap;	//设备状态上传间隔时间，单位h
	{
		{'2','1','8','.','8','5','.','5','.','1','6','1',0},
		{'2','2','0','.','2','5','0','.','2','9','.','1','8','8',0}
	},	//char cServerIP[2][16];	//设备连接物联网平台IP地址，共2组
	{7243,7183},				//unsigned short usServerPort[2];	//设备连接物联网平台端口，共2组
	{0},						//char cMonitorMode;	//备用
	{0},						// char cSensorBaudRate;	//设备485波特率; 0->9600; 1->2400; 2->4800; 3->19200; 4->38400; 5->115200
	{0},						//char cBackUpArr[50];	//备用数据数组，长度50字节
	{EEP_VERSION}				//short sEEP_Version;			//存储版本号
};
//unsigned short gus_BarCnt = 0;
//unsigned short gus_BarCnt1 = 0;
//unsigned char guc_NFC_USART3_RecvCnt = 0;
//unsigned char guc_NFC_USART3_RecvData[5][10] = {0};	//NFC USART3接收数据缓存
//unsigned char guc_NFC_GetDataCnt = 0;
//unsigned char guc_NFC_GetData[5][10] = {0};	//NFC USART3接收数据缓存
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
SystemPataSt * GetSystemPara(void)
{
	return &gSt_systemPara;
}

void func_SystemPara_Init(void)
{
	memset((char*)&gSt_systemPara,0,sizeof(SystemPataSt));
}

/*十进制转BCD码*/
int decimal_bcd_code(int decimal)
{
	int sum = 0, i;//i计数变量，sum返回的BCD码
	for ( i = 0; decimal > 0; i++)
	{
		sum |= ((decimal % 10 ) << ( 4*i));
		decimal /= 10;
	}
	return sum;
}

/*BCD码转十进制*/
int bcd_decimal_code( int bcd)
{
	int sum = 0, c = 1;  // sum返回十进制，c每次翻10倍

	for(int i = 1; bcd > 0; i++)
	{
		if( i >= 2)
		{
			c*=10;
		}

		sum += (bcd%16) * c;

		bcd /= 16;  // 除以16同理与十进制除10将小数点左移一次，取余16也同理
	}

	return sum;
}


//判断接收的数据数组中是否包含指定的字符串
//0->包含; 1->不包含
unsigned char func_Array_Find_Str(char *ucRecvBuf, unsigned short usDataLen, char *ucCheckBuf, unsigned short usCheckLen,unsigned short *usPosi)
{
	unsigned short i = 0;
	unsigned char ucRes = 1;
	*usPosi = 0xFFFF;	//未找到
	for(i = 0; i < usDataLen; i++)
	{
		if(ucRecvBuf[i] == ucCheckBuf[0])
		{
			if(memcmp(ucRecvBuf+i, ucCheckBuf, usCheckLen) == 0)
			{
				*usPosi = i + usCheckLen + 1;	// 找到
				ucRes = 0;	// 找到
				break;
			}
		}
	}

	return ucRes;
}

//查找字符串在数组中的位置
unsigned short func_Array_Find_Str_Pos(char *ucRecvBuf, unsigned short usDataLen, char *ucCheckBuf, unsigned short usCheckLen)
{
	unsigned short i = 0;
	unsigned short ucRes = 0xFFFF;	//未找到
	for(i = 0; i < usDataLen; i++)
	{
		if(ucRecvBuf[i] == ucCheckBuf[0])
		{
			if(memcmp(ucRecvBuf+i, ucCheckBuf, usCheckLen) == 0)
			{
				ucRes = i + usCheckLen + 1;	// 找到
				break;
			}
		}
	}

	return ucRes;
}
/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
