/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\WatchDog\WatchDog.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.复位电阻51K，对应复位时间为373.162144s，约6分钟
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-31       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "WatchDog.h"
#include "hc32f460_gpio.h"
#include "hc32f460_exint_nmi_swi.h"
#include "hc32f460_interrupts.h"
#include "User_Data.h"
#include "hc32f460.h"
/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define WATCHDOG_WAKE_PORT	PortB
#define WATCHDOG_WAKE_PIN	Pin07


/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
//static SystemPataSt *pst_MagSystemPara;
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
#if 0
 /**
 *******************************************************************************
 ** \brief ExtInt07 callback function
 **
 ** \param  None
 **
 ** \retval None
 **
 ******************************************************************************/
void ExtInt08_Callback(void)
{
    if (Set == EXINT_IrqFlgGet(ExtiCh08))
    {
        PORT_SetBits(WATCHDOG_DONE_PORT,WATCHDOG_DONE_PIN);
        Ddl_Delay1ms(10);
        PORT_ResetBits(WATCHDOG_DONE_PORT,WATCHDOG_DONE_PIN);
        /* clear int request flag */
        EXINT_IrqFlgClr(ExtiCh08);
    }
}
#endif

uint8_t drv_WatchDog_Init(void)
{
    stc_port_init_t stcPortInit;
    #if 0
	stc_exint_config_t stcExtiConfig;
    stc_irq_regi_conf_t stcIrqRegiConf;
    

    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcExtiConfig);
    MEM_ZERO_STRUCT(stcIrqRegiConf);
    MEM_ZERO_STRUCT(stcPortInit);

    /**************************************************************************/
    /* External Int Ch.7                                                      */
    /**************************************************************************/
    stcExtiConfig.enExitCh = ExtiCh08;

    /* Filter setting */
    stcExtiConfig.enFilterEn = Enable;
    stcExtiConfig.enFltClk = Pclk3Div8;
    stcExtiConfig.enExtiLvl = ExIntFallingEdge;
    EXINT_Init(&stcExtiConfig);

    /* Set External Int */
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enExInt = Enable;
    PORT_Init(WATCHDOG_WAKE_PORT, WATCHDOG_WAKE_PIN, &stcPortInit);

    /* Select External Int Ch.7 */
    stcIrqRegiConf.enIntSrc = INT_PORT_EIRQ8;

    /* Register External Int to Vect.No.000 */
    stcIrqRegiConf.enIRQn = Int001_IRQn;

    /* Callback function */
    stcIrqRegiConf.pfnCallback = &ExtInt08_Callback;

    /* Registration IRQ */
    enIrqRegistration(&stcIrqRegiConf);

    /* Clear pending */
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);

    /* Set priority */
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_01);

    /* Enable NVIC */
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
    #endif
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Disable;
    PORT_Init(WATCHDOG_DONE_PORT, WATCHDOG_DONE_PIN, &stcPortInit);
	return 0;
}

void func_WatchDog_Refresh(void)
{
    PORT_SetBits(WATCHDOG_DONE_PORT,WATCHDOG_DONE_PIN);
    Ddl_Delay1ms(10);
    PORT_ResetBits(WATCHDOG_DONE_PORT,WATCHDOG_DONE_PIN);
}


/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
