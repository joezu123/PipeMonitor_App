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
#define SYSTEM_RECORD_START_ADDR  0x1000  //系统测量数据保存地址
//#define SYSTEM_BACKUP_PARA_ADDR 0x4000  //系统配置参数备份地址，写以一扇区为单位4096Bytes
#define SYSTEM_RECORD_SIZE  sizeof(DevMeasRecordDataSt)
/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/




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
extern void func_Save_Device_MeasData();
extern void func_Get_Device_MeasData_Record(int nRecordIndex, DevMeasRecordDataSt *pstMeasData);
#ifdef __cplusplus
}
#endif

#endif /* __DRV_STORAGE_GD25_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
