/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\OLED\OLED.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-04-02       Joe             First version
 @endverbatim

 */
#ifndef __OLED_H__
#define __OLED_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "drv_IIC.h"
/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#ifdef HW_VERSION_V1_1
#define PWRLCM_GPIO_PORT	PortE
#define PWRLCM_GPIO_PIN	  Pin11
#else
#define PWRLCM_GPIO_PORT	PortE
#define PWRLCM_GPIO_PIN	  Pin10
#endif

#define PWRLCM_PIN_OPEN()	PORT_SetBits(PWRLCM_GPIO_PORT,PWRLCM_GPIO_PIN)
#define PWRLCM_PIN_CLOSE()	PORT_ResetBits(PWRLCM_GPIO_PORT, PWRLCM_GPIO_PIN)

#define LCMRS_PORT	PortD
#define LCMRS_PIN	Pin04

#define LCMRS_PIN_SET	PORT_SetBits(LCMRS_PORT, LCMRS_PIN)
#define LCMRS_PIN_RESET	PORT_ResetBits(LCMRS_PORT, LCMRS_PIN)

#define LCMRST_PORT	PortD
#define LCMRST_PIN	Pin03

#define LCMRST_PIN_SET	PORT_SetBits(LCMRST_PORT, LCMRST_PIN)
#define LCMRST_PIN_RESET	PORT_ResetBits(LCMRST_PORT, LCMRST_PIN)

#define LCMCS_PORT	PortD
#define LCMCS_PIN	Pin02

#define LCMCS_PIN_SET	PORT_SetBits(LCMCS_PORT, LCMCS_PIN)
#define LCMCS_PIN_RESET	PORT_ResetBits(LCMCS_PORT, LCMCS_PIN)

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/




/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern void OLED_Write_CMD(uint8_t ucmd);
extern void OLED_Write_Data(uint8_t uData);
extern uint8_t drv_OLED_Init(void);
extern void OLED_Test(uint8_t ucType);
extern void func_display_string(uint8_t X_dot,uint8_t Y_dot,signed short *text);

extern void func_OLED_PowerUp_Init(void);
extern void func_OLED_PowerDown_DeInit(void);

extern void clear_screen();
extern void display_128x64(uint8_t *dp) ;
extern void func_Display_128x64(unsigned char ucType);

extern SystemPataSt *pst_OLEDSystemPara;
#ifdef __cplusplus
}
#endif

#endif /* __OLED_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
