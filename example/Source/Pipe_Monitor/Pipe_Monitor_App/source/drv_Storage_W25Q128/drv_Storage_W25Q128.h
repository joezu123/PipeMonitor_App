/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\driver\drv_device\drv_Storage_GD25\drv_Storage_GD25.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-20       Joe             First version
 @endverbatim

 */
#ifndef __DRV_STORAGE_W25Q128_H__
#define __DRV_STORAGE_W25Q128_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ddl.h"
#include "hc32f460_gpio.h"
#include "User_Data.h"
#include "drv_SPI.h"

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define SYSTEM_PARA_ADDR  0x0000  //系统配置参数保存地址，写以一扇区为单位4096Bytes
#define DATA_BLOCK_SIZE         1024    // 单个数据块大小（字节），4KB整数倍
#define NORMAL_DATA_AREA_ADDR   0x000000// 正常区起始地址（0x000000 ~ 0x000FFF）
#define BACKUP_DATA_AREA_ADDR   0x001000// 备份区起始地址（0x001000 ~ 0x001FFF）
#define CRC32_STORE_ADDR        0x002000// CRC32校验值存储地址
#define SYSTEM_RECORD_START_ADDR  0x3000  //系统测量数据保存地址
//#define SYSTEM_BACKUP_PARA_ADDR 0x4000  //系统配置参数备份地址，写以一扇区为单  位4096Bytes
#define SYSTEM_RECORD_SIZE  sizeof(DevMeasRecordDataSt)
/***** 类型2：记录数据区（0x4000起始，结构体单位，无需备份，高效率） *****/
#define RECORD_STRUCT_SIZE       sizeof(DevMeasRecordDataSt) // 单个记录结构体大小
#define MAX_RECORD_COUNT         377650//389800                          // 最大记录条数（可调整）(16777216-0x3000-升级文件区域1024*512) / SYSTEM_RECORD_SIZE
#define RECORD_DATA_START_ADDR   0x003000// 记录数据区起始地址（与设置参数区隔离开）
#define RECORD_DATA_MAX_ADDR     (RECORD_DATA_START_ADDR + MAX_RECORD_COUNT * RECORD_STRUCT_SIZE)

#define INIT_RETRY_COUNT       3       // 初始化最大重试次数
/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

// 引脚电平操作宏
#define NSS_LOW()              (PORT_ResetBits(SPI_W25Q128_NSS_PORT, SPI_W25Q128_NSS_PIN))  // NSS拉低（选中W25Q128）
#define NSS_HIGH()             (PORT_SetBits(SPI_W25Q128_NSS_PORT, SPI_W25Q128_NSS_PIN)) // NSS拉高（取消选中）
#define SCK_LOW()              (PORT_ResetBits(SPI_W25Q128_SCK_PORT, SPI_W25Q128_SCK_PIN))  // SCK拉低
#define SCK_HIGH()             (PORT_SetBits(SPI_W25Q128_SCK_PORT, SPI_W25Q128_SCK_PIN)) // SCK拉高
#define MOSI_LOW()             (PORT_ResetBits(SPI_W25Q128_MOSI_PORT, SPI_W25Q128_MOSI_PIN)) // MOSI拉低
#define MOSI_HIGH()            (PORT_SetBits(SPI_W25Q128_MOSI_PORT, SPI_W25Q128_MOSI_PIN))// MOSI拉高
#define MISO_READ()            (PORT_GetBit(SPI_W25Q128_MISO_PORT, SPI_W25Q128_MISO_PIN)) // 读取MISO电平

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern uint8_t drv_Storage_W25Q128_Init(void);
extern void W25Q128_Spi_flash_buffer_write(uint8_t* pbuffer, uint32_t write_addr, uint16_t len);
extern void W25Q128_Get_ReadDataBytes(uint32_t dest_addr, uint8_t *data, uint32_t len);
extern unsigned char func_Save_Device_Parameter(en_SaveParaCMD eCMD, unsigned char *cDataArr);
extern void func_Device_Parameter_Init(void);
extern unsigned char func_Save_Device_MeasData();
extern void func_Get_Device_MeasData_Record(int nRecordIndex, DevMeasRecordDataSt *pstMeasData);

#ifdef NEW_W25Q128_DRIVER
extern int System_PowerOn_Storage_Init(void);
extern bool Data_DoubleArea_Write(const uint8_t *p_new_data, uint32_t length);
extern bool RecordData_Write(uint32_t record_index, const DevMeasRecordDataSt *p_record);
extern bool RecordData_Read(uint32_t record_index, DevMeasRecordDataSt *p_record);
extern bool W25Q128_Check_Exist(void);
extern bool g_flash_ready;
#endif
#ifdef __cplusplus
}
#endif

#endif /* __DRV_STORAGE_GD25_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
