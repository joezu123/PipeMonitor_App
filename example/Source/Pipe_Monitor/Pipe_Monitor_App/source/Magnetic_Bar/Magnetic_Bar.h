/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\Magnetic_Bar\Magnetic_Barp.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-28       Joe             First version
 @endverbatim

 */
#ifndef __MAGNETIC_BAR_H__
#define __MAGNETIC_BAR_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ddl.h"

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define MAGNETIC_BAR_PORT	PortD
#define MAGNETIC_BAR_PIN	Pin07

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/




/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern uint8_t drv_Magnetic_Bar_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAGNETIC_BAR_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
