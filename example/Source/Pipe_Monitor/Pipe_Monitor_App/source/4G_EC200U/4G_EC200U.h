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


/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

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
