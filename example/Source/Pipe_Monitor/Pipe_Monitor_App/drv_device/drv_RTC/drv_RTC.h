/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\driver\drv_device\drv_RTC\drv_RTC.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-17       Joe             First version
 @endverbatim

 */
#ifndef __DRV_RTC_H__
#define __DRV_RTC_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "User_Data.h"
#include "hc32f460_rtc.h"
#include "hc32f460_interrupts.h"
#include "hc32f460.h"
#include "time.h"
/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/


/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/




/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern int drv_mcu_RTC_Config(void);
extern int drv_mcu_Set_RTC_Time(char *cCurDateTime);
extern int drv_mcu_Get_RTC_Time(char *cCurDateTime);
extern int drv_mcu_Get_RTC_Minute(int *nSecond);
extern time_t func_Get_Linux_Time_Sec(void);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_RTC_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
