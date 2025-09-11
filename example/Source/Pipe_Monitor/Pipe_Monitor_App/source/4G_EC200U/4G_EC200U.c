/**
 *******************************************************************************
 * @file  Pipe_Monitor_BootLoader\drivers\device_drv\4G_EC200U\4G_EC200U.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-12       Joe             First version
 @endverbatim

 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "4G_EC200U.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "drv_Storage_W25Q128.h"
#include "WatchDog.h"
#include "drv_RTC.h"
/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define EC200U_BUF_SIZE             (800U)
/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
static SystemPataSt *pst_EC200USystemPara;
/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
en_4G_Module_Init_State gE_4G_Module_Init_CMD = Module_START_WAIT_CMD;
//uint8_t ucSendBuf[EC200U_BUF_SIZE] = {0};
//uint8_t ucRecvBuf[EC200U_BUF_SIZE] = {0};
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

void func_EC200U_4G_PownDown_Deinit(void)
{
    stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Ana;
	PORT_Init(EC200U_4G_MODULE_RST_PORT, EC200U_4G_MODULE_RST_PIN, &stcPortInit);

	PWRDCE_PIN_CLOSE();	//关闭4G模块电源
    PORT_ResetBits(EC200U_4G_MODULE_RST_PORT, EC200U_4G_MODULE_RST_PIN);
    PORT_ResetBits(USART1_TX_PORT, USART1_TX_PIN);
    PORT_ResetBits(USART1_RX_PORT, USART1_TX_PIN);
}

/**
 * @brief  4G EC200U Module GPIO Initialize.
 * @param  None
 * @retval None
 */
void EC200U_4G_Module_GPIO_Init(void)
{
    stc_port_init_t stcGpioInit;

    MEM_ZERO_STRUCT(stcGpioInit);
    stcGpioInit.enPinMode = Pin_Mode_Out;
    stcGpioInit.enPullUp = Enable;
    
    (void)PORT_Init(EC200U_4G_MODULE_RST_PORT, EC200U_4G_MODULE_RST_PIN, &stcGpioInit);
    #ifdef HW_VERSION_V1_1
    (void)PORT_Init(USART1_TX_PORT, USART1_TX_PIN, &stcGpioInit);  
    //PORT_SetBits(USART1_TX_PORT,USART1_TX_PIN);

    stcGpioInit.enPinMode = Pin_Mode_In;
    stcGpioInit.enPullUp = Enable;
    (void)PORT_Init(USART1_RX_PORT, USART1_RX_PIN, &stcGpioInit);
    #endif

    PWRDCE_PIN_OPEN();
    Ddl_Delay1ms(100);
}

#if 0
uint16_t func_get_onenet_test_value(uint8_t *ucDataArr, unsigned char ucType)
{
    uint8_t ucTempArr[300] = {0};
    //uint8_t ucTempArr1[310] = {0};
    uint16_t usDataLen = 0;
    double dTemp = 0.0;
    if(ucType == 0)
    {
        pst_EC200USystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A = 6;
        pst_EC200USystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent = 1.666;
        pst_EC200USystemPara->DeviceRunPara.esDeviceSensorsData.sPhotosensitive_XYC_ALS_Data = 10;
    }
    
    #if 1
    dTemp = (double)(pst_EC200USystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent / 34.0f);
    sprintf((char *)ucTempArr, "{\"id\":\"2440181484\",\"version\":\"1.0\",\"params\":{\"attitude\":{\"value\":%d},\"Battery\":{\"value\":%.3lf},\"Photo\":{\"value\":%d},\"Water\":{\"value\":false}}}", pst_EC200USystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A,
                                                                                        dTemp,
                                                                                        pst_EC200USystemPara->DeviceRunPara.esDeviceSensorsData.sPhotosensitive_XYC_ALS_Data);
    #else
    sprintf((char *)ucTempArr, "{\"id\":\"2440114148\",\"version\":\"1.0\",\"params\":{\"level\":{\"value\":%.1f}}}", 9.6);
    #endif
    usDataLen = strlen((char *)ucTempArr);
    //ucTempArr1[0] = 0x01;
    //ucTempArr1[1] = 0x00;
    //ucTempArr1[2] = usDataLen;
    //sprintf(ucTempArr1,"\x01\x00%x",usDataLen);
    //memcpy(ucTempArr1+3, ucTempArr, usDataLen);
    //usDataLen += 3;
    memcpy(ucDataArr, ucTempArr, usDataLen);
    return usDataLen;                          
}
#endif
//获取设备注册命令数据
uint16_t func_Get_DevRegCMD_Data(uint8_t *ucDataArr)
{
    char ucTempArr[500] = {0};
    //uint8_t ucTempDataArr[50] = {0};
    uint8_t ucTempMonitorNameArr[600] = {0};
    uint8_t ucParaNameArr[30] = {0};
    //uint8_t ucDataArr[20] = {0}; //用于存储数据
    uint16_t usDataLen = 0;
    uint8_t j = 0;
    uint8_t l = 0;
    uint8_t i = 0;
    uint8_t ucMeasWaterVolumeFlag = 0;
    time_t now;
    struct tm tm;
    //time(&now);
    drv_mcu_Get_RTC_Time(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime);
    sscanf(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    tm.tm_year -= 1900; // 由于tm_year是从1900年开始计数的
    tm.tm_mon -= 1;     // tm_mon是从0开始的，所以需要减1
    tm.tm_isdst = -1;
    now = mktime(&tm) - 8*60*60; 
    sprintf(ucTempArr, "{\"clientId\":\"%s\",\"pver\":24,\"msgId\":%ld,\"data\":{\"colname\":[\"deviceName\",\"collectGap\",\"reportGap\"],", 
                                                                                        pst_EC200USystemPara->DevicePara.cDeviceID,
                                                                                        (long)now);
    usDataLen =  strlen(ucTempArr);
    sprintf(&ucTempArr[usDataLen],"\"colvalue\":[\"%s\",\"%dm\",\"%dm\"],",pst_EC200USystemPara->DevicePara.cDeviceID,
                                                            pst_EC200USystemPara->DevicePara.nDeviceSampleGapCnt,
                                                            pst_EC200USystemPara->DevicePara.nDeviceUploadCnt);

    sprintf((char *)ucTempMonitorNameArr,"\"colmonitor\":[");
    //if(pst_EC200USystemPara->DevicePara.cMeasSensorCount > 0)
    //拼接外接传感器数据
    for(l=0; l<2; l++)
    {
        for(i=0; i<pst_EC200USystemPara->DevicePara.cMeasSensorCount[l]; i++)
        {
            memset(ucParaNameArr, 0, sizeof(ucParaNameArr));
            //memset(ucDataArr, 0, sizeof(ucDataArr));
            j = 0;
            //if(i != 0)
            {
            //    ucParaNameArr[j++] = ',';
            }
            
            //ucDataArr[0] = ',';
            switch (pst_EC200USystemPara->DevicePara.eMeasSensor[l][i])
            {
            case Meas_BY_Integrated_Conductivity:  //  一体式电导率
                sprintf((char *)&ucParaNameArr[j], "\"COND\",");
                //sprintf((char *)&ucDataArr[1], "\"%.3f\"", pst_EC200USystemPara->DeviceRunPara.esMeasData.fWaterQuality_CODValue);
                break;
            case Meas_BY_Radar_Level:
            case Meas_HZ_Radar_Level:
                sprintf((char *)&ucParaNameArr[j], "\"water_height\",");
                //sprintf((char *)&ucDataArr[1], "\"%.3f\"", pst_EC200USystemPara->DeviceRunPara.esMeasData.fWaterLevel);
                break;
            #ifndef WATERLEVEL_RADAR_PRESS
            case Meas_BY_Pressure_Level:
                sprintf((char *)&ucParaNameArr[j], "\"pipeline_water_height\",");
            //sprintf((char *)&ucDataArr[1], "\"%.3f\"", pst_EC200USystemPara->DeviceRunPara.esMeasData.fWaterLevel);
                break;
            #endif
            case Meas_Flowmeter:
            case Meas_HZ_Radar_Ultrasonic_Flow:
            case Meas_HZ_Ultrasonic_Flow:
            case Meas_HX_Radar_Ultrasonic_Flow:
            case Meas_HX_Flowmeter:
                sprintf((char *)&ucParaNameArr[j], "\"curr_volume_s\",");
                ucMeasWaterVolumeFlag = 1;
                //sprintf((char *)&ucDataArr[1], "\"%.3f\"", pst_EC200USystemPara->DeviceRunPara.esMeasData.fVolumeValue);
                break;
            default:
                break;
            }
            
            (void)strcat((char *)ucTempMonitorNameArr, (char *)ucParaNameArr);
            //(void)strcat((char *)ucTempDataArr, ucDataArr);
        }
    }
    

    if(ucMeasWaterVolumeFlag == 1)
    {
        //当前设备支持流量测量，因此要加上累计流量数据
        memset(ucParaNameArr, 0, sizeof(ucParaNameArr));
        sprintf((char *)&ucParaNameArr[j], "\"total_volume\",");

        (void)strcat((char *)ucTempMonitorNameArr, (char *)ucParaNameArr);
    }

    //拼接板级传感器数据
    //姿态传感器数据
    memset(ucParaNameArr, 0, sizeof(ucParaNameArr));
    //memset(ucDataArr, 0, sizeof(ucDataArr));
    sprintf((char *)&ucParaNameArr[0], "\"attitude\",");
    //sprintf((char *)&ucDataArr[0], ",\"%d\"", pst_EC200USystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A);
    (void)strcat((char *)ucTempMonitorNameArr, (char *)ucParaNameArr);
    //(void)strcat((char *)ucTempDataArr, ucDataArr);

    //水浸传感器
    memset(ucParaNameArr, 0, sizeof(ucParaNameArr));
    //memset(ucDataArr, 0, sizeof(ucDataArr));
    sprintf((char *)&ucParaNameArr[0], "\"water_immersion\",");
    //sprintf((char *)&ucDataArr[0], ",\"%d\"", pst_EC200USystemPara->DeviceRunPara.esDeviceSensorsData.cWater_Immersion_Status);
    (void)strcat((char *)ucTempMonitorNameArr, (char *)ucParaNameArr);
    //(void)strcat((char *)ucTempDataArr, ucDataArr);

    //光照传感器
    memset(ucParaNameArr, 0, sizeof(ucParaNameArr));
    //memset(ucDataArr, 0, sizeof(ucDataArr));
    sprintf((char *)&ucParaNameArr[0], "\"photosensitive\",");
    //sprintf((char *)&ucDataArr[0], ",\"%d\"]}}", pst_EC200USystemPara->DeviceRunPara.esDeviceSensorsData.cPhotosensitive_XYC_ALS_Status);
    (void)strcat((char *)ucTempMonitorNameArr, (char *)ucParaNameArr);

    //设备开机时间
    memset(ucParaNameArr, 0, sizeof(ucParaNameArr));
    sprintf((char *)&ucParaNameArr[0], "\"timeStart\",");
    (void)strcat((char *)ucTempMonitorNameArr, (char *)ucParaNameArr);

    //设备运行时间
    memset(ucParaNameArr, 0, sizeof(ucParaNameArr));
    sprintf((char *)&ucParaNameArr[0], "\"timeRun\",");
    (void)strcat((char *)ucTempMonitorNameArr, (char *)ucParaNameArr);

    //电池电量
    memset(ucParaNameArr, 0, sizeof(ucParaNameArr));
    sprintf((char *)&ucParaNameArr[0], "\"batV\",");
    (void)strcat((char *)ucTempMonitorNameArr, (char *)ucParaNameArr);

    //纬度
    memset(ucParaNameArr, 0, sizeof(ucParaNameArr));
    sprintf((char *)&ucParaNameArr[0], "\"lng\",");
    (void)strcat((char *)ucTempMonitorNameArr, (char *)ucParaNameArr);

    //经度
    memset(ucParaNameArr, 0, sizeof(ucParaNameArr));
    sprintf((char *)&ucParaNameArr[0], "\"lat\",");
    (void)strcat((char *)ucTempMonitorNameArr, (char *)ucParaNameArr);

    //设备状态
    memset(ucParaNameArr, 0, sizeof(ucParaNameArr));
    //memset(ucDataArr, 0, sizeof(ucDataArr));
    sprintf((char *)&ucParaNameArr[0], "\"status\"]}}");
    //sprintf((char *)&ucDataArr[0], ",\"%d\"]}}", pst_EC200USystemPara->DeviceRunPara.esDeviceSensorsData.cPhotosensitive_XYC_ALS_Status);
    (void)strcat((char *)ucTempMonitorNameArr, (char *)ucParaNameArr);
    //(void)strcat((char *)ucTempDataArr, ucDataArr);

    (void)strcat((char *)ucTempArr, (char *)ucTempMonitorNameArr);
    usDataLen = strlen((char *)ucTempArr);

    memcpy(ucDataArr, ucTempArr, usDataLen);
    return usDataLen;                          
}

//获取设备监测项数据上报报文
uint16_t func_Get_DataUploadCMD_Data(uint8_t *ucDataArr)
{
    uint8_t ucTempArr[800] = {0};
    uint8_t ucTempValueArr[6][150] = {{0}}; //用于存储数据
    uint8_t ucTempAttiArr[50] = {0}; //用于存储姿态数据
    uint8_t ucTempWaterImmersionArr[50] = {0}; //用于存储水浸数据
    uint8_t ucTempPhotosensitiveArr[50] = {0}; //用于存储光照数据
    //uint8_t ucParaNameArr[10] = {0};
    uint8_t ucWaterLevel_Radar_Flag = 0;    //雷达液位测量传感器存在标志位
    uint8_t ucWaterLevel_Pressure_Flag = 0; //压力液位测量传感器存在标志位
    uint8_t ucWaterQuality_COD_Flag = 0; //水质COD测量传感器存在标志位
    uint8_t ucWaterQuality_COND_Flag = 0; //水质电导率测量传感器存在标志位
    uint8_t ucMeasWaterVolumeFlag = 0; //流量测量传感器存在标志位

    uint16_t usDataLen = 0;
    //uint8_t ucMeasWaterVolumeFlag = 0;
    uint8_t i = 0;
    uint8_t l = 0;
    int nPosi = 0;
    uint8_t ucRecordCnt = 0;
    uint32_t ulRecordCnt = 0;
    DevMeasRecordDataSt st_TempValue;
    unsigned long ulTime = 0;
    time_t now;
    //time(&now);
    struct tm tm;
    //time(&now);
    drv_mcu_Get_RTC_Time(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime);
    sscanf(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    tm.tm_year -= 1900; // 由于tm_year是从1900年开始计数的
    tm.tm_mon -= 1;     // tm_mon是从0开始的，所以需要减1
    tm.tm_isdst = -1;
    now = mktime(&tm) - 8*60*60; //将时间转换为UTC时间，减去8小时

    //if(pst_EC200USystemPara->DeviceRunPara.ulUploadRecordLostCnt == pst_EC200USystemPara->DevicePara.nDeviceUploadCnt / pst_EC200USystemPara->DevicePara.nDeviceSaveRecordCnt)
    {
    //    pst_EC200USystemPara->DeviceRunPara.ulUploadRecordStartTime = (long)now; //记录上传数据开始时间
    }
    if(pst_EC200USystemPara->DeviceRunPara.cBarTouchFlag == 3)
    {
        ulTime = pst_EC200USystemPara->DeviceRunPara.ulBarTouchEventUploadTime;
    }
    else
    {
        ulTime = pst_EC200USystemPara->DeviceRunPara.ulUploadRecordStartTime;
    }

    sprintf((char *)ucTempArr, "{\"clientId\":\"%s\",\"pver\":24,\"beginTime\":%ld,\"msgId\":%ld,\"gap\":%d,\"data\":{", 
                                                                                        pst_EC200USystemPara->DevicePara.cDeviceID,
                                                                                        ulTime,
                                                                                        (long)now,
                                                                                        pst_EC200USystemPara->DevicePara.nDeviceSaveRecordCnt * 60);
    //根据当前设备上传记录间隔与采用间隔；当前滞留计数，计算本次需要上传的数据个数
    //ulRecordCnt = 0;//pst_EC200USystemPara->DevicePara.nDeviceUploadCnt / pst_EC200USystemPara->DevicePara.nDeviceSaveRecordCnt;
    ulRecordCnt = pst_EC200USystemPara->DeviceRunPara.ulUploadRecordLostCnt;
    if(ulRecordCnt >= 10)
    {
        ucRecordCnt = 10;
    }
    else
    {
        ucRecordCnt = (uint8_t)ulRecordCnt;
    }
    if(pst_EC200USystemPara->DeviceRunPara.cBarTouchFlag == 3)
    {
        ucRecordCnt = 1;
    }
    pst_EC200USystemPara->DeviceRunPara.ucCurUploadRecordCnt = ucRecordCnt; //记录本次上传数据个数
    
    //先确认设备外接传感器类型
    for(l=0; l<2; l++)
    {
        for(i=0; i<pst_EC200USystemPara->DevicePara.cMeasSensorCount[l]; i++)
        {
            switch (pst_EC200USystemPara->DevicePara.eMeasSensor[l][i])
            {
            case Meas_BY_Radar_Level:
            case Meas_HZ_Radar_Level:
                ucWaterLevel_Radar_Flag = 1;
                memcpy(&ucTempValueArr[0], "\"water_height\":[", 16);
                break;
            #ifndef WATERLEVEL_RADAR_PRESS
            case Meas_BY_Pressure_Level:
                ucWaterLevel_Pressure_Flag = 1;
                memcpy(&ucTempValueArr[1], "\"pipeline_water_height\":[", 25);
                break;
            #endif
            case Meas_HX_WaterQuality_COD:
                ucWaterQuality_COD_Flag = 1;
                memcpy(&ucTempValueArr[2], "\"COD\":[", 7);
                break;
            case Meas_BY_Integrated_Conductivity:
                ucWaterQuality_COND_Flag = 1;
                memcpy(&ucTempValueArr[3], "\"COND\":[", 8);
                break;
            case Meas_Flowmeter:
            case Meas_HZ_Radar_Ultrasonic_Flow:
            case Meas_HZ_Ultrasonic_Flow:
            case Meas_HX_Radar_Ultrasonic_Flow:
            case Meas_HX_Flowmeter:
                ucMeasWaterVolumeFlag = 1;
                memcpy(&ucTempValueArr[4], "\"curr_volume_s\":[", 17);
                memcpy(&ucTempValueArr[5], "\"total_volume\":[", 16);
                break;
            default:
                break;
            }
        }
    }
    

    memcpy(&ucTempAttiArr[0], "\"attitude\":[", 12);
    memcpy(&ucTempWaterImmersionArr[0], "\"water_immersion\":[", 19);
    memcpy(&ucTempPhotosensitiveArr[0], "\"photosensitive\":[", 18);

    if(pst_EC200USystemPara->DeviceRunPara.cBarTouchFlag == 0)
    {
        //根据本次要上传的数据个数，从存储中读取该数据
        for(nPosi=0; nPosi<ucRecordCnt; nPosi++)
        {
            func_Get_Device_MeasData_Record(pst_EC200USystemPara->DevicePara.nDeviceRecordCnt - ulRecordCnt + nPosi, &st_TempValue);
            if(ucWaterLevel_Radar_Flag == 1)
            {
                sprintf((char *)&ucTempValueArr[0][strlen((char *)ucTempValueArr[0])], "%.3lf,", (double)st_TempValue.fWaterLevel_Radar);
            }
            #ifndef WATERLEVEL_RADAR_PRESS
            if(ucWaterLevel_Pressure_Flag == 1)
            {
                sprintf((char *)&ucTempValueArr[1][strlen((char *)ucTempValueArr[1])], "%.3lf,", (double)st_TempValue.fWaterLevel_Pres);
            }
            #endif
            if(ucWaterQuality_COD_Flag == 1)
            {
                sprintf((char *)&ucTempValueArr[2][strlen((char *)ucTempValueArr[2])], "%.3lf,", (double)st_TempValue.fWaterQuality_COD);
            }
            if(ucWaterQuality_COND_Flag == 1)
            {
                sprintf((char *)&ucTempValueArr[3][strlen((char *)ucTempValueArr[3])], "%.3lf,", (double)st_TempValue.fWaterQuality_COND);
            }
            if(ucMeasWaterVolumeFlag == 1)
            {
                sprintf((char *)&ucTempValueArr[4][strlen((char *)ucTempValueArr[4])], "%.3lf,", (double)st_TempValue.fWaterVolume_s);
                sprintf((char *)&ucTempValueArr[5][strlen((char *)ucTempValueArr[5])], "%.3lf,", (double)st_TempValue.fWaterVolume_Total);
            }

            sprintf((char *)&ucTempAttiArr[strlen((char *)ucTempAttiArr)], "%d,", st_TempValue.nAttitude_SC7A);
            sprintf((char *)&ucTempWaterImmersionArr[strlen((char *)ucTempWaterImmersionArr)], "%d,", st_TempValue.cWater_Immersion_Status);
            sprintf((char *)&ucTempPhotosensitiveArr[strlen((char *)ucTempPhotosensitiveArr)], "%d,", st_TempValue.cPhotosensitive_XYC_ALS_Status);
        }
    }
    else
    {
        if(ucWaterLevel_Radar_Flag == 1)
        {
            sprintf((char *)&ucTempValueArr[0][strlen((char *)ucTempValueArr[0])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterLevel_Radar);
        }
        #ifndef WATERLEVEL_RADAR_PRESS
        if(ucWaterLevel_Pressure_Flag == 1)
        {
            sprintf((char *)&ucTempValueArr[1][strlen((char *)ucTempValueArr[1])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterLevel_Pres);
        }
        #endif
        if(ucWaterQuality_COD_Flag == 1)
        {
            sprintf((char *)&ucTempValueArr[2][strlen((char *)ucTempValueArr[2])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterQuality_COD);
        }
        if(ucWaterQuality_COND_Flag == 1)
        {
            sprintf((char *)&ucTempValueArr[3][strlen((char *)ucTempValueArr[3])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterQuality_COND);
        }
        if(ucMeasWaterVolumeFlag == 1)
        {
            sprintf((char *)&ucTempValueArr[4][strlen((char *)ucTempValueArr[4])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterVolume_s);
            sprintf((char *)&ucTempValueArr[5][strlen((char *)ucTempValueArr[5])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterVolume_Total);
        }

        sprintf((char *)&ucTempAttiArr[strlen((char *)ucTempAttiArr)], "%d,", gSt_DevMeasRecordData.nAttitude_SC7A);
        sprintf((char *)&ucTempWaterImmersionArr[strlen((char *)ucTempWaterImmersionArr)], "%d,", gSt_DevMeasRecordData.cWater_Immersion_Status);
        sprintf((char *)&ucTempPhotosensitiveArr[strlen((char *)ucTempPhotosensitiveArr)], "%d,", gSt_DevMeasRecordData.cPhotosensitive_XYC_ALS_Status);
    }

    if(ucWaterLevel_Radar_Flag == 1)
    {
        //去掉最后一个逗号
        memcpy(&ucTempValueArr[0][strlen((char *)ucTempValueArr[0])-1], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[0]);
    }
    #ifndef WATERLEVEL_RADAR_PRESS
    if(ucWaterLevel_Pressure_Flag == 1)
    {
        //去掉最后一个逗号
        memcpy(&ucTempValueArr[1][strlen((char *)ucTempValueArr[1])-1], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[1]);
    }
    #endif
    if(ucWaterQuality_COD_Flag == 1)
    {
        //去掉最后一个逗号
        memcpy(&ucTempValueArr[2][strlen((char *)ucTempValueArr[2])-1], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[2]);
    }
    if(ucWaterQuality_COND_Flag == 1)
    {
        //去掉最后一个逗号
        memcpy(&ucTempValueArr[3][strlen((char *)ucTempValueArr[3])-1], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[3]);
    }
    if(ucMeasWaterVolumeFlag == 1)
    {
        //去掉最后一个逗号
        memcpy(&ucTempValueArr[4][strlen((char *)ucTempValueArr[4])-1], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[4]);
        memcpy(&ucTempValueArr[5][strlen((char *)ucTempValueArr[5])-1], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[5]);
    }
    memcpy(&ucTempAttiArr[strlen((char *)ucTempAttiArr)-1], "],", 2);
    (void)strcat((char *)ucTempArr, (char *)ucTempAttiArr);
    memcpy(&ucTempWaterImmersionArr[strlen((char *)ucTempWaterImmersionArr)-1], "],", 2);
    (void)strcat((char *)ucTempArr, (char *)ucTempWaterImmersionArr);
    memcpy(&ucTempPhotosensitiveArr[strlen((char *)ucTempPhotosensitiveArr)-1], "]}}", 3);
    (void)strcat((char *)ucTempArr, (char *)ucTempPhotosensitiveArr);
    
    usDataLen = strlen((char *)ucTempArr);

    memcpy(ucDataArr, ucTempArr, usDataLen);
    return usDataLen;                          
}

void func_Get_DevStatusData(void)
{
    time_t now;
    struct tm tm;
    //time(&now);
    drv_mcu_Get_RTC_Time(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime);
    sscanf(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    tm.tm_year -= 1900; // 由于tm_year是从1900年开始计数的
    tm.tm_mon -= 1;     // tm_mon是从0开始的，所以需要减1
    tm.tm_isdst = -1;
    now = mktime(&tm) - 8*60*60; 
    if(strlen(pst_EC200USystemPara->DeviceRunPara.esDeviceRunState.cDevStartDateTime) > 1)
    {
        if(pst_EC200USystemPara->DeviceRunPara.ulUploadStatusLostCnt == 0)
        {
            pst_EC200USystemPara->DeviceRunPara.ulUploadStatusStartTime = (long)now; //记录上传数据开始时间
        }
        pst_EC200USystemPara->DeviceRunPara.ulUploadStatusLostCnt++;
    }
    //记录此次设备状态数据
    memcpy(pst_EC200USystemPara->DeviceRunPara.esLostStatusArr[pst_EC200USystemPara->DeviceRunPara.cLostStatusArrCnt].cDevStartDateTime,pst_EC200USystemPara->DeviceRunPara.esDeviceRunState.cDevStartDateTime,20);
    pst_EC200USystemPara->DeviceRunPara.esLostStatusArr[pst_EC200USystemPara->DeviceRunPara.cLostStatusArrCnt].nDevStartDays = pst_EC200USystemPara->DeviceRunPara.esDeviceRunState.nDevStartDays;
    pst_EC200USystemPara->DeviceRunPara.esLostStatusArr[pst_EC200USystemPara->DeviceRunPara.cLostStatusArrCnt].fBattleVoltage = pst_EC200USystemPara->DeviceRunPara.esDeviceRunState.fBattleVoltage;
    pst_EC200USystemPara->DeviceRunPara.esLostStatusArr[pst_EC200USystemPara->DeviceRunPara.cLostStatusArrCnt].fDevLoca_lng = pst_EC200USystemPara->DeviceRunPara.esDeviceRunState.fDevLoca_lng;
    pst_EC200USystemPara->DeviceRunPara.esLostStatusArr[pst_EC200USystemPara->DeviceRunPara.cLostStatusArrCnt].fDevLoca_lat = pst_EC200USystemPara->DeviceRunPara.esDeviceRunState.fDevLoca_lat;
    pst_EC200USystemPara->DeviceRunPara.esLostStatusArr[pst_EC200USystemPara->DeviceRunPara.cLostStatusArrCnt].usDevStatus = pst_EC200USystemPara->DeviceRunPara.usDevStatus;
    pst_EC200USystemPara->DeviceRunPara.cLostStatusArrCnt++;
    if(pst_EC200USystemPara->DeviceRunPara.cLostStatusArrCnt >= 10) //最多记录10条状态数据
    {
        pst_EC200USystemPara->DeviceRunPara.cLostStatusArrCnt = 0; //超过最大记录数，清零
    }

}

//获取设备状态上报报文
uint16_t func_Get_DevStatusCMD_Data(uint8_t *ucDataArr)
{
    uint16_t usDataLen = 0;
    uint8_t ucTempArr[800] = {0};
    uint8_t ucTempValueArr[6][150] = {{0}}; //用于存储数据
    char nPosi = 0;
    long lValue = pst_EC200USystemPara->DeviceRunPara.cDevUploadStatusTime * 60 * 60;

    time_t now;
    struct tm tm;
    //time(&now);
    drv_mcu_Get_RTC_Time(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime);
    sscanf(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    tm.tm_year -= 1900; // 由于tm_year是从1900年开始计数的
    tm.tm_mon -= 1;     // tm_mon是从0开始的，所以需要减1
    tm.tm_isdst = -1;
    now = mktime(&tm) - 8*60*60; 

    //time(&now);
    sprintf((char *)ucTempArr, "{\"clientId\":\"%s\",\"pver\":24,\"beginTime\":%ld,\"msgId\":%ld,\"gap\":%ld,\"data\":{", 
                                                                                        pst_EC200USystemPara->DevicePara.cDeviceID,
                                                                                        (long)pst_EC200USystemPara->DeviceRunPara.ulUploadStatusStartTime,
                                                                                        (long)now,
                                                                                        (long)lValue//86400
                                                                                        );

    //pst_EC200USystemPara->DeviceRunPara.ucCurUploadStatusCnt = pst_EC200USystemPara->DeviceRunPara.ulUploadStatusLostCnt; //记录本次上传数据个数
    memcpy(&ucTempValueArr[0], "\"timeStart\":[\"", 14);
    memcpy(&ucTempValueArr[1], "\"timeRun\":[", 11);
    memcpy(&ucTempValueArr[2], "\"batV\":[", 8);
    memcpy(&ucTempValueArr[3], "\"lng\":[", 7);
    memcpy(&ucTempValueArr[4], "\"lat\":[", 7);
    memcpy(&ucTempValueArr[5], "\"status\":[", 10);
    for(nPosi=0; nPosi<pst_EC200USystemPara->DeviceRunPara.ulUploadStatusLostCnt; nPosi++)
    {
        sprintf((char *)&ucTempValueArr[0][strlen((char *)ucTempValueArr[0])], "%s\",\"", pst_EC200USystemPara->DeviceRunPara.esLostStatusArr[nPosi].cDevStartDateTime);
        sprintf((char *)&ucTempValueArr[1][strlen((char *)ucTempValueArr[1])], "%d,", pst_EC200USystemPara->DeviceRunPara.esLostStatusArr[nPosi].nDevStartDays);
        sprintf((char *)&ucTempValueArr[2][strlen((char *)ucTempValueArr[2])], "%.3lf,", (double)pst_EC200USystemPara->DeviceRunPara.esLostStatusArr[nPosi].fBattleVoltage);
        sprintf((char *)&ucTempValueArr[3][strlen((char *)ucTempValueArr[3])], "%.5lf,", (double)pst_EC200USystemPara->DeviceRunPara.esLostStatusArr[nPosi].fDevLoca_lng);
        sprintf((char *)&ucTempValueArr[4][strlen((char *)ucTempValueArr[4])], "%.5lf,", (double)pst_EC200USystemPara->DeviceRunPara.esLostStatusArr[nPosi].fDevLoca_lat);
        sprintf((char *)&ucTempValueArr[5][strlen((char *)ucTempValueArr[5])], "%2x,", pst_EC200USystemPara->DeviceRunPara.esLostStatusArr[nPosi].usDevStatus);
    }
    //去掉最后一个逗号
    memcpy(&ucTempValueArr[0][strlen((char *)ucTempValueArr[0])-2], "],", 2);
    (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[0]);
    memcpy(&ucTempValueArr[1][strlen((char *)ucTempValueArr[1])-1], "],", 2);
    (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[1]);
    memcpy(&ucTempValueArr[2][strlen((char *)ucTempValueArr[2])-1], "],", 2);
    (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[2]);
    memcpy(&ucTempValueArr[3][strlen((char *)ucTempValueArr[3])-1], "],", 2);
    (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[3]);
    memcpy(&ucTempValueArr[4][strlen((char *)ucTempValueArr[4])-1], "],", 2);
    (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[4]);
    memcpy(&ucTempValueArr[5][strlen((char *)ucTempValueArr[5])-1], "]}}", 3);
    (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[5]);

    usDataLen = strlen((char *)ucTempArr);

    memcpy(ucDataArr, ucTempArr, usDataLen);
    return usDataLen;          
}

/**
 * @brief  4G EC200U Module AT CMD Initialize.
 * @param  ucDataUploadEnable: 0: disable; 1: enable
 * @retval  0: 流程执行结束 1: 本次阶段完成 2: 通信超时; 4:执行的是等待时间命令，无串口通信
 */
uint8_t EC200U_4G_Module_Configuration_Init(unsigned char ucDataUploadEnable)
{
    //int32_t i32Ret = 0;
    //int32_t i32Ret1 = 0;
    uint8_t u8Temp = 1;
    uint16_t usSendDataLen = 0;
    uint8_t ucRecvCheckData[50] = {0};
    uint8_t ucSendBuf[EC200U_BUF_SIZE] = {0};
    //uint8_t ucRecvBuf[EC200U_BUF_SIZE] = {0};
    uint16_t usRecvTimeOutCnt = 0;
    static uint8_t ucRetryCnt = 0;
    //uint8_t ucFlag = 0;
    uint16_t usRecvLen = 0;
    unsigned short usPosition = 0;

    if((ucDataUploadEnable == 0) )
    {
        u8Temp = 1;
    }
    else
    {
        u8Temp = 0; //如果是数据上传阶段，则不需要等待
    }

    do
    {
        if((gE_4G_Module_Init_CMD >= Module_TEST_AT_CMD) || (gE_4G_Module_Init_CMD <= Module_DCE_RST_STAGE2))
        {
            memset(ucSendBuf, 0, EC200U_BUF_SIZE);
            //memset(ucRecvBuf, 0, EC200U_BUF_SIZE);
            memset(ucRecvCheckData, 0, 50);
            pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB] = 0;
            memset(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], 0, USART_DATA_LEN_MAX);
        }
        
        switch (gE_4G_Module_Init_CMD)
        {
        case Module_START_WAIT_CMD: //
            EC200U_4G_Module_GPIO_Init();
            //拉低4G模块电源引脚2s以上，让4G模块开机
            DCERST_PIN_RESET;
            break;
        case Module_DCE_RST_STAGE1:
            //等待1s
            break;
        case Module_DCE_RST_STAGE2: //拉低4G模块电源引脚2s以上，让4G模块开机
            DCERST_PIN_SET;
            break;
        case Module_DCE_RST_STAGE3:
            sprintf((char *)ucRecvCheckData, "RDY");
            break;
        case Module_TEST_AT_CMD: //测试AT指令
            (void)strcpy((char *)ucSendBuf, "AT\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_TEST_ATE0_CMD: //测试AT指令
            (void)strcpy((char *)ucSendBuf, "ATE0\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_QUERY_SIM_CARD_STATE_CMD: //查询SIM卡状态
            (void)strcpy((char *)ucSendBuf, "AT+CPIN?\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "+CPIN: READY");
            break;
        case Module_QUERY_SIGNAL_STRENGTH_CMD: //查询信号强度
            (void)strcpy((char *)ucSendBuf, "AT+CSQ\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "+CSQ:");
            break;
        #if 0
        case Module_QICSGP_CMD: //设置PDP上下文
            //(void)strcpy((char *)ucSendBuf, "AT+QICSGP=1,\"CMNET\",\"\",\"\",1\r\n");
            //(void)strcpy((char *)ucSendBuf, "AT+QICSGP=1,\"CMNET\",\"\",\"\",0\r\n");
            (void)strcpy((char *)ucSendBuf, "AT+QICSGP=2,3,\"CMNET\",\"\",\"\",1\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_QIACT_CMD:  //建立PDP上下文
            //(void)strcpy((char *)ucSendBuf, "AT+QIACT=1\r\n");
            (void)strcpy((char *)ucSendBuf, "AT+QIACT=2\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_PDP_TEST_CMD:
            (void)strcpy((char *)ucSendBuf, "AT+QICSGP=2\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        #endif
        case Module_QUERY_PS_DOMAIN_REG_STATE_CMD://查询PS域注册状态：0：未注册，1/5：注册，2：正在搜索
            //(void)strcpy((char *)ucSendBuf, "AT+CREG?\r\n");    //CS域，只使用在2G网络上
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_Register;
            (void)strcpy((char *)ucSendBuf, "AT+CGREG?\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "+CGREG: 0,1");
            break;
        case Module_ACTIVATE_NETWORK_CMD: //激活网络
            (void)strcpy((char *)ucSendBuf, "AT+CGATT=1\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_QUERY_NETWORK_ACTIVATE_STATE_CMD: //查询网络激活状态
            (void)strcpy((char *)ucSendBuf, "AT+CGATT?\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "+CGATT: 1");
            break;
        
        case Module_QUERY_IMSI_CMD: //查询IMSI号
            (void)strcpy((char *)ucSendBuf, "AT+CIMI\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_QUERY_IMEI_CMD: //查询IMEI号
            (void)strcpy((char *)ucSendBuf, "AT+CGSN\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_QUERY_LOCAL_DATE_TIME_CMD: //查询本地日期时间
            (void)strcpy((char *)ucSendBuf, "AT+QLTS=2\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            func_WatchDog_Refresh();
            break;
        //MQTT配置  
        case Module_SET_DATA_FORMAT_CMD: //设置数据格式
            (void)strcpy((char *)ucSendBuf, "AT+QMTCFG=\"recv/mode\",0,0,1\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_SET_MQTT_KEEPALIVE_TIME_CMD:    //心跳时间建议60s~300s.这里设置120s
            (void)strcpy((char *)ucSendBuf, "AT+QMTCFG=\"keepalive\",0,120\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_SET_MQTT_VERSION_CMD:   //设置MQTT 版本
            (void)strcpy((char *)ucSendBuf, "AT+QMTCFG=\"version\",0,4\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
            #if 0
        case Module_Check_MQTT_OPEN_CMD1: //检查MQTT是否打开
            (void)strcpy((char *)ucSendBuf, "AT+QMTOPEN?\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
            #endif
        case Module_OPEN_MQTT_INTERFACE_CMD: //打开物联网云端口
            //(void)strcpy((char *)ucSendBuf, "AT+QMTOPEN=0,\"studio-mqtt.heclouds.com\",1883\r\n");
            //(void)strcpy((char *)ucSendBuf, "AT+QMTOPEN=0,\"218.85.5.161\",7243\r\n");
            //(void)strcpy((char *)ucSendBuf, "AT+QMTOPEN=0,\"220.250.29.188\",7183\r\n");
            (void)sprintf((char *)ucSendBuf, "AT+QMTOPEN=0,\"%s\",%d\r\n",pst_EC200USystemPara->DevicePara.cServerIP[1], pst_EC200USystemPara->DevicePara.usServerPort[1]);
            //(void)strcpy((char *)ucSendBuf, "AT+QMTCONN=0,\"ZCJ2025042801\",\"iVOw212I78\",\"version=2018-10-31&res=products\%2FiVOw212I78\%2Fdevices\%2FZCJ2025042801&et=1749953701&method=md5&sign=Ojs6ZSs5SoXw1Ckzurdsjw\%3D\%3D\"\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "QMTOPEN");
            break;
            #if 0
        case Module_Check_MQTT_OPEN_CMD2: //检查MQTT是否打开
            (void)strcpy((char *)ucSendBuf, "AT+QMTOPEN?\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
            
        case Module_Check_MQTT_CONN_CMD1:   //检查MQTT连接状态
            (void)strcpy((char *)ucSendBuf, "AT+QMTCONN?\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
            #endif
        case Module_CONN_MQTT_INTERFACE_CMD: //连接物联网云端口
            //(void)strcpy((char *)ucSendBuf, "AT+QMTCONN=0,\"clientid\",\"username\",\"userpwd\"\r\n");
            //(void)strcpy((char *)ucSendBuf, "AT+MIPSTART=\"studio-mqtt.heclouds.com\",1883\r\n");
            //(void)strcpy((char *)ucSendBuf, "AT+QMTCONN=0,\"ZCJ2025042802\",\"jP1B7MpRy3\",\"version=2018-10-31&res=products\%2FjP1B7MpRy3\%2Fdevices\%2FZCJ2025042802&et=1749953701&method=md5&sign=UYUYifYvoXYzT5R7vS3Wbw\%3D\%3D\"\r\n");
            //(void)sprintf((char *)ucSendBuf, "AT+QMTCONN=0,\"%s\",\"xfgd\",\"xfgd@1234\"\r\n",pst_EC200USystemPara->DevicePara.cDeviceID);
            (void)sprintf((char *)ucSendBuf, "AT+QMTCONN=0,\"%s\",\"nbwater\",\"nbwater@1234\"\r\n",pst_EC200USystemPara->DevicePara.cDeviceID);
            //(void)strcpy((char *)ucSendBuf, "AT+QMTCONN=0,\"ZCJ2025042801\",\"iVOw212I78\",\"version=2018-10-31&res=products\%2FiVOw212I78\%2Fdevices\%2FZCJ2025042801&et=1749953701&method=md5&sign=FXcY9ZA%2BJhyld8Bd3VrpQg\%3D\%3D\"\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        #ifdef CONNECT_MQTT
        case Module_SUBSCRIBE_TOPIC_REGISTER_CMD:   //订阅主题注册
            sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0100/0004/devReg/%s\",2\r\n",pst_EC200USystemPara->DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_SUBSCRIBE_TOPIC_DATAUPLOAD_CMD: //订阅主题数据上传
            sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0100/0004/dataUpload/%s\",2\r\n",pst_EC200USystemPara->DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_SUBSCRIBE_TOPIC_STATUS_CMD: //订阅主题状态上报
            sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0100/0004/devStatus/%s\",2\r\n",pst_EC200USystemPara->DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        #if 0
        case Module_SUBSCRIBE_TOPIC_ALARMDATA_CMD: //订阅主题报警数据上报
            sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0100/0004/alarmData/%s\",2\r\n",pst_EC200USystemPara->DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        #endif
        case Module_SUBSCRIBE_TOPIC_SETCONFIG_CMD:  //订阅主题设置配置
            sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0100/0004/setConfig/%s\",2\r\n",pst_EC200USystemPara->DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_SUBSCRIBE_TOPIC_GETCONFIG_CMD:  //订阅主题获取配置
            sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0100/0004/getConfig/%s\",2\r\n",pst_EC200USystemPara->DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        #if 0
        case Module_SUBSCRIBE_TOPIC_GETDATA_CMD:    //订阅主题获取数据
            sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0100/0004/getData/%s\",2\r\n",pst_EC200USystemPara->DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
       
        case Module_SUBSCRIBE_DATAPT_CMD: //订阅主题数据透传
            sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0100/0004/dataPt/%s\",2\r\n",pst_EC200USystemPara->DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;

        #endif
        case Module_PUBLISH_TOPIC_REGISTER_CMD: //发布主题注册
            usSendDataLen = func_Get_DevRegCMD_Data(ucSendBuf);
            sprintf((char *)ucSendBuf, "AT+QMTPUBEX=0,0,0,0,\"data/up/0100/0004/devReg/%s\",%d\r\n",pst_EC200USystemPara->DevicePara.cDeviceID,usSendDataLen);
            usSendDataLen = strlen((char *)ucSendBuf);
            //pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_Upload;
            drv_mcu_USART_SendData(MODULE_4G_NB, ucSendBuf, usSendDataLen);
            Ddl_Delay1ms(400);
            pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB] = 0;
            memset(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], 0, USART_DATA_LEN_MAX);
            memset(ucSendBuf, 0, EC200U_BUF_SIZE);
            usSendDataLen = func_Get_DevRegCMD_Data(ucSendBuf);
            memcpy(ucSendBuf+usSendDataLen, "\r\n", 2);
            usSendDataLen += 2;
            sprintf((char *)ucRecvCheckData, "\"res\":");
            break;
        case Module_PUBLISH_TOPIC_DATAUPLOAD_CMD:   //发布监测项数据上报主题 
            usSendDataLen = func_Get_DataUploadCMD_Data(ucSendBuf);
            sprintf((char *)ucSendBuf, "AT+QMTPUBEX=0,0,0,0,\"data/up/0100/0004/dataUpload/%s\",%d\r\n",pst_EC200USystemPara->DevicePara.cDeviceID,usSendDataLen);
            usSendDataLen = strlen((char *)ucSendBuf);
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_Upload;
            drv_mcu_USART_SendData(MODULE_4G_NB, ucSendBuf, usSendDataLen);
            Ddl_Delay1ms(400);
            pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB] = 0;
            memset(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], 0, USART_DATA_LEN_MAX);
            memset(ucSendBuf, 0, EC200U_BUF_SIZE);
            usSendDataLen = func_Get_DataUploadCMD_Data(ucSendBuf);
            memcpy(ucSendBuf+usSendDataLen, "\r\n", 2);
            usSendDataLen += 2;
            sprintf((char *)ucRecvCheckData, "\"res\":");
            break;
        case Module_PUBLISH_TOPIC_STATUS_CMD:   //发布设备状态上报主题
            usSendDataLen = func_Get_DevStatusCMD_Data(ucSendBuf);
            sprintf((char *)ucSendBuf, "AT+QMTPUBEX=0,0,0,0,\"data/up/0100/0004/devStatus/%s\",%d\r\n",pst_EC200USystemPara->DevicePara.cDeviceID,usSendDataLen);
            usSendDataLen = strlen((char *)ucSendBuf);
            //pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_Upload;
            drv_mcu_USART_SendData(MODULE_4G_NB, ucSendBuf, usSendDataLen);
            Ddl_Delay1ms(400);
            pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB] = 0;
            memset(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], 0, USART_DATA_LEN_MAX);
            memset(ucSendBuf, 0, EC200U_BUF_SIZE);
            usSendDataLen = func_Get_DevStatusCMD_Data(ucSendBuf);
            memcpy(ucSendBuf+usSendDataLen, "\r\n", 2);
            usSendDataLen += 2;
            sprintf((char *)ucRecvCheckData, "\"res\":");
            break;
        #endif
        #if 0
        case Module_PUBLISH_TOPIC_CMD: //发布物联网云端口:发布主题为GetVersionUpdateFlag，QoS为2，消息体为Get
            //(void)strcpy((char *)ucSendBuf, "AT+QMTPUBEX=0,1,2,0,\"GetVersionUpdateFlag\",3\r\n");
            usSendDataLen = func_get_onenet_test_value(ucSendBuf,0);
            memset(ucSendBuf, 0, EC200U_BUF_SIZE);
            sprintf((char *)ucSendBuf, "AT+QMTPUBEX=0,0,0,0,\"$sys/jP1B7MpRy3/ZCJ2025042802/thing/property/post\",%d\r\n",usSendDataLen);
            //sprintf((char *)ucSendBuf, "AT+QMTPUBEX=0,0,0,0,\"$sys/iVOw212I78/ZCJ2025042801/thing/property/post\",%d\r\n",usSendDataLen);
            usSendDataLen = strlen((char *)ucSendBuf);
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_Upload;
            drv_mcu_USART_SendData(MODULE_4G_NB, ucSendBuf, usSendDataLen);
            Ddl_Delay1ms(400);
            pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB] = 0;
            memset(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], 0, USART_DATA_LEN_MAX);
            memset(ucSendBuf, 0, EC200U_BUF_SIZE);
            usSendDataLen = func_get_onenet_test_value(ucSendBuf,1);
            //strcat(ucSendBuf, "\r\n");
            memcpy(ucSendBuf+usSendDataLen, "\r\n", 2);
            usSendDataLen += 2;
            //sprintf((char*)&ucSendBuf[0],"%s\r\n","{\"type\":\"variant_data\",\"version\":\"1.0\",\"time\":1638766638000,\"params\":{\"UAV\":220.5}}");
            //memset(ucSendBuf, 0, EC200U_BUF_SIZE);
            //(void)strcpy((char *)ucSendBuf, "Get");
            //usSendDataLen = strlen((char *)ucSendBuf);

            //drv_mcu_USART_SendData(MODULE_4G_NB, ucSendBuf, usSendDataLen);
            //Ddl_Delay1ms(8000);
            //guc_4GInitCnt++;
            sprintf((char *)ucRecvCheckData, "QMTPUBEX");
            break;
        case Module_CLOSE_CONN_CMD: //关闭MQTT客户端网络
            (void)strcpy((char *)ucSendBuf, "AT+QMTCLOSE=0\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_DISCONNECT_MQTT_INTERFACE_CMD: //断开MQTT服务器
            (void)strcpy((char *)ucSendBuf, "AT+QMTDISC=0\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_TUNS_TOPIC_CMD: //退订主题
            (void)strcpy((char *)ucSendBuf, "AT+QMTUNS=0,1,\"topic\"\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        #endif
        default:
            break;
        }

        if(gE_4G_Module_Init_CMD <= Module_DCE_RST_STAGE2)
        {
            gE_4G_Module_Init_CMD++;
            Ddl_Delay1ms(1000); //等待1s
            return 4;
        }

        if(gE_4G_Module_Init_CMD >= Module_TEST_AT_CMD)
        {
            //发送AT指令
            drv_mcu_USART_SendData(MODULE_4G_NB, ucSendBuf, usSendDataLen);
        }
        
        //等待接收到OK
        usRecvTimeOutCnt = 0;
        while(pst_EC200USystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_4G_NB] == 0)
        {
            Ddl_Delay1ms(20);
            usRecvTimeOutCnt++;
            if(usRecvTimeOutCnt >= 300)
            {
                if(strlen(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB]) > 0)
                {
                    //ucFlag = 1;
                    //如果接收到数据，但没有OK，则继续等待
                    if(func_Array_Find_Str((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],strlen(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB]),(char *)ucRecvCheckData,strlen((char*)ucRecvCheckData), &usPosition) == 0)
                    {
                        //接收到数据，但没有OK
                        //u8Temp = 1;
                        //ucFlag = 2;
                        break;
                    }
                    //else
                    //{
                        //ucFlag = 3;
                    //}
                }
                //else
                //{
                //    ucFlag = 4;
                //}
            }
            if(usRecvTimeOutCnt >= 1000)
            {
                u8Temp = 0;
                return 2;
            }
        }
        pst_EC200USystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_4G_NB] = 0;
        
        if(pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[0] == 0)
        {
            usRecvLen = strlen(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB]);
        }
        else
        {
            usRecvLen = pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[0];
        }
        if(usRecvLen == 0)
        {
            usRecvLen = 0;
        }
        //if (strstr((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], (char *)ucRecvCheckData) != NULL) //接收到的数据中包含OK
        if(func_Array_Find_Str((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],usRecvLen,(char *)ucRecvCheckData,strlen((char*)ucRecvCheckData), &usPosition) == 0) //接收到的数据中包含OK
        {
            if(gE_4G_Module_Init_CMD == Module_QUERY_IMSI_CMD)
            {
                memset(&pst_EC200USystemPara->DevicePara.cDeviceIMSI[0], 0, 16);
                memcpy(&pst_EC200USystemPara->DevicePara.cDeviceIMSI[0], &pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB][2], 15);
            }
            else if(gE_4G_Module_Init_CMD == Module_QUERY_IMEI_CMD)
            {
                memset(&pst_EC200USystemPara->DevicePara.cDeviceIMEI[0], 0, 16);
                memcpy(&pst_EC200USystemPara->DevicePara.cDeviceIMEI[0], &pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB][2], 15);
            }
            else if(gE_4G_Module_Init_CMD == Module_QUERY_LOCAL_DATE_TIME_CMD)
            {
                if(drv_mcu_Set_RTC_Time(&pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB][0]) == 1)
                {
                    return 5;
                }
                if((ucDataUploadEnable == 0) && (pst_EC200USystemPara->DevicePara.cDeviceRegisterFlag == 1))
                {
                    return 0;
                }
            }
            else
            {
                //gE_4G_Module_Init_CMD++;
            }
            #ifdef CONNECT_MQTT
            if(gE_4G_Module_Init_CMD == Module_SUBSCRIBE_TOPIC_GETCONFIG_CMD)
            {
                if(pst_EC200USystemPara->DevicePara.cDeviceRegisterFlag == 0)   //当前设备未注册
                {
                    gE_4G_Module_Init_CMD = Module_PUBLISH_TOPIC_REGISTER_CMD;
                }
                else
                {
                    if(ucDataUploadEnable == 1)
                    {
                        gE_4G_Module_Init_CMD = Module_PUBLISH_TOPIC_DATAUPLOAD_CMD;
                    }
                    else
                    {
                        return 0;
                    }
                }
            }
            else if(gE_4G_Module_Init_CMD == Module_PUBLISH_TOPIC_REGISTER_CMD)
            {
                //if (strstr((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], "\"res\":0") != NULL)
                if(func_Array_Find_Str((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],usRecvLen,"\"res\":0",7, &usPosition) == 0)
                {
                    //注册成功
                    pst_EC200USystemPara->DevicePara.cDeviceRegisterFlag = 1; //设置设备注册标志位
                    func_Save_Device_Parameter(DEV_REG_FLAG, (unsigned char*)&pst_EC200USystemPara->DevicePara.cDeviceRegisterFlag);
                    if(ucDataUploadEnable == 1)
                    {   
                        gE_4G_Module_Init_CMD = Module_PUBLISH_TOPIC_DATAUPLOAD_CMD;
                    }
                    else
                    {
                        return 0; //注册成功，且不需要上传数据
                    }
                }
            }
            else if(gE_4G_Module_Init_CMD == Module_PUBLISH_TOPIC_DATAUPLOAD_CMD)
            {
                //if (strstr((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], "\"res\":0") != NULL)
                if(func_Array_Find_Str((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],usRecvLen,"\"res\":0",7, &usPosition) == 0)
                {
                    if(pst_EC200USystemPara->DeviceRunPara.cBarTouchFlag != 3)
                    {
                        //上传数据成功
                        if(pst_EC200USystemPara->DeviceRunPara.ucCurUploadRecordCnt == (pst_EC200USystemPara->DevicePara.nDeviceUploadCnt / pst_EC200USystemPara->DevicePara.nDeviceSaveRecordCnt))
                        {
                            pst_EC200USystemPara->DeviceRunPara.ulUploadRecordLostCnt = 0; //上传数据成功，清零上传记录丢失计数
                            pst_EC200USystemPara->DeviceRunPara.ulUploadRecordStartTime = 0;
                        }
                        else
                        {
                            if((pst_EC200USystemPara->DeviceRunPara.ulUploadRecordLostCnt + pst_EC200USystemPara->DevicePara.nDeviceUploadCnt / pst_EC200USystemPara->DevicePara.nDeviceSaveRecordCnt) >= pst_EC200USystemPara->DeviceRunPara.ucCurUploadRecordCnt)
                            {
                                pst_EC200USystemPara->DeviceRunPara.ulUploadRecordLostCnt -= (pst_EC200USystemPara->DeviceRunPara.ucCurUploadRecordCnt);
                                if(pst_EC200USystemPara->DeviceRunPara.ulUploadRecordLostCnt == 0)
                                {
                                    pst_EC200USystemPara->DeviceRunPara.ulUploadRecordStartTime = 0;
                                }
                                else
                                {
                                    pst_EC200USystemPara->DeviceRunPara.ulUploadRecordStartTime += pst_EC200USystemPara->DeviceRunPara.ucCurUploadRecordCnt * pst_EC200USystemPara->DevicePara.nDeviceSaveRecordCnt * 60;
                                }
                            }
                            else
                            {
                                pst_EC200USystemPara->DeviceRunPara.ulUploadRecordLostCnt = 0; //上传数据成功，清零上传记录丢失计数
                                pst_EC200USystemPara->DeviceRunPara.ulUploadRecordStartTime = 0;
                            }
                        }
                    }
                }
                else
                {
                    //pst_EC200USystemPara->DeviceRunPara.ulUploadRecordLostCnt += pst_EC200USystemPara->DevicePara.nDeviceUploadCnt / pst_EC200USystemPara->DevicePara.nDeviceSaveRecordCnt; //上传数据失败，上传记录丢失计数+1
                }

                if((pst_EC200USystemPara->DeviceRunPara.cDeviceStatusUploadFlag == 1) && (pst_EC200USystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ucDataValidFlag == 1))
                //if(pst_EC200USystemPara->DeviceRunPara.cDeviceStatusUploadFlag == 1)
                {
                    func_Get_DevStatusData();
                    gE_4G_Module_Init_CMD = Module_PUBLISH_TOPIC_STATUS_CMD; //如果需要上报设备状态，则继续执行
                }
                else
                {
                    return 0;
                }
            }
            else if(gE_4G_Module_Init_CMD == Module_PUBLISH_TOPIC_STATUS_CMD)
            {
                if(func_Array_Find_Str((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],usRecvLen,"\"res\":0",7, &usPosition) == 0)
                {
                    pst_EC200USystemPara->DeviceRunPara.cDeviceStatusUploadFlag = 0; //上报设备状态成功后，清除设备状态上报标志位
                    //上传数据成功
                    pst_EC200USystemPara->DeviceRunPara.ulUploadStatusLostCnt = 0; //上传数据成功，清零上传记录丢失计数
                    pst_EC200USystemPara->DeviceRunPara.ulUploadStatusStartTime = 0;
                    pst_EC200USystemPara->DeviceRunPara.cLostStatusArrCnt = 0; //清除设备状态上报丢失计数
                }
                return 0;
            }
            else
            {
                gE_4G_Module_Init_CMD++;
                ucRetryCnt = 0;
            }
            #else
            if(gE_4G_Module_Init_CMD == Module_PUBLISH_TOPIC_CMD)
            {
                return 0;
            }
            #endif
            
        }
        else
        {
            //考虑上述case 4的情况，如果查询到的不是1，那么需要继续查询是否为5，1表示注册本地网;5表示注册漫游网
            if (gE_4G_Module_Init_CMD == Module_QUERY_PS_DOMAIN_REG_STATE_CMD)
            {
                //if (strstr((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], "+CGREG: 0,5") == NULL)
                if(func_Array_Find_Str((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[0],"+CGREG:0,5",10, &usPosition) == 0)
                {
                    u8Temp = 0;
                    return 2;
                }
                else
                {
                    gE_4G_Module_Init_CMD++;
                }
            }
            else
            {
                //gE_4G_Module_Init_CMD++;
                
                Ddl_Delay1ms(200);
                if(ucRetryCnt >= 5) //重复发送三次
                {
                    ucRetryCnt = 0;
                    u8Temp = 0;
                    if(gE_4G_Module_Init_CMD < Module_SUBSCRIBE_TOPIC_GETCONFIG_CMD)
                    {
                        return 2;
                    }
                    else
                    {
                        return 0;
                    }
                }
                ucRetryCnt++;
            }
        }
    }while(u8Temp == 1);
    return 1;
}

/**
 * @brief  4G EC200U Module Init.
 * @param  ucDataUploadEnable: 0: disable; 1: enable
 * @retval  0: module init success  1: module init failed
 */
uint8_t drv_EC200U_4G_Module_Init(unsigned char ucDataUploadEnable)
{
    uint8_t ucResult = 0;
    uint16_t usRecvTimeOutCnt = 0;
    static uint8_t ucFailedCnt = 0;
    static uint8_t ucInitFlag = 0;
    if(ucInitFlag == 0)
    {
        ucInitFlag = 1;
        pst_EC200USystemPara = GetSystemPara();
    }

    if(ucDataUploadEnable == 1)
    {
        ucResult = EC200U_4G_Module_Configuration_Init(ucDataUploadEnable);
        if((ucResult == 0) || (ucResult == 2))
        {
            ucFailedCnt = 0;
            gE_4G_Module_Init_CMD = Module_START_WAIT_CMD;
        }
        else if(ucResult == 5)
        {
            ucFailedCnt++;
            if(ucFailedCnt >= 3)
            {
                ucFailedCnt = 0;
                ucResult = 2;
                gE_4G_Module_Init_CMD = Module_START_WAIT_CMD;
            }
        }
    }
    else
    {
        EC200U_4G_Module_GPIO_Init();
        func_WatchDog_Refresh();
        for (uint8_t i = 0; i < 30; i++)
        {
            //拉低4G模块电源引脚2s以上，让4G模块开机
            DCERST_PIN_RESET;
            //等待4G模块开机
            Ddl_Delay1ms(2000);
            DCERST_PIN_SET;
            //等待4G模块串口开始工作
            Ddl_Delay1ms(2000);
            func_WatchDog_Refresh();
            //等待模块启动成功主动传回"RDY\r\n"
            usRecvTimeOutCnt = 0;
            while(pst_EC200USystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_4G_NB] == 0)
            {
                Ddl_Delay1ms(10);
                usRecvTimeOutCnt++;
                if(usRecvTimeOutCnt >= 1000)
                {
                    //drv_mcu_USART_SendData(MODULE_4G_NB, "AT\r\n", 4);
                    ucResult = 1;
                    break;
                }
            }
            pst_EC200USystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_4G_NB] = 0;

            if(strstr((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], "RDY") != NULL)
            {
                //模块启动成功
                ucResult = 0;
                break;
            }
            else
            {
                //模块启动失败
                ucResult = 1;
            }
        }
        if(ucResult == 0)
        {
            gE_4G_Module_Init_CMD = Module_TEST_AT_CMD;
            pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB] = 0;
            memset(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], 0, USART_DATA_LEN_MAX);
            ucResult = EC200U_4G_Module_Configuration_Init(ucDataUploadEnable);
            gE_4G_Module_Init_CMD = Module_START_WAIT_CMD;
            if(ucResult == 0)
            {
                pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_OK;
            }
            memset(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], 0, USART_DATA_LEN_MAX);
        }
    }
    
    return ucResult;
}

uint8_t func_Upload_Sensor_Value_To_OneNet_Text(void)
{
    #if 0
    //uint8_t ucTempArr[300] = {0};
    //uint16_t usDataLen = 0;
    uint8_t ucSendBuf[EC200U_BUF_SIZE] = {0};
    uint16_t usSendDataLen = 0;
    uint8_t ucRecvCheckData[50] = {0};
    uint16_t usRecvTimeOutCnt = 0;
    //uint8_t u8Temp = 1;
    uint8_t ucRetryCnt = 0;

    gE_4G_Module_Init_CMD = Module_OPEN_MQTT_INTERFACE_CMD;

    while(1)
    {
        memset(ucSendBuf, 0, EC200U_BUF_SIZE);
        //memset(ucRecvBuf, 0, EC200U_BUF_SIZE);
        memset(ucRecvCheckData, 0, 50);
        pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB] = 0;
        memset(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], 0, USART_DATA_LEN_MAX);
        switch (gE_4G_Module_Init_CMD)
        {
        case Module_OPEN_MQTT_INTERFACE_CMD:
            (void)strcpy((char *)ucSendBuf, "AT+QMTOPEN=0,\"studio-mqtt.heclouds.com\",1883\r\n");
            //(void)strcpy((char *)ucSendBuf, "AT+QMTCONN=0,\"ZCJ2025042801\",\"iVOw212I78\",\"version=2018-10-31&res=products\%2FiVOw212I78\%2Fdevices\%2FZCJ2025042801&et=1749953701&method=md5&sign=Ojs6ZSs5SoXw1Ckzurdsjw\%3D\%3D\"\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_CONN_MQTT_INTERFACE_CMD: //连接物联网云端口
            //(void)strcpy((char *)ucSendBuf, "AT+QMTCONN=0,\"clientid\",\"username\",\"userpwd\"\r\n");
            //(void)strcpy((char *)ucSendBuf, "AT+MIPSTART=\"studio-mqtt.heclouds.com\",1883\r\n");
            (void)strcpy((char *)ucSendBuf, "AT+QMTCONN=0,\"ZCJ2025042802\",\"jP1B7MpRy3\",\"version=2018-10-31&res=products\%2FjP1B7MpRy3\%2Fdevices\%2FZCJ2025042802&et=1749953701&method=md5&sign=UYUYifYvoXYzT5R7vS3Wbw\%3D\%3D\"\r\n");

            //(void)strcpy((char *)ucSendBuf, "AT+QMTCONN=0,\"ZCJ2025042801\",\"iVOw212I78\",\"version=2018-10-31&res=products\%2FiVOw212I78\%2Fdevices\%2FZCJ2025042801&et=1749953701&method=md5&sign=FXcY9ZA%2BJhyld8Bd3VrpQg\%3D\%3D\"\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_PUBLISH_TOPIC_CMD: //发布物联网云端口:发布主题为GetVersionUpdateFlag，QoS为2，消息体为Get
            //(void)strcpy((char *)ucSendBuf, "AT+QMTPUBEX=0,1,2,0,\"GetVersionUpdateFlag\",3\r\n");
            usSendDataLen = func_get_onenet_test_value(ucSendBuf,1);
            memset(ucSendBuf, 0, EC200U_BUF_SIZE);
            sprintf((char *)ucSendBuf, "AT+QMTPUBEX=0,0,0,0,\"$sys/jP1B7MpRy3/ZCJ2025042802/thing/property/post\",%d\r\n",usSendDataLen);
            //sprintf((char *)ucSendBuf, "AT+QMTPUBEX=0,0,0,0,\"$sys/iVOw212I78/ZCJ2025042801/thing/property/post\",%d\r\n",usSendDataLen);
            usSendDataLen = strlen((char *)ucSendBuf);

            drv_mcu_USART_SendData(MODULE_4G_NB, ucSendBuf, usSendDataLen);
            Ddl_Delay1ms(400);
            pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB] = 0;
            memset(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], 0, USART_DATA_LEN_MAX);
            memset(ucSendBuf, 0, EC200U_BUF_SIZE);
            usSendDataLen = func_get_onenet_test_value(ucSendBuf,1);
            //strcat(ucSendBuf, "\r\n");
            memcpy(ucSendBuf+usSendDataLen, "\r\n", 2);
            usSendDataLen += 2;
            //sprintf((char*)&ucSendBuf[0],"%s\r\n","{\"type\":\"variant_data\",\"version\":\"1.0\",\"time\":1638766638000,\"params\":{\"UAV\":220.5}}");
            //memset(ucSendBuf, 0, EC200U_BUF_SIZE);
            //(void)strcpy((char *)ucSendBuf, "Get");
            //usSendDataLen = strlen((char *)ucSendBuf);

            //drv_mcu_USART_SendData(MODULE_4G_NB, ucSendBuf, usSendDataLen);
            //Ddl_Delay1ms(8000);
            //guc_4GInitCnt1++;
            sprintf((char *)ucRecvCheckData, "QMTPUBEX");
            //Ddl_Delay1ms(1000);
            break;
        
        case Module_DISCONNECT_MQTT_INTERFACE_CMD: //断开MQTT服务器
            (void)strcpy((char *)ucSendBuf, "AT+QMTDISC=0\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_CLOSE_CONN_CMD: //关闭MQTT客户端网络
            (void)strcpy((char *)ucSendBuf, "AT+QMTCLOSE=0\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        default:
            break;
        }
        pst_EC200USystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_4G_NB] = 0;
        drv_mcu_USART_SendData(MODULE_4G_NB, ucSendBuf, usSendDataLen);

        //等待接收到OK
        usRecvTimeOutCnt = 0;
        while((pst_EC200USystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_4G_NB] == 0))
        {
            Ddl_Delay1ms(20);
            usRecvTimeOutCnt++;
            if(usRecvTimeOutCnt >= 500)
            {
                return 2;
            }
        }
        if(pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[0] > 0)
        {
            usRecvTimeOutCnt = 0;
            while(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[0]-1] == 0x00)
            {
                Ddl_Delay1ms(20);
                usRecvTimeOutCnt++;
                if(usRecvTimeOutCnt >= 500)
                {
                    return 2;
                }
            }
        }
        pst_EC200USystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_4G_NB] = 0;
        
        //if (strstr((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], (char *)ucRecvCheckData) != NULL) //接收到的数据中包含OK
        if(func_Array_Find_Str((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[0],(char *)ucRecvCheckData,strlen((char*)ucRecvCheckData), &usPosition) == 0) //接收到的数据中包含OK
        {
            if(gE_4G_Module_Init_CMD == Module_PUBLISH_TOPIC_CMD)
            {
                return 0;
            }
            else if(gE_4G_Module_Init_CMD == Module_CONN_MQTT_INTERFACE_CMD)
            {
                gE_4G_Module_Init_CMD = Module_PUBLISH_TOPIC_CMD;
            }
            else if(gE_4G_Module_Init_CMD == Module_DISCONNECT_MQTT_INTERFACE_CMD)
            {
                gE_4G_Module_Init_CMD = Module_OPEN_MQTT_INTERFACE_CMD;
            }
            else
            {
                gE_4G_Module_Init_CMD++;
            }
        }
        else
        {
            if(func_Array_Find_Str((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[0],"ERROR",5, &usPosition) == 0) //接收到的数据中包含OK
            {
                if(gE_4G_Module_Init_CMD == Module_CONN_MQTT_INTERFACE_CMD)
                {
                    gE_4G_Module_Init_CMD = Module_DISCONNECT_MQTT_INTERFACE_CMD;
                }
                ucRetryCnt++;
                if(ucRetryCnt >= 3) //重复发送三次
                {
                    //u8Temp = 0;
                    return 3;
                }
            }
            else
            {
                ucRetryCnt++;
                if(ucRetryCnt >= 3) //重复发送三次
                {
                    //u8Temp = 0;
                    return 2;
                }
            }
        }
    }
    //sprintf((char*)&ucSendBuf[0],"%s\r\n","{\"type\":\"variant_data\",\"version\":\"1.0\",\"time\":1638766638000,\"params\":{\"UAV\":220.5}}");
    
    #endif
    return 1;
}

/******************************************************************************
 * EOF (not truncated)
 *****************************************************************************/
