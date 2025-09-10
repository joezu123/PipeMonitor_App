/**
 *******************************************************************************
 * @file  Pipe_Monitor_App\drivers\device_drv\drv_Timer.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-13       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "drv_Timer.h"
#include "hc32f460_timer4_oco.h"
#include "hc32f460_timer4_cnt.h"
#include "hc32f460_interrupts.h"
#include "User_Data.h"
#include "Mainloop.h"
#include "Magnetic_Bar.h"
#include "4G_EC200U.h"
#include "Display.h"
#include "OLED.h"
/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* Timer4 CNT */
#define TIMER4_UNIT                     (M4_TMR41)
#define TIMER4_CNT_CYCLE_VAL            (39000)        /* Timer4 counter cycle value */

/* Timer4 OCO */
#define TIMER4_OCO_HIGH_CH              (Timer4OcoOuh)  /* only Timer4OcoOuh  Timer4OcoOvh  Timer4OcoOwh */

/* Timer4 OCO interrupt number */
#define TIMER4_OCO_HIGH_CH_INT_NUM      (INT_TMR41_GCMUH)
/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
static SystemPataSt *pst_TimerSystemPara;
/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void OcoIrqCallback(void);
/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

static void OcoIrqCallback(void)
{
    #ifdef TIMER1_DISPOSE
    gc_SystemPosi = 20;
    //OLED_Test(6);
    TIMER4_OCO_ClearIrqFlag(TIMER4_UNIT, TIMER4_OCO_HIGH_CH);
    pst_TimerSystemPara->DeviceRunPara.cTimer4_1SEC_ReflashFlag = 1;
    gc_SystemPosi = 22;
    //OLED_Test(6);
    #else
    unsigned char ucLCDShowDelayCnt = 0 ;	//LCD界面显示延时计数器
    static unsigned char ucNFCShowCnt = 0;	//NFC认证通过界面显示计数器
    static unsigned char ucNFCShowFlag = 0;	//NFC认证通过界面显示标志位：1->显示，0->不显示
    static char cBarEventFinishFlag = 0;	//磁棒操作要执行的事件完成标志位:1->完成，0->未完成
    static char cServerEventFinishFlag = 0;	//服务器通讯事件完成标志位:1->完成，0->未完成
    static char cMeasSensorEventFinishFlag = 0;	//测量传感器事件完成标志位:1->完成，0->未完成
	static char cBTEventFinishFlag = 0;	//蓝牙事件完成标志位:1->完成，0->未完成
	static char cBTWaitConnectEventFinishFlag = 0;	//蓝牙等待连接事件完成标志位:1->完成，0->未完成
	static unsigned char ucBarLostCnt = 0;
    static char cInitFlag = 0;

    TIMER4_OCO_ClearIrqFlag(TIMER4_UNIT, TIMER4_OCO_HIGH_CH);

    pst_TimerSystemPara->DeviceRunPara.cTimer1sTriggerFlag = 1;

    if(pst_TimerSystemPara->DeviceRunPara.eCurPowerType == Power_ON)	//开机状态下
	{
		ucLCDShowDelayCnt = 19;//9;	//开机状态下，延时9秒
	}
	else	//关机状态下	
    {
        ucLCDShowDelayCnt = 3;
    }

	//磁棒状态检测
    if(pst_TimerSystemPara->DeviceRunPara.esDeviceSensorsData.cMagnetic_Bar_Status == 1)
    {
		//当前定时器事件是磁棒触摸事件
        cBarEventFinishFlag = 0;
    }
    else
    {
        cBarEventFinishFlag = 1;
    }

	//判断当前定时器是否由设备定时上传数据到服务器后等待服务器通讯延时所产生
    if(pst_TimerSystemPara->DeviceRunPara.cWaitServerCMDFlag == 1)
    {
        cServerEventFinishFlag = 0;
    }
    else
    {
        cServerEventFinishFlag = 1;
    }

	//判断当前定时器是否由测量传感器延时所产生
    if(pst_TimerSystemPara->DeviceRunPara.cMeasDelayFlag == 1)
    {
        cMeasSensorEventFinishFlag = 0;
    }
    else
    {
        cMeasSensorEventFinishFlag = 1;
    }

	//磁棒持续接触，显示开/关机界面
	if(PORT_GetBit(MAGNETIC_BAR_PORT,MAGNETIC_BAR_PIN) == Reset)	//设备当前磁棒保持接触状态
	{
		pst_TimerSystemPara->DeviceRunPara.cBarTouchCnt++;
		ucBarLostCnt = 0;
		pst_TimerSystemPara->DeviceRunPara.esDeviceSensorsData.cMagnetic_Bar_Status = 1;

        if(pst_TimerSystemPara->DeviceRunPara.eCurPowerType == Power_ON)	//开机状态下
        {
            //显示""关机中XS"
            func_Measure_WaterLevel_View_Show(pst_TimerSystemPara->DeviceRunPara.cBarTouchCnt);
            if(pst_TimerSystemPara->DeviceRunPara.cBarTouchCnt >= 5)
            {
                pst_TimerSystemPara->DeviceRunPara.cBarTouchCnt = 0;
                cBarEventFinishFlag = 1;
				pst_TimerSystemPara->DeviceRunPara.esDeviceSensorsData.cMagnetic_Bar_Status = 0;
                pst_TimerSystemPara->DeviceRunPara.eCurPowerType = Power_OFF;
                func_Enter_LowPower_Stop_Mode();
            }
        }
        else	//关机状态下
        {
            if(pst_TimerSystemPara->DeviceRunPara.ucOLEDInitFlag == 0)
            {
                func_OLED_PowerUp_Init();
                pst_TimerSystemPara->DeviceRunPara.ucOLEDInitFlag = 1;
            }
            func_PowerOn_View_Show(pst_TimerSystemPara->DeviceRunPara.cBarTouchCnt);
            if(pst_TimerSystemPara->DeviceRunPara.cBarTouchCnt >= 5)
            {
                func_System_Soft_Reset();
            }
        }
	}
	else	//磁棒断开接触状态
	{
		//判断之前磁棒是否为接触状态：磁棒唤醒事件
        if(pst_TimerSystemPara->DeviceRunPara.esDeviceSensorsData.cMagnetic_Bar_Status == 1)
        {
            pst_TimerSystemPara->DeviceRunPara.cBarTouchCnt = 0;
            ucBarLostCnt++;
            if(pst_TimerSystemPara->DevicePara.cMonitorMode == 1)
            {
                if(ucBarLostCnt >= 3)
                {
                    ucBarLostCnt = 0;
                    func_Measure_Water_Quality_View_Show(0);
                }
            }
            else
            {
                if(pst_TimerSystemPara->DeviceRunPara.eCurPowerType == Power_ON)	//开机状态下
                {
                    if((ucBarLostCnt % 3) == 0)	//3S触发
                    {
                        if(guc_NFC_Card_Flag == 0)	//当前未进行NFC认证或NFC认证不通过
                        {
                            if(pst_TimerSystemPara->DevicePara.cDeviceIdenFlag == 0) //当前设备未经过验证
                            {
                                func_Not_Certified_View_Show();
                            }
                            else	//设备经过NFC验证，显示需要激活
                            {
                                if(pst_TimerSystemPara->DeviceRunPara.cEveryNFCDisposeFlag == 0)
                                {
                                    cInitFlag = 0;
                                    func_diplay_NFCCheck_Menu();
                                }
                            }
                        }
                        else	//NFC认证通过，显示认证通过界面
                        {
                            if(pst_TimerSystemPara->DeviceRunPara.cEveryNFCDisposeFlag == 0)
                            {
                                func_display_Authorize_Menu();
                                ucNFCShowFlag = 1;
                            }
                            else
                            {
                                if(cInitFlag == 0)
                                {
                                    cInitFlag = 1;
                                    ucBarLostCnt = 0;
                                }
                                switch(pst_TimerSystemPara->DeviceRunPara.eShowView)
                                {
                                case SType_Meas_Level:
                                    func_Measure_WaterLevel_View_Show(0);
                                    break;
                                case SType_Meas_Water_Quality:
                                    func_Measure_Water_Quality_View_Show(0);
                                    break;
                                case SType_Meas_Sensor_Value:
                                    func_Meas_Sensor_Value_View_Show(0);
                                    break;
                                case SType_BD_Data:
                                    func_GPSData_View_Show(0);
                                    break;
                                case SType_Dev_ID_PD:
                                    func_Device_ID_PD_View_Show(0);
                                    break;
                                case SType_Dev_SW_HW:
                                    func_Device_SW_HW_View_Show(0);
                                    break;
                                default:
                                    func_Measure_WaterLevel_View_Show(0);
                                    break;
                                }
                                pst_TimerSystemPara->DeviceRunPara.eShowView++;
                                if(pst_TimerSystemPara->DeviceRunPara.eShowView >= SType_Max)
                                {
                                    //ucBarLostCnt = 100;
                                    pst_TimerSystemPara->DeviceRunPara.eShowView = SType_Meas_Level;
                                    //pst_TimerSystemPara->DeviceRunPara.cEveryNFCDisposeFlag = 0;
                                }
                            }
                        }
                    }
                }
                
                if(ucNFCShowFlag == 0)	//当前显示界面为正常测量界面或者NFC未经过认证界面
                {
                    if(ucBarLostCnt >= ucLCDShowDelayCnt)	//本轮显示界面已轮询完成
                    {
                        ucBarLostCnt = 0;
                        pst_TimerSystemPara->DeviceRunPara.esDeviceSensorsData.cMagnetic_Bar_Status = 0;
                        cBarEventFinishFlag = 1;
                        pst_TimerSystemPara->DeviceRunPara.cEveryNFCDisposeFlag = 1;
                        pst_TimerSystemPara->DeviceRunPara.eShowView = SType_Meas_Level;
                        if(pst_TimerSystemPara->DeviceRunPara.ucOLEDInitFlag == 1)
                        {
                            pst_TimerSystemPara->DeviceRunPara.ucOLEDInitFlag = 0;
                            func_OLED_PowerDown_DeInit(); //关闭OLED电源
                        }
                    }
                }
                else	//当前界面为NFC认证信息界面：认证通过界面
                {
                    ucNFCShowCnt++;
                    if(ucNFCShowCnt >= 2)	//2S后，NFC认证通过界面显示完成
                    {
                        ucNFCShowCnt = 0;
                        ucNFCShowFlag = 0;
                        guc_NFC_Card_Flag = 0;
                        ucBarLostCnt = 0;
                        cBarEventFinishFlag = 1;
                        pst_TimerSystemPara->DeviceRunPara.esDeviceSensorsData.cMagnetic_Bar_Status = 0;
                    }
                }
            }
            
        }
	}
	//磁棒状态检测结束

	//判断当前定时器是否由延时等待服务器通讯事件产生
    if(pst_TimerSystemPara->DeviceRunPara.cWaitServerCMDFlag == 1)
    {
        pst_TimerSystemPara->DeviceRunPara.cWaitServerCMDCnt++;
        if(pst_TimerSystemPara->DeviceRunPara.cWaitServerCMDCnt >= 30)
        {
            cServerEventFinishFlag = 1;
            pst_TimerSystemPara->DeviceRunPara.cWaitServerCMDCnt = 0;
            pst_TimerSystemPara->DeviceRunPara.cWaitServerCMDFlag = 0;
        }
    }

	//判断当前定时器是否由测量传感器延时等待事件产生
    if(pst_TimerSystemPara->DeviceRunPara.cMeasDelayFlag == 1)
    {
        pst_TimerSystemPara->DeviceRunPara.cMeasDelayCnt++;
        if(pst_TimerSystemPara->DeviceRunPara.cMeasDelayCnt >= 20)
        {
            if(pst_TimerSystemPara->DevicePara.cMonitorMode == 0)
            {
                cMeasSensorEventFinishFlag = 1;
                pst_TimerSystemPara->DeviceRunPara.cMeasDelayCnt = 0;
                pst_TimerSystemPara->DeviceRunPara.cMeasDelayFlag = 0;
            }
            else
            {
                pst_TimerSystemPara->DeviceRunPara.cMeasDelayCnt = 5;
            }
        }
    }

	//判断当前定时器是否由蓝牙通讯事件产生
	if(pst_TimerSystemPara->DeviceRunPara.cDeviceBTConnectFlag == 1)
	{
		pst_TimerSystemPara->DeviceRunPara.usDeviceBTWaitCnt++;
		if(pst_TimerSystemPara->DeviceRunPara.usDeviceBTWaitCnt >= 180)	//等待180s
		{
			cBTEventFinishFlag = 1;
			pst_TimerSystemPara->DeviceRunPara.cDeviceBTConnectFlag = 0;
			pst_TimerSystemPara->DeviceRunPara.usDeviceBTWaitCnt = 0;
		}
		else
		{
			cBTEventFinishFlag = 0;
		}
	}
	else
	{
		cBTEventFinishFlag = 1;
	}

	//判断当前定时器是否由蓝牙等待连接事件产生
	if(pst_TimerSystemPara->DeviceRunPara.cDeviceBTWaitConnectFlag == 1)
	{
		pst_TimerSystemPara->DeviceRunPara.usDeviceBTWaitConnectCnt++;
		if(pst_TimerSystemPara->DeviceRunPara.usDeviceBTWaitConnectCnt >= pst_TimerSystemPara->DeviceRunPara.usDeviceBTWaitConnectMaxCnt)	//等待20s
		{
			cBTWaitConnectEventFinishFlag = 1;
			pst_TimerSystemPara->DeviceRunPara.cDeviceBTWaitConnectFlag = 0;
			pst_TimerSystemPara->DeviceRunPara.usDeviceBTWaitConnectCnt = 0;
            if(pst_TimerSystemPara->DeviceRunPara.cDebugModel == 1)
            {
                pst_TimerSystemPara->DeviceRunPara.cDebugModel = 0;
            }
		}
		else
		{
			cBTWaitConnectEventFinishFlag = 0;
		}
	}
	else
	{
		cBTWaitConnectEventFinishFlag = 1;
	}

    //pst_TimerSystemPara->DeviceRunPara.c4GTimerCnt++;
	//当前引起定时器产生的事件均已处理完后，关闭定时器，退出诊断模式
    if((cBarEventFinishFlag == 1) && (cServerEventFinishFlag == 1) && (cMeasSensorEventFinishFlag == 1) 
		&& (pst_TimerSystemPara->DeviceRunPara.cConnectServerFlag == 0) && (cBTEventFinishFlag == 1)
		&& (cBTWaitConnectEventFinishFlag == 1))
    {
        drv_mcu_Timer4_Stop();
        pst_TimerSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_RUN;
    }
    #endif
}

void drv_mcu_Timer4_Start(void)
{
	/* Start CNT */
    pst_TimerSystemPara->DeviceRunPara.cTimer4StartFlag = 1;
    TIMER4_CNT_ClearCountVal(TIMER4_UNIT);
    TIMER4_CNT_Start(TIMER4_UNIT);
}

void drv_mcu_Timer4_Stop(void)
{
	TIMER4_CNT_ClearCountVal(TIMER4_UNIT);
    TIMER4_CNT_Stop(TIMER4_UNIT);
	pst_TimerSystemPara->DeviceRunPara.cTimer4StartFlag = 0;
}

void drv_mcu_Timer4_OCD_Init(void)
{
	stc_irq_regi_conf_t stcIrqRegiCfg;
    stc_timer4_cnt_init_t stcCntInit;
    stc_timer4_oco_init_t stcOcoInit;
    stc_oco_high_ch_compare_mode_t stcHighChCmpMode;
    uint16_t OcoHighChOccrVal = TIMER4_CNT_CYCLE_VAL / 2u;

    /* Clear structures */
    MEM_ZERO_STRUCT(stcCntInit);
    MEM_ZERO_STRUCT(stcOcoInit);
    MEM_ZERO_STRUCT(stcIrqRegiCfg);
    MEM_ZERO_STRUCT(stcHighChCmpMode);

    /* Enable peripheral clock */
    PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM41 | PWC_FCG2_PERIPH_TIM42 | PWC_FCG2_PERIPH_TIM43, Enable);

    /* Timer4 CNT : Initialize CNT configuration structure */
    stcCntInit.enBufferCmd = Disable;
    stcCntInit.enClk = Timer4CntPclk;
    stcCntInit.enClkDiv = Timer4CntPclkDiv1024;   /* CNT clock divide */
    stcCntInit.enCntMode = Timer4CntSawtoothWave;
    stcCntInit.enZeroIntMsk = Timer4CntIntMask0;
    stcCntInit.enPeakIntMsk = Timer4CntIntMask0;
    TIMER4_CNT_Init(TIMER4_UNIT, &stcCntInit);                      /* Initialize CNT */
    TIMER4_CNT_SetCycleVal(TIMER4_UNIT, TIMER4_CNT_CYCLE_VAL);      /* Set CNT Cycle value */

    /* Timer4 OCO : Initialize OCO channel configuration structure */
    stcOcoInit.enOcoIntCmd = Enable;
    stcOcoInit.enPortLevel = OcPortLevelLow;
    stcOcoInit.enOcmrBufMode = OcmrBufDisable;
    stcOcoInit.enOccrBufMode = OccrBufDisable;
    TIMER4_OCO_Init(TIMER4_UNIT, TIMER4_OCO_HIGH_CH, &stcOcoInit);  /* Initialize OCO high channel */

    if (!(TIMER4_OCO_HIGH_CH % 2))
    {
        /* ocmr[15:0] = 0x0FFF     0000 1111 1111 1111   */
        stcHighChCmpMode.enCntZeroMatchOpState = OcoOpOutputReverse;     /* Bit[11:10]  11 */
        stcHighChCmpMode.enCntZeroNotMatchOpState = OcoOpOutputHold;     /* Bit[15:14]  00 */
        stcHighChCmpMode.enCntUpCntMatchOpState = OcoOpOutputReverse;    /* Bit[9:8]    11 */
        stcHighChCmpMode.enCntPeakMatchOpState = OcoOpOutputReverse;     /* Bit[7:6]    11 */
        stcHighChCmpMode.enCntPeakNotMatchOpState = OcoOpOutputHold;     /* Bit[13:12]  00 */
        stcHighChCmpMode.enCntDownCntMatchOpState = OcoOpOutputReverse;  /* Bit[5:4]    11 */

        stcHighChCmpMode.enCntZeroMatchOcfState = OcoOcfSet;     /* bit[3] 1 */
        stcHighChCmpMode.enCntUpCntMatchOcfState = OcoOcfSet;    /* bit[2] 1 */
        stcHighChCmpMode.enCntPeakMatchOcfState = OcoOcfSet;     /* bit[1] 1 */
        stcHighChCmpMode.enCntDownCntMatchOcfState = OcoOcfSet;  /* bit[0] 1 */

        stcHighChCmpMode.enMatchConditionExtendCmd = Disable;

        TIMER4_OCO_SetHighChCompareMode(TIMER4_UNIT, TIMER4_OCO_HIGH_CH, &stcHighChCmpMode);  /* Set OCO high channel compare mode */
    }

    /* Set OCO compare value */
    TIMER4_OCO_WriteOccr(TIMER4_UNIT, TIMER4_OCO_HIGH_CH, OcoHighChOccrVal);

    /* Enable OCO */
    TIMER4_OCO_OutputCompareCmd(TIMER4_UNIT, TIMER4_OCO_HIGH_CH, Enable);

    /* Set Timer4 OCO IRQ */
    MEM_ZERO_STRUCT(stcIrqRegiCfg);
    stcIrqRegiCfg.enIRQn = Int012_IRQn;
    stcIrqRegiCfg.pfnCallback = &OcoIrqCallback;
    stcIrqRegiCfg.enIntSrc = TIMER4_OCO_HIGH_CH_INT_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    
}

/**
 * @brief  Timer Initialize.
 * @param  None
 * @retval None
 */
void drv_mcu_Timer_Init(void)
{
    drv_mcu_Timer0_Init();
	drv_mcu_Timer4_OCD_Init();
    pst_TimerSystemPara = GetSystemPara();
}

/**
 * @brief  Timer0 Initialize.
 * @param  None
 * @retval None
 */     
void drv_mcu_Timer0_Init(void)
{
	stc_tim0_base_init_t stcTimerCfg;
	stc_tim0_trigger_init_t stcTimer0TrigInit;

	MEM_ZERO_STRUCT(stcTimerCfg);
	MEM_ZERO_STRUCT(stcTimer0TrigInit);

	/* Enable Timer0 peripheral clock */
	PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM01 | PWC_FCG2_PERIPH_TIM02, Enable);

	/* Clear CNTAR register for channel A/B */
	TIMER0_WriteCntReg(M4_TMR01, Tim0_ChannelA, 0u);	//USART1 对应的定时器通道
    TIMER0_WriteCntReg(M4_TMR01, Tim0_ChannelB, 0u);	//USART2 对应的定时器通道
    TIMER0_WriteCntReg(M4_TMR02, Tim0_ChannelA, 0u);	//USART3 对应的定时器通道
    TIMER0_WriteCntReg(M4_TMR02, Tim0_ChannelB, 0u);	//USART4 对应的定时器通道

	/* Timer0 peripheral function configuration */
	/*计算公式： 超时时间=cmpvalue * (1/Clocksource) * clockdiv
	example: 1600us = 8000 * (1/40M) * 8 */
	stcTimerCfg.Tim0_ClockDivision = Tim0_ClkDiv8;//Tim0_ClkDiv8;
	stcTimerCfg.Tim0_SyncClockSource = Tim0_Pclk1;
	//stcTimerCfg.Tim0_AsyncClockSource = Tim0AsyncClkDisable;
	stcTimerCfg.Tim0_CounterMode = Tim0_Async;
	stcTimerCfg.Tim0_CmpValue = 8000;

	TIMER0_BaseInit(M4_TMR01, Tim0_ChannelA, &stcTimerCfg);
	stcTimerCfg.Tim0_CounterMode = Tim0_Sync;
	TIMER0_BaseInit(M4_TMR01, Tim0_ChannelB, &stcTimerCfg);
	TIMER0_BaseInit(M4_TMR02, Tim0_ChannelA, &stcTimerCfg);
	TIMER0_BaseInit(M4_TMR02, Tim0_ChannelB, &stcTimerCfg);

	/*Clear compare flag */
	TIMER0_ClearFlag(M4_TMR01, Tim0_ChannelA);
	TIMER0_ClearFlag(M4_TMR01, Tim0_ChannelB);
	TIMER0_ClearFlag(M4_TMR02, Tim0_ChannelA);
	TIMER0_ClearFlag(M4_TMR02, Tim0_ChannelB);

	/*Config Timer0 hardware trigger configuration */
	//stcTimer0TrigInit.Tim0_OCMode = Tim0OcInvalid;
	//stcTimer0TrigInit.Tim0_SelTrigSrc = EVT_TMR0_0_CMP;
	stcTimer0TrigInit.Tim0_InTrigEnable = false;
	stcTimer0TrigInit.Tim0_InTrigClear = true;
	stcTimer0TrigInit.Tim0_InTrigStop = false;
	stcTimer0TrigInit.Tim0_InTrigStart = true;

	TIMER0_HardTriggerInit(M4_TMR01, Tim0_ChannelA, &stcTimer0TrigInit);
	TIMER0_HardTriggerInit(M4_TMR01, Tim0_ChannelB, &stcTimer0TrigInit);
	TIMER0_HardTriggerInit(M4_TMR02, Tim0_ChannelA, &stcTimer0TrigInit);
	TIMER0_HardTriggerInit(M4_TMR02, Tim0_ChannelB, &stcTimer0TrigInit);
}
/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
