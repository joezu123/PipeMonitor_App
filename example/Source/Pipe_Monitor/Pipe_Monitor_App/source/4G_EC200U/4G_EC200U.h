/**
 *******************************************************************************
 * @file  Pipe_Monitor_BootLoader\drivers\device_drv\4G_EC200U\4G_EC200U.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-12       Joe             First version
 @endverbatim

 */
#ifndef __4G_EC200U_H__
#define __4G_EC200U_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32f460_gpio.h"
#include "hc32f460_usart.h"
#include "hc32f460_utility.h"
#include "drv_USART.h"
#include "User_Data.h"
#include "drv_RTC.h"

#define CONNECT_MQTT	1
/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/
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
/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define PWRDCE_GPIO_PIN            (Pin02)
#define PWRDCE_GPIO_PORT           (PortE)

#define PWRDCE_PIN_OPEN()  PORT_SetBits(PWRDCE_GPIO_PORT, PWRDCE_GPIO_PIN)
#define PWRDCE_PIN_CLOSE() PORT_ResetBits(PWRDCE_GPIO_PORT, PWRDCE_GPIO_PIN)

#define EC200U_4G_MODULE_RST_PIN               (Pin03)
#define EC200U_4G_MODULE_RST_PORT              (PortE)

#define DCERST_PIN_RESET PORT_ResetBits(EC200U_4G_MODULE_RST_PORT, EC200U_4G_MODULE_RST_PIN)
#define DCERST_PIN_SET   PORT_SetBits(EC200U_4G_MODULE_RST_PORT, EC200U_4G_MODULE_RST_PIN)


/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/

extern uint8_t drv_EC200U_4G_Module_Init(unsigned char ucDataUploadEnable);
extern uint8_t func_Upload_Sensor_Value_To_OneNet_Text(void);
extern void func_EC200U_4G_PownDown_Deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* __COM_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
