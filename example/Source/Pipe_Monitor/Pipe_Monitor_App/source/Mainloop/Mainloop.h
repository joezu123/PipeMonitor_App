/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\Mainloop\Mainloop.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-18       Joe             First version
 @endverbatim

 */
#ifndef __MAINLOOP_H__
#define __MAINLOOP_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ddl.h"
#include "user_data.h"
#include "drv_RTC.h"
#include "MQTT_SK.h"
#include "drv_Storage_W25Q128.h"
/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/

#define BOOT_GPIO_PORT  PortB
#define BOOT_GPIO_PIN   Pin11

//#define BOOT_PIN_OPEN() PORT_SetBits(BOOT_GPIO_PORT,BOOT_GPIO_PIN)
#ifndef HW_VERSION_V1_1
#define MAINPWR3V8_GPIO_PORT  PortA
#define MAINPWR3V8_GPIO_PIN   Pin08

#define MAINPWR3V8_PIN_OPEN()  PORT_SetBits(MAINPWR3V8_GPIO_PORT,MAINPWR3V8_GPIO_PIN)
#define MAINPWR3V8_PIN_CLOSE()  PORT_ResetBits(MAINPWR3V8_GPIO_PORT,MAINPWR3V8_GPIO_PIN)
#endif

#ifdef HW_VERSION_V1_1
#define SENSOR_PWRVO1_GPIO_PORT  PortA
#define SENSOR_PWRVO1_GPIO_PIN   Pin09
#else
#define SENSOR_PWRVO1_GPIO_PORT  PortB
#define SENSOR_PWRVO1_GPIO_PIN   Pin13
#endif

#define SENSOR_PWRVO1_PIN_OPEN()  PORT_SetBits(SENSOR_PWRVO1_GPIO_PORT,SENSOR_PWRVO1_GPIO_PIN)
#define SENSOR_PWRVO1_PIN_CLOSE()  PORT_ResetBits(SENSOR_PWRVO1_GPIO_PORT,SENSOR_PWRVO1_GPIO_PIN)

#ifdef HW_VERSION_V1_1
#define SENSOR_PWRVO2_GPIO_PORT  PortC
#define SENSOR_PWRVO2_GPIO_PIN   Pin08
#else
#define SENSOR_PWRVO2_GPIO_PORT  PortB
#define SENSOR_PWRVO2_GPIO_PIN   Pin12
#endif

#define SENSOR_PWRVO2_PIN_OPEN()  PORT_SetBits(SENSOR_PWRVO2_GPIO_PORT,SENSOR_PWRVO2_GPIO_PIN)
#define SENSOR_PWRVO2_PIN_CLOSE()  PORT_ResetBits(SENSOR_PWRVO2_GPIO_PORT,SENSOR_PWRVO2_GPIO_PIN)

#ifdef HW_VERSION_V1_1
#define SENSOR_PWRRS1_GPIO_PORT PortD
#define SENSOR_PWRRS1_GPIO_PIN  Pin09

#define SENSOR_PWRRS1_PIN_OPEN()  PORT_SetBits(SENSOR_PWRRS1_GPIO_PORT,SENSOR_PWRRS1_GPIO_PIN)
#define SENSOR_PWRRS1_PIN_CLOSE()  PORT_ResetBits(SENSOR_PWRRS1_GPIO_PORT,SENSOR_PWRRS1_GPIO_PIN)

#define SENSOR_PWRRS2_GPIO_PORT PortD
#define SENSOR_PWRRS2_GPIO_PIN  Pin08

#define SENSOR_PWRRS2_PIN_OPEN()  PORT_SetBits(SENSOR_PWRRS2_GPIO_PORT,SENSOR_PWRRS2_GPIO_PIN)
#define SENSOR_PWRRS2_PIN_CLOSE()  PORT_ResetBits(SENSOR_PWRRS2_GPIO_PORT,SENSOR_PWRRS2_GPIO_PIN)
#endif


#ifdef HW_VERSION_V1_1
#define PWRUP_CTLUP1_GPIO_PORT  PortC
#define PWRUP_CTLUP1_GPIO_PIN   Pin06
#else
#define PWRUP_CTLUP1_GPIO_PORT  PortD
#define PWRUP_CTLUP1_GPIO_PIN   Pin14
#endif

#define PWRUP_CTLUP1_PIN_OPEN()  PORT_SetBits(PWRUP_CTLUP1_GPIO_PORT,PWRUP_CTLUP1_GPIO_PIN)
#define PWRUP_CTLUP1_PIN_CLOSE()  PORT_ResetBits(PWRUP_CTLUP1_GPIO_PORT,PWRUP_CTLUP1_GPIO_PIN)

#ifdef HW_VERSION_V1_1
#define PWRUP_CTLUP2_GPIO_PORT  PortD
#define PWRUP_CTLUP2_GPIO_PIN   Pin14
#else
#define PWRUP_CTLUP2_GPIO_PORT  PortC
#define PWRUP_CTLUP2_GPIO_PIN   Pin06
#endif
#define PWRUP_CTLUP2_PIN_OPEN()  PORT_SetBits(PWRUP_CTLUP2_GPIO_PORT,PWRUP_CTLUP2_GPIO_PIN)
#define PWRUP_CTLUP2_PIN_CLOSE()  PORT_ResetBits(PWRUP_CTLUP2_GPIO_PORT,PWRUP_CTLUP2_GPIO_PIN)

#ifdef HW_VERSION_V1_1
#define PWRSNR_GPIO_PORT  PortB
#define PWRSNR_GPIO_PIN   Pin07
#else
#define PWRSNR_GPIO_PORT  PortH
#define PWRSNR_GPIO_PIN   Pin02
#endif

#define PWRSNR_PIN_OPEN()  PORT_SetBits(PWRSNR_GPIO_PORT,PWRSNR_GPIO_PIN)
#define PWRSNR_PIN_CLOSE()  PORT_ResetBits(PWRSNR_GPIO_PORT,PWRSNR_GPIO_PIN)

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/




/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern void func_Mainloop_Init(void);
extern int func_Get_Flash_Data(char cDataType, int nStartAddr, int nLen, char *cDataArr);
extern int func_Set_Flash_Data(char cDataType);

extern void func_LowPower_Stop_Mode_Init(void);
extern void func_Enter_LowPower_Stop_Mode(void);
extern void func_System_Soft_Reset(void);

extern void func_Meas_Sensor_PowerOn_Init(void);
extern void func_Meas_Sensor_PowerDown_DeInit(void);

extern void func_System_Mainloop_Dispose(void);
extern void func_BD_Data_Dispose(void);
#ifdef __cplusplus
}
#endif

#endif /* __MAINLOOP_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
