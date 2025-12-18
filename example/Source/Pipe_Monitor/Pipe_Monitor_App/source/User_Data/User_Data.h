/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\User_Data\User_Data.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-14       Joe             First version
 @endverbatim

 */
#ifndef __USER_DATA_H__
#define __USER_DATA_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files	
 ******************************************************************************/

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define HW_VERSION_V1_1	1	//硬件V1.1版本

#ifdef HW_VERSION_V1_1
//#define LORA_TP1109	 1 //Lora-TP1109
#endif

//#define TIMER1_DISPOSE 1 //定时器1处理

#define USART_DATA_LEN_MAX  800
#define USART4_SEND_DATA_MAX  2500
#define DEVICE_HW_VERSION   "V1.0.0"
#define DEVICE_SW_VERSION   "V1.0.0"
#define RECORD_DATA_FORMAT	16	//记录数据格式：每条记录所占用字节数，例如1液位+1流量+2水质=4*4=16
#define EEP_VERSION	2			//存储版本号
//=#define SM4_ENTRY_ENABLE	1	//SM4加密使能

#define DUMMY_BYTE     					0xff 

#ifndef TRUE
#define	TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#define NEW_MAINLOOP	1	//新主循环
#define WATERLEVEL_RADAR_PRESS	1	//液位测量使用雷达液位计+压力水位计，最终水位值由2者计算得出

//extern unsigned char guc_NFC_USART3_RecvCnt;
//extern unsigned char guc_NFC_USART3_RecvData[5][10];	//NFC USART3接收数据缓存
//extern unsigned char guc_NFC_GetDataCnt;
//extern unsigned char guc_NFC_GetData[5][10];	//NFC USART3接收数据缓存

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/
typedef enum _PARA_TYPE
{
	Type_Char,	//单字节
	Type_Short,	//双字节
	Type_Float,	//四字节
	Type_Float_Low,	//四字节，低位在前
	Type_Float_CDAB,	//四字节，CDAB顺序
	Type_Long,	//四字节
	Type_Arr	
}enPara_Type;

typedef enum _USART_DEVICE
{
  	MODULE_4G_NB,	//USART1
	MODULE_BT,		//USART2
	MODULE_BD,		//USART3
	MODULE_NFC_RFID,	//USART3
    MODULE_MEAS_SENSOR1,	//2路485-1 USART4
	MODULE_MEAS_SENSOR2,	//2路485-2 USART4
    MODULE_CAMERA
} en_usart_device_t;

/*USART通讯使用的数据结构体 */
typedef struct _SysUsartData
{
    unsigned char ucUsartxRecvDataFlag[4];	//USART接收数据标志
    unsigned short usUsartxRecvDataLen[4];	//USART接收数据长度
    char ucUsartxRecvDataArr[3][USART_DATA_LEN_MAX];	//USART接收数据数组
	char ucUsart4RecvDataArr[1100];	//USART4接收数据数组
    unsigned short usUsartxSendDataLen[4];	//USART发送数据长度
    unsigned char ucUsartxSendDataArr[3][USART_DATA_LEN_MAX];	//USART发送数据数组
	unsigned char ucUsart1SendDataToServerArr[USART4_SEND_DATA_MAX];	//USART1发送数据到服务器数组
	unsigned char ucUsartxSendDataFlag[4];	//USART发送数据标志
	unsigned short usUsartxCurSendDataLen[4];	//USART当前发送数据长度
	en_usart_device_t enUsart3Source;	//USART3数据源
	en_usart_device_t enUsart4Source;	//USART4数据源
}SysUsartDataSt;

#if 0
/*设备设置参数-服务器上发参数*/
typedef struct _ServerPara
{
	int nChannelID;	//通道序号
	char cIPAddr[20];	//IP字符串，支持域名
	int nPort;			//端口
	char cProcArr[10];	//UPD/TCP/COAP/MQTT
	int nAppProc;		//1:本协议
	int nDeviceUploadRecordCnt;	//设备设置的上传记录时间，单位Min，注意通讯协议上的单位为s,要注意时间转换
	int nDelayTime;		//延时时间，单位min
}ServerParaSt;	//50Bytes


/*设备设置参数-以太网参数 */
typedef struct _EthPara
{
	char cAutoIP;	//0->关闭自动获取IP; 1->自动获取IP
	char cLocAddrArr[20];	//本地地址
	char cMaskArr[20];		//子网掩码
	char cGatewayArr[20];	//网关地址
	char cDNSArr[20];		//DNS服务器
}EthParaSt;	//81Bytes

#endif

/*设备外接传感器类型 */
typedef enum _MeasSensorType
{
	Meas_NULL = 1,
	Meas_BY_Integrated_Conductivity = 2,	//博雅-一体式电导率
	Meas_BY_Radar_Level,					//博雅-雷达液位计
	Meas_BY_Pressure_Level,				//博雅-压力液位计
	Meas_BY_BlackLight,					//博雅-黑光图像站
	Meas_HZ_Radar_Ultrasonic_Flow,		//航征-雷达超声波流量计
	Meas_HZ_Ultrasonic_Flow,			//航征-超声波流量计
	Meas_HZ_Radar_Level,				//航征-雷达液位计
	Meas_HX_Flowmeter,					//恒星-截污流量计
	Meas_HX_WaterQuality_COD,				//恒星-水质COD传感器	
	Meas_HX_Radar_Ultrasonic_Flow,	//恒星-雷达超声波流量计
	Meas_HX_Integrated_Conductivity,	//恒星-一体式电导率
	Meas_HX_Radar_Level,				//恒星-雷达液位计
	//Meas_WaterLevel_Radar,				//雷达液位计
	//Meas_WaterLevel_Pressure,			//压力液位计
	Meas_Flowmeter,					//流量计
	Meas_Max
}EMeasSensorType;

typedef struct _MeasSensorPara
{
	EMeasSensorType eMeasSensor;	//设备外接测量传感器设备型号,可配置
	char cGetDataFlag;	//设备外接测量传感器是否已经获取数据标志位
}MeasSensorParaSt;	//7Bytes

/*存储参数命令号 */
typedef enum _SavePara
{
	DEV_BEGIN_PARA = 0,
	DEV_ID = 1,		//设备ID
	DEV_IDEN_FLAG,	//NFC认证标志位
	DEV_REG_FLAG,	//设备服务器通讯注册标志位
	DEV_PD_DATE,	//设备生产日期
	DEV_SAMPLE_GAP,	//采样间隔时间
	DEV_RECORD_GAP,	//记录保存间隔时间
	DEV_UPLOAD_GAP,	//记录上传间隔时间
	DEV_HIS_RECORD,	//历史记录数据个数
	DEV_SENSOR_ENABLE_1, //485-1 使能标志
	DEV_SENSOR_ENABLE_2, //485-2 使能标志	
	DEV_SENSOR_CNT_1,	//设备485-1外接传感器数量
	DEV_SENSOR_CNT_2,	//设备485-2外接传感器数量
	DEV_CHN1_SENSOR1_TYPE,	//设备485-1外接传感器类型1
	DEV_CHN1_SENSOR2_TYPE,	//设备485-1外接传感器类型2
	DEV_CHN1_SENSOR3_TYPE,	//设备485-1外接传感器类型3
	DEV_CHN1_SENSOR4_TYPE,	//设备485-1外接传感器类型4
	DEV_CHN1_SENSOR5_TYPE,	//设备485-1外接传感器类型5
	DEV_CHN1_SENSOR6_TYPE,	//设备485-1外接传感器类型6
	DEV_CHN1_SENSOR7_TYPE,	//设备485-1外接传感器类型7
	DEV_CHN1_SENSOR8_TYPE,	//设备485-1外接传感器类型8
	DEV_CHN1_SENSOR9_TYPE,	//设备485-1外接传感器类型9
	DEV_CHN1_SENSOR10_TYPE,	//设备485-1外接传感器类型10
	DEV_CHN2_SENSOR1_TYPE,	//设备485-2外接传感器类型1
	DEV_CHN2_SENSOR2_TYPE,	//设备485-2外接传感器类型2
	DEV_CHN2_SENSOR3_TYPE,	//设备485-2外接传感器类型3
	DEV_CHN2_SENSOR4_TYPE,	//设备485-2外接传感器类型4
	DEV_CHN2_SENSOR5_TYPE,	//设备485-2外接传感器类型5
	DEV_CHN2_SENSOR6_TYPE,	//设备485-2外接传感器类型6
	DEV_CHN2_SENSOR7_TYPE,	//设备485-2外接传感器类型7
	DEV_CHN2_SENSOR8_TYPE,	//设备485-2外接传感器类型8
	DEV_CHN2_SENSOR9_TYPE,	//设备485-2外接传感器类型9
	DEV_CHN2_SENSOR10_TYPE,	//设备485-2外接传感器类型10
	DEV_TOTAL_VOLUME,	//设备工作累计流量
	DEV_SOFTRESET,	//设备软复位
	DEV_STATUS_UPLOAD_GAP,	//状态上传间隔时间
	DEV_IP_ADDRESS1,	//服务器1 IP地址
	DEV_IP_ADDRESS2,	//服务器2 IP地址
	DEV_IP_PORT1,		//服务器1 端口
	DEV_IP_PORT2,		//服务器2 端口
	DEV_DEBUG_MODEL,	//设备调试模式
	DEV_INSTALL_HEIGHT,	//设备安装井深
	DEV_SENSOR_BAUDRATE,	//设备485波特率
	DEV_LONGPOWER_MODEL,	//设备长供电模式
	DEV_FACTORY_RESET,	//设备恢复出厂设置
	DEV_4G_CONNECT_STATUS,	//4G连接状态
	DEV_END_PARA
}en_SaveParaCMD;


/*设备参数数据结构体*/
#pragma pack(1)
typedef struct _SysDevicePara
{
	char cDeviceID[17];	//设备ID:MQ+4位厂商代码+4位客户端类型+6位自定义字符串
	char cDeviceIdenFlag;	//设备是否经过NFC认证标志位	0->未验证
	char cDeviceRegisterFlag;	//设备是否在服务器通讯上注册标志位	0->未注册
	char cDeviceIMSI[16];	//设备IMSI号；是一个用于在全球范围内唯一地识别移动用户的标识符。IMSI存储在手机的SIM卡中，用于在蜂窝网络中进行用户身份验证和位置管理。
	char cDeviceIMEI[16];	//设备IMEI号；通常所说的手机序列号、手机“串号”，用于在移动电话网络中识别每一部独立的手机等移动通信设备，相当于移动电话的身份证。序列号共有15~17位数字
	char cDeviceHWVersion[10];	//设备硬件版本号
	char cDeviceSWVersion[10];	//设备软件版本号
	char cDevicePDDate[10];		//设备生产日期: YY-MM-DD
	int nDeviceSampleGapCnt;	//设备设置的采样间隔时间，单位Min，注意通讯协议上的单位为s,要注意时间转换
	int nDeviceSaveRecordCnt;	//设备设置的保存记录时间，单位Min，注意通讯协议上的单位为s,要注意时间转换
	int nDeviceUploadCnt;		//设备设置的上传间隔时间，单位Min，注意通讯协议上的单位为s,要注意时间转换
	int nDeviceRecordCnt;	//设备历史数据记录计数
	//ServerParaSt esServerPara;	//服务器上发参数
	char cMQTT_UserNameArr[20];	//登录用户名
	char cMQTT_UserPWDArr[20];	//登录密码
	char cHeartGap;				//心跳间隔，单位s
	char cDps;					//离散估长	单位s
	//char cAPNArr[20];			//接入点名称
	//char cVPDNUserNameArr[20];	//VPDN用户名
	//char cVPDNUserPWDArr[20];	//VPDN密码
	//EthParaSt esEthPara;		//以太网参数
	char cParaVersion;			//设备参数版本号
	char cMeasSensorEnableFlag[2];	//两路485使能开关
	char cMeasSensorCount[2];		//设备两路外接测量传感器数量
	EMeasSensorType eMeasSensor[2][10];	//设备两路485外接测量传感器设备型号,可配置
	float fTotal_Volume;	//设备工作累计流量，单位m3
	float fInit_Height;		//设备安装时井深，用于计算液位高度，单位m，该值减去雷达液位计的空高值即为液位高度
	unsigned char ucUploadStatusGap;	//设备状态上传间隔时间，单位h 
	char cServerIP[2][16];	//设备连接物联网平台IP地址，共2组
	unsigned short usServerPort[2];	//设备连接物联网平台端口，共2组
	char cMonitorMode;		//设备监测模式; 0->正常模式; 1->调试模式:便携式采集
	char cSensorBaudRate;	//设备485波特率; 0->9600; 1->2400; 2->4800; 3->19200; 4->38400; 5->115200
	char cBackUpArr[50];	//备用数据数组，长度50字节
	short sEEP_Version;			//存储版本号
}SysDeviceParaSt;	//325Bytes
#pragma pack()

typedef enum _DeviceRunState
{
	DEVICE_RUN_STATE_STOP = 0,	//设备停止
	DEVICE_RUN_STATE_RUN,	//设备运行
	DEVICE_RUN_STATE_FAULT,	//设备故障
	DEVICE_RUN_STATE_SLEEP,	//设备休眠
	DEVICE_RUN_STATE_DIAG,	//设备调试
	DEVICE_RUN_STATE_MAX
}en_DeviceRunState;

/*设备状态数据结构体 */
typedef struct _SysDeviceStatusPara
{
	char cDevStartDateTime[20];	//设备启动时间
	int nDevStartDays;	//设备运行时间天数
	int nDevSignalCSQ;	//设备信号质量
	int nDevSignalRSRP;	//设备信号强度
	char cDevimmersionFlag;	//设备浸水标志 0->未浸没; 1->浸没
	float fBattleVoltage;	//电池电压
	double fDevLoca_lng;	//设备经度
	double fDevLoca_lat;	//设备纬度
	unsigned short usDevStatus;	//设备状态
}SysDeviceStatusParaSt;

//GPS NMEA-0183协议重要参数结构体定义
//卫星信息
typedef struct
{
	unsigned char  ucNum;    //卫星编号
	unsigned char  ucEledeg;  //卫星仰角
	unsigned short usAzideg;  //卫星方位角
	unsigned char  ucSn;    //信噪比
}NEMA_SlmsgSt;
 
//UTC时间信息
typedef struct
{
	unsigned short usYear;  //年份
	unsigned char ucMonth;  //月份
	unsigned char ucDate;  //日期
	unsigned char ucHour;   //小时
	unsigned char ucMin;   //分钟
	unsigned char ucSec;   //秒钟
}NEMA_UTC_TimeSt;        
 
//NMEA 0183 协议解析后数据存放结构体
typedef struct
{
	unsigned char ucSvnum;          //可见卫星数
	//NEMA_SlmsgSt esSlmsg[12];    //最多12颗卫星
	NEMA_UTC_TimeSt esUTC;      //UTC时间
  	unsigned long ulLatitude;        //纬度 分扩大100000倍,实际要除以100000
  	unsigned char ucNshemi;          //北纬/南纬,N:北纬;S:南纬
	unsigned long ulLongitude;          //经度 分扩大100000倍,实际要除以100000
	unsigned char ucEwhemi;          //东经/西经,E:东经;W:西经
	unsigned char ucGpssta;          //GPS状态:0,未定位;1,非差分定位;2,差分定位;6,正在估算.
	unsigned char ucPosslnum;        //用于定位的卫星数,0~12.
	unsigned char ucPossl[12];        //用于定位的卫星编号
	unsigned char ucFixmode;          //定位类型:1,没有定位;2,2D定位;3,3D定位
	unsigned short usPdop;          //位置精度因子 0~500,对应实际值0~50.0
	unsigned short usHdop;          //水平精度因子 0~500,对应实际值0~50.0
	unsigned short usVdop;          //垂直精度因子 0~500,对应实际值0~50.0 
 
	unsigned short usCourse;       //航向
  	int nAltitude;         //海拔高度,放大了10倍,实际除以10.单位:0.1m
	unsigned long ulSpeed;          //地面速率,放大了1000倍,实际除以10.单位:0.001公里/小时
	unsigned char ucDataValidFlag;	//定位数据是否有效标志位; 0->无效；1->有效
}NMEA_MsgSt;

/*设备板级传感器数据结构体 */
typedef struct _SysDeviceSensorData
{
	//signed short  sDev_Attitude_SC7A[3][3];	//姿态传感器数据
	int nDev_Attitude_SC7A;	//姿态传感器数据--Z轴倾角加速度
	char cWater_Immersion_Status;			//水浸传感器状态; 0->未浸没； 1->浸没
	float fWater_Immersion_Level;			//水浸传感器液位值
	unsigned char ucBattery_Level_Flag;		//电池电量标志位; 当采集电量<30%时，进行确认处理
	float fBattery_Level_Percent;					//设备当前电池电量-百分比
	char cMagnetic_Bar_Status;				//磁控检测状态； 0->未激活； 1->激活
	char cPhotosensitive_XYC_ALS_Status;	//光照检测状态: 0->未激活； 1->激活
	short sPhotosensitive_XYC_ALS_Data;	//光照传感器数据
	float fDevTemperature;	//设备MCU 内置温度传感器数据
	NMEA_MsgSt esBD_NEMAData;				//北斗定位信息结构体
}SysDeviceSensorsData;

//博雅-一体式电导率数据结构体
typedef struct _IntegratedConductivityData
{
	float fConductivityValue;	//电导率值
	float fTemperatureValue;	//温度值
	float fSalinityValue;	//盐度值
	float fTDSValue;		//总溶解固体值
}esIntegratedConductivityDataSt;	//8Bytes

//博雅-液位计数据结构体
typedef struct _BY_LevelData
{
	float fRadarWaterLevelValue;	//雷达液位值
	float fRadarEmptyHeightValue;	//雷达空高值
	float fPressureWaterLevelValue;	//压力液位值
}esBY_LevelDataSt;	//12Bytes

//航征-液位计数据结构体
typedef struct _HZ_LevelData
{
	float fRadarWaterLevelValue;	//雷达液位值
	float fRadarEmptyHeightValue;	//雷达空高值
	float fRadarRoll_Angle;	//航征-雷达液位计横滚角值
	float fRadarVertical_Angle;	//航征-雷达液位计垂直角值
	float fRadarInstallHeight;	//航征-雷达液位计安装高度
}esHZ_LevelDataSt;	//12Bytes

//航征-雷达超声波流量计数据结构体
typedef struct _HZ_Radar_Ultrasonic_FlowData
{
	float fFlowValue;	//雷达超声波瞬时流量值
	float fPositiveCumulativeTraffic;	//正向累计流量
	float fDischargeArea;	//过水面积
	float fSurfaceVelocity;	//表面流速
	float fCross_SectionVelocity;	//横截面流速(断面流速)
	float fWaterLevel;	//水位高度
	float fEmptyHeight;	//空高值
	float fRadarRoll_Angle;	//航征-流量计横滚角值
	float fRadarVertical_Angle;	//航征-流量计垂直角值
	float fRadarInstallHeight;	//航征-流量计安装高度
}esHZ_Radar_Ultrasonic_FlowDataSt;	//12Bytes

//恒星-流量计数据结构体
typedef struct _HX_FlowmeterData
{
	float fFlowValue;	//流量值
	float fCumulativeTraffic;	//累计流量
	float fSurfaceVelocity;	//表面流速
	float fCross_SectionVelocity;	//横截面流速(断面流速)
	float fUltraFlowVelocity;	//超声波流速
	float fWaterLevel;	//水位高度
	float fEmptyHeight;	//空高值
	float fTemperature;	//温度值
	float fRadarRoll_Angle;	//航征-流量计横滚角值
	float fRadarVertical_Angle;	//航征-流量计垂直角值
	float fVoltage;	//电压值
}esHX_FlowmeterDataSt;	//24Bytes

//恒星-COD传感器数据结构体
typedef struct _HX_WaterQuality_CODData
{
	unsigned char ucSensorStatus;	//传感器状态 0->正常; 1->异常
	float fCODValue;	//COD值
	float fTurbidity;	//浊度值
	float fCODSignalValue;	//COD信号值
	float fTurbiditySignalValue;	//浊度信号值
	float fTemperature;	//温度值
} esHX_WaterQuality_CODDataSt;	//8Bytes

/*外挂传感器测量数据结构体 */
typedef struct _SysMeasSensorData
{
	float fWaterLevel_Radar;		//液位-雷达
	float fWaterLevel_Pressure;	//液位-压力
	float fVolumeValue;		//瞬时流量 .单位m3/s
	float fWaterQuality_DDValue;	//水质-电导率
	float fWaterQuality_CODValue;	//水质-COD
	esIntegratedConductivityDataSt es_IntegratedConductivityData;	//一体式电导率数据
	esBY_LevelDataSt esBY_LevelData;	//博雅-雷达液位计数据
	esHZ_LevelDataSt esHZ_LevelData;	//航征-雷达液位计数据
	esHZ_Radar_Ultrasonic_FlowDataSt esHZ_Radar_Ultrasonic_FlowData;	//航征-雷达超声波流量计数据
	esHX_FlowmeterDataSt esHX_FlowmeterData;	//恒星-流量计数据
	esHX_WaterQuality_CODDataSt esHX_WaterQuality_CODData;	//恒星-COD传感器数据
	//esIntegratedConductivityDataSt esHX_
	float fHXRadarWaterLevelValue;	//HX-雷达液位计
}SysMeasSensorData;

typedef enum _ShowViewType
{
	SType_Meas_Level,	//测量数据-液位界面
	SType_Meas_Water_Quality,	//测量数据-水质界面
	SType_Meas_Sensor_Value,	//板级传感器数据:姿态；水浸；光照、
	SType_BD_Data,	//北斗数据界面
	SType_Dev_ID_PD,		//设备ID,PD界面
	SType_Dev_SW_HW,	//设备SW,HW界面
	SType_Max			//异常
}eShowViewType;

typedef enum _PowerType
{
	Power_ON,	//开机状态
	Power_OFF	//关机状态
}ePowerType;

typedef enum _UploadModuleStatus
{
	Status_OK,			//在线
	Status_Init,		//初始化
	Status_AT_Test,		//AT测试
	Status_SIM_Status,	//SIM状态
	Status_Signal_Strength,	//信号强度
	Status_Register,	//注册
	Status_Active_Network,	//激活网络
	Status_Check_Network,	//查询网络状态
	Status_Get_IMSI,		//获取IMSI
	Status_Get_IMEI,		//获取IMEI
	Status_Get_DateTime,	//获取日期时间
	Status_MQTT_SET_Data_Format,	//MQTT 设置数据格式
	Status_MQTT_Set_KeepAlive_Time,	//MQTT 设置心跳时间
	Status_MQTT_Set_Version,		//MQTT 设置版本
	Status_MQTT_Open_Interface,		//MQTT 打开端口
	Status_MQTT_Connect_Interface,	//MQTT 连接端口
	Status_MQTT_Subscribe_Topic_Register,	//MQTT 订阅注册主题
	Status_MQTT_Subscribe_Topic_DataUpload,	//MQTT 订阅监测项上报主题
	Status_MQTT_Subscribe_Topic_StatusUpload,	//MQTT 订阅状态上报主题
	Status_MQTT_Subscribe_Topic_SetPara,	//MQTT 订阅设置参数主题
	Status_MQTT_Subscribe_Topic_GetPara,	//MQTT 订阅读取参数主题
	Status_MQTT_Publish_Topic_Register,		//MQTT 发布注册主题
	Status_MQTT_Publish_Topic_DataUoload,	//MQTT 发布监测项上传主题
	Status_MQTT_Publish_Topic_StatusUpload,	//MQTT 发布状态上报主题
	Status_Init_Failed,	//初始化失败
	//Status_Upload,		//上传数据
	Status_Err			//异常
}eUploadModuleStatus;

typedef struct _BlackLightDataStr
{
	char cResolution;	//分辨率: 1->160*120; 3->320*240; 5->640*480; 6:1280*640
	char cCompression_Radio;	//压缩比
	unsigned long ulData_Size;	//数据大小
	unsigned long ulCurGetDataSize;	//当前获取数据大小
	unsigned char ucPhotoDataCnt;	//图像数据个数
	unsigned char ucCurPhotoDataCnt;	//当前传输图像数据个数
	unsigned char ucPhotoData[136][400];	//图像数据
}BlackLightDataSt;

/*设备运行参数数据结构体 */
typedef struct _SysDeviceRunPara
{
	SysDeviceStatusParaSt esDeviceRunState;	//设备运行状态
	en_DeviceRunState enDeviceRunMode;	//设备运行模式
	SysDeviceSensorsData esDeviceSensorsData;	//设备板级传感器数据
	int nDeviceCurSaveRecordCount;	//设备累计计数，单位Min
	int nDeviceCurUploadRecordCount;	//设备累计计数，单位Min
	int nDeviceCurSampleCount;			//设备累计采用间隔计数，单位min
	char cDeviceCurDateTime[20];	//设备当前时间
	char cDeviceBTPowerOnFlag;		//设备蓝牙电源是否开启标志位
	char cDeviceBTConnectFlag;	//设备蓝牙连接状态 0->未连接; 1->已连接
	unsigned short usDeviceBTWaitCnt;		//等待蓝牙无通讯计数值，单位1s
	char cDeviceBTWaitConnectFlag;	//等待蓝牙连接标志位; 0->未连接; 1->已连接
	unsigned short usDeviceBTWaitConnectCnt;	//等待蓝牙连接计数值，单位1s
	unsigned short usDeviceBTWaitConnectMaxCnt;	//等待蓝牙连接最大计数值，单位1s,通过磁棒唤醒时值为20s，通过4G下发指令进入调试模式时，值为30minn
	unsigned short usBTRecValue;
	SysMeasSensorData esMeasData;	//实时外挂测量传感器数据
	unsigned short usDevStatus;	//设备状态字: 0->正常; 0x01->4G初始化异常; 0x02->NFC初始化异常; 0x04->姿态传感器异常;0x08->水浸传感器异常; 
								//0x10->光照传感器异常; 0x20->MODBUS通信异常; 0x40->BT异常
	eShowViewType eShowView;	//当前要显示的界面类型
	ePowerType eCurPowerType;	//当前设备开关机状态
	char cBarTouchCnt;			//磁棒接触时间计数，单位s
	unsigned char ucOLEDInitFlag;	//OLED上电初始化标志位: 1->已初始化; 0->未初始化
	eUploadModuleStatus enUploadStatus;		//设备上传模块状态标志位
	char cWaitServerCMDFlag;	//上传完数据后，等待一段时间服务器是否有需要通讯标志位
	char cWaitServerCMDCnt;		//等待服务器通讯计数值，1s一次
	char cConnectServerFlag;	//连接服务器标志位; 0->未连接; 1->已连接
	char cTimer4StartFlag;		//定时器Timer4启动标志位
	char cMeasDelayFlag;		//开启外接传感器测量标志位
	char cMeasDelayCnt;			//外接传感器等待数据稳定计数值，1s一次
	unsigned char ucCurUploadRecordCnt;	//当前上传记录计数值
	unsigned long ulUploadRecordLostCnt;	//上传记录丢失计数值
	unsigned long ulUploadRecordStartTime;	//上传记录开始时间, UNIX时间戳
	char cDeviceStatusUploadFlag;	//设备状态上传标志位; 0->未上传; 1->已上传
	unsigned char ucCurUploadStatusCnt;	//当前上传状态计数值
	unsigned long ulUploadStatusLostCnt;	//上传状态丢失计数值
	unsigned long ulUploadStatusStartTime;	//上传状态开始时间, UNIX时间戳
	char cLostStatusArrCnt;				//丢失状态数组计数值
	SysDeviceStatusParaSt esLostStatusArr[10];	//丢失状态数组
	char cDeviceGetDataFlag;		//设备获取数据标志位; 0->未获取; 1->已获取
	unsigned long ulDeviceGetDataStartCnt;	//设备获取数据开始时间对应的数据起始
	unsigned long ulDeviceGetDataEndCnt;	//设备获取数据结束时间对应的数据结束
	char cRTC_1MIN_ReflashFlag;		//RTC每分钟刷新标志位; 0->未刷新; 1->已刷新
	char cTimer4_1SEC_ReflashFlag;	//定时器4每秒刷新标志位; 0->未刷新; 1->已刷新
	char cBarTouchFlag;		//磁棒接触标志位; 0->未接触; 1->已接触
	unsigned long ulBarTouchEventUploadTime;	//磁棒接触事件上传时间, UNIX时间戳
	char cGetMeasSensorValueFlag;	//获取外接测量传感器数据标志位; 0->未获取; 1->已获取
	char cGetMeasSensorValueSuccFlag;
	char cBD_GetValueCnt;		//北斗获取数据标志位; 0->未获取; 1->已获取
	char cBD_GetDataFlag;		//USART3 BD通讯获的数据标志位
	char cBD_LostDataCnt;		//USART3 BD通讯丢失数据计数值; 1s一次
	char cGetSensorCnt;
	char cTimer1sTriggerFlag;	//定时器1s触发标志位; 0->未触发; 1->已触发
	char cModbusErrCnt;	//Modbus通讯错误计数值; 
	char cDevUploadStatusTime;	//设备上传状态时间间隔，单位hour
	char cDebugModel;	//设备调试模式; 0->正常模式； 1->调试模式
	char cDebugModelStartFlag;	//设备是否已经进入调试模式标志位; 0->未进入； 1->已进入
	char cEveryNFCDisposeFlag;	//每次NFC操作标志位; 0->未操作; 1->已操作
	unsigned short usUploadStatusDataCurCnt;	//当前上传状态数据计数值
	char c4GInitFlag;	//4G模块初始化标志位，重新授权通过时，需要重新注册设备
	char cTotalSensorCnt;	//当前设备外接传感器总数
	char cMeasShowViewCnt;	//当前设备测量数据需要显示的界面个数。每个界面显示最多2个测量参数
	char cCurMeasShowViewCnt;	//当前已显示的测量数据界面个数
	char c4GInitWaitCnt;	//开机4G初始化等待时间
	char cLongPowerModel;	//设备长供电模式，给外接传感器进行30分钟供电，便于外接传感器进行调试，比如雷达液位计通过蓝牙小程序调试等
	unsigned short usLongPowerModelWaitCnt;	//长供电模式，当前等待时间，单位s
	char cBoardSensorXYCValueChangeCnt;	//板级XYC传感器数据变化计数值
	char cBoardSensorPhotoValueChangeCnt;	//板级光照传感器数据变化计数值
	char cBDRestartFlag;	//北斗重启标志位
	int nSignalStrength;	//信号强度
	char cSaveDataTimeDelayFlag;	//保存数据时间延迟标志位
	char c4GUploadDataContinueFlag;	//4G上传数据连续标志位
	char c4GInitFailedCnt;	//4G初始化失败计数值
	char c4GInitFailedFlag;	//4G初始化失败标志位
	char c4GPowerOffCnt;	//4G模块断电标志位
	char cPowerOffFlag;
	char cPowerOffCnt;
	char cShowConnectFlag;	//显示联系电话界面标志位
	char cShowConnectCnt;	//显示联系电话界面计数值
	//char cModbusNULLCnt;	//Modbus通讯NULL计数值
	//char cModbusNULLCnt111;
	char cBlackLightFlag;		//是否为黑光图像站设备 	
	BlackLightDataSt st_BlackLightData;	//黑光图像站数据结构体
	//char c4GTimerCnt;
}SysDeviceRunParaSt;

/*排水管网数据结构体-液位信息 */
typedef struct _MeasLevelData
{
	float fWater_height;	//检查井液位值
	float fPipeWater_height;	//管道液位值
	float fWater_elevation;		//检查井液位高程
}MeasLevelDataSt;

/*排水管网数据结构体-流量信息*/
typedef struct _MeasFlowData
{
	float fCurr_volume_h;	//瞬时流量 m³/h
	float fCurr_volume_s;	//瞬时流量 m³/s
	float fFlow_velocity;	//流速	m/s
	float fSurface_velocity;	//水面流速 m/s
	float fTotal_volume;	//累计流量
}MeasFlowDataSt;

/*排水管网数据结构体-水质信息 */
typedef struct _MeasWaterQualityData
{
	float fPH;	//PH值
	float fTN;	//总氮
	float fTP;	//总磷
	float fNHN;	//氨氮
	float fBOD;	//生化需氧量
	float fCOD;	//化学需氧量
	float fSS;	//悬浮物浊度
	float fCOND;	//电导率
	float fKRQ;	//可燃气
	float fO2;	//氧气
	float fH2S;	//硫化氢
	float fCO;	//一氧化碳
	float fWater_Temp;	//水温
}MeasWaterQualityDataSt;

/*设备上传记录数据结构体 */
typedef struct _SysDeviceUploadData
{
	char cDeviceStartUploadDateTime[20];	//设备启动上传时间
	int nDeviceUploadRecordCntMax;	//设备上传记录计数最大值
	MeasLevelDataSt esMeasLevelData[10];	//液位信息
	MeasFlowDataSt esMeasFlowData[10];	//流量信息
	MeasWaterQualityDataSt esMeasWaterQualityData[10];	//水质信息
}SysDeviceUploadDataSt;

/*设备存储记录结构体*/
typedef struct _SystemRecordData
{
	MeasLevelDataSt esLevel;		//液位
	MeasFlowDataSt esFlow;			//流量
	MeasWaterQualityDataSt esWaterQua;	//水质
}SystemRecordDataSt;

/*设备测量数据记录结构体-暂定 */
typedef struct _DevMeasRecordData
{
	float fWaterLevel;		//液位
	float fWaterLevel_Radar;		//液位-雷达
	float fWaterLevel_Pres;		//液位-压力
	float fWaterQuality_COND;		//水质-电导率
	float fWaterQuality_COD;		//水质-COD
	float fWaterVolume;			//流量-瞬时流量 m³/h
	float fWaterSpeed;			//流量-流速
	float fWaterVolume_Total;	//流量-累计流量 m³
	unsigned short nAttitude_SC7A;		//姿态传感器数据--Z轴倾角加速度
	char cWater_Immersion_Status;	//水浸传感器状态; 0->未浸没； 1->浸没
	char cPhotosensitive_XYC_ALS_Status;	//光照检测状态: 0->未激活； 1->激活
	char cBT_ConnectFlag;	//蓝牙连接状态: 0->未连接；1->连接
	char cDebug_Model;		//设备是否处于调试模式: 0->正常模式： 1->调试模式
	char cLongPowerModel;	//设备是否处于长供电模式: 0->正常模式; 1->长供电模式
	float fDevTemperature;	//设备MCU内部温度
}DevMeasRecordDataSt;	//目前结构体大小为43字节

/*系统运行全局变量结构体 */
typedef struct _SystemPara
{
  	SysUsartDataSt UsartData;	//USART通讯使用的数据结构体
	SysDeviceParaSt DevicePara;	//设备参数数据结构体
	SysDeviceRunParaSt DeviceRunPara;	//设备运行参数数据结构体
	SysDeviceUploadDataSt DeviceUploadData;	//设备上传记录数据结构体
	
}SystemPataSt;

#define CONNECT_MQTT	1
typedef enum _4G_Module_Init_State
{
	Module_START_WAIT_CMD = 0,	//等待模块启动完成
	Module_DCE_RST_STAGE1,
	Module_DCE_RST_STAGE2,
	Module_DCE_RST_STAGE3,
	//Module_DCE_RST_STAGE4,
	Module_TEST_AT_CMD,	//测试AT指令
	Module_TEST_ATE0_CMD,	//关闭回显
	Module_QUERY_SIM_CARD_STATE_CMD,	//查询SIM卡状态
	Module_QUERY_SIGNAL_STRENGTH_CMD,	//查询信号强度
	//Module_QICSGP_CMD,	//设置PDP上下文参数; APN等
	//Module_QIACT_CMD,	//激活PDP上下文; APN等
	//Module_PDP_TEST_CMD,
	Module_QUERY_PS_DOMAIN_REG_STATE_CMD,	//查询PS域注册状态
	Module_ACTIVATE_NETWORK_CMD,			//激活网络
	Module_QUERY_NETWORK_ACTIVATE_STATE_CMD,	//查询网络激活状态
	
	Module_QUERY_IMSI_CMD,	//查询IMSI号
	Module_QUERY_IMEI_CMD,	//查询IMEI号
	Module_QUERY_LOCAL_DATE_TIME_CMD,	//查询本地日期时间
	Module_SET_DATA_FORMAT_CMD,	//设置数据格式
	Module_SET_MQTT_KEEPALIVE_TIME_CMD,	//设置MQTT心跳时间
	Module_SET_MQTT_VERSION_CMD,	//设置MQTT协议版本; 4->3.1.1； 3->3.1
	//Module_Check_MQTT_OPEN_CMD1,	//检查MQTT是否打开
	Module_OPEN_MQTT_INTERFACE_CMD,	//打开物联网云端口
	//Module_Check_MQTT_OPEN_CMD2,	//检查MQTT是否打开
	//Module_Check_MQTT_CONN_CMD1,	//检查MQTT连接状态
	Module_CONN_MQTT_INTERFACE_CMD,	//连接物联网云端口
	//Module_Check_MQTT_CONN_CMD2,	//检查MQTT连接状态
	//Module_CONNECT_CMD,	//连接MQTT服务器
	//Module_SUBSCRIBE_TOPIC_CMD,	//订阅主题
	//Module_CHECKPUBEX_CMD,	//检查MQTT订阅状态
	#ifdef CONNECT_MQTT
	Module_SUBSCRIBE_TOPIC_REGISTER_CMD,	//订阅主题注册
	Module_SUBSCRIBE_TOPIC_DATAUPLOAD_CMD,	//订阅主题数据上传
	Module_SUBSCRIBE_TOPIC_STATUS_CMD,	//订阅主题状态上报
	//Module_SUBSCRIBE_TOPIC_ALARMDATA_CMD,	//订阅主题报警数据上报
	Module_SUBSCRIBE_TOPIC_SETCONFIG_CMD,	//订阅主题参数配置
	Module_SUBSCRIBE_TOPIC_GETCONFIG_CMD,	//订阅主题参数获取
	//Module_SUBSCRIBE_TOPIC_GETDATA_CMD,		//订阅主题获取数据
	//Module_SUBSCRIBE_DATAPT_CMD,	//订阅主题数据透传

	Module_PUBLISH_TOPIC_REGISTER_CMD,	//发布主题注册
	Module_PUBLISH_TOPIC_DATAUPLOAD_CMD,	//发布主题数据上传
	//Module_PUBLISH_TOPIC_DATABASEUPLOAD_CMD,	//发布监测项原始数据上传
	Module_PUBLISH_TOPIC_SATATUSUPLOAD_CMD,	//发布主题状态上报
	Module_PUBLISH_TOPIC_STATUS_CMD,	//发布主题状态上报
	Module_PUBLISH_TOPIC_ALARMDATA_CMD,	//发布主题报警数据上报
	Module_PUBLISH_TOPIC_SETCONFIG_CMD,	//发布主题参数配置
	Module_PUBLISH_TOPIC_GETCONFIG_CMD,	//发布主题参数获取
	Module_PUBLISH_TOPIC_GETDATA_CMD,		//发布主题获取数据
	Module_PUBLISH_TOPIC_DATAPT_CMD,	//发布主题数据透传
	#endif
	Module_PUBLISH_TOPIC_CMD,	//发布主题
	
	Module_DISCONNECT_MQTT_INTERFACE_CMD,	//断开MQTT服务器
	Module_CLOSE_CONN_CMD,	//关闭MQTT客户端网络
	Module_TUNS_TOPIC_CMD,	//退订主题
	Module_INIT_STATE_MAX
}en_4G_Module_Init_State;

extern unsigned char guc_OLED_Buf[128][8];	//OLED显示数据缓存;8*8=64行128列
extern unsigned char  picc_atqa[2],picc_uid[15],picc_sak[3];
extern unsigned char guc_TextBDData[20];
extern unsigned char guc_NFCPWRInitFlag;
extern unsigned char guc_SystemPowerInitFlag;
//extern unsigned char guc_RTC_TimeOutFlag;
extern unsigned char guc_LcdDipRevesAttr;
extern unsigned char guc_NFC_Card_Flag;
extern SysDeviceParaSt gs_DeviceDefaultPara;
extern unsigned char guc_RTURecvArr[100];
extern char gc_SystemPosi;
extern DevMeasRecordDataSt gSt_DevMeasRecordData;	//设备测量数据记录结构体
extern unsigned char guc_SystemTestFlag;
extern unsigned short gus_SystemTestArr[10];
extern unsigned char ucMeasPosi;
extern unsigned char ucMeasValue[10];
extern en_4G_Module_Init_State gE_4G_Module_Init_CMD;
//extern unsigned short gus_BarCnt;
//extern unsigned short gus_BarCnt1;
/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
SystemPataSt * GetSystemPara(void);
extern void func_SystemPara_Init(void);

extern int decimal_bcd_code(int decimal);
extern int bcd_decimal_code( int bcd);
extern unsigned char func_Array_Find_Str(char *ucRecvBuf, unsigned short usDataLen, char *ucCheckBuf, unsigned short usCheckLen, unsigned short* usPosi);
extern unsigned short func_Array_Find_Str_Pos(char *ucRecvBuf, unsigned short usDataLen, char *ucCheckBuf, unsigned short usCheckLen);

#ifdef __cplusplus
}
#endif

#endif /* __USER_DATA_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
