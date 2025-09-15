/*******************************************************************************
 * Copyright (C) 2020, Huada Semiconductor Co., Ltd. All rights reserved.
 *
 * This software component is licensed by HDSC under BSD 3-Clause license
 * (the "License"); You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                    opensource.org/licenses/BSD-3-Clause
 */
/******************************************************************************/
/** \file main.c
 **
 ** \brief This sample demonstrates how to check UART data receive by timeout.
 **
 **   - 2021-04-16 CDT First version for Device Driver Library of USART
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ddl.h"
#include "hc32f460_sram.h"
#include "hc32f460_efm.h"
#include "hc32f460_clk.h"
#include "hc32f460_gpio.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "4G_EC200U.h"
#include "User_Data.h"
#include "drv_USART.h"
#include "drv_Timer.h"
#include "Mainloop.h"
#include "drv_SPI.h"
#include "drv_Storage_W25Q128.h"
#include "drv_SC7A20H.h"
#include "drv_ADC.h"
#include "Magnetic_Bar.h"
#include "WatchDog.h"
#include "LoRa_RA02.h"
#include "Photosensitive_XYC_ALS.h"
#include "OLED.h"
#include "Display.h"
#include "LC86L_BD.h"
#include "RF_NFC.h"
#include "BT_TB05.h"
#include "drv_RTC.h"
#include "drv_LKT4202.h"
#include "ModbusRTU.h"
#include "Lora_TP1109.h"
#include "RXNTTL518.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
//#define JOE_TEST    1   //测试变量
/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

 #define EXAMPLE_PERIPH_WP               (LL_PERIPH_GPIO | LL_PERIPH_EFM | LL_PERIPH_FCG | \
    LL_PERIPH_PWC_CLK_RMU | LL_PERIPH_SRAM)
/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static SystemPataSt *pst_MainSystemPara;
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

static stc_clk_sysclk_cfg_t m_stcSysclkCfg =
{
    /* Default system clock division. */
    .enHclkDiv  = ClkSysclkDiv1,  // 40MHz
    .enExclkDiv = ClkSysclkDiv1,  // 40MHz
    .enPclk0Div = ClkSysclkDiv1,  // 40MHz
    .enPclk1Div = ClkSysclkDiv1,  // 40MHz
    .enPclk2Div = ClkSysclkDiv1,  // 40MHz
    .enPclk3Div = ClkSysclkDiv1,  // 40MHz
    .enPclk4Div = ClkSysclkDiv1,  // 40MHz
};

/**
 *******************************************************************************
 ** \brief  drv_mcu_Clock_Init
 ** \param  None
 ** \retval None
 ** \note   Initialize the clock. use mpll  40M
 ******************************************************************************/
static int drv_mcu_Clock_Init(void)
{
    stc_clk_mpll_cfg_t      stcMpllCfg;
    stc_sram_config_t       stcSramConfig;
    stc_clk_xtal_cfg_t stcXtalCfg;

    MEM_ZERO_STRUCT(stcMpllCfg);
    MEM_ZERO_STRUCT(stcSramConfig);

    /* Set bus clock division first. */
    CLK_SysClkConfig(&m_stcSysclkCfg);

    /* Switch system clock source to MPLL. */
    /* Use XTAL as MPLL source. */
    stcXtalCfg.enFastStartup = Disable;
    stcXtalCfg.enMode = ClkXtalModeOsc;
    stcXtalCfg.enDrv  = ClkXtalLowDrv;
    CLK_XtalConfig(&stcXtalCfg);
    CLK_XtalCmd(Enable);

    /* MPLL config (XTAL / pllmDiv * plln / PllpDiv = 40M). */
    stcMpllCfg.pllmDiv = 1ul;
    stcMpllCfg.plln =50ul;
    stcMpllCfg.PllpDiv = 10ul;
    stcMpllCfg.PllqDiv = 10ul;
    stcMpllCfg.PllrDiv = 10ul;
    CLK_SetPllSource(ClkPllSrcXTAL);
    CLK_MpllConfig(&stcMpllCfg);

    /* flash read wait cycle setting */
    EFM_Unlock();
    EFM_SetLatency(EFM_LATENCY_5);
    EFM_Lock();

    /* sram init include read/write wait cycle setting */
    stcSramConfig.u8SramIdx = Sram12Idx | Sram3Idx | SramHsIdx | SramRetIdx;
    stcSramConfig.enSramRC = SramCycle2;
    stcSramConfig.enSramWC = SramCycle2;
    stcSramConfig.enSramEccMode = EccMode3;
    stcSramConfig.enSramEccOp = SramNmi;
    stcSramConfig.enSramPyOp = SramNmi;
    SRAM_Init(&stcSramConfig);

    /* Enable MPLL. */
    CLK_MpllCmd(Enable);

    /* Wait MPLL ready. */
    while(Set != CLK_GetFlagStatus(ClkFlagMPLLRdy))
    {
        ;
    }

    /* Set system clock source. */
    CLK_SetSysClkSource(CLKSysSrcMPLL);

    return 0;
}

uint8_t func_Board_Power_And_Control_GPIO_Init(void)
{
	stc_port_init_t stcPortInit;

	MEM_ZERO_STRUCT(stcPortInit);

	stcPortInit.enPinMode = Pin_Mode_Out;

    //PORT_Init(OLED_IIC_SCL_PORT, OLED_IIC_SCL_PIN, &stcPortInit);
    //PORT_SetBits(OLED_IIC_SCL_PORT, OLED_IIC_SCL_PIN);
    //PORT_ResetBits(OLED_IIC_SCL_PORT, OLED_IIC_SCL_PIN);

    #ifndef HW_VERSION_V1_1
	PORT_Init(MAINPWR3V8_GPIO_PORT, MAINPWR3V8_GPIO_PIN, &stcPortInit);
    #endif
	PORT_Init(SENSOR_PWRVO1_GPIO_PORT, SENSOR_PWRVO1_GPIO_PIN, &stcPortInit);
	PORT_Init(SENSOR_PWRVO2_GPIO_PORT, SENSOR_PWRVO2_GPIO_PIN, &stcPortInit);
    #ifdef HW_VERSION_V1_1
    PORT_Init(SENSOR_PWRRS1_GPIO_PORT, SENSOR_PWRRS1_GPIO_PIN, &stcPortInit);
	PORT_Init(SENSOR_PWRRS2_GPIO_PORT, SENSOR_PWRRS2_GPIO_PIN, &stcPortInit);
    #endif
	PORT_Init(PWRUP_CTLUP1_GPIO_PORT, PWRUP_CTLUP1_GPIO_PIN, &stcPortInit);
	PORT_Init(PWRUP_CTLUP2_GPIO_PORT, PWRUP_CTLUP2_GPIO_PIN, &stcPortInit);
	PORT_Init(PWRSNR_GPIO_PORT, PWRSNR_GPIO_PIN, &stcPortInit);

    //蓝牙电源控制引脚
    PORT_Init(PWRBLE_GPIO_PORT, PWRBLE_GPIO_PIN, &stcPortInit);
    //DCE电源控制引脚
    PORT_Init(PWRDCE_GPIO_PORT, PWRDCE_GPIO_PIN, &stcPortInit);
    //LORA电源控制引脚
    PORT_Init(PWRLORA_GPIO_PORT, PWRLORA_GPIO_PIN, &stcPortInit);
    //北斗电源控制引脚
    PORT_Init(PWRBD_GPIO_PORT, PWRBD_GPIO_PIN, &stcPortInit);
    //BD 复位引脚
    PORT_Init(BDRST_GPIO_PORT, BDRST_GPIO_PIN, &stcPortInit);
    //OLED电源控制引脚
    PORT_Init(PWRLCM_GPIO_PORT, PWRLCM_GPIO_PIN, &stcPortInit);
    //RF电源控制引脚
    PORT_Init(PWRRF_GPIO_PORT, PWRRF_GPIO_PIN, &stcPortInit);
    //电池电量检测电源控制引脚
    PORT_Init(BATTERY_PWRCHK_GPIO_PORT, BATTERY_PWRCHK_GPIO_PIN, &stcPortInit);

    #ifndef HW_VERSION_V1_1
	MAINPWR3V8_PIN_CLOSE();	//关闭3.8V电源
	Ddl_Delay1ms(100);
	MAINPWR3V8_PIN_OPEN();	//打开3.8V电源
    #endif

	PWRSNR_PIN_CLOSE();	//关闭传感器电源
	//Ddl_Delay1ms(100);
	//PWRSNR_PIN_OPEN();	//打开传感器电源

    #if 1
    SENSOR_PWRVO1_PIN_CLOSE();    //关闭传感器电源
    PWRUP_CTLUP1_PIN_CLOSE();    //关闭传感器电源
    SENSOR_PWRVO2_PIN_CLOSE();    //关闭传感器电源
    PWRUP_CTLUP2_PIN_CLOSE();    //关闭传感器电源
    #ifdef HW_VERSION_V1_1
    SENSOR_PWRRS1_PIN_CLOSE();	//关闭传感器电源
    SENSOR_PWRRS2_PIN_CLOSE();	//关闭传感器电源
    #endif
    PWRBLE_PIN_CLOSE();	//关闭蓝牙模块电源
    PWRLORA_PIN_CLOSE();	//关闭LoRa模块电源
    PWRBD_PIN_CLOSE();	//关闭北斗模块电源
    BATTERY_PWRCHK_CLOSE();	//关闭电池电量检测电源
    PWRLCM_PIN_CLOSE();	//关闭OLED电源
    PWRRF_PIN_CLOSE();	//关闭RF模块电源
    #endif

    PWRSNR_PIN_CLOSE();	//关闭传感器电源
	Ddl_Delay1ms(100);
	PWRSNR_PIN_OPEN();	//打开传感器电源
	return 0;
}

void func_Power_Test()
{
    stc_port_init_t stcPortInit;

	MEM_ZERO_STRUCT(stcPortInit);

	stcPortInit.enPinMode = Pin_Mode_Out;

    #ifndef HW_VERSION_V1_1
    PORT_Init(MAINPWR3V8_GPIO_PORT, MAINPWR3V8_GPIO_PIN, &stcPortInit);
    #endif
    PORT_Init(SENSOR_PWRVO1_GPIO_PORT, SENSOR_PWRVO1_GPIO_PIN, &stcPortInit);
	PORT_Init(SENSOR_PWRVO2_GPIO_PORT, SENSOR_PWRVO2_GPIO_PIN, &stcPortInit);
    PORT_Init(PWRBLE_GPIO_PORT, PWRBLE_GPIO_PIN, &stcPortInit);
    PORT_Init(PWRSNR_GPIO_PORT, PWRSNR_GPIO_PIN, &stcPortInit);
    PORT_Init(PWRDCE_GPIO_PORT, PWRDCE_GPIO_PIN, &stcPortInit);
    PORT_Init(PWRLORA_GPIO_PORT, PWRLORA_GPIO_PIN, &stcPortInit);
    PORT_Init(PWRBD_GPIO_PORT, PWRBD_GPIO_PIN, &stcPortInit);
    PORT_Init(BATTERY_PWRCHK_GPIO_PORT, BATTERY_PWRCHK_GPIO_PIN, &stcPortInit);
    PORT_Init(PWRLCM_GPIO_PORT, PWRLCM_GPIO_PIN, &stcPortInit);
    PORT_Init(PWRRF_GPIO_PORT, PWRRF_GPIO_PIN, &stcPortInit);
    PORT_Init(PWRUP_CTLUP1_GPIO_PORT, PWRUP_CTLUP1_GPIO_PIN, &stcPortInit);
	PORT_Init(PWRUP_CTLUP2_GPIO_PORT, PWRUP_CTLUP2_GPIO_PIN, &stcPortInit);

    #ifndef HW_VERSION_V1_1
    MAINPWR3V8_PIN_CLOSE();	//关闭3.8V电源
	Ddl_Delay1ms(100);
	MAINPWR3V8_PIN_OPEN();	//打开3.8V电源
    Ddl_Delay1ms(100);
    #endif

    BATTERY_PWRCHK_CLOSE();	//关闭电池电量检测电源
    Ddl_Delay1ms(100);
    BATTERY_PWRCHK_OPEN();	//打开电池电量检测电源
    Ddl_Delay1ms(100);

    SENSOR_PWRVO1_PIN_CLOSE();    //关闭传感器电源
    Ddl_Delay1ms(100);
    SENSOR_PWRVO1_PIN_OPEN();    //打开传感器电源
    Ddl_Delay1ms(100);

    PWRUP_CTLUP1_PIN_CLOSE();    //关闭传感器电源
    Ddl_Delay1ms(100);
    PWRUP_CTLUP1_PIN_OPEN();    //打开传感器电源
    Ddl_Delay1ms(100);

    SENSOR_PWRVO2_PIN_CLOSE();    //关闭传感器电源
    Ddl_Delay1ms(100);
    SENSOR_PWRVO2_PIN_OPEN();    //打开传感器电源
    Ddl_Delay1ms(100);

    PWRUP_CTLUP2_PIN_CLOSE();    //关闭传感器电源
    Ddl_Delay1ms(100);
    PWRUP_CTLUP2_PIN_OPEN();    //打开传感器电源
    Ddl_Delay1ms(100);

    PWRBLE_PIN_CLOSE();	//关闭蓝牙模块电源
    Ddl_Delay1ms(100);
    PWRBLE_PIN_OPEN();	//打开蓝牙模块电源
    Ddl_Delay1ms(100);

    PWRSNR_PIN_CLOSE();	//关闭传感器电源
    Ddl_Delay1ms(100);
    PWRSNR_PIN_OPEN();	//打开传感器电源
    Ddl_Delay1ms(100);

    PWRDCE_PIN_CLOSE();	//关闭4G模块电源
    Ddl_Delay1ms(100);
    PWRDCE_PIN_OPEN();	//打开4G模块电源
    Ddl_Delay1ms(100);

    PWRLORA_PIN_CLOSE();	//关闭LoRa模块电源
    Ddl_Delay1ms(100);
    PWRLORA_PIN_OPEN();	//打开LoRa模块电源
    Ddl_Delay1ms(100);

    PWRBD_PIN_CLOSE();	//关闭北斗模块电源
    Ddl_Delay1ms(100);
    PWRBD_PIN_OPEN();	//打开北斗模块电源
    Ddl_Delay1ms(100);

    
    
    PWRLCM_PIN_CLOSE();	//关闭OLED电源
    Ddl_Delay1ms(100);
    PWRLCM_PIN_OPEN();	//打开OLED电源
    Ddl_Delay1ms(100);

    PWRRF_PIN_CLOSE();	//关闭RF模块电源
    Ddl_Delay1ms(100);
    PWRRF_PIN_OPEN();	//打开RF模块电源
    Ddl_Delay1ms(100);

    #ifndef HW_VERSION_V1_1
    MAINPWR3V8_PIN_CLOSE();	//关闭3.8V电源
    #endif
    SENSOR_PWRVO1_PIN_CLOSE();    //关闭传感器电源
    PWRUP_CTLUP1_PIN_CLOSE();    //关闭传感器电源
    SENSOR_PWRVO2_PIN_CLOSE();    //关闭传感器电源
    PWRUP_CTLUP2_PIN_CLOSE();    //关闭传感器电源
    PWRBLE_PIN_CLOSE();	//关闭蓝牙模块电源
    PWRSNR_PIN_CLOSE();	//关闭传感器电源
    PWRDCE_PIN_CLOSE();	//关闭4G模块电源
    PWRLORA_PIN_CLOSE();	//关闭LoRa模块电源
    PWRBD_PIN_CLOSE();	//关闭北斗模块电源
    BATTERY_PWRCHK_CLOSE();	//关闭电池电量检测电源
    PWRLCM_PIN_CLOSE();	//关闭OLED电源
    PWRRF_PIN_CLOSE();	//关闭RF模块电源
}


void func_Close_AllPower(void)
{
    #ifndef HW_VERSION_V1_1
    MAINPWR3V8_PIN_CLOSE();	//关闭3.8V电源
    #endif
    SENSOR_PWRVO1_PIN_CLOSE();    //关闭传感器电源
    PWRUP_CTLUP1_PIN_CLOSE();    //关闭传感器电源
    SENSOR_PWRVO2_PIN_CLOSE();    //关闭传感器电源
    PWRUP_CTLUP2_PIN_CLOSE();    //关闭传感器电源
    #ifdef HW_VERSION_V1_1
    SENSOR_PWRRS1_PIN_CLOSE();	//关闭传感器电源
    SENSOR_PWRRS2_PIN_CLOSE();	//关闭传感器电源
    #endif
    PWRSNR_PIN_CLOSE();	//关闭传感器电源
    PWRBLE_PIN_CLOSE();	//关闭蓝牙模块电源
    PWRLORA_PIN_CLOSE();	//关闭LoRa模块电源
    PWRBD_PIN_CLOSE();	//关闭北斗模块电源
    BATTERY_PWRCHK_CLOSE();	//关闭电池电量检测电源
    PWRLCM_PIN_CLOSE();	//关闭OLED电源
    PWRRF_PIN_CLOSE();	//关闭RF模块电源
}



/**
 *******************************************************************************
 ** \brief  Main function of project
 **
 ** \param  None
 **
 ** \retval int32_t return value, if needed
 ** 
 ******************************************************************************/
int32_t main(void)
{
    uint8_t u8Result = 0;
    uint8_t i = 0;
    uint8_t j = 0;
    //static uint8_t uc4GInitFlag = 0;
    static uint8_t ucMonitorFlag = 0;
    //DevMeasRecordDataSt st_TempValue;
    
    /* Initialize Clock */
    drv_mcu_Clock_Init();
    // ucValueArr[10] = {0};

    #if 0
    func_Power_Test();
    while(1)
    {
        //轮询喂狗
        PORT_SetBits(WATCHDOG_DONE_PORT,WATCHDOG_DONE_PIN);
        Ddl_Delay1ms(500);
        PORT_ResetBits(WATCHDOG_DONE_PORT,WATCHDOG_DONE_PIN);
        Ddl_Delay1ms(500);
    }
    #endif

    func_Board_Power_And_Control_GPIO_Init();

    //看门狗初始化
    drv_WatchDog_Init();

    func_SystemPara_Init();

    pst_MainSystemPara = GetSystemPara();
    func_WatchDog_Refresh();
    //SPI初始化
    drv_mcu_SPI_Init(); 
    drv_Storage_W25Q128_Init();
    func_Device_Parameter_Init();
    //OLED初始化
    drv_OLED_Init();
    //u8Result = 6;
    func_display_PowerOn_Menu();
    //OLED_Test(0);
    func_WatchDog_Refresh();
    #if 0

    Ddl_Delay1ms(1000);
    OLED_Test(u8Result);
    #endif
    /* Initialize LED */
    //BSP_LED_Init();
    
    /* Initialize Timer0 */
    drv_mcu_Timer_Init();
    drv_mcu_RTC_Config();
    drv_ModbusRTU_Init();   
    //EC200U_4G_Module_GPIO_Init();
    drv_mcu_USART_Init();
    pst_MainSystemPara->UsartData.enUsart3Source = MODULE_NFC_RFID;
    //drv_mcu_ChangeUSART3_Source(MODULE_NFC_RFID);
    #ifdef HW_VERSION_V1_1
    u8Result = func_RXNTTL518_Init();
    if(u8Result == 1)
    {
        pst_MainSystemPara->DeviceRunPara.usDevStatus |= 0x02;
    }
    #else
    u8Result = drv_RF_NFC_Module_Init();
    if(u8Result == 1)
    {
        pst_MainSystemPara->DeviceRunPara.usDevStatus |= 0x02;
    }
    #endif
    //蓝牙模块初始化
    drv_BT05_Module_Init();
    func_WatchDog_Refresh();
    func_MQTT_SK_Init(); //MQTT_SK 初始化
    func_Mainloop_Init(); //主循环初始化
    drv_Lora_TP1109_Init();
    //Ddl_Delay1ms(1000);
    //func_BT05_PowerDown_DeInit();
    //Ddl_Delay1ms(1000);
    
    //测试数据
    #ifdef JOE_TEST
    pst_MainSystemPara->DevicePara.nDeviceUploadCnt = 2;
    pst_MainSystemPara->DevicePara.nDeviceSampleGapCnt = 1;
    pst_MainSystemPara->DevicePara.nDeviceSaveRecordCnt = 1;
    pst_MainSystemPara->DevicePara.cMeasSensorCount = 2;
    pst_MainSystemPara->DevicePara.eMeasSensor[0] = Meas_BY_Integrated_Conductivity;
    pst_MainSystemPara->DevicePara.eMeasSensor[1] = Meas_BY_Radar_Level;
    //pst_MainSystemPara->DevicePara.fTotal_Volume = 1.234;
    //func_Save_Device_Parameter(DEV_TOTAL_VOLUME, (unsigned char*)&pst_MainSystemPara->DevicePara.fTotal_Volume);
    #if 0
    //验证设备测量数据记录功能
    func_Get_Device_MeasData_Record(0, &st_TempValue);
    gSt_DevMeasRecordData.cPhotosensitive_XYC_ALS_Status = 0;
    gSt_DevMeasRecordData.fWaterLevel_Radar = 0.1234;
    gSt_DevMeasRecordData.fWaterLevel_Pres = 0.5678;
    gSt_DevMeasRecordData.cWater_Immersion_Status = 1;
    gSt_DevMeasRecordData.fWaterQuality_COD = 0.9;
    gSt_DevMeasRecordData.fWaterQuality_COND = 1.2;
    gSt_DevMeasRecordData.fWaterVolume_s = 0.3456;
    gSt_DevMeasRecordData.fWaterVolume_Total = 0.7890;
    func_Save_Device_MeasData();
    func_Get_Device_MeasData_Record(0, &st_TempValue);
    #endif
    //验证断点续传
    //pst_MainSystemPara->DeviceRunPara.ulUploadRecordLostCnt = 6;
    //pst_MainSystemPara->DeviceRunPara.ulUploadRecordStartTime = 1749783132;
    #endif
    //验证设备状态上报
    //pst_MainSystemPara->DeviceRunPara.cDeviceStatusUploadFlag = 1 ;

    //OLED_Test(1);
    pst_MainSystemPara->DevicePara.cMonitorMode = 1;
    pst_MainSystemPara->DevicePara.cDeviceIdenFlag = 1;
    //pst_MainSystemPara->DevicePara.cDeviceRegisterFlag = 0;
    //pst_MainSystemPara->DevicePara.cMeasSensorEnableFlag[0] = 1;
    pst_MainSystemPara->DevicePara.cMeasSensorEnableFlag[1] = 1;
    //pst_MainSystemPara->DevicePara.cMeasSensorCount[0] = 1;
    pst_MainSystemPara->DevicePara.cMeasSensorCount[1] = 1;
    if(pst_MainSystemPara->DevicePara.cMonitorMode == 0)
    {
        ucMonitorFlag = 1;
    }
    pst_MainSystemPara->DevicePara.eMeasSensor[1][0] = Meas_BY_Integrated_Conductivity;
    //pst_MainSystemPara->DevicePara.eMeasSensor[0][0] = Meas_BY_Radar_Level;
    //pst_MainSystemPara->DevicePara.nDeviceUploadCnt = 10;
    //pst_MainSystemPara->DevicePara.nDeviceSampleGapCnt = 5;
    //pst_MainSystemPara->DevicePara.nDeviceSaveRecordCnt = 5;
    
    #ifndef JOE_TEST
    if((pst_MainSystemPara->DevicePara.cDeviceIdenFlag == 1) && (pst_MainSystemPara->DeviceRunPara.c4GInitFlag == 0))
    {
        u8Result = drv_EC200U_4G_Module_Init(0);
        if(u8Result != 0)
        {
            pst_MainSystemPara->DeviceRunPara.usDevStatus |= 0x01;
        }
        else
        {
            pst_MainSystemPara->DeviceRunPara.enUploadStatus = Status_OK;
        }
        pst_MainSystemPara->DeviceRunPara.c4GInitFlag = 1;
    }
    
    //OLED_Test(2);
    #else 
    drv_mcu_Set_RTC_Time("\r\n+QLTS: \"2025/05/12,15:15:03+32.0\"\r\n\r\nOK\r\n");
    #endif
    //func_Measure_WaterLevel_View_Show();
    //NFC初始化及修改USART3波特率
    

    //清除BT最后设置名称命令返回的数据
    pst_MainSystemPara->UsartData.ucUsartxRecvDataFlag[1] = 0;
    pst_MainSystemPara->UsartData.usUsartxRecvDataLen[1] = 0;
    memset(pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[1],0,sizeof(pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[1]));
    //Ddl_Delay1ms(1000);
    //func_RF_NFC_PowerDown_DeInit(); //关闭RF模块电源
    //Ddl_Delay1ms(1000);
    
    //drv_LoRa_RA02_Init();
    //PWRLORA_PIN_CLOSE();	//关闭LoRa模块电源
    //读取FLASH中设备参数信息
    #if 0
    W25Q128_Get_ReadDataBytes(SYSTEM_RECORD_START_ADDR,(uint8_t*)&u8Result,1);
    u8Result = 6;
    W25Q128_Spi_flash_buffer_write(&u8Result,SYSTEM_RECORD_START_ADDR,1);
    W25Q128_Get_ReadDataBytes(SYSTEM_RECORD_START_ADDR,(uint8_t*)&u8Result,1);

    W25Q128_Get_ReadDataBytes(SYSTEM_PARA_ADDR,(uint8_t*)&u8Result,1);
    u8Result = 10;
    W25Q128_Spi_flash_buffer_write(&u8Result,SYSTEM_PARA_ADDR,1);
    W25Q128_Get_ReadDataBytes(SYSTEM_PARA_ADDR,(uint8_t*)&u8Result,1);
    #endif
    
    //W25Q128_Get_ReadDataBytes(SYSTEM_PARA_ADDR,(uint8_t*)&pst_MainSystemPara->DevicePara.cDeviceID[0],sizeof(SysDeviceParaSt));
    //pst_MainSystemPara->DevicePara.cDeviceIdenFlag = 1;
    //W25Q128_Spi_flash_buffer_write(&pst_MainSystemPara->DevicePara.cDeviceIdenFlag,SYSTEM_PARA_ADDR+(&pst_MainSystemPara->DevicePara.cDeviceIdenFlag-&pst_MainSystemPara->DevicePara.cDeviceID[0]),sizeof(pst_MainSystemPara->DevicePara.cDeviceIdenFlag));
    //W25Q128_Get_ReadDataBytes(SYSTEM_PARA_ADDR,(uint8_t*)&pst_MainSystemPara->DevicePara.cDeviceID[0],sizeof(SysDeviceParaSt));
    
    u8Result = drv_Attitude_Sensor_SC7A20H_Init();
    if(u8Result == 1)
    {
        pst_MainSystemPara->DeviceRunPara.usDevStatus |= 0x04;
    }
    //OLED_Test(3);
    //Ddl_Delay1ms(100);
    //开机先读取设备姿态传感器数据，作为判断标准
    //func_SC7A20H_Read_FIFO_Buf(&pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A[0],&pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A[1],&pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A[2]);
    pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A = SC7A20_Task();

    //ADC 初始化，涉及水浸传感器及电池电量检测
    drv_mcu_ADC_Init();

    //读取当前电池电量
    pst_MainSystemPara->DeviceRunPara.esDeviceRunState.fBattleVoltage = drv_Get_Battery_Level_Value(&pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent);
    drv_Get_Water_Immersion_Sensor_Status(&pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.cWater_Immersion_Status, &pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.fWater_Immersion_Level);
    //磁控外部中断初始化
    drv_Magnetic_Bar_Init();
    
    //加密芯片初始化
    drv_LKT4202_Init();
    //drv_LKT4202_Random_Test();

    uint8_t ucTestKey[16] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10};
    drv_LKT4202_Send_EncryKEY(ucTestKey);
    drv_LKT4202_Send_DecryKEY(ucTestKey);

    //uint8_t ucTestData[16] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10};
    uint8_t cTestDataArr[16] = {0};
    uint8_t ucEncryData[16] = {0};
    uint8_t ucDecryData[16] = {0};
    sprintf((char*)&cTestDataArr[0],"%.3f",1.234); //将浮点数转换为字符串
    sprintf((char*)&cTestDataArr[7],"%.3f",1316.943); //将浮点数转换为字符串
    
    drv_LKT4202_SendData_Encry(cTestDataArr, (char*)ucEncryData);
    drv_LKT4202_SendData_Decry(ucEncryData, (char*)ucDecryData);
    float fValue1 = 0.0;
    float fValue2 = 0.0;
    sscanf((char*)&ucDecryData[0], "%f", &fValue1);
    sscanf((char*)&ucDecryData[7], "%f", &fValue2);
    //OLED_Test(4);
    //光照感应初始化
    drv_Photosensitive_XYC_ALS_Init();
    
    //Ddl_Delay1ms(100);
    //pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.sPhotosensitive_XYC_ALS_Data = func_ReadPhoto_XYC_ALS_Data();
    
    #if 1
    //北斗模块初始化
    if(pst_MainSystemPara->DevicePara.cDeviceIdenFlag == 1)
    {
        func_BD_PowerUp_Init();
        drv_mcu_ChangeUSART3_Source(MODULE_BD);
        //drv_LC86L_BD_Init();
    }
    
    //OLED_Test(5);
    //func_BD_Test();
    #endif
    func_RF_NFC_PowerDown_DeInit(); //关闭RF模块电源
    func_LowPower_Stop_Mode_Init();
    pst_MainSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_SLEEP;
    //Ddl_Delay1ms(10000);
    //func_BD_PownDown_Deinit();
    //Ddl_Delay1ms(10000);
    //func_BD_PowerUp_Init();
	//drv_mcu_ChangeUSART3_Source(MODULE_BD);
    //Ddl_Delay1ms(5000);
    //pst_MainSystemPara->DevicePara.cDeviceIdenFlag = 1;
    //OLED_Test(6);
    //u8Result = 6;
    #if 0
    func_Enter_LowPower_Stop_Mode();
    //ADC 初始化，涉及水浸传感器及电池电量检测
    drv_mcu_ADC_Init();
    //必须在下方开启PWRCTLUP1和CTLUP2之前采集电量电压，否则会导致电压变低
    //读取当前电池电量
    pst_MainSystemPara->DeviceRunPara.esDeviceRunState.fBattleVoltage = drv_Get_Battery_Level_Value(&pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent);
    
    //VSNR,用于控制姿态传感器及水浸传感器电源
    PWRSNR_PIN_CLOSE();	//关闭传感器电源
    Ddl_Delay1ms(100);
    PWRSNR_PIN_OPEN();	//打开传感器电源
    //外接485电源
    SENSOR_PWRVO1_PIN_OPEN();
    SENSOR_PWRVO2_PIN_OPEN();
    
    #if 0
    PWRUP_CTLUP1_PIN_OPEN();
    PWRUP_CTLUP2_PIN_OPEN();
    #endif
    //光照
    drv_Photosensitive_XYC_ALS_Init();
    drv_Attitude_Sensor_SC7A20H_Init();
    Ddl_Delay1ms(100);
    //drv_Photosensitive_XYC_ALS_Init();

    pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.sPhotosensitive_XYC_ALS_Data = func_ReadPhoto_XYC_ALS_Data();
    pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A = SC7A20_Task();
    drv_Get_Water_Immersion_Sensor_Status(&pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.cWater_Immersion_Status, &pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.fWater_Immersion_Level);
    #endif
    
    //判断当前设备是否为黑光图像站设备
    for(j=0; j<2; j++)
    {
        for(i=0; i<pst_MainSystemPara->DevicePara.cMeasSensorCount[j]; i++)
        {
            if(pst_MainSystemPara->DevicePara.eMeasSensor[j][i] == Meas_BY_BlackLight)
            {
                pst_MainSystemPara->DeviceRunPara.cBlackLightFlag = 1;
                break;
            }
        }
    }
    
    //测试外接传感器通讯
    #ifdef JOE_TEST
    BATTERY_PWRCHK_OPEN();
    func_Meas_Sensor_PowerOn_Init();
    //Ddl_Delay1ms(5000);
    //func_WatchDog_Refresh();
    Ddl_Delay1ms(2000);
    func_WatchDog_Refresh();

    //char cTestArr[4] = {0x00,0x00,0xA0,0x40};
    //char cNewArr[4] = {0};
    //RegData_HL_Swap_func(cNewArr,cTestArr, 4,1,4);
    //float fValue = *(float*)cNewArr;
    //fValue = *(float*)cTestArr;
    
    while(1)
    {
        //轮询喂狗
        func_WatchDog_Refresh();
        if(pst_MainSystemPara->DeviceRunPara.cBlackLightFlag == 0)
        {
            func_Meas_Sensor_Dispose();
        }
        else
        {
            func_BlackLight_Sensor_Dispose();
        }
        Ddl_Delay1ms(5000);
    }
    #endif

    //DevMeasRecordDataSt st_TempValue;
    //for(i=0; i<pst_MainSystemPara->DevicePara.nDeviceRecordCnt; i++)
    //{
    //    func_Get_Device_MeasData_Record(i, &st_TempValue);
    //}
    #if 0
    //func_Power_Test();
    pst_MainSystemPara->DeviceRunPara.esDeviceRunState.fDevLoca_lat = 130.256389;
    pst_MainSystemPara->DeviceRunPara.esDeviceRunState.fDevLoca_lng = 26.458239;
    pst_MainSystemPara->DeviceRunPara.esDeviceRunState.fBattleVoltage = 80.564;
    pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A = 20;
    pst_MainSystemPara->DeviceRunPara.usDevStatus = 0x00FF;
    pst_MainSystemPara->DevicePara.fTotal_Volume = 2356.1235;
    pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.cPhotosensitive_XYC_ALS_Status = 1;
    gSt_DevMeasRecordData.fWaterLevel_Radar = 12.5638;
    gSt_DevMeasRecordData.fWaterLevel_Pres = 15.7890;
    gSt_DevMeasRecordData.fWaterQuality_COD = 2.1234;
    gSt_DevMeasRecordData.fWaterQuality_COND = 121.5678;
    gSt_DevMeasRecordData.fWaterVolume_s = 23.3456;
    gSt_DevMeasRecordData.fWaterVolume_Total = 0.7890;
    
    while(1)
    {
        //func_BT_Dispose();
        //轮询喂狗
        func_WatchDog_Refresh();
        Ddl_Delay1ms(500);
    }
    #endif

    #if 0
    func_RXNTTL518_Init();;
    while(1)
    {
        func_NFC_Check_Dispose();
    }
    #endif
    pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent = 103.0;
    pst_MainSystemPara->DeviceRunPara.cDevUploadStatusTime = 24;
    
    //pst_MainSystemPara->DeviceRunPara.nDeviceCurSampleCount = 2;
    while(1)
    {
        //1Min喂狗
        if(pst_MainSystemPara->DeviceRunPara.cRTC_1MIN_ReflashFlag == 1)
        {
            pst_MainSystemPara->DeviceRunPara.cRTC_1MIN_ReflashFlag = 0;
            //轮询喂狗
            func_WatchDog_Refresh();
        }

        if((pst_MainSystemPara->DevicePara.cDeviceIdenFlag == 1) && (pst_MainSystemPara->DeviceRunPara.c4GInitFlag == 0))
        {
            u8Result = drv_EC200U_4G_Module_Init(0);
            if(u8Result != 0)
            {
                pst_MainSystemPara->DeviceRunPara.usDevStatus |= 0x01;
            }
            else
            {
                pst_MainSystemPara->DeviceRunPara.enUploadStatus = Status_OK;
            }
            pst_MainSystemPara->DeviceRunPara.c4GInitFlag = 1;
            if(pst_MainSystemPara->DeviceRunPara.ucOLEDInitFlag == 1)
            {
                pst_MainSystemPara->DeviceRunPara.ucOLEDInitFlag = 0;
                func_OLED_PowerDown_DeInit(); //关闭OLED电源
            }
        }

        #ifdef NEW_MAINLOOP
        if(ucMonitorFlag == 0)
        {
            ucMonitorFlag = 1;
            func_Enter_LowPower_Stop_Mode();
        }
        else
        {
            func_System_Mainloop_Dispose();
        }
        
        #else
        //磁棒状态检测
        if(pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.cMagnetic_Bar_Status == 1)
        {
            //开启OLED电源及初始化配置，要注意，每次接触事件，仅初始化一次
            if(guc_NFCPWRInitFlag == 0)
            {
                guc_NFCPWRInitFlag = 1;
                if(pst_MainSystemPara->DevicePara.cDeviceIdenFlag == 0) //当前设备未经过验证
                {
                    func_RF_NFC_PowerUp_Init();
                }
            }
            
            if(pst_MainSystemPara->DevicePara.cDeviceIdenFlag == 0) //当前设备未经过验证
            {
                //func_Not_Certified_View_Show();
                //磁棒激活状态下，检测是否有卡片
                ucNFCFindCardFlag = func_FindCar();
                if((ucNFCFindCardFlag == 1))   //找到卡片
                {
                    //验证卡是否在白名单中
                    for(j=0; j<20; j++)
                    {
                        if((strncmp((char*)picc_uid, (char*)guc_NFC_ID[j], 5) == 0) && (picc_uid[0] != 0x00))
                        {
                            //找到卡片
                            if(guc_NFC_Card_Flag == 0)
                            {
                                guc_NFC_Card_Flag = 1;
                                pst_MainSystemPara->DevicePara.cDeviceIdenFlag = 1;
                                //W25Q128_Spi_flash_buffer_write(&pst_MainSystemPara->DevicePara.cDeviceIdenFlag,SYSTEM_PARA_ADDR+(&pst_MainSystemPara->DevicePara.cDeviceIdenFlag-&pst_MainSystemPara->DevicePara.cDeviceID[0]),1);
                                func_Save_Device_Parameter(DEV_IDEN_FLAG, (unsigned char*)&pst_MainSystemPara->DevicePara.cDeviceIdenFlag);
                            }
                            break;
                        }
                        else
                        {
                            guc_NFC_Card_Flag = 0;
                        }
                    }
                }
                else
                {
                    guc_NFC_Card_Flag = 0;
                }
            }
        }

        if(pst_MainSystemPara->DeviceRunPara.eCurPowerType == Power_ON)	//开机状态下
        {
            //每次系統從低功耗模式下喚醒時，需配置相關模塊電源及初始化操作
            if(pst_MainSystemPara->DeviceRunPara.enDeviceRunMode != DEVICE_RUN_STATE_SLEEP)
            {
                if(guc_SystemPowerInitFlag == 0)
                { 
                    guc_SystemPowerInitFlag = 1;
                    //目前僅考慮在模塊喚醒狀態下才可以進行藍牙通訊，後續可考虑蓝牙电源不关闭，一直支持通讯操作
                    func_BT05_PowerUp_Init();
                    //Lora模块电源控制
                    //func_LoRa_RA02_PowerUp_Init();
                    //ADC 初始化，涉及水浸传感器及电池电量检测
                    drv_mcu_ADC_Init();
                    //必须在下方开启PWRCTLUP1和CTLUP2之前采集电量电压，否则会导致电压变低
                    
                    //光照
                    //VLT_PIN_OPEN();	//VLT引脚拉高，给XYC_ALS供电
                    drv_Photosensitive_XYC_ALS_Init(); 
                    //VSNR,用于控制姿态传感器及水浸传感器电源
                    PWRSNR_PIN_CLOSE();	//关闭传感器电源
                    Ddl_Delay1ms(100);
                    PWRSNR_PIN_OPEN();	//打开传感器电源
                    
                    drv_Attitude_Sensor_SC7A20H_Init();
                    //当前采用间隔计数值达到设置的采样间隔，开启外接传感器电源
                    if((pst_MainSystemPara->DeviceRunPara.nDeviceCurSampleCount >= pst_MainSystemPara->DevicePara.nDeviceSampleGapCnt)
                    && (pst_MainSystemPara->DevicePara.cMeasSensorCount > 0))
                    {
                        pst_MainSystemPara->DeviceRunPara.nDeviceCurSampleCount = 0;
                        func_Meas_Sensor_PowerOn_Init();
                        pst_MainSystemPara->DeviceRunPara.cMeasDelayFlag = 1;
                        pst_MainSystemPara->DeviceRunPara.cMeasDelayCnt = 0;
                        if(pst_MainSystemPara->DeviceRunPara.cTimer4StartFlag == 0)
                        {
                            pst_MainSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_DIAG;
                            drv_mcu_Timer4_Start();
                        }
                    }
                    

                    //Ddl_Delay1ms(100);
                    
                    //测试，开启OLED电源及初始化配置，实际使用应在磁棒接触时才开启
                    //func_OLED_PowerUp_Init();
                    //OLED_Test(u8Result);
                }
            }
            
            if((pst_MainSystemPara->DevicePara.cDeviceIdenFlag == 1) && (pst_MainSystemPara->DeviceRunPara.enDeviceRunMode != DEVICE_RUN_STATE_SLEEP)) //当前设备经过验证
            {
                pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.cPhotosensitive_XYC_ALS_Status = 0;
                pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.sPhotosensitive_XYC_ALS_Data = func_ReadPhoto_XYC_ALS_Data();
                if(pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.cPhotosensitive_XYC_ALS_Status == 0)
                {
                    pst_MainSystemPara->DeviceRunPara.usDevStatus &= 0xFFEF;
                }
                else
                {
                    pst_MainSystemPara->DeviceRunPara.usDevStatus |= 0x0010;
                }
                
                //func_SC7A20H_Read_FIFO_Buf(&pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A[0],&pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A[1],&pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A[2]);
                pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A = SC7A20_Task();
                
                //读取水浸传感器状态
                drv_Get_Water_Immersion_Sensor_Status(&pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.cWater_Immersion_Status, &pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.fWater_Immersion_Level);
                if(pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.cWater_Immersion_Status == 0)
                {
                    pst_MainSystemPara->DeviceRunPara.usDevStatus &= 0xFFF7;
                }
                else
                {
                    pst_MainSystemPara->DeviceRunPara.usDevStatus |= 0x0008;
                }

                //读取当前电池电量
                pst_MainSystemPara->DeviceRunPara.esDeviceRunState.fBattleVoltage = drv_Get_Battery_Level_Value(&pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent);

                //外接485测量设备处理
                if(pst_MainSystemPara->DeviceRunPara.cMeasDelayFlag == 1)
                {
                    func_Meas_Sensor_Dispose();
                }

                //计时到保存记录时间
                if(pst_MainSystemPara->DeviceRunPara.nDeviceCurSaveRecordCount >= pst_MainSystemPara->DevicePara.nDeviceSaveRecordCnt)
                {
                    pst_MainSystemPara->DeviceRunPara.nDeviceCurSaveRecordCount = 0;
                    //将本次采集的数据存入存储中
                    #ifdef JOE_TEST
                    gSt_DevMeasRecordData.nAttitude_SC7A = pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A + pst_MainSystemPara->DeviceRunPara.nDeviceCurUploadRecordCount * 10;
                    #else
                    gSt_DevMeasRecordData.nAttitude_SC7A = pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A;
                    #endif
                    gSt_DevMeasRecordData.cWater_Immersion_Status = pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.cWater_Immersion_Status;
                    gSt_DevMeasRecordData.cPhotosensitive_XYC_ALS_Status = pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.cPhotosensitive_XYC_ALS_Status;
                    for(i=0; i<pst_MainSystemPara->DevicePara.cMeasSensorCount; i++)
                    {
                        if(pst_MainSystemPara->DevicePara.eMeasSensor[i] == Meas_Flowmeter)
                        {
                            gSt_DevMeasRecordData.fWaterVolume_s = pst_MainSystemPara->DeviceRunPara.esMeasData.fVolumeValue;
                            pst_MainSystemPara->DevicePara.fTotal_Volume += pst_MainSystemPara->DeviceRunPara.esMeasData.fVolumeValue;
                            func_Save_Device_Parameter(DEV_TOTAL_VOLUME, (unsigned char*)&pst_MainSystemPara->DevicePara.fTotal_Volume);
                            gSt_DevMeasRecordData.fWaterVolume_Total = pst_MainSystemPara->DevicePara.fTotal_Volume;
                        }
                        else if(pst_MainSystemPara->DevicePara.eMeasSensor[i] == Meas_BY_Pressure_Level)
                        {
                            gSt_DevMeasRecordData.fWaterLevel_Pres = pst_MainSystemPara->DeviceRunPara.esMeasData.fWaterLevel_Pressure;
                        }
                        else if(pst_MainSystemPara->DevicePara.eMeasSensor[i] == Meas_WaterLevel_Radar)
                        {
                            gSt_DevMeasRecordData.fWaterLevel_Radar = pst_MainSystemPara->DeviceRunPara.esMeasData.fWaterLevel_Radar;
                        }
                        else if(pst_MainSystemPara->DevicePara.eMeasSensor[i] == Meas_BY_Integrated_Conductivity)
                        {
                            gSt_DevMeasRecordData.fWaterQuality_COND = pst_MainSystemPara->DeviceRunPara.esMeasData.fWaterQuality_DDValue;
                        }
                        else if(pst_MainSystemPara->DevicePara.eMeasSensor[i] == Meas_HX_WaterQuality_COD)
                        {
                            gSt_DevMeasRecordData.fWaterQuality_COD = pst_MainSystemPara->DeviceRunPara.esMeasData.fWaterQuality_CODValue;
                        }
                    }
                    func_Save_Device_MeasData();
                    pst_MainSystemPara->DevicePara.nDeviceRecordCnt++;
                    func_Save_Device_Parameter(DEV_HIS_RECORD, (unsigned char*)&pst_MainSystemPara->DevicePara.nDeviceRecordCnt);
                }

                //蓝牙通讯处理
                if(pst_MainSystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_BT] == 1)  //蓝牙模块收到数据
                {
                    if(strstr(pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[1],"BLE_CONNECT") != NULL)
                    {
                        pst_MainSystemPara->DeviceRunPara.cDeviceBTConnectFlag = 1;
                    }
                    else if(strstr(pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[1],"BLE_DISCONNECT") != NULL)
                    {
                        pst_MainSystemPara->DeviceRunPara.cDeviceBTConnectFlag = 0;
                    }
                    else
                    {

                    }
                    if(pst_MainSystemPara->DeviceRunPara.cDeviceBTConnectFlag == 1)
                    {
                        //drv_BT05_SendData((uint8_t *)"OK",2);
                        pst_MainSystemPara->DeviceRunPara.usBTRecValue = (pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BT][0] - 0x30) * 100 \
                            + (pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BT][1] - 0x30) * 10 \
                            + (pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BT][2] - 0x30);

                        guc_TestValue1 = 1;
                        guc_TestValue2 = 0;
                        sprintf((char*)ucValueArr,"OK:%d",pst_MainSystemPara->DeviceRunPara.usBTRecValue+10);
                        
                    }
                    else
                    {
                        pst_MainSystemPara->DeviceRunPara.usBTRecValue = 0;
                    }
                    memset(pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BT], 0, USART_DATA_LEN_MAX);
                    pst_MainSystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_BT] = 0;
                }

                if(guc_TestValue1 == 1)
                {
                    drv_BT05_SendData(ucValueArr,strlen((char*)ucValueArr));
                    guc_TestValue2++;
                    if(guc_TestValue2 >= 10)
                    {
                        guc_TestValue2 = 0;
                        guc_TestValue1 = 0;
                    }
                }
                
                //func_BD_Test();
                
                
                #if 0
                if(pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ucDataValidFlag == 0)
                {
                    if(pst_MainSystemPara->UsartData.enUsart3Source == MODULE_NFC_RFID)
                    {
                        drv_mcu_ChangeUSART3_Source(MODULE_BD);
                    }
                    //北斗模块收到数据
                    if(pst_MainSystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_BD] == 1)
                    {
                        if(strstr(pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BD],"RMC") != NULL)
                        {
                            memcpy(guc_TextBDData, &pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BD][42], 20);
                            NMEA_GPRMC_Analysis((uint8_t *)pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BD]);
                        }   
                        pst_MainSystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_BD] = 0;
                        //memset(pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BD], 0, USART_DATA_LEN_MAX);
                        pst_MainSystemPara->UsartData.usUsartxRecvDataLen[MODULE_BD] = 0;
                        //正常程序应该是开机先进行卫星定位，直到定位成功后，才进入正常运行模式，可进入低功耗休眠，此时可以关闭北斗电源
                        if(pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ucDataValidFlag == 1)
                        {
                            func_BD_PownDown_Deinit();
                        }
                    }
                }
                #endif
                
                #if 1
                //定时上传4G数据到测试平台
                //if(guc_RTC_TimeOutFlag == 1)
                {
                    //guc_RTC_TimeOutFlag = 0;
                    //uc4GUploadCnt++;
                    //if(uc4GUploadCnt >= 1)
                    if(pst_MainSystemPara->DeviceRunPara.nDeviceCurUploadRecordCount >= pst_MainSystemPara->DevicePara.nDeviceUploadCnt)
                    {
                        pst_MainSystemPara->DeviceRunPara.nDeviceCurUploadRecordCount = 0;
                        //uc4GUploadCnt = 0;
                        
                        //pst_MainSystemPara->DeviceRunPara.enUploadStatus = Status_Upload;
                        //func_Upload_Sensor_Value_To_OneNet_Text();
                        pst_MainSystemPara->DeviceRunPara.enUploadStatus = Status_Register;
                        //重复尝试重新初始化4G模块
                        u8Result = drv_EC200U_4G_Module_Init(1);
                        if(u8Result != 0)
                        {
                            pst_MainSystemPara->DeviceRunPara.usDevStatus |= 0x0001;
                        }
                        else
                        {
                            pst_MainSystemPara->DeviceRunPara.usDevStatus &= 0xFFFE;
                            pst_MainSystemPara->DeviceRunPara.enUploadStatus = Status_OK;
                            pst_MainSystemPara->DeviceRunPara.cWaitServerCMDFlag = 1;
                            pst_MainSystemPara->DeviceRunPara.cWaitServerCMDCnt = 0;
                        }
                        //pst_MainSystemPara->DeviceRunPara.enUploadStatus = Status_OK;
                        if(pst_MainSystemPara->DeviceRunPara.cTimer4StartFlag == 0)
                        {
                            pst_MainSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_DIAG;
                            drv_mcu_Timer4_Start();
                        }
                        //pst_MainSystemPara->DeviceRunPara.c4GTimerCnt = 0;
                    }
                }
                #endif

                if((pst_MainSystemPara->DeviceRunPara.usDevStatus & 0X01) == 0X01)  //当前4G模块初始化异常
                {
                    pst_MainSystemPara->DeviceRunPara.enUploadStatus = Status_Register;
                    //重复尝试重新初始化4G模块
                    u8Result = drv_EC200U_4G_Module_Init(1);
                    if(u8Result != 0)
                    {
                        pst_MainSystemPara->DeviceRunPara.usDevStatus |= 0x0001;
                    }
                    else
                    {
                        pst_MainSystemPara->DeviceRunPara.usDevStatus &= 0xFFFE;
                        pst_MainSystemPara->DeviceRunPara.enUploadStatus = Status_OK;
                    }
                }

                if(pst_MainSystemPara->DeviceRunPara.cWaitServerCMDFlag == 1)
                {
                    if(pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB][30] != 0x00)  //4G/NB模块收到数据
                    {
                        //if(func_Array_Find_Str(pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],pst_MainSystemPara->DevicePara.cDeviceID) != NULL)
                        {
                            if(func_Array_Find_Str(pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],100,"setConfig",9) == 0)
                            {
                                MQTT_SK_Set_Config();
                            }
                            else if(func_Array_Find_Str(pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],100,"getConfig",9) == 0)
                            {
                                MQTT_SK_Get_Config();
                            }
                            else if(func_Array_Find_Str(pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],100,"getData",7) == 0)
                            {
                                MQTT_SK_Get_Data();
                            }
                            else if(func_Array_Find_Str(pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],100,"dataPt",6) == 0)
                            {
                                MQTT_SK_Data_Pass_Through();
                            }
                        }
                    }
                }
            }

            //Ddl_Delay1ms(800);
            if(pst_MainSystemPara->DeviceRunPara.enDeviceRunMode != DEVICE_RUN_STATE_DIAG)
            {
                //pst_MainSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_SLEEP;
                func_Enter_LowPower_Stop_Mode();
            }
        }
        else
        {
            if(pst_MainSystemPara->DeviceRunPara.enDeviceRunMode != DEVICE_RUN_STATE_DIAG)
            {
                //pst_MainSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_SLEEP;
                func_Enter_LowPower_Stop_Mode();
            }
        }
        #endif
    }
}

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
