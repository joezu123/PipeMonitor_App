/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\Lora_TP1109\Lora_TP1109.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-04-03       Joe             First version
 @endverbatim

 */
#ifndef __LORA_TP1109_H__
#define __LORA_TP1109_H__

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
#define PWRNUB_GPIO_PORT        PortA
#define PWRNUB_GPIO_PIN         Pin10

#define PWRNUB_GPIO_OPEN()           (PORT_SetBits(PWRNUB_GPIO_PORT, PWRNUB_GPIO_PIN))
#define PWRNUB_GPIO_CLOSE()          (PORT_ResetBits(PWRNUB_GPIO_PORT, PWRNUB_GPIO_PIN))

#define NUBWAKE_GPIO_PORT        PortC
#define NUBWAKE_GPIO_PIN         Pin07

#define NUBWAKE_GPIO_OPEN()           (PORT_SetBits(NUBWAKE_GPIO_PORT, NUBWAKE_GPIO_PIN))
#define NUBWAKE_GPIO_CLOSE()          (PORT_ResetBits(NUBWAKE_GPIO_PORT, NUBWAKE_GPIO_PIN))

#define NUBRST_GPIO_PORT        PortD
#define NUBRST_GPIO_PIN         Pin15

#define NUBRST_GPIO_OPEN()           (PORT_SetBits(NUBRST_GPIO_PORT, NUBRST_GPIO_PIN))
#define NUBRST_GPIO_CLOSE()          (PORT_ResetBits(NUBRST_GPIO_PORT, NUBRST_GPIO_PIN))

#define NUBINT_GPIO_PORT        PortD
#define NUBINT_GPIO_PIN         Pin13

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/




/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern void drv_Lora_TP1109_Init(void);
extern void drv_Lora_TP1109_PowerOn(void);
extern void drv_Lora_TP1109_Reset(void);
extern void func_Lora_TP1109_AT_Command(char *cCmd, char *cResp, uint16_t usRespLen, uint16_t usTimeout);
#ifdef __cplusplus
}
#endif

#endif /* __LORA_TP1109_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
