/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\driver\drv_device\drv_SPI\drv_SPI.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-20       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "drv_SPI.h"

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

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

 /**
 *******************************************************************************
 ** \brief SPI Init function
 **
 ** \param [in]  None
 **
 ** \retval 0: init success; 1: init failed.
 **
 ** \note 
 ******************************************************************************/
uint8_t drv_mcu_SPI_Init(void)
{
	stc_spi_init_t stcSpiInit;

    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcSpiInit);

    /* Configuration peripheral clock */
    #ifdef SPI_HART
    PWC_Fcg1PeriphClockCmd(SPI_W25Q128_UNIT_CLOCK, Enable);
    #ifndef HW_VERSION_V1_1
	PWC_Fcg1PeriphClockCmd(SPI_LORA_RA02_UNIT_CLOCK, Enable);
    #endif
    
    /* Configuration SPI pin */
    PORT_SetFunc(SPI_W25Q128_SCK_PORT, SPI_W25Q128_SCK_PIN, SPI_W25Q128_SCK_FUNC, Disable);
    //PORT_SetFunc(SPI_W25Q128_NSS_PORT, SPI_W25Q128_NSS_PIN, SPI_W25Q128_NSS_FUNC, Disable);
    PORT_SetFunc(SPI_W25Q128_MOSI_PORT, SPI_W25Q128_MOSI_PIN, SPI_W25Q128_MOSI_FUNC, Disable);
    PORT_SetFunc(SPI_W25Q128_MISO_PORT, SPI_W25Q128_MISO_PIN, SPI_W25Q128_MISO_FUNC, Disable);

    #ifndef HW_VERSION_V1_1
	PORT_SetFunc(SPI_LORA_RA02_SCK_PORT, SPI_LORA_RA02_SCK_PIN, SPI_LORA_RA02_SCK_FUNC, Disable);
    //PORT_SetFunc(SPI_LORA_RA02_NSS_PORT, SPI_LORA_RA02_NSS_PIN, SPI_LORA_RA02_NSS_FUNC, Disable);
    PORT_SetFunc(SPI_LORA_RA02_MOSI_PORT, SPI_LORA_RA02_MOSI_PIN, SPI_LORA_RA02_MOSI_FUNC, Disable);
    PORT_SetFunc(SPI_LORA_RA02_MISO_PORT, SPI_LORA_RA02_MISO_PIN, SPI_LORA_RA02_MISO_FUNC, Disable);
    #endif
    #endif
    /* Configuration SPI structure */
    stcSpiInit.enClkDiv = SpiClkDiv64;
    stcSpiInit.enFrameNumber = SpiFrameNumber1;
    stcSpiInit.enDataLength = SpiDataLengthBit8;
    stcSpiInit.enFirstBitPosition = SpiFirstBitPositionMSB;
    stcSpiInit.enSckPolarity = SpiSckIdleLevelLow;
    stcSpiInit.enSckPhase = SpiSckOddSampleEvenChange;
    stcSpiInit.enReadBufferObject = SpiReadReceiverBuffer;
    stcSpiInit.enWorkMode = SpiWorkMode3Line;
    stcSpiInit.enTransMode = SpiTransFullDuplex;
    stcSpiInit.enCommAutoSuspendEn = Disable;
    stcSpiInit.enModeFaultErrorDetectEn = Disable;
    stcSpiInit.enParitySelfDetectEn = Disable;
    stcSpiInit.enParityEn = Disable;
    stcSpiInit.enParity = SpiParityEven;

    stcSpiInit.enMasterSlaveMode = SpiModeMaster;
    stcSpiInit.stcDelayConfig.enSsSetupDelayOption = SpiSsSetupDelayCustomValue;
    stcSpiInit.stcDelayConfig.enSsSetupDelayTime = SpiSsSetupDelaySck1;
    stcSpiInit.stcDelayConfig.enSsHoldDelayOption = SpiSsHoldDelayCustomValue;
    stcSpiInit.stcDelayConfig.enSsHoldDelayTime = SpiSsHoldDelaySck1;
    stcSpiInit.stcDelayConfig.enSsIntervalTimeOption = SpiSsIntervalCustomValue;
    stcSpiInit.stcDelayConfig.enSsIntervalTime = SpiSsIntervalSck6PlusPck2;
    //stcSpiInit.stcSsConfig.enSsValidBit = SpiSsValidChannel0;
    //stcSpiInit.stcSsConfig.enSs0Polarity = SpiSsLowValid;

    #ifdef SPI_HART
    SPI_Init(SPI_W25Q128_UNIT, &stcSpiInit);
    SPI_Cmd(SPI_W25Q128_UNIT, Enable);
    
    #ifndef HW_VERSION_V1_1
	SPI_Init(SPI_LORA_RA02_UNIT, &stcSpiInit);
    SPI_Cmd(SPI_LORA_RA02_UNIT, Enable);
    #endif
    #endif
	return 0;
}


/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
