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
#ifndef __DRV_ADC_H__
#define __DRV_ADC_H__

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
#include "hc32f460_adc.h"
/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#ifdef HW_VERSION_V1_1
#define BATTERY_PWRCHK_GPIO_PORT		PortB
#define BATTERY_PWRCHK_GPIO_PIN		Pin12
#else
#define BATTERY_PWRCHK_GPIO_PORT		PortC
#define BATTERY_PWRCHK_GPIO_PIN		Pin09
#endif
#define BATTERY_PWRCHK_OPEN()			(PORT_SetBits(BATTERY_PWRCHK_GPIO_PORT, BATTERY_PWRCHK_GPIO_PIN))
#define BATTERY_PWRCHK_CLOSE()		(PORT_ResetBits(BATTERY_PWRCHK_GPIO_PORT, BATTERY_PWRCHK_GPIO_PIN))


/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/




/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern uint8_t drv_mcu_ADC_Init();
extern uint8_t drv_Water_Immersion_Sensor_Init();
extern uint8_t drv_Battery_Level_Init();
extern uint8_t drv_Get_Water_Immersion_Sensor_Status(char* cDevStatus, float * fValue);
extern float drv_Get_Battery_Level_Value(float* fValue);

extern void func_ADC_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_ADC_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
