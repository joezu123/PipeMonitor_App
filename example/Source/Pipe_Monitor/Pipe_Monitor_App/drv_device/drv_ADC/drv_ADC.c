/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\driver\drv_device\drv_ADC\drv_ADC.c
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
#include "drv_ADC.h"
#include "hc32f460.h"
#include "hc32f460_clk.h"
#include "hc32f460_pwc.h"
#include "hc32f460_gpio.h"
#include "User_Data.h"
/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/*
 * If you remap the mapping between the channel and the pin with the function
 * ADC_ChannelRemap, define ADC_CH_REMAP as non-zero, otherwise define as 0.
 */
#define ADC_CH_REMAP                (0u)

/* ADC clock selection definition. */
#define ADC_CLK_PCLK                (1u)
#define ADC_CLK_MPLLQ               (2u)
#define ADC_CLK_UPLLR               (3u)

/* Select PCLK as ADC clock. */
#define ADC_CLK                     (ADC_CLK_UPLLR)

/* ADC1 channel definition for this example. */
//#define ADC1_SA_NORMAL_CHANNEL      (ADC1_CH0 | ADC1_CH10)
#ifdef HW_VERSION_V1_1
#define ADC1_SA_AVG_CHANNEL         (ADC1_CH15)	//---->原理图上ADC8; PC5引脚；电量传感器检测
#else
#define ADC1_SA_AVG_CHANNEL         (ADC1_CH4)	//---->原理图上ADC4; PA4引脚；水浸传感器检测
#endif
//#define ADC1_SA_CHANNEL             (ADC1_SA_NORMAL_CHANNEL | ADC1_SA_AVG_CHANNEL)
#define ADC1_SA_CHANNEL_COUNT       (4u)

#define ADC1_AVG_CHANNEL            (ADC1_SA_AVG_CHANNEL)
//#define ADC1_CHANNEL                (ADC1_SA_CHANNEL)

/* ADC1 channel sampling time.     ADC1_CH0  ADC1_CH10  ADC1_CH12   ADC1_CH13 */
//#define ADC1_SA_CHANNEL_SAMPLE_TIME { 0x30,     0x80,      0x50,      0x60 }

/* ADC2 channel definition for this example. */
//#define ADC2_SA_NORMAL_CHANNEL      (ADC2_CH0 | ADC2_CH2)
#ifdef HW_VERSION_V1_1
#define ADC2_SA_AVG_CHANNEL         (ADC2_CH6)	//---->原理图上ADC12 PC0引脚；水浸传感器检测
#else
#define ADC2_SA_AVG_CHANNEL         (ADC2_CH4)	//---->原理图上ADC8; PB0引脚；电池电量检测
#endif
//#define ADC2_SA_CHANNEL             (ADC2_SA_NORMAL_CHANNEL | ADC2_SA_AVG_CHANNEL)
#define ADC2_SA_CHANNEL_COUNT       (3u)

#define ADC2_AVG_CHANNEL            (ADC2_SA_AVG_CHANNEL)
//#define ADC2_CHANNEL                (ADC2_SA_CHANNEL)

/* ADC2 channel sampling time.      ADC2_CH0  ADC2_CH2  ADC2_CH5  */
//#define ADC2_SA_CHANNEL_SAMPLE_TIME { 0x60,     0x50,     0x40 }

/* ADC resolution definitions. */
#define ADC_RESOLUTION_8BIT         (8u)
#define ADC_RESOLUTION_10BIT        (10u)
#define ADC_RESOLUTION_12BIT        (12u)

#define ADC1_RESOLUTION             (ADC_RESOLUTION_12BIT)
#define ADC2_RESOLUTION             (ADC_RESOLUTION_12BIT)

/* Scan mode definitions. */
#define ADC1_SCAN_MODE              (AdcMode_SAContinuous)
#define ADC2_SCAN_MODE              (AdcMode_SAContinuous)

/* ADC reference voltage. The voltage of pin VREFH. */
#define ADC_VREF                    (3.288f)

/* ADC accuracy. */
#define ADC1_ACCURACY               (1ul << ADC1_RESOLUTION)

/* ADC2 continuous conversion times. */
#define ADC1_CONTINUOUS_TIMES       (3u)
#define ADC2_CONTINUOUS_TIMES       (3u)

/* Timeout value definitions. */
#define TIMEOUT_VAL                 (10u)


/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
static uint16_t m_au16Adc1Value[ADC1_CH_COUNT];
static uint16_t m_au16Adc2Value[ADC2_CH_COUNT];
/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void AdcClockConfig(void);
static void AdcInitConfig(void);
static void AdcChannelConfig(void);

//static void AdcSetChannelPinMode(const M4_ADC_TypeDef *ADCx,
//                                 uint32_t u32Channel,
//                                 en_pin_mode_t enMode);
//static void AdcSetPinMode(uint8_t u8AdcPin, en_pin_mode_t enMode);
/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 *******************************************************************************
 ** \brief  ADC clock configuration.
 **
 ** \note   1) ADCLK max frequency is 60MHz.
 **         2) If PCLK2 and PCLK4 are selected as the ADC clock,
 **            the following conditions must be met:
 **            a. ADCLK(PCLK2) max 60MHz;
 **            b. PCLK4 : ADCLK = 1:1, 2:1, 4:1, 8:1, 1:2, 1:4
 **
 ******************************************************************************/
static void AdcClockConfig(void)
{
#if (ADC_CLK == ADC_CLK_PCLK)
    /* Set bus clock division, depends on the system clock frequency. */
    m_stcSysclkCfg.enPclk2Div = ClkSysclkDiv64;
    m_stcSysclkCfg.enPclk4Div = ClkSysclkDiv16;

    CLK_SysClkConfig(&m_stcSysclkCfg);
    CLK_SetPeriClkSource(ClkPeriSrcPclk);

#elif (ADC_CLK == ADC_CLK_MPLLQ)
    stc_clk_xtal_cfg_t stcXtalCfg;
    stc_clk_mpll_cfg_t stcMpllCfg;

    if (CLKSysSrcMPLL == CLK_GetSysClkSource())
    {
        /*
         * Configure MPLLQ(same as MPLLP and MPLLR) when you
         * configure MPLL as the system clock.
         */
    }
    else
    {
        /* Use XTAL as MPLL source. */
        stcXtalCfg.enFastStartup = Enable;
        stcXtalCfg.enMode = ClkXtalModeOsc;
        stcXtalCfg.enDrv  = ClkXtalLowDrv;
        CLK_XtalConfig(&stcXtalCfg);
        CLK_XtalCmd(Enable);

        /* Set MPLL out 240MHz. */
        stcMpllCfg.pllmDiv = 1u;
        /* mpll = 8M / pllmDiv * plln */
        stcMpllCfg.plln    = 30u;
        stcMpllCfg.PllpDiv = 16u;
        stcMpllCfg.PllqDiv = 16u;
        stcMpllCfg.PllrDiv = 16u;
        CLK_SetPllSource(ClkPllSrcXTAL);
        CLK_MpllConfig(&stcMpllCfg);
        CLK_MpllCmd(Enable);
    }
    CLK_SetPeriClkSource(ClkPeriSrcMpllp);

#elif (ADC_CLK == ADC_CLK_UPLLR)
    stc_clk_xtal_cfg_t stcXtalCfg;
    stc_clk_upll_cfg_t stcUpllCfg;

    MEM_ZERO_STRUCT(stcXtalCfg);
    MEM_ZERO_STRUCT(stcUpllCfg);

    /* Use XTAL as UPLL source. */
    stcXtalCfg.enFastStartup = Enable;
    stcXtalCfg.enMode = ClkXtalModeOsc;
    stcXtalCfg.enDrv = ClkXtalLowDrv;
    CLK_XtalConfig(&stcXtalCfg);
    CLK_XtalCmd(Enable);

    /* Set UPLL out 240MHz. */
    stcUpllCfg.pllmDiv = 2u;
    /* upll = 8M(XTAL) / pllmDiv * plln */
    stcUpllCfg.plln    = 60u;
    stcUpllCfg.PllpDiv = 16u;
    stcUpllCfg.PllqDiv = 16u;
    stcUpllCfg.PllrDiv = 16u;	// 240M / 16 = 15M
    CLK_SetPllSource(ClkPllSrcXTAL);
    CLK_UpllConfig(&stcUpllCfg);
    CLK_UpllCmd(Enable);
    CLK_SetPeriClkSource(ClkPeriSrcUpllr);
#endif
}

/**
 *******************************************************************************
 ** \brief  ADC initial configuration.
 **
 ******************************************************************************/
static void AdcInitConfig(void)
{
    stc_adc_init_t stcAdcInit;

    MEM_ZERO_STRUCT(stcAdcInit);

#if (ADC1_RESOLUTION == ADC_RESOLUTION_8BIT)
    stcAdcInit.enResolution = AdcResolution_8Bit;
#elif (ADC1_RESOLUTION == ADC_RESOLUTION_10BIT)
    stcAdcInit.enResolution = AdcResolution_10Bit;
#else
    stcAdcInit.enResolution = AdcResolution_12Bit;
#endif
    stcAdcInit.enDataAlign  = AdcDataAlign_Right;
    stcAdcInit.enAutoClear  = AdcClren_Disable;
    stcAdcInit.enScanMode   = ADC1_SCAN_MODE;
    /* 1. Enable ADC1. */
    PWC_Fcg3PeriphClockCmd(PWC_FCG3_PERIPH_ADC1, Enable);
    /* 2. Initialize ADC1. */
    ADC_Init(M4_ADC1, &stcAdcInit);

#if (ADC2_RESOLUTION == ADC_RESOLUTION_8BIT)
    stcAdcInit.enResolution = AdcResolution_8Bit;
#elif (ADC2_RESOLUTION == ADC_RESOLUTION_10BIT)
    stcAdcInit.enResolution = AdcResolution_10Bit;
#else
    stcAdcInit.enResolution = AdcResolution_12Bit;
#endif
    stcAdcInit.enScanMode   = ADC2_SCAN_MODE;
    /* 1. Enable ADC2. */
    PWC_Fcg3PeriphClockCmd(PWC_FCG3_PERIPH_ADC2, Enable);
    /* 2. Initialize ADC2. */
    ADC_Init(M4_ADC2, &stcAdcInit);
}

/**
 *******************************************************************************
 ** \brief  ADC channel configuration.
 **
 ******************************************************************************/
static void AdcChannelConfig(void)
{
    stc_adc_ch_cfg_t stcChCfg;
	uint8_t uSampTime = 0x50;

    MEM_ZERO_STRUCT(stcChCfg);

    stcChCfg.u32Channel  = ADC1_SA_AVG_CHANNEL;
    stcChCfg.u8Sequence  = ADC_SEQ_A;
    stcChCfg.pu8SampTime = &uSampTime;
    /* 1. Set the ADC pin to analog mode. */
    //AdcSetChannelPinMode(M4_ADC1, ADC1_AVG_CHANNEL, Pin_Mode_Ana);
    #ifdef HW_VERSION_V1_1
    drv_Battery_Level_Init();
    #else
	drv_Water_Immersion_Sensor_Init();
    #endif
    /* 2. Add ADC channel. */
    ADC_AddAdcChannel(M4_ADC1, &stcChCfg);

    /* 3. Configure the average channel if you need. */
    ADC_ConfigAvg(M4_ADC1, AdcAvcnt_32);
    /* 4. Add average channel if you need. */
    ADC_AddAvgChannel(M4_ADC1, ADC1_AVG_CHANNEL);

    stcChCfg.u32Channel  = ADC2_SA_AVG_CHANNEL;
    stcChCfg.pu8SampTime = &uSampTime;
    /* 1. Set the ADC pin to analog mode. */
    //AdcSetChannelPinMode(M4_ADC2, ADC2_AVG_CHANNEL, Pin_Mode_Ana);
    #ifdef HW_VERSION_V1_1
    drv_Water_Immersion_Sensor_Init();
    #else
	drv_Battery_Level_Init();
    #endif
    /* 2. Add ADC channel. */
    ADC_AddAdcChannel(M4_ADC2, &stcChCfg);

    /* 3. Configure the average channel if you need. */
    ADC_ConfigAvg(M4_ADC2, AdcAvcnt_64);
    /* 4. Add average channel if you need. */
    ADC_AddAvgChannel(M4_ADC2, ADC2_AVG_CHANNEL);
}

/*ADC Init */
uint8_t drv_mcu_ADC_Init()
{
	AdcClockConfig();
    AdcInitConfig();
    AdcChannelConfig();

	return 0;
}

void func_ADC_DeInit(void)
{
    /* Disable ADC1. */
    PWC_Fcg3PeriphClockCmd(PWC_FCG3_PERIPH_ADC1, Disable);
    /* Disable ADC2. */
    PWC_Fcg3PeriphClockCmd(PWC_FCG3_PERIPH_ADC2, Disable);

    ADC_DeInit(M4_ADC1);
    ADC_DeInit(M4_ADC2);
    
}

//水浸传感器；ADC4; 引脚PA4; ADC12_IN4; ADC1
uint8_t drv_Water_Immersion_Sensor_Init()
{
	en_port_t enPort = PortA;
    en_pin_t enPin   = Pin00;
    stc_port_init_t stcPortInit;

    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Ana;
    stcPortInit.enPullUp  = Disable;
    #ifdef HW_VERSION_V1_1
    enPort = PortC;
    enPin  = Pin00;
    #else
	enPort = PortA;
    enPin  = Pin04;
    #endif
	PORT_Init(enPort, enPin, &stcPortInit);
	return 0;
}

//电池电量检测；ADC8; 引脚PB0; ADC12_IN8; ADC2
uint8_t drv_Battery_Level_Init()
{
	en_port_t enPort = PortA;
    en_pin_t enPin   = Pin00;
    stc_port_init_t stcPortInit;

    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Ana;
    stcPortInit.enPullUp  = Disable;
    #ifdef HW_VERSION_V1_1
    enPort = PortC;
    enPin  = Pin05;
    #else
	enPort = PortB;
    enPin  = Pin00;
    #endif
	PORT_Init(enPort, enPin, &stcPortInit);

	enPort = BATTERY_PWRCHK_GPIO_PORT;
    enPin = BATTERY_PWRCHK_GPIO_PIN;
	stcPortInit.enPinMode = Pin_Mode_Out;
	PORT_Init(enPort, enPin, &stcPortInit);
	return 0;
}

//获取水浸传感器状态
uint8_t drv_Get_Water_Immersion_Sensor_Status(char* cDevStatus, float * fValue)
{
	uint8_t u8Count = 0u;
	uint16_t usValue[3] = {0};
	uint16_t usSumValue = 0;
	double fValue1 = 0.0;
    
    #if 0
    ADC_StartConvert(M4_ADC1);
    while (u8Count < ADC1_CONTINUOUS_TIMES)
    {
        if (Set == ADC_GetEocFlag(M4_ADC1, ADC_SEQ_A))
        {
            ADC_GetChData(M4_ADC1, ADC1_AVG_CHANNEL, m_au16Adc1Value, 1);
            ADC_ClrEocFlag(M4_ADC1, ADC_SEQ_A);
			usValue[u8Count] = m_au16Adc1Value[4];
            u8Count++;
            // TODO: USE THE m_au16Adc2Value.
        }
    }

    /*
     *  DO NOT forget to stop ADC when your mode is
     *  AdcMode_SAContinuous or AdcMode_SAContinuousSBOnce
     *  unless you need.
     */
    ADC_StopConvert(M4_ADC1);
    #else
    #ifdef HW_VERSION_V1_1
    while (u8Count < ADC2_CONTINUOUS_TIMES)
    {
        ADC_PollingSa(M4_ADC2, m_au16Adc2Value, ADC2_CH_COUNT, 10);
        usValue[u8Count] = m_au16Adc2Value[6];
        u8Count++;

    }
    #else
    while (u8Count < ADC1_CONTINUOUS_TIMES)
    {
        ADC_PollingSa(M4_ADC1, m_au16Adc1Value, ADC1_CH_COUNT, 10);
        usValue[u8Count] = m_au16Adc1Value[4];
        u8Count++;
    }
    #endif
    #endif

	usSumValue = usValue[0] + usValue[1] + usValue[2];
	usSumValue = usSumValue / 3;
	fValue1 =  ((double)usSumValue * ADC_VREF) / (double)ADC1_ACCURACY;
    *fValue = fValue1;

	if(fValue1 > 1.0)
	{
		*cDevStatus = 0x00;
	}
	else
	{
		*cDevStatus = 0x01;
	}
	return 0;
}

//获取当前电池电量
float drv_Get_Battery_Level_Value(float* fValue)
{
	uint8_t u8Count = 0u;
	uint16_t usValue[3] = {0};
	uint16_t usSumValue = 0;
    double fTempValue = 0.0;
    float fBatteryValue_V = 0.0;
	BATTERY_PWRCHK_OPEN();

    #if 0
	u8Count = 0u;
    memset(m_au16Adc1Value, 0, sizeof(m_au16Adc1Value));
    ADC_StartConvert(M4_ADC2);
    while (u8Count < ADC2_CONTINUOUS_TIMES)
    {
        if (Set == ADC_GetEocFlag(M4_ADC2, ADC_SEQ_A))
        {
            ADC_GetChData(M4_ADC2, ADC2_AVG_CHANNEL, m_au16Adc2Value, ADC2_CH_COUNT);
            ADC_ClrEocFlag(M4_ADC2, ADC_SEQ_A);
			usValue[u8Count] = m_au16Adc1Value[4];
            u8Count++;
            // TODO: USE THE m_au16Adc2Value.
        }
    }

    /*
     *  DO NOT forget to stop ADC when your mode is
     *  AdcMode_SAContinuous or AdcMode_SAContinuousSBOnce
     *  unless you need.
     */
    ADC_StopConvert(M4_ADC2);
    #else
    #ifdef HW_VERSION_V1_1
    while (u8Count < ADC1_CONTINUOUS_TIMES)
    {
        ADC_PollingSa(M4_ADC1, m_au16Adc1Value, ADC1_CH_COUNT, 10);
        usValue[u8Count] = m_au16Adc1Value[15];
        u8Count++;
    }
    #else
    while (u8Count < ADC2_CONTINUOUS_TIMES)
    {
        ADC_PollingSa(M4_ADC2, m_au16Adc2Value, ADC2_CH_COUNT, 10);
        usValue[u8Count] = m_au16Adc2Value[4];
        u8Count++;

    }
    #endif
    #endif
    //此处不能关闭电源，因为外接485电路模块需要此电源供电，修改为在进入低功耗模式时关闭电源
	//BATTERY_PWRCHK_CLOSE();

	usSumValue = usValue[0] + usValue[1] + usValue[2];
	usSumValue = usSumValue / 3;
	fTempValue =  ((double)usSumValue * ADC_VREF) / 4096.0;
    fBatteryValue_V = fTempValue; //电池电压值
    //暂时以电量采样值0.8~1.3V为计算空间
    fTempValue = 1 - (1.3-fTempValue);  //==>1~0.5
    if(fTempValue < 0.0)
    {
        fTempValue = 0.0;
    }
    if(fTempValue >= 1.0)
    {
        fTempValue = 1.0;
    }
    fTempValue = fTempValue - 0.5;      //==>0.5~0
    if(fTempValue < 0.0)
    {
        fTempValue = 0.0;
    }
    if(fTempValue >= 0.5)
    {
        fTempValue = 0.5;
    }
    *fValue = fTempValue / 0.5 * 100;

	return fBatteryValue_V;
}
/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
