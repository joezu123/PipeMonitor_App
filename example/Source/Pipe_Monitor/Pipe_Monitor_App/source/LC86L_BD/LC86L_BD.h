/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\LC86L_BD\LC86L_BD.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-04-03       Joe             First version
 @endverbatim

 */
#ifndef __LC86L_BD_H__
#define __LC86L_BD_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "drv_USART.h"

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
extern uint8_t drv_LC86L_BD_Init(void);
extern void NMEA_GPRMC_Analysis(uint8_t *buf);
extern void TurnOff_LC86L_BD_Power(void);
extern void TurnOn_LC86L_BD_Power(void);
extern void func_BD_Test(void);
extern void func_BD_PowerUp_Init(void);
extern void func_BD_PownDown_Deinit(void);
#ifdef __cplusplus
}
#endif

#endif /* __LC86L_BD_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
