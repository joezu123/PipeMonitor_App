/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\WatchDog\WatchDog.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-31       Joe             First version
 @endverbatim

 */
#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ddl.h"
#include "User_Data.h"
/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#ifdef HW_VERSION_V1_1
#define WATCHDOG_DONE_PORT  PortC
#define WATCHDOG_DONE_PIN   Pin10
#else
#define WATCHDOG_DONE_PORT  PortB
#define WATCHDOG_DONE_PIN   Pin08
#endif
/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/




/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern uint8_t drv_WatchDog_Init(void);
extern void func_WatchDog_Refresh(void);
#ifdef __cplusplus
}
#endif

#endif /* __WATCHDOG_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
