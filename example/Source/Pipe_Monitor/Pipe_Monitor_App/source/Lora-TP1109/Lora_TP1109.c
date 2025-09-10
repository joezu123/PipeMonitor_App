/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\Lora_TP1109\Lora_TP1109.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-07-16       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "Lora_TP1109.h"
#include "hc32f460_utility.h"
#include "User_Data.h"
#include "hc32f460_exint_nmi_swi.h"
/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static SystemPataSt *pst_LoraSystemPara;
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
 /**
 *******************************************************************************
 ** \brief ExtInt07 callback function
 **
 ** \param  None
 **
 ** \retval None
 **
 ******************************************************************************/
void ExtInt13_Callback(void)
{
    //static unsigned char ucCnt = 0;
    //gus_BarCnt++;
    /* Recover clock. */
    PWC_IrqClkRecover();
 
    if (Set == EXINT_IrqFlgGet(ExtiCh13))
    {
        
    }
    
    /* Switch system clock as MRC. */
    PWC_IrqClkBackup();
}


void drv_Lora_TP1109_Init(void)
{
	stc_port_init_t stcPortInit;
	stc_exint_config_t stcExtiConfig;
    stc_irq_regi_conf_t stcIrqRegiConf;
    /* configure structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);
	MEM_ZERO_STRUCT(stcExtiConfig);
    MEM_ZERO_STRUCT(stcIrqRegiConf);

    /* Flash NSS */
    stcPortInit.enPinMode = Pin_Mode_Out;
	
    PORT_Init(PWRNUB_GPIO_PORT, PWRNUB_GPIO_PIN, &stcPortInit);
	PORT_Init(NUBWAKE_GPIO_PORT, NUBWAKE_GPIO_PIN, &stcPortInit);
	PORT_Init(NUBRST_GPIO_PORT, NUBRST_GPIO_PIN, &stcPortInit);
	pst_LoraSystemPara = GetSystemPara();

	stcExtiConfig.enExitCh = ExtiCh13;

    /* Filter setting */
    stcExtiConfig.enFilterEn = Enable;
    stcExtiConfig.enFltClk = Pclk3Div8;
    stcExtiConfig.enExtiLvl = ExIntFallingEdge;
    EXINT_Init(&stcExtiConfig); 

    /* Set External Int */
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enExInt = Enable;
    PORT_Init(NUBINT_GPIO_PORT, NUBINT_GPIO_PIN, &stcPortInit);

    /* Select External Int Ch.7 */
    stcIrqRegiConf.enIntSrc = INT_PORT_EIRQ13;

    /* Register External Int to Vect.No.000 */
    stcIrqRegiConf.enIRQn = Int013_IRQn;

    /* Callback function */
    stcIrqRegiConf.pfnCallback = &ExtInt13_Callback;

    /* Registration IRQ */
    enIrqRegistration(&stcIrqRegiConf);

    /* Clear pending */
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);

    /* Set priority */
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_00);

    /* Enable NVIC */
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

    //pst_LoraSystemPara = GetSystemPara();

	//drv_Lora_TP1109_Reset();
    PWRNUB_GPIO_CLOSE();
    NUBWAKE_GPIO_CLOSE();
    NUBRST_GPIO_CLOSE();    
}

void drv_Lora_TP1109_PowerOn(void)
{
	PWRNUB_GPIO_CLOSE();	//关闭传感器电源
	Ddl_Delay1ms(100);
	PWRNUB_GPIO_OPEN();	//打开传感器电源
}

void drv_Lora_TP1109_Reset(void)
{
	NUBRST_GPIO_CLOSE();
	Ddl_Delay1ms(10);
	NUBRST_GPIO_OPEN();	//打开Nubia模块复位
}

//模块AT指令操作
void func_Lora_TP1109_AT_Command(char *cCmd, char *cResp, uint16_t usRespLen, uint16_t usTimeout)
{
	uint8_t ucSendBuf[100] = {0};
	uint16_t usSendDataLen = 0;
	//发送AT指令
	drv_mcu_USART_SendData(MODULE_4G_NB, ucSendBuf, usSendDataLen);
}
/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
