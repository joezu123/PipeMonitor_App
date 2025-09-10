/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\ModbusRTU\ModbusRTU.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-05-16       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "ModbusRTU.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define MB_FUNC_HANDLERS_MAX                    ( 16 )
#define REG_INPUT_START 0x1000
#define REG_INPUT_NREGS 0xD000
#define MD_MAX_REG_UNIT 60
/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
extern eMBException    eMBFuncReadHoldingRegister( unsigned char * pucFrame, unsigned short * usLen );
extern eMBException    eMBFuncWriteHoldingRegister( unsigned char * pucFrame, unsigned short * usLen );
extern eMBException    eMBFuncWriteMultipleHoldingRegister( unsigned char * pucFrame, unsigned short * usLen );
/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static xMBFunctionHandler xFuncHandlers[MB_FUNC_HANDLERS_MAX] =
{
	//{MB_FUNC_OTHER_REPORT_SLAVEID, eMBFuncReportSlaveID},
	//{MB_FUNC_READ_INPUT_REGISTER, eMBFuncReadInputRegister},
	{MB_FUNC_READ_HOLDING_REGISTER, eMBFuncReadHoldingRegister},
	{MB_FUNC_WRITE_REGISTER, eMBFuncWriteHoldingRegister},
	{MB_FUNC_WRITE_MULTIPLE_REGISTERS, eMBFuncWriteMultipleHoldingRegister},
	//{MB_FUNC_READWRITE_MULTIPLE_REGISTERS, eMBFuncReadWriteMultipleHoldingRegister},
	//{MB_FUNC_READ_COILS, eMBFuncReadCoils},
	//{MB_FUNC_WRITE_SINGLE_COIL, eMBFuncWriteCoil},
	//{MB_FUNC_WRITE_MULTIPLE_COILS, eMBFuncWriteMultipleCoils},
	//{MB_FUNC_READ_DISCRETE_INPUTS, eMBFuncReadDiscreteInputs},
};

typedef struct Mb_Get_RegUnit_Info_S
{
  short StartUnit;//起始单元
  short UnitNum;//单元个数
  short StartUnitReg;//起始单元寄存器地址偏移
  short EndUnitReg;//结束单元寄存器数 
  short retStat;//查询结果
}Mb_Get_RegUnit_Info_S;

typedef eMBErrorCode( *RWRegFunction) ( unsigned char * pucRegBuffer, unsigned short usAddress, unsigned short usNRegs, eMBRegisterMode eMode);    

typedef struct MB_Data_Reg_Unit_S
{
  unsigned short RegAddr;//寄存器地址
  unsigned short ArgRegNum;//参数占寄存器大小
  RWRegFunction   RWRegFunc;//读写函数
}MB_Data_Reg_Unit_S;

static volatile unsigned short usRcvBufferPos;
static unsigned char *pucSndBufferCur;

eMBException prveMBError2Exception( eMBErrorCode eErrorCode )
{
    eMBException    eStatus;

    switch ( eErrorCode )
    {
        case MB_ENOERR:
            eStatus = MB_EX_NONE;
            break;

        case MB_ENOREG:
            eStatus = MB_EX_ILLEGAL_DATA_ADDRESS;
            break;

        case MB_ETIMEDOUT:
            eStatus = MB_EX_SLAVE_BUSY;
            break;

        default:
            eStatus = MB_EX_SLAVE_DEVICE_FAILURE;
            break;
    }

    return eStatus;
}

//设备ID 读写函数
eMBErrorCode Unit_RW_DeviceID_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned short usAddrP = 2*usAddressP;
    unsigned char *cData;
    unsigned char rwData[32];
    memset(rwData,0,32);
    if(eMode == MB_REG_READ)
    {  
        memcpy(rwData,(unsigned char*)&pst_MBSystemPara->DevicePara.cDeviceID[0],16);
        cData = (unsigned char*)&rwData[0];
        RegData_HL_Swap_func(pucRegBuffer, cData + usAddrP,16, 0, 16);        
    }
    else
    {
        if(usAddrP != 0)
        {
            return MB_ENOREG;
        }
        RegData_HL_Swap_func((unsigned char *)&rwData[0], pucRegBuffer,2*usNRegs , 0, 16);   
        if(1 !=  func_Save_Device_Parameter(DEV_ID, (unsigned char *)&rwData[0]) )
        {
            return MB_EINVAL;
        }
    }
    return MB_ENOERR;
}

//设备已在服务器注册标志位
eMBErrorCode Unit_RW_DeviceRegisterFlag_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = pst_MBSystemPara->DevicePara.cDeviceIdenFlag;
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_IDEN_FLAG);
}

//设备IMSI数据
eMBErrorCode Unit_R_DeviceIMSI_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned short usAddrP = 2*usAddressP;
    unsigned char *cData;
    unsigned char rwData[32];
    memset(rwData,0,32);
    if(eMode == MB_REG_READ)
    {  
        memcpy(rwData,(unsigned char*)&pst_MBSystemPara->DevicePara.cDeviceIMSI[0],16);
        cData = (unsigned char*)&rwData[0];
        RegData_HL_Swap_func(pucRegBuffer, cData + usAddrP,16, 0, 16);        
    }
    return MB_ENOERR;
}

//设备IMEI数据
eMBErrorCode Unit_R_DeviceIMEI_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned short usAddrP = 2*usAddressP;
    unsigned char *cData;
    unsigned char rwData[32];
    memset(rwData,0,32);
    if(eMode == MB_REG_READ)
    {  
        memcpy(rwData,(unsigned char*)&pst_MBSystemPara->DevicePara.cDeviceIMEI[0],16);
        cData = (unsigned char*)&rwData[0];
        RegData_HL_Swap_func(pucRegBuffer, cData + usAddrP,16, 0, 16);        
    }
    return MB_ENOERR;
}

//设备硬件版本号数据
eMBErrorCode Unit_R_DeviceHWVersion_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned short usAddrP = 2*usAddressP;
    unsigned char *cData;
    unsigned char rwData[20];
    memset(rwData,0,20);
    if(eMode == MB_REG_READ)
    {  
        memcpy(rwData,(unsigned char*)&pst_MBSystemPara->DevicePara.cDeviceHWVersion[0],10);
        cData = (unsigned char*)&rwData[0];
        RegData_HL_Swap_func(pucRegBuffer, cData + usAddrP,10, 0, 10);        
    }
    return MB_ENOERR;
}

//设备软件版本号数据
eMBErrorCode Unit_R_DeviceSWVersion_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned short usAddrP = 2*usAddressP;
    unsigned char *cData;
    unsigned char rwData[20];
    memset(rwData,0,20);
    if(eMode == MB_REG_READ)
    {  
        memcpy(rwData,(unsigned char*)&pst_MBSystemPara->DevicePara.cDeviceSWVersion[0],10);
        cData = (unsigned char*)&rwData[0];
        RegData_HL_Swap_func(pucRegBuffer, cData + usAddrP,10, 0, 10);        
    }
    return MB_ENOERR;
}

//设备生产日期-只读
eMBErrorCode Unit_R_DevicePDDate_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned short usAddrP = 2*usAddressP;
    unsigned char *cData;
    unsigned char rwData[20];
    memset(rwData,0,20);
    if(eMode == MB_REG_READ)
    {  
        memcpy(rwData,(unsigned char*)&pst_MBSystemPara->DevicePara.cDevicePDDate[0],10);
        cData = (unsigned char*)&rwData[0];
        RegData_HL_Swap_func(pucRegBuffer, cData + usAddrP,10, 0, 10);        
    }
    return MB_ENOERR;
}

//设备采样间隔时间
eMBErrorCode Unit_RW_DeviceSampleGap_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned short usValue;
    usValue = (unsigned short)pst_MBSystemPara->DevicePara.nDeviceSampleGapCnt;
    return Short_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, usValue,DEV_SAMPLE_GAP);
}

//设备记录间隔时间
eMBErrorCode Unit_RW_DeviceRecordGap_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned short usValue;
    usValue = (unsigned short)pst_MBSystemPara->DevicePara.nDeviceSaveRecordCnt;
    return Short_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, usValue,DEV_RECORD_GAP);
}

//设备记录上传间隔时间
eMBErrorCode Unit_RW_DeviceUploadGap_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned short usValue;
    usValue = (unsigned short)pst_MBSystemPara->DevicePara.nDeviceUploadCnt;
    return Short_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, usValue,DEV_UPLOAD_GAP);
}



//设备当前纬度数据
eMBErrorCode Unit_R_DeviceLatitude_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    float fVlaue = (float)pst_MBSystemPara->DeviceRunPara.esDeviceRunState.fDevLoca_lat;
    return Float_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, fVlaue,DEV_BEGIN_PARA);
}

//设备当前经度数据
eMBErrorCode Unit_R_DeviceLngitude_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    float fVlaue = pst_MBSystemPara->DeviceRunPara.esDeviceRunState.fDevLoca_lng;
    return Float_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, fVlaue,DEV_BEGIN_PARA);
}

//设备当前电量百分比数据
eMBErrorCode Unit_R_DeviceBatteryLevel_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    float fVlaue = pst_MBSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent;
    return Float_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, fVlaue,DEV_BEGIN_PARA);
}

//设备当前状态-水浸标志
eMBErrorCode Unit_R_DeviceWaterImmersion_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DeviceRunPara.esDeviceSensorsData.cWater_Immersion_Status;
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_BEGIN_PARA);
}

//设备当前状态-姿态数据
eMBErrorCode Unit_R_DeviceAttitude_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned short sVlaue;
    sVlaue = (unsigned short)pst_MBSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A;
    return Short_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, sVlaue, DEV_BEGIN_PARA);
}

//设备当前状态-光照标志
eMBErrorCode Unit_R_DevicePhotosensitive_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DeviceRunPara.esDeviceSensorsData.cPhotosensitive_XYC_ALS_Status;
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_BEGIN_PARA);
}

//设备当前状态-报警信息
eMBErrorCode Unit_R_DeviceAlarmStatus_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
   	unsigned short sVlaue = pst_MBSystemPara->DeviceRunPara.usDevStatus;
    return Short_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, sVlaue, DEV_BEGIN_PARA);
}

//设备累计流量
eMBErrorCode Unit_RW_Device_Total_Volume_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    float fVlaue = pst_MBSystemPara->DevicePara.fTotal_Volume;
    return Float_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, fVlaue,DEV_TOTAL_VOLUME);
}

//设备485-1使能标志
eMBErrorCode Unit_RW_DeviceSensor1Enable_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = pst_MBSystemPara->DevicePara.cMeasSensorEnableFlag[0];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_SENSOR_ENABLE_1);
}

//设备485-2使能标志
eMBErrorCode Unit_RW_DeviceSensor2Enable_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = pst_MBSystemPara->DevicePara.cMeasSensorEnableFlag[1];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_SENSOR_ENABLE_2);
}

//设备485-1外接传感器数量
eMBErrorCode Unit_RW_DeviceSensor1Cnt_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = pst_MBSystemPara->DevicePara.cMeasSensorCount[0];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_SENSOR_CNT_1);
}

//设备485-2外接传感器数量
eMBErrorCode Unit_RW_DeviceSensor2Cnt_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = pst_MBSystemPara->DevicePara.cMeasSensorCount[1];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_SENSOR_CNT_2);
}

//设备485-1外接传感器类型1
eMBErrorCode Unit_RW_Device_Chn1_Sensor1Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[0][0];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN1_SENSOR1_TYPE);
}

//设备485-1外接传感器类型2
eMBErrorCode Unit_RW_Device_Chn1_Sensor2Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[0][1];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN1_SENSOR2_TYPE);
}

//设备485-1外接传感器类型3
eMBErrorCode Unit_RW_Device_Chn1_Sensor3Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[0][2];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN1_SENSOR3_TYPE);
}

//设备485-1外接传感器类型4
eMBErrorCode Unit_RW_Device_Chn1_Sensor4Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[0][3];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN1_SENSOR4_TYPE);
}

//设备485-1外接传感器类型5
eMBErrorCode Unit_RW_Device_Chn1_Sensor5Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[0][4];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN1_SENSOR5_TYPE);
}

//设备485-1外接传感器类型6
eMBErrorCode Unit_RW_Device_Chn1_Sensor6Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[0][5];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN1_SENSOR6_TYPE);
}

//设备485-1外接传感器类型7
eMBErrorCode Unit_RW_Device_Chn1_Sensor7Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[0][6];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN1_SENSOR7_TYPE);
}

//设备485-1外接传感器类型8
eMBErrorCode Unit_RW_Device_Chn1_Sensor8Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[0][7];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN1_SENSOR8_TYPE);
}

//设备485-1外接传感器类型9
eMBErrorCode Unit_RW_Device_Chn1_Sensor9Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[0][8];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN1_SENSOR9_TYPE);
}

//设备485-1外接传感器类型10
eMBErrorCode Unit_RW_Device_Chn1_Sensor10Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[0][9];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN1_SENSOR10_TYPE);
}

//设备485-2外接传感器类型1
eMBErrorCode Unit_RW_Device_Chn2_Sensor1Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[1][0];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN2_SENSOR1_TYPE);
}

//设备485-2外接传感器类型2
eMBErrorCode Unit_RW_Device_Chn2_Sensor2Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[1][1];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN2_SENSOR2_TYPE);
}

//设备485-2外接传感器类型3
eMBErrorCode Unit_RW_Device_Chn2_Sensor3Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[1][2];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN2_SENSOR3_TYPE);
}

//设备485-2外接传感器类型4
eMBErrorCode Unit_RW_Device_Chn2_Sensor4Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[1][3];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN2_SENSOR4_TYPE);
}

//设备485-2外接传感器类型5
eMBErrorCode Unit_RW_Device_Chn2_Sensor5Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[1][4];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN2_SENSOR5_TYPE);
}

//设备485-2外接传感器类型6
eMBErrorCode Unit_RW_Device_Chn2_Sensor6Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[1][5];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN2_SENSOR6_TYPE);
}

//设备485-2外接传感器类型7
eMBErrorCode Unit_RW_Device_Chn2_Sensor7Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[1][6];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN2_SENSOR7_TYPE);
}

//设备485-2外接传感器类型8
eMBErrorCode Unit_RW_Device_Chn2_Sensor8Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[1][7];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN2_SENSOR8_TYPE);
}

//设备485-2外接传感器类型9
eMBErrorCode Unit_RW_Device_Chn2_Sensor9Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[1][8];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN2_SENSOR9_TYPE);
}

//设备485-1外接传感器类型10
eMBErrorCode Unit_RW_Device_Chn2_Sensor10Type_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.eMeasSensor[1][9];
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_CHN2_SENSOR10_TYPE);
}

//设备状态报文上传间隔
eMBErrorCode Unit_RW_Device_Status_Upload_Gap_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DevicePara.ucUploadStatusGap;
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_STATUS_UPLOAD_GAP);
}

//设备上传IP地址1
eMBErrorCode Unit_RW_Device_ServerIP1_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned short usAddrP = 2*usAddressP;
    unsigned char *cData;
    unsigned char rwData[32];
    memset(rwData,0,32);
    if(eMode == MB_REG_READ)
    {  
        memcpy(rwData,(unsigned char*)&pst_MBSystemPara->DevicePara.cServerIP[0],16);
        cData = (unsigned char*)&rwData[0];
        RegData_HL_Swap_func(pucRegBuffer, cData + usAddrP,16, 0, 16);        
    }
    else
    {
        if(usAddrP != 0)
        {
            return MB_ENOREG;
        }
        RegData_HL_Swap_func((unsigned char *)&rwData[0], pucRegBuffer,2*usNRegs , 0, 16);   
        if(1 !=  func_Save_Device_Parameter(DEV_IP_ADDRESS1, (unsigned char *)&rwData[0]) )
        {
            return MB_EINVAL;
        }
    }
    return MB_ENOERR;
}

//设备上传IP地址2
eMBErrorCode Unit_RW_Device_ServerIP2_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned short usAddrP = 2*usAddressP;
    unsigned char *cData;
    unsigned char rwData[32];
    memset(rwData,0,32);
    if(eMode == MB_REG_READ)
    {  
        memcpy(rwData,(unsigned char*)&pst_MBSystemPara->DevicePara.cServerIP[1],16);
        cData = (unsigned char*)&rwData[0];
        RegData_HL_Swap_func(pucRegBuffer, cData + usAddrP,16, 0, 16);        
    }
    else
    {
        if(usAddrP != 0)
        {
            return MB_ENOREG;
        }
        RegData_HL_Swap_func((unsigned char *)&rwData[0], pucRegBuffer,2*usNRegs , 0, 16);   
        if(1 !=  func_Save_Device_Parameter(DEV_IP_ADDRESS2, (unsigned char *)&rwData[0]) )
        {
            return MB_EINVAL;
        }
    }
    return MB_ENOERR;
}

//设备上传端口号1
eMBErrorCode Unit_R_Device_ServerPort1_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
   	unsigned short sVlaue = pst_MBSystemPara->DevicePara.usServerPort[0];
    return Short_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, sVlaue, DEV_IP_PORT1);
}

//设备上传端口号2
eMBErrorCode Unit_R_Device_ServerPort2_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
   	unsigned short sVlaue = pst_MBSystemPara->DevicePara.usServerPort[1];
    return Short_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, sVlaue, DEV_IP_PORT2);
}

//设备调试模式开关
eMBErrorCode Unit_RW_Device_Debug_Model_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = (unsigned char)pst_MBSystemPara->DeviceRunPara.cDebugModel;
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_DEBUG_MODEL);
}

//设备外接水位计安装高度
eMBErrorCode Unit_RW_Device_Install_Height_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    float fVlaue = pst_MBSystemPara->DevicePara.fInit_Height;
    return Float_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, fVlaue,DEV_INSTALL_HEIGHT);
}

//设备外接传感器波特率设置
eMBErrorCode Unit_RW_Device_Sensor_Baud_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    char cVlaue = (char)pst_MBSystemPara->DevicePara.cSensorBaudRate;
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_SENSOR_BAUDRATE);
}

//设备测量信息：雷达液位计
eMBErrorCode Unit_R_DeviceRadarLevel_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    float fVlaue = gSt_DevMeasRecordData.fWaterLevel_Radar;
    return Float_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, fVlaue,DEV_BEGIN_PARA);
}

//设备测量信息：压力液位计
eMBErrorCode Unit_R_DevicePressLevel_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    float fVlaue = gSt_DevMeasRecordData.fWaterLevel_Pres;
    return Float_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, fVlaue,DEV_BEGIN_PARA);
}

//设备测量信息：流量计
eMBErrorCode Unit_R_DeviceWaterVolume_s_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    float fVlaue = gSt_DevMeasRecordData.fWaterVolume_s;
    return Float_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, fVlaue,DEV_BEGIN_PARA);
}

//设备测量信息：电导率计
eMBErrorCode Unit_R_DeviceWaterQuality_COND_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    float fVlaue = gSt_DevMeasRecordData.fWaterQuality_COND;
    return Float_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, fVlaue,DEV_BEGIN_PARA);
}

//设备生产日期-只写
eMBErrorCode Unit_W_DevicePDDate_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned short usAddrP = 2*usAddressP;
    //unsigned char *cData;
    unsigned char rwData[20];
    memset(rwData,0,20);
    if(eMode == MB_REG_WRITE)
    {
        if(usAddrP != 0)
        {
            return MB_ENOREG;
        }
        RegData_HL_Swap_func((unsigned char *)&rwData[0], pucRegBuffer,2*usNRegs , 0, 10);   
        if(1 !=  func_Save_Device_Parameter(DEV_PD_DATE, (unsigned char *)&rwData[0]) )
        {
            return MB_EINVAL;
        }
    }
    return MB_ENOERR;
}

//设备软复位
eMBErrorCode Unit_W_DeviceReset_func( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode)
{
    unsigned char cVlaue = 1;
    return char_reg_rw_mode(pucRegBuffer, usAddressP, usNRegs, eMode, cVlaue, DEV_SOFTRESET);
}

/*参数单元表,寄存器地址对应参数占用的寄存器数，读写操作，预留空间调用Unit_RW_Null_func函数*/
static const MB_Data_Reg_Unit_S Modbus_Data_Reg_Unit[MD_MAX_REG_UNIT] = 
{
    {MB_RW_DEVICE_PARA_DEVICE_ID,MB_RW_DEVICE_PARA_REGISTER_FLAG - MB_RW_DEVICE_PARA_DEVICE_ID,Unit_RW_DeviceID_func}, 
    {MB_RW_DEVICE_PARA_REGISTER_FLAG,1,Unit_RW_DeviceRegisterFlag_func}, 
    {MB_R_DEVICE_PARA_IMSI,MB_R_DEVICE_PARA_IMEI - MB_R_DEVICE_PARA_IMSI,Unit_R_DeviceIMSI_func}, 
    {MB_R_DEVICE_PARA_IMEI,MB_R_DEVICE_PARA_HW_VERSION - MB_R_DEVICE_PARA_IMEI,Unit_R_DeviceIMEI_func}, 
    {MB_R_DEVICE_PARA_HW_VERSION,MB_R_DEVICE_PARA_SW_VERSION - MB_R_DEVICE_PARA_HW_VERSION,Unit_R_DeviceHWVersion_func},
    {MB_R_DEVICE_PARA_SW_VERSION,MB_R_DEVICE_PARA_PD_DATE - MB_R_DEVICE_PARA_SW_VERSION,Unit_R_DeviceSWVersion_func}, 
    {MB_R_DEVICE_PARA_PD_DATE,MB_RW_DEVICE_PARA_SAMPLE_GAP_CNT - MB_R_DEVICE_PARA_PD_DATE,Unit_R_DevicePDDate_func}, 
    {MB_RW_DEVICE_PARA_SAMPLE_GAP_CNT,MB_RW_DEVICE_PARA_RECORD_GAP_CNT - MB_RW_DEVICE_PARA_SAMPLE_GAP_CNT,Unit_RW_DeviceSampleGap_func}, 
    {MB_RW_DEVICE_PARA_RECORD_GAP_CNT,MB_RW_DEVICE_PARA_UPLOAD_GAP_CNT - MB_RW_DEVICE_PARA_RECORD_GAP_CNT,Unit_RW_DeviceRecordGap_func}, 
    {MB_RW_DEVICE_PARA_UPLOAD_GAP_CNT,1,Unit_RW_DeviceUploadGap_func}, 
    
    {MB_R_DEVICE_PARA_LOCA_LATITUDE,MB_R_DEVICE_PARA_LOCA_LNGITUDE - MB_R_DEVICE_PARA_LOCA_LATITUDE,Unit_R_DeviceLatitude_func},
    {MB_R_DEVICE_PARA_LOCA_LNGITUDE,MB_R_DEVICE_PARA_BATTERY_LEVEL - MB_R_DEVICE_PARA_LOCA_LNGITUDE,Unit_R_DeviceLngitude_func}, 
    {MB_R_DEVICE_PARA_BATTERY_LEVEL,MB_R_DEVICE_PARA_WATER_IMMERSION - MB_R_DEVICE_PARA_BATTERY_LEVEL,Unit_R_DeviceBatteryLevel_func},
    {MB_R_DEVICE_PARA_WATER_IMMERSION,MB_R_DEVICE_PARA_ATTITUDE_DATA - MB_R_DEVICE_PARA_WATER_IMMERSION,Unit_R_DeviceWaterImmersion_func}, 
    {MB_R_DEVICE_PARA_ATTITUDE_DATA,MB_R_DEVICE_PARA_PHOTOSENSITIVE_DATA - MB_R_DEVICE_PARA_ATTITUDE_DATA,Unit_R_DeviceAttitude_func},  
    {MB_R_DEVICE_PARA_PHOTOSENSITIVE_DATA,MB_R_DEVICE_PARA_ALARM_STATUS - MB_R_DEVICE_PARA_PHOTOSENSITIVE_DATA,Unit_R_DevicePhotosensitive_func},
    {MB_R_DEVICE_PARA_ALARM_STATUS,MB_RW_DEVICE_TOTAL_VOLUME_DATA - MB_R_DEVICE_PARA_ALARM_STATUS,Unit_R_DeviceAlarmStatus_func},
    {MB_RW_DEVICE_TOTAL_VOLUME_DATA,MB_RW_DEVICE_PARA_SENSOR1_ENABLE - MB_RW_DEVICE_TOTAL_VOLUME_DATA,Unit_RW_Device_Total_Volume_func},

    {MB_RW_DEVICE_PARA_SENSOR1_ENABLE,MB_RW_DEVICE_PARA_SENSOR2_ENABLE - MB_RW_DEVICE_PARA_SENSOR1_ENABLE,Unit_RW_DeviceSensor1Enable_func},
    {MB_RW_DEVICE_PARA_SENSOR2_ENABLE,MB_RW_DEVICE_PARA_SENSOR1_CNT - MB_RW_DEVICE_PARA_SENSOR2_ENABLE,Unit_RW_DeviceSensor2Enable_func},
    {MB_RW_DEVICE_PARA_SENSOR1_CNT,MB_RW_DEVICE_PARA_SENSOR2_CNT - MB_RW_DEVICE_PARA_SENSOR1_CNT,Unit_RW_DeviceSensor1Cnt_func}, 
    {MB_RW_DEVICE_PARA_SENSOR2_CNT,MB_RW_DEVICE_PARA_CHN1_SENSOR1_TYPE - MB_RW_DEVICE_PARA_SENSOR2_CNT,Unit_RW_DeviceSensor2Cnt_func}, 
    {MB_RW_DEVICE_PARA_CHN1_SENSOR1_TYPE,MB_RW_DEVICE_PARA_CHN1_SENSOR2_TYPE - MB_RW_DEVICE_PARA_CHN1_SENSOR1_TYPE,Unit_RW_Device_Chn1_Sensor1Type_func},
    {MB_RW_DEVICE_PARA_CHN1_SENSOR2_TYPE,MB_RW_DEVICE_PARA_CHN1_SENSOR3_TYPE - MB_RW_DEVICE_PARA_CHN1_SENSOR2_TYPE,Unit_RW_Device_Chn1_Sensor2Type_func}, 
    {MB_RW_DEVICE_PARA_CHN1_SENSOR3_TYPE,MB_RW_DEVICE_PARA_CHN1_SENSOR4_TYPE - MB_RW_DEVICE_PARA_CHN1_SENSOR3_TYPE,Unit_RW_Device_Chn1_Sensor3Type_func}, 
    {MB_RW_DEVICE_PARA_CHN1_SENSOR4_TYPE,MB_RW_DEVICE_PARA_CHN1_SENSOR5_TYPE - MB_RW_DEVICE_PARA_CHN1_SENSOR4_TYPE,Unit_RW_Device_Chn1_Sensor4Type_func},
    {MB_RW_DEVICE_PARA_CHN1_SENSOR5_TYPE,MB_RW_DEVICE_PARA_CHN1_SENSOR6_TYPE - MB_RW_DEVICE_PARA_CHN1_SENSOR5_TYPE,Unit_RW_Device_Chn1_Sensor5Type_func},
    {MB_RW_DEVICE_PARA_CHN1_SENSOR6_TYPE,MB_RW_DEVICE_PARA_CHN1_SENSOR7_TYPE - MB_RW_DEVICE_PARA_CHN1_SENSOR6_TYPE,Unit_RW_Device_Chn1_Sensor6Type_func},
    {MB_RW_DEVICE_PARA_CHN1_SENSOR7_TYPE,MB_RW_DEVICE_PARA_CHN1_SENSOR8_TYPE - MB_RW_DEVICE_PARA_CHN1_SENSOR7_TYPE,Unit_RW_Device_Chn1_Sensor7Type_func},
    {MB_RW_DEVICE_PARA_CHN1_SENSOR8_TYPE,MB_RW_DEVICE_PARA_CHN1_SENSOR9_TYPE - MB_RW_DEVICE_PARA_CHN1_SENSOR8_TYPE,Unit_RW_Device_Chn1_Sensor8Type_func},
    {MB_RW_DEVICE_PARA_CHN1_SENSOR9_TYPE,MB_RW_DEVICE_PARA_CHN1_SENSOR10_TYPE - MB_RW_DEVICE_PARA_CHN1_SENSOR9_TYPE,Unit_RW_Device_Chn1_Sensor9Type_func},
    {MB_RW_DEVICE_PARA_CHN1_SENSOR10_TYPE,MB_RW_DEVICE_PARA_CHN2_SENSOR1_TYPE - MB_RW_DEVICE_PARA_CHN1_SENSOR10_TYPE,Unit_RW_Device_Chn1_Sensor10Type_func},

    {MB_RW_DEVICE_PARA_CHN2_SENSOR1_TYPE,MB_RW_DEVICE_PARA_CHN2_SENSOR2_TYPE - MB_RW_DEVICE_PARA_CHN2_SENSOR1_TYPE,Unit_RW_Device_Chn2_Sensor1Type_func},
    {MB_RW_DEVICE_PARA_CHN2_SENSOR2_TYPE,MB_RW_DEVICE_PARA_CHN2_SENSOR3_TYPE - MB_RW_DEVICE_PARA_CHN2_SENSOR2_TYPE,Unit_RW_Device_Chn2_Sensor2Type_func}, 
    {MB_RW_DEVICE_PARA_CHN2_SENSOR3_TYPE,MB_RW_DEVICE_PARA_CHN2_SENSOR4_TYPE - MB_RW_DEVICE_PARA_CHN2_SENSOR3_TYPE,Unit_RW_Device_Chn2_Sensor3Type_func}, 
    {MB_RW_DEVICE_PARA_CHN2_SENSOR4_TYPE,MB_RW_DEVICE_PARA_CHN2_SENSOR5_TYPE - MB_RW_DEVICE_PARA_CHN2_SENSOR4_TYPE,Unit_RW_Device_Chn2_Sensor4Type_func},
    {MB_RW_DEVICE_PARA_CHN2_SENSOR5_TYPE,MB_RW_DEVICE_PARA_CHN2_SENSOR6_TYPE - MB_RW_DEVICE_PARA_CHN2_SENSOR5_TYPE,Unit_RW_Device_Chn2_Sensor5Type_func},
    {MB_RW_DEVICE_PARA_CHN2_SENSOR6_TYPE,MB_RW_DEVICE_PARA_CHN2_SENSOR7_TYPE - MB_RW_DEVICE_PARA_CHN2_SENSOR6_TYPE,Unit_RW_Device_Chn2_Sensor6Type_func},
    {MB_RW_DEVICE_PARA_CHN2_SENSOR7_TYPE,MB_RW_DEVICE_PARA_CHN2_SENSOR8_TYPE - MB_RW_DEVICE_PARA_CHN2_SENSOR7_TYPE,Unit_RW_Device_Chn2_Sensor7Type_func},
    {MB_RW_DEVICE_PARA_CHN2_SENSOR8_TYPE,MB_RW_DEVICE_PARA_CHN2_SENSOR9_TYPE - MB_RW_DEVICE_PARA_CHN2_SENSOR8_TYPE,Unit_RW_Device_Chn2_Sensor8Type_func},
    {MB_RW_DEVICE_PARA_CHN2_SENSOR9_TYPE,MB_RW_DEVICE_PARA_CHN2_SENSOR10_TYPE - MB_RW_DEVICE_PARA_CHN2_SENSOR9_TYPE,Unit_RW_Device_Chn2_Sensor9Type_func},
    {MB_RW_DEVICE_PARA_CHN2_SENSOR10_TYPE,MB_RW_DEVICE_STATUS_UPLOAD_GAP - MB_RW_DEVICE_PARA_CHN2_SENSOR10_TYPE,Unit_RW_Device_Chn2_Sensor10Type_func},
    {MB_RW_DEVICE_STATUS_UPLOAD_GAP,MB_RW_DEVICE_SERVER1_IP_ADDR - MB_RW_DEVICE_STATUS_UPLOAD_GAP,Unit_RW_Device_Status_Upload_Gap_func},
    {MB_RW_DEVICE_SERVER1_IP_ADDR,MB_RW_DEVICE_SERVER2_IP_ADDR - MB_RW_DEVICE_SERVER1_IP_ADDR,Unit_RW_Device_ServerIP1_func},
    {MB_RW_DEVICE_SERVER2_IP_ADDR,MB_RW_DEVICE_SERVER1_PORT - MB_RW_DEVICE_SERVER2_IP_ADDR,Unit_RW_Device_ServerIP2_func},
    {MB_RW_DEVICE_SERVER1_PORT,MB_RW_DEVICE_SERVER2_PORT - MB_RW_DEVICE_SERVER1_PORT,Unit_R_Device_ServerPort1_func},
    {MB_RW_DEVICE_SERVER2_PORT,MB_RW_DEVICE_DEBUG_MODEL - MB_RW_DEVICE_SERVER2_PORT,Unit_R_Device_ServerPort2_func},
    {MB_RW_DEVICE_DEBUG_MODEL,MB_RW_DEVICE_INSTALL_HEIGHT - MB_RW_DEVICE_DEBUG_MODEL,Unit_RW_Device_Debug_Model_func},
    {MB_RW_DEVICE_INSTALL_HEIGHT,MB_RW_DEVICE_SENSOR_BAUDRATE - MB_RW_DEVICE_INSTALL_HEIGHT,Unit_RW_Device_Install_Height_func},
    {MB_RW_DEVICE_SENSOR_BAUDRATE,1,Unit_RW_Device_Sensor_Baud_func},

    {MB_R_DEVICE_RADAR_LEVEL_SENSOR_VALUE,MB_R_DEVICE_PRESSURE_LEVEL_SENSOR_VALUE - MB_R_DEVICE_RADAR_LEVEL_SENSOR_VALUE,Unit_R_DeviceRadarLevel_func},
    {MB_R_DEVICE_PRESSURE_LEVEL_SENSOR_VALUE,MB_R_DEVICE_FLOWERMETER_SENSOR_VALUE - MB_R_DEVICE_PRESSURE_LEVEL_SENSOR_VALUE,Unit_R_DevicePressLevel_func},
    {MB_R_DEVICE_FLOWERMETER_SENSOR_VALUE,MB_R_DEVICE_INTEGRATED_CONDUCTIVITY_SENSOR_VALUE - MB_R_DEVICE_FLOWERMETER_SENSOR_VALUE,Unit_R_DeviceWaterVolume_s_func},
    {MB_R_DEVICE_INTEGRATED_CONDUCTIVITY_SENSOR_VALUE,2,Unit_R_DeviceWaterQuality_COND_func},
    
    {MB_W_DEVICE_PARA_PD_DATE,5,Unit_W_DevicePDDate_func}, 
    {MB_W_DEVICE_RESET_CMD,1,Unit_W_DeviceReset_func},
};

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/


eMBErrorCode eMBRTUReceive(unsigned char *ucRTUBuf, unsigned char * pucRcvAddress, unsigned char ** pucFrame, unsigned short * pusLength )
{
    //char            xFrameReceived = FALSE;
    eMBErrorCode    eStatus = MB_ENOERR;

    /* Length and CRC check */
    if( ( usRcvBufferPos >= MB_SER_PDU_SIZE_MIN ) && ( func_Get_MBCRC16( ucRTUBuf, usRcvBufferPos ) == 0 ) )
    {
        /* Save the address field. All frames are passed to the upper layed
         * and the decision if a frame is used is done there.
         */
        *pucRcvAddress = ucRTUBuf[MB_SER_PDU_ADDR_OFF];

        /* Total length of Modbus-PDU is Modbus-Serial-Line-PDU minus
         * size of address field and CRC checksum.
         */
        *pusLength = ( unsigned short )( usRcvBufferPos - MB_SER_PDU_PDU_OFF - MB_SER_PDU_SIZE_CRC );

        /* Return the start of the Modbus PDU to the caller. */
        *pucFrame = ( unsigned char * ) & ucRTUBuf[MB_SER_PDU_PDU_OFF];
       // xFrameReceived = TRUE;
    }
    else
    {
        eStatus = MB_EIO;
    }

    return eStatus;
}

//返回帧数据处理
eMBErrorCode eMBRTUSend( unsigned char ucSlaveAddress,unsigned char *ptSendBuf, unsigned char * pucFrame, unsigned short usLength , unsigned short *usSendCnt)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    unsigned short          usCRC16;
	unsigned short usSndBufferCount = 0;

    /* Check if the receiver is still in idle state. If not we where to
     * slow with processing the received frame and the master sent another
     * frame on the network. We have to abort sending the frame.
     */
	/* First byte before the Modbus-PDU is the slave address. */
	pucSndBufferCur = ( unsigned char * ) pucFrame - 1;
	usSndBufferCount = 1; 

	/* Now copy the Modbus-PDU into the Modbus-Serial-Line-PDU. */
	pucSndBufferCur[MB_SER_PDU_ADDR_OFF] = ucSlaveAddress;
	usSndBufferCount += usLength;

	memcpy(ptSendBuf,pucSndBufferCur,usSndBufferCount);

	/* Calculate CRC16 checksum for Modbus-Serial-Line-PDU. */
	usCRC16 = func_Get_MBCRC16( ( unsigned char * ) pucSndBufferCur, usSndBufferCount );
	ptSendBuf[usSndBufferCount++] = ( unsigned char )( usCRC16 & 0xFF );
	ptSendBuf[usSndBufferCount++] = ( unsigned char )( usCRC16 >> 8 );
	*usSendCnt = usSndBufferCount;

    return eStatus;
}

/*************************************************
Function   : gfunc_Modbus_Matching
Description: 地址匹配，返回映射地址起始位置，和
Input : * usAddress 输入地址，usNRegs 寄存器数
Return: 返回需要读写参数单元信息 Mb_Get_RegUnit_Info_S；
*************************************************/

Mb_Get_RegUnit_Info_S gfunc_Modbus_Matching(unsigned short usAddress, unsigned short usNRegs )
{
    Mb_Get_RegUnit_Info_S   getUintInfo;
    unsigned short   StartUnit = 0,UnitNum = 0,EndUnitReg = 0,StartUnitReg = 0;
    short index = 0, cReqNum = 0;
    memset(&getUintInfo, 0 ,sizeof(Mb_Get_RegUnit_Info_S));
    
    while(index < MD_MAX_REG_UNIT)//获取起始单元,偏移
    {
        if((Modbus_Data_Reg_Unit[index].RegAddr +  Modbus_Data_Reg_Unit[index].ArgRegNum) > usAddress )
        {
            StartUnit = index;//获取起始单元
            StartUnitReg = usAddress - Modbus_Data_Reg_Unit[StartUnit].RegAddr;//起始单元偏移
            /*偏移地址超过，寄存器单元空间，或者小于寄存器单元空间*/
            if(StartUnitReg > Modbus_Data_Reg_Unit[StartUnit].ArgRegNum || usAddress < Modbus_Data_Reg_Unit[StartUnit].RegAddr)
            {
                getUintInfo.retStat = 1;
                return getUintInfo;
            }
            break;
        }
        index++;
    }
    
    //计算单元个数
    cReqNum = usNRegs;
    cReqNum = cReqNum - (Modbus_Data_Reg_Unit[StartUnit + UnitNum].ArgRegNum - StartUnitReg);//第一个单元
    UnitNum++;
    while(0 < cReqNum)
    {
      cReqNum = cReqNum - Modbus_Data_Reg_Unit[StartUnit + UnitNum].ArgRegNum;
      UnitNum++;
    }
    if((StartUnit + UnitNum) > MD_MAX_REG_UNIT)
    {
        getUintInfo.retStat = 1;
        return getUintInfo;
    }
    
    //计算结束单元寄存器数
    if(UnitNum == 1)
    {
        EndUnitReg = cReqNum + (Modbus_Data_Reg_Unit[StartUnit + UnitNum - 1].ArgRegNum - StartUnitReg);;
    }
    else
    {
        EndUnitReg = cReqNum + Modbus_Data_Reg_Unit[StartUnit + UnitNum - 1].ArgRegNum;
    }
    
    
    if(index == MD_MAX_REG_UNIT )
    {
        getUintInfo.retStat = 1;
        return getUintInfo;
    }
    getUintInfo.StartUnit = StartUnit;//起始单元
    getUintInfo.UnitNum = UnitNum;//单元个数
    getUintInfo.StartUnitReg = StartUnitReg;//起始单元寄存器地址偏移
    getUintInfo.EndUnitReg = EndUnitReg;//结束单元寄存器数 
    getUintInfo.retStat = 0;//查询结果
    
    return getUintInfo;
}


//寄存器读写03 06 10
eMBErrorCode eMBRegHoldingCB( unsigned char * pucRegBuffer, unsigned short usAddress, unsigned short usNRegs, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
	usAddress --;
    short i = 0, RegAdressP = 0, Nreges = 0;
    Mb_Get_RegUnit_Info_S getUintInfo;
    if( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS)
    {
        getUintInfo = gfunc_Modbus_Matching(usAddress, usNRegs);//将需要读的，寄存器数，进行分解成参数单元。寄存器单元组成情况。
        if(getUintInfo.retStat == 1)//地址不存在
        {
            return MB_ENOREG;
        }
        /*根据参数单元组成情况，分布对单个单元处理后进行组帧*/
        for(i=0; i<getUintInfo.UnitNum; i++)
        {
            if(i == 0)//头偏移
            { 
                RegAdressP = getUintInfo.StartUnitReg;//与单个单元地址起始偏移
            }
            else
            {
                RegAdressP = 0;
            }
            if(i == (getUintInfo.UnitNum - 1))//尾个数
            {                
                Nreges =  getUintInfo.EndUnitReg;//单个单元寄存器数
            }
            else
            {
                Nreges = Modbus_Data_Reg_Unit[getUintInfo.StartUnit + i].ArgRegNum;
            }
            eStatus = Modbus_Data_Reg_Unit[getUintInfo.StartUnit + i].RWRegFunc(pucRegBuffer,RegAdressP,Nreges,eMode);//单个d读写单元写
            if( eStatus != MB_ENOERR)//读写错误
            {
                return eStatus;
            }
            pucRegBuffer += ((Nreges - RegAdressP)*2);
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

//03 功能码处理
eMBException eMBFuncReadHoldingRegister( unsigned char * pucFrame, unsigned short * usLen )
{
    unsigned short          usRegAddress;
    unsigned short          usRegCount;
    unsigned char          *pucFrameCur;

    eMBException    eStatus = MB_EX_NONE;
    eMBErrorCode    eRegStatus;

    if( *usLen == ( MB_PDU_FUNC_READ_SIZE + MB_PDU_SIZE_MIN ) )
    {
        usRegAddress = ( unsigned short )( pucFrame[MB_PDU_FUNC_READ_ADDR_OFF] << 8 );
        usRegAddress |= ( unsigned short )( pucFrame[MB_PDU_FUNC_READ_ADDR_OFF + 1] );
        usRegAddress++;

        usRegCount = ( unsigned short )( pucFrame[MB_PDU_FUNC_READ_REGCNT_OFF] << 8 );
        usRegCount = ( unsigned short )( pucFrame[MB_PDU_FUNC_READ_REGCNT_OFF + 1] );

        /* Check if the number of registers to read is valid. If not
         * return Modbus illegal data value exception. 
         */
        if( ( usRegCount >= 1 ) && ( usRegCount <= MB_PDU_FUNC_READ_REGCNT_MAX ) )
        {
            /* Set the current PDU data pointer to the beginning. */
            pucFrameCur = &pucFrame[MB_PDU_FUNC_OFF];
            *usLen = MB_PDU_FUNC_OFF;

            /* First byte contains the function code. */
            *pucFrameCur++ = MB_FUNC_READ_HOLDING_REGISTER;
            *usLen += 1;

            /* Second byte in the response contain the number of bytes. */
            *pucFrameCur++ = ( unsigned char ) ( usRegCount * 2 );
            *usLen += 1;

            /* Make callback to fill the buffer. */
            eRegStatus = eMBRegHoldingCB( pucFrameCur, usRegAddress, usRegCount, MB_REG_READ );
            /* If an error occured convert it into a Modbus exception. */
            if( eRegStatus != MB_ENOERR )
            {
                eStatus = prveMBError2Exception( eRegStatus );
            }
            else
            {
                *usLen += usRegCount * 2;
            }
        }
        else
        {
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else
    {
        /* Can't be a valid request because the length is incorrect. */
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}

//06 功能码处理
eMBException eMBFuncWriteHoldingRegister( unsigned char * pucFrame, unsigned short * usLen )
{
    unsigned short          usRegAddress;
    eMBException    eStatus = MB_EX_NONE;
    eMBErrorCode    eRegStatus;

    if( *usLen == ( MB_PDU_FUNC_WRITE_SIZE + MB_PDU_SIZE_MIN ) )
    {
        usRegAddress = ( unsigned short )( pucFrame[MB_PDU_FUNC_WRITE_ADDR_OFF] << 8 );
        usRegAddress |= ( unsigned short )( pucFrame[MB_PDU_FUNC_WRITE_ADDR_OFF + 1] );
        usRegAddress++;

        /* Make callback to update the value. */
        eRegStatus = eMBRegHoldingCB( &pucFrame[MB_PDU_FUNC_WRITE_VALUE_OFF],
                                      usRegAddress, 1, MB_REG_WRITE );

        /* If an error occured convert it into a Modbus exception. */
        if( eRegStatus != MB_ENOERR )
        {
            eStatus = prveMBError2Exception( eRegStatus );
        }
    }
    else
    {
        /* Can't be a valid request because the length is incorrect. */
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}

//0x10 功能码处理
eMBException eMBFuncWriteMultipleHoldingRegister( unsigned char * pucFrame, unsigned short * usLen )
{
    unsigned short          usRegAddress;
    unsigned short          usRegCount;
    unsigned char           ucRegByteCount;

    eMBException    eStatus = MB_EX_NONE;
    eMBErrorCode    eRegStatus;

    if( *usLen >= ( MB_PDU_FUNC_WRITE_MUL_SIZE_MIN + MB_PDU_SIZE_MIN ) )
    {
        usRegAddress = ( unsigned short )( pucFrame[MB_PDU_FUNC_WRITE_MUL_ADDR_OFF] << 8 );
        usRegAddress |= ( unsigned short )( pucFrame[MB_PDU_FUNC_WRITE_MUL_ADDR_OFF + 1] );
        usRegAddress++;

        usRegCount = ( unsigned short )( pucFrame[MB_PDU_FUNC_WRITE_MUL_REGCNT_OFF] << 8 );
        usRegCount |= ( unsigned short )( pucFrame[MB_PDU_FUNC_WRITE_MUL_REGCNT_OFF + 1] );

        ucRegByteCount = pucFrame[MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF];

        if( ( usRegCount >= 1 ) &&
            ( usRegCount <= MB_PDU_FUNC_WRITE_MUL_REGCNT_MAX ) &&
            ( ucRegByteCount == ( unsigned char ) ( 2 * usRegCount ) ) )
        {
            /* Make callback to update the register values. */
            eRegStatus =
                eMBRegHoldingCB( &pucFrame[MB_PDU_FUNC_WRITE_MUL_VALUES_OFF],
                                 usRegAddress, usRegCount, MB_REG_WRITE );

            /* If an error occured convert it into a Modbus exception. */
            if( eRegStatus != MB_ENOERR )
            {
                eStatus = prveMBError2Exception( eRegStatus );
            }
            else
            {
                /* The response contains the function code, the starting
                 * address and the quantity of registers. We reuse the
                 * old values in the buffer because they are still valid.
                 */
                *usLen = MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF;
            }
        }
        else
        {
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else
    {
        /* Can't be a valid request because the length is incorrect. */
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}

 /* ******************************************************************************************************************
 * @brief  已经完成DMA串口收发，直接modbus处理函数
 * @param   usRcvBuffer     接收帧指针
            pucSndBuffer    发送帧指针
            rcvNum          接收字符串大小
            SndNum          发送字符串大小
 * @retval 
 *******************************************************************************************************************/ 
eMBErrorCode eMBDMASendRevic( unsigned char *usRcvBuffer, unsigned char * pucSndBuffer, unsigned short rcvNum, unsigned short *SndNum)
{
	static unsigned char   *ucMBFrame;
    static unsigned char    ucRcvAddress;
    static unsigned char    ucFunctionCode;
    static unsigned short   usLength;
	static eMBException eException;

	int             i;
	eMBErrorCode    eStatus = MB_ENOERR;

	usRcvBufferPos = rcvNum;

	eStatus = eMBRTUReceive(usRcvBuffer, &ucRcvAddress, &ucMBFrame, &usLength );
	//经过上一步函数处理后，ucMBFrame指针指向例如01 03 00 00 00 02 C4 0B 的接收帧中的03指令码位置; usLength长度为接收帧长度-地址字节-CRC校验字节
	if( eStatus == MB_ENOERR )
	{
		/* Check if the frame is for us. If not ignore the frame. */
		if( ( ucRcvAddress == MB_DEVICE_ADDR ) || ( ucRcvAddress == MB_ADDRESS_BROADCAST ) )
		{
			ucFunctionCode = ucMBFrame[MB_PDU_FUNC_OFF];
            eException = MB_EX_ILLEGAL_FUNCTION;
            for( i = 0; i < MB_FUNC_HANDLERS_MAX; i++ )
            {
                /* No more function handlers registered. Abort. */
                if( xFuncHandlers[i].ucFunctionCode == 0 )
                {
                    break;
                }
                else if( xFuncHandlers[i].ucFunctionCode == ucFunctionCode )
                {
                    eException = xFuncHandlers[i].pxHandler( ucMBFrame, &usLength );
                    break;
                }
            }

            /* If the request was not sent to the broadcast address we
             * return a reply. */
            if( ucRcvAddress != MB_ADDRESS_BROADCAST )
            {
                if( eException != MB_EX_NONE )
                {
                    /* An exception occured. Build an error frame. */
                    usLength = 0;
                    ucMBFrame[usLength++] = ( unsigned char )( ucFunctionCode | MB_FUNC_ERROR );
                    ucMBFrame[usLength++] = eException;
                }
          
                eStatus = eMBRTUSend( MB_DEVICE_ADDR, pucSndBuffer, ucMBFrame, usLength, SndNum );
            }
		}
	}
	return eStatus;
}


/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
