/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\OLED\FONT.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-04-23       Joe             First version
 @endverbatim

 */
#ifndef __FONT_H__
#define __FONT_H__

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


/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/




/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/
extern const signed char g_sSKLogo[1024];
extern const signed char g_sSKLogo_New[1024];
extern const signed char g_sint8_Font_16x10_12864[72][20];
extern const signed char g_sint8_Font_16x12_12864[64][24] ;
extern const signed char g_sint8_Font_6x12_AsciiDot[102][12];
/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern void func_sprintf_metricsystem(signed short * buffer, float fb5h,char fb9h,char fbah);

#ifdef __cplusplus
}
#endif

#endif /* __FONT_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
