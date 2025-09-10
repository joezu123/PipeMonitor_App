/**
 *******************************************************************************
 * @file  Pipe_Monitor_App\drivers\device_drv\drv_USART.c
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
#include "drv_USART.h"
#include "hc32f460_utility.h"
#include "hc32f460_pwc.h"
#include "BT_TB05.h"
/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define USART1_RX_DMA   1  
#define USART2_RX_DMA  1   
#define USART3_RX_DMA  1
#define USART3_INIT  1   
#define USART4_INIT  1
#define USART4_RX_DMA  1

uint8_t guc_StartPosi = 10;
uint8_t gucNFCRecvCnt = 0;
uint8_t gucBTRecvArr[300] = {0};
uint16_t gusBTRecvCnt = 0;
//uint8_t gucNFCRecvData[20] = {0};
/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static SystemPataSt *pst_UsartSystemPara;

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  Dma1Btc0IrqCallback.
 * @param  None
 * @retval None
 * @note   DMA1 channel 0 block transfer complete IRQ callback function.
 *        This function is used to clear the USART1 receive timeout flag.
 */
static void Dma1Btc0IrqCallback(void)
{
    //PWC_IrqClkRecover();
    USART_ClearStatus(M4_USART1, UsartRxTimeOut);  //清楚接收超时标志
    DMA_ClearIrqFlag(M4_DMA1, DmaCh0, BlkTrnCpltIrq);
    #if 0
    TIMER0_Cmd(M4_TMR01, Tim0_ChannelA,Disable);

    pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[0] = USART_DATA_LEN_MAX - DMA_GetTransferCnt(M4_DMA1, DmaCh0);
    //pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[1] = USART_DATA_LEN_MAX - DMA_GetTransferCnt(M4_DMA1, DmaCh1);
    if(pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[0] >= USART_DATA_LEN_MAX)
    {
        pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[0] = 0;
        memset(pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[0], 0, USART_DATA_LEN_MAX);
    }
    else
    {
        //if(pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[0] == strlen(pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[0]))
        {
            pst_UsartSystemPara->UsartData.ucUsartxRecvDataFlag[0] = 1;
        }
    }
    
    DMA_ChannelCmd(M4_DMA1, DmaCh0, Disable);  //超时重启DMA，以进行新一轮的接收
    DMA_SetDesAddress(M4_DMA1, DmaCh0, (uint32_t)(pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[0]));
    DMA_SetTransferCnt(M4_DMA1, DmaCh0, USART_DATA_LEN_MAX);
    DMA_ChannelCmd(M4_DMA1, DmaCh0, Enable);
    if(pst_UsartSystemPara->DeviceRunPara.enDeviceRunMode == DEVICE_RUN_STATE_SLEEP)
    {
        pst_UsartSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_RUN;
    }
    //TIMER0_Cmd(M4_TMR01, Tim0_ChannelA,Enable);
    /* Switch system clock as MRC. */
    PWC_IrqClkBackup();
    #endif
}

/**
 * @brief  Dma1Btc1IrqCallback.
 * @param  None
 * @retval None
 * @note   DMA1 channel 1 block transfer complete IRQ callback function.
 *        This function is used to clear the USART2 receive timeout flag.
 */
static void Dma1Btc1IrqCallback(void)
{
    USART_ClearStatus(M4_USART2, UsartRxTimeOut);  //清楚接收超时标志
    DMA_ClearIrqFlag(M4_DMA1, DmaCh1, BlkTrnCpltIrq);
    #if 0
    #ifdef USART2_RX_DMA
    TIMER0_Cmd(M4_TMR01, Tim0_ChannelB,Disable);
    USART_ClearStatus(M4_USART2, UsartRxTimeOut);
    #else
    USART_ClearStatus(M4_USART2, UsartRxNoEmpty);
    #endif
    
    #ifdef USART2_RX_DMA
    //gusBTRecvCnt = 300 - DMA_GetTransferCnt(M4_DMA1, DmaCh1);
    pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[1] = USART_DATA_LEN_MAX - DMA_GetTransferCnt(M4_DMA1, DmaCh1);
    if(pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[1] >= USART_DATA_LEN_MAX)
    {
        pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[1] = 0;
        memset(pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[1], 0, USART_DATA_LEN_MAX);
    }
    else
    {
        pst_UsartSystemPara->UsartData.ucUsartxRecvDataFlag[1] = 1;
        pst_UsartSystemPara->DeviceRunPara.usDeviceBTWaitCnt = 0;
    }
    #else
    pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[1][pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[1]] = (unsigned char)USART_RecData(M4_USART2); //USART_ReceiveData(M4_USART2);
    pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[1] ++;//= USART_DATA_LEN_MAX - DMA_GetTransferCnt(M4_DMA1, DmaCh1);
    #endif
    //if(pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[1] == strlen(pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[1]))
    
    
    #ifdef USART2_RX_DMA
    DMA_ChannelCmd(M4_DMA1, DmaCh1, Disable);  //超时重启DMA，以进行新一轮的接收
    DMA_SetDesAddress(M4_DMA1, DmaCh1, (uint32_t)(&pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[1][0]));
    DMA_SetTransferCnt(M4_DMA1, DmaCh1, USART_DATA_LEN_MAX);
    //DMA_SetDesAddress(M4_DMA1, DmaCh1, (uint32_t)(&gucBTRecvArr[0]));
    //DMA_SetTransferCnt(M4_DMA1, DmaCh1, 300);
    DMA_ChannelCmd(M4_DMA1, DmaCh1, Enable);
    //pst_UsartSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_RUN;
    #endif
    #endif
}

/**
 * @brief  Dma1Btc2IrqCallback.
 * @param  None
 * @retval None
 * @note   DMA1 channel 2 block transfer complete IRQ callback function.
 *        This function is used to clear the USART3 receive timeout flag.
 */
//unsigned char guc_TestCnt = 0;
static void Dma1Btc2IrqCallback(void)
{
    //guc_TestCnt++;
    USART_ClearStatus(M4_USART3, UsartRxTimeOut);  //清楚接收超时标志
    DMA_ClearIrqFlag(M4_DMA1, DmaCh2, BlkTrnCpltIrq);
    #if 0
    #ifdef USART3_RX_DMA
    TIMER0_Cmd(M4_TMR02, Tim0_ChannelA,Disable);
    USART_ClearStatus(M4_USART3, UsartRxTimeOut);
    #else
    //USART_ClearStatus(M4_USART3, UsartRxNoEmpty);
    #endif
    
    #ifdef USART3_RX_DMA
    
    pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2] = USART_DATA_LEN_MAX - DMA_GetTransferCnt(M4_DMA1, DmaCh2);
    //gucNFCRecvData[guc_StartPosi++] = 0xB0;
    if(pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2] >= USART_DATA_LEN_MAX)
    {
        pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2] = 0;
        memset(pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[2], 0, USART_DATA_LEN_MAX);
    }
    else
    {
        //memcpy(&guc_Usart3Data[guc_Usart3Cnt], pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[2], pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2]);
        //guc_Usart3Cnt += pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2];
    }
    #else
    pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[2][pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2]] = (unsigned char)USART_RecData(M4_USART3); //USART_ReceiveData(M4_USART2);
    pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2] ++;
    #endif
    pst_UsartSystemPara->UsartData.ucUsartxRecvDataFlag[2] = 1;
    
    //memcpy(&guc_NFC_USART3_RecvData[guc_NFC_USART3_RecvCnt++][0], pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[2], pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2]);
    
    #ifdef USART3_RX_DMA
    DMA_ChannelCmd(M4_DMA1, DmaCh2, Disable);  //超时重启DMA，以进行新一轮的接收
    DMA_SetDesAddress(M4_DMA1, DmaCh2, (uint32_t)(&pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[2][0]));
    DMA_SetTransferCnt(M4_DMA1, DmaCh2, USART_DATA_LEN_MAX);
    DMA_ChannelCmd(M4_DMA1, DmaCh2, Enable);
    //pst_UsartSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_RUN;
    #endif
    #endif
}

/**
 * @brief  Dma1Btc3IrqCallback.
 * @param  None
 * @retval None
 * @note   DMA1 channel 3 block transfer complete IRQ callback function.
 *        This function is used to clear the USART4 receive timeout flag.
 */
static void Dma1Btc3IrqCallback(void)
{
    USART_ClearStatus(M4_USART4, UsartRxTimeOut);  //清楚接收超时标志
    DMA_ClearIrqFlag(M4_DMA1, DmaCh3, BlkTrnCpltIrq);
    #if 0
    #ifdef USART4_RX_DMA
    TIMER0_Cmd(M4_TMR02, Tim0_ChannelB,Disable);
    USART_ClearStatus(M4_USART4, UsartRxTimeOut);
    #else
    USART_ClearStatus(M4_USART4, UsartRxNoEmpty);
    #endif
    //usUsart4RecvCnt++;
    #ifdef USART4_RX_DMA
    pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[3] = 1100 - DMA_GetTransferCnt(M4_DMA1, DmaCh3);
    if(pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[3] >= 1100)
    {
        pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[3] = 0;
        memset(pst_UsartSystemPara->UsartData.ucUsart4RecvDataArr, 0, 1100);
    }
    #else
    pst_UsartSystemPara->UsartData.ucUsart4RecvDataArr[pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[3]] = (unsigned char)USART_RecData(M4_USART4); //USART_ReceiveData(M4_USART2);
    pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[3] ++;//= USART_DATA_LEN_MAX - DMA_GetTransferCnt(M4_DMA1, DmaCh1);
    if(pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[3] >= 1100)
    {
        pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[3] = 0;
        memset(pst_UsartSystemPara->UsartData.ucUsart4RecvDataArr, 0, 1100);
    }
    #endif
    pst_UsartSystemPara->UsartData.ucUsartxRecvDataFlag[3] = 1;
    #ifdef USART4_RX_DMA
    DMA_ChannelCmd(M4_DMA1, DmaCh3, Disable);  //超时重启DMA，以进行新一轮的接收
    DMA_SetDesAddress(M4_DMA1, DmaCh3, (uint32_t)(pst_UsartSystemPara->UsartData.ucUsart4RecvDataArr));
    DMA_SetTransferCnt(M4_DMA1, DmaCh3, 1100);
    DMA_ChannelCmd(M4_DMA1, DmaCh3, Enable);
    #endif
    #endif
}

/**
 * @brief  Dma1Init.
 * @param  None
 * @retval None
 * @note   DMA1初始化，配合USART接收数据使用，采用TIM0超时中断.
 */
static void Dma1Init(void)
{
    stc_dma_config_t stcDmaInit;
    stc_irq_regi_conf_t stcIrqRegiCfg;

    /* Enable peripheral clock */
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_DMA1, Enable);

    /* Enable DMA. */
    DMA_Cmd(M4_DMA1,Enable);

    /* Initialize DMA. */
    MEM_ZERO_STRUCT(stcDmaInit);
    stcDmaInit.u16BlockSize = 1u; /* 1 block */
    #ifdef USART1_RX_DMA
    stcDmaInit.u32SrcAddr = ((uint32_t)(&M4_USART1->DR)+2ul); /* Set source address. receive register*/
    stcDmaInit.u32DesAddr = (uint32_t)(&pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[0][0]);     /* Set destination address. */
    #endif
    stcDmaInit.stcDmaChCfg.enSrcInc = AddressFix;  /* Set source address mode. */
    stcDmaInit.stcDmaChCfg.enDesInc = AddressIncrease;  /* Set destination address mode. */
    stcDmaInit.stcDmaChCfg.enIntEn = Enable;       /* Enable interrupt. */
    stcDmaInit.stcDmaChCfg.enTrnWidth = Dma8Bit;   /* Set data width 8bit. */
    #ifdef USART1_RX_DMA
    DMA_InitChannel(M4_DMA1, DmaCh0, &stcDmaInit);
    #endif
    #ifdef USART2_RX_DMA
    stcDmaInit.u32SrcAddr = ((uint32_t)(&M4_USART2->DR)+2ul); /* Set source address. receive register*/
    stcDmaInit.u32DesAddr = (uint32_t)(&pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[1][0]);
    DMA_InitChannel(M4_DMA1, DmaCh1, &stcDmaInit);
    #endif

    #ifdef HW_VERSION_V1_1
    #ifdef USART3_RX_DMA
    stcDmaInit.u32SrcAddr = ((uint32_t)(&M4_USART3->DR)+2ul); /* Set source address. receive register*/
    stcDmaInit.u32DesAddr = (uint32_t)(&pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[2][0]);
    DMA_InitChannel(M4_DMA1, DmaCh2, &stcDmaInit);
    #endif
    #else
    #ifndef USART3_INIT
    #ifdef USART3_RX_DMA
    stcDmaInit.u32SrcAddr = ((uint32_t)(&M4_USART3->DR)+2ul); /* Set source address. receive register*/
    stcDmaInit.u32DesAddr = (uint32_t)(&pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[2][0]);
    DMA_InitChannel(M4_DMA1, DmaCh2, &stcDmaInit);
    #endif
    #endif
    #endif

    //#ifndef USART4_INIT
    #ifdef USART4_RX_DMA
    stcDmaInit.u32SrcAddr = ((uint32_t)(&M4_USART4->DR)+2ul); /* Set source address. receive register*/
    stcDmaInit.u32DesAddr = (uint32_t)(&pst_UsartSystemPara->UsartData.ucUsart4RecvDataArr[0]);
    DMA_InitChannel(M4_DMA1, DmaCh3, &stcDmaInit);
    #endif
    //#endif

    /* Enable the specified DMA channel. */
    #ifdef USART1_RX_DMA
    DMA_ChannelCmd(M4_DMA1, DmaCh0, Enable);
    #endif
    #ifdef USART2_RX_DMA
    DMA_ChannelCmd(M4_DMA1, DmaCh1, Enable);
    #endif
    #ifdef HW_VERSION_V1_1
    #ifdef USART3_RX_DMA
    DMA_ChannelCmd(M4_DMA1, DmaCh2, Enable);
    #endif
    #else
    #ifndef USART3_INIT
    #ifdef USART3_RX_DMA
    DMA_ChannelCmd(M4_DMA1, DmaCh2, Enable);
    #endif
    #endif
    #endif

    //#ifndef USART4_INIT
    #ifdef USART4_RX_DMA
    DMA_ChannelCmd(M4_DMA1, DmaCh3, Enable);
    #endif
    //#endif

    /* Clear DMA flag. */
    #ifdef USART1_RX_DMA
    DMA_ClearIrqFlag(M4_DMA1, DmaCh0, TrnCpltIrq);
    #endif
    #ifdef USART2_RX_DMA
    DMA_ClearIrqFlag(M4_DMA1, DmaCh1, TrnCpltIrq);
    #endif

    #ifdef HW_VERSION_V1_1
    #ifdef USART3_RX_DMA
    DMA_ClearIrqFlag(M4_DMA1, DmaCh2, TrnCpltIrq);
    #endif
    #else
    #ifndef USART3_INIT
    #ifdef USART3_RX_DMA
    DMA_ClearIrqFlag(M4_DMA1, DmaCh2, TrnCpltIrq);
    #endif
    #endif
    #endif

    //#ifndef USART4_INIT
    #ifdef USART4_RX_DMA
    DMA_ClearIrqFlag(M4_DMA1, DmaCh3, TrnCpltIrq);
    #endif
    //#endif

    /* Enable peripheral circuit trigger function. */
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_AOS,Enable);

    /* Set DMA trigger source. */
    #ifdef USART1_RX_DMA
    DMA_SetTriggerSrc(M4_DMA1, DmaCh0, EVT_USART1_RI);
    #endif
    #ifdef USART2_RX_DMA
    DMA_SetTriggerSrc(M4_DMA1, DmaCh1, EVT_USART2_RI);
    #endif

    #ifdef HW_VERSION_V1_1
    #ifdef USART3_RX_DMA
    DMA_SetTriggerSrc(M4_DMA1, DmaCh2, EVT_USART3_RI);
    #endif
    #else
    #ifndef USART3_INIT
    #ifdef USART3_RX_DMA
    DMA_SetTriggerSrc(M4_DMA1, DmaCh2, EVT_USART3_RI);
    #endif
    #endif
    #endif

    //#ifndef USART4_INIT
    #ifdef USART4_RX_DMA
    DMA_SetTriggerSrc(M4_DMA1, DmaCh3, EVT_USART4_RI);
    #endif
    //#endif

    #ifdef USART1_RX_DMA
    /* Set DMA block transfer complete IRQ */
    stcIrqRegiCfg.enIRQn = DMA1_CH0_BTCINT_IRQn;
    stcIrqRegiCfg.pfnCallback = &Dma1Btc0IrqCallback;
    stcIrqRegiCfg.enIntSrc = DMA1_CH0_BTCINT_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
    #endif

    #ifdef USART2_RX_DMA
    stcIrqRegiCfg.enIRQn = DMA1_CH1_BTCINT_IRQn;
    stcIrqRegiCfg.pfnCallback = &Dma1Btc1IrqCallback;
    stcIrqRegiCfg.enIntSrc = DMA1_CH1_BTCINT_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
    #endif

    #ifdef HW_VERSION_V1_1
    #ifdef USART3_RX_DMA
    stcIrqRegiCfg.enIRQn = DMA1_CH2_BTCINT_IRQn;
    stcIrqRegiCfg.pfnCallback = &Dma1Btc2IrqCallback;
    stcIrqRegiCfg.enIntSrc = DMA1_CH2_BTCINT_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
    #endif
    #else
    #ifndef USART3_INIT
    #ifdef USART3_RX_DMA
    stcIrqRegiCfg.enIRQn = DMA1_CH2_BTCINT_IRQn;
    stcIrqRegiCfg.pfnCallback = &Dma1Btc2IrqCallback;
    stcIrqRegiCfg.enIntSrc = DMA1_CH2_BTCINT_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
    #endif
    #endif
    #endif

    //#ifndef USART4_INIT
    #ifdef USART4_RX_DMA
    stcIrqRegiCfg.enIRQn = DMA1_CH3_BTCINT_IRQn;
    stcIrqRegiCfg.pfnCallback = &Dma1Btc3IrqCallback;
    stcIrqRegiCfg.enIntSrc = DMA1_CH3_BTCINT_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
    #endif
    //#endif
}

/**
 * @brief  Usart1ErrIrqCallback.
 * @param  None
 * @retval None
 * @note   USART1 error IRQ callback function.//串口接收错误中断回调
 */
static void Usart1ErrIrqCallback(void)
{
    if (Set == USART_GetStatus(M4_USART1, UsartFrameErr))
        USART_ClearStatus(M4_USART1, UsartFrameErr);

    if (Set == USART_GetStatus(M4_USART1, UsartParityErr))
        USART_ClearStatus(M4_USART1, UsartParityErr);

    if (Set == USART_GetStatus(M4_USART1, UsartOverrunErr))
        USART_ClearStatus(M4_USART1, UsartOverrunErr);
}

/**
 * @brief  Usart2ErrIrqCallback.
 * @param  None
 * @retval None
 * @note   USART2 error IRQ callback function.//串口接收错误中断回调
 */
static void Usart2ErrIrqCallback(void)
{
    if (Set == USART_GetStatus(M4_USART2, UsartFrameErr))
        USART_ClearStatus(M4_USART2, UsartFrameErr);

    if (Set == USART_GetStatus(M4_USART2, UsartParityErr))
        USART_ClearStatus(M4_USART2, UsartParityErr);

    if (Set == USART_GetStatus(M4_USART2, UsartOverrunErr))
        USART_ClearStatus(M4_USART2, UsartOverrunErr);
}

/**
 * @brief  Usart3ErrIrqCallback.
 * @param  None
 * @retval None
 * @note   USART3 error IRQ callback function.//串口接收错误中断回调
 */
static void Usart3ErrIrqCallback(void)
{
    if (Set == USART_GetStatus(M4_USART3, UsartFrameErr))
        USART_ClearStatus(M4_USART3, UsartFrameErr);

    if (Set == USART_GetStatus(M4_USART3, UsartParityErr))
        USART_ClearStatus(M4_USART3, UsartParityErr);

    if (Set == USART_GetStatus(M4_USART3, UsartOverrunErr))
        USART_ClearStatus(M4_USART3, UsartOverrunErr);
}

/**
 * @brief  Usart4ErrIrqCallback.
 * @param  None
 * @retval None
 * @note   USART4 error IRQ callback function.//串口接收错误中断回调
 */
static void Usart4ErrIrqCallback(void)
{
    if (Set == USART_GetStatus(M4_USART4, UsartFrameErr))
        USART_ClearStatus(M4_USART4, UsartFrameErr);

    if (Set == USART_GetStatus(M4_USART4, UsartParityErr))
        USART_ClearStatus(M4_USART4, UsartParityErr);

    if (Set == USART_GetStatus(M4_USART4, UsartOverrunErr))
        USART_ClearStatus(M4_USART4, UsartOverrunErr);
}

#ifndef USART1_RX_DMA
//unsigned short usRx1Cnt;
static void UsartRxIrqCallback(void)
{
    //usRx1Cnt++;
    uint16_t u16Data = USART_RecData(M4_USART1);
    pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[0][pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[0]++] = (unsigned char)u16Data; //USART_ReceiveData(M4_USART1);
}
#endif
/**
 * @brief  Usart1TimeoutIrqCallback.
 * @param  None
 * @retval None
 * @note   USART41 timeout IRQ callback function.
 *       This function is used to restart DMA when timeout occurs.
 */
//unsigned short gus_USART1TimeOutCnt = 0;
static void Usart1TimeoutIrqCallback(void)
{
    /* Recover clock. */
    PWC_IrqClkRecover();
    TIMER0_Cmd(M4_TMR01, Tim0_ChannelA,Disable);
    //TIMER0_ClearFlag (M4_TMR01, Tim0_ChannelA);
    USART_ClearStatus(M4_USART1, UsartRxTimeOut);
    //NVIC_ClearPendingIRQ(USART1_RTO_IRQn);
    //gus_USART1TimeOutCnt++;
    
    #ifdef USART1_RX_DMA
    pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[0] = USART_DATA_LEN_MAX - DMA_GetTransferCnt(M4_DMA1, DmaCh0);
    //pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[1] = USART_DATA_LEN_MAX - DMA_GetTransferCnt(M4_DMA1, DmaCh1);
    if(pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[0] >= USART_DATA_LEN_MAX)
    {
        if(strlen(pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[0]) < USART_DATA_LEN_MAX)
        {
            pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[0] = strlen(pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[0]);
            pst_UsartSystemPara->UsartData.ucUsartxRecvDataFlag[0] = 1;
        }
        else
        {
            pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[0] = 0;
            pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[0] = 0;
            memset(pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[0], 0, USART_DATA_LEN_MAX);
        }
        
    }
    else
    {
        //if(pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[0] == strlen(pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[0]))
        {
            pst_UsartSystemPara->UsartData.ucUsartxRecvDataFlag[0] = 1;
        }
    }
    

    DMA_ChannelCmd(M4_DMA1, DmaCh0, Disable);  //超时重启DMA，以进行新一轮的接收
    DMA_SetDesAddress(M4_DMA1, DmaCh0, (uint32_t)(pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[0]));
    DMA_SetTransferCnt(M4_DMA1, DmaCh0, USART_DATA_LEN_MAX);
    DMA_ChannelCmd(M4_DMA1, DmaCh0, Enable);
    #else
    pst_UsartSystemPara->UsartData.ucUsartxRecvDataFlag[0] = 1;
    #endif
    if(pst_UsartSystemPara->DeviceRunPara.enDeviceRunMode == DEVICE_RUN_STATE_SLEEP)
    {
        pst_UsartSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_RUN;
    }
    //TIMER0_Cmd(M4_TMR01, Tim0_ChannelA,Enable);
    /* Switch system clock as MRC. */
    PWC_IrqClkBackup();
}

/**
 * @brief  Usart2TimeoutIrqCallback.
 * @param  None
 * @retval None
 * @note   USART2 timeout IRQ callback function.
 *       This function is used to restart DMA when timeout occurs.
 */
static void Usart2TimeoutIrqCallback(void)
{
    #ifdef USART2_RX_DMA
    TIMER0_Cmd(M4_TMR01, Tim0_ChannelB,Disable);
    USART_ClearStatus(M4_USART2, UsartRxTimeOut);
    #else
    USART_ClearStatus(M4_USART2, UsartRxNoEmpty);
    #endif

    #ifdef USART2_RX_DMA
    //gusBTRecvCnt = 300 - DMA_GetTransferCnt(M4_DMA1, DmaCh1);
    pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[1] = USART_DATA_LEN_MAX - DMA_GetTransferCnt(M4_DMA1, DmaCh1);
    if(pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[1] >= USART_DATA_LEN_MAX)
    {
        pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[1] = 0;
        memset(pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[1], 0, USART_DATA_LEN_MAX);
    }
    else
    {
        pst_UsartSystemPara->UsartData.ucUsartxRecvDataFlag[1] = 1;
        pst_UsartSystemPara->DeviceRunPara.usDeviceBTWaitCnt = 0;
    }
    #else
    pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[1][pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[1]] = (unsigned char)USART_RecData(M4_USART2); //USART_ReceiveData(M4_USART2);
    pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[1] ++;//= USART_DATA_LEN_MAX - DMA_GetTransferCnt(M4_DMA1, DmaCh1);
    #endif
    //if(pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[1] == strlen(pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[1]))
    
    
    #ifdef USART2_RX_DMA
    DMA_ChannelCmd(M4_DMA1, DmaCh1, Disable);  //超时重启DMA，以进行新一轮的接收
    DMA_SetDesAddress(M4_DMA1, DmaCh1, (uint32_t)(&pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[1][0]));
    DMA_SetTransferCnt(M4_DMA1, DmaCh1, USART_DATA_LEN_MAX);
    //DMA_SetDesAddress(M4_DMA1, DmaCh1, (uint32_t)(&gucBTRecvArr[0]));
    //DMA_SetTransferCnt(M4_DMA1, DmaCh1, 300);
    DMA_ChannelCmd(M4_DMA1, DmaCh1, Enable);
    //pst_UsartSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_RUN;
    #endif

    func_BT_Dispose();
}

/**
 * @brief  Usart3TimeoutIrqCallback.
 * @param  None
 * @retval None
 * @note   USART3 timeout IRQ callback function.
 *       This function is used to restart DMA when timeout occurs.
 */
//unsigned char guc_TestCnt1 = 0;
//uint8_t guc_Usart3Cnt = 0;
//uint8_t guc_Usart3Data[10] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
static void Usart3TimeoutIrqCallback(void)
{
    //guc_TestCnt1++;
    #ifdef USART3_RX_DMA
    TIMER0_Cmd(M4_TMR02, Tim0_ChannelA,Disable);
    USART_ClearStatus(M4_USART3, UsartRxTimeOut);
    #else
    //USART_ClearStatus(M4_USART3, UsartRxNoEmpty);
    #endif

    #ifdef USART3_RX_DMA
    
    pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2] = USART_DATA_LEN_MAX - DMA_GetTransferCnt(M4_DMA1, DmaCh2);
    //gucNFCRecvData[guc_StartPosi++] = 0xB0;
    if(pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2] >= USART_DATA_LEN_MAX)
    {
        pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2] = 0;
        memset(pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[2], 0, USART_DATA_LEN_MAX);
    }
    else
    {
        //memcpy(&guc_Usart3Data[guc_Usart3Cnt], pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[2], pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2]);
        //guc_Usart3Cnt += pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2];
    }
    #else
    pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[2][pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2]] = (unsigned char)USART_RecData(M4_USART3); //USART_ReceiveData(M4_USART2);
    pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2] ++;
    #endif
    pst_UsartSystemPara->UsartData.ucUsartxRecvDataFlag[2] = 1;
    
    //memcpy(&guc_NFC_USART3_RecvData[guc_NFC_USART3_RecvCnt++][0], pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[2], pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2]);
    
    #ifdef USART3_RX_DMA
    DMA_ChannelCmd(M4_DMA1, DmaCh2, Disable);  //超时重启DMA，以进行新一轮的接收
    DMA_SetDesAddress(M4_DMA1, DmaCh2, (uint32_t)(&pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[2][0]));
    DMA_SetTransferCnt(M4_DMA1, DmaCh2, USART_DATA_LEN_MAX);
    DMA_ChannelCmd(M4_DMA1, DmaCh2, Enable);
    //pst_UsartSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_RUN;
    #endif
}

/**
 * @brief  Usart4TimeoutIrqCallback.
 * @param  None
 * @retval None
 * @note   USART4 timeout IRQ callback function.
 *       This function is used to restart DMA when timeout occurs.
 */
//unsigned short usUsart4RecvCnt = 0;
static void Usart4TimeoutIrqCallback(void)
{
    #ifdef USART4_RX_DMA
    TIMER0_Cmd(M4_TMR02, Tim0_ChannelB,Disable);
    USART_ClearStatus(M4_USART4, UsartRxTimeOut);
    #else
    USART_ClearStatus(M4_USART4, UsartRxNoEmpty);
    #endif
    //usUsart4RecvCnt++;
    #ifdef USART4_RX_DMA
    pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[3] = 1100 - DMA_GetTransferCnt(M4_DMA1, DmaCh3);
    if(pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[3] >= 1100)
    {
        pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[3] = 0;
        memset(pst_UsartSystemPara->UsartData.ucUsart4RecvDataArr, 0, 1100);
    }
    #else
    pst_UsartSystemPara->UsartData.ucUsart4RecvDataArr[pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[3]] = (unsigned char)USART_RecData(M4_USART4); //USART_ReceiveData(M4_USART2);
    pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[3] ++;//= USART_DATA_LEN_MAX - DMA_GetTransferCnt(M4_DMA1, DmaCh1);
    if(pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[3] >= 1100)
    {
        pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[3] = 0;
        memset(pst_UsartSystemPara->UsartData.ucUsart4RecvDataArr, 0, 1100);
    }
    #endif
    pst_UsartSystemPara->UsartData.ucUsartxRecvDataFlag[3] = 1;
    #ifdef USART4_RX_DMA
    DMA_ChannelCmd(M4_DMA1, DmaCh3, Disable);  //超时重启DMA，以进行新一轮的接收
    DMA_SetDesAddress(M4_DMA1, DmaCh3, (uint32_t)(pst_UsartSystemPara->UsartData.ucUsart4RecvDataArr));
    DMA_SetTransferCnt(M4_DMA1, DmaCh3, 1100);
    DMA_ChannelCmd(M4_DMA1, DmaCh3, Enable);
    #endif
    //pst_UsartSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_RUN;
}

/**
 * @brief  USART Usart1TxIrqCallback.
 * @param  None
 * @retval None
 * @note   USART1 TX IRQ callback function.
 */
static void Usart1TxIrqCallback(void)
{
    USART_SendData(M4_USART1, (uint16_t)pst_UsartSystemPara->UsartData.ucUsart1SendDataToServerArr[pst_UsartSystemPara->UsartData.usUsartxCurSendDataLen[0]]);
    pst_UsartSystemPara->UsartData.usUsartxCurSendDataLen[0]++;
    
    if (pst_UsartSystemPara->UsartData.usUsartxCurSendDataLen[0] >= pst_UsartSystemPara->UsartData.usUsartxSendDataLen[0])
    {
        USART_FuncCmd(M4_USART1, UsartTxEmptyInt, Disable);
        USART_FuncCmd(M4_USART1, UsartTxCmpltInt, Enable);
    }
}

/**
 * @brief  USART UsartTxCmpltIrqCallback.
 * @param  None
 * @retval None
 * @note   USART1 TX complete IRQ callback function.
 */
static void Usart1TxCmpltIrqCallback(void)
{
    USART_FuncCmd(M4_USART1, UsartTx, Disable);
    USART_FuncCmd(M4_USART1, UsartTxCmpltInt, Disable);
    pst_UsartSystemPara->UsartData.ucUsartxSendDataFlag[0] = 0;
    
}

/**
 * @brief  USART Usart2TxIrqCallback.
 * @param  None
 * @retval None
 * @note   USART2 TX IRQ callback function.
 */
static void Usart2TxIrqCallback(void)
{
    USART_SendData(M4_USART2, (uint16_t)pst_UsartSystemPara->UsartData.ucUsartxSendDataArr[0][pst_UsartSystemPara->UsartData.usUsartxCurSendDataLen[1]]);
    pst_UsartSystemPara->UsartData.usUsartxCurSendDataLen[1]++;
    
    if (pst_UsartSystemPara->UsartData.usUsartxCurSendDataLen[1] >= pst_UsartSystemPara->UsartData.usUsartxSendDataLen[1])
    {
        USART_FuncCmd(M4_USART2, UsartTxEmptyInt, Disable);
        USART_FuncCmd(M4_USART2, UsartTxCmpltInt, Enable);
        
    }
}

/**
 * @brief  USART UsartTxCmpltIrqCallback.
 * @param  None
 * @retval None
 * @note   USART2 TX complete IRQ callback function.
 */
static void Usart2TxCmpltIrqCallback(void)
{
    USART_FuncCmd(M4_USART2, UsartTx, Disable);
    USART_FuncCmd(M4_USART2, UsartTxCmpltInt, Disable);
    pst_UsartSystemPara->UsartData.ucUsartxSendDataFlag[1] = 0;
}

/**
 * @brief  USART Usart3TxIrqCallback.
 * @param  None
 * @retval None
 * @note   USART3 TX IRQ callback function.
 */
static void Usart3TxIrqCallback(void)
{
    USART_SendData(M4_USART3, (uint16_t)pst_UsartSystemPara->UsartData.ucUsartxSendDataArr[1][pst_UsartSystemPara->UsartData.usUsartxCurSendDataLen[2]]);
    pst_UsartSystemPara->UsartData.usUsartxCurSendDataLen[2]++;
    
    if (pst_UsartSystemPara->UsartData.usUsartxCurSendDataLen[2] >= pst_UsartSystemPara->UsartData.usUsartxSendDataLen[2])
    {
        USART_FuncCmd(M4_USART3, UsartTxEmptyInt, Disable);
        USART_FuncCmd(M4_USART3, UsartTxCmpltInt, Enable);
    }
}

/**
 * @brief  USART UsartTxCmpltIrqCallback.
 * @param  None
 * @retval None
 * @note   USART3 TX complete IRQ callback function.
 */
static void Usart3TxCmpltIrqCallback(void)
{
    USART_FuncCmd(M4_USART3, UsartTx, Disable);
    USART_FuncCmd(M4_USART3, UsartTxCmpltInt, Disable);
    pst_UsartSystemPara->UsartData.ucUsartxSendDataFlag[2] = 0;
}

/**
 * @brief  USART Usart4TxIrqCallback.
 * @param  None
 * @retval None
 * @note   USART4 TX IRQ callback function.
 */
static void Usart4TxIrqCallback(void)
{
    USART_SendData(M4_USART4, (uint16_t)pst_UsartSystemPara->UsartData.ucUsartxSendDataArr[2][pst_UsartSystemPara->UsartData.usUsartxCurSendDataLen[3]]);
    pst_UsartSystemPara->UsartData.usUsartxCurSendDataLen[3]++;
    
    if (pst_UsartSystemPara->UsartData.usUsartxCurSendDataLen[3] >= pst_UsartSystemPara->UsartData.usUsartxSendDataLen[3])
    {
        USART_FuncCmd(M4_USART4, UsartTxEmptyInt, Disable);
        USART_FuncCmd(M4_USART4, UsartTxCmpltInt, Enable);
    }
}

/**
 * @brief  USART UsartTxCmpltIrqCallback.
 * @param  None
 * @retval None
 * @note   USART4 TX complete IRQ callback function.
 */
static void Usart4TxCmpltIrqCallback(void)
{
    USART_FuncCmd(M4_USART4, UsartTx, Disable);
    USART_FuncCmd(M4_USART4, UsartTxCmpltInt, Disable);
    pst_UsartSystemPara->UsartData.ucUsartxSendDataFlag[3] = 0;
    if(pst_UsartSystemPara->UsartData.enUsart4Source == MODULE_MEAS_SENSOR2)
    {
        RST2_PIN_CLOSE();
    }
    else
    {
        RST1_PIN_CLOSE();
    }
}

/**
 * @brief  USART Initialize.
 * @param  None
 * @retval None
 */
void drv_mcu_USART_Init(void)
{
    en_result_t enRet = Ok;
    stc_irq_regi_conf_t stcIrqRegiCfg;

    MEM_ZERO_STRUCT(stcIrqRegiCfg);

    /*配置串口使用的时钟和基本通信配置*/
    stc_usart_uart_init_t stcInitCfg =
    {
        UsartIntClkCkOutput,
        UsartClkDiv_4,
        UsartDataBits8,
        UsartDataLsbFirst,
        UsartOneStopBit,
        UsartParityNone,
        UsartSampleBit8,
        UsartStartBitFallEdge,
        UsartRtsEnable,
    }; 

    pst_UsartSystemPara = GetSystemPara();

    /*DMA 配置初始化*/
    Dma1Init();

    /*打开时钟*/
    PWC_Fcg1PeriphClockCmd(PWC_FCG1_PERIPH_USART1, Enable);
    PWC_Fcg1PeriphClockCmd(PWC_FCG1_PERIPH_USART2, Enable);
    PWC_Fcg1PeriphClockCmd(PWC_FCG1_PERIPH_USART3, Enable);
    PWC_Fcg1PeriphClockCmd(PWC_FCG1_PERIPH_USART4, Enable);

    /*配置相应的IO作为串口的RX引脚*/
    PORT_SetFunc(USART1_RX_PORT, USART1_RX_PIN, USART1_RX_FUNC, Disable);
    PORT_SetFunc(USART1_TX_PORT, USART1_TX_PIN, USART1_TX_FUNC, Disable);

    PORT_SetFunc(USART2_RX_PORT, USART2_RX_PIN, USART2_RX_FUNC, Disable);
    PORT_SetFunc(USART2_TX_PORT, USART2_TX_PIN, USART2_TX_FUNC, Disable);
    #ifndef USART3_INIT
#if 0
    PORT_SetFunc(USART3_RX_BD_PORT, USART3_RX_BD_PIN, USART3_RX_FUNC, Disable);
    PORT_SetFunc(USART3_TX_BD_PORT, USART3_TX_BD_PIN, USART3_TX_FUNC, Disable);
#else
    PORT_SetFunc(USART3_RX_RFID_PORT, USART3_RX_RFID_PIN, USART3_RX_FUNC, Disable);
    PORT_SetFunc(USART3_TX_RFID_PORT, USART3_TX_RFID_PIN, USART3_TX_FUNC, Disable);
#endif
    #endif

    #ifndef USART4_INIT
    //PORT_SetFunc(USART4_RX_485_1_PORT, USART4_RX_485_1_PIN, USART4_RX_FUNC, Disable);
    //PORT_SetFunc(USART4_TX_485_1_PORT, USART4_TX_485_1_PIN, USART4_TX_FUNC, Disable);

    PORT_SetFunc(USART4_RX_485_2_PORT, USART4_RX_485_2_PIN, USART4_RX_FUNC, Disable);
    PORT_SetFunc(USART4_TX_485_2_PORT, USART4_TX_485_2_PIN, USART4_TX_FUNC, Disable);
    #endif

    /*初始化串口配置*/
    enRet = USART_UART_Init(M4_USART1, &stcInitCfg);
    if (enRet != Ok)while (1);

    /*初始化串口配置*/
    enRet = USART_UART_Init(M4_USART2, &stcInitCfg);
    if (enRet != Ok)while (1);

    #ifndef USART3_INIT
    /*初始化串口配置*/
    //stcInitCfg.enDetectMode = UsartStartBitLowLvl;
    enRet = USART_UART_Init(M4_USART3, &stcInitCfg);
    if (enRet != Ok)while (1);
    #endif

    #ifndef USART4_INIT
    /*初始化串口配置*/
    //stcInitCfg.enDetectMode = UsartStartBitFallEdge;
    enRet = USART_UART_Init(M4_USART4, &stcInitCfg);
    if (enRet != Ok)while (1);
    #endif
    /*串口波特率设置*/
    enRet = USART_SetBaudrate(M4_USART1, USART1_BAUDRATE);
    if (enRet != Ok)while (1);

    /*串口波特率设置*/
    enRet = USART_SetBaudrate(M4_USART2, USART2_BAUDRATE);
    if (enRet != Ok)while (1);

    #ifndef USART3_INIT
    /*串口波特率设置*/
    enRet = USART_SetBaudrate(M4_USART3, USART3_BAUDRATE);
    if (enRet != Ok)while (1);
    #endif

    #ifndef USART4_INIT
    /*串口波特率设置*/
    enRet = USART_SetBaudrate(M4_USART4, USART4_BAUDRATE);
    if (enRet != Ok)while (1);
    #endif

    /*设置串口接收中断舍弃*/

    /*设置串口接收错误中断*/
    stcIrqRegiCfg.enIRQn = USART1_EI_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart1ErrIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART1_EI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    stcIrqRegiCfg.enIRQn = USART2_EI_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart2ErrIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART2_EI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    #ifndef USART3_INIT
    stcIrqRegiCfg.enIRQn = USART3_EI_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart3ErrIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART3_EI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
    #endif

    #ifndef USART4_INIT
    stcIrqRegiCfg.enIRQn = USART4_EI_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart4ErrIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART4_EI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
    #endif

    #ifndef USART1_RX_DMA
    /* Set USART RX IRQ */
    stcIrqRegiCfg.enIRQn = USART1_RI_IRQn;
    stcIrqRegiCfg.pfnCallback = &UsartRxIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART1_RI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
    #endif
    /*设置接收超时中断*/
    stcIrqRegiCfg.enIRQn = USART1_RTO_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart1TimeoutIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART1_RTO_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_00);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    stcIrqRegiCfg.enIRQn = USART2_RTO_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart2TimeoutIrqCallback;
    #ifdef USART2_RX_DMA
    stcIrqRegiCfg.enIntSrc = USART2_RTO_NUM;
    #else
    stcIrqRegiCfg.enIntSrc = USART2_RI_NUM;
    #endif
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_01);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    #ifndef USART3_INIT
    stcIrqRegiCfg.enIRQn = USART3_RTO_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart3TimeoutIrqCallback;
    #ifdef USART3_RX_DMA
    stcIrqRegiCfg.enIntSrc = USART3_RTO_NUM;
    #else
    stcIrqRegiCfg.enIntSrc = USART3_RI_NUM;
    #endif
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_03);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
    #endif

    #ifndef USART4_INIT
    stcIrqRegiCfg.enIRQn = USART4_RTO_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart4TimeoutIrqCallback;
    #ifdef USART4_RX_DMA
    stcIrqRegiCfg.enIntSrc = USART4_RTO_NUM;
    #else
    stcIrqRegiCfg.enIntSrc = USART4_RI_NUM;
    #endif
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
    #endif

    /* Set USART TX IRQ */
    stcIrqRegiCfg.enIRQn = USART1_TI_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart1TxIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART1_TI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    stcIrqRegiCfg.enIRQn = USART2_TI_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart2TxIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART2_TI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    #ifndef USART3_INIT
    stcIrqRegiCfg.enIRQn = USART3_TI_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart3TxIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART3_TI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
    #endif

    #ifndef USART4_INIT
    stcIrqRegiCfg.enIRQn = USART4_TI_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart4TxIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART4_TI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
    #endif

    /* Set USART TX complete IRQ */
    stcIrqRegiCfg.enIRQn = USART1_TCI_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart1TxCmpltIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART1_TCI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    stcIrqRegiCfg.enIRQn = USART2_TCI_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart2TxCmpltIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART2_TCI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    #ifndef USART3_INIT
    stcIrqRegiCfg.enIRQn = USART3_TCI_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart3TxCmpltIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART3_TCI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
    #endif

    #ifndef USART4_INIT
    stcIrqRegiCfg.enIRQn = USART4_TCI_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart4TxCmpltIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART4_TCI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
    #endif

    USART_FuncCmd(M4_USART1, UsartRx, Enable);//使能接收
    USART_FuncCmd(M4_USART1, UsartRxInt, Enable);//使能接收中断
    USART_FuncCmd(M4_USART1, UsartTimeOut, Enable);//使能超时
    USART_FuncCmd(M4_USART1, UsartTimeOutInt, Enable);//使能超时中断

    USART_FuncCmd(M4_USART2, UsartRx, Enable);//使能接收
    USART_FuncCmd(M4_USART2, UsartRxInt, Enable);//使能接收中断
    #ifdef USART2_RX_DMA
    USART_FuncCmd(M4_USART2, UsartTimeOut, Enable);//使能超时
    USART_FuncCmd(M4_USART2, UsartTimeOutInt, Enable);//使能超时中断
    #endif

    #ifndef USART3_INIT
    USART_FuncCmd(M4_USART3, UsartRx, Enable);//使能接收
    USART_FuncCmd(M4_USART3, UsartRxInt, Enable);//使能接收中断
    
    #ifdef USART3_RX_DMA
    USART_FuncCmd(M4_USART3, UsartTimeOutInt, Enable);//使能超时中断
    USART_FuncCmd(M4_USART3, UsartTimeOut, Enable);//使能超时
    #endif
    #endif
    
    #ifndef USART4_INIT
    USART_FuncCmd(M4_USART4, UsartRx, Enable);//使能接收
    USART_FuncCmd(M4_USART4, UsartRxInt, Enable);//使能接收中断
    #ifdef USART4_RX_DMA
    USART_FuncCmd(M4_USART4, UsartTimeOut, Enable);//使能超时
    USART_FuncCmd(M4_USART4, UsartTimeOutInt, Enable);//使能超时中断
    #endif
    #endif
}

/**
 * @brief  USART send data.
 * @param  [in] pu8Buff                 Pointer to the buffer to be sent
 * @param  [in] u16Len                  Send buffer length
 * @retval None
 */
void drv_mcu_USART_SendData(en_usart_device_t e_Module_Type,uint8_t *pu8Buff, uint16_t u16Len)
{
    //uint16_t i = 0;
    //uint8_t ch = 0;
    //uint16_t j = 0;
    switch (e_Module_Type)
    {
    case MODULE_4G_NB:
        pst_UsartSystemPara->UsartData.usUsartxCurSendDataLen[0] = 0;
        pst_UsartSystemPara->UsartData.usUsartxSendDataLen[0] = u16Len;
        pst_UsartSystemPara->UsartData.ucUsartxSendDataFlag[0] = 1;
        memset(pst_UsartSystemPara->UsartData.ucUsart1SendDataToServerArr, 0, USART_DATA_LEN_MAX);
        memcpy(pst_UsartSystemPara->UsartData.ucUsart1SendDataToServerArr, pu8Buff, u16Len);
        USART_FuncCmd(M4_USART1, UsartTxAndTxEmptyInt, Enable);
        break;
    case MODULE_BT:
        pst_UsartSystemPara->UsartData.usUsartxCurSendDataLen[1] = 0;
        pst_UsartSystemPara->UsartData.usUsartxSendDataLen[1] = u16Len;
        pst_UsartSystemPara->UsartData.ucUsartxSendDataFlag[1] = 1;
        memset(pst_UsartSystemPara->UsartData.ucUsartxSendDataArr[0], 0, USART_DATA_LEN_MAX);
        memcpy(pst_UsartSystemPara->UsartData.ucUsartxSendDataArr[0], pu8Buff, u16Len);
        USART_FuncCmd(M4_USART2, UsartTxAndTxEmptyInt, Enable);
        break;
    case MODULE_BD:
    case MODULE_NFC_RFID:
        pst_UsartSystemPara->UsartData.usUsartxCurSendDataLen[2] = 0;
        pst_UsartSystemPara->UsartData.usUsartxSendDataLen[2] = u16Len;
        pst_UsartSystemPara->UsartData.ucUsartxSendDataFlag[2] = 1;
        memset(pst_UsartSystemPara->UsartData.ucUsartxSendDataArr[1], 0, USART_DATA_LEN_MAX);
        //memset(pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[2], 0, USART_DATA_LEN_MAX);
        memcpy(pst_UsartSystemPara->UsartData.ucUsartxSendDataArr[1], pu8Buff, u16Len);
        #if 1
        USART_FuncCmd(M4_USART3, UsartTxAndTxEmptyInt, Enable);
        #else
        for(j=0; j<u16Len; j++)
        {
            ch = pu8Buff[j];
            i = 0;
            USART_SendData(M4_USART3, (uint16_t)ch);
            while(USART_GetStatus(M4_USART3, UsartTxEmpty) == Reset)
            {
                i++;
                if(i >= 10000)
                {
                    break;
                }
            };
        }
        
        #endif
        break;
    case MODULE_MEAS_SENSOR1:
    case MODULE_MEAS_SENSOR2:
        pst_UsartSystemPara->UsartData.usUsartxCurSendDataLen[3] = 0;
        pst_UsartSystemPara->UsartData.usUsartxSendDataLen[3] = u16Len;
        pst_UsartSystemPara->UsartData.ucUsartxSendDataFlag[3] = 1;
        memset(pst_UsartSystemPara->UsartData.ucUsartxSendDataArr[2], 0, USART4_SEND_DATA_MAX);
        memcpy(pst_UsartSystemPara->UsartData.ucUsartxSendDataArr[2], pu8Buff, u16Len);
        USART_FuncCmd(M4_USART4, UsartTxAndTxEmptyInt, Enable);
        break;
    default:
        break;
    }
    
}

//设置USART3波特率,从9600设置为115200
void drv_mcu_SetUSART3_Baud(void)
{
    USART_SetBaudrate(M4_USART3, 115200ul);
}

void drv_mcu_USART3_DMA_RX_Special_Init()
{
    stc_dma_config_t stcDmaInit;
    stc_irq_regi_conf_t stcIrqRegiCfg;

    /* Initialize DMA. */
    MEM_ZERO_STRUCT(stcDmaInit);
    stcDmaInit.u16BlockSize = 1u; /* 1 block */
    stcDmaInit.u32SrcAddr = ((uint32_t)(&M4_USART3->DR)+2ul); /* Set source address. receive register*/
    stcDmaInit.u32DesAddr = (uint32_t)(&pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[2][0]);
    stcDmaInit.stcDmaChCfg.enSrcInc = AddressFix;  /* Set source address mode. */
    stcDmaInit.stcDmaChCfg.enDesInc = AddressIncrease;  /* Set destination address mode. */
    stcDmaInit.stcDmaChCfg.enIntEn = Enable;       /* Enable interrupt. */
    stcDmaInit.stcDmaChCfg.enTrnWidth = Dma8Bit;   /* Set data width 8bit. */
    DMA_InitChannel(M4_DMA1, DmaCh2, &stcDmaInit);

    DMA_ChannelCmd(M4_DMA1, DmaCh2, Enable);
    DMA_ClearIrqFlag(M4_DMA1, DmaCh2, TrnCpltIrq);
    DMA_SetTriggerSrc(M4_DMA1, DmaCh2, EVT_USART3_RI);

    stcIrqRegiCfg.enIRQn = DMA1_CH2_BTCINT_IRQn;
    stcIrqRegiCfg.pfnCallback = &Dma1Btc2IrqCallback;
    stcIrqRegiCfg.enIntSrc = DMA1_CH2_BTCINT_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_01);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
}

void drv_mcu_USART3_Special_Init(en_usart_device_t e_Module_Type)
{
    en_result_t enRet = Ok;
    stc_irq_regi_conf_t stcIrqRegiCfg;
    stc_port_init_t stcPortInit;

	MEM_ZERO_STRUCT(stcPortInit);

	stcPortInit.enPinMode = Pin_Mode_Out; 

    MEM_ZERO_STRUCT(stcIrqRegiCfg);
    stc_usart_uart_init_t stcInitCfg =
    {
        UsartIntClkCkOutput,
        UsartClkDiv_4,
        UsartDataBits8,
        UsartDataLsbFirst,
        UsartOneStopBit,
        UsartParityNone,
        UsartSampleBit8,
        UsartStartBitFallEdge,
        UsartRtsEnable,
    }; 

    USART_DeInit(M4_USART3);
    PWC_Fcg1PeriphClockCmd(PWC_FCG1_PERIPH_USART3, Enable);

    switch (e_Module_Type)
    {
    case MODULE_BD:
        PORT_SetFunc(USART3_RX_RFID_PORT, USART3_RX_RFID_PIN, Func_Spi4_Miso, Disable);
        PORT_SetFunc(USART3_TX_RFID_PORT, USART3_TX_RFID_PIN, Func_Spi4_Mosi, Disable);
        PORT_SetFunc(USART3_RX_BD_PORT, USART3_RX_BD_PIN, USART3_RX_FUNC, Disable);
        PORT_SetFunc(USART3_TX_BD_PORT, USART3_TX_BD_PIN, USART3_TX_FUNC, Disable);
        //PORT_Init(USART3_RX_RFID_PORT, USART3_RX_RFID_PIN, &stcPortInit);
        //PORT_Init(USART3_TX_RFID_PORT, USART3_TX_RFID_PIN, &stcPortInit);
        //PORT_ResetBits(USART3_RX_RFID_PORT,USART3_RX_RFID_PIN);
        //PORT_ResetBits(USART3_TX_RFID_PORT,USART3_TX_RFID_PIN);
        break;
    case MODULE_NFC_RFID:
        PORT_SetFunc(USART3_RX_BD_PORT, USART3_RX_BD_PIN, Func_Spi4_Miso, Disable);
        PORT_SetFunc(USART3_TX_BD_PORT, USART3_TX_BD_PIN, Func_Spi4_Mosi, Disable);
        PORT_SetFunc(USART3_RX_RFID_PORT, USART3_RX_RFID_PIN, USART3_RX_FUNC, Disable);
        PORT_SetFunc(USART3_TX_RFID_PORT, USART3_TX_RFID_PIN, USART3_TX_FUNC, Disable);
        //PORT_Init(USART3_RX_BD_PORT, USART3_RX_BD_PIN, &stcPortInit);
        //PORT_Init(USART3_TX_BD_PORT, USART3_TX_RFID_PIN, &stcPortInit);
        //PORT_ResetBits(USART3_RX_BD_PORT,USART3_RX_BD_PIN);
        //PORT_ResetBits(USART3_TX_BD_PORT,USART3_TX_RFID_PIN);
        break;
    default:
        break;
    }

    /*初始化串口配置*/
    //stcInitCfg.enDetectMode = UsartStartBitLowLvl;
    enRet = USART_UART_Init(M4_USART3, &stcInitCfg);
    if (enRet != Ok)while (1);
    /*串口波特率设置*/
    USART_SetBaudrate(M4_USART3, USART3_BAUDRATE);

    #ifndef HW_VERSION_V1_1
    if(e_Module_Type == MODULE_BD)
    #endif
    {
        stcIrqRegiCfg.enIRQn = USART3_EI_IRQn;
        stcIrqRegiCfg.pfnCallback = &Usart3ErrIrqCallback;
        stcIrqRegiCfg.enIntSrc = USART3_EI_NUM;
        enIrqRegistration(&stcIrqRegiCfg);
        NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_01);
        NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
        NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

        stcIrqRegiCfg.enIRQn = USART3_RTO_IRQn;
        stcIrqRegiCfg.pfnCallback = &Usart3TimeoutIrqCallback;
        #ifdef USART3_RX_DMA
        stcIrqRegiCfg.enIntSrc = USART3_RTO_NUM;
        #else
        stcIrqRegiCfg.enIntSrc = USART3_RI_NUM;
        #endif
        enIrqRegistration(&stcIrqRegiCfg);
        NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_00);
        NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
        NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

        #if 1
        stcIrqRegiCfg.enIRQn = USART3_TI_IRQn;
        stcIrqRegiCfg.pfnCallback = &Usart3TxIrqCallback;
        stcIrqRegiCfg.enIntSrc = USART3_TI_NUM;
        enIrqRegistration(&stcIrqRegiCfg);
        NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_01);
        NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
        NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

        stcIrqRegiCfg.enIRQn = USART3_TCI_IRQn;
        stcIrqRegiCfg.pfnCallback = &Usart3TxCmpltIrqCallback;
        stcIrqRegiCfg.enIntSrc = USART3_TCI_NUM;
        enIrqRegistration(&stcIrqRegiCfg);
        NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_01);
        NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
        NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);
        #endif
        USART_FuncCmd(M4_USART3, UsartRx, Enable);//使能接收
        USART_FuncCmd(M4_USART3, UsartRxInt, Enable);//使能接收中断
        
        #ifdef USART3_RX_DMA
        USART_FuncCmd(M4_USART3, UsartTimeOutInt, Enable);//使能超时中断
        USART_FuncCmd(M4_USART3, UsartTimeOut, Enable);//使能超时
        #endif

        //USART_FuncCmd(M4_USART3, UsartRx, Enable);//使能接收
        #ifndef HW_VERSION_V1_1
        USART_FuncCmd(M4_USART3, UsartTx, Enable);//使能接收
        #endif
    }
    #ifndef HW_VERSION_V1_1
    else
    {
        USART_FuncCmd(M4_USART3, UsartRx, Enable);//使能接收
        USART_FuncCmd(M4_USART3, UsartTx, Enable);//使能接收
    }
   #endif
}

void drv_mcu_ChangeUSART3_Source(en_usart_device_t e_Module_Type)
{
    #ifdef HW_VERSION_V1_1
    drv_mcu_USART3_Special_Init(e_Module_Type);
    #else
    #ifdef USART3_INIT
    #ifdef USART3_RX_DMA
    DMA_DeInit(M4_DMA1, DmaCh2);
    if(e_Module_Type == MODULE_BD)
    {
        drv_mcu_USART3_DMA_RX_Special_Init();
    }
    #endif
    drv_mcu_USART3_Special_Init(e_Module_Type);
    #endif
    #endif
    switch (e_Module_Type)
    {
    case MODULE_BD:
        //关闭RF供电电源
        PWRRF_PIN_CLOSE();
        //开启供电
        PWRBD_PIN_CLOSE();
        Ddl_Delay1ms(100);
        PWRBD_PIN_OPEN();
        Ddl_Delay1ms(100);

        //复位模块
        BDRST_PIN_SET;
        Ddl_Delay1ms(100);
        BDRST_PIN_RESET;
        Ddl_Delay1ms(1000);
        BDRST_PIN_SET;
        Ddl_Delay1ms(100);
        pst_UsartSystemPara->UsartData.enUsart3Source = MODULE_BD;
        break;
    case MODULE_NFC_RFID:
        //关闭BD模块电源
        PWRBD_PIN_CLOSE();
        //开启RF供电电源
        PWRRF_PIN_CLOSE();
        Ddl_Delay1ms(100);
        PWRRF_PIN_OPEN();

        #ifndef HW_VERSION_V1_1
        //RF模块复位
        RFRST_PIN_SET;
        Ddl_Delay1ms(100);
        RFRST_PIN_RESET;
        Ddl_Delay1ms(100);
        RFRST_PIN_SET;
        Ddl_Delay1ms(100);
        #endif
        pst_UsartSystemPara->UsartData.enUsart3Source = MODULE_NFC_RFID;
        break;
    default:
        break;
    }
}

void drv_mcu_USART4_Special_Init(en_usart_device_t e_Module_Type, unsigned long ulBaudRate)
{
    en_result_t enRet = Ok;
    stc_irq_regi_conf_t stcIrqRegiCfg;
    stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);
    MEM_ZERO_STRUCT(stcIrqRegiCfg);
    stc_usart_uart_init_t stcInitCfg =
    {
        UsartIntClkCkOutput,
        UsartClkDiv_4,
        UsartDataBits8,
        UsartDataLsbFirst,
        UsartOneStopBit,
        UsartParityNone,
        UsartSampleBit8,
        UsartStartBitFallEdge,
        UsartRtsEnable,
    }; 
    
    NVIC_DisableIRQ(USART4_EI_IRQn); //使能USART4错误中断
    NVIC_DisableIRQ(USART4_RTO_IRQn); //使能USART4错误中断
    NVIC_DisableIRQ(USART4_TI_IRQn); //使能USART4错误中断
    NVIC_DisableIRQ(USART4_TCI_IRQn); //使能USART4错误中断
    USART_FuncCmd(M4_USART4, UsartRx, Disable);//使能接收
    USART_FuncCmd(M4_USART4, UsartRxInt, Disable);//使能接收中断
    #ifdef USART4_RX_DMA
    USART_FuncCmd(M4_USART4, UsartTimeOutInt, Disable);//使能超时中断
    USART_FuncCmd(M4_USART4, UsartTimeOut, Disable);//使能超时
    #endif
    //PORT_SetFunc(USART4_RX_485_2_PORT, USART4_RX_485_2_PIN, USART4_RX_FUNC, Enable);
    //PORT_SetFunc(USART4_TX_485_2_PORT, USART4_TX_485_2_PIN, USART4_TX_FUNC, Enable);
    //PORT_SetFunc(USART4_RX_485_1_PORT, USART4_RX_485_1_PIN, USART4_RX_FUNC, Enable);
    //PORT_SetFunc(USART4_TX_485_1_PORT, USART4_TX_485_1_PIN, USART4_TX_FUNC, Enable);
    PWC_Fcg1PeriphClockCmd(PWC_FCG1_PERIPH_USART4, Disable);
    enRet = USART_DeInit(M4_USART4);
    PWC_Fcg1PeriphClockCmd(PWC_FCG1_PERIPH_USART4, Enable);

    switch (e_Module_Type)
    {
    case MODULE_MEAS_SENSOR1:
        
        //stcPortInit.enPinMode = Pin_Mode_Ana;
        //PORT_Init(USART4_RX_485_2_PORT, USART4_RX_485_2_PIN, &stcPortInit);
        //PORT_Init(USART4_TX_485_2_PORT, USART4_TX_485_2_PIN, &stcPortInit);
        //stcPortInit.enPinMode = Pin_Mode_In;
        //PORT_Init(USART4_RX_485_2_PORT, USART4_RX_485_2_PIN, &stcPortInit);
        //7.3 增加下面这条语句的原因是，在进行串口引脚切换时，485_1切换到485_2后，再切换回485_1的时候接收数据异常,
        //因此在485_1进行设置前，先将485_2RX引脚映射为其他功能引脚
        PORT_SetFunc(USART4_RX_485_2_PORT, USART4_RX_485_2_PIN, Func_Spi4_Miso, Disable);
        PORT_SetFunc(USART4_RX_485_1_PORT, USART4_RX_485_1_PIN, USART4_RX_FUNC, Disable);
        PORT_SetFunc(USART4_TX_485_1_PORT, USART4_TX_485_1_PIN, USART4_TX_FUNC, Disable);
        
        break;
    case MODULE_MEAS_SENSOR2:
        //stcPortInit.enPinMode = Pin_Mode_In;
        //PORT_Init(USART4_RX_485_2_PORT, USART4_RX_485_2_PIN, &stcPortInit);
        PORT_SetFunc(USART4_RX_485_2_PORT, USART4_RX_485_2_PIN, USART4_RX_FUNC, Disable);
        PORT_SetFunc(USART4_TX_485_2_PORT, USART4_TX_485_2_PIN, USART4_TX_FUNC, Disable);
        
        break;
    default:
        break;
    }

    /*初始化串口配置*/
    //stcInitCfg.enDetectMode = UsartStartBitLowLvl;
    enRet = USART_UART_Init(M4_USART4, &stcInitCfg);
    if (enRet != Ok)while (1);
    /*串口波特率设置*/
    if(ulBaudRate == 115200)
    {
        USART_SetBaudrate(M4_USART4, 115200);
    }
    else
    {
        USART_SetBaudrate(M4_USART4, USART4_BAUDRATE);
    }
    

    stcIrqRegiCfg.enIRQn = USART4_EI_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart4ErrIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART4_EI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    stcIrqRegiCfg.enIRQn = USART4_RTO_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart4TimeoutIrqCallback;
    #ifdef USART4_RX_DMA
    stcIrqRegiCfg.enIntSrc = USART4_RTO_NUM;
    #else
    stcIrqRegiCfg.enIntSrc = USART4_RI_NUM;
    #endif
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    stcIrqRegiCfg.enIRQn = USART4_TI_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart4TxIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART4_TI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    stcIrqRegiCfg.enIRQn = USART4_TCI_IRQn;
    stcIrqRegiCfg.pfnCallback = &Usart4TxCmpltIrqCallback;
    stcIrqRegiCfg.enIntSrc = USART4_TCI_NUM;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    USART_FuncCmd(M4_USART4, UsartRx, Enable);//使能接收
    USART_FuncCmd(M4_USART4, UsartRxInt, Enable);//使能接收中断
    #ifdef USART4_RX_DMA
    USART_FuncCmd(M4_USART4, UsartTimeOutInt, Enable);//使能超时中断
    USART_FuncCmd(M4_USART4, UsartTimeOut, Enable);//使能超时
    #endif
}

void drv_mcu_ChangeUSART4_Source(en_usart_device_t e_Module_Type,unsigned long ulBaud)
{
    pst_UsartSystemPara->UsartData.enUsart4Source = e_Module_Type;
    drv_mcu_USART4_Special_Init(e_Module_Type, ulBaud);
    
}

void drv_mcu_ChangeUSART4_Baud(unsigned long ulValue)
{
    if(ulValue == 115200)
    {
        USART_SetBaudrate(M4_USART4, 115200ul);
    }
    else if(ulValue == 9600)
    {
        USART_SetBaudrate(M4_USART4, 9600ul);
    }
    else
    {
        USART_SetBaudrate(M4_USART4, USART4_BAUDRATE);
    }
}

void putchar(uint8_t ch)
{
    uint16_t i = 0;
    USART_SendData(M4_USART3, (uint16_t)ch);
    while(USART_GetStatus(M4_USART3, UsartTxEmpty) == Reset)
    {
        i++;
        if(i >= 10000)
        {
            break;
        }
    };
}

uint8_t getchar(uint8_t *ch, uint16_t usTimeOut)
{
    uint16_t i = 0;

    while(USART_GetStatus(M4_USART3, UsartRxNoEmpty) == Reset)
    {
        i++;
        if(i >= usTimeOut)
        {
            break;
        }
    };
    if(i >= usTimeOut)
    {
        return 1;
    }
    else
    {
        *ch = (uint8_t)USART_RecData(M4_USART3);
        return 0;
    }
}

void drv_RF_NFC_SendData(uint8_t ucCMD, uint8_t ucData)
{
    uint8_t ucSendBuf[20];
    uint16_t usWaitCnt = 20000;
    //uint16_t i = 0;
    uint8_t ucRecv = 0;
    ucSendBuf[0] = ucCMD & 0x3F;
    ucSendBuf[1] = ucData;

    #if 0
    ucSendBuf[2] = ucCMD | 0x80;
    drv_mcu_USART_SendData(MODULE_NFC_RFID, &ucSendBuf[2], 1);
    Ddl_Delay1ms(30);
    #endif
    #if 0
    drv_mcu_USART_SendData(MODULE_NFC_RFID, &ucSendBuf[0], 1);
    while(pst_UsartSystemPara->UsartData.ucUsartxSendDataFlag[2] == 1)
    {
        //等待发送完成
        i++;
        if(i >= usWaitCnt)
        {
            break;
        }
    }
    drv_mcu_USART_SendData(MODULE_NFC_RFID, &ucSendBuf[1], 1);
    i = 0;
    while(pst_UsartSystemPara->UsartData.ucUsartxSendDataFlag[2] == 1)
    {
        //等待发送完成
        i++;
        if(i >= usWaitCnt)
        {
            break;
        }
    }
    //等待接收到数据
    i = 0;
    while(pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2] == 0)
    {
        i++;
        if(i >= usWaitCnt)
        {
            break;
        }
    }

    #else
    putchar(ucSendBuf[0]);
    putchar(ucSendBuf[1]);
    getchar(&ucRecv, usWaitCnt);
    #endif
    //Ddl_Delay1ms(20);
}


uint8_t drv_RF_NFC_ReceiveData(uint8_t ucCMD)
{
    uint8_t ucSend;
    uint8_t ucRecv;
    uint8_t ucStatus = 1;
    uint16_t usWaitCnt = 10000;
    //uint16_t i = 0;
    ucSend = ucCMD | 0x80;
    #if 0
    drv_mcu_USART_SendData(MODULE_NFC_RFID, &ucSend, 1);
    //Ddl_Delay1ms(20);
    #if 1
    #if 0
    if(pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2] > 0)
    {
        #if 0
        usDataLen = pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2];
        memcpy(ucDataArr, pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[2], usDataLen);
        pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2] = 0;
        #else
        ucRecv = pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[2][0];
        pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2] = 0;
        #endif
    }
    else
    {
        ucRecv = 0;//memset(ucDataArr, 0, usDataLen);
    }
    #else
    while(pst_UsartSystemPara->UsartData.ucUsartxSendDataFlag[2] == 1)
    {
        //等待发送完成
        i++;
        if(i >= usWaitCnt)
        {
            break;
        }
    }
    i = 0;
    while(pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2] == 0)
    {
        i++;
        if(i >= usWaitCnt)
        {
            break;
        }
    }
    if(i < usWaitCnt)
    {
        //gucNFCRecvCnt = 0;
        //memset(gucNFCRecvData, 0, sizeof(gucNFCRecvData));
        
        memcpy(&gucNFCRecvData[gucNFCRecvCnt], pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[2], pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2]);
        gucNFCRecvCnt += pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2];
        ucRecv = pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[2][pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2]-1];
       
        gucNFCRecvData[guc_StartPosi++] = 0xA0;

        //memcpy(&guc_NFC_GetData[guc_NFC_GetDataCnt++][0], pst_UsartSystemPara->UsartData.ucUsartxRecvDataArr[2], pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2]);
        pst_UsartSystemPara->UsartData.usUsartxRecvDataLen[2] = 0;
    }
    else
    {
        ucRecv = 0;
    }
    #endif
    return ucRecv;
    #endif

    #else
    putchar(ucSend);
    ucStatus = getchar(&ucRecv, usWaitCnt);
    if(ucStatus == 0)
    {
        return ucRecv;
    }
    else
    {
        return 0xFF;
    }
    #endif
}
/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
