/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\driver\drv_device\drv_SC7A20H\drv_SC7A20H.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-21       Joe             First version
 @endverbatim

 */
#ifndef __DRV_SC7A20H_H__
#define __DRV_SC7A20H_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "User_Data.h"
#include "drv_IIC.h"
/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define SCINT1_GPIO_PORT  PortB
#define SCINT1_GPIO_PIN   Pin04

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/




/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern uint8_t drv_Attitude_Sensor_SC7A20H_Init(void);
extern uint8_t func_SC7A20H_Read_FIFO_Buf(int *x_buf,int *y_buf,int *z_buf);
extern int SC7A20_Task(void);
#ifdef __cplusplus
}
#endif

#endif /* __DRV_SC7A20H_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
