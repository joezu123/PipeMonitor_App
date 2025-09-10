/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\driver\drv_device\drv_LKT4202\drv_LKT4202.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-21       Joe             First version
 @endverbatim

 */
#ifndef __DRV_LKT4202_H__
#define __DRV_LKT4202_H__

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


/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/




/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern uint8_t drv_LKT4202_Init(void);
extern uint8_t drv_LKT4202_Send_EncryKEY(uint8_t* cKeyBuf);
extern uint8_t drv_LKT4202_Send_DecryKEY(uint8_t* cKeyBuf);
extern uint8_t drv_LKT4202_SendData_Encry(uint8_t *uBaseDataArr, char *uEnDataArr);
extern uint8_t drv_LKT4202_SendData_Decry(uint8_t *uBaseDataArr, char *uDeDataArr);
extern uint8_t drv_LKT4202_Create_MFFile(void);
extern uint8_t drv_LKT4202_Create_KeyFile(void);
extern uint8_t drv_LKT4202_Random_Test(void);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_LKT4202_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
