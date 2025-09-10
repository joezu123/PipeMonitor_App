/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\driver\drv_device\drv_RTC\drv_RTC.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-17       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "drv_RTC.h"
#include "stdio.h"
#include "string.h"
#include "hc32f460_pwc.h"
#include "hc32f460_clk.h"
#include "system_hc32f460.h"
#include "hc32f460_utility.h"
#include "Mainloop.h"
#include "math.h"
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
static SystemPataSt *pst_RTCSystemPara;
static const float EPSINON = 0.000001f;
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

 /**
 *******************************************************************************
 ** \brief Rtc period callback function
 **
 ** \param [in]  None
 **
 ** \retval None
 **
 ** \note This function is called when the rtc period interrupt occurs
 ******************************************************************************/
void RtcPeriod_IrqCallback(void)
{
    //static uint16_t u16Cnt = 0;
    unsigned short usTime = pst_RTCSystemPara->DeviceRunPara.cDevUploadStatusTime * 60;
    /* Recover clock. */
    PWC_IrqClkRecover();
    if(pst_RTCSystemPara->DeviceRunPara.eCurPowerType == Power_ON)	//开机状态下
    {
        #ifndef HW_VERSION_V1_1
        MAINPWR3V8_PIN_CLOSE();	//关闭3.8V电源
        Ddl_Delay1ms(100);
        MAINPWR3V8_PIN_OPEN();	//打开3.8V电源
        #endif

        pst_RTCSystemPara->DeviceRunPara.cRTC_1MIN_ReflashFlag = 1;
        if(pst_RTCSystemPara->DevicePara.cMonitorMode == 0)
        {
            pst_RTCSystemPara->DeviceRunPara.usUploadStatusDataCurCnt++;
            //guc_RTC_TimeOutFlag = 1;
            if(pst_RTCSystemPara->DeviceRunPara.usUploadStatusDataCurCnt >= usTime)	//1天1440,，测试时4H上传一次
            {
                pst_RTCSystemPara->DeviceRunPara.usUploadStatusDataCurCnt = 0;
                pst_RTCSystemPara->DeviceRunPara.esDeviceRunState.nDevStartDays++;
                pst_RTCSystemPara->DeviceRunPara.cDeviceStatusUploadFlag = 1;
                //重新开启定位芯片，获取设备安装位置
                pst_RTCSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ucDataValidFlag = 0;
                pst_RTCSystemPara->DeviceRunPara.cBD_GetValueCnt = 0;
            }
            
            pst_RTCSystemPara->DeviceRunPara.nDeviceCurSaveRecordCount++;
            pst_RTCSystemPara->DeviceRunPara.nDeviceCurUploadRecordCount++;
            pst_RTCSystemPara->DeviceRunPara.nDeviceCurSampleCount++;
        }
        
        if((pst_RTCSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ucDataValidFlag == 0) && (pst_RTCSystemPara->DevicePara.cDeviceIdenFlag == 1))
        {
            pst_RTCSystemPara->DeviceRunPara.cBD_GetValueCnt++;
            if(pst_RTCSystemPara->DeviceRunPara.cBD_GetValueCnt >= 10)  //BD定位持续10分钟未得到有效数据，重启BD定位模块
            {
                pst_RTCSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ucDataValidFlag = 1;
                pst_RTCSystemPara->DeviceRunPara.cBD_GetValueCnt = 0;
            }
        }
        if(pst_RTCSystemPara->DeviceRunPara.enDeviceRunMode == DEVICE_RUN_STATE_SLEEP)
        {
            pst_RTCSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_RUN;
        }

        if((pst_RTCSystemPara->DeviceRunPara.cBD_GetDataFlag == 0) && (pst_RTCSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ucDataValidFlag == 0))
        {
            pst_RTCSystemPara->DeviceRunPara.cBD_LostDataCnt++;
        }
    }
    /* Switch system clock as MRC. */
    PWC_IrqClkBackup();
}

/**
 *******************************************************************************
 ** \brief Xtal32 clock config
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
static void Xtal32_ClockConfig(void)
{
    stc_clk_xtal32_cfg_t stcXtal32Cfg;

    /* configure structure initialization */
    MEM_ZERO_STRUCT(stcXtal32Cfg);

    /* Stop xtal32 */
    CLK_Xtal32Cmd(Disable);
    Ddl_Delay1ms(100u);
    /* Configuration xtal32 structure */
    stcXtal32Cfg.enDrv = ClkXtal32HighDrv;
    stcXtal32Cfg.enFilterMode = ClkXtal32FilterModeFull;
    CLK_Xtal32Config(&stcXtal32Cfg);
    /* Startup xtal32 */
    CLK_Xtal32Cmd(Enable);
    /* wait for xtal32 running */
    Ddl_Delay1ms(3000u);
}

/**
 *******************************************************************************
 ** \brief clock measure configuration
 **
 ** \param [in] None
 **
 ** \retval None
 **
 ******************************************************************************/
static void Clock_MeasureConfig(void)
{
    stc_clk_fcm_cfg_t stcClkFcmCfg;
    stc_clk_fcm_window_cfg_t stcClkFcmWinCfg;
    stc_clk_fcm_measure_cfg_t stcClkFcmMeasureCfg;
    stc_clk_fcm_reference_cfg_t stcClkFcmReferCfg;
    stc_clk_fcm_interrupt_cfg_t stcClkFcmIntCfg;

    /* Configure structure initialization */
    MEM_ZERO_STRUCT(stcClkFcmCfg);
    MEM_ZERO_STRUCT(stcClkFcmWinCfg);
    MEM_ZERO_STRUCT(stcClkFcmMeasureCfg);
    MEM_ZERO_STRUCT(stcClkFcmReferCfg);
    MEM_ZERO_STRUCT(stcClkFcmIntCfg);

    /* Enable FCM clock */
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_FCM, Enable);

    /* use xtal measure xtal32 */
    stcClkFcmWinCfg.windowLower = 20000u; /* zero error = 48828 */
    stcClkFcmWinCfg.windowUpper = 40000u;

    stcClkFcmMeasureCfg.enSrc = ClkFcmSrcXtal;
    stcClkFcmMeasureCfg.enSrcDiv = ClkFcmMeaDiv1;

    stcClkFcmReferCfg.enRefSel = ClkFcmInterRef;
    stcClkFcmReferCfg.enExtRef = Disable;
    stcClkFcmReferCfg.enIntRefSrc = ClkFcmSrcXtal32;
    stcClkFcmReferCfg.enIntRefDiv = ClkFcmIntrefDiv128;
    stcClkFcmReferCfg.enEdge = ClkFcmEdgeRising;
    stcClkFcmReferCfg.enFilterClk = ClkFcmFilterClkNone;

    stcClkFcmCfg.pstcFcmIntCfg = &stcClkFcmIntCfg;
    stcClkFcmCfg.pstcFcmMeaCfg = &stcClkFcmMeasureCfg;
    stcClkFcmCfg.pstcFcmRefCfg = &stcClkFcmReferCfg;
    stcClkFcmCfg.pstcFcmWindowCfg = &stcClkFcmWinCfg;
    /* enable clock measure */
    CLK_FcmConfig(&stcClkFcmCfg);
}

/**
 *******************************************************************************
 ** \brief Get rtc compensation value
 **
 ** \param [in]  None
 **
 ** \retval uint16_t                            Rtc compensation value
 **
 ******************************************************************************/
static uint16_t Rtc_GetCompenValue(void)
{
    uint32_t u32Tmp = 0ul;
    en_flag_status_t enStaTmp;
    float clkMeasureVal;
    uint16_t integerVal = 0u, decimalsVal = 0u;
    uint16_t clkCompenVal = 0u;
    stc_clk_freq_t stcClkFreq;

    MEM_ZERO_STRUCT(stcClkFreq);
    /* start measure */
    CLK_FcmCmd(Enable);
    do
    {
        enStaTmp = CLK_GetFcmFlag(ClkFcmFlagErrf);
        /* counter overflow or trigger frequency abnormal */
        if ((Set == CLK_GetFcmFlag(ClkFcmFlagOvf)) || (Set == enStaTmp))
        {
            CLK_FcmCmd(Disable);
            CLK_ClearFcmFlag(ClkFcmFlagOvf);
            CLK_ClearFcmFlag(ClkFcmFlagErrf);
            u32Tmp = 0xffu;
        }
    } while (Reset == CLK_GetFcmFlag(ClkFcmFlagMendf));

    if (0xffu != u32Tmp)
    {
        /* Get measure result */
        CLK_GetClockFreq(&stcClkFreq);
        u32Tmp = CLK_GetFcmCounter();
        clkMeasureVal = ((float)stcClkFreq.sysclkFreq * 128.0f) / (float)u32Tmp;
        /* stop measure */ 
        CLK_FcmCmd(Disable);
        CLK_ClearFcmFlag(ClkFcmFlagMendf);

        /* calculate clock compensation value */
        if (!((clkMeasureVal >= -EPSINON) && (clkMeasureVal <= EPSINON)))
        {
            clkMeasureVal = (clkMeasureVal - (float)XTAL32_VALUE) / (float)XTAL32_VALUE * (float)1000000.0f;
            clkMeasureVal = clkMeasureVal * (float)XTAL32_VALUE / 1000000.0f;

            if (clkMeasureVal < -EPSINON)    /* negative */
            {
                clkMeasureVal = (float)fabs((double)clkMeasureVal);
                integerVal = (uint16_t)(((~((uint32_t)clkMeasureVal)) + 1u) & 0x0Fu);
                /* Magnify one thousand times */
                u32Tmp = (uint32_t)clkMeasureVal;
                clkMeasureVal = (clkMeasureVal - (float)u32Tmp) * 1000.0f;
                decimalsVal = (uint16_t)((((~((uint32_t)clkMeasureVal)) & 0x3E0u) >> 5u) + 1u);
            }
            else                            /* positive */
            {
                clkMeasureVal += 1.0f;
                integerVal = (uint16_t)(((uint32_t)clkMeasureVal) & 0x0Fu);
                /* Magnify one thousand times */
                u32Tmp = (uint32_t)clkMeasureVal;
                clkMeasureVal = (float)((clkMeasureVal - (float)u32Tmp) * 1000.0f);
                decimalsVal = (uint16_t)(((uint32_t)clkMeasureVal & 0x3E0u) >> 5u);
            }
        }
        clkCompenVal = ((uint16_t)(integerVal << 5u) | decimalsVal) & 0x1FFu;
    }

    return clkCompenVal;
}

/**
 *******************************************************************************
 ** \brief Configure Rtc peripheral function
 **
 ** \param [in] None
 **
 ** \retval 0:	Success 1: Failed
 **
 ******************************************************************************/
int drv_mcu_RTC_Config(void)
{
    stc_rtc_init_t stcRtcInit;
    stc_irq_regi_conf_t stcIrqRegiConf;
    uint16_t clkCompenVal = 0u;

    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcRtcInit);
    MEM_ZERO_STRUCT(stcIrqRegiConf);

    /* Configure XTAL32 clock */
    Xtal32_ClockConfig();
    /* Configure clock measure */
    Clock_MeasureConfig();

    /* Reset rtc counter */
    if (RTC_DeInit() == ErrorTimeout)
    { 
        return 1;
    }
    else
    {
		pst_RTCSystemPara = GetSystemPara();
        clkCompenVal = Rtc_GetCompenValue();
        /* Configuration rtc structure */
        stcRtcInit.enClkSource = RtcClkXtal32;
        stcRtcInit.enPeriodInt = RtcPeriodIntOneMin;	//1MIN 唤醒
        stcRtcInit.enTimeFormat = RtcTimeFormat24Hour;
        stcRtcInit.enCompenWay = RtcOutputCompenUniform;
        stcRtcInit.enCompenEn = Enable;
        stcRtcInit.u16CompenVal = clkCompenVal;
        RTC_Init(&stcRtcInit);

        /* Configure interrupt of rtc period */
        stcIrqRegiConf.enIntSrc = INT_RTC_PRD;
        stcIrqRegiConf.enIRQn = Int049_IRQn;
        stcIrqRegiConf.pfnCallback = &RtcPeriod_IrqCallback;
        enIrqRegistration(&stcIrqRegiConf);
        NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
        NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
        NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

        /* Enable period interrupt */
        RTC_IrqCmd(RtcIrqPeriod, Enable);
        /* Startup rtc count */
        RTC_Cmd(Enable);
    }
	return 0;
}

/**
 *******************************************************************************
 ** \brief set Rtc datetime function
 **
 ** \param [in] current datetime: "2025-03-17 12:00:00"
 **
 ** \retval 0:	Success 1: Failed
 **
 ******************************************************************************/
int drv_mcu_Set_RTC_Time(char *cCurDateTime)
{
	stc_rtc_date_time_t stcRtcDateTimeCfg;
    stc_rtc_init_t stcRtcInit;
	char *cTemp;

    /* configure structure initialization */
    MEM_ZERO_STRUCT(stcRtcDateTimeCfg);
    MEM_ZERO_STRUCT(stcRtcInit);

	cTemp = strchr(cCurDateTime,'"');
    if(strlen(pst_RTCSystemPara->DeviceRunPara.esDeviceRunState.cDevStartDateTime) < 1)
    {
        memcpy(pst_RTCSystemPara->DeviceRunPara.esDeviceRunState.cDevStartDateTime,&cTemp[1],19);
        pst_RTCSystemPara->DeviceRunPara.esDeviceRunState.cDevStartDateTime[10] = ' ';
        pst_RTCSystemPara->DeviceRunPara.esDeviceRunState.cDevStartDateTime[4] = '-';
        pst_RTCSystemPara->DeviceRunPara.esDeviceRunState.cDevStartDateTime[7] = '-';
    }
    
	stcRtcDateTimeCfg.u8Year  = decimal_bcd_code(((cTemp[3] - '0')*10+(cTemp[4] - '0')));
	stcRtcDateTimeCfg.u8Month = decimal_bcd_code((cTemp[6] - '0')*10+(cTemp[7] - '0'));
	stcRtcDateTimeCfg.u8Day	 = decimal_bcd_code((cTemp[9] - '0')*10+(cTemp[10] - '0'));
	cTemp = strchr(cCurDateTime,',');
	stcRtcDateTimeCfg.u8Hour   = decimal_bcd_code((cTemp[1] - '0')*10+(cTemp[2] - '0'));
	stcRtcDateTimeCfg.u8Minute = decimal_bcd_code((cTemp[4] - '0')*10+(cTemp[5] - '0'));
	stcRtcDateTimeCfg.u8Second = decimal_bcd_code((cTemp[7] - '0')*10+(cTemp[8] - '0'));
	stcRtcDateTimeCfg.u8Weekday = RtcWeekdayMonday;
    //stcRtcDateTimeCfg.enAmPm = 

    if(stcRtcDateTimeCfg.u8Month < 1 || stcRtcDateTimeCfg.u8Month > 12)
    {
        return 1; // Invalid month
    }
    if (RTC_SetDateTime(RtcDataFormatBcd, &stcRtcDateTimeCfg, Enable, Enable) != Ok)
    {
        return 1;
    }
	return 0;
}

/**
 *******************************************************************************
 ** \brief get Rtc datetime function
 **
 ** \param [in] current datetime "2025-03-17 12:00:00"
 **
 ** \retval 0:	Success 1: Failed
 **
 ******************************************************************************/
int drv_mcu_Get_RTC_Time(char *cCurDateTime)
{
	stc_rtc_date_time_t stcCurrDateTime;

    /* configure structure initialization */
    MEM_ZERO_STRUCT(stcCurrDateTime);

	if (RTC_GetDateTime(RtcDataFormatDec, &stcCurrDateTime) != Ok)
	{
		return 1;
	}
	else
	{
		sprintf(cCurDateTime,"20%02d-%02d-%02d %02d:%02d:%02d",(stcCurrDateTime.u8Year),
														(stcCurrDateTime.u8Month),
														(stcCurrDateTime.u8Day),
														(stcCurrDateTime.u8Hour),
														(stcCurrDateTime.u8Minute),
														(stcCurrDateTime.u8Second));
	}
	return 0;
}
/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
