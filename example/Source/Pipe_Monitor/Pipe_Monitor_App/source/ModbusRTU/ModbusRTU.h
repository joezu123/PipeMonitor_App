/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\ModbusRTU\ModbusRTU.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-05-16       Joe             First version
 @endverbatim

 */
#ifndef __MODBUSRTU_H__
#define __MODBUSRTU_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "drv_USART.h"
#include "User_Data.h"
#include "Mainloop.h"
#include "hc32f460_utility.h"
/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define BY_INTEGRATED_CONDUCTIVITY_SENSOR_ADDR 10    //博雅一体式数字电导率设备地址
#define FLOWERMETER_SENSOR_ADDR 1
#define BY_RADAR_WATER_LEVEL_SENSOR_ADDR 2 //博雅-雷达水位计设备地址
#define BY_PRESSURE_WATER_LEVEL_SENSOR_ADDR 1 //博雅-压力水位计设备地址
#define HZ_RADAR_ULTRASONIC_FLOW_SENSOR_ADDR 0x34 //航征-雷达超声波流量计设备地址
#define HZ_RADAR_WATER_LEVEL_SENSOR_ADDR 0x35 //航征-雷达水位计设备地址

#define HX_FLOWMETER_SENSOR_ADDR 0x10 //恒星-截污流量计设备地址
#define HX_WATER_QUALITY_COD_SENSOR_ADDR 0x11 //恒星-水质COD传感器设备地址
#define HX_RADAR_ULTRASONIC_FLOW_SENSOR_ADDR 0x12 //恒星-雷达超声波流量计设备地址

#define MB_DEVICE_ADDR 0xF0 //Modbus设备地址

/* ----------------------- Defines ------------------------------------------*/
#define MB_ADDRESS_BROADCAST    ( 0 )   /*! Modbus broadcast address. */
#define MB_ADDRESS_MIN          ( 1 )   /*! Smallest possible slave address. */
#define MB_ADDRESS_MAX          ( 247 ) /*! Biggest possible slave address. */
#define MB_FUNC_NONE                          (  0 )
#define MB_FUNC_READ_COILS                    (  1 )
#define MB_FUNC_READ_DISCRETE_INPUTS          (  2 )
#define MB_FUNC_WRITE_SINGLE_COIL             (  5 )
#define MB_FUNC_WRITE_MULTIPLE_COILS          ( 15 )
#define MB_FUNC_READ_HOLDING_REGISTER         (  3 )
#define MB_FUNC_READ_INPUT_REGISTER           (  4 )
#define MB_FUNC_WRITE_REGISTER                (  6 )
#define MB_FUNC_WRITE_MULTIPLE_REGISTERS      ( 16 )
#define MB_FUNC_READWRITE_MULTIPLE_REGISTERS  ( 23 )
#define MB_FUNC_DIAG_READ_EXCEPTION           (  7 )
#define MB_FUNC_DIAG_DIAGNOSTIC               (  8 )
#define MB_FUNC_DIAG_GET_COM_EVENT_CNT        ( 11 )
#define MB_FUNC_DIAG_GET_COM_EVENT_LOG        ( 12 )
#define MB_FUNC_OTHER_REPORT_SLAVEID          ( 17 )
#define MB_FUNC_ERROR                         ( 128 )

typedef enum
{
    MB_EX_NONE = 0x00,
    MB_EX_ILLEGAL_FUNCTION = 0x01,
    MB_EX_ILLEGAL_DATA_ADDRESS = 0x02,
    MB_EX_ILLEGAL_DATA_VALUE = 0x03,
    MB_EX_SLAVE_DEVICE_FAILURE = 0x04,
    MB_EX_ACKNOWLEDGE = 0x05,
    MB_EX_SLAVE_BUSY = 0x06,
    MB_EX_MEMORY_PARITY_ERROR = 0x08,
    MB_EX_GATEWAY_PATH_FAILED = 0x0A,
    MB_EX_GATEWAY_TGT_FAILED = 0x0B
} eMBException;

#define MB_SER_PDU_SIZE_MIN     4       /*!< Minimum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_MAX     256     /*!< Maximum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_CRC     2       /*!< Size of CRC field in PDU. */
#define MB_SER_PDU_ADDR_OFF     0       /*!< Offset of slave address in Ser-PDU. */
#define MB_SER_PDU_PDU_OFF      1       /*!< Offset of Modbus-PDU in Ser-PDU. */

#define MB_PDU_SIZE_MAX     253 /*!< Maximum size of a PDU. */
#define MB_PDU_SIZE_MIN     1   /*!< Function Code */
#define MB_PDU_FUNC_OFF     0   /*!< Offset of function code in PDU. */
#define MB_PDU_DATA_OFF     1   /*!< Offset for response data in PDU. */

#define MB_PDU_FUNC_READ_ADDR_OFF               ( MB_PDU_DATA_OFF + 0)
#define MB_PDU_FUNC_READ_REGCNT_OFF             ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_READ_SIZE                   ( 4 )
#define MB_PDU_FUNC_READ_REGCNT_MAX             ( 0x007D )

#define MB_PDU_FUNC_WRITE_ADDR_OFF              ( MB_PDU_DATA_OFF + 0)
#define MB_PDU_FUNC_WRITE_VALUE_OFF             ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_WRITE_SIZE                  ( 4 )

#define MB_PDU_FUNC_WRITE_MUL_ADDR_OFF          ( MB_PDU_DATA_OFF + 0 )
#define MB_PDU_FUNC_WRITE_MUL_REGCNT_OFF        ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF       ( MB_PDU_DATA_OFF + 4 )
#define MB_PDU_FUNC_WRITE_MUL_VALUES_OFF        ( MB_PDU_DATA_OFF + 5 )
#define MB_PDU_FUNC_WRITE_MUL_SIZE_MIN          ( 5 )
#define MB_PDU_FUNC_WRITE_MUL_REGCNT_MAX        ( 0x0078 )

#define MB_DEVICE_PARA_BASE 0x0100 //设备参数寄存器基地址

#define MB_RW_DEVICE_PARA_DEVICE_ID         (MB_DEVICE_PARA_BASE + 0x0000) //设备ID寄存器地址
#define MB_RW_DEVICE_PARA_REGISTER_FLAG     (MB_DEVICE_PARA_BASE + 0x0008) //注册标志寄存器地址
#define MB_R_DEVICE_PARA_IMSI               (MB_DEVICE_PARA_BASE + 0x0009) //IMSI寄存器地址
#define MB_R_DEVICE_PARA_IMEI               (MB_DEVICE_PARA_BASE + 0x0011) //IMEI寄存器地址
#define MB_R_DEVICE_PARA_HW_VERSION         (MB_DEVICE_PARA_BASE + 0x0019) //硬件版本寄存器地址
#define MB_R_DEVICE_PARA_SW_VERSION       	(MB_DEVICE_PARA_BASE + 0x001E) //软件版本寄存器地址
#define MB_R_DEVICE_PARA_PD_DATE            (MB_DEVICE_PARA_BASE + 0x0023) //生产日期寄存器地址
#define MB_RW_DEVICE_PARA_SAMPLE_GAP_CNT    (MB_DEVICE_PARA_BASE + 0x0028) //采样间隔寄存器地址
#define MB_RW_DEVICE_PARA_RECORD_GAP_CNT	(MB_DEVICE_PARA_BASE + 0x0029) //记录间隔寄存器地址
#define MB_RW_DEVICE_PARA_UPLOAD_GAP_CNT	(MB_DEVICE_PARA_BASE + 0x002A) //上传间隔寄存器地址

#define MB_R_DEVICE_PARA_LOCA_LATITUDE      (MB_DEVICE_PARA_BASE + 0x0032) //纬度寄存器地址
#define MB_R_DEVICE_PARA_LOCA_LNGITUDE      (MB_DEVICE_PARA_BASE + 0x0034) //经度寄存器地址
#define MB_R_DEVICE_PARA_BATTERY_LEVEL      (MB_DEVICE_PARA_BASE + 0x0036) //电池电量寄存器地址
#define MB_R_DEVICE_PARA_WATER_IMMERSION    (MB_DEVICE_PARA_BASE + 0x0038) //水浸传感器寄存器地址
#define MB_R_DEVICE_PARA_ATTITUDE_DATA       (MB_DEVICE_PARA_BASE + 0x0039) //姿态传感器数据寄存器地址
#define MB_R_DEVICE_PARA_PHOTOSENSITIVE_DATA (MB_DEVICE_PARA_BASE + 0x003A) //光照传感器寄存器地址
#define MB_R_DEVICE_PARA_ALARM_STATUS		 (MB_DEVICE_PARA_BASE + 0x003B) //报警状态寄存器地址
#define MB_RW_DEVICE_TOTAL_VOLUME_DATA    (MB_DEVICE_PARA_BASE + 0X003C)  //设备累计流量寄存器地址

#define MB_RW_DEVICE_PARA_SENSOR1_ENABLE  (MB_DEVICE_PARA_BASE + 0X003E)  //设备485-1使能标志
#define MB_RW_DEVICE_PARA_SENSOR2_ENABLE  (MB_DEVICE_PARA_BASE + 0X003F)  //设备485-2使能标志
#define MB_RW_DEVICE_PARA_SENSOR1_CNT		(MB_DEVICE_PARA_BASE + 0x0040) //485-1传感器数量寄存器地址
#define MB_RW_DEVICE_PARA_SENSOR2_CNT		(MB_DEVICE_PARA_BASE + 0x0041) //485-2传感器数量寄存器地址
#define MB_RW_DEVICE_PARA_CHN1_SENSOR1_TYPE		(MB_DEVICE_PARA_BASE + 0x0042) //485-1传感器类型1寄存器地址
#define MB_RW_DEVICE_PARA_CHN1_SENSOR2_TYPE		(MB_DEVICE_PARA_BASE + 0x0043) //485-1传感器类型2寄存器地址
#define MB_RW_DEVICE_PARA_CHN1_SENSOR3_TYPE		(MB_DEVICE_PARA_BASE + 0x0044) //485-1传感器类型3寄存器地址
#define MB_RW_DEVICE_PARA_CHN1_SENSOR4_TYPE		(MB_DEVICE_PARA_BASE + 0x0045) //485-1传感器类型4寄存器地址
#define MB_RW_DEVICE_PARA_CHN1_SENSOR5_TYPE		(MB_DEVICE_PARA_BASE + 0x0046) //485-1传感器类型5寄存器地址
#define MB_RW_DEVICE_PARA_CHN1_SENSOR6_TYPE		(MB_DEVICE_PARA_BASE + 0x0047) //485-1传感器类型6寄存器地址
#define MB_RW_DEVICE_PARA_CHN1_SENSOR7_TYPE		(MB_DEVICE_PARA_BASE + 0x0048) //485-1传感器类型7寄存器地址
#define MB_RW_DEVICE_PARA_CHN1_SENSOR8_TYPE		(MB_DEVICE_PARA_BASE + 0x0049) //485-1传感器类型8寄存器地址
#define MB_RW_DEVICE_PARA_CHN1_SENSOR9_TYPE		(MB_DEVICE_PARA_BASE + 0x004A) //485-1传感器类型9寄存器地址
#define MB_RW_DEVICE_PARA_CHN1_SENSOR10_TYPE		(MB_DEVICE_PARA_BASE + 0x004B) //485-1传感器类型10寄存器地址

#define MB_RW_DEVICE_PARA_CHN2_SENSOR1_TYPE		(MB_DEVICE_PARA_BASE + 0x004C) //485-2传感器类型1寄存器地址
#define MB_RW_DEVICE_PARA_CHN2_SENSOR2_TYPE		(MB_DEVICE_PARA_BASE + 0x004D) //485-2传感器类型2寄存器地址
#define MB_RW_DEVICE_PARA_CHN2_SENSOR3_TYPE		(MB_DEVICE_PARA_BASE + 0x004E) //485-2传感器类型3寄存器地址
#define MB_RW_DEVICE_PARA_CHN2_SENSOR4_TYPE		(MB_DEVICE_PARA_BASE + 0x004F) //485-2传感器类型4寄存器地址
#define MB_RW_DEVICE_PARA_CHN2_SENSOR5_TYPE		(MB_DEVICE_PARA_BASE + 0x0050) //485-2传感器类型5寄存器地址
#define MB_RW_DEVICE_PARA_CHN2_SENSOR6_TYPE		(MB_DEVICE_PARA_BASE + 0x0051) //485-2传感器类型6寄存器地址
#define MB_RW_DEVICE_PARA_CHN2_SENSOR7_TYPE		(MB_DEVICE_PARA_BASE + 0x0052) //485-2传感器类型7寄存器地址
#define MB_RW_DEVICE_PARA_CHN2_SENSOR8_TYPE		(MB_DEVICE_PARA_BASE + 0x0053) //485-2传感器类型8寄存器地址
#define MB_RW_DEVICE_PARA_CHN2_SENSOR9_TYPE		(MB_DEVICE_PARA_BASE + 0x0054) //485-2传感器类型9寄存器地址
#define MB_RW_DEVICE_PARA_CHN2_SENSOR10_TYPE		(MB_DEVICE_PARA_BASE + 0x0055) //485-2传感器类型10寄存器地址
#define MB_RW_DEVICE_STATUS_UPLOAD_GAP        (MB_DEVICE_PARA_BASE + 0x0056) //状态上传间隔寄存器地址 
#define MB_RW_DEVICE_SERVER1_IP_ADDR     (MB_DEVICE_PARA_BASE + 0x0057) //服务器1 IP地址寄存器地址
#define MB_RW_DEVICE_SERVER2_IP_ADDR     (MB_DEVICE_PARA_BASE + 0x005F) //服务器2 IP地址寄存器地址
#define MB_RW_DEVICE_SERVER1_PORT       (MB_DEVICE_PARA_BASE + 0x0067) //服务器1 端口寄存器地址
#define MB_RW_DEVICE_SERVER2_PORT       (MB_DEVICE_PARA_BASE + 0x0068) //服务器2 端口寄存器地址`  
#define MB_RW_DEVICE_DEBUG_MODEL        (MB_DEVICE_PARA_BASE + 0x0069) //设备调试模式寄存器地址
#define MB_RW_DEVICE_INSTALL_HEIGHT     (MB_DEVICE_PARA_BASE + 0x006A) //设备安装井深寄存器地址
#define MB_RW_DEVICE_SENSOR_BAUDRATE    (MB_DEVICE_PARA_BASE + 0x006C) //设备485波特率寄存器地址

#define MB_R_DEVICE_RADAR_LEVEL_SENSOR_VALUE (MB_DEVICE_PARA_BASE + 0x0070) //雷达水位计传感器值寄存器地址
#define MB_R_DEVICE_PRESSURE_LEVEL_SENSOR_VALUE (MB_DEVICE_PARA_BASE + 0x0072) //压力水位计传感器值寄存器地址
#define MB_R_DEVICE_FLOWERMETER_SENSOR_VALUE (MB_DEVICE_PARA_BASE + 0x0074) //截污流量计传感器值寄存器地址
#define MB_R_DEVICE_INTEGRATED_CONDUCTIVITY_SENSOR_VALUE (MB_DEVICE_PARA_BASE + 0x0076) //一体式数字电导率传感器值寄存器地址

#define MB_W_DEVICE_PARA_PD_DATE          (MB_DEVICE_PARA_BASE + 0x0100) //生产日期寄存器地址
#define MB_W_DEVICE_RESET_CMD             (MB_DEVICE_PARA_BASE + 0x0105) //设备复位命令寄存器地址

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

 typedef enum
 {
	 MB_REG_READ,                /*!< Read register values and pass to protocol stack. */
	 MB_REG_WRITE                /*!< Update register values. */
 } eMBRegisterMode;
 
 /*! \ingroup modbus
  * \brief Errorcodes used by all function in the protocol stack.
  */
 typedef enum
 {
	 MB_ENOERR,                  /*!< no error. */
	 MB_ENOREG,                  /*!< illegal register address. */
	 MB_EINVAL,                  /*!< illegal argument. */
	 MB_EPORTERR,                /*!< porting layer error. */
	 MB_ENORES,                  /*!< insufficient resources. */
	 MB_EIO,                     /*!< I/O error. */
	 MB_EILLSTATE,               /*!< protocol stack in illegal state. */
	 MB_ETIMEDOUT                /*!< timeout error occurred. */
 } eMBErrorCode;

 typedef         eMBException( *pxMBFunctionHandler ) ( unsigned char * pucFrame, unsigned short * pusLength );

 typedef struct
{
    unsigned char           ucFunctionCode;
    pxMBFunctionHandler pxHandler;
} xMBFunctionHandler;



/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern unsigned short func_Get_MBCRC16( unsigned char * pucFrame, unsigned short usLen );
extern void RegData_HL_Swap_func(unsigned char * OScbuf,const unsigned char * ScData,unsigned char IDataNum, unsigned char HLflag, unsigned char Typelong);
extern eMBErrorCode char_reg_rw_mode( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode, unsigned char CVlaue, en_SaveParaCMD inOpCommand);
extern eMBErrorCode Short_reg_rw_mode( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode, unsigned short SVlaue, en_SaveParaCMD inOpCommand);
extern eMBErrorCode Float_reg_rw_mode( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode, float FVlaue, en_SaveParaCMD inOpCommand);
extern eMBErrorCode Int_reg_rw_mode( unsigned char * pucRegBuffer, unsigned short usAddressP, unsigned short usNRegs, eMBRegisterMode eMode, int IVlaue, en_SaveParaCMD inOpCommand);
extern void drv_ModbusRTU_Init(void);
extern void func_Meas_Sensor_Dispose(void);
extern unsigned char func_BlackLight_Sensor_Dispose(void);


extern unsigned char func_Get_Meas_BY_Integrated_Conductivity_Sensor_Value(en_usart_device_t ucDeviceType);

extern eMBErrorCode eMBDMASendRevic( unsigned char *usRcvBuffer, unsigned char * pucSndBuffer, unsigned short rcvNum, unsigned short *SndNum);

extern SystemPataSt *pst_MBSystemPara;
#ifdef __cplusplus
}
#endif

#endif /* __BT_TB05_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
