/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\driver\drv_device\drv_ADC\drv_ADC.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-28       Joe             First version
 @endverbatim

 */
#ifndef __DRV_OTS_TEMPERATURE_H__
#define __DRV_OTS_TEMPERATURE_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ddl.h"
#include "hc32f460_ots.h"
/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/


/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/




/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/
extern void drv_OTS_Temperature_Init(void);
extern void func_OTS_Temperature_GetValue(float *fValue);
/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* __DRV_ADC_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
