/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\MQTT_SK\MQTT_SK.c
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
#include "hc32f460_utility.h"
#include "MQTT_SK.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "drv_Storage_W25Q128.h"
#include "stddef.h"
#include "time.h"
#include "drv_USART.h"
#include "drv_RTC.h"
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
static SystemPataSt *pst_MQTTSystemPara;
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 * @brief  MQTT SK Device Regist.
 * @param  None
 * @retval 0: registed success 1: register failed 
 */
uint8_t MQTT_SK_Device_Regist(void)
{
	#if 0
	char cSendBuf[500] = {0};
	//char cRecvBuf[500] = {0};
	uint16_t usSendDataLen = 0;
	uint16_t usRecvTimeOutCnt = 0;
	uint8_t ucRecvCheckData[10] = {0};

	sprintf(cSendBuf, "{\"clientId\":\"%s\",\"pver\":1,\"time\":\"%s\",\"type\":\"devReg\",\"data\":{\"devRegReq\":{\"baseInfo\":{\"imsi\":\"%s\",\"imei\":\"%s\",\"hwVer\":\"%s\",\"swVer\":\"%s\"}}}}", 
					pst_MQTTSystemPara->DevicePara.cDeviceID, 
					pst_MQTTSystemPara->DeviceRunPara.cDeviceCurDateTime, 
					pst_MQTTSystemPara->DevicePara.cDeviceIMSI,
					pst_MQTTSystemPara->DevicePara.cDeviceIMEI,
					pst_MQTTSystemPara->DevicePara.cDeviceHWVersion,
					pst_MQTTSystemPara->DevicePara.cDeviceSWVersion);
	usSendDataLen = strlen((char *)cSendBuf);
	sprintf((char *)ucRecvCheckData, "res");
	//发送AT指令
	drv_mcu_USART_SendData(MODULE_4G_NB, (uint8_t*)cSendBuf, usSendDataLen);
	//等待接收到OK
	usRecvTimeOutCnt = 0;
	while(pst_MQTTSystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_4G_NB] == 0)
	{
		Ddl_Delay1ms(10);
		usRecvTimeOutCnt++;
		if(usRecvTimeOutCnt >= 500)
		{
			return 1;
		}
	}
	pst_MQTTSystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_4G_NB] = 0;
	 
	if (strstr((char *)pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], (char *)ucRecvCheckData) != NULL) //接收到的数据中包含OK
	{
		if(pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB][pst_MQTTSystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB]-4] == '0')
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
		#endif
	return 1;
}

/**
 * @brief  MQTT_SK_Data_Upload
 * @param  None
 * @retval 0: upload success 1: upload failed 
 */
uint8_t MQTT_SK_Data_Upload(void)
{
	char cSendBuf[500] = {0};
	uint8_t cSendtoServerBuf[1500]	= {0};
	uint16_t usSendDataLen1 = 0;
	uint16_t usSendDataLen2 = 0;
	uint16_t usRecvTimeOutCnt = 0;
	uint8_t ucRecvCheckData[10] = {0};
	uint8_t i = 0;
	int nRecordCnt = 0;

	if(pst_MQTTSystemPara->DeviceUploadData.nDeviceUploadRecordCntMax >= 10)
	{
		nRecordCnt = 10;
	}
	else
	{
		nRecordCnt = pst_MQTTSystemPara->DeviceUploadData.nDeviceUploadRecordCntMax;
	}

	sprintf(cSendBuf, "{\"clientId\":\"%s\",\"pver\":1,\"time\":\"%s\",\"type\":\"dataUpload\",\
		\"data\":{\"dataUploadReg\":{\"devStatus\":{\"timeStart\":\"%s\",\"timeRun\":%d,\"csq\":\
		%d,\"immersion\":%d,\"batV\":%.2lf,\"lng\":%.6lf,\"lat\":%.6lf},\"psgwData\":{\"timeBegin\":\
		\"%s\",\"gap\":{\"unit\":0,\"val\":%d},\"dataNum\":%d,\"drainage_water_level\":{\"water_height\":[", 
					pst_MQTTSystemPara->DevicePara.cDeviceID, 
					pst_MQTTSystemPara->DeviceRunPara.cDeviceCurDateTime, 
					pst_MQTTSystemPara->DeviceRunPara.esDeviceRunState.cDevStartDateTime,
					pst_MQTTSystemPara->DeviceRunPara.esDeviceRunState.nDevStartDays,
					pst_MQTTSystemPara->DeviceRunPara.esDeviceRunState.nDevSignalCSQ,
					pst_MQTTSystemPara->DeviceRunPara.esDeviceRunState.cDevimmersionFlag,
					(double)pst_MQTTSystemPara->DeviceRunPara.esDeviceRunState.fBattleVoltage,
					pst_MQTTSystemPara->DeviceRunPara.esDeviceRunState.fDevLoca_lng,
					pst_MQTTSystemPara->DeviceRunPara.esDeviceRunState.fDevLoca_lat,
					pst_MQTTSystemPara->DeviceUploadData.cDeviceStartUploadDateTime,
					pst_MQTTSystemPara->DevicePara.nDeviceSaveRecordCnt,
					nRecordCnt
					);
	usSendDataLen1 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf,cSendBuf,usSendDataLen1);
	memset(cSendBuf, 0, 500);
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasLevelData[i].fWater_height);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}
	memcpy(cSendBuf+usSendDataLen2-1,"],\"pipeline_water_height\":[",27);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasLevelData[i].fPipeWater_height);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"],\"water_elevation\":[",21);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasLevelData[i].fWater_elevation);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"]},\"drainage_flow\":{\"curr_volume\":[",35);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasFlowData[i].fCurr_volume_h);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"],\"curr_volume_s\":[",19);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasFlowData[i].fCurr_volume_s);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"],\"flow_velocity\":[",19);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasFlowData[i].fFlow_velocity);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"],\"surface_flow_velocoty\":[",27);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasFlowData[i].fSurface_velocity);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"],\"total_volume\":[",18);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasFlowData[i].fTotal_volume);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"]},\"drainage_quality\":{\"PH\":[",29);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasWaterQualityData[i].fPH);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"],\"TN\":[",8);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasWaterQualityData[i].fTN);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"],\"TP\":[",8);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasWaterQualityData[i].fTP);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"],\"NHN\":[",9);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasWaterQualityData[i].fNHN);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"],\"BOD\":[",9);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasWaterQualityData[i].fBOD);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"],\"COD\":[",9);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasWaterQualityData[i].fCOD);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"],\"SS\":[",8);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasWaterQualityData[i].fSS);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"],\"COND\":[",10);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasWaterQualityData[i].fCOND);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"],\"KRQ\":[",9);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasWaterQualityData[i].fKRQ);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"],\"O2\":[",8);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasWaterQualityData[i].fO2);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"],\"H2S\":[",9);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasWaterQualityData[i].fH2S);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"],\"CO\":[",8);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasWaterQualityData[i].fCO);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"],\"water_temperature\":[",23);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;
	memset(cSendBuf, 0, 500);
	usSendDataLen2 = 0;
	for(i=0; i<nRecordCnt; i++)
	{
		sprintf(cSendBuf+usSendDataLen2,"%.2lf,",(double)pst_MQTTSystemPara->DeviceUploadData.esMeasWaterQualityData[i].fWater_Temp);
		usSendDataLen2 = strlen((char *)cSendBuf);
	}

	memcpy(cSendBuf+usSendDataLen2-1,"]}}}}}",23);
	usSendDataLen2 = strlen((char *)cSendBuf);
	memcpy(cSendtoServerBuf+usSendDataLen1,cSendBuf,usSendDataLen2);
	usSendDataLen1 += usSendDataLen2;

	memcpy((char *)ucRecvCheckData, "\"msg\":200",9);
	//发送AT指令
	drv_mcu_USART_SendData(MODULE_4G_NB, cSendtoServerBuf, usSendDataLen1);
	//等待接收到OK
	usRecvTimeOutCnt = 0;
	while(pst_MQTTSystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_4G_NB] == 0)
	{
		SysTick_Delay(10);
		usRecvTimeOutCnt++;
		if(usRecvTimeOutCnt >= 500)
		{
			return 1;
		}
	}
	pst_MQTTSystemPara->UsartData.ucUsartxRecvDataFlag[MODULE_4G_NB] = 0;
	 
	if (strstr((char *)pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], (char *)ucRecvCheckData) != NULL) //接收到的数据中包含OK
	{
		return 0;
	}
	return 1;
}

/**
 * @brief  MQTT_SK_Set_Config
 * @param  None
 * @retval 0: set success 1: set failed 
 * 
 */

//拼接返回设置参数命令
uint16_t func_Get_SetConfigResult_CMD(uint8_t *ucDataArr, unsigned char ucResult, char* cmsgIdArr)
{
    uint8_t ucTempArr[300] = {0};
    uint16_t usDataLen = 0;
    //char c_Result[10] = {0};
    //char c_Result1[20] = {0};
    //time_t now;
    //struct tm tm;
	//drv_mcu_Get_RTC_Time(pst_MQTTSystemPara->DeviceRunPara.cDeviceCurDateTime);
    //sscanf(pst_MQTTSystemPara->DeviceRunPara.cDeviceCurDateTime, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    //tm.tm_year -= 1900; // 由于tm_year是从1900年开始计数的
    //tm.tm_mon -= 1;     // tm_mon是从0开始的，所以需要减1
    //tm.tm_isdst = -1;
    //now = mktime(&tm) - 8*60*60; 

    sprintf((char *)ucTempArr, "{\"clientId\":\"%s\",\"pver\":%d,\"msgId\":%s,\"functionId\":\"setConfig\",\"data\":{\"res\":%d}}", pst_MQTTSystemPara->DevicePara.cDeviceID,
                                                                                        24,
                                                                                        cmsgIdArr,
                                                                                        ucResult);

    usDataLen = strlen((char *)ucTempArr);

    memcpy(ucDataArr, ucTempArr, usDataLen);
    return usDataLen;      
}


void MQTT_Send_SetConfig_Result(unsigned char ucRes, char* cmsgIdArr)
{
	uint16_t usSendDataLen = 0;
	uint8_t ucSendBuf[1500] = {0};
	usSendDataLen = func_Get_SetConfigResult_CMD(ucSendBuf,ucRes,cmsgIdArr);
	memset(ucSendBuf, 0, 1500);
	sprintf((char *)ucSendBuf, "AT+QMTPUBEX=0,0,0,0,\"data/up/0100/0004/setConfig/%s\",%d\r\n",pst_MQTTSystemPara->DevicePara.cDeviceID, usSendDataLen);
	//sprintf((char *)ucSendBuf, "AT+QMTPUBEX=0,0,0,0,\"$sys/iVOw212I78/ZCJ2025042801/thing/property/post\",%d\r\n",usSendDataLen);
	usSendDataLen = strlen((char *)ucSendBuf);
	drv_mcu_USART_SendData(MODULE_4G_NB, (uint8_t*)ucSendBuf, usSendDataLen);
	Ddl_Delay1ms(400);
	pst_MQTTSystemPara->UsartData.usUsartxRecvDataLen[0] = 0;
	memset(pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[0], 0, USART_DATA_LEN_MAX);
	memset(ucSendBuf, 0, 1500);
	usSendDataLen = func_Get_SetConfigResult_CMD(ucSendBuf,ucRes,cmsgIdArr);
	memcpy(ucSendBuf+usSendDataLen, "\r\n", 2);
	usSendDataLen += 2;
	//sprintf((char *)ucRecvCheckData, "\"res\":");
	drv_mcu_USART_SendData(MODULE_4G_NB, (uint8_t*)ucSendBuf, usSendDataLen);
}

uint8_t MQTT_SK_Set_Config(unsigned short usBasePosi)
{
	char cDataBuf[1500] = {0};
	char cBuf[50] = {0};
	uint16_t usDataLen = 0;
	uint16_t usDataLen1 = 0;
	char *cp_Data;
	char *cp_Data1;
	char *cp_Data2;
	//uint8_t i = 0;
	uint8_t cSetResult = 0;

	uint16_t usStartPosi = 0;
	uint16_t usRecvDataLen = 0;
	char *pStr;
	char *pItem;
	char czItem[40][50] = {0};	//最多30个参数，每个参数最大长度50
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t ucRes = 1;
	char cmsgIdArr[20] = {0};
	//char cValueBuf[300] = {0};

	//查找数据起始位置
	if(func_Array_Find_Str(&pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB][usBasePosi],pst_MQTTSystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB],"data\":{",7, &usStartPosi) == 0)
	{
		//找到数据起始位置，开始解析具体参数
		usStartPosi = usStartPosi - 1;
		usRecvDataLen = pst_MQTTSystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB] - usStartPosi - usBasePosi;
		memcpy(cDataBuf, &pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB][usStartPosi+usBasePosi], usRecvDataLen);
		//复制msgId号
		if(func_Array_Find_Str(&pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB][usBasePosi],pst_MQTTSystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB],"msgId\":",7, &usStartPosi) == 0)
		{
			memcpy(cmsgIdArr, &pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB][usStartPosi+usBasePosi], 19);
		}
		pStr = strtok(cDataBuf, ",");
		while (pStr != NULL)
		{
			memcpy(czItem[i], pStr, strlen(pStr));
			i++;
			pStr = strtok(NULL, ",");
		}
		for (j = 0; j < i && j < 40; j++)
		{
			pItem = strtok(czItem[j], ":");
			if (pItem != NULL)
			{
				if(strcmp(pItem,"\"DevID\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					memcpy(pst_MQTTSystemPara->DevicePara.cDeviceID, pItem, 16);
					//#pragma diag_suppress=Pa039
					ucRes = func_Save_Device_Parameter(DEV_ID, (unsigned char*)&pst_MQTTSystemPara->DevicePara.cDeviceID);
					//g_rRun_info.ucDownFlg = atoi(pItem);
				}
				else if (strcmp(pItem, "\"DevSampGap\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.nDeviceSampleGapCnt = atoi(pItem);
					#pragma diag_suppress=Pa039
					ucRes = func_Save_Device_Parameter(DEV_SAMPLE_GAP, (unsigned char*)&pst_MQTTSystemPara->DevicePara.nDeviceSampleGapCnt);
					#pragma diag_warning=Pa039
					//memcpy(g_rRun_info.uczName, pItem, strlen(pItem) > 21?21:strlen(pItem));
				}
				else if (strcmp(pItem, "\"DevSaveGap\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.nDeviceSaveRecordCnt = atoi(pItem);
					#pragma diag_suppress=Pa039
					ucRes = func_Save_Device_Parameter(DEV_RECORD_GAP, (unsigned char*)&pst_MQTTSystemPara->DevicePara.nDeviceSaveRecordCnt);
					#pragma diag_warning=Pa039
					//memcpy(g_rRun_info.uczWifiName, pItem, 10);
				}
				else if (strcmp(pItem, "\"DevUploadGap\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.nDeviceUploadCnt = atoi(pItem);
					#pragma diag_suppress=Pa039
					ucRes = func_Save_Device_Parameter(DEV_UPLOAD_GAP, (unsigned char*)&pst_MQTTSystemPara->DevicePara.nDeviceUploadCnt);
					#pragma diag_warning=Pa039
					//memcpy(g_rRun_info.uczWifiPwd, pItem, 10);
				}
				else if (strcmp(pItem, "\"Dev485Enable1\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.cMeasSensorEnableFlag[0] = atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_SENSOR_ENABLE_1, (unsigned char*)&pst_MQTTSystemPara->DevicePara.cMeasSensorEnableFlag[0]);
				}
				else if (strcmp(pItem, "\"Dev485Enable2\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.cMeasSensorEnableFlag[1] = atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_SENSOR_ENABLE_2, (unsigned char*)&pst_MQTTSystemPara->DevicePara.cMeasSensorEnableFlag[1]);
				}
				else if (strcmp(pItem, "\"DevSensorCnt1\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.cMeasSensorCount[0] = atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_SENSOR_CNT_1, (unsigned char*)&pst_MQTTSystemPara->DevicePara.cMeasSensorCount[0]);
				}
				else if (strcmp(pItem, "\"DevSensorCnt2\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.cMeasSensorCount[1] = atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_SENSOR_CNT_2, (unsigned char*)&pst_MQTTSystemPara->DevicePara.cMeasSensorCount[1]);
				}
				else if (strcmp(pItem, "\"DevSensorType1[0]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[0][0] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN1_SENSOR1_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[0][0]);
				}
				else if (strcmp(pItem, "\"DevSensorType1[1]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[0][1] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN1_SENSOR2_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[0][1]);
				}
				else if (strcmp(pItem, "\"DevSensorType1[2]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[0][2] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN1_SENSOR3_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[0][2]);
				}
				else if (strcmp(pItem, "\"DevSensorType1[3]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[0][3] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN1_SENSOR4_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[0][3]);
				}
				else if (strcmp(pItem, "\"DevSensorType1[4]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[0][4] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN1_SENSOR5_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[0][4]);
				}
				else if (strcmp(pItem, "\"DevSensorType1[5]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[0][5] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN1_SENSOR6_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[0][5]);
				}
				else if (strcmp(pItem, "\"DevSensorType1[6]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[0][6] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN1_SENSOR7_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[0][6]);
				}
				else if (strcmp(pItem, "\"DevSensorType1[7]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[0][7] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN1_SENSOR8_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[0][7]);
				}
				else if (strcmp(pItem, "\"DevSensorType1[8]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[0][8] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN1_SENSOR9_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[0][8]);
				}
				else if (strcmp(pItem, "\"DevSensorType1[9]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[0][9] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN1_SENSOR10_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[0][9]);
				}
				else if (strcmp(pItem, "\"DevSensorType2[0]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[1][0] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN2_SENSOR1_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[1][0]);
				}
				else if (strcmp(pItem, "\"DevSensorType2[1]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[1][1] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN2_SENSOR2_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[1][1]);
				}
				else if (strcmp(pItem, "\"DevSensorType2[2]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[1][2] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN2_SENSOR3_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[1][2]);
				}
				else if (strcmp(pItem, "\"DevSensorType2[3]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[1][3] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN2_SENSOR4_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[1][3]);
				}
				else if (strcmp(pItem, "\"DevSensorType2[4]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[1][4] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN2_SENSOR5_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[1][4]);
				}
				else if (strcmp(pItem, "\"DevSensorType2[5]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[1][5] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN2_SENSOR6_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[1][5]);
				}
				else if (strcmp(pItem, "\"DevSensorType2[6]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[1][6] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN2_SENSOR7_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[1][6]);
				}
				else if (strcmp(pItem, "\"DevSensorType2[7]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[1][7] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN2_SENSOR8_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[1][7]);
				}
				else if (strcmp(pItem, "\"DevSensorType2[8]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[1][8] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN2_SENSOR9_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[1][8]);
				}
				else if (strcmp(pItem, "\"DevSensorType2[9]\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.eMeasSensor[1][9] = (EMeasSensorType)atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_CHN2_SENSOR10_TYPE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.eMeasSensor[1][9]);
				}
				else if (strcmp(pItem, "\"DevTotalFlow\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.fTotal_Volume = atof(pItem);
					#pragma diag_suppress=Pa039
					ucRes = func_Save_Device_Parameter(DEV_TOTAL_VOLUME, (unsigned char*)&pst_MQTTSystemPara->DevicePara.fTotal_Volume);
					#pragma diag_warning=Pa039
				}
				else if (strcmp(pItem, "\"DevReset\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					//pst_MQTTSystemPara->DevicePara.fTotal_Volume = atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_SOFTRESET, (unsigned char*)&pst_MQTTSystemPara->DevicePara.ucUploadStatusGap);
				}
				else if (strcmp(pItem, "\"DevUploadStatusGap\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.ucUploadStatusGap = atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_STATUS_UPLOAD_GAP, (unsigned char*)&pst_MQTTSystemPara->DevicePara.ucUploadStatusGap);
				}
				else if (strcmp(pItem, "\"DevServerIP1\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					memcpy(pst_MQTTSystemPara->DevicePara.cServerIP[0], pItem, strlen(pItem) > 16?16:strlen(pItem));
					ucRes = func_Save_Device_Parameter(DEV_IP_ADDRESS1, (unsigned char*)&pst_MQTTSystemPara->DevicePara.cServerIP[0]);
				}
				else if (strcmp(pItem, "\"DevServerIP2\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					memcpy(pst_MQTTSystemPara->DevicePara.cServerIP[1], pItem, strlen(pItem) > 16?16:strlen(pItem));
					ucRes = func_Save_Device_Parameter(DEV_IP_ADDRESS2, (unsigned char*)&pst_MQTTSystemPara->DevicePara.cServerIP[1]);
				}
				else if (strcmp(pItem, "\"DevServerPort1\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.usServerPort[0] = atoi(pItem);
					#pragma diag_suppress=Pa039
					ucRes = func_Save_Device_Parameter(DEV_IP_PORT1, (unsigned char*)&pst_MQTTSystemPara->DevicePara.usServerPort[0]);
					#pragma diag_warning=Pa039
				}
				else if (strcmp(pItem, "\"DevServerPort2\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.usServerPort[1] = atoi(pItem);
					#pragma diag_suppress=Pa039
					ucRes = func_Save_Device_Parameter(DEV_IP_PORT2, (unsigned char*)&pst_MQTTSystemPara->DevicePara.usServerPort[1]);
					#pragma diag_warning=Pa039
				}
				else if (strcmp(pItem, "\"DevDebugEnable\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DeviceRunPara.cDebugModel = atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_DEBUG_MODEL, (unsigned char*)&pst_MQTTSystemPara->DeviceRunPara.cDebugModel);
				}
				else if (strcmp(pItem, "\"DevInstallHeight\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.fInit_Height = atof(pItem);
					#pragma diag_suppress=Pa039
					ucRes = func_Save_Device_Parameter(DEV_INSTALL_HEIGHT, (unsigned char*)&pst_MQTTSystemPara->DevicePara.fInit_Height);
					#pragma diag_warning=Pa039
				}
				else if (strcmp(pItem, "\"DevSensorBaud\"") == 0)
				{
					pItem = strtok(NULL, ":");
					if(pItem[strlen(pItem)-1] == '}') //处理字符串中的引号
					{
						pItem[strlen(pItem)-1] = '\0';
					}
					pst_MQTTSystemPara->DevicePara.cSensorBaudRate = atoi(pItem);
					ucRes = func_Save_Device_Parameter(DEV_SENSOR_BAUDRATE, (unsigned char*)&pst_MQTTSystemPara->DevicePara.cSensorBaudRate);
				}
			}
		}
	}
	else	//数据帧错误
	{
		MQTT_Send_SetConfig_Result(1,cmsgIdArr);
	}

	if(ucRes == 1)
	{
		MQTT_Send_SetConfig_Result(0,cmsgIdArr);
	}
	else
	{
		MQTT_Send_SetConfig_Result(1,cmsgIdArr);
	}

	return ucRes;
}


//拼接返回设置参数命令
uint16_t func_Get_GetConfigResult_CMD(uint8_t *ucDataArr, char* cmsgIdArr)
{
    uint8_t ucTempArr[1300] = {0};
    uint16_t usDataLen = 0;
    //char c_Result[10] = {0};
    //char c_Result1[20] = {0};
    //time_t now;
    //struct tm tm;
	//drv_mcu_Get_RTC_Time(pst_MQTTSystemPara->DeviceRunPara.cDeviceCurDateTime);
    //sscanf(pst_MQTTSystemPara->DeviceRunPara.cDeviceCurDateTime, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    //tm.tm_year -= 1900; // 由于tm_year是从1900年开始计数的
    //tm.tm_mon -= 1;     // tm_mon是从0开始的，所以需要减1
    //tm.tm_isdst = -1;
    //now = mktime(&tm) - 8*60*60; 

    sprintf((char *)ucTempArr, "{\"clientId\":\"%s\",\"pver\":%d,\"msgId\":%s,\"functionId\":\"getConfig\",\"data\":{", pst_MQTTSystemPara->DevicePara.cDeviceID,
                                                                                        24,
                                                                                        cmsgIdArr
                                                                                        );

    usDataLen = strlen((char *)ucTempArr);
	//拼接设备ID参数
	sprintf((char *)ucTempArr+usDataLen, "\"DevID\":\"%s\",", pst_MQTTSystemPara->DevicePara.cDeviceID);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备IMSI号
	sprintf((char *)ucTempArr+usDataLen, "\"DevIMSI\":\"%s\",", pst_MQTTSystemPara->DevicePara.cDeviceIMSI);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备IMEI号
	sprintf((char *)ucTempArr+usDataLen, "\"DevIMEI\":\"%s\",", pst_MQTTSystemPara->DevicePara.cDeviceIMEI);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备硬件版本号
	sprintf((char *)ucTempArr+usDataLen, "\"DevHWVer\":%d,", (pst_MQTTSystemPara->DevicePara.cDeviceHWVersion[0]-0x30)*100+
													(pst_MQTTSystemPara->DevicePara.cDeviceHWVersion[2]-0x30)*10+
													(pst_MQTTSystemPara->DevicePara.cDeviceHWVersion[4]-0x30));
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备软件版本号
	sprintf((char *)ucTempArr+usDataLen, "\"DevSWVer\":%d,", (pst_MQTTSystemPara->DevicePara.cDeviceSWVersion[0]-0x30)*100+
													(pst_MQTTSystemPara->DevicePara.cDeviceSWVersion[2]-0x30)*10+
													(pst_MQTTSystemPara->DevicePara.cDeviceSWVersion[4]-0x30));
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备生产日期
	sprintf((char *)ucTempArr+usDataLen, "\"DevPD\":\"%s\",", pst_MQTTSystemPara->DevicePara.cDevicePDDate);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备采样间隔
	sprintf((char *)ucTempArr+usDataLen, "\"DevSampGap\":%d,", pst_MQTTSystemPara->DevicePara.nDeviceSampleGapCnt*60);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备记录间隔
	sprintf((char *)ucTempArr+usDataLen, "\"DevSaveGap\":%d,", pst_MQTTSystemPara->DevicePara.nDeviceSaveRecordCnt*60);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备上传间隔
	sprintf((char *)ucTempArr+usDataLen, "\"DevUploadGap\":%d,", pst_MQTTSystemPara->DevicePara.nDeviceUploadCnt*60);
	usDataLen = strlen((char *)ucTempArr);
	//拼接485通道1使能标志
	sprintf((char *)ucTempArr+usDataLen, "\"Dev485Enable1\":%d,", pst_MQTTSystemPara->DevicePara.cMeasSensorEnableFlag[0]);
	usDataLen = strlen((char *)ucTempArr);
	//拼接485通道2使能标志
	sprintf((char *)ucTempArr+usDataLen, "\"Dev485Enable2\":%d,", pst_MQTTSystemPara->DevicePara.cMeasSensorEnableFlag[1]);
	usDataLen = strlen((char *)ucTempArr);
	//拼接485通道1传感器数量
	sprintf((char *)ucTempArr+usDataLen, "\"DevSensorCnt1\":%d,", pst_MQTTSystemPara->DevicePara.cMeasSensorCount[0]);
	usDataLen = strlen((char *)ucTempArr);
	//拼接485通道2传感器数量
	sprintf((char *)ucTempArr+usDataLen, "\"DevSensorCnt2\":%d,", pst_MQTTSystemPara->DevicePara.cMeasSensorCount[1]);
	usDataLen = strlen((char *)ucTempArr);
	//拼接485通道1传感器类型
	sprintf((char *)ucTempArr+usDataLen, "\"DevSensorType1\":[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d],",
		pst_MQTTSystemPara->DevicePara.eMeasSensor[0][0],
		pst_MQTTSystemPara->DevicePara.eMeasSensor[0][1],
		pst_MQTTSystemPara->DevicePara.eMeasSensor[0][2],
		pst_MQTTSystemPara->DevicePara.eMeasSensor[0][3],
		pst_MQTTSystemPara->DevicePara.eMeasSensor[0][4],
		pst_MQTTSystemPara->DevicePara.eMeasSensor[0][5],
		pst_MQTTSystemPara->DevicePara.eMeasSensor[0][6],
		pst_MQTTSystemPara->DevicePara.eMeasSensor[0][7],
		pst_MQTTSystemPara->DevicePara.eMeasSensor[0][8],
		pst_MQTTSystemPara->DevicePara.eMeasSensor[0][9]);
	usDataLen = strlen((char *)ucTempArr);
	//拼接485通道2传感器类型	
	sprintf((char *)ucTempArr+usDataLen, "\"DevSensorType2\":[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d],",
		pst_MQTTSystemPara->DevicePara.eMeasSensor[1][0],
		pst_MQTTSystemPara->DevicePara.eMeasSensor[1][1],
		pst_MQTTSystemPara->DevicePara.eMeasSensor[1][2],
		pst_MQTTSystemPara->DevicePara.eMeasSensor[1][3],
		pst_MQTTSystemPara->DevicePara.eMeasSensor[1][4],
		pst_MQTTSystemPara->DevicePara.eMeasSensor[1][5],
		pst_MQTTSystemPara->DevicePara.eMeasSensor[1][6],
		pst_MQTTSystemPara->DevicePara.eMeasSensor[1][7],
		pst_MQTTSystemPara->DevicePara.eMeasSensor[1][8],
		pst_MQTTSystemPara->DevicePara.eMeasSensor[1][9]);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备总流量
	sprintf((char *)ucTempArr+usDataLen, "\"DevTotalFlow\":%.3f,", pst_MQTTSystemPara->DevicePara.fTotal_Volume);
	usDataLen = strlen((char *)ucTempArr);
	//拼接状态信息：纬度
	sprintf((char *)ucTempArr+usDataLen, "\"DevLat\":%.6f,", pst_MQTTSystemPara->DeviceRunPara.esDeviceRunState.fDevLoca_lat);
	usDataLen = strlen((char *)ucTempArr);																					
	//拼接状态信息：经度
	sprintf((char *)ucTempArr+usDataLen, "\"DevLng\":%.6f,", pst_MQTTSystemPara->DeviceRunPara.esDeviceRunState.fDevLoca_lng);
	usDataLen = strlen((char *)ucTempArr);
	//拼接状态信息：电量百分比
	sprintf((char *)ucTempArr+usDataLen, "\"DevBatteryPer\":%f,", pst_MQTTSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent * 100);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备状态信息：水浸标志
	sprintf((char *)ucTempArr+usDataLen, "\"DevWaterImmi\":%d,", pst_MQTTSystemPara->DeviceRunPara.esDeviceSensorsData.cWater_Immersion_Status);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备状态信息：姿态数据
	sprintf((char *)ucTempArr+usDataLen, "\"DevAttitude\":%d,", pst_MQTTSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备状态信息：光照标志
	sprintf((char *)ucTempArr+usDataLen, "\"DevLight\":%d,", pst_MQTTSystemPara->DeviceRunPara.esDeviceSensorsData.cPhotosensitive_XYC_ALS_Status);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备状态信息：报警状态
	sprintf((char *)ucTempArr+usDataLen, "\"DevAlarmStatus\":%d,", pst_MQTTSystemPara->DeviceRunPara.usDevStatus);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备测量信息
	sprintf((char *)ucTempArr+usDataLen, "\"DevSensorValue\":[%.3f,%.3f,%.3f,%.3f],",gSt_DevMeasRecordData.fWaterLevel_Radar,gSt_DevMeasRecordData.fWaterLevel_Pres,gSt_DevMeasRecordData.fWaterQuality_COND,gSt_DevMeasRecordData.fWaterVolume_s);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备状态报文上报间隔
	sprintf((char *)ucTempArr+usDataLen, "\"DevUploadStatusGap\":%d,", pst_MQTTSystemPara->DevicePara.ucUploadStatusGap);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备服务器IP1
	sprintf((char *)ucTempArr+usDataLen, "\"DevServerIP1\":\"%s\",", pst_MQTTSystemPara->DevicePara.cServerIP[0]);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备服务器IP2
	sprintf((char *)ucTempArr+usDataLen, "\"DevServerIP2\":\"%s\",", pst_MQTTSystemPara->DevicePara.cServerIP[1]);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备服务器端口1
	sprintf((char *)ucTempArr+usDataLen, "\"DevServerPort1\":%d,", pst_MQTTSystemPara->DevicePara.usServerPort[0]);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备服务器端口2
	sprintf((char *)ucTempArr+usDataLen, "\"DevServerPort2\":%d,", pst_MQTTSystemPara->DevicePara.usServerPort[1]);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备调试模式
	sprintf((char *)ucTempArr+usDataLen, "\"DevDebugEnable\":%d,", pst_MQTTSystemPara->DeviceRunPara.cDebugModel);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备安装高度
	sprintf((char *)ucTempArr+usDataLen, "\"DevInstallHeight\":%.3f,", pst_MQTTSystemPara->DevicePara.fInit_Height);
	usDataLen = strlen((char *)ucTempArr);
	//拼接设备传感器波特率
	sprintf((char *)ucTempArr+usDataLen, "\"DevSensorBaud\":%d", pst_MQTTSystemPara->DevicePara.cSensorBaudRate);
	usDataLen = strlen((char *)ucTempArr);
	//拼接结束符
	sprintf((char *)ucTempArr+usDataLen, "}}");
	usDataLen = strlen((char *)ucTempArr);

    memcpy(ucDataArr, ucTempArr, usDataLen);
    return usDataLen;      
}

/**
 * @brief  MQTT_SK_Set_Config
 * @param  None
 * @retval 0: get success 1: get failed 
 */
uint8_t MQTT_SK_Get_Config(unsigned short usBasePosi)
{
	uint16_t usSendDataLen = 0;
	uint8_t ucSendBuf[1500] = {0};
	char cmsgIdArr[20] = {0};
	unsigned short usStartPosi = 0;
	//复制msgId号
	if(func_Array_Find_Str(&pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB][usBasePosi],pst_MQTTSystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB],"msgId\":",7, &usStartPosi) == 0)
	{
		memcpy(cmsgIdArr, &pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB][usStartPosi+usBasePosi], 19);
	}
	usSendDataLen = func_Get_GetConfigResult_CMD(ucSendBuf,cmsgIdArr);
	memset(ucSendBuf, 0, 1500);
	sprintf((char *)ucSendBuf, "AT+QMTPUBEX=0,0,0,0,\"data/up/0100/0004/getConfig/%s\",%d\r\n",pst_MQTTSystemPara->DevicePara.cDeviceID, usSendDataLen);
	//sprintf((char *)ucSendBuf, "AT+QMTPUBEX=0,0,0,0,\"$sys/iVOw212I78/ZCJ2025042801/thing/property/post\",%d\r\n",usSendDataLen);
	usSendDataLen = strlen((char *)ucSendBuf);
	drv_mcu_USART_SendData(MODULE_4G_NB, (uint8_t*)ucSendBuf, usSendDataLen);
	Ddl_Delay1ms(400);
	pst_MQTTSystemPara->UsartData.usUsartxRecvDataLen[0] = 0;
	memset(pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[0], 0, USART_DATA_LEN_MAX);
	memset(ucSendBuf, 0, 1500);
	usSendDataLen = func_Get_GetConfigResult_CMD(ucSendBuf,cmsgIdArr);
	memcpy(ucSendBuf+usSendDataLen, "\r\n", 2);
	usSendDataLen += 2;
	//sprintf((char *)ucRecvCheckData, "\"res\":");
	drv_mcu_USART_SendData(MODULE_4G_NB, (uint8_t*)ucSendBuf, usSendDataLen);
}

/**
 * @brief  MQTT_SK_Get_Data
 * @param  None
 * @retval 0: get success 1: get failed 
 */
uint8_t MQTT_SK_Get_Data(void)
{
	uint8_t ucRes = 0;
	unsigned short usPosi = 0;
	struct tm tmTime_Begin;
	struct tm tmTime_End;
	time_t tTime_Begin;
	time_t tTime_End;
	unsigned short usDataLen;
	unsigned long ulStartTimeDif = 0;
	time_t now;
	char cSendBuf[100] = {0};
	char cTimeArr[20];

    //time(&now);
	struct tm tm;
    //time(&now);
    drv_mcu_Get_RTC_Time(pst_MQTTSystemPara->DeviceRunPara.cDeviceCurDateTime);
    sscanf(pst_MQTTSystemPara->DeviceRunPara.cDeviceCurDateTime, "%d-%d-%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    tm.tm_year -= 1900; // 由于tm_year是从1900年开始计数的
    tm.tm_mon -= 1;     // tm_mon是从0开始的，所以需要减1
	tm.tm_isdst = -1;
    now = mktime(&tm) - 8*60*60; 

	if((func_Array_Find_Str(pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],USART_DATA_LEN_MAX/2,"timeBegin",9, &usPosi)  == 0)
		&& func_Array_Find_Str(pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],USART_DATA_LEN_MAX/2,"timeEnd",7, &usPosi) == 0)
	{
		func_Array_Find_Str(pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],USART_DATA_LEN_MAX, "\"timeBegin\":", 12, &usPosi);
		if(usPosi != 0xFFFF)
		{
			// 使用strptime解析时间字符串
			//strftime(&pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB][usPosi], USART_DATA_LEN_MAX,"%Y-%m-%d %H:%M:%S",  &tmTime_Begin);
			sscanf((pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB]+usPosi),"%d-%d-%d %d:%d:%d", 
				&tmTime_Begin.tm_year, &tmTime_Begin.tm_mon, &tmTime_Begin.tm_mday, 
				&tmTime_Begin.tm_hour, &tmTime_Begin.tm_min, &tmTime_Begin.tm_sec);
			tmTime_Begin.tm_year -= 1900; // 年份从1900开始
			tmTime_Begin.tm_mon -= 1; // 月份从0开始
			tm.tm_isdst = -1;
			// 将struct tm转换为time_t
			tTime_Begin = mktime(&tmTime_Begin) - 8*60*60;
		}
		else
		{
			ucRes = 1; // 如果没有找到时间范围，返回错误
		}

		func_Array_Find_Str(pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],USART_DATA_LEN_MAX, "\"timeEnd\":", 10, &usPosi);
		if(usPosi != 0xFFFF)
		{ 
			// 使用strptime解析时间字符串
			//strftime(&pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB][usPosi],USART_DATA_LEN_MAX,"%Y-%m-%d %H:%M:%S",  &tmTime_End);
			sscanf((pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB]+usPosi),"%d-%d-%d %d:%d:%d", 
				&tmTime_End.tm_year, &tmTime_End.tm_mon, &tmTime_End.tm_mday, 
				&tmTime_End.tm_hour, &tmTime_End.tm_min, &tmTime_End.tm_sec);
			tmTime_End.tm_year -= 1900; // 年份从1900开始
			tmTime_End.tm_mon -= 1; // 月份从0开始
			tm.tm_isdst = -1;
			// 将struct tm转换为time_t
			tTime_End = mktime(&tmTime_End) - 8*60*60;
		}
		else
		{
			ucRes = 1; // 如果没有找到时间范围，返回错误
		}
		if((long)now <= (long)tTime_Begin)
		{
			ucRes = 2; // 如果当前时间小于等于开始时间，返回错误
		}
		else
		{
			if(tTime_Begin > tTime_End)
			{
				ucRes = 1; // 如果开始时间大于结束时间，返回错误
			}
			else
			{
				ulStartTimeDif = (unsigned long)(now - tTime_Begin);
				ulStartTimeDif = ulStartTimeDif / 60; // 转换为分钟
				if(ulStartTimeDif > pst_MQTTSystemPara->DevicePara.nDeviceSaveRecordCnt * pst_MQTTSystemPara->DevicePara.nDeviceRecordCnt) // 如果时间范围超过设备总记录时间
				{
					ucRes = 2;
				}
				else
				{
					pst_MQTTSystemPara->DeviceRunPara.ulDeviceGetDataStartCnt = pst_MQTTSystemPara->DevicePara.nDeviceRecordCnt - (ulStartTimeDif / pst_MQTTSystemPara->DevicePara.nDeviceSaveRecordCnt); // 计算开始记录位置
				
					if(now < tTime_End) // 如果当前时间小于结束时间
					{
						//ulStartTimeDif = (unsigned long)(now - tTime_Begin);
						pst_MQTTSystemPara->DeviceRunPara.ulDeviceGetDataEndCnt = pst_MQTTSystemPara->DevicePara.nDeviceRecordCnt; // 计算结束记录位置
					}
					else
					{
						ulStartTimeDif = (unsigned long)(now - tTime_End);
						ulStartTimeDif = ulStartTimeDif / 60; // 转换为分钟
						pst_MQTTSystemPara->DeviceRunPara.ulDeviceGetDataEndCnt = pst_MQTTSystemPara->DevicePara.nDeviceRecordCnt - (ulStartTimeDif / pst_MQTTSystemPara->DevicePara.nDeviceSaveRecordCnt); // 计算结束记录位置
					}
					
					pst_MQTTSystemPara->DeviceRunPara.cDeviceGetDataFlag = 1; // 设置获取数据标志位
					ucRes = 0; // 如果时间范围有效，返回成功
				}
			}
		}
	}
	else
	{
		ucRes = 1; // 如果没有找到时间范围，返回错误
	}

	//拼接返回指令
	func_Array_Find_Str(pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB],USART_DATA_LEN_MAX, "\"pver\":24,", 10, &usPosi);
	if(usPosi != 0xFFFF)
	{
		memcpy(cTimeArr,pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB]+usPosi-1,18);
	}
	sprintf(cSendBuf,"{\"clientId\":\"%s\",\"pver\":24,%s,\"data\":{\"res\":%d}}", 
					pst_MQTTSystemPara->DevicePara.cDeviceID, 
					cTimeArr, 
					ucRes);

	usDataLen = strlen(cSendBuf);
	//发送AT指令
	//drv_mcu_USART_SendData(MODULE_4G_NB, (uint8_t *)cSendBuf, usDataLen);
	//usSendDataLen = func_Get_DevStatusCMD_Data(ucSendBuf);
	sprintf((char *)cSendBuf, "AT+QMTPUBEX=0,0,0,0,\"data/up/0100/0004/getData/%s\",%d\r\n",pst_MQTTSystemPara->DevicePara.cDeviceID,usDataLen);
	usDataLen = strlen((char *)cSendBuf);
	//pst_EC200USystemPara->DeviceRunPara.enUploadStatus = Status_Upload;
	drv_mcu_USART_SendData(MODULE_4G_NB, (uint8_t*)cSendBuf, usDataLen);
	Ddl_Delay1ms(400);
	pst_MQTTSystemPara->UsartData.usUsartxRecvDataLen[MODULE_4G_NB] = 0;
	memset(pst_MQTTSystemPara->UsartData.ucUsartxRecvDataArr[MODULE_4G_NB], 0, USART_DATA_LEN_MAX);
	memset(cSendBuf, 0, 100);
	//usDataLen = func_Get_DevStatusCMD_Data(cSendBuf);
	sprintf(cSendBuf,"{\"clientId\":\"%s\",\"pver\":24,%s,\"data\":{\"res\":%d}}", 
		pst_MQTTSystemPara->DevicePara.cDeviceID, 
		cTimeArr, 
		ucRes);

	usDataLen = strlen(cSendBuf);
	memcpy(cSendBuf+usDataLen, "\r\n", 2);
	usDataLen += 2;
	//发送AT指令
	drv_mcu_USART_SendData(MODULE_4G_NB, (uint8_t*)cSendBuf, usDataLen);
	return ucRes;
}

/**
 * @brief  MQTT_SK_Data_Pass_Through
 * @param  None
 * @retval 0: get success 1: get failed 
 */
uint8_t MQTT_SK_Data_Pass_Through(void)
{
	return 0;
}


void func_MQTT_SK_Init(void)
{
	pst_MQTTSystemPara = GetSystemPara();
}
/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
