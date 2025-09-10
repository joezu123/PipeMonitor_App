/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\BT_TB05\BT_TB05.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-04-03       Joe             First version
 @endverbatim

 */
#ifndef __BT_TB05_H__
#define __BT_TB05_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "drv_USART.h"
#include "User_Data.h"
/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#ifdef HW_VERSION_V1_1
#define PWRBLE_GPIO_PORT PortC
#define PWRBLE_GPIO_PIN  Pin02
#define PWRBLE_PIN_OPEN()  PORT_SetBits(PWRBLE_GPIO_PORT, PWRBLE_GPIO_PIN)
#define PWRBLE_PIN_CLOSE() PORT_ResetBits(PWRBLE_GPIO_PORT, PWRBLE_GPIO_PIN)
#else
#define PWRBLE_GPIO_PORT PortC
#define PWRBLE_GPIO_PIN  Pin03
#define PWRBLE_PIN_OPEN()  PORT_ResetBits(PWRBLE_GPIO_PORT, PWRBLE_GPIO_PIN)
#define PWRBLE_PIN_CLOSE() PORT_SetBits(PWRBLE_GPIO_PORT, PWRBLE_GPIO_PIN)
#endif



#ifdef HW_VERSION_V1_1
#define BTRST_GPIO_PORT  PortC
#define BTRST_GPIO_PIN   Pin01
#else
#define BTRST_GPIO_PORT  PortC
#define BTRST_GPIO_PIN   Pin02
#endif

#define BTRST_PIN_SET   PORT_SetBits(BTRST_GPIO_PORT, BTRST_GPIO_PIN)
#define BTRST_PIN_RESET PORT_ResetBits(BTRST_GPIO_PORT, BTRST_GPIO_PIN)

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/




/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern uint8_t drv_BT05_Module_Init(void);
extern void drv_BT05_SendData(uint8_t *ucDataArr, uint16_t usDataLen);
extern void func_BT05_PowerUp_Init(void);
extern void func_BT05_PowerDown_DeInit(void);

extern void func_BT_Dispose(void);

#ifdef __cplusplus
}
#endif

#endif /* __BT_TB05_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
