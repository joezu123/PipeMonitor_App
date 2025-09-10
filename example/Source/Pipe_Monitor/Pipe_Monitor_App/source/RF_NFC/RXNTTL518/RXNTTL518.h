/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\RF_NFC\RXNTTL518.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-31       Joe             First version
 @endverbatim

 */
#ifndef __RXNTTL518_H__
#define __RXNTTL518_H__

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
extern unsigned char func_RXNTTL518_Init(void);
extern unsigned char func_Search_Card(void);

#ifdef __cplusplus
}
#endif

#endif /* __RF_NFC_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
