/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\Magnetic_Bar\Magnetic_Bar.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-28       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "Magnetic_Bar.h"
#include "hc32f460_gpio.h"
#include "hc32f460_exint_nmi_swi.h"
#include "hc32f460_interrupts.h"
#include "User_Data.h"
#include "hc32f460.h"
#include "hc32f460_pwc.h"
#include "OLED.h"
#include "Display.h"
#include "Mainloop.h"
#include "drv_Timer.h"
#include "drv_USART.h"

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
static SystemPataSt *pst_MagSystemPara;
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
void ExtInt07_Callback(void)
{
    //static unsigned char ucCnt = 0;
    //gus_BarCnt++;
    /* Recover clock. */
    PWC_IrqClkRecover();
 
    if (Set == EXINT_IrqFlgGet(ExtiCh07))
    {
        //gus_BarCnt1++;
        pst_MagSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_DIAG;
        /* clear int request flag */
        EXINT_IrqFlgClr(ExtiCh07);

        #if 0
        if(pst_MagSystemPara->DevicePara.cDeviceIdenFlag == 1)  //设备认证通过后，才需要进行磁棒唤醒采集发送数据操作
        {
            if(pst_MagSystemPara->DeviceRunPara.cBarTouchFlag == 0)
            {
                pst_MagSystemPara->DeviceRunPara.cBarTouchFlag = 1;
            }
        }
        #endif
        
        if(pst_MagSystemPara->DeviceRunPara.esDeviceSensorsData.cMagnetic_Bar_Status == 0)
        //&& (pst_MagSystemPara->DeviceRunPara.eCurPowerType == Power_ON))
        {
            guc_NFC_Card_Flag = 0;
            guc_NFCPWRInitFlag = 0;    //NFC模块开启电源及初始化配置标志位
            pst_MagSystemPara->DeviceRunPara.cEveryNFCDisposeFlag= 0;
            pst_MagSystemPara->DeviceRunPara.esDeviceSensorsData.cMagnetic_Bar_Status = 1;
            drv_mcu_ChangeUSART3_Source(MODULE_NFC_RFID);
            #ifndef HW_VERSION_V1_1
            MAINPWR3V8_PIN_CLOSE();	//关闭3.8V电源
            Ddl_Delay1ms(100);
            MAINPWR3V8_PIN_OPEN();	//打开3.8V电源
            #endif

            func_OLED_PowerUp_Init();
            pst_MagSystemPara->DeviceRunPara.ucOLEDInitFlag = 1;
            func_display_PowerOn_Menu();
            //OLED_Test(6);
            if(pst_MagSystemPara->DeviceRunPara.cTimer4StartFlag == 0)
            {
                drv_mcu_Timer4_Start();
            }
        }
    }
    
    /* Switch system clock as MRC. */
    PWC_IrqClkBackup();
    //gc_SystemPosi = 21;
    //OLED_Test(6);
}

uint8_t drv_Magnetic_Bar_Init(void)
{
	stc_exint_config_t stcExtiConfig;
    stc_irq_regi_conf_t stcIrqRegiConf;
    stc_port_init_t stcPortInit;

    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcExtiConfig);
    MEM_ZERO_STRUCT(stcIrqRegiConf);
    MEM_ZERO_STRUCT(stcPortInit);

	pst_MagSystemPara = GetSystemPara();

    /**************************************************************************/
    /* External Int Ch.7                                                      */
    /**************************************************************************/
    stcExtiConfig.enExitCh = ExtiCh07;

    /* Filter setting */
    stcExtiConfig.enFilterEn = Enable;
    stcExtiConfig.enFltClk = Pclk3Div8;
    stcExtiConfig.enExtiLvl = ExIntFallingEdge;
    EXINT_Init(&stcExtiConfig); 

    /* Set External Int */
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enExInt = Enable;
    PORT_Init(MAGNETIC_BAR_PORT, MAGNETIC_BAR_PIN, &stcPortInit);

    /* Select External Int Ch.7 */
    stcIrqRegiConf.enIntSrc = INT_PORT_EIRQ7;

    /* Register External Int to Vect.No.000 */
    stcIrqRegiConf.enIRQn = Int011_IRQn;

    /* Callback function */
    stcIrqRegiConf.pfnCallback = &ExtInt07_Callback;

    /* Registration IRQ */
    enIrqRegistration(&stcIrqRegiConf);

    /* Clear pending */
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);

    /* Set priority */
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_00);

    /* Enable NVIC */
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

	return 0;
}


/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
