/**
 *******************************************************************************
 * @file  Pipe_Monitor_App\drivers\device_drv\drv_Timer.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-13       Joe             First version
 @endverbatim

 */
#ifndef __DRV_TIMER_H__
#define __DRV_TIMER_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32f460_gpio.h"
#include "hc32f460_timer0.h"
#include "hc32f460_pwc.h"
#include "hc32f460.h"

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern void drv_mcu_Timer_Init(void);
extern void drv_mcu_Timer0_Init(void);
extern void drv_mcu_Timer4_OCD_Init(void);
extern void drv_mcu_Timer4_Start(void);
extern void drv_mcu_Timer4_Stop(void);


#ifdef __cplusplus
}
#endif

#endif /* __DRV_TIMER_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
