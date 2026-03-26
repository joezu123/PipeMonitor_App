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
#include "drv_LKT4202.h"
#include "User_Data.h"
/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define EC200U_BUF_SIZE             (1024U)
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
    PORT_SetFunc(USART1_RX_PORT, USART1_RX_PIN, Func_Spi2_Nss0, Disable);
    PORT_SetFunc(USART1_TX_PORT, USART1_TX_PIN, Func_Spi2_Sck, Disable);
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

    PORT_SetFunc(USART1_RX_PORT, USART1_RX_PIN, Func_Gpio, Disable);
    PORT_SetFunc(USART1_TX_PORT, USART1_TX_PIN, Func_Gpio, Disable);
    
    (void)PORT_Init(EC200U_4G_MODULE_RST_PORT, EC200U_4G_MODULE_RST_PIN, &stcGpioInit);
    #ifdef HW_VERSION_V1_1
    (void)PORT_Init(USART1_TX_PORT, USART1_TX_PIN, &stcGpioInit);  
    //PORT_SetBits(USART1_TX_PORT,USART1_TX_PIN);

    stcGpioInit.enPinMode = Pin_Mode_In;
    stcGpioInit.enPullUp = Enable;
    (void)PORT_Init(USART1_RX_PORT, USART1_RX_PIN, &stcGpioInit);
    #endif
    PORT_SetFunc(USART1_RX_PORT, USART1_RX_PIN, USART1_RX_FUNC, Disable);
    PORT_SetFunc(USART1_TX_PORT, USART1_TX_PIN, USART1_TX_FUNC, Disable);

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
    char ucTempArr[700] = {0};
    //uint8_t ucTempDataArr[50] = {0};
    uint8_t ucTempMonitorNameArr[700] = {0};
    uint8_t ucParaNameArr[80] = {0};
    //uint8_t ucDataArr[20] = {0}; //用于存储数据
    uint16_t usDataLen = 0;
    uint8_t j = 0;
    uint8_t l = 0;
    uint8_t i = 0;
    uint8_t ucMeasWaterVolumeFlag = 0;
    time_t now;
    #if 0
    struct tm tm;
    //time(&now);
    drv_mcu_Get_RTC_Time(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime);
    sscanf(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    tm.tm_year -= 1900; // 由于tm_year是从1900年开始计数的
    tm.tm_mon -= 1;     // tm_mon是从0开始的，所以需要减1
    tm.tm_isdst = -1;
    now = mktime(&tm) - 8*60*60; 
    #else
    now = func_Get_Linux_Time_Sec();
    #endif
    sprintf(ucTempArr, "{\"clientId\":\"%s\",\"pver\":24,\"msgId\":%ld,\"data\":{\"colname\":[\"deviceName\",\"collectGap\",\"reportGap\"],", 
                                                                                        pst_EC200USystemPara->DevicePara.cDeviceID,
                                                                                        (long)now);
    usDataLen =  strlen(ucTempArr);
    sprintf(&ucTempArr[usDataLen],"\"colvalue\":[\"%s\",\"%dm\",\"%dm\"],",pst_EC200USystemPara->DevicePara.cDeviceID,
                                                            pst_EC200USystemPara->DevicePara.nDeviceSampleGapCnt,
                                                            pst_EC200USystemPara->DevicePara.nDeviceUploadCnt);

    sprintf((char *)ucTempMonitorNameArr,"\"colmonitor\":[");

    if(pst_EC200USystemPara->DeviceRunPara.cBlackLightFlag == 0)
    {
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
                case Meas_HX_Integrated_Conductivity:
                case Meas_HZ_Integrated_Conductivity:
                    sprintf((char *)&ucParaNameArr[j], "\"COND\",");
                    j = strlen((char *)&ucParaNameArr);
                    sprintf((char *)&ucParaNameArr[j], "\"COND_B\",");
                    break;
                case Meas_BY_Radar_Level:
                case Meas_HZ_Radar_Level:
                case Meas_HX_Radar_Level:
                    sprintf((char *)&ucParaNameArr[j], "\"water_height\",");
                    j = strlen((char *)&ucParaNameArr);
                    sprintf((char *)&ucParaNameArr[j], "\"water_height_B\",");
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
                    sprintf((char *)&ucParaNameArr[j], "\"curr_volume\","); //瞬时流量/h
                    j = strlen((char *)&ucParaNameArr);
                    sprintf((char *)&ucParaNameArr[j], "\"curr_volume_B\",");
                    j = strlen((char *)&ucParaNameArr);
                    sprintf((char *)&ucParaNameArr[j], "\"flow_velocity\",");
                    j = strlen((char *)&ucParaNameArr);
                    sprintf((char *)&ucParaNameArr[j], "\"flow_velocity_B\",");
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
            j = strlen((char *)&ucParaNameArr);
            sprintf((char *)&ucParaNameArr[j], "\"total_volume_B\",");
            (void)strcat((char *)ucTempMonitorNameArr, (char *)ucParaNameArr);
        }
    }
    else    //黑光图像站
    {
        memset(ucParaNameArr, 0, sizeof(ucParaNameArr));
        //memset(ucDataArr, 0, sizeof(ucDataArr));
        j = 0;
        sprintf((char *)&ucParaNameArr[j], "\"pictime\",\"picture\",\"piccount\",\"index\",");
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

    //设备蓝牙连接状态
    memset(ucParaNameArr, 0, sizeof(ucParaNameArr));
    sprintf((char *)&ucParaNameArr[0], "\"BT_Connected\",");
    (void)strcat((char *)ucTempMonitorNameArr, (char *)ucParaNameArr);

    //设备是否开启调试模式标志
    memset(ucParaNameArr, 0, sizeof(ucParaNameArr));
    sprintf((char *)&ucParaNameArr[0], "\"Debug_Model\",");
    (void)strcat((char *)ucTempMonitorNameArr, (char *)ucParaNameArr);

    //设备是否开启长供电模式
    memset(ucParaNameArr, 0, sizeof(ucParaNameArr));
    sprintf((char *)&ucParaNameArr[0], "\"LongPowerModel\",");
    (void)strcat((char *)ucTempMonitorNameArr, (char *)ucParaNameArr);

    //设备MCU内部温度
    memset(ucParaNameArr, 0, sizeof(ucParaNameArr));
    sprintf((char *)&ucParaNameArr[0], "\"DevTemperature\",");
    (void)strcat((char *)ucTempMonitorNameArr, (char *)ucParaNameArr);

    //设备信号强度
    memset(ucParaNameArr, 0, sizeof(ucParaNameArr));
    sprintf((char *)&ucParaNameArr[0], "\"CSQ\",");
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

    //NFC状态
    memset(ucParaNameArr, 0, sizeof(ucParaNameArr));
    sprintf((char *)&ucParaNameArr[0], "\"NFC\",");
    (void)strcat((char *)ucTempMonitorNameArr, (char *)ucParaNameArr);

    //ICCID
    memset(ucParaNameArr, 0, sizeof(ucParaNameArr));
    sprintf((char *)&ucParaNameArr[0], "\"ICCID\",");
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

time_t now;
unsigned long ulTime = 0;
uint32_t ulRecordCnt = 0;
uint8_t ucRecordCnt = 0;
DevMeasRecordDataSt st_TempValue[10];
//uint8_t ucBaseDataValueArr[6][150] = {{0}};
uint8_t ucWaterLevel_Radar_Flag = 0;    //雷达液位测量传感器存在标志位
#ifndef WATERLEVEL_RADAR_PRESS
uint8_t ucWaterLevel_Pressure_Flag = 0; //压力液位测量传感器存在标志位
#endif
uint8_t ucWaterQuality_COD_Flag = 0; //水质COD测量传感器存在标志位
uint8_t ucWaterQuality_COND_Flag = 0; //水质电导率测量传感器存在标志位
uint8_t ucMeasWaterVolumeFlag = 0; //流量测量传感器存在标志位
uint8_t ucMeasSensorExistFlag = 0; //测量传感器存在标志位

//CRC校验的范围：去除起始帧68，结束帧16，以及crc本身以外的全部数据。CRC16标准为crc16/xmodem标准，算法如下：
unsigned short CRC16_SM4(unsigned char *puchMsg, unsigned int usDataLen)  
{  
    unsigned short wCRCin = 0x0000;  
    unsigned short wCPoly = 0x1021;  
    unsigned char wChar = 0;  
    
    while (usDataLen--)     
    {  
        wChar = *(puchMsg++);  
        wCRCin ^= (wChar << 8);  
        for(int i = 0;i < 8;i++)  
        {  
            if(wCRCin & 0x8000)  
            {
                wCRCin = (wCRCin << 1) ^ wCPoly; 
            }
            else  
            {
                wCRCin = wCRCin << 1;  
            }
        }  
    }  
    return (wCRCin) ;  
}

void hex_to_str(uint8_t *pucData, uint16_t ucDataLen, uint8_t *pucStr)
{
    uint8_t ddl, ddh;
    uint16_t i;
    for (i = 0; i < ucDataLen; i++)
    {
        ddh = ('0' + pucData[i] / 16);
        ddl = ('0' + pucData[i] % 16);
        if (ddh > '9')
        {
            ddh = ddh + ('a' - '9' - 1);
        }
        if (ddl > '9')
        {
            ddl = ddl + ('a' - '9' - 1);
        }
        pucStr[i * 2] = ddh;
        pucStr[i * 2 + 1] = ddl;
    }
}

//获取设备发送黑光图像站照片数据
uint16_t func_Get_BlackLightDataUploadCMD_Data(uint8_t *ucDataArr, unsigned char* ucPosi)
{
    uint8_t ucTempArr[1200] = {0};
    //uint8_t ucArr[100] = {0};
    struct tm tm;
    unsigned short usDataLen = 0;
    //unsigned char ucLen = 0;
    uint8_t ucEntryArr[1200] = {0};
    uint16_t usDataLength = 0;
    uint8_t l = 0;
    //time(&now);
    #if 0
    drv_mcu_Get_RTC_Time(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime);
    sscanf(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    tm.tm_year -= 1900; // 由于tm_year是从1900年开始计数的
    tm.tm_mon -= 1;     // tm_mon是从0开始的，所以需要减1
    tm.tm_isdst = -1;
    now = mktime(&tm) - 8*60*60; //将时间转换为UTC时间，减去8小时
    #else
    now = func_Get_Linux_Time_Sec();
    #endif
    ulTime = pst_EC200USystemPara->DeviceRunPara.ulUploadRecordStartTime;
    sprintf((char *)ucTempArr, "{\"clientId\":\"%s\",\"pver\":24,\"beginTime\":%ld,\"msgId\":%ld,\"gap\":%d,\"data\":{\"pictime\":[%ld],\"picture\":[\"", 
                                                                                        pst_EC200USystemPara->DevicePara.cDeviceID,
                                                                                        (long)now,
                                                                                        (long)now,
                                                                                        pst_EC200USystemPara->DevicePara.nDeviceSaveRecordCnt * 60,
                                                                                        ulTime);
    usDataLen = strlen((char *)ucTempArr);
    //sprintf((char *)ucTempArr[strlen(ucTempArr)],"%s", &pst_EC200USystemPara->DeviceRunPara.st_BlackLightData.ucPhotoData[*ucPosi][0]);
    if(*ucPosi < pst_EC200USystemPara->DeviceRunPara.st_BlackLightData.ucPhotoDataCnt - 1)
    {
        hex_to_str(&pst_EC200USystemPara->DeviceRunPara.st_BlackLightData.ucPhotoData[*ucPosi][0],400,&ucTempArr[strlen((char *)ucTempArr)]);
    }
    else
    {
        hex_to_str(&pst_EC200USystemPara->DeviceRunPara.st_BlackLightData.ucPhotoData[*ucPosi][0],pst_EC200USystemPara->DeviceRunPara.st_BlackLightData.ulCurGetDataSize - ((pst_EC200USystemPara->DeviceRunPara.st_BlackLightData.ucPhotoDataCnt-1) * 400),&ucTempArr[strlen((char *)ucTempArr)]);
    }
    
    //memcpy((char *)ucTempArr[strlen(ucTempArr)], &pst_EC200USystemPara->DeviceRunPara.st_BlackLightData.ucPhotoData[*ucPosi][0],1024);
    //usDataLen += 1024;
    //(void)strcat((char *)ucTempArr, &pst_EC200USystemPara->DeviceRunPara.st_BlackLightData.ucPhotoData[*ucPosi][0]);
    sprintf((char*)&ucTempArr[strlen((char *)ucTempArr)],"\"],\"piccount\":[%d],\"index\":[%d]}}",pst_EC200USystemPara->DeviceRunPara.st_BlackLightData.ucPhotoDataCnt,*ucPosi);
    //ucLen = strlen((char *)ucArr);
    //(void)strcat((char *)ucTempArr[usDataLen], ucArr);
    //usDataLen += ucLen;
    usDataLen = strlen((char *)ucTempArr);
    //#ifdef SM4_ENTRY_ENABLE
    if(pst_EC200USystemPara->DevicePara.cSM4EntryFlag == 1)
    {
        l = usDataLen % 16;
        if(l != 0)
        {
            usDataLength = usDataLen + 16-l;
        }
        drv_LKT4202_SendData_Encry(&ucTempArr[0], (char*)ucEntryArr,usDataLength);
        unsigned short usCRC16 = CRC16_SM4(ucEntryArr, usDataLength);
        memcpy(&ucEntryArr[usDataLength], &usCRC16, 2);
        usDataLength += 2;
        memcpy(ucDataArr, ucEntryArr, usDataLength);
        usDataLen = usDataLength;
    }
    else
    {
        memcpy(ucDataArr, ucTempArr, usDataLen);
    }
    //#else
    //memset(ucTempArr,0,strlen((char *)ucTempArr));
    //drv_LKT4202_SendData_Decry(ucEntryArr, (char*)ucTempArr,usDataLength);
    //usDataLen = strlen((char *)ucEntryArr);

    
    //#endif
    return usDataLen;                          
}

//获取设备监测项数据上报报文
uint16_t func_Get_DataUploadCMD_Data(uint8_t *ucDataArr)
{
    uint8_t ucTempArr[1700] = {0};
    uint8_t ucEntryArr[1700] = {0};
    uint8_t ucTempValueArr[12][150] = {{0}}; //用于存储数据
    //uint8_t ucEntryValueArr[12][150] = {{0}}; //用于存储数据
    //uint8_t ucTempAttiArr[50] = {0}; //用于存储姿态数据
    //uint8_t ucTempWaterImmersionArr[50] = {0}; //用于存储水浸数据
    //uint8_t ucTempPhotosensitiveArr[50] = {0}; //用于存储光照数据
    //uint8_t ucBT_ConnArr[50] = {0}; //用于存储设备蓝牙连接状态数据
    //uint8_t ucDebugModelArr[50] = {0};  //用于存储设备是否开启调试模式数据
    //uint8_t ucLongPowerModelArr[50] = {0};  //用于存储设备是否开启长供电模式数据
    //uint8_t ucDevTempArr[150] = {0};    //用于存储设备内部MCU温度数据
    //uint8_t ucCSQArr[50] = {0};    //用于存储设备信号强度数据
    //uint8_t ucParaNameArr[10] = {0};
    ucWaterLevel_Radar_Flag = 0;    //雷达液位测量传感器存在标志位
    #ifndef WATERLEVEL_RADAR_PRESS
    ucWaterLevel_Pressure_Flag = 0; //压力液位测量传感器存在标志位
    #endif
    ucWaterQuality_COD_Flag = 0; //水质COD测量传感器存在标志位
    ucWaterQuality_COND_Flag = 0; //水质电导率测量传感器存在标志位
    ucMeasWaterVolumeFlag = 0; //流量测量传感器存在标志位
    ucMeasSensorExistFlag = 0;
    uint8_t ucRightValueCnt = 0;
    double dRightValueSum = 0.0;
    //uint8_t j = 0;
    uint8_t k = 0;

    uint16_t usDataLen = 0;
    uint16_t usDataLength = 0;
    //uint8_t ucMeasWaterVolumeFlag = 0;
    uint8_t i = 0;
    uint8_t l = 0;
    int nPosi = 0;
    char cCheckCnt = 0;
    char cn = 0;
    
    //memset(ucBaseDataValueArr[0],0,150);
    //memset(ucBaseDataValueArr[1],0,150);
    //memset(ucBaseDataValueArr[2],0,150);
    //memset(ucBaseDataValueArr[3],0,150);
    //memset(ucBaseDataValueArr[4],0,150);
    //memset(ucBaseDataValueArr[5],0,150);
    #if 0
    //time(&now);
    struct tm tm;
    //time(&now);
    drv_mcu_Get_RTC_Time(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime);
    sscanf(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    tm.tm_year -= 1900; // 由于tm_year是从1900年开始计数的
    tm.tm_mon -= 1;     // tm_mon是从0开始的，所以需要减1
    tm.tm_isdst = -1;
    now = mktime(&tm) - 8*60*60; //将时间转换为UTC时间，减去8小时
    #else
    now = func_Get_Linux_Time_Sec();
    #endif
    //if(pst_EC200USystemPara->DeviceRunPara.ulUploadRecordLostCnt == pst_EC200USystemPara->DevicePara.nDeviceUploadCnt / pst_EC200USystemPara->DevicePara.nDeviceSaveRecordCnt)
    {
    //    pst_EC200USystemPara->DeviceRunPara.ulUploadRecordStartTime = (long)now; //记录上传数据开始时间
    }
    if(pst_EC200USystemPara->DeviceRunPara.cBarTouchFlag == 3)
    {
        ulTime = pst_EC200USystemPara->DeviceRunPara.ulBarTouchEventUploadTime;
        if((ulTime < 1762136855) || (ulTime > (long)now))
        {
            ulTime = (long)now-(pst_EC200USystemPara->DevicePara.nDeviceUploadCnt * 60); //记录上传数据开始时间
        }
    }
    else
    {
        if((ulTime < 1762136855) || (ulTime > (long)now))
        {
            pst_EC200USystemPara->DeviceRunPara.ulUploadRecordStartTime = (long)now-(pst_EC200USystemPara->DevicePara.nDeviceUploadCnt * 60); //记录上传数据开始时间
        }
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
            case Meas_HX_Radar_Level:
                ucWaterLevel_Radar_Flag = 1;
                ucMeasSensorExistFlag = 1;
                memcpy(&ucTempValueArr[0], "\"water_height\":[", 16);
                memcpy(&ucTempValueArr[6], "\"water_height_B\":[", 18);
                //memcpy(&ucBaseDataValueArr[0], "\"water_height_B\":[", 18);
                break;
            #ifndef WATERLEVEL_RADAR_PRESS
            case Meas_BY_Pressure_Level:
                ucWaterLevel_Pressure_Flag = 1;
                memcpy(&ucTempValueArr[1], "\"pipeline_water_height\":[", 25);
                break;
            #endif
            case Meas_HX_WaterQuality_COD:
                ucWaterQuality_COD_Flag = 1;
                ucMeasSensorExistFlag = 1;
                memcpy(&ucTempValueArr[1], "\"COD\":[", 7);
                memcpy(&ucTempValueArr[7], "\"COD_B\":[", 9);
                //memcpy(&ucBaseDataValueArr[1], "\"COD_B\":[", 9);
                break;
            case Meas_BY_Integrated_Conductivity:
            case Meas_HX_Integrated_Conductivity:
            case Meas_HZ_Integrated_Conductivity:
                ucWaterQuality_COND_Flag = 1;
                ucMeasSensorExistFlag = 1;
                memcpy(&ucTempValueArr[2], "\"COND\":[", 8);
                memcpy(&ucTempValueArr[8], "\"COND_B\":[", 10);
                //memcpy(&ucBaseDataValueArr[2], "\"COND_B\":[", 10);
                break;
            case Meas_Flowmeter:
            case Meas_HZ_Radar_Ultrasonic_Flow:
            case Meas_HZ_Ultrasonic_Flow:
            case Meas_HX_Radar_Ultrasonic_Flow:
            case Meas_HX_Flowmeter:
                ucMeasWaterVolumeFlag = 1;
                ucMeasSensorExistFlag = 1;
                memcpy(&ucTempValueArr[3], "\"curr_volume\":[", 15);
                memcpy(&ucTempValueArr[9], "\"curr_volume_B\":[", 17);
                //memcpy(&ucBaseDataValueArr[3], "\"curr_volume_B\":[", 17);
                memcpy(&ucTempValueArr[4], "\"flow_velocity\":[", 17);
                memcpy(&ucTempValueArr[10], "\"flow_velocity_B\":[", 19);
                //memcpy(&ucBaseDataValueArr[4], "\"flow_velocity_B\":[", 19);
                memcpy(&ucTempValueArr[5], "\"total_volume\":[", 16);
                memcpy(&ucTempValueArr[11], "\"total_volume_B\":[", 18);
                //memcpy(&ucBaseDataValueArr[5], "\"total_volume_B\":[", 18);
                break;
            default:
                break;
            }
        }
    }

    if(pst_EC200USystemPara->DeviceRunPara.cBarTouchFlag == 0)
    {
        //根据本次要上传的数据个数，从存储中读取该数据
        for(nPosi=0; nPosi<ucRecordCnt; nPosi++)
        {
            func_Get_Device_MeasData_Record(pst_EC200USystemPara->DevicePara.nDeviceRecordCnt - ulRecordCnt + nPosi, &st_TempValue[nPosi]);
            if((st_TempValue[nPosi].cWater_Immersion_Status == 0xFF) && (st_TempValue[nPosi].nAttitude_SC7A == 0xFFFF))
            {
                memcpy(&st_TempValue[nPosi].fWaterLevel, &gt_MeasData.fWaterLevel, sizeof(DevMeasRecordDataSt));
            }
        }
        for(nPosi=0; nPosi<ucRecordCnt; nPosi++)
        {
            if(ucWaterLevel_Radar_Flag == 1)
            {
                if((double)st_TempValue[nPosi].fWaterLevel < 0.01)  //当出现数据异常时
                {
                    ucRightValueCnt = 0;
                    dRightValueSum = 0.0;
                    if(nPosi < 1)   //当前异常数据为本次要上传数据的第一项
                    {
                        for(k=1; k<ucRecordCnt; k++)    //判断本次上传的数据中，后续部分是否存在正确数据，若存在，则记录相关数据
                        {
                            if((double)st_TempValue[k].fWaterLevel >= 0.01)
                            {
                                ucRightValueCnt++;
                                dRightValueSum += (double)st_TempValue[k].fWaterLevel;
                            }
                        }
                        if(ucRightValueCnt > 0)
                        {
                            dRightValueSum = dRightValueSum / ucRightValueCnt;
                        }
                        else
                        {
                            dRightValueSum = (double)st_TempValue[nPosi].fWaterLevel;
                        }
                    }
                    else
                    {
                        for(k=0; k<nPosi; k++)
                        {
                            if((double)st_TempValue[k].fWaterLevel >= 0.01)
                            {
                                ucRightValueCnt++;
                                dRightValueSum += (double)st_TempValue[k].fWaterLevel;
                            }
                        }
                        if(ucRightValueCnt > 0)
                        {
                            dRightValueSum = dRightValueSum / ucRightValueCnt;
                        }
                        else
                        {
                            dRightValueSum = (double)st_TempValue[nPosi].fWaterLevel;
                        }
                    }
                }
                else
                {
                    dRightValueSum = (double)st_TempValue[nPosi].fWaterLevel;
                }
                if((dRightValueSum < 0.001) || (dRightValueSum >= 100.0))
                {
                    dRightValueSum = 0.0;
                }
                sprintf((char *)&ucTempValueArr[0][strlen((char *)ucTempValueArr[0])], "%.3lf,", dRightValueSum);
                sprintf((char *)&ucTempValueArr[6][strlen((char *)ucTempValueArr[6])], "%.3lf,", (double)st_TempValue[nPosi].fWaterLevel);
                //sprintf((char *)&ucBaseDataValueArr[0][strlen((char *)ucBaseDataValueArr[0])], "%.3lf,", (double)st_TempValue[nPosi].fWaterLevel);
            }
            #ifndef WATERLEVEL_RADAR_PRESS
            if(ucWaterLevel_Pressure_Flag == 1)
            {
                sprintf((char *)&ucTempValueArr[1][strlen((char *)ucTempValueArr[1])], "%.3lf,", (double)st_TempValue.fWaterLevel_Pres);
            }
            #endif
            if(ucWaterQuality_COD_Flag == 1)
            {
                if((double)st_TempValue[nPosi].fWaterQuality_COD < 10.0)  //当出现数据异常时
                {
                    ucRightValueCnt = 0;
                    dRightValueSum = 0.0;
                    if(nPosi < 1)   //当前异常数据为本次要上传数据的第一项
                    {
                        for(k=1; k<ucRecordCnt; k++)    //判断本次上传的数据中，后续部分是否存在正确数据，若存在，则记录相关数据
                        {
                            if((double)st_TempValue[k].fWaterQuality_COD >= 10.0)
                            {
                                ucRightValueCnt++;
                                dRightValueSum += (double)st_TempValue[k].fWaterQuality_COD;
                            }
                        }
                        if(ucRightValueCnt > 0)
                        {
                            dRightValueSum = dRightValueSum / ucRightValueCnt;
                        }
                        else
                        {
                            dRightValueSum = (double)st_TempValue[nPosi].fWaterQuality_COD;
                        }
                    }
                    else
                    {
                        for(k=0; k<nPosi; k++)
                        {
                            if((double)st_TempValue[k].fWaterQuality_COD >=10.0)
                            {
                                ucRightValueCnt++;
                                dRightValueSum += (double)st_TempValue[k].fWaterQuality_COD;
                            }
                        }
                        if(ucRightValueCnt > 0)
                        {
                            dRightValueSum = dRightValueSum / ucRightValueCnt;
                        }
                        else
                        {
                            dRightValueSum = (double)st_TempValue[nPosi].fWaterQuality_COD;
                        }
                    }
                }
                else
                {
                    dRightValueSum = (double)st_TempValue[nPosi].fWaterQuality_COD;
                }
                if((dRightValueSum < 0.001) || (dRightValueSum >= 1000000.0))
                {
                    dRightValueSum = 0.0;
                }
                sprintf((char *)&ucTempValueArr[1][strlen((char *)ucTempValueArr[1])], "%.3lf,", dRightValueSum);
                sprintf((char *)&ucTempValueArr[7][strlen((char *)ucTempValueArr[7])], "%.3lf,", (double)st_TempValue[nPosi].fWaterQuality_COD);
                //sprintf((char *)&ucBaseDataValueArr[1][strlen((char *)ucBaseDataValueArr[1])], "%.3lf,", (double)st_TempValue[nPosi].fWaterQuality_COD);
            }
            if(ucWaterQuality_COND_Flag == 1)
            {
                if((double)st_TempValue[nPosi].fWaterQuality_COND < 10.0)  //当出现数据异常时
                {
                    ucRightValueCnt = 0;
                    dRightValueSum = 0.0;
                    if(nPosi < 1)   //当前异常数据为本次要上传数据的第一项
                    {
                        for(k=1; k<ucRecordCnt; k++)    //判断本次上传的数据中，后续部分是否存在正确数据，若存在，则记录相关数据
                        {
                            if((double)st_TempValue[k].fWaterQuality_COND >= 10.0)
                            {
                                ucRightValueCnt++;
                                dRightValueSum += (double)st_TempValue[k].fWaterQuality_COND;
                            }
                        }
                        if(ucRightValueCnt > 0)
                        {
                            dRightValueSum = dRightValueSum / ucRightValueCnt;
                        }
                        else
                        {
                            dRightValueSum = (double)st_TempValue[nPosi].fWaterQuality_COND;
                        }
                    }
                    else
                    {
                        for(k=0; k<nPosi; k++)
                        {
                            if((double)st_TempValue[k].fWaterQuality_COND >=10.0)
                            {
                                ucRightValueCnt++;
                                dRightValueSum += (double)st_TempValue[k].fWaterQuality_COND;
                            }
                        }
                        if(ucRightValueCnt > 0)
                        {
                            dRightValueSum = dRightValueSum / ucRightValueCnt;
                        }
                        else
                        {
                            dRightValueSum = (double)st_TempValue[nPosi].fWaterQuality_COND;
                        }
                    }
                }
                else
                {
                    dRightValueSum = (double)st_TempValue[nPosi].fWaterQuality_COND;
                }
                if((dRightValueSum < 0.001) || (dRightValueSum >= 1000000.0))
                {
                    dRightValueSum = 0.0;
                }
                sprintf((char *)&ucTempValueArr[2][strlen((char *)ucTempValueArr[2])], "%.3lf,", dRightValueSum);
                sprintf((char *)&ucTempValueArr[8][strlen((char *)ucTempValueArr[8])], "%.3lf,", (double)st_TempValue[nPosi].fWaterQuality_COND);
                //sprintf((char *)&ucBaseDataValueArr[2][strlen((char *)ucBaseDataValueArr[2])], "%.3lf,", (double)st_TempValue[nPosi].fWaterQuality_COND);
            }
            if(ucMeasWaterVolumeFlag == 1)
            {
                if((double)st_TempValue[nPosi].fWaterVolume < 10.0)  //当出现数据异常时
                {
                    ucRightValueCnt = 0;
                    dRightValueSum = 0.0;
                    if(nPosi < 1)   //当前异常数据为本次要上传数据的第一项
                    {
                        for(k=1; k<ucRecordCnt; k++)    //判断本次上传的数据中，后续部分是否存在正确数据，若存在，则记录相关数据
                        {
                            if((double)st_TempValue[k].fWaterVolume >= 10.0)
                            {
                                ucRightValueCnt++;
                                dRightValueSum += (double)st_TempValue[k].fWaterVolume;
                            }
                        }
                        if(ucRightValueCnt > 0)
                        {
                            dRightValueSum = dRightValueSum / ucRightValueCnt;
                        }
                        else
                        {
                            dRightValueSum = (double)st_TempValue[nPosi].fWaterVolume;
                        }
                    }
                    else
                    {
                        for(k=0; k<nPosi; k++)
                        {
                            if((double)st_TempValue[k].fWaterVolume >=10.0)
                            {
                                ucRightValueCnt++;
                                dRightValueSum += (double)st_TempValue[k].fWaterVolume;
                            }
                        }
                        if(ucRightValueCnt > 0)
                        {
                            dRightValueSum = dRightValueSum / ucRightValueCnt;
                        }
                        else
                        {
                            dRightValueSum = (double)st_TempValue[nPosi].fWaterVolume;
                        }
                    }
                }
                else
                {
                    dRightValueSum = (double)st_TempValue[nPosi].fWaterVolume;
                }
                if((dRightValueSum < 0.001) || (dRightValueSum >= 10000000.0))
                {
                    dRightValueSum = 0.0;
                }
                sprintf((char *)&ucTempValueArr[3][strlen((char *)ucTempValueArr[3])], "%.3lf,", dRightValueSum);
                sprintf((char *)&ucTempValueArr[9][strlen((char *)ucTempValueArr[9])], "%.3lf,", (double)st_TempValue[nPosi].fWaterVolume);
                //sprintf((char *)&ucBaseDataValueArr[3][strlen((char *)ucBaseDataValueArr[3])], "%.3lf,", (double)st_TempValue[nPosi].fWaterVolume);

                if((double)st_TempValue[nPosi].fWaterSpeed < 0.01)  //当出现数据异常时
                {
                    ucRightValueCnt = 0;
                    dRightValueSum = 0.0;
                    if(nPosi < 1)   //当前异常数据为本次要上传数据的第一项
                    {
                        for(k=1; k<ucRecordCnt; k++)    //判断本次上传的数据中，后续部分是否存在正确数据，若存在，则记录相关数据
                        {
                            if((double)st_TempValue[k].fWaterSpeed >= 0.01)
                            {
                                ucRightValueCnt++;
                                dRightValueSum += (double)st_TempValue[k].fWaterSpeed;
                            }
                        }
                        if(ucRightValueCnt > 0)
                        {
                            dRightValueSum = dRightValueSum / ucRightValueCnt;
                        }
                        else
                        {
                            dRightValueSum = (double)st_TempValue[nPosi].fWaterSpeed;
                        }
                    }
                    else
                    {
                        for(k=0; k<nPosi; k++)
                        {
                            if((double)st_TempValue[k].fWaterSpeed >= 0.01)
                            {
                                ucRightValueCnt++;
                                dRightValueSum += (double)st_TempValue[k].fWaterSpeed;
                            }
                        }
                        if(ucRightValueCnt > 0)
                        {
                            dRightValueSum = dRightValueSum / ucRightValueCnt;
                        }
                        else
                        {
                            dRightValueSum = (double)st_TempValue[nPosi].fWaterSpeed;
                        }
                    }
                }
                else
                {
                    dRightValueSum = (double)st_TempValue[nPosi].fWaterSpeed;
                }
                if((dRightValueSum < 0.001) || (dRightValueSum >= 10000000000000000.0))
                {
                    dRightValueSum = 0.0;
                }
                sprintf((char *)&ucTempValueArr[4][strlen((char *)ucTempValueArr[4])], "%.3lf,", dRightValueSum);
                sprintf((char *)&ucTempValueArr[10][strlen((char *)ucTempValueArr[10])], "%.3lf,", (double)st_TempValue[nPosi].fWaterSpeed);
                //sprintf((char *)&ucBaseDataValueArr[4][strlen((char *)ucBaseDataValueArr[4])], "%.3lf,", (double)st_TempValue[nPosi].fWaterSpeed);

                if((double)st_TempValue[nPosi].fWaterVolume_Total < 10.0)  //当出现数据异常时
                {
                    ucRightValueCnt = 0;
                    dRightValueSum = 0.0;
                    if(nPosi < 1)   //当前异常数据为本次要上传数据的第一项
                    {
                        for(k=1; k<ucRecordCnt; k++)    //判断本次上传的数据中，后续部分是否存在正确数据，若存在，则记录相关数据
                        {
                            if((double)st_TempValue[k].fWaterVolume_Total >= 10.0)
                            {
                                ucRightValueCnt++;
                                dRightValueSum += (double)st_TempValue[k].fWaterVolume_Total;
                            }
                        }
                        if(ucRightValueCnt > 0)
                        {
                            dRightValueSum = dRightValueSum / ucRightValueCnt;
                        }
                        else
                        {
                            dRightValueSum = (double)st_TempValue[nPosi].fWaterVolume_Total;
                        }
                    }
                    else
                    {
                        for(k=0; k<nPosi; k++)
                        {
                            if((double)st_TempValue[k].fWaterVolume_Total >= 10.0)
                            {
                                ucRightValueCnt++;
                                dRightValueSum += (double)st_TempValue[k].fWaterVolume_Total;
                            }
                        }
                        if(ucRightValueCnt > 0)
                        {
                            dRightValueSum = dRightValueSum / ucRightValueCnt;
                        }
                        else
                        {
                            dRightValueSum = (double)st_TempValue[nPosi].fWaterVolume_Total;
                        }
                    }
                }
                else
                {
                    dRightValueSum = (double)st_TempValue[nPosi].fWaterVolume_Total;
                }
                if((dRightValueSum < 0.001) || (dRightValueSum >= 10000000000000000000000.0))
                {
                    dRightValueSum = 0.0;
                }
                sprintf((char *)&ucTempValueArr[5][strlen((char *)ucTempValueArr[5])], "%.3lf,", dRightValueSum);
                sprintf((char *)&ucTempValueArr[11][strlen((char *)ucTempValueArr[11])], "%.3lf,", (double)st_TempValue[nPosi].fWaterVolume_Total);
                //sprintf((char *)&ucBaseDataValueArr[5][strlen((char *)ucBaseDataValueArr[5])], "%.3lf,", (double)st_TempValue[nPosi].fWaterVolume_Total);
            }
        }
        if(ucRecordCnt > 0)
        {
            //ucTempValueArr[0][16] = 'n';
            //ucTempValueArr[0][17] = 'a';
            //ucTempValueArr[0][18] = 'n';    
            //ucTempValueArr[2][8] = 'n';
            //ucTempValueArr[2][9] = 'a';
            //ucTempValueArr[2][10] = 'n';
            for(cCheckCnt=0; cCheckCnt<12; cCheckCnt++)
            {
                if(strlen((char *)ucTempValueArr[cCheckCnt]) > 0)
                {
                    for(cn=0; cn<strlen((char *)ucTempValueArr[cCheckCnt]); cn++)
                    {
                        if((ucTempValueArr[cCheckCnt][cn] == 'n') && (ucTempValueArr[cCheckCnt][cn+1] == 'a') && (ucTempValueArr[cCheckCnt][cn+2] == 'n'))
                        {
                            //sprintf((char *)&ucTempValueArr[cCheckCnt][cn], "%.1lf,", 0.0);
                            ucTempValueArr[cCheckCnt][cn] = '0';
                            ucTempValueArr[cCheckCnt][cn+1] = '.';
                            ucTempValueArr[cCheckCnt][cn+2] = '0';
                        }
                    }
                }
            }
        }
    }
    else
    {
        ucRecordCnt = 1;
        if(ucWaterLevel_Radar_Flag == 1)
        {
            sprintf((char *)&ucTempValueArr[0][strlen((char *)ucTempValueArr[0])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterLevel);
            sprintf((char *)&ucTempValueArr[6][strlen((char *)ucTempValueArr[6])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterLevel);
            //sprintf((char *)&ucBaseDataValueArr[0][strlen((char *)ucBaseDataValueArr[0])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterLevel);
        }
        #ifndef WATERLEVEL_RADAR_PRESS
        if(ucWaterLevel_Pressure_Flag == 1)
        {
            sprintf((char *)&ucTempValueArr[1][strlen((char *)ucTempValueArr[1])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterLevel_Pres);
        }
        #endif
        if(ucWaterQuality_COD_Flag == 1)
        {
            sprintf((char *)&ucTempValueArr[1][strlen((char *)ucTempValueArr[1])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterQuality_COD);
            sprintf((char *)&ucTempValueArr[7][strlen((char *)ucTempValueArr[7])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterQuality_COD);
            //sprintf((char *)&ucBaseDataValueArr[1][strlen((char *)ucBaseDataValueArr[1])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterQuality_COD);
        }
        if(ucWaterQuality_COND_Flag == 1)
        {
            sprintf((char *)&ucTempValueArr[2][strlen((char *)ucTempValueArr[2])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterQuality_COND);
            sprintf((char *)&ucTempValueArr[8][strlen((char *)ucTempValueArr[8])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterQuality_COND);
            //sprintf((char *)&ucBaseDataValueArr[2][strlen((char *)ucBaseDataValueArr[2])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterQuality_COND);
        }
        if(ucMeasWaterVolumeFlag == 1)
        {
            sprintf((char *)&ucTempValueArr[3][strlen((char *)ucTempValueArr[3])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterVolume);
            sprintf((char *)&ucTempValueArr[9][strlen((char *)ucTempValueArr[9])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterVolume);
            //sprintf((char *)&ucBaseDataValueArr[3][strlen((char *)ucBaseDataValueArr[3])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterVolume);
            sprintf((char *)&ucTempValueArr[4][strlen((char *)ucTempValueArr[4])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterSpeed);
            sprintf((char *)&ucTempValueArr[10][strlen((char *)ucTempValueArr[10])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterSpeed);
            //sprintf((char *)&ucBaseDataValueArr[4][strlen((char *)ucBaseDataValueArr[4])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterSpeed);
            sprintf((char *)&ucTempValueArr[5][strlen((char *)ucTempValueArr[5])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterVolume_Total);
            sprintf((char *)&ucTempValueArr[11][strlen((char *)ucTempValueArr[11])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterVolume_Total);
            //sprintf((char *)&ucBaseDataValueArr[5][strlen((char *)ucBaseDataValueArr[5])], "%.3lf,", (double)gSt_DevMeasRecordData.fWaterVolume_Total);
        }
    }

    if(ucWaterLevel_Radar_Flag == 1)
    {
        if(ucRecordCnt > 0)
        {
            memcpy(&ucTempValueArr[0][strlen((char *)ucTempValueArr[0])-1], "],", 2);
        }
        else
        {
            memcpy(&ucTempValueArr[0][strlen((char *)ucTempValueArr[0])], "],", 2);
        }
        //drv_LKT4202_SendData_Encry(&ucTempValueArr[0][0], (char*)ucEntryValueArr[0]);
        //去掉最后一个逗号
        
        (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[0]);
        
        //drv_LKT4202_SendData_Encry(&ucTempValueArr[6][0], (char*)ucEntryValueArr[6]);
        if(ucRecordCnt > 0)
        {
            memcpy(&ucTempValueArr[6][strlen((char *)ucTempValueArr[6])-1], "],", 2);
        }
        else
        {
            memcpy(&ucTempValueArr[6][strlen((char *)ucTempValueArr[6])], "],", 2);
        }
        //memcpy(&ucBaseDataValueArr[0][strlen((char *)ucBaseDataValueArr[0])-1], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[6]);
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
        //drv_LKT4202_SendData_Encry(&ucTempValueArr[1][0], (char*)ucEntryValueArr[1]);
        //去掉最后一个逗号
        if(ucRecordCnt > 0)
        {
            memcpy(&ucTempValueArr[1][strlen((char *)ucTempValueArr[1])-1], "],", 2);
        }
        else
        {
            memcpy(&ucTempValueArr[1][strlen((char *)ucTempValueArr[1])], "],", 2);
        }
        (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[1]);

        //drv_LKT4202_SendData_Encry(&ucTempValueArr[7][0], (char*)ucEntryValueArr[7]);
        //memcpy(&ucBaseDataValueArr[1][strlen((char *)ucBaseDataValueArr[1])-1], "],", 2);
        if(ucRecordCnt > 0)
        {
            memcpy(&ucTempValueArr[7][strlen((char *)ucTempValueArr[7])-1], "],", 2);
        }
        else
        {
            memcpy(&ucTempValueArr[7][strlen((char *)ucTempValueArr[7])], "],", 2);
        }
        (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[7]);
    }
    if(ucWaterQuality_COND_Flag == 1)
    {
        //drv_LKT4202_SendData_Encry(&ucTempValueArr[2][0], (char*)ucEntryValueArr[2]);
        //去掉最后一个逗号
        if(ucRecordCnt > 0)
        {
            memcpy(&ucTempValueArr[2][strlen((char *)ucTempValueArr[2])-1], "],", 2);
        }
        else
        {
            memcpy(&ucTempValueArr[2][strlen((char *)ucTempValueArr[2])], "],", 2);
        }
        (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[2]);

        //drv_LKT4202_SendData_Encry(&ucTempValueArr[8][0], (char*)ucEntryValueArr[8]);
        //memcpy(&ucBaseDataValueArr[2][strlen((char *)ucBaseDataValueArr[2])-1], "],", 2);
        if(ucRecordCnt > 0)
        {
            memcpy(&ucTempValueArr[8][strlen((char *)ucTempValueArr[8])-1], "],", 2);
        }
        else
        {
            memcpy(&ucTempValueArr[8][strlen((char *)ucTempValueArr[8])], "],", 2);
        }
        (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[8]);
    }
    if(ucMeasWaterVolumeFlag == 1)
    {
        //drv_LKT4202_SendData_Encry(&ucTempValueArr[3][0], (char*)ucEntryValueArr[2]);
        //去掉最后一个逗号
        if(ucRecordCnt > 0)
        {
            memcpy(&ucTempValueArr[3][strlen((char *)ucTempValueArr[3])-1], "],", 2);
        }
        else
        {
            memcpy(&ucTempValueArr[3][strlen((char *)ucTempValueArr[3])], "],", 2);
        }
        (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[3]);

        //drv_LKT4202_SendData_Encry(&ucTempValueArr[9][0], (char*)ucEntryValueArr[9]);
        //memcpy(&ucBaseDataValueArr[3][strlen((char *)ucBaseDataValueArr[3])-1], "],", 2);
        if(ucRecordCnt > 0)
        {
            memcpy(&ucTempValueArr[9][strlen((char *)ucTempValueArr[9])-1], "],", 2);
        }
        else
        {
            memcpy(&ucTempValueArr[9][strlen((char *)ucTempValueArr[9])], "],", 2);
        }
        (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[9]);

        //drv_LKT4202_SendData_Encry(&ucTempValueArr[4][0], (char*)ucEntryValueArr[4]);
        if(ucRecordCnt > 0)
        {
            memcpy(&ucTempValueArr[4][strlen((char *)ucTempValueArr[4])-1], "],", 2);
        }
        else
        {
            memcpy(&ucTempValueArr[4][strlen((char *)ucTempValueArr[4])], "],", 2);
        }
        (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[4]);

        //drv_LKT4202_SendData_Encry(&ucTempValueArr[10][0], (char*)ucEntryValueArr[10]);
        //memcpy(&ucBaseDataValueArr[4][strlen((char *)ucBaseDataValueArr[4])-1], "],", 2);
        if(ucRecordCnt > 0)
        {
            memcpy(&ucTempValueArr[10][strlen((char *)ucTempValueArr[10])-1], "],", 2);
        }
        else
        {
            memcpy(&ucTempValueArr[10][strlen((char *)ucTempValueArr[10])], "],", 2);
        }
        (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[10]);

        //drv_LKT4202_SendData_Encry(&ucTempValueArr[5][0], (char*)ucEntryValueArr[5]);
        if(ucRecordCnt > 0)
        {
            memcpy(&ucTempValueArr[5][strlen((char *)ucTempValueArr[5])-1], "],", 2);
        }
        else
        {
            memcpy(&ucTempValueArr[5][strlen((char *)ucTempValueArr[5])], "],", 2);
        }
        (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[5]);

        //drv_LKT4202_SendData_Encry(&ucTempValueArr[11][0], (char*)ucEntryValueArr[11]);
        //memcpy(&ucBaseDataValueArr[5][strlen((char *)ucBaseDataValueArr[5])-1], "],", 2);
        if(ucRecordCnt > 0)
        {
            memcpy(&ucTempValueArr[11][strlen((char *)ucTempValueArr[11])-1], "],", 2);
        }
        else
        {
            memcpy(&ucTempValueArr[11][strlen((char *)ucTempValueArr[11])], "],", 2);
        }
        (void)strcat((char *)ucTempArr, (char *)ucTempValueArr[11]);
    }
    if((ucMeasSensorExistFlag == 1) && (ucRecordCnt > 0))
    {
        memcpy(&ucTempArr[strlen((char *)ucTempArr)-1], "}}", 2);
    }
    else
    {
        memcpy(&ucTempArr[strlen((char *)ucTempArr)], "}}", 2);
    }
    
    usDataLen = strlen((char *)ucTempArr);
    //#ifdef SM4_ENTRY_ENABLE
    if(pst_EC200USystemPara->DevicePara.cSM4EntryFlag == 1)
    {
    
        l = usDataLen % 16;
        if(l != 0)
        {
            usDataLength = usDataLen + 16-l;
        }
        drv_LKT4202_SendData_Encry(&ucTempArr[0], (char*)ucEntryArr,usDataLength);
        unsigned short usCRC16 = CRC16_SM4(ucEntryArr, usDataLength);
        memcpy(&ucEntryArr[usDataLength], &usCRC16, 2);
        usDataLength += 2;
        memcpy(ucDataArr, ucEntryArr, usDataLength);
        usDataLen = usDataLength;
    }
    //#else
    //memset(ucTempArr,0,strlen((char *)ucTempArr));
    //drv_LKT4202_SendData_Decry(ucEntryArr, (char*)ucTempArr,usDataLength);
    //usDataLen = strlen((char *)ucEntryArr);
    else
    {
        memcpy(ucDataArr, ucTempArr, usDataLen);
    }
    
    //#endif
    return usDataLen;                          
}

#if 0
//获取设备监测项数据上报报文
uint16_t func_Get_BaseDataUploadCMD_Data(uint8_t *ucDataArr)
{
    uint8_t ucTempArr[1200] = {0};
    uint8_t ucEntryArr[1200] = {0};
    //uint8_t ucTempValueArr[12][150] = {{0}}; //用于存储数据
    //uint8_t ucEntryValueArr[12][150] = {{0}}; //用于存储数据
    //uint8_t ucTempAttiArr[50] = {0}; //用于存储姿态数据
    //uint8_t ucTempWaterImmersionArr[50] = {0}; //用于存储水浸数据
    //uint8_t ucTempPhotosensitiveArr[50] = {0}; //用于存储光照数据
    //uint8_t ucBT_ConnArr[50] = {0}; //用于存储设备蓝牙连接状态数据
    //uint8_t ucDebugModelArr[50] = {0};  //用于存储设备是否开启调试模式数据
    //uint8_t ucLongPowerModelArr[50] = {0};  //用于存储设备是否开启长供电模式数据
    //uint8_t ucDevTempArr[150] = {0};    //用于存储设备内部MCU温度数据
    //uint8_t ucCSQArr[50] = {0};    //用于存储设备信号强度数据
    //uint8_t ucParaNameArr[10] = {0};
    //uint8_t ucWaterLevel_Radar_Flag = 0;    //雷达液位测量传感器存在标志位
    #ifndef WATERLEVEL_RADAR_PRESS
    //uint8_t ucWaterLevel_Pressure_Flag = 0; //压力液位测量传感器存在标志位
    #endif
    //uint8_t ucWaterQuality_COD_Flag = 0; //水质COD测量传感器存在标志位
    //uint8_t ucWaterQuality_COND_Flag = 0; //水质电导率测量传感器存在标志位
    //uint8_t ucMeasWaterVolumeFlag = 0; //流量测量传感器存在标志位
    uint8_t ucRightValueCnt = 0;
    double dRightValueSum = 0.0;
    //uint8_t j = 0;
    uint8_t k = 0;

    uint16_t usDataLen = 0;
    unsigned short usDataLength = 0;
    //uint8_t ucMeasWaterVolumeFlag = 0;
    uint8_t i = 0;
    uint8_t l = 0;
    int nPosi = 0;
    
    //time(&now);
    //struct tm tm;
    //time(&now);
    //drv_mcu_Get_RTC_Time(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime);
    //sscanf(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    //tm.tm_year -= 1900; // 由于tm_year是从1900年开始计数的
    //tm.tm_mon -= 1;     // tm_mon是从0开始的，所以需要减1
    //tm.tm_isdst = -1;
    //now = mktime(&tm) - 8*60*60; //将时间转换为UTC时间，减去8小时

    //if(pst_EC200USystemPara->DeviceRunPara.ulUploadRecordLostCnt == pst_EC200USystemPara->DevicePara.nDeviceUploadCnt / pst_EC200USystemPara->DevicePara.nDeviceSaveRecordCnt)
    {
    //    pst_EC200USystemPara->DeviceRunPara.ulUploadRecordStartTime = (long)now; //记录上传数据开始时间
    }
    //if(pst_EC200USystemPara->DeviceRunPara.cBarTouchFlag == 3)
    {
    //    ulTime = pst_EC200USystemPara->DeviceRunPara.ulBarTouchEventUploadTime;
    }
    //else
    {
    //    ulTime = pst_EC200USystemPara->DeviceRunPara.ulUploadRecordStartTime;
    }

    sprintf((char *)ucTempArr, "{\"clientId\":\"%s\",\"pver\":24,\"beginTime\":%ld,\"msgId\":%ld,\"gap\":%d,\"data\":{", 
                                                                                        pst_EC200USystemPara->DevicePara.cDeviceID,
                                                                                        ulTime,
                                                                                        (long)now,
                                                                                        pst_EC200USystemPara->DevicePara.nDeviceSaveRecordCnt * 60);
    //根据当前设备上传记录间隔与采用间隔；当前滞留计数，计算本次需要上传的数据个数
    //ulRecordCnt = 0;//pst_EC200USystemPara->DevicePara.nDeviceUploadCnt / pst_EC200USystemPara->DevicePara.nDeviceSaveRecordCnt;
    //ulRecordCnt = pst_EC200USystemPara->DeviceRunPara.ulUploadRecordLostCnt;
    //if(ulRecordCnt >= 10)
    {
    //    ucRecordCnt = 10;
    }
    //else
    {
    //    ucRecordCnt = (uint8_t)ulRecordCnt;
    }
    //if(pst_EC200USystemPara->DeviceRunPara.cBarTouchFlag == 3)
    {
    //    ucRecordCnt = 1;
    }
    //pst_EC200USystemPara->DeviceRunPara.ucCurUploadRecordCnt = ucRecordCnt; //记录本次上传数据个数

    if(ucWaterLevel_Radar_Flag == 1)
    {
        (void)strcat((char *)ucTempArr, (char *)ucBaseDataValueArr[0]);
    }

    if(ucWaterQuality_COD_Flag == 1)
    {
        (void)strcat((char *)ucTempArr, (char *)ucBaseDataValueArr[1]);
    }
    if(ucWaterQuality_COND_Flag == 1)
    {
        (void)strcat((char *)ucTempArr, (char *)ucBaseDataValueArr[2]);
    }
    if(ucMeasWaterVolumeFlag == 1)
    {
        (void)strcat((char *)ucTempArr, (char *)ucBaseDataValueArr[3]);
        (void)strcat((char *)ucTempArr, (char *)ucBaseDataValueArr[4]);
        (void)strcat((char *)ucTempArr, (char *)ucBaseDataValueArr[5]);
    }
    memcpy(&ucTempArr[strlen((char *)ucTempArr)-1], "}}", 2);
    usDataLen = strlen((char *)ucTempArr);
    l = usDataLen % 16;
    if(l != 0)
    {
        usDataLength = usDataLen + 16-l;
    }
    //drv_LKT4202_SendData_Encry(&ucTempArr[0], (char*)ucEntryArr,usDataLength);
    //usDataLen = strlen((char *)ucEntryArr);

    memcpy(ucDataArr, ucTempArr, usDataLen);
    return usDataLen;                          
}
#endif
//获取设备监测项数据上报报文
uint16_t func_Get_StatusUploadCMD_Data(uint8_t *ucDataArr)
{
    uint8_t ucTempArr[2300] = {0};
    uint8_t ucEntryArr[2300] = {0};
    //uint8_t ucTempValueArr[12][150] = {{0}}; //用于存储数据
    uint8_t ucTempAttiArr[50] = {0}; //用于存储姿态数据
    uint8_t ucTempWaterImmersionArr[50] = {0}; //用于存储水浸数据
    uint8_t ucTempPhotosensitiveArr[50] = {0}; //用于存储光照数据
    uint8_t ucBT_ConnArr[50] = {0}; //用于存储设备蓝牙连接状态数据
    uint8_t ucDebugModelArr[50] = {0};  //用于存储设备是否开启调试模式数据
    uint8_t ucLongPowerModelArr[50] = {0};  //用于存储设备是否开启长供电模式数据
    uint8_t ucDevTempArr[150] = {0};    //用于存储设备内部MCU温度数据
    uint8_t ucCSQArr[50] = {0};    //用于存储设备信号强度数据
    uint8_t ucBatteryArr[150] = {0};    //用于存储设备电池电量数据
    uint8_t ucNFCFlag[50] = {0};    //用于存储设备NFC状态数据
    //uint8_t ucParaNameArr[10] = {0};
    //uint8_t ucWaterLevel_Radar_Flag = 0;    //雷达液位测量传感器存在标志位
    #ifndef WATERLEVEL_RADAR_PRESS
    uint8_t ucWaterLevel_Pressure_Flag = 0; //压力液位测量传感器存在标志位
    #endif
    //uint8_t ucWaterQuality_COD_Flag = 0; //水质COD测量传感器存在标志位
    //uint8_t ucWaterQuality_COND_Flag = 0; //水质电导率测量传感器存在标志位
    //uint8_t ucMeasWaterVolumeFlag = 0; //流量测量传感器存在标志位
    //uint8_t ucRightValueCnt = 0;
    //double dRightValueSum = 0.0;
    //uint8_t j = 0;
    //uint8_t k = 0;

    uint16_t usDataLen = 0;
    unsigned short usDataLength = 0;
    //uint8_t ucMeasWaterVolumeFlag = 0;
    //uint8_t i = 0;
    uint8_t l = 0;
    int nPosi = 0;
    //char cCheckCnt,cn;
    //uint8_t ucRecordCnt = 0;
    //DevMeasRecordDataSt st_TempValue[10];
    //time(&now);
    //struct tm tm;
    //time(&now);
    //drv_mcu_Get_RTC_Time(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime);
    //sscanf(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    //tm.tm_year -= 1900; // 由于tm_year是从1900年开始计数的
    //tm.tm_mon -= 1;     // tm_mon是从0开始的，所以需要减1
    //tm.tm_isdst = -1;
    //now = mktime(&tm) - 8*60*60; //将时间转换为UTC时间，减去8小时

    //if(pst_EC200USystemPara->DeviceRunPara.ulUploadRecordLostCnt == pst_EC200USystemPara->DevicePara.nDeviceUploadCnt / pst_EC200USystemPara->DevicePara.nDeviceSaveRecordCnt)
    {
    //    pst_EC200USystemPara->DeviceRunPara.ulUploadRecordStartTime = (long)now; //记录上传数据开始时间
    }
    //if(pst_EC200USystemPara->DeviceRunPara.cBarTouchFlag == 3)
    {
    //    ulTime = pst_EC200USystemPara->DeviceRunPara.ulBarTouchEventUploadTime;
    }
    //else
    {
    //    ulTime = pst_EC200USystemPara->DeviceRunPara.ulUploadRecordStartTime;
    }

    sprintf((char *)ucTempArr, "{\"clientId\":\"%s\",\"pver\":24,\"beginTime\":%ld,\"msgId\":%ld,\"gap\":%d,\"data\":{", 
                                                                                        pst_EC200USystemPara->DevicePara.cDeviceID,
                                                                                        ulTime,
                                                                                        (long)now,
                                                                                        pst_EC200USystemPara->DevicePara.nDeviceSaveRecordCnt * 60);
    //根据当前设备上传记录间隔与采用间隔；当前滞留计数，计算本次需要上传的数据个数
    //ulRecordCnt = 0;//pst_EC200USystemPara->DevicePara.nDeviceUploadCnt / pst_EC200USystemPara->DevicePara.nDeviceSaveRecordCnt;
    //ulRecordCnt = pst_EC200USystemPara->DeviceRunPara.ulUploadRecordLostCnt;
    //if(ulRecordCnt >= 10)
    {
    //    ucRecordCnt = 10;
    }
    //else
    {
    //    ucRecordCnt = (uint8_t)ulRecordCnt;
    }
    //if(pst_EC200USystemPara->DeviceRunPara.cBarTouchFlag == 3)
    {
    //    ucRecordCnt = 1;
    }
    //pst_EC200USystemPara->DeviceRunPara.ucCurUploadRecordCnt = ucRecordCnt; //记录本次上传数据个数

    memcpy(&ucTempAttiArr[0], "\"attitude\":[", 12);
    memcpy(&ucTempWaterImmersionArr[0], "\"water_immersion\":[", 19);
    memcpy(&ucTempPhotosensitiveArr[0], "\"photosensitive\":[", 18);
    memcpy(&ucBT_ConnArr[0], "\"BT_Connected\":[", 16);
    memcpy(&ucDebugModelArr[0], "\"Debug_Model\":[", 15);
    memcpy(&ucLongPowerModelArr[0], "\"LongPowerModel\":[", 19);
    memcpy(&ucDevTempArr[0], "\"DevTemperature\":[", 18);
    memcpy(&ucBatteryArr[0], "\"batV\":[", 8);
    memcpy(&ucCSQArr[0], "\"CSQ\":[", 7);
    memcpy(&ucNFCFlag[0], "\"NFC\":[", 7);

    if(pst_EC200USystemPara->DeviceRunPara.cBarTouchFlag == 0)
    {
        if(pst_EC200USystemPara->DeviceRunPara.cBlackLightFlag == 1)
        {
            ucRecordCnt = 1;
        }
        //根据本次要上传的数据个数，从存储中读取该数据
        for(nPosi=0; nPosi<ucRecordCnt; nPosi++)
        {
            sprintf((char *)&ucTempAttiArr[strlen((char *)ucTempAttiArr)], "%d,", st_TempValue[nPosi].nAttitude_SC7A);
            sprintf((char *)&ucTempWaterImmersionArr[strlen((char *)ucTempWaterImmersionArr)], "%d,", st_TempValue[nPosi].cWater_Immersion_Status);
            sprintf((char *)&ucTempPhotosensitiveArr[strlen((char *)ucTempPhotosensitiveArr)], "%d,", st_TempValue[nPosi].cPhotosensitive_XYC_ALS_Status);
            sprintf((char *)&ucBT_ConnArr[strlen((char *)ucBT_ConnArr)], "%d,", st_TempValue[nPosi].cBT_ConnectFlag);
            sprintf((char *)&ucDebugModelArr[strlen((char *)ucDebugModelArr)], "%d,", st_TempValue[nPosi].cDebug_Model);
            sprintf((char *)&ucLongPowerModelArr[strlen((char *)ucLongPowerModelArr)], "%d,", st_TempValue[nPosi].cLongPowerModel);
            sprintf((char *)&ucDevTempArr[strlen((char *)ucDevTempArr)], "%.3lf,", (double)st_TempValue[nPosi].fDevTemperature);
            sprintf((char *)&ucBatteryArr[strlen((char *)ucBatteryArr)], "%.3lf,", (double)pst_EC200USystemPara->DeviceRunPara.esDeviceRunState.fBattleVoltage);
            sprintf((char *)&ucCSQArr[strlen((char *)ucCSQArr)], "%d,", pst_EC200USystemPara->DeviceRunPara.nSignalStrength);
            sprintf((char *)&ucNFCFlag[strlen((char *)ucNFCFlag)], "%d,", 0); 
        }
        
    
    }
    else
    {
        sprintf((char *)&ucTempAttiArr[strlen((char *)ucTempAttiArr)], "%d,", gSt_DevMeasRecordData.nAttitude_SC7A);
        sprintf((char *)&ucTempWaterImmersionArr[strlen((char *)ucTempWaterImmersionArr)], "%d,", gSt_DevMeasRecordData.cWater_Immersion_Status);
        sprintf((char *)&ucTempPhotosensitiveArr[strlen((char *)ucTempPhotosensitiveArr)], "%d,", gSt_DevMeasRecordData.cPhotosensitive_XYC_ALS_Status);
        sprintf((char *)&ucBT_ConnArr[strlen((char *)ucBT_ConnArr)], "%d,", gSt_DevMeasRecordData.cBT_ConnectFlag);
        sprintf((char *)&ucDebugModelArr[strlen((char *)ucDebugModelArr)], "%d,", gSt_DevMeasRecordData.cDebug_Model);
        sprintf((char *)&ucLongPowerModelArr[strlen((char *)ucLongPowerModelArr)], "%d,", gSt_DevMeasRecordData.cLongPowerModel);
        sprintf((char *)&ucDevTempArr[strlen((char *)ucDevTempArr)], "%.3lf,", (double)gSt_DevMeasRecordData.fDevTemperature);
        sprintf((char *)&ucBatteryArr[strlen((char *)ucBatteryArr)], "%.3lf,", (double)pst_EC200USystemPara->DeviceRunPara.esDeviceRunState.fBattleVoltage);
        sprintf((char *)&ucCSQArr[strlen((char *)ucCSQArr)], "%d,", pst_EC200USystemPara->DeviceRunPara.nSignalStrength);
        sprintf((char *)&ucNFCFlag[strlen((char *)ucNFCFlag)], "%d,", 1); 
    }

    if(ucRecordCnt > 0)
    {
        memcpy(&ucTempAttiArr[strlen((char *)ucTempAttiArr)-1], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucTempAttiArr);
        memcpy(&ucTempWaterImmersionArr[strlen((char *)ucTempWaterImmersionArr)-1], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucTempWaterImmersionArr);
        memcpy(&ucTempPhotosensitiveArr[strlen((char *)ucTempPhotosensitiveArr)-1], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucTempPhotosensitiveArr);
        memcpy(&ucBT_ConnArr[strlen((char *)ucBT_ConnArr)-1], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucBT_ConnArr);
        memcpy(&ucDebugModelArr[strlen((char *)ucDebugModelArr)-1], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucDebugModelArr);
        memcpy(&ucLongPowerModelArr[strlen((char *)ucLongPowerModelArr)-1], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucLongPowerModelArr);
        memcpy(&ucDevTempArr[strlen((char *)ucDevTempArr)-1], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucDevTempArr);
        memcpy(&ucBatteryArr[strlen((char *)ucBatteryArr)-1], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucBatteryArr);
        memcpy(&ucNFCFlag[strlen((char *)ucNFCFlag)-1], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucNFCFlag);
        memcpy(&ucCSQArr[strlen((char *)ucCSQArr)-1], "]}}", 3);
        (void)strcat((char *)ucTempArr, (char *)ucCSQArr);
    }
    else
    {
        memcpy(&ucTempAttiArr[strlen((char *)ucTempAttiArr)], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucTempAttiArr);
        memcpy(&ucTempWaterImmersionArr[strlen((char *)ucTempWaterImmersionArr)], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucTempWaterImmersionArr);
        memcpy(&ucTempPhotosensitiveArr[strlen((char *)ucTempPhotosensitiveArr)], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucTempPhotosensitiveArr);
        memcpy(&ucBT_ConnArr[strlen((char *)ucBT_ConnArr)], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucBT_ConnArr);
        memcpy(&ucDebugModelArr[strlen((char *)ucDebugModelArr)], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucDebugModelArr);
        memcpy(&ucLongPowerModelArr[strlen((char *)ucLongPowerModelArr)], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucLongPowerModelArr);
        memcpy(&ucDevTempArr[strlen((char *)ucDevTempArr)], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucDevTempArr);
        memcpy(&ucBatteryArr[strlen((char *)ucBatteryArr)], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucBatteryArr);
        memcpy(&ucNFCFlag[strlen((char *)ucNFCFlag)], "],", 2);
        (void)strcat((char *)ucTempArr, (char *)ucNFCFlag);
        memcpy(&ucCSQArr[strlen((char *)ucCSQArr)], "]}}", 3);
        (void)strcat((char *)ucTempArr, (char *)ucCSQArr);
    }

    usDataLen = strlen((char *)ucTempArr);
    //#ifdef SM4_ENTRY_ENABLE
    if(pst_EC200USystemPara->DevicePara.cSM4EntryFlag == 1)
    {
        l = usDataLen % 16;
        if(l != 0)
        {
            usDataLength = usDataLen + 16-l;
        }
        drv_LKT4202_SendData_Encry(&ucTempArr[0], (char*)ucEntryArr,usDataLength);
        unsigned short usCRC16 = CRC16_SM4(ucEntryArr, usDataLength);
        memcpy(&ucEntryArr[usDataLength], &usCRC16, 2);
        usDataLength += 2;
        memcpy(ucDataArr, ucEntryArr, usDataLength);
        usDataLen = usDataLength;
    }
    //#else
    //usDataLen = strlen((char *)ucEntryArr);
    else
    {
        memcpy(ucDataArr, ucTempArr, usDataLen);
    }
    
    //#endif
    return usDataLen;                          
}

void func_Get_DevStatusData(void)
{
    time_t now;
    #if 0
    struct tm tm;
    //time(&now);
    drv_mcu_Get_RTC_Time(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime);
    sscanf(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    tm.tm_year -= 1900; // 由于tm_year是从1900年开始计数的
    tm.tm_mon -= 1;     // tm_mon是从0开始的，所以需要减1
    tm.tm_isdst = -1;
    now = mktime(&tm) - 8*60*60; 
    #else
    now = func_Get_Linux_Time_Sec();
    #endif
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
    //pst_EC200USystemPara->DeviceRunPara.esLostStatusArr[pst_EC200USystemPara->DeviceRunPara.cLostStatusArrCnt].fBattleVoltage = pst_EC200USystemPara->DeviceRunPara.esDeviceRunState.fBattleVoltage;
    memcpy(pst_EC200USystemPara->DeviceRunPara.esLostStatusArr[pst_EC200USystemPara->DeviceRunPara.cLostStatusArrCnt].cDevICCID,pst_EC200USystemPara->DeviceRunPara.cICCID,20);
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
    uint8_t ucEntryArr[800] = {0};
    uint8_t ucTempValueArr[6][150] = {{0}}; //用于存储数据
    char nPosi = 0;
    unsigned short usDataLength = 0;
    uint8_t l = 0;
    long lValue = pst_EC200USystemPara->DeviceRunPara.cDevUploadStatusTime * 60 * 60;

    time_t now;
    #if 0
    struct tm tm;
    //time(&now);
    drv_mcu_Get_RTC_Time(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime);
    sscanf(pst_EC200USystemPara->DeviceRunPara.cDeviceCurDateTime, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    tm.tm_year -= 1900; // 由于tm_year是从1900年开始计数的
    tm.tm_mon -= 1;     // tm_mon是从0开始的，所以需要减1
    tm.tm_isdst = -1;
    now = mktime(&tm) - 8*60*60; 
    #else
    now = func_Get_Linux_Time_Sec();
    #endif
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
    memcpy(&ucTempValueArr[2], "\"ICCID\":[", 9);
    memcpy(&ucTempValueArr[3], "\"lng\":[", 7);
    memcpy(&ucTempValueArr[4], "\"lat\":[", 7);
    memcpy(&ucTempValueArr[5], "\"status\":[", 10);
    if(pst_EC200USystemPara->DeviceRunPara.ulUploadStatusLostCnt < 1)
    {
        pst_EC200USystemPara->DeviceRunPara.ulUploadStatusLostCnt = 1;
    }
    for(nPosi=0; nPosi<pst_EC200USystemPara->DeviceRunPara.ulUploadStatusLostCnt; nPosi++)
    {
        //pst_EC200USystemPara->DeviceRunPara.esLostStatusArr[nPosi].usDevStatus = 0x0103;
        sprintf((char *)&ucTempValueArr[0][strlen((char *)ucTempValueArr[0])], "%s\",\"", pst_EC200USystemPara->DeviceRunPara.esLostStatusArr[nPosi].cDevStartDateTime);
        sprintf((char *)&ucTempValueArr[1][strlen((char *)ucTempValueArr[1])], "%d,", pst_EC200USystemPara->DeviceRunPara.esLostStatusArr[nPosi].nDevStartDays);
        sprintf((char *)&ucTempValueArr[2][strlen((char *)ucTempValueArr[2])], "%s,", pst_EC200USystemPara->DeviceRunPara.esLostStatusArr[nPosi].cDevICCID);
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
    //#ifdef SM4_ENTRY_ENABLE
    if(pst_EC200USystemPara->DevicePara.cSM4EntryFlag == 1)
    {
        l = usDataLen % 16;
        if(l != 0)
        {
            usDataLength = usDataLen + 16-l;
        }
        
        drv_LKT4202_SendData_Encry(&ucTempArr[0], (char*)ucEntryArr,usDataLength);
        unsigned short usCRC16 = CRC16_SM4(ucEntryArr, usDataLength);
        memcpy(&ucEntryArr[usDataLength], &usCRC16, 2);
        usDataLength += 2;
        memcpy(ucDataArr, ucEntryArr, usDataLength);
        usDataLen = usDataLength;
    }
    //#else
    else
    {
        memcpy(ucDataArr, ucTempArr, usDataLen);
    }
    
    //#endif
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
    uint8_t ucTempSendBuf[200] = {0};
    unsigned short usTempSendLen = 0;
    //uint8_t ucRecvBuf[EC200U_BUF_SIZE] = {0};
    uint16_t usRecvTimeOutCnt = 0;
    static uint8_t ucRetryCnt = 0;
    //uint8_t ucFlag = 0;
    uint16_t usRecvLen = 0;
    unsigned short usPosition = 0;
    char cCSQ[2] = {0};
    char cICCID[21] = {0};
    int nClientType = 4;

    if(pst_EC200USystemPara->DeviceRunPara.cBlackLightFlag == 0)
    {
        nClientType = 4;
    }
    else
    {
        nClientType = 7;
    }

    //if((ucDataUploadEnable == 0) )
    //{
    //    u8Temp = 1;
    //}
    //else
    //{
        u8Temp = 0; //如果是数据上传阶段，则不需要等待
    //}

    do
    {
        if((gE_4G_Module_Init_CMD >= Module_TEST_ATE0_CMD) || (gE_4G_Module_Init_CMD <= Module_DCE_RST_STAGE2))
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
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_Init;
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
            #if 0
        case Module_TEST_AT_CMD: //测试AT指令
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_AT_Test;
            (void)strcpy((char *)ucSendBuf, "AT\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
            #endif
        case Module_TEST_ATE0_CMD: //测试AT指令
            (void)strcpy((char *)ucSendBuf, "ATE0\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_QUERY_ICCID_CMD: //查询ICCID
            (void)strcpy((char *)ucSendBuf, "AT+QCCID\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "+QCCID:");
            break;
            #if 0
        case Module_QUERY_SIM_CARD_STATE_CMD: //查询SIM卡状态
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_SIM_Status;
            (void)strcpy((char *)ucSendBuf, "AT+CPIN?\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "+CPIN: READY");
            break;
            #endif
        case Module_QUERY_SIGNAL_STRENGTH_CMD: //查询信号强度
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_Signal_Strength;
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
        #if 0
        case Module_QUERY_PS_DOMAIN_REG_STATE_CMD://查询PS域注册状态：0：未注册，1/5：注册，2：正在搜索
            //(void)strcpy((char *)ucSendBuf, "AT+CREG?\r\n");    //CS域，只使用在2G网络上
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_Register;
            (void)strcpy((char *)ucSendBuf, "AT+CGREG?\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "+CGREG: 0,1");
            break;
        #endif
        case Module_ACTIVATE_NETWORK_CMD: //激活网络
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_Active_Network;
            (void)strcpy((char *)ucSendBuf, "AT+CGATT=1\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
            #if 0
        case Module_QUERY_NETWORK_ACTIVATE_STATE_CMD: //查询网络激活状态
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_Check_Network;
            (void)strcpy((char *)ucSendBuf, "AT+CGATT?\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "+CGATT: 1");
            break;  
        
        case Module_QUERY_IMSI_CMD: //查询IMSI号
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_Get_IMSI;
            (void)strcpy((char *)ucSendBuf, "AT+CIMI\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_QUERY_IMEI_CMD: //查询IMEI号
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_Get_IMEI;
            (void)strcpy((char *)ucSendBuf, "AT+CGSN\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
            #endif
        case Module_QUERY_LOCAL_DATE_TIME_CMD: //查询本地日期时间
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_Get_DateTime;
            (void)strcpy((char *)ucSendBuf, "AT+QLTS=2\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            func_WatchDog_Refresh();
            break;
        //MQTT配置  
        case Module_SET_DATA_FORMAT_CMD: //设置数据格式
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_MQTT_SET_Data_Format;
            (void)strcpy((char *)ucSendBuf, "AT+QMTCFG=\"recv/mode\",0,0,1\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);

            drv_mcu_USART_SendData(MODULE_4G_NB, ucSendBuf, usSendDataLen);
            Ddl_Delay1ms(200);
            pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB] = 0;
            memset(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], 0, USART_DATA_LEN_MAX);

            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_MQTT_Set_KeepAlive_Time;
            (void)strcpy((char *)ucSendBuf, "AT+QMTCFG=\"keepalive\",0,120\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);

            drv_mcu_USART_SendData(MODULE_4G_NB, ucSendBuf, usSendDataLen);
            Ddl_Delay1ms(200);
            pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB] = 0;
            memset(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], 0, USART_DATA_LEN_MAX);

            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_MQTT_Set_Version;
            (void)strcpy((char *)ucSendBuf, "AT+QMTCFG=\"version\",0,4\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);

            sprintf((char *)ucRecvCheckData, "OK");
            break;
            #if 0
        case Module_SET_MQTT_KEEPALIVE_TIME_CMD:    //心跳时间建议60s~300s.这里设置120s
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_MQTT_Set_KeepAlive_Time;
            (void)strcpy((char *)ucSendBuf, "AT+QMTCFG=\"keepalive\",0,120\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_SET_MQTT_VERSION_CMD:   //设置MQTT 版本
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_MQTT_Set_Version;
            (void)strcpy((char *)ucSendBuf, "AT+QMTCFG=\"version\",0,4\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
            #endif
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
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_MQTT_Open_Interface;
            (void)sprintf((char *)ucSendBuf, "AT+QMTOPEN=0,\"%s\",%d\r\n",pst_EC200USystemPara->DevicePara.cServerIP[0], pst_EC200USystemPara->DevicePara.usServerPort[0]);
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
            (void)sprintf((char *)ucSendBuf, "AT+QMTCONN=0,\"%s\",\"xfgd\",\"xfgd@1234\"\r\n",pst_EC200USystemPara->DevicePara.cDeviceID);
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_MQTT_Connect_Interface;
           // (void)sprintf((char *)ucSendBuf, "AT+QMTCONN=0,\"%s\",\"nbwater\",\"nbwater@1234\"\r\n",pst_EC200USystemPara->DevicePara.cDeviceID);
            //(void)strcpy((char *)ucSendBuf, "AT+QMTCONN=0,\"ZCJ2025042801\",\"iVOw212I78\",\"version=2018-10-31&res=products\%2FiVOw212I78\%2Fdevices\%2FZCJ2025042801&et=1749953701&method=md5&sign=FXcY9ZA%2BJhyld8Bd3VrpQg\%3D\%3D\"\r\n");
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        #ifdef CONNECT_MQTT
                #if 0
        case Module_SUBSCRIBE_TOPIC_REGISTER_CMD:   //订阅主题注册
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_MQTT_Subscribe_Topic_Register;
            sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0000/000%d/devReg/%s\",2\r\n",nClientType,pst_EC200USystemPara->DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_SUBSCRIBE_TOPIC_DATAUPLOAD_CMD: //订阅主题数据上传
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_MQTT_Subscribe_Topic_DataUpload;
            sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0000/000%d/dataUpload/%s\",2\r\n",nClientType,pst_EC200USystemPara->DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_SUBSCRIBE_TOPIC_STATUS_CMD: //订阅主题状态上报
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_MQTT_Subscribe_Topic_StatusUpload;
            sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0000/000%d/devStatus/%s\",2\r\n",nClientType,pst_EC200USystemPara->DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        #if 0
        case Module_SUBSCRIBE_TOPIC_ALARMDATA_CMD: //订阅主题报警数据上报
            sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0000/000%d/alarmData/%s\",2\r\n",nClientType,pst_EC200USystemPara->DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        #endif
        case Module_SUBSCRIBE_TOPIC_SETCONFIG_CMD:  //订阅主题设置配置
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_MQTT_Subscribe_Topic_SetPara;
            sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0000/000%d/setConfig/%s\",2\r\n",nClientType,pst_EC200USystemPara->DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
        case Module_SUBSCRIBE_TOPIC_GETCONFIG_CMD:  //订阅主题获取配置
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_MQTT_Subscribe_Topic_GetPara;
            sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0000/000%d/getConfig/%s\",2\r\n",nClientType,pst_EC200USystemPara->DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            pst_EC200USystemPara->DeviceRunPara.st_BlackLightData.ucCurPhotoDataCnt = 0;
            break;
                #else
        case Module_SUBSCRIBE_TOPIC_REGISTER_CMD:   //订阅主题注册
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_MQTT_Subscribe_Topic_Register;
            sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0000/000%d/devReg/%s\",2,\"data/down/0000/000%d/dataUpload/%s\",2,\"data/down/0000/000%d/devStatus/%s\",2,\"data/down/0000/000%d/setConfig/%s\",2,\"data/down/0000/000%d/getConfig/%s\",2\r\n",
            nClientType,pst_EC200USystemPara->DevicePara.cDeviceID,
            nClientType,pst_EC200USystemPara->DevicePara.cDeviceID,
            nClientType,pst_EC200USystemPara->DevicePara.cDeviceID,
            nClientType,pst_EC200USystemPara->DevicePara.cDeviceID,
            nClientType,pst_EC200USystemPara->DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
                #endif
        #if 0
        case Module_SUBSCRIBE_TOPIC_GETDATA_CMD:    //订阅主题获取数据
            sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0000/000%d/getData/%s\",2\r\n",nClientType,pst_EC200USystemPara->DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;
       
        case Module_SUBSCRIBE_DATAPT_CMD: //订阅主题数据透传
            sprintf((char *)ucSendBuf, "AT+QMTSUB=0,2,\"data/down/0000/000%d/dataPt/%s\",2\r\n",nClientType,pst_EC200USystemPara->DevicePara.cDeviceID);
            usSendDataLen = strlen((char *)ucSendBuf);
            sprintf((char *)ucRecvCheckData, "OK");
            break;

        #endif
        case Module_PUBLISH_TOPIC_REGISTER_CMD: //发布主题注册
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_MQTT_Publish_Topic_Register;
            usSendDataLen = func_Get_DevRegCMD_Data(ucSendBuf);
            sprintf((char *)ucTempSendBuf, "AT+QMTPUBEX=0,0,0,0,\"data/up/0000/000%d/devReg/%s\",%d\r\n",nClientType,pst_EC200USystemPara->DevicePara.cDeviceID,usSendDataLen);
            usTempSendLen = strlen((char *)ucTempSendBuf);
            //pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_Upload;
            drv_mcu_USART_SendData(MODULE_4G_NB, ucTempSendBuf, usTempSendLen);
            Ddl_Delay1ms(400);
            pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB] = 0;
            memset(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], 0, USART_DATA_LEN_MAX);
            memset(ucTempSendBuf, 0, 200);
            //usSendDataLen = func_Get_DevRegCMD_Data(ucSendBuf);
            memcpy(ucSendBuf+usSendDataLen, "\r\n", 2);
            usSendDataLen += 2;
            sprintf((char *)ucRecvCheckData, "\"res\":");
            pst_EC200USystemPara->DeviceRunPara.c4GUploadDataContinueFlag = 1;
            break;
        case Module_PUBLISH_TOPIC_DATAUPLOAD_CMD:   //发布监测项数据上报主题 
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_MQTT_Publish_Topic_DataUoload;
            if(pst_EC200USystemPara->DeviceRunPara.cBlackLightFlag == 1)
            {
                pst_EC200USystemPara->DeviceRunPara.c4GUploadDataContinueFlag = 1;
            }
            
            if(pst_EC200USystemPara->DeviceRunPara.cBlackLightFlag == 0)
            {
                usSendDataLen = func_Get_DataUploadCMD_Data(ucSendBuf);
            }
            else
            {
                usSendDataLen = func_Get_BlackLightDataUploadCMD_Data(ucSendBuf,&pst_EC200USystemPara->DeviceRunPara.st_BlackLightData.ucCurPhotoDataCnt);
            }
            
            sprintf((char *)ucTempSendBuf, "AT+QMTPUBEX=0,0,0,0,\"data/up/0000/000%d/dataUpload/%s\",%d\r\n",nClientType,pst_EC200USystemPara->DevicePara.cDeviceID,usSendDataLen);
            usTempSendLen = strlen((char *)ucTempSendBuf);
            //pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_Upload;
            drv_mcu_USART_SendData(MODULE_4G_NB, ucTempSendBuf, usTempSendLen);
            Ddl_Delay1ms(400);
            pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB] = 0;
            memset(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], 0, USART_DATA_LEN_MAX);
            memset(ucTempSendBuf, 0, 200);
            //usSendDataLen = func_Get_DataUploadCMD_Data(ucSendBuf);
            memcpy(ucSendBuf+usSendDataLen, "\r\n", 2);
            usSendDataLen += 2;
            sprintf((char *)ucRecvCheckData, "\"res\":");
            break;
        #if 0
        case Module_PUBLISH_TOPIC_DATABASEUPLOAD_CMD:   //发布监测项数据上报主题 
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_MQTT_Publish_Topic_DataUoload;
            usSendDataLen = func_Get_BaseDataUploadCMD_Data(ucSendBuf);
            sprintf((char *)ucTempSendBuf, "AT+QMTPUBEX=0,0,0,0,\"data/up/0000/000%d/dataUpload/%s\",%d\r\n",nClientType,pst_EC200USystemPara->DevicePara.cDeviceID,usSendDataLen);
            usTempSendLen = strlen((char *)ucTempSendBuf);
            //pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_Upload;
            drv_mcu_USART_SendData(MODULE_4G_NB, ucTempSendBuf, usTempSendLen);
            Ddl_Delay1ms(400);
            pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB] = 0;
            memset(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], 0, USART_DATA_LEN_MAX);
            memset(ucTempSendBuf, 0, 200);
            //usSendDataLen = func_Get_BaseDataUploadCMD_Data(ucSendBuf);
            memcpy(ucSendBuf+usSendDataLen, "\r\n", 2);
            usSendDataLen += 2;
            sprintf((char *)ucRecvCheckData, "\"res\":");
            break;
        #endif
        case Module_PUBLISH_TOPIC_SATATUSUPLOAD_CMD:   //发布监测项状态数据上报主题 
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_MQTT_Publish_Topic_DataUoload;
            usSendDataLen = func_Get_StatusUploadCMD_Data(ucSendBuf);
            sprintf((char *)ucTempSendBuf, "AT+QMTPUBEX=0,0,0,0,\"data/up/0000/000%d/dataUpload/%s\",%d\r\n",nClientType,pst_EC200USystemPara->DevicePara.cDeviceID,usSendDataLen);
            usTempSendLen = strlen((char *)ucTempSendBuf);
            //pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_Upload;
            drv_mcu_USART_SendData(MODULE_4G_NB, ucTempSendBuf, usTempSendLen);
            Ddl_Delay1ms(400);
            pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB] = 0;
            memset(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], 0, USART_DATA_LEN_MAX);
            memset(ucTempSendBuf, 0, 200);
            //usSendDataLen = func_Get_StatusUploadCMD_Data(ucSendBuf);
            memcpy(ucSendBuf+usSendDataLen, "\r\n", 2);
            usSendDataLen += 2;
            sprintf((char *)ucRecvCheckData, "\"res\":");
            break;
        case Module_PUBLISH_TOPIC_STATUS_CMD:   //发布设备状态上报主题
            pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_MQTT_Publish_Topic_StatusUpload;
            usSendDataLen = func_Get_DevStatusCMD_Data(ucSendBuf);
            sprintf((char *)ucTempSendBuf, "AT+QMTPUBEX=0,0,0,0,\"data/up/0000/000%d/devStatus/%s\",%d\r\n",nClientType,pst_EC200USystemPara->DevicePara.cDeviceID,usSendDataLen);
            usTempSendLen = strlen((char *)ucTempSendBuf);
            //pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_Upload;
            drv_mcu_USART_SendData(MODULE_4G_NB, ucTempSendBuf, usTempSendLen);
            Ddl_Delay1ms(400);
            pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB] = 0;
            memset(pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], 0, USART_DATA_LEN_MAX);
            memset(ucTempSendBuf, 0, 200);
            //usSendDataLen = func_Get_DevStatusCMD_Data(ucSendBuf);
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

        if(gE_4G_Module_Init_CMD >= Module_TEST_ATE0_CMD)
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
            //if(gE_4G_Module_Init_CMD == Module_QUERY_IMSI_CMD)
            //{
            //    memset(&pst_EC200USystemPara->DevicePara.cDeviceIMSI[0], 0, 16);
            //    memcpy(&pst_EC200USystemPara->DevicePara.cDeviceIMSI[0], &pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB][2], 15);
            //}
            //else if(gE_4G_Module_Init_CMD == Module_QUERY_IMEI_CMD)
            {
            //    memset(&pst_EC200USystemPara->DevicePara.cDeviceIMEI[0], 0, 16);
            //    memcpy(&pst_EC200USystemPara->DevicePara.cDeviceIMEI[0], &pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB][2], 15);
            }
            if(gE_4G_Module_Init_CMD == Module_QUERY_LOCAL_DATE_TIME_CMD)
            {
                if(drv_mcu_Set_RTC_Time(&pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB][0]) == 1)
                {
                    return 5;
                }
                if((ucDataUploadEnable == 0) && (pst_EC200USystemPara->DevicePara.cDeviceRegisterFlag == 1))
                {
                    pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_OK;
                    return 0;
                }
                if(pst_EC200USystemPara->DevicePara.cMonitorMode == 1)
                {
                    pst_EC200USystemPara->DeviceRunPara.c4GInitFlag = 1;
                    pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_OK;
                    return 0;
                }
            }
            else if(gE_4G_Module_Init_CMD == Module_QUERY_ICCID_CMD)  //ICCID
            {
                memcpy(cICCID,&pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB][10], 20);
                memcpy(pst_EC200USystemPara->DeviceRunPara.cICCID, cICCID, 20);
            }
            else if(gE_4G_Module_Init_CMD == Module_QUERY_SIGNAL_STRENGTH_CMD)  //4G信号强度
            {
                memcpy(cCSQ,&pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB][8], 2);
                sscanf((char *)cCSQ, "%d", &pst_EC200USystemPara->DeviceRunPara.nSignalStrength);
            }
            else
            {
                //gE_4G_Module_Init_CMD++;
            }
            #ifdef CONNECT_MQTT
            if(gE_4G_Module_Init_CMD == Module_SUBSCRIBE_TOPIC_REGISTER_CMD)
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
                        pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_OK;
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
                        pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_OK;
                        return 0; //注册成功，且不需要上传数据
                    }
                }
            }
            else if(gE_4G_Module_Init_CMD == Module_PUBLISH_TOPIC_DATAUPLOAD_CMD)
            {
                if(func_Array_Find_Str((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],usRecvLen,"\"res\":0",7, &usPosition) == 0)
                {
                    if((pst_EC200USystemPara->DeviceRunPara.cBlackLightFlag == 0))
                    {
                        gE_4G_Module_Init_CMD++;
                        ucRetryCnt = 0;
                    }
                    else
                    {
                        ucRetryCnt = 0;
                        pst_EC200USystemPara->DeviceRunPara.st_BlackLightData.ucCurPhotoDataCnt++;
                        if(pst_EC200USystemPara->DeviceRunPara.st_BlackLightData.ucCurPhotoDataCnt >= pst_EC200USystemPara->DeviceRunPara.st_BlackLightData.ucPhotoDataCnt)
                        {
                            pst_EC200USystemPara->DeviceRunPara.st_BlackLightData.ucCurPhotoDataCnt = 0;
                            gE_4G_Module_Init_CMD++;
                        }
                    }
                }
            }
            else if(gE_4G_Module_Init_CMD == Module_PUBLISH_TOPIC_SATATUSUPLOAD_CMD)
            {
                //if (strstr((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], "\"res\":0") != NULL)
                if(func_Array_Find_Str((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],usRecvLen,"\"res\":0",7, &usPosition) == 0)
                {
                    if((pst_EC200USystemPara->DeviceRunPara.cBlackLightFlag == 0))
                    {
                        if(pst_EC200USystemPara->DeviceRunPara.cBarTouchFlag != 3)
                        {
                            //上传数据成功
                            if(pst_EC200USystemPara->DeviceRunPara.ucCurUploadRecordCnt == pst_EC200USystemPara->DeviceRunPara.ulUploadRecordLostCnt)
                            {
                                pst_EC200USystemPara->DeviceRunPara.ulUploadRecordLostCnt = 0; //上传数据成功，清零上传记录丢失计数
                                pst_EC200USystemPara->DeviceRunPara.ulUploadRecordStartTime = 0;
                            }
                            else
                            {
                                if((pst_EC200USystemPara->DeviceRunPara.ulUploadRecordLostCnt) >= pst_EC200USystemPara->DeviceRunPara.ucCurUploadRecordCnt)
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
                        if(pst_EC200USystemPara->DeviceRunPara.cBlackLightFlag == 1)
                        {
                            pst_EC200USystemPara->DeviceRunPara.st_BlackLightData.ucCurPhotoDataCnt++;
                        }
                        else
                        {
                            gE_4G_Module_Init_CMD++;
                            ucRetryCnt = 0;
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
                    pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_OK;
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
                    pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_OK;
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
            //if (gE_4G_Module_Init_CMD == Module_QUERY_PS_DOMAIN_REG_STATE_CMD)
            {
                //if (strstr((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], "+CGREG: 0,5") == NULL)
            //    if(func_Array_Find_Str((char *)pst_EC200USystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],pst_EC200USystemPara->UsartData.usUsartxRecvDataLen[0],"+CGREG:0,5",10, &usPosition) == 0)
                {
            //        u8Temp = 0;
            //        return 2;
                }
            //    else
                {
            //        gE_4G_Module_Init_CMD++;
                }
            }
            //else
            {
                //gE_4G_Module_Init_CMD++;
                
                Ddl_Delay1ms(200);
                if(ucRetryCnt >= 5) //重复发送三次
                {
                    ucRetryCnt = 0;
                    u8Temp = 0;
                    if(gE_4G_Module_Init_CMD < Module_SUBSCRIBE_TOPIC_REGISTER_CMD)
                    {
                        //if(gE_4G_Module_Init_CMD == Module_QUERY_SIM_CARD_STATE_CMD)
                        {
                        //    return 6;
                        }
                        //else
                        {
                            return 2;
                        }
                       
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
    //uint16_t usRecvTimeOutCnt = 0;
    static uint8_t ucFailedCnt = 0;
    static uint8_t ucInitFlag = 0;
    if(ucInitFlag == 0)
    {
        ucInitFlag = 1;
        pst_EC200USystemPara = GetSystemPara();
    }

    //if(ucDataUploadEnable == 1)
    {
        ucResult = EC200U_4G_Module_Configuration_Init(ucDataUploadEnable);
        if((ucResult == 0) || (ucResult == 2) || (ucResult == 6))
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
    #if 0
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
    #endif
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
