/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\RF_NFC\RF_NFC.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-31       Joe             First version
 @endverbatim

 */
#ifndef __RF_NFC_H__
#define __RF_NFC_H__

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

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/




/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/
extern uint8_t guc_NFC_ID[20][5];

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern uint8_t drv_RF_NFC_Module_Init(void);
//extern void drv_RF_NFC_SendData(uint8_t ucCMD, uint8_t ucData, uint16_t usDataLen);
//extern void drv_RF_NFC_ReceiveData(uint8_t ucCMD, uint8_t *ucDataArr, uint16_t usDataLen);
extern uint8_t drv_RF_NFC_MCU_TO_PCD_Test(void);
extern uint8_t func_FindCar(void);
extern uint8_t TyteB_Test (void);
extern void func_RF_NFC_PowerUp_Init(void);
extern void func_RF_NFC_PowerDown_DeInit(void);
#ifdef __cplusplus
}
#endif

#endif /* __RF_NFC_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
