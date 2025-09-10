/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\Mainloop\Mainloop.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-18       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "Mainloop.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "time.h"
#include "stddef.h"
#include "hc32f460_utility.h"
#include "hc32f460_pwc.h"
#include "drv_ADC.h"
#include "drv_USART.h"
#include "ModbusRTU.h"
#include "Magnetic_Bar.h"
#include "RF_NFC.h"
#include "4G_EC200U.h"
#include "drv_Timer.h"
#include "drv_SPI.h"
#include "drv_Storage_W25Q128.h"
#include "drv_SC7A20H.h"
#include "drv_ADC.h"
#include "LoRa_RA02.h"
#include "Photosensitive_XYC_ALS.h"
#include "OLED.h"
#include "Display.h"
#include "LC86L_BD.h"
#include "BT_TB05.h"
#include "drv_RTC.h"
#include "drv_LKT4202.h"
#include "RXNTTL518.h"
//#include "ModbusRTU.h"

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
static SystemPataSt *pst_MainloopSystemPara;
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

void func_Mainloop_Init(void)
{
	pst_MainloopSystemPara = GetSystemPara();
}


 //软件复位
void func_System_Soft_Reset(void)
{
	__set_FAULTMASK(1);
    NVIC_SystemReset();
}

//进入低功耗模式，关闭相关电源
void func_Enter_LowPower_Stop_Mode(void)
{
    uint32_t u32tmp1, u32tmp2;

	pst_MainloopSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_SLEEP;
    guc_SystemPowerInitFlag = 0;
	//关闭相关电源
	func_Meas_Sensor_PowerDown_DeInit();
	func_BT05_PowerDown_DeInit(); //关闭蓝牙模块电源
    func_ADC_DeInit();
    PWRSNR_PIN_CLOSE();	//关闭传感器电源
    PWRLORA_PIN_CLOSE();	//关闭LoRa模块电源
    //PWRBD_PIN_CLOSE();	//关闭北斗模块电源
    //BATTERY_PWRCHK_CLOSE();	//关闭电池电量检测电源
    func_OLED_PowerDown_DeInit(); //关闭OLED电源
    pst_MainloopSystemPara->DeviceRunPara.ucOLEDInitFlag = 0;
    guc_NFCPWRInitFlag = 0;
    func_RF_NFC_PowerDown_DeInit(); //关闭RF模块电源
    VLT_PIN_CLOSE();    //关闭光照传感器电源
    //DCERST_PIN_RESET;   //关闭4G电源
	gc_SystemPosi = 9;
    if(pst_MainloopSystemPara->DeviceRunPara.cWaitServerCMDFlag == 0)
    {
        func_EC200U_4G_PownDown_Deinit();   //关闭4G电源
		#ifndef HW_VERSION_V1_1
		if(pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ucDataValidFlag == 1)
		{
			MAINPWR3V8_PIN_CLOSE();	//关闭3.8V电源
		}
		#endif
    }
    gc_SystemPosi = 10;
    //Ddl_Delay1ms(100000);
    if(pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ucDataValidFlag == 1)
	{
		/* Ensure DMA disable */
		u32tmp1 = M4_DMA1->EN_f.EN;
		u32tmp2 = M4_DMA2->EN_f.EN;
		while((0ul != u32tmp1) && ((0ul != u32tmp2)))
		{
			;
		}
		gc_SystemPosi = 11;
		/* Ensure FLASH is ready */
		while(1ul != M4_EFM->FSR_f.RDY)
		{
			;
		}
		gc_SystemPosi = 12;
		/* Enter stop mode. */
		PWC_EnterStopMd();
	}
    else
	{
		PWC_EnterSleepMd();
	}
    
    //#endif
	gc_SystemPosi = 13;
}

/**
 * @brief  func_Get_Flash_Data
 * @param  nStartAddr: start address
 * @param  cDataArr: data array
 * @param  nLen: data length
 * @retval 0: operation success 1: operation failed
 * @note   read record data from flash
 */	
int func_Get_Flash_Data(char cDataType, int nStartAddr, int nLen, char *cDataArr)
{
	//read record data from flash
	return 0;
}


/**
 *******************************************************************************
 ** \brief  func_Set_Flash_Data
 ** \param  cDataType: data type
 ** \param  nStartAddr: data addr
 ** \param  nLen: data length
 ** \param  cDataArr: data array
 ** \retval 0: operation success 1: operation failed
 ** \note   write record data to flash 
 ******************************************************************************/
int func_Set_Flash_Data(char cDataType)
{
	pst_MainloopSystemPara = GetSystemPara();
	if(pst_MainloopSystemPara->DeviceUploadData.nDeviceUploadRecordCntMax == 0)
	{
		drv_mcu_Get_RTC_Time(pst_MainloopSystemPara->DeviceUploadData.cDeviceStartUploadDateTime);
	}
	
    if(cDataType == 0)	//定时保存时间间隔到，保存记录次数参数及此时测量数据
	{
		//写入数据
		//write record data to flash
		#pragma diag_suppress=Pa039
		W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_MainloopSystemPara->DevicePara.nDeviceRecordCnt, offsetof(SysDeviceParaSt,nDeviceRecordCnt),sizeof(pst_MainloopSystemPara->DevicePara.nDeviceRecordCnt));
		#pragma diag_warning=Pa039
	}
	else
	{
		//write record data to flash
	}
	
    return 0;
}

void func_LowPower_Stop_Mode_Init(void)
{
	stc_pwc_stop_mode_cfg_t stcPwcStopCfg;
    //uint32_t u32tmp1, u32tmp2;

    MEM_ZERO_STRUCT(stcPwcStopCfg);

	/* Config stop mode. */
    stcPwcStopCfg.enStpDrvAbi = StopHighspeed;
    stcPwcStopCfg.enStopClk = ClkFix;
    stcPwcStopCfg.enStopFlash = Wait;
    stcPwcStopCfg.enPll = Enable;

    while(Ok != PWC_StopModeCfg(&stcPwcStopCfg))
    {
        ;
    }

    /* Set wake up source EIRQ12, EIRQ13. */
    enIntWakeupEnable(Extint7WU);
    enIntWakeupEnable(RtcPeriodWU);
	enIntWakeupEnable(Usart1RxWU);

	#if 0
    /* Ensure DMA disable */
    u32tmp1 =  M4_DMA1->EN_f.EN;
    u32tmp2 =  M4_DMA2->EN_f.EN;
    while((0ul != u32tmp1) && ((0ul != u32tmp2)))
    {
        ;
    }
    /* Ensure FLASH is ready */
    while(1ul != M4_EFM->FSR_f.RDY)
    {
        ;
    }
	#endif
}

//设备外挂传感器电源上电操作
void func_Meas_Sensor_PowerOn_Init(void)
{
	uint8_t ucDelayFlag = 0;
	//外接485电源
	#if 1
	if(pst_MainloopSystemPara->DevicePara.cMeasSensorEnableFlag[0] == 1)
	{
		PWRUP_CTLUP1_PIN_OPEN();
		ucDelayFlag = 1;
	}
	if(pst_MainloopSystemPara->DevicePara.cMeasSensorEnableFlag[1] == 1)
	{
		PWRUP_CTLUP2_PIN_OPEN();
		ucDelayFlag = 1;
	}
	if(ucDelayFlag == 1)	//如果485电源打开了，则延时500ms
	{
		Ddl_Delay1ms(500);
	}
	#endif
	if(pst_MainloopSystemPara->DevicePara.cMeasSensorEnableFlag[0] == 1)
	{
		SENSOR_PWRVO1_PIN_OPEN();    //打开传感器电源
		#ifdef HW_VERSION_V1_1
		SENSOR_PWRRS1_PIN_OPEN();	//打开传感器电源
		#endif
	}
	if(pst_MainloopSystemPara->DevicePara.cMeasSensorEnableFlag[1] == 1)
	{
		SENSOR_PWRVO2_PIN_OPEN();    //打开传感器电源
		#ifdef HW_VERSION_V1_1
		SENSOR_PWRRS2_PIN_OPEN();	//打开传感器电源
		#endif
	}
}

//设备外挂传感器电源断电操作
void func_Meas_Sensor_PowerDown_DeInit(void)
{
	if(pst_MainloopSystemPara->DevicePara.cMeasSensorEnableFlag[0] == 1)
	{
		SENSOR_PWRVO1_PIN_CLOSE();    //关闭传感器电源
		PWRUP_CTLUP1_PIN_CLOSE();    //关闭传感器电源
		#ifdef HW_VERSION_V1_1
		SENSOR_PWRRS1_PIN_CLOSE();	//关闭传感器电源
		#endif
	}
	if(pst_MainloopSystemPara->DevicePara.cMeasSensorEnableFlag[1] == 1)
	{
		SENSOR_PWRVO2_PIN_CLOSE();    //关闭传感器电源
		PWRUP_CTLUP2_PIN_CLOSE();    //关闭传感器电源
		#ifdef HW_VERSION_V1_1
		SENSOR_PWRRS2_PIN_CLOSE();	//关闭传感器电源
		#endif
	}
	BATTERY_PWRCHK_CLOSE();
	PORT_ResetBits(USART4_TX_485_1_PORT, USART4_TX_485_1_PIN);
    PORT_ResetBits(USART4_RX_485_1_PORT, USART4_RX_485_1_PIN);
	PORT_ResetBits(USART4_TX_485_2_PORT, USART4_TX_485_2_PIN);
    PORT_ResetBits(USART4_RX_485_2_PORT, USART4_RX_485_2_PIN);
}

//定时器1s处理程序
void func_System_Timer1s_Dispose(void)
{
	unsigned char ucLCDShowDelayCnt = 0 ;	//LCD界面显示延时计数器
    static unsigned char ucNFCShowCnt = 0;	//NFC认证通过界面显示计数器
    static unsigned char ucNFCShowFlag = 0;	//NFC认证通过界面显示标志位：1->显示，0->不显示
    static char cBarEventFinishFlag = 0;	//磁棒操作要执行的事件完成标志位:1->完成，0->未完成
    static char cServerEventFinishFlag = 0;	//服务器通讯事件完成标志位:1->完成，0->未完成
    static char cMeasSensorEventFinishFlag = 0;	//测量传感器事件完成标志位:1->完成，0->未完成
	static char cBTEventFinishFlag = 0;	//蓝牙事件完成标志位:1->完成，0->未完成
	static char cBTWaitConnectEventFinishFlag = 0;	//蓝牙等待连接事件完成标志位:1->完成，0->未完成
	static unsigned char ucBarLostCnt = 0;

    if(pst_MainloopSystemPara->DeviceRunPara.eCurPowerType == Power_ON)	//开机状态下
	{
		ucLCDShowDelayCnt = 9;	//开机状态下，延时9秒
	}
	else	//关机状态下	
    {
        ucLCDShowDelayCnt = 3;
    }

	//磁棒状态检测
    if(pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.cMagnetic_Bar_Status == 1)
    {
		//当前定时器事件是磁棒触摸事件
        cBarEventFinishFlag = 0;
    }
    else
    {
        cBarEventFinishFlag = 1;
    }

	//判断当前定时器是否由设备定时上传数据到服务器后等待服务器通讯延时所产生
    if(pst_MainloopSystemPara->DeviceRunPara.cWaitServerCMDFlag == 1)
    {
        cServerEventFinishFlag = 0;
    }
    else
    {
        cServerEventFinishFlag = 1;
    }

	//判断当前定时器是否由测量传感器延时所产生
    if(pst_MainloopSystemPara->DeviceRunPara.cMeasDelayFlag == 1)
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
		pst_MainloopSystemPara->DeviceRunPara.cBarTouchCnt++;
		ucBarLostCnt = 0;
		pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.cMagnetic_Bar_Status = 1;

        if(pst_MainloopSystemPara->DeviceRunPara.eCurPowerType == Power_ON)	//开机状态下
        {
            //显示""关机中XS"
            func_Measure_WaterLevel_View_Show(pst_MainloopSystemPara->DeviceRunPara.cBarTouchCnt);
            if(pst_MainloopSystemPara->DeviceRunPara.cBarTouchCnt >= 5)
            {
                pst_MainloopSystemPara->DeviceRunPara.cBarTouchCnt = 0;
                cBarEventFinishFlag = 1;
				pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.cMagnetic_Bar_Status = 0;
                pst_MainloopSystemPara->DeviceRunPara.eCurPowerType = Power_OFF;
            }
        }
        else	//关机状态下
        {
            if(pst_MainloopSystemPara->DeviceRunPara.ucOLEDInitFlag == 0)
            {
                func_OLED_PowerUp_Init();
                pst_MainloopSystemPara->DeviceRunPara.ucOLEDInitFlag = 1;
            }
            func_PowerOn_View_Show(pst_MainloopSystemPara->DeviceRunPara.cBarTouchCnt);
            if(pst_MainloopSystemPara->DeviceRunPara.cBarTouchCnt >= 5)
            {
                func_System_Soft_Reset();
            }
        }
	}
	else	//磁棒断开接触状态
	{
		//判断之前磁棒是否为接触状态：磁棒唤醒事件
        if(pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.cMagnetic_Bar_Status == 1)
        {
            pst_MainloopSystemPara->DeviceRunPara.cBarTouchCnt = 0;
            ucBarLostCnt++;

            if(pst_MainloopSystemPara->DeviceRunPara.eCurPowerType == Power_ON)	//开机状态下
            {
                if((ucBarLostCnt % 2) == 0)	//2S触发
                {
                    if(guc_NFC_Card_Flag == 0)	//当前未进行NFC认证或NFC认证不通过
                    {
                        if(pst_MainloopSystemPara->DevicePara.cDeviceIdenFlag == 0) //当前设备未经过验证
                        {
                            func_Not_Certified_View_Show();
                        }
                        else	//设备经过NFC验证，显示正常测量数据界面
                        {
                            switch(pst_MainloopSystemPara->DeviceRunPara.eShowView)
                            {
                            case SType_Meas_Level:
                                func_Measure_WaterLevel_View_Show(0);
                                break;
                            case SType_Meas_Water_Quality:
                                func_Measure_Water_Quality_View_Show(0);
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
                            pst_MainloopSystemPara->DeviceRunPara.eShowView++;
                            if(pst_MainloopSystemPara->DeviceRunPara.eShowView >= SType_Max)
                            {
                                pst_MainloopSystemPara->DeviceRunPara.eShowView = SType_Meas_Level;
                            }
                        }
                    }
                    else	//NFC认证通过，显示认证通过界面
                    {
                        func_display_Authorize_Menu();
                        ucNFCShowFlag = 1;
                        
                    }
                }
            }
            
            if(ucNFCShowFlag == 0)	//当前显示界面为正常测量界面或者NFC未经过认证界面
            {
                if(ucBarLostCnt >= ucLCDShowDelayCnt)	//本轮显示界面已轮询完成
                {
                    ucBarLostCnt = 0;
                    pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.cMagnetic_Bar_Status = 0;
                    cBarEventFinishFlag = 1;
                    if(pst_MainloopSystemPara->DeviceRunPara.ucOLEDInitFlag == 1)
                    {
                        pst_MainloopSystemPara->DeviceRunPara.ucOLEDInitFlag = 0;
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
                    pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.cMagnetic_Bar_Status = 0;
                    uint8_t u8Result = drv_EC200U_4G_Module_Init(0);
					if(u8Result != 0)
					{
						pst_MainloopSystemPara->DeviceRunPara.usDevStatus |= 0x01;
					}
					else
					{
						pst_MainloopSystemPara->DeviceRunPara.enUploadStatus = Status_OK;
					}
                    if(pst_MainloopSystemPara->DeviceRunPara.ucOLEDInitFlag == 1)
                    {
                        pst_MainloopSystemPara->DeviceRunPara.ucOLEDInitFlag = 0;
                        func_OLED_PowerDown_DeInit(); //关闭OLED电源
                    }
                }
            }
        }
	}
	//磁棒状态检测结束

	//判断当前定时器是否由延时等待服务器通讯事件产生
    if(pst_MainloopSystemPara->DeviceRunPara.cWaitServerCMDFlag == 1)
    {
        pst_MainloopSystemPara->DeviceRunPara.cWaitServerCMDCnt++;
        if(pst_MainloopSystemPara->DeviceRunPara.cWaitServerCMDCnt >= 20)
        {
            cServerEventFinishFlag = 1;
            pst_MainloopSystemPara->DeviceRunPara.cWaitServerCMDCnt = 0;
            pst_MainloopSystemPara->DeviceRunPara.cWaitServerCMDFlag = 0;
        }
    }

	//判断当前定时器是否由测量传感器延时等待事件产生
    if(pst_MainloopSystemPara->DeviceRunPara.cMeasDelayFlag == 1)
    {
        pst_MainloopSystemPara->DeviceRunPara.cMeasDelayCnt++;
        if(pst_MainloopSystemPara->DeviceRunPara.cMeasDelayCnt >= 10)
        {
            cMeasSensorEventFinishFlag = 1;
            pst_MainloopSystemPara->DeviceRunPara.cMeasDelayCnt = 0;
            pst_MainloopSystemPara->DeviceRunPara.cMeasDelayFlag = 0;
        }
    }

	//判断当前定时器是否由蓝牙通讯事件产生
	if(pst_MainloopSystemPara->DeviceRunPara.cDeviceBTConnectFlag == 1)
	{
		pst_MainloopSystemPara->DeviceRunPara.usDeviceBTWaitCnt++;
		if(pst_MainloopSystemPara->DeviceRunPara.usDeviceBTWaitCnt >= 180)	//等待20s
		{
			cBTEventFinishFlag = 1;
			pst_MainloopSystemPara->DeviceRunPara.cDeviceBTConnectFlag = 0;
			pst_MainloopSystemPara->DeviceRunPara.usDeviceBTWaitCnt = 0;
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
	if(pst_MainloopSystemPara->DeviceRunPara.cDeviceBTWaitConnectFlag == 1)
	{
		pst_MainloopSystemPara->DeviceRunPara.usDeviceBTWaitConnectCnt++;
		if(pst_MainloopSystemPara->DeviceRunPara.usDeviceBTWaitConnectCnt >= pst_MainloopSystemPara->DeviceRunPara.usDeviceBTWaitConnectMaxCnt)	//等待20s
		{
			cBTWaitConnectEventFinishFlag = 1;
			pst_MainloopSystemPara->DeviceRunPara.cDeviceBTWaitConnectFlag = 0;
			pst_MainloopSystemPara->DeviceRunPara.usDeviceBTWaitConnectCnt = 0;
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
		&& (pst_MainloopSystemPara->DeviceRunPara.cConnectServerFlag == 0) && (cBTEventFinishFlag == 1)
		&& (cBTWaitConnectEventFinishFlag == 1))
    {
        drv_mcu_Timer4_Stop();
        pst_MainloopSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_RUN;
    }
}

//NFC认证程序
void func_NFC_Check_Dispose(void)
{
	char ucNFCFindCardFlag = 0;
	char j = 0;

	//开启NFC电源及初始化配置，要注意，每次接触事件，仅初始化一次
	if(guc_NFCPWRInitFlag == 0)
	{
		guc_NFCPWRInitFlag = 1;
		#ifndef HW_VERSION_V1_1
		func_RF_NFC_PowerUp_Init();
		#else
		func_RXNTTL518_Init();
		#endif
	}
		
	//磁棒激活状态下，检测是否有卡片
	#ifdef HW_VERSION_V1_1
	ucNFCFindCardFlag = func_Search_Card();
	if(ucNFCFindCardFlag == 0)   //找到卡片
	{
		//找到卡片
		if(guc_NFC_Card_Flag == 0)
		{
			guc_NFC_Card_Flag = 1;
			if(pst_MainloopSystemPara->DevicePara.cDeviceIdenFlag == 0)
			{
				pst_MainloopSystemPara->DevicePara.cDeviceIdenFlag = 1;
				pst_MainloopSystemPara->DeviceRunPara.cDeviceStatusUploadFlag = 1;
				func_Save_Device_Parameter(DEV_IDEN_FLAG, (unsigned char*)&pst_MainloopSystemPara->DevicePara.cDeviceIdenFlag);
			}
			else
			{
				pst_MainloopSystemPara->DeviceRunPara.cEveryNFCDisposeFlag = 1;
				func_BD_PowerUp_Init();
				drv_mcu_ChangeUSART3_Source(MODULE_BD);
				//drv_LC86L_BD_Init();
			}
			
			if(pst_MainloopSystemPara->DevicePara.cDeviceIdenFlag == 1)  //设备认证通过后，才需要进行磁棒唤醒采集发送数据操作
			{
				if(pst_MainloopSystemPara->DeviceRunPara.cBarTouchFlag == 0)
				{
					pst_MainloopSystemPara->DeviceRunPara.cBarTouchFlag = 1;
				}
			}
		}
	}
	else
	{
		guc_NFC_Card_Flag = 0;
	}
	#else
	ucNFCFindCardFlag = func_FindCar();
	if(ucNFCFindCardFlag == 1)   //找到卡片
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
					pst_MainloopSystemPara->DevicePara.cDeviceIdenFlag = 1;
					func_Save_Device_Parameter(DEV_IDEN_FLAG, (unsigned char*)&pst_MainloopSystemPara->DevicePara.cDeviceIdenFlag);
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
	#endif
}

//设备初始化处理程序;每次设备从低功耗模式下唤醒时，需配置相关模块电源及初始化操作
void func_DeviceStart_Sensor_PowerUp_Dispose(void)
{
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
	//if(pst_MainloopSystemPara->DeviceRunPara.nDeviceCurSaveRecordCount >= pst_MainloopSystemPara->DevicePara.nDeviceSaveRecordCnt)
	//{
	//	Ddl_Delay1ms(500);
	//}
}

//获取设备板级传感器数据
void func_Get_Device_Sensors_Value(void)
{
	unsigned char ucRes = 1;
	static unsigned char ucRetryCnt = 0;
	float fValue = 0.0;
	static unsigned char ucChangeCnt = 0;

	//pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.cPhotosensitive_XYC_ALS_Status = 0;
	pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.sPhotosensitive_XYC_ALS_Data = func_ReadPhoto_XYC_ALS_Data();
	if((pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.sPhotosensitive_XYC_ALS_Data > 30))
	//if(pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.cPhotosensitive_XYC_ALS_Status == 0)
	{
		pst_MainloopSystemPara->DeviceRunPara.usDevStatus |= 0x0010;
		pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.cPhotosensitive_XYC_ALS_Status = 1;
	}
	else
	{
		pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.cPhotosensitive_XYC_ALS_Status = 0;
		pst_MainloopSystemPara->DeviceRunPara.usDevStatus &= 0xFFEF;
	}
	
	//func_SC7A20H_Read_FIFO_Buf(&pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A[0],&pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A[1],&pst_MainSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A[2]);
	pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A = SC7A20_Task();
	
	//读取水浸传感器状态
	drv_Get_Water_Immersion_Sensor_Status(&pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.cWater_Immersion_Status, &pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.fWater_Immersion_Level);
	if(pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.cWater_Immersion_Status == 0)
	{
		pst_MainloopSystemPara->DeviceRunPara.usDevStatus &= 0xFFF7;
	}
	else
	{
		pst_MainloopSystemPara->DeviceRunPara.usDevStatus |= 0x0008;
	}

	//读取当前电池电量
	//pst_MainloopSystemPara->DeviceRunPara.esDeviceRunState.fBattleVoltage = drv_Get_Battery_Level_Value(&pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent);
	pst_MainloopSystemPara->DeviceRunPara.esDeviceRunState.fBattleVoltage = drv_Get_Battery_Level_Value(&fValue);
	if((double)fValue <= 30.0)
	{
		if(ucRetryCnt >= 3)
		{
			ucRetryCnt = 0;
			pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.ucBattery_Level_Flag = 1;
		}
		else
		{
			pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.ucBattery_Level_Flag = 0;
			ucRetryCnt++;	//电池电量低
		}
		
	}
	else
	{
		ucRetryCnt = 0;
		pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.ucBattery_Level_Flag = 1;	//电池电量正常
	}

	if((fValue + 2.0) < pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent)
	{
		ucChangeCnt++;
		if(ucChangeCnt > 3)
		{
			ucChangeCnt = 0;
			//pst_MainloopSystemPara->DeviceRunPara.esDeviceRunState.fBattleVoltage = fValue;
			pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent = fValue;
		}
	}
	else
	{
		ucChangeCnt = 0;
		//pst_MainloopSystemPara->DeviceRunPara.esDeviceRunState.fBattleVoltage = fValue;
		pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent = fValue;
	}
	
	//外接485测量设备处理
	if((pst_MainloopSystemPara->DeviceRunPara.cMeasDelayFlag == 1) && (pst_MainloopSystemPara->DeviceRunPara.cMeasDelayCnt >= 5) && (pst_MainloopSystemPara->DeviceRunPara.cTimer1sTriggerFlag == 1))
	{
		if(pst_MainloopSystemPara->DeviceRunPara.cBlackLightFlag == 0)
		{
			func_Meas_Sensor_Dispose();
		}
		else
		{
			ucRes = func_BlackLight_Sensor_Dispose();
			if(ucRes == 0)
			{
				pst_MainloopSystemPara->DeviceRunPara.cMeasDelayFlag = 0;
				pst_MainloopSystemPara->DeviceRunPara.cMeasDelayCnt = 0;
			}
		}
	}
	pst_MainloopSystemPara->DeviceRunPara.cTimer1sTriggerFlag = 0;
}

//设备测量记录保存
void func_Save_Device_MeasRecord_Dispose()
{
	uint8_t ucRecordFlag = 0;
	uint8_t ucPressLevelExistFlag = 0;
	uint8_t ucRadarLevelExistFlag = 0;
	float fDifValue = 0.0;
	//float fRadarValue = 0.0;
	//float fPresValue = 0.0;
	uint8_t i = 0;
	uint8_t l = 0;
	static uint8_t ucSaveRecordFlag = 0;	//记录数据保存标志位：1->保存，0->不保存
	uint8_t ucTimeFlag = 0;
	struct tm tm;
	time_t now;

	#ifdef JOE_TEST
	gSt_DevMeasRecordData.nAttitude_SC7A = pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A + pst_MainloopSystemPara->DeviceRunPara.nDeviceCurUploadRecordCount * 10;
	#else
	gSt_DevMeasRecordData.nAttitude_SC7A = pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A;
	#endif
	gSt_DevMeasRecordData.cWater_Immersion_Status = pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.cWater_Immersion_Status;
	gSt_DevMeasRecordData.cPhotosensitive_XYC_ALS_Status = pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.cPhotosensitive_XYC_ALS_Status;

	if(pst_MainloopSystemPara->DeviceRunPara.nDeviceCurSaveRecordCount >= pst_MainloopSystemPara->DevicePara.nDeviceSaveRecordCnt)
	{
		pst_MainloopSystemPara->DeviceRunPara.nDeviceCurSaveRecordCount = 0;
		ucSaveRecordFlag = 1;	//需要保存记录数据
	}

	if((pst_MainloopSystemPara->DevicePara.cMeasSensorCount[0] > 0) || (pst_MainloopSystemPara->DevicePara.cMeasSensorCount[1] > 0))
	{
		if(pst_MainloopSystemPara->DeviceRunPara.cMeasDelayCnt >= 15)
		{
			ucTimeFlag = 1;
		}
		if(pst_MainloopSystemPara->DeviceRunPara.cGetMeasSensorValueFlag == 1)
		{
			for(l=0; l<2; l++)
			{
				for(i=0; i<pst_MainloopSystemPara->DevicePara.cMeasSensorCount[l]; i++)
				{
					if((pst_MainloopSystemPara->DevicePara.eMeasSensor[l][i] == Meas_Flowmeter)
						|| (pst_MainloopSystemPara->DevicePara.eMeasSensor[l][i] == Meas_HZ_Radar_Ultrasonic_Flow)
						|| (pst_MainloopSystemPara->DevicePara.eMeasSensor[l][i] == Meas_HZ_Ultrasonic_Flow))
					{
						gSt_DevMeasRecordData.fWaterVolume_s = pst_MainloopSystemPara->DeviceRunPara.esMeasData.esHZ_Radar_Ultrasonic_FlowData.fFlowValue;
						pst_MainloopSystemPara->DevicePara.fTotal_Volume += pst_MainloopSystemPara->DeviceRunPara.esMeasData.esHZ_Radar_Ultrasonic_FlowData.fFlowValue;
						#pragma diag_suppress=Pa039
						func_Save_Device_Parameter(DEV_TOTAL_VOLUME, (unsigned char*)&pst_MainloopSystemPara->DevicePara.fTotal_Volume);
						#pragma diag_warning=Pa039
						gSt_DevMeasRecordData.fWaterVolume_Total = pst_MainloopSystemPara->DevicePara.fTotal_Volume;
					}
					else if((pst_MainloopSystemPara->DevicePara.eMeasSensor[l][i] == Meas_HX_Radar_Ultrasonic_Flow)
						|| (pst_MainloopSystemPara->DevicePara.eMeasSensor[l][i] == Meas_HX_Flowmeter))
					{
						gSt_DevMeasRecordData.fWaterVolume_s = pst_MainloopSystemPara->DeviceRunPara.esMeasData.esHX_FlowmeterData.fFlowValue;
						pst_MainloopSystemPara->DevicePara.fTotal_Volume += pst_MainloopSystemPara->DeviceRunPara.esMeasData.esHX_FlowmeterData.fFlowValue;
						#pragma diag_suppress=Pa039
						func_Save_Device_Parameter(DEV_TOTAL_VOLUME, (unsigned char*)&pst_MainloopSystemPara->DevicePara.fTotal_Volume);
						#pragma diag_warning=Pa039
						gSt_DevMeasRecordData.fWaterVolume_Total = pst_MainloopSystemPara->DevicePara.fTotal_Volume;
					}
					else if(pst_MainloopSystemPara->DevicePara.eMeasSensor[l][i] == Meas_BY_Pressure_Level)
					{
						ucPressLevelExistFlag = 1;
						gSt_DevMeasRecordData.fWaterLevel_Pres = pst_MainloopSystemPara->DeviceRunPara.esMeasData.esBY_LevelData.fPressureWaterLevelValue;
					}
					else if(pst_MainloopSystemPara->DevicePara.eMeasSensor[l][i] == Meas_BY_Radar_Level)
					{
						ucRadarLevelExistFlag = 1;
						gSt_DevMeasRecordData.fWaterLevel_Radar = pst_MainloopSystemPara->DeviceRunPara.esMeasData.esBY_LevelData.fRadarWaterLevelValue;
					}
					else if (pst_MainloopSystemPara->DevicePara.eMeasSensor[l][i] == Meas_HZ_Radar_Level)
					{
						ucRadarLevelExistFlag = 1;
						gSt_DevMeasRecordData.fWaterLevel_Radar = pst_MainloopSystemPara->DeviceRunPara.esMeasData.esHZ_LevelData.fRadarWaterLevelValue;
					}
					else if(pst_MainloopSystemPara->DevicePara.eMeasSensor[l][i] == Meas_BY_Integrated_Conductivity)
					{
						gSt_DevMeasRecordData.fWaterQuality_COND = pst_MainloopSystemPara->DeviceRunPara.esMeasData.esBY_IntegratedConductivityData.fConductivityValue;
					}
					else if(pst_MainloopSystemPara->DevicePara.eMeasSensor[l][i] == Meas_HX_WaterQuality_COD)
					{
						gSt_DevMeasRecordData.fWaterQuality_COD = pst_MainloopSystemPara->DeviceRunPara.esMeasData.esHX_WaterQuality_CODData.fCODValue;
					}
					#ifdef WATERLEVEL_RADAR_PRESS
					//当前同时存在雷达测高及压力测高时，进行数据判断，当两者测量值相差超过0.3m时，以压力测高值为准
					if((ucPressLevelExistFlag == 1) && (ucRadarLevelExistFlag == 1))
					{
						fDifValue = gSt_DevMeasRecordData.fWaterLevel_Radar - gSt_DevMeasRecordData.fWaterLevel_Pres;
						if(fabs(fDifValue) >= 0.3)
						{
							gSt_DevMeasRecordData.fWaterLevel_Radar = gSt_DevMeasRecordData.fWaterLevel_Pres;
						}
					}
					else
					{
						if(ucPressLevelExistFlag == 1)
						{
							gSt_DevMeasRecordData.fWaterLevel_Radar = gSt_DevMeasRecordData.fWaterLevel_Pres;
						}
					}
					#endif
					ucRecordFlag = 1;
				}
			}
		}
	}
	else
	{
		ucRecordFlag = 1;
		ucTimeFlag = 1;
	}
	
	if((pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.ucBattery_Level_Flag == 1) 
	&& (ucRecordFlag == 1) && (ucSaveRecordFlag == 1)
	&& (ucTimeFlag == 1))
	{
		//if(pst_MainloopSystemPara->DeviceRunPara.nDeviceCurSaveRecordCount >= pst_MainloopSystemPara->DevicePara.nDeviceSaveRecordCnt)
		{
			//pst_MainloopSystemPara->DeviceRunPara.nDeviceCurSaveRecordCount = 0;
			ucSaveRecordFlag = 0;
			//将本次采集的数据存入存储中
			if((double)pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent >= 30.0)
			{
				func_Save_Device_MeasData();
				pst_MainloopSystemPara->DevicePara.nDeviceRecordCnt++;
				#pragma diag_suppress=Pa039
				func_Save_Device_Parameter(DEV_HIS_RECORD, (unsigned char*)&pst_MainloopSystemPara->DevicePara.nDeviceRecordCnt);
				#pragma diag_warning=Pa039
				drv_mcu_Get_RTC_Time(pst_MainloopSystemPara->DeviceRunPara.cDeviceCurDateTime);
				sscanf(pst_MainloopSystemPara->DeviceRunPara.cDeviceCurDateTime, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
				tm.tm_year -= 1900; // 由于tm_year是从1900年开始计数的
				tm.tm_mon -= 1;     // tm_mon是从0开始的，所以需要减1
				tm.tm_isdst = -1; // 自动判断夏令时
				now = mktime(&tm) - 8*60*60; 
				if(strlen(pst_MainloopSystemPara->DeviceRunPara.esDeviceRunState.cDevStartDateTime) > 1)
				{
					if(pst_MainloopSystemPara->DeviceRunPara.ulUploadRecordLostCnt == 0)
					{
						pst_MainloopSystemPara->DeviceRunPara.ulUploadRecordStartTime = (long)now; //记录上传数据开始时间
					}
					//pst_MainloopSystemPara->DeviceRunPara.ulUploadRecordLostCnt += pst_MainloopSystemPara->DevicePara.nDeviceUploadCnt / pst_MainloopSystemPara->DevicePara.nDeviceSaveRecordCnt;
					pst_MainloopSystemPara->DeviceRunPara.ulUploadRecordLostCnt++;
				}
			}	
		}
	}
}

//BD定位数据处理
void func_BD_Data_Dispose(void)
{
	#if 0
	char cData[] = {0x24,0x47,0x50,0x54,0x58,0x54,0x2C,0x30,0x31,0x2C,0x30,0x31,0x2C,0x30,0x32,0x2C,0x41,0x4E,0x54,0x53,0x54,0x41,0x54,0x55,0x53,0x3D,0x4F,0x50,
					0x45,0x4E,0x2A,0x32,0x42,0x0D,0x0A,0x24,0x47,0x4E,0x52,0x4D,0x43,0x2C,0x30,0x37,0x35,0x39,0x32,0x36,0x2E,0x30,0x30,0x30,0x2C,0x41,0x2C,0x32,	
					0x36,0x30,0x35,0x2E,0x35,0x37,0x33,0x33,0x32,0x32,0x2C,0x4E,0x2C,0x31,0x31,0x39,0x31,0x38,0x2E,0x39,0x31,0x33,0x35,0x37,0x36,0x2C,0x45,0x2C	,
					0x31,0x2E,0x32,0x33,0x2C,0x31,0x35,0x34,0x2E,0x31,0x33,0x2C,0x31,0x38,0x30,0x36,0x32,0x35,0x2C,0x2C,0x2C,0x41,0x2C,0x56,0x2A,0x30,0x33,0x0D,0x0A};
	#endif
	#if 1
	if(pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ucDataValidFlag == 0)
	{
		if((pst_MainloopSystemPara->UsartData.enUsart3Source == MODULE_NFC_RFID) || (pst_MainloopSystemPara->DeviceRunPara.cBD_GetValueCnt == 0))
		{
			if(pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.cMagnetic_Bar_Status == 0)
			{
				pst_MainloopSystemPara->DeviceRunPara.cBD_GetValueCnt++;
				func_BD_PowerUp_Init();
				drv_mcu_ChangeUSART3_Source(MODULE_BD);
			}
			
		}
		//pst_MainloopSystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_BD] = 1;
		//memcpy(pst_MainloopSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BD], cData, sizeof(cData));
		//北斗模块收到数据
		if(pst_MainloopSystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_BD] == 1)
		{
			if(pst_MainloopSystemPara->UsartData.usUsartxRecvDataLen[MODULE_BD] != 0)
			{
				pst_MainloopSystemPara->DeviceRunPara.cBD_GetDataFlag = 1;
				pst_MainloopSystemPara->DeviceRunPara.cBD_LostDataCnt = 0;
				if(strstr(pst_MainloopSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BD],"RMC") != NULL)
				{
					memcpy(guc_TextBDData, &pst_MainloopSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BD][42], 20);
					NMEA_GPRMC_Analysis((uint8_t *)pst_MainloopSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BD]);
				}   
				pst_MainloopSystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_BD] = 0;
				//memset(pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_BD], 0, USART_DATA_LEN_MAX);
				pst_MainloopSystemPara->UsartData.usUsartxRecvDataLen[MODULE_BD] = 0;
			}
			else
			{
				pst_MainloopSystemPara->DeviceRunPara.cBD_GetDataFlag = 0;
			}
			
		}
		else
		{
			pst_MainloopSystemPara->DeviceRunPara.cBD_GetDataFlag = 0;
		}
		if(pst_MainloopSystemPara->DeviceRunPara.cBD_LostDataCnt >= 3)
		{
			pst_MainloopSystemPara->DeviceRunPara.cBD_LostDataCnt = 0;
			if(pst_MainloopSystemPara->DevicePara.cDeviceIdenFlag == 1)
			{
				func_BD_PowerUp_Init();
				drv_mcu_ChangeUSART3_Source(MODULE_BD);
				//drv_LC86L_BD_Init();
			}
		}
	}

	//正常程序应该是开机先进行卫星定位，直到定位成功后，才进入正常运行模式，可进入低功耗休眠，此时可以关闭北斗电源
	if(pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ucDataValidFlag == 1)
	{
		pst_MainloopSystemPara->DeviceRunPara.cBD_GetValueCnt = 0;
		func_BD_PownDown_Deinit();
	}
	#else
	pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ucDataValidFlag = 1;
	#endif
}

//4G网络通讯处理
void func_4G_Connect_Server_Dispose(void)
{
	uint8_t ucRes = 0xFF;
	struct tm tm;
	time_t now;
    //time(&now);	

	if(pst_MainloopSystemPara->DeviceRunPara.cConnectServerFlag == 0)
	{
		//定时上传4G数据到测试平台
		if((pst_MainloopSystemPara->DeviceRunPara.nDeviceCurUploadRecordCount >= pst_MainloopSystemPara->DevicePara.nDeviceUploadCnt)
		|| (pst_MainloopSystemPara->DeviceRunPara.cBarTouchFlag == 2))
		{
			drv_mcu_Get_RTC_Time(pst_MainloopSystemPara->DeviceRunPara.cDeviceCurDateTime);
			sscanf(pst_MainloopSystemPara->DeviceRunPara.cDeviceCurDateTime, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
			tm.tm_year -= 1900; // 由于tm_year是从1900年开始计数的
			tm.tm_mon -= 1;     // tm_mon是从0开始的，所以需要减1
			tm.tm_isdst = -1; // 自动判断夏令时
			now = mktime(&tm) - 8*60*60; 

			if(pst_MainloopSystemPara->DeviceRunPara.cBarTouchFlag == 2)
			{
				pst_MainloopSystemPara->DeviceRunPara.cBarTouchFlag = 3; //清除触摸标志位
				if(strlen(pst_MainloopSystemPara->DeviceRunPara.esDeviceRunState.cDevStartDateTime) > 1)
				{
					pst_MainloopSystemPara->DeviceRunPara.ulBarTouchEventUploadTime = (long)now; //记录上传数据开始时间
				}
			}
			//OLED_Test(1);
			if(pst_MainloopSystemPara->DeviceRunPara.nDeviceCurUploadRecordCount >= pst_MainloopSystemPara->DevicePara.nDeviceUploadCnt)
			{
				pst_MainloopSystemPara->DeviceRunPara.nDeviceCurUploadRecordCount = 0;
				
			}
			//先统计本次要上传的数据个数
			pst_MainloopSystemPara->DeviceRunPara.cConnectServerFlag = 1;
		}
	}
	
	if((pst_MainloopSystemPara->DeviceRunPara.cConnectServerFlag == 1) && (pst_MainloopSystemPara->DeviceRunPara.c4GInitFlag == 1))
	{
		//OLED_Test(2);
		pst_MainloopSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_DIAG;
		ucRes = drv_EC200U_4G_Module_Init(1);
		//OLED_Test(3);
		//重复尝试重新初始化4G模块
		if(ucRes == 2)
		{
			pst_MainloopSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_RUN;
			pst_MainloopSystemPara->DeviceRunPara.usDevStatus |= 0x0001;
			pst_MainloopSystemPara->DeviceRunPara.cConnectServerFlag = 0;
			pst_MainloopSystemPara->DeviceRunPara.cBarTouchFlag = 0;
		}
		else if(ucRes == 0)
		{
			pst_MainloopSystemPara->DeviceRunPara.cConnectServerFlag = 0;
			pst_MainloopSystemPara->DeviceRunPara.cBarTouchFlag = 0;
			pst_MainloopSystemPara->DeviceRunPara.usDevStatus &= 0xFFFE;
			pst_MainloopSystemPara->DeviceRunPara.enUploadStatus = Status_OK;
			pst_MainloopSystemPara->DeviceRunPara.cWaitServerCMDFlag = 1;
			pst_MainloopSystemPara->DeviceRunPara.cWaitServerCMDCnt = 0;
			//pst_MainSystemPara->DeviceRunPara.enUploadStatus = Status_OK;
			if(pst_MainloopSystemPara->DeviceRunPara.cTimer4StartFlag == 0)
			{
				pst_MainloopSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_DIAG;
				drv_mcu_Timer4_Start();
			}
		}
	}
}

//MQTT协议处理
void func_Get_Server_MQTT_Dispose(void)
{
	unsigned short usPosition = 0;
	
	if(pst_MainloopSystemPara->DeviceRunPara.cWaitServerCMDFlag == 1)
	{
		if(pst_MainloopSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB][30] != 0x00)  //4G/NB模块收到数据
		{
			//if(func_Array_Find_Str(pst_MainSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],pst_MainSystemPara->DevicePara.cDeviceID) != NULL)
			{
				if(func_Array_Find_Str(pst_MainloopSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],100,"setConfig",9, &usPosition) == 0)
				{
					MQTT_SK_Set_Config(usPosition);
				}
				else if(func_Array_Find_Str(pst_MainloopSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],100,"getConfig",9, &usPosition) == 0)
				{
					MQTT_SK_Get_Config(usPosition);
				}
				else if(func_Array_Find_Str(pst_MainloopSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],100,"getData",7, &usPosition) == 0)
				{
					MQTT_SK_Get_Data();
				}
				else if(func_Array_Find_Str(pst_MainloopSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],100,"dataPt",6, &usPosition) == 0)
				{
					MQTT_SK_Data_Pass_Through();
				}
			}
		}
	}
}

//设备主循环处理程序
void func_System_Mainloop_Dispose(void)
{
	//1. 当前存在磁棒唤醒事件+当前设备未经过认证
	//磁棒状态检测
	if((pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.cMagnetic_Bar_Status == 1)
		//&& (pst_MainloopSystemPara->DevicePara.cDeviceIdenFlag == 0)) //当前设备未经过验证
		&& (pst_MainloopSystemPara->DeviceRunPara.cEveryNFCDisposeFlag == 0)
		&& (pst_MainloopSystemPara->DevicePara.cMonitorMode == 0)) //当前设备未经过验证
	{
		func_NFC_Check_Dispose();
	}
	else
	{
		if(pst_MainloopSystemPara->DeviceRunPara.eCurPowerType == Power_ON)	//开机状态下
		{
			gc_SystemPosi = 1;
			//每次系统从低功耗模式下唤醒时，需配置相关模块电源及初始化操作
			if(guc_SystemPowerInitFlag == 0)
			{ 
				guc_SystemPowerInitFlag = 1;
				func_DeviceStart_Sensor_PowerUp_Dispose();
				if(pst_MainloopSystemPara->DevicePara.cMonitorMode == 1)
				{
					func_Meas_Sensor_PowerOn_Init();
				}
			}
			gc_SystemPosi = 2;
			if(pst_MainloopSystemPara->DevicePara.cDeviceIdenFlag == 1) //当前设备NFC认证通过
			{
				//优先考虑设备是否处于调试模式
				if(pst_MainloopSystemPara->DeviceRunPara.cDebugModel == 1)
				{
					if(pst_MainloopSystemPara->DeviceRunPara.cDebugModelStartFlag == 0)	//调试模式下，且调试模式未启动
					{
						pst_MainloopSystemPara->DeviceRunPara.cDebugModelStartFlag = 1;
						if(pst_MainloopSystemPara->DeviceRunPara.cDeviceBTPowerOnFlag == 0)
						{
							func_BT05_PowerUp_Init();
						}
						
						pst_MainloopSystemPara->DeviceRunPara.cDeviceBTWaitConnectFlag = 1;
						pst_MainloopSystemPara->DeviceRunPara.usDeviceBTWaitConnectCnt = 0;
						pst_MainloopSystemPara->DeviceRunPara.usDeviceBTWaitConnectMaxCnt = 1800;	//调试模式下，蓝牙等待连接时间为30分钟
						if(pst_MainloopSystemPara->DeviceRunPara.cTimer4StartFlag == 0)
						{
							pst_MainloopSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_DIAG;
							drv_mcu_Timer4_Start();
						}
						
						//开启外接设备电源，进行外接设备参数读写操作
						//func_Meas_Sensor_PowerOn_Init();
					}
				}
				else
				{
					if(pst_MainloopSystemPara->DeviceRunPara.cDebugModelStartFlag == 1)
					{
						pst_MainloopSystemPara->DeviceRunPara.cDebugModelStartFlag = 0;
					}
					//目前僅考慮在模塊喚醒狀態下才可以進行藍牙通訊，後續可考虑蓝牙电源不关闭，一直支持通讯操作
					if(pst_MainloopSystemPara->DeviceRunPara.esDeviceSensorsData.cMagnetic_Bar_Status == 1)
					{
						if(pst_MainloopSystemPara->DeviceRunPara.cDeviceBTPowerOnFlag == 0)
						{
							func_BT05_PowerUp_Init();
						}
						
						pst_MainloopSystemPara->DeviceRunPara.cDeviceBTWaitConnectFlag = 1;
						pst_MainloopSystemPara->DeviceRunPara.usDeviceBTWaitConnectCnt = 0;
						pst_MainloopSystemPara->DeviceRunPara.usDeviceBTWaitConnectMaxCnt = 30;
						if(pst_MainloopSystemPara->DeviceRunPara.cTimer4StartFlag == 0)
						{
							pst_MainloopSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_DIAG;
							drv_mcu_Timer4_Start();
						}
					}
				}
				if(pst_MainloopSystemPara->DeviceRunPara.cMeasDelayFlag == 0)
				{
					if(pst_MainloopSystemPara->DevicePara.cMonitorMode == 1)
					{
						pst_MainloopSystemPara->DeviceRunPara.cGetMeasSensorValueSuccFlag = 0;
						pst_MainloopSystemPara->DeviceRunPara.cGetMeasSensorValueFlag = 0;
						pst_MainloopSystemPara->DeviceRunPara.cGetSensorCnt = 0;
						pst_MainloopSystemPara->DeviceRunPara.cMeasDelayFlag = 1;
						if(pst_MainloopSystemPara->DeviceRunPara.cTimer4StartFlag == 0)
						{
							pst_MainloopSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_DIAG;
							drv_mcu_Timer4_Start();
						}
					}
					else
					{
						//当前采用间隔计数值达到设置的采样间隔，开启外接传感器电源
						if(((pst_MainloopSystemPara->DeviceRunPara.nDeviceCurSampleCount >= pst_MainloopSystemPara->DevicePara.nDeviceSampleGapCnt)
						|| (pst_MainloopSystemPara->DeviceRunPara.cBarTouchFlag == 1))
						&& (pst_MainloopSystemPara->DevicePara.cMeasSensorCount > 0))
						{
							if(pst_MainloopSystemPara->DeviceRunPara.cBarTouchFlag == 1)
							{
								pst_MainloopSystemPara->DeviceRunPara.cBarTouchFlag = 2;
							}
							if(pst_MainloopSystemPara->DeviceRunPara.nDeviceCurSampleCount >= pst_MainloopSystemPara->DevicePara.nDeviceSampleGapCnt)
							{
								pst_MainloopSystemPara->DeviceRunPara.nDeviceCurSampleCount = 0;
							}
							//非调试模式下开启外接传感器电源，因为调试模式下，已经在前面开启了电源
							if(pst_MainloopSystemPara->DevicePara.cMonitorMode == 0)
							{
								func_Meas_Sensor_PowerOn_Init();
							}
							
							pst_MainloopSystemPara->DeviceRunPara.cGetMeasSensorValueSuccFlag = 0;
							pst_MainloopSystemPara->DeviceRunPara.cGetMeasSensorValueFlag = 0;
							pst_MainloopSystemPara->DeviceRunPara.cGetSensorCnt = 0;
							pst_MainloopSystemPara->DeviceRunPara.cMeasDelayFlag = 1;
							pst_MainloopSystemPara->DeviceRunPara.cMeasDelayCnt = 0;
							
							if(pst_MainloopSystemPara->DeviceRunPara.cTimer4StartFlag == 0)
							{
								pst_MainloopSystemPara->DeviceRunPara.enDeviceRunMode = DEVICE_RUN_STATE_DIAG;
								drv_mcu_Timer4_Start();
							}
						}
					}
					
				}
				
				//获取设备板级传感器数据
				func_Get_Device_Sensors_Value();
				gc_SystemPosi = 3;
				//当前模式为正常运行模式
				if(pst_MainloopSystemPara->DevicePara.cMonitorMode == 0)
				{
					func_Save_Device_MeasRecord_Dispose();
					gc_SystemPosi = 4;
					//蓝牙通讯处理
					
					gc_SystemPosi = 5;
					//BD定位数据处理
					func_BD_Data_Dispose();
					gc_SystemPosi = 6;
					//OLED_Test(6);
					//4G网络通讯处理
					func_4G_Connect_Server_Dispose();
					gc_SystemPosi = 7;
					//OLED_Test(4);
					//MQTT协议处理
					func_Get_Server_MQTT_Dispose();
					gc_SystemPosi = 8;
				}
				
			}
		}
	}
	
	#ifdef TIMER1_DISPOSE
	//判断当前1s定时器是否开启
	if(pst_MainloopSystemPara->DeviceRunPara.cTimer4StartFlag == 1)
	{
		gc_SystemPosi = 9;
		while(pst_MainloopSystemPara->DeviceRunPara.cTimer4_1SEC_ReflashFlag == 0)
		{

		}
		gc_SystemPosi = 10;
		pst_MainloopSystemPara->DeviceRunPara.cTimer4_1SEC_ReflashFlag = 0;	//清除标志位
		func_System_Timer1s_Dispose();
		gc_SystemPosi = 11;
	}
	#endif
	//OLED_Test(5);
	if(((pst_MainloopSystemPara->DeviceRunPara.enDeviceRunMode != DEVICE_RUN_STATE_DIAG) && (pst_MainloopSystemPara->DevicePara.cMonitorMode == 0)))
	{
		func_Enter_LowPower_Stop_Mode();
	}
}


/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
