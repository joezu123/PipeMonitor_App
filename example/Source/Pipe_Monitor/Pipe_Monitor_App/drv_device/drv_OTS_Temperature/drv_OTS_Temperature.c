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
#include "drv_OTS_Temperature.h"
#include "hc32f460.h"
#include "User_Data.h"
/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
/* OTS clock selection. */
#define OTS_CLK_SEL_XTAL            (0u)
#define OTS_CLK_SEL_HRC             (1u)

/* Select XTAL as OTS clock. */
#define OTS_CLK_SEL                 (OTS_CLK_SEL_XTAL)

/*
 * Definitions about OTS interrupt for the example.
 * OTS independent IRQn: [Int000_IRQn, Int031_IRQn], [Int110_IRQn, Int113_IRQn].
 */

/* OTS parameters, slope K and offset M. Different chip, different parameters. */
#define OTS_XTAL_K                  (737272.73f)
#define OTS_XTAL_M                  (27.55f)
#define OTS_HRC_K                   (3002.59f)
#define OTS_HRC_M                   (27.92f)

/* Timeout value. */
#define TIMEOUT_VAL                 (10000u)

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static void OtsConfig(void);
static void OtsInitConfig(void);
static void OtsClockConfig(void);

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
/**
 *******************************************************************************
 ** \brief  OTS configuration, including initial configuration and
 **         clock configuration.
 **
 ** \param  None.
 **
 ** \retval None.
 **
 ******************************************************************************/
static void OtsConfig(void)
{
    OtsInitConfig();
    OtsClockConfig();
}

/**
 *******************************************************************************
 ** \brief  OTS initial configuration.
 **
 ** \param  None.
 **
 ** \retval None.
 **
 ******************************************************************************/
static void OtsInitConfig(void)
{
    stc_ots_init_t stcOtsInit;

    stcOtsInit.enAutoOff = OtsAutoOff_Disable;
#if (OTS_CLK_SEL == OTS_CLK_SEL_HRC)
    stcOtsInit.enClkSel   = OtsClkSel_Hrc;
    stcOtsInit.f32SlopeK  = OTS_HRC_K;
    stcOtsInit.f32OffsetM = OTS_HRC_M;
#else
    stcOtsInit.enClkSel   = OtsClkSel_Xtal;
    stcOtsInit.f32SlopeK  = OTS_XTAL_K;
    stcOtsInit.f32OffsetM = OTS_XTAL_M;
#endif

    /* 1. Enable OTS. */
    PWC_Fcg3PeriphClockCmd(PWC_FCG3_PERIPH_OTS, Enable);
    /* 2. Initialize OTS. */
    OTS_Init(&stcOtsInit);
}

/**
 *******************************************************************************
 ** \brief  OTS clock configuration.
 **
 ** \param  None.
 **
 ** \retval None.
 **
 ******************************************************************************/
static void OtsClockConfig(void)
{
#if (OTS_CLK_SEL == OTS_CLK_SEL_HRC)
    /* Enable HRC for OTS. */
    CLK_HrcCmd(Enable);
    /* Enable XTAL32 while clock selecting HRC. */
    CLK_Xtal32Cmd(Enable);
#else
    /* Enable XTAL for OTS. */
    CLK_XtalCmd(Enable);
#endif

    /* Enable LRC for OTS. */
    CLK_LrcCmd(Enable);
}

/*
 * If you remap the mapping between the channel and the pin with the function
 * ADC_ChannelRemap, define ADC_CH_REMAP as non-zero, otherwise define as 0.
 */
void drv_OTS_Temperature_Init(void)
{
    /* Config OTS. */
    OtsConfig();
}

void func_OTS_Temperature_GetValue(float *fValue)
{   
    OTS_Start();
    (void)OTS_Polling(fValue, TIMEOUT_VAL);
}
/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
