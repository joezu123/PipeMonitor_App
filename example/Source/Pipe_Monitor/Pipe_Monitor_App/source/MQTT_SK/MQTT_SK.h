/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\MQTT_SK\MQTT_SK.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-17       Joe             First version
 @endverbatim

 */
#ifndef __MQTT_SK_H__
#define __MQTT_SK_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "drv_USART.h"
#include "drv_RTC.h"
/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/


/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/
typedef enum _MQTT_SK_CMD
{
    MQTT_SK_CMD_INIT = 0,	//初始化
    MQTT_SK_CMD_DEV_REG,	//设备注册
    MQTT_SK_CMD_DATAUPLOAD,	//数据上传
    MQTT_SK_CMD_SETCONFIG,	//设置配置
    MQTT_SK_CMD_GETCONFIG,	//获取配置
    MQTT_SK_CMD_GETDATA,	//获取数据
    MQTT_SK_CMD_DATAPT,  //数据透传
    MQTT_SK_CMD_MAX
}en_MQTT_SK_CMD;


/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern uint8_t MQTT_SK_Device_Regist(void);
extern uint8_t MQTT_SK_Data_Upload(void);
extern uint8_t MQTT_SK_Set_Config(unsigned short usPosi);
extern uint8_t MQTT_SK_Get_Config(unsigned short usPosi);
extern uint8_t MQTT_SK_Get_Data(void);
extern uint8_t MQTT_SK_Data_Pass_Through(void);
extern uint8_t MQTT_SK_Dispose(en_MQTT_SK_CMD enType, uint8_t *pbuff, uint16_t u16Len); 
extern void func_MQTT_SK_Init(void);
#ifdef __cplusplus
}
#endif

#endif /* __MQTT_SK_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
