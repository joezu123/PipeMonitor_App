/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\driver\drv_device\drv_Storage_GD25\drv_Storage_GD25.c
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
#include "drv_Storage_W25Q128.h"
#include "Mainloop.h"
/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define CMD_PAGE_PROGRAM           0x02
#define CMD_READ_DATA_BYTES        0x03
#define CMD_READ_STA_REG           0x05
#define CMD_WRITE_ENABLE           0x06
#define CMD_SECTOR_ERASE           0x20
#define GET_DEVICE_ID              0x90
#define GET_IDENTIFICATION         0x9f
 

// W25Q128 基础指令
#define W25Q128_CMD_READ        0x03    // 普通读取指令
#define W25Q128_CMD_PAGE_PROG   0x02    // 页编程（写入）指令
#define W25Q128_CMD_SECTOR_ERASE 0x20   // 4KB扇区擦除指令
#define W25Q128_CMD_WRITE_ENABLE 0x06   // 写使能指令
#define W25Q128_CMD_READ_STATUS  0x05   // 读取状态寄存器指令
 
#define  SFLASH_ID       						0xC84018  //Flash ID

 
#define SPI_FLASH_NSS_ENABLE 	    PORT_ResetBits(SPI_W25Q128_NSS_PORT, SPI_W25Q128_NSS_PIN)
#define SPI_FLASH_NSS_DISENABLE 	PORT_SetBits(SPI_W25Q128_NSS_PORT, SPI_W25Q128_NSS_PIN)
 
 
#define SPI_FLASH_PAGE_SIZE    0x100 //GD25Qxx每页有256个字节
#define WIP_FLAG         0x01     /* write in progress(wip)flag */
 
#define DEVICE_SIZE 	0x1000000 	// 16M = 256 * BLOCK
#define BLOCK_SIZE		0x10000		// 64K	= 16 * SECTOR
#define SECTOR_SIZE		0x1000 		// 4K = 16 * PAGE
#define PAGE_SIZE		  0x100 		// 256Bytes
#define W25Q128_SECTOR_SIZE      4096                      // W25Q128 单个扇区大小（4KB）

uint8_t guc_W25Q128_Buf[4096];
bool g_flash_ready = false;                                // Flash就绪标志
/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
extern void func_Device_Parameter_Factory_Reset(void);
/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
SystemPataSt *pst_W25Q128SystemPara;
/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
void Delay_NOP(uint32_t nops)
{
    while(nops--)
    {
        Ddl_Delay1us(1); //__asm__ volatile ("nop");
    }
}


union W25Q128_UINT32_DATA_TYPE
{
	uint8_t b[4];
    uint32_t Uint32_Data;
};

#ifdef NEW_W25Q128_DRIVER
unsigned char g_data_buffer[DATA_BLOCK_SIZE];
/************************** 软件SPI 读写实现（适配指定引脚） **************************/
/**
 * @brief  软件SPI发送一个字节（标准SPI：CPOL=0，CPHA=0）
 * @param  data: 待发送字节
 * @retval 接收的应答字节（同步收发）
 */
uint8_t SPI_Send_Byte(uint8_t data)
{
    uint8_t recv_data = 0;
    for (uint8_t i = 0; i < 8; i++)
    {
        // 1. 输出MOSI数据位（高位先行）
        if (data & 0x80)
        {
            MOSI_HIGH();
        }
        else
        {
            MOSI_LOW();
        }
        data <<= 1;
        Delay_NOP(10); // 时序延时，保证通信稳定
        
        // 2. SCK上升沿，从机采样MOSI数据
        SCK_HIGH();
        Delay_NOP(10);
        
        // 3. 读取MISO数据位（同步接收）
        recv_data <<= 1;
        if (MISO_READ())
        {
            recv_data |= 0x01;
        }
        
        // 4. SCK下降沿，从机更新MISO数据
        SCK_LOW();
        Delay_NOP(10);
    }
    return recv_data;
}


/**
 * @brief  软件SPI接收一个字节（发送0xFF作为填充）
 * @retval 接收的字节数据
 */
uint8_t SPI_Recv_Byte(void)
{
    return SPI_Send_Byte(0xFF);
}

/************************** CRC32 校验函数 **************************/
/**
 * @brief  计算数据块的CRC32校验值
 * @param  p_data: 待校验数据指针
 * @param  length: 待校验数据长度
 * @retval 计算得到的CRC32校验值
 */
uint32_t crc32_calculate(const uint8_t *p_data, uint32_t length)
{
    uint32_t crc = 0xFFFFFFFF;
    while (length--)
    {
        crc ^= *p_data++;
        for (uint8_t i = 0; i < 8; i++)
        {
            crc = (crc >> 1) ^ ((crc & 0x01) ? 0xEDB88320 : 0x00000000);
        }
    }
    return crc ^ 0xFFFFFFFF;
}

/************************** W25Q128 底层操作函数（对接HC342F460 SPI） **************************/
/**
 * @brief  W25Q128 写使能
 */
void W25Q128_WriteEnable(void)
{
    NSS_LOW();                // 选中W25Q128
    SPI_Send_Byte(W25Q128_CMD_WRITE_ENABLE); // 发送写使能指令
    NSS_HIGH();               // 取消选中
    Delay_NOP(20);            // 短暂延时
}

/**
 * @brief  等待W25Q128操作完成（忙检测）
 */
void W25Q128_WaitBusy(void)
{
    if(!g_flash_ready) return;
    uint8_t status = 0x01;
    uint32_t timeout = 0xFFFF;  // 超时保护，避免死等
    NSS_LOW();
    SPI_Send_Byte(W25Q128_CMD_READ_STATUS);
    while ((status & 0x01) && (timeout--))
    {
        status = SPI_Recv_Byte();
        Delay_NOP(5);
    }
    NSS_HIGH();
}

/**
 * @brief  W25Q128 4KB扇区擦除
 * @param  sector_addr: 扇区起始地址
 */
void W25Q128_SectorErase(uint32_t sector_addr)
{
    W25Q128_WriteEnable();    // 先写使能
    W25Q128_WaitBusy();       // 等待前序操作完成
    
    NSS_LOW();
    // 发送扇区擦除指令
    SPI_Send_Byte(W25Q128_CMD_SECTOR_ERASE);
    // 发送3字节地址（高位先行）
    SPI_Send_Byte((sector_addr >> 16) & 0xFF);
    SPI_Send_Byte((sector_addr >> 8) & 0xFF);
    SPI_Send_Byte(sector_addr & 0xFF);
    NSS_HIGH();
    
    W25Q128_WaitBusy();       // 等待擦除完成（擦除耗时较长）
}

/**
 * @brief  W25Q128 读取数据
 * @param  read_addr: 读取起始地址
 * @param  p_buffer: 数据接收缓冲区
 * @param  length: 读取数据长度
 */
void W25Q128_ReadData(uint32_t read_addr, uint8_t *p_buffer, uint32_t length)
{
    NSS_LOW();
    // 发送读数据指令
    SPI_Send_Byte(W25Q128_CMD_READ);
    // 发送3字节地址
    SPI_Send_Byte((read_addr >> 16) & 0xFF);
    SPI_Send_Byte((read_addr >> 8) & 0xFF);
    SPI_Send_Byte(read_addr & 0xFF);
    // 连续读取数据
    for (uint32_t i = 0; i < length; i++)
    {
        p_buffer[i] = SPI_Recv_Byte();
    }
    NSS_HIGH();
}

/**
 * @brief  W25Q128 页编程（最大256字节）
 * @param  write_addr: 写入起始地址
 * @param  p_buffer: 待写入数据缓冲区
 * @param  length: 写入数据长度（≤256）
 */
void W25Q128_PageProgram(uint32_t write_addr, const uint8_t *p_buffer, uint32_t length)
{
    if (length > 256) length = 256; // 限制最大写入长度
    
    W25Q128_WriteEnable();
    W25Q128_WaitBusy();
    
    NSS_LOW();
    // 发送页编程指令
    SPI_Send_Byte(W25Q128_CMD_PAGE_PROG);
    // 发送3字节地址
    SPI_Send_Byte((write_addr >> 16) & 0xFF);
    SPI_Send_Byte((write_addr >> 8) & 0xFF);
    SPI_Send_Byte(write_addr & 0xFF);
    // 连续发送数据
    for (uint32_t i = 0; i < length; i++)
    {
        SPI_Send_Byte(p_buffer[i]);
    }
    NSS_HIGH();
    
    W25Q128_WaitBusy();
}

/************************** 批量读写封装函数 **************************/
/**
 * @brief  W25Q128 批量写入数据（自动分页，支持任意长度）
 * @param  write_addr: 写入起始地址
 * @param  p_buffer: 待写入数据缓冲区
 * @param  length: 写入数据长度
 */
void W25Q128_BulkWrite(uint32_t write_addr, const uint8_t *p_buffer, uint32_t length)
{
    uint32_t page_remain = 256 - (write_addr % 256); // 当前页剩余可写入字节
    uint32_t write_len = 0;
    
    while (length > 0)
    {
        write_len = (length > page_remain) ? page_remain : length;
        W25Q128_PageProgram(write_addr, p_buffer, write_len);
        
        length -= write_len;
        write_addr += write_len;
        p_buffer += write_len;
        page_remain = 256; // 后续页剩余字节数均为256
    }
}

/**
 * @brief  W25Q128 批量读取数据（支持任意长度）
 * @param  read_addr: 读取起始地址
 * @param  p_buffer: 数据接收缓冲区
 * @param  length: 读取数据长度
 */
void W25Q128_BulkRead(uint32_t read_addr, uint8_t *p_buffer, uint32_t length)
{
	if(!g_flash_ready) return;
    W25Q128_ReadData(read_addr, p_buffer, length);
}

/************************** 核心业务功能实现 **************************/
/**
 * @brief  校验指定数据区的有效性
 * @param  data_area_addr: 数据区起始地址（正常区/备份区）
 * @retval true: 数据有效，false: 数据异常
 */
bool DataArea_CheckValid(uint32_t data_area_addr)
{
    uint8_t data_buf[DATA_BLOCK_SIZE] = {0};
    uint32_t stored_crc = 0;
    uint32_t calc_crc = 0;
    
    // 1. 读取指定数据区数据
    W25Q128_BulkRead(data_area_addr, data_buf, DATA_BLOCK_SIZE);
    
    // 2. 读取存储的CRC32校验值
    W25Q128_BulkRead(CRC32_STORE_ADDR, (uint8_t*)&stored_crc, sizeof(uint32_t));
    
    // 3. 计算并对比CRC32
    calc_crc = crc32_calculate(data_buf, DATA_BLOCK_SIZE);
    return (calc_crc == stored_crc);
}

// 新增：读取W25Q128 JEDEC ID，检测芯片是否正常响应
bool W25Q128_Check_Exist(void)
{
    uint8_t jedec_id[3] = {0};
    NSS_LOW();
    SPI_Send_Byte(GET_IDENTIFICATION);
    jedec_id[0] = SPI_Recv_Byte();  // 厂商ID: 0xEF
    jedec_id[1] = SPI_Recv_Byte();  // 类型ID: 0x40
    jedec_id[2] = SPI_Recv_Byte();  // 容量ID: 0x18（W25Q128）
    NSS_HIGH();

    // 验证ID是否匹配W25Q128
    if(jedec_id[0] == 0xEF && jedec_id[1] == 0x40 && jedec_id[2] == 0x18)
    {
        return true;
    }
    return false;
}

/**
 * @brief  开机初始化：数据区检测、故障修复、双区同步（HC342F460专用）
 * @retval 0: 初始化成功，1: 正常区/备份区均异常（已初始化默认数据）
 */
int System_PowerOn_Storage_Init(void)
{
    uint8_t normal_data[DATA_BLOCK_SIZE] = {0};
    uint8_t backup_data[DATA_BLOCK_SIZE] = {0};
    uint32_t data_crc = 0;
	int nReturn = 1;
	char cResetFlag = 0;
	int retry = 0;
    int init_result = 1;
	char cReadBackupFlag = 0;

	pst_W25Q128SystemPara = GetSystemPara();
    
	while(retry < INIT_RETRY_COUNT)
    {
		// 步骤1：校验正常数据区
		bool normal_area_valid = DataArea_CheckValid(NORMAL_DATA_AREA_ADDR);
		
		if (normal_area_valid)
		{
			// 正常区有效，同步到备份区
			W25Q128_BulkRead(NORMAL_DATA_AREA_ADDR, normal_data, DATA_BLOCK_SIZE);
			if((normal_data[0] == 'M') && (normal_data[1] == 'Q'))
			{
				W25Q128_SectorErase(BACKUP_DATA_AREA_ADDR); // 擦除备份区
				W25Q128_BulkWrite(BACKUP_DATA_AREA_ADDR, normal_data, DATA_BLOCK_SIZE); // 写入备份区
				
				memcpy(g_data_buffer, normal_data, DATA_BLOCK_SIZE); // 更新全局缓存
				cReadBackupFlag = 0;
				nReturn = 0;//return 0;
				init_result = 0;
				break;
			}
			else
			{
				cReadBackupFlag = 1;
			}
			//memcpy(&pst_W25Q128SystemPara->DevicePara.cDeviceID[0], normal_data, sizeof(SysDeviceParaSt));
			//bool backup_area_valid = DataArea_CheckValid(BACKUP_DATA_AREA_ADDR);
			//W25Q128_BulkRead(BACKUP_DATA_AREA_ADDR, normal_data, DATA_BLOCK_SIZE);
			//memset(&pst_W25Q128SystemPara->DevicePara.cDeviceID[0], 0, sizeof(SysDeviceParaSt));
			//memcpy(&pst_W25Q128SystemPara->DevicePara.cDeviceID[0], normal_data, sizeof(SysDeviceParaSt));
			
			
		}
		else
		{
			cReadBackupFlag = 1;
		}
		if(cReadBackupFlag == 1)
		{
			// 正常区异常，校验备份区
			bool backup_area_valid = DataArea_CheckValid(BACKUP_DATA_AREA_ADDR);
			
			if (backup_area_valid)
			{
				// 备份区有效，覆盖修复正常区
				W25Q128_BulkRead(BACKUP_DATA_AREA_ADDR, backup_data, DATA_BLOCK_SIZE);
				if((backup_data[0] == 'M') && (backup_data[1] == 'Q'))
				{
					W25Q128_SectorErase(NORMAL_DATA_AREA_ADDR); // 擦除正常区
					W25Q128_BulkWrite(NORMAL_DATA_AREA_ADDR, backup_data, DATA_BLOCK_SIZE); // 修复正常区
					
					memcpy(g_data_buffer, backup_data, DATA_BLOCK_SIZE); // 更新全局缓存
					nReturn = 0;//return 0;
					init_result = 0;
					break;
				}
			}
		}
		retry++;
        Ddl_Delay1ms(1000);  // 重试前延时
	}
        // 多次重试失败，初始化默认参数
    if(init_result != 0)
    {
		// 双区均异常，初始化默认数据
		// memset(g_data_buffer, 0x00, DATA_BLOCK_SIZE);
		memcpy(&pst_W25Q128SystemPara->DevicePara.cDeviceID[0], &gs_DeviceDefaultPara, sizeof(gs_DeviceDefaultPara));
		memcpy(g_data_buffer, &gs_DeviceDefaultPara, DATA_BLOCK_SIZE);

		data_crc = crc32_calculate(g_data_buffer, DATA_BLOCK_SIZE);
		
		// 擦除双区并写入默认数据
		W25Q128_SectorErase(NORMAL_DATA_AREA_ADDR);
		W25Q128_SectorErase(BACKUP_DATA_AREA_ADDR);
		W25Q128_BulkWrite(NORMAL_DATA_AREA_ADDR, g_data_buffer, DATA_BLOCK_SIZE);
		W25Q128_BulkWrite(BACKUP_DATA_AREA_ADDR, g_data_buffer, DATA_BLOCK_SIZE);
		
		// 存储CRC32校验值
		W25Q128_SectorErase(CRC32_STORE_ADDR);
		W25Q128_BulkWrite(CRC32_STORE_ADDR, (uint8_t*)&data_crc, sizeof(uint32_t));
		
		nReturn = 1;//return 1;
    }
    
	memcpy(&pst_W25Q128SystemPara->DevicePara.cDeviceID[0], g_data_buffer, sizeof(SysDeviceParaSt));
	if(pst_W25Q128SystemPara->DevicePara.sEEP_Version != EEP_VERSION)
	{
		//记录备份数据
		//W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceID[0],SYSTEM_BACKUP_PARA_ADDR,sizeof(gs_DeviceDefaultPara));
		//采用默认参数
		memcpy(&pst_W25Q128SystemPara->DevicePara.cDeviceID[0], &gs_DeviceDefaultPara, sizeof(gs_DeviceDefaultPara));
		memcpy(g_data_buffer, &gs_DeviceDefaultPara, DATA_BLOCK_SIZE);
		cResetFlag = 1;
	}
	if( strcmp( (char*)&pst_W25Q128SystemPara->DevicePara.cDeviceSWVersion[0], (char *)&gs_DeviceDefaultPara.cDeviceSWVersion[0]) )
	{
		memcpy(&pst_W25Q128SystemPara->DevicePara.cDeviceSWVersion[0],&gs_DeviceDefaultPara.cDeviceSWVersion[0],10);
		memcpy(g_data_buffer, &pst_W25Q128SystemPara->DevicePara.cDeviceID[0], sizeof(SysDeviceParaSt));
		cResetFlag = 1;
	}
	if(cResetFlag)
	{
		data_crc = crc32_calculate(g_data_buffer, DATA_BLOCK_SIZE);
		// 擦除双区并写入默认数据
		W25Q128_SectorErase(NORMAL_DATA_AREA_ADDR);
		W25Q128_SectorErase(BACKUP_DATA_AREA_ADDR);
		W25Q128_BulkWrite(NORMAL_DATA_AREA_ADDR, g_data_buffer, DATA_BLOCK_SIZE);
		W25Q128_BulkWrite(BACKUP_DATA_AREA_ADDR, g_data_buffer, DATA_BLOCK_SIZE);
		
		// 存储CRC32校验值
		W25Q128_SectorErase(CRC32_STORE_ADDR);
		W25Q128_BulkWrite(CRC32_STORE_ADDR, (uint8_t*)&data_crc, sizeof(uint32_t));
		nReturn = 2;
		#if 0
		Ddl_Delay1ms(5000);
		bool normal_area_valid = DataArea_CheckValid(NORMAL_DATA_AREA_ADDR);
		
		if (normal_area_valid)
		{
			// 正常区有效，同步到备份区
			W25Q128_BulkRead(NORMAL_DATA_AREA_ADDR, normal_data, DATA_BLOCK_SIZE);

			memcpy(&pst_W25Q128SystemPara->DevicePara.cDeviceID[0], normal_data, sizeof(SysDeviceParaSt));
			bool backup_area_valid = DataArea_CheckValid(BACKUP_DATA_AREA_ADDR);
			W25Q128_BulkRead(BACKUP_DATA_AREA_ADDR, normal_data, DATA_BLOCK_SIZE);
			memset(&pst_W25Q128SystemPara->DevicePara.cDeviceID[0], 0, sizeof(SysDeviceParaSt));
			memcpy(&pst_W25Q128SystemPara->DevicePara.cDeviceID[0], normal_data, sizeof(SysDeviceParaSt));
		}
		#endif
	}
	return nReturn;
}

/**
 * @brief  数据写入：同时写入正常区和备份区（HC342F460专用）
 * @param  p_new_data: 待写入新数据
 * @param  length: 写入长度（≤DATA_BLOCK_SIZE）
 * @retval true: 写入成功，false: 长度超限
 */
bool Data_DoubleArea_Write(const uint8_t *p_new_data, uint32_t length)
{
    if (length > DATA_BLOCK_SIZE)
    {
        return false;
    }
    
    uint32_t new_crc = 0;
    
    // 1. 更新全局数据缓存
    memcpy(g_data_buffer, p_new_data, length);
    
    // 2. 计算新CRC32值
    new_crc = crc32_calculate(g_data_buffer, DATA_BLOCK_SIZE);
    
    // 3. 写入正常区
    W25Q128_SectorErase(NORMAL_DATA_AREA_ADDR);
    W25Q128_BulkWrite(NORMAL_DATA_AREA_ADDR, g_data_buffer, DATA_BLOCK_SIZE);
    
    // 4. 写入备份区
    W25Q128_SectorErase(BACKUP_DATA_AREA_ADDR);
    W25Q128_BulkWrite(BACKUP_DATA_AREA_ADDR, g_data_buffer, DATA_BLOCK_SIZE);
    
    // 5. 更新CRC32存储
    W25Q128_SectorErase(CRC32_STORE_ADDR);
    W25Q128_BulkWrite(CRC32_STORE_ADDR, (uint8_t*)&new_crc, sizeof(uint32_t));
    
    return true;
}

/************************** 类型2：记录数据专属接口（无备份，新增核心） **************************/
/**
 * @brief  写入单个记录结构体到指定索引（无备份，直接写入0x4000起始区）
 * @param  record_index: 记录索引（0 ~ MAX_RECORD_COUNT-1）
 * @param  p_record: 待写入记录结构体指针
 * @retval true: 写入成功，false: 索引超限/指针为空/地址越界
 */
bool RecordData_Write(uint32_t record_index, const DevMeasRecordDataSt *p_record)
{
    // 1. 参数合法性校验
    if (p_record == NULL || record_index >= MAX_RECORD_COUNT)
    {
        return false;
    }
    
    // 2. 计算目标记录的物理地址与所属扇区信息
    uint32_t record_flash_addr = RECORD_DATA_START_ADDR + (record_index * RECORD_STRUCT_SIZE);
    uint32_t sector_base_addr = record_flash_addr & 0xFFFFF000;  // 所属4KB扇区起始地址（对齐）
    uint32_t offset_in_sector = record_flash_addr - sector_base_addr;  // 记录在扇区内的偏移
    uint32_t sector_record_count = W25Q128_SECTOR_SIZE / RECORD_STRUCT_SIZE;  // 单个扇区可存记录数
    
    // 3. 地址越界校验
    if (record_flash_addr >= RECORD_DATA_MAX_ADDR || 
        (offset_in_sector + RECORD_STRUCT_SIZE) > W25Q128_SECTOR_SIZE)
    {
        return false;
    }
    
    // 4. 第一步：读取所属扇区的所有原有数据到缓存（保留原有有效记录）
    memset(guc_W25Q128_Buf, 0x00, W25Q128_SECTOR_SIZE);  // 初始化缓存为Flash擦除后默认值（0xFF）
    W25Q128_BulkRead(sector_base_addr, guc_W25Q128_Buf, W25Q128_SECTOR_SIZE);
    
    // 5. 第二步：在扇区缓存中更新目标记录（仅修改当前索引，其他记录保持不变）
    memcpy(&guc_W25Q128_Buf[offset_in_sector], p_record, RECORD_STRUCT_SIZE);
    
    // 6. 第三步：擦除所属扇区（Flash写入前必须擦除）
    W25Q128_SectorErase(sector_base_addr);
    
    // 7. 第四步：将更新后的完整扇区缓存写回Flash（原有记录+新记录一并保存）
    W25Q128_BulkWrite(sector_base_addr, guc_W25Q128_Buf, W25Q128_SECTOR_SIZE);
    
    return true;
}

/**
 * @brief  从指定索引读取单个记录结构体（无备份，直接读取0x4000起始区）
 * @param  record_index: 记录索引（0 ~ MAX_RECORD_COUNT-1）
 * @param  p_record: 记录结构体接收缓冲区指针
 * @retval true: 读取成功，false: 索引超限/指针为空/地址越界
 */
bool RecordData_Read(uint32_t record_index, DevMeasRecordDataSt *p_record)
{
    // 1. 参数合法性校验
    if (p_record == NULL || record_index >= MAX_RECORD_COUNT)
    {
        return false;
    }
    
    // 2. 计算记录在Flash中的物理地址
    uint32_t record_flash_addr = RECORD_DATA_START_ADDR + (record_index * RECORD_STRUCT_SIZE);
    if (record_flash_addr >= RECORD_DATA_MAX_ADDR)
    {
        return false; // 地址越界
    }
    
    // 3. 直接从Flash读取记录数据（无需备份区，直接读取目标地址）
    W25Q128_BulkRead(record_flash_addr, (uint8_t*)p_record, RECORD_STRUCT_SIZE);
    
    // 4. 校验记录有效标志
    //if (p_record->record_valid != 0x01)
    //{
    //    memset(p_record, 0x00, RECORD_STRUCT_SIZE);
    //    return false;
    //}
    
    return true;
}


#else

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 *******************************************************************************
 ** \brief SPI flash write byte function
 **
 ** \param [in] u8Data                      SPI write data to flash
 **
 ** \retval uint8_t                         SPI receive data from flash
 **
 ******************************************************************************/
uint8_t W25Q128_SendRcvByte(uint8_t TxData)
{
	uint8_t Rxdata = 0;
	#ifdef SPI_HART
	

	/* Wait tx buffer empty */
    while (Reset == SPI_GetFlag(SPI_W25Q128_UNIT, SpiFlagSendBufferEmpty))
    {
    }
    /* Send data */
    SPI_SendData8(SPI_W25Q128_UNIT, TxData);
    /* Wait rx buffer full */
    while (Reset == SPI_GetFlag(SPI_W25Q128_UNIT, SpiFlagReceiveBufferFull))
    {
    }
    /* Receive data */
    Rxdata = SPI_ReceiveData8(SPI_W25Q128_UNIT);
	return Rxdata;
	#else
	uint8_t i=0;
	for(i=0;i<8;i++)
	{
		SPI_W25_SCK_L;					
		if(TxData & 0x80)
		{
			SPI_W25_SI_H; 
		} 
		else 
		{
			SPI_W25_SI_L;
		}
		TxData<<=1;		
		Rxdata<<=1;
		SPI_W25_SCK_H;  //时钟平时为低。 在上升沿送数据
		if(SPI_W25_SO)
		{
			Rxdata |= 0x01;
		} 
	}
	SPI_W25_SCK_L;
	
	return Rxdata;
	#endif
}
 
 /*
  * @brief 读取DEVICE_ID
  * @return 返回ID号
  */
uint16_t W25Q128_Get_DeviceId(void)
{
	uint16_t dev_id = 0;
	 
	SPI_FLASH_NSS_ENABLE;
	 
	W25Q128_SendRcvByte(GET_DEVICE_ID);
	W25Q128_SendRcvByte(0);
	W25Q128_SendRcvByte(0);
	W25Q128_SendRcvByte(0);
  
	 
	dev_id = W25Q128_SendRcvByte(DUMMY_BYTE) << 8;
   	dev_id |= W25Q128_SendRcvByte(DUMMY_BYTE);
  
	SPI_FLASH_NSS_DISENABLE;
		 
	return dev_id;
}
  
 /*
  * @brief 读取Identifica
  * @param data_out 输出ID号
  */
void W25Q128_Get_Identifica(uint8_t *data_out)
{
	SPI_FLASH_NSS_ENABLE;
  
	W25Q128_SendRcvByte(GET_IDENTIFICATION);
	data_out [0] = W25Q128_SendRcvByte(0);
	data_out [1] = W25Q128_SendRcvByte(0);
	data_out [2] = W25Q128_SendRcvByte(0);
	 
	SPI_FLASH_NSS_DISENABLE;
}
  
 /*
  * @brief 读取数据
  * @param dest_addr 目标地址
  * @param data 读取的数据存放的数组
  * @param len 将要读取的数据长度 
  */
void W25Q128_Get_ReadDataBytes(uint32_t dest_addr, uint8_t *data, uint32_t len)
{	
	SPI_FLASH_NSS_ENABLE;
	 
	W25Q128_SendRcvByte(CMD_READ_DATA_BYTES);
	W25Q128_SendRcvByte((uint8_t)(dest_addr >> 16));
	W25Q128_SendRcvByte((uint8_t)(dest_addr >> 8));
	W25Q128_SendRcvByte((uint8_t)(dest_addr));
	 
	while(len--)
	{	
		*data = W25Q128_SendRcvByte(DUMMY_BYTE);
		data++;
	}
	 
	SPI_FLASH_NSS_DISENABLE;
}
  
//函数功能:发送"写使能命令"
void W25Q128_Spi_flash_write_enable(void)
{
	SPI_FLASH_NSS_ENABLE;
	W25Q128_SendRcvByte(CMD_WRITE_ENABLE); //发送"写使能命令"
	SPI_FLASH_NSS_DISENABLE;
}
 
//函数功能:等待空闲
void W25Q128_Spi_flash_wait_for_write_end(void)
{
	uint8_t flash_status = 0;
	do
	{
		SPI_FLASH_NSS_ENABLE;
		W25Q128_SendRcvByte(CMD_READ_STA_REG);//发送"读状态寄存器命令"
  
		flash_status = W25Q128_SendRcvByte(DUMMY_BYTE);
		//发送DUMMY_BYTE数据为的是读取状态寄存器的值
		SPI_FLASH_NSS_DISENABLE;
	}while( (flash_status & WIP_FLAG) == 0x01 );
	//WIP位置1,表示芯片正处于编程/擦除/写状态
  
	
}
 
 
//函数功能:擦除扇区,其首地址为sector_addr
void W25Q128_Spi_flash_sector_erase(uint32_t sector_addr)
{
	union W25Q128_UINT32_DATA_TYPE addr;
  
	addr.Uint32_Data = sector_addr * 4096;
  
	W25Q128_Spi_flash_write_enable();//发送"写使能命令"
	W25Q128_Spi_flash_wait_for_write_end();//等待空闲
	SPI_FLASH_NSS_ENABLE;
	W25Q128_SendRcvByte(CMD_SECTOR_ERASE); //发送扇区擦除命令
	W25Q128_SendRcvByte( addr.b[2] );     //发送扇区地址的bit16~bit23
	W25Q128_SendRcvByte( addr.b[1] );     //发送扇区地址的bit8~bit15
	W25Q128_SendRcvByte( addr.b[0] );     //发送扇区地址的bit0~bit7
	SPI_FLASH_NSS_DISENABLE;
 
	W25Q128_Spi_flash_wait_for_write_end();//等待空闲
}
 

//函数功能:将pbuffer[]中的num_byte_to_write个字节型数据写入首地址为write_addr的Flash空间
static void W25Q128_Spi_flash_page_write(uint8_t* pbuffer, uint32_t write_addr, uint16_t num_byte_to_write)
{
	union W25Q128_UINT32_DATA_TYPE addr;
  
	addr.Uint32_Data=write_addr;
  
	W25Q128_Spi_flash_write_enable();//发送"写使能命令"
  
	SPI_FLASH_NSS_ENABLE;
  
	W25Q128_SendRcvByte(CMD_PAGE_PROGRAM);//发送"页编程命令"
	W25Q128_SendRcvByte( addr.b[2] );   //发送页地址的bit16~bit23
	W25Q128_SendRcvByte( addr.b[1] );   //发送页地址的bit8~bit15
	W25Q128_SendRcvByte( addr.b[0] );   //发送页地址的bit0~bit7
	while(num_byte_to_write--)
	{
		W25Q128_SendRcvByte(*pbuffer);
		pbuffer++;
	}
  
	SPI_FLASH_NSS_DISENABLE;
  
	W25Q128_Spi_flash_wait_for_write_end();//等待空闲
}
 
//无检验写SPI FLASH
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECKOK
void W25Q128_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{                                           
	uint16_t pageremain;        
	pageremain = 256 - (WriteAddr%256); //单页剩余的字节数                          
	if(NumByteToWrite <= pageremain)
	{
		pageremain=NumByteToWrite;//不大于256个字节
	}
	while(1)
	{           
		W25Q128_Spi_flash_page_write(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)
		{
			break;//写入结束了
		}
		else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;        

			NumByteToWrite-=pageremain;                          //减去已经写入了的字节数
			if(NumByteToWrite>256)
			{
				pageremain=256;//一次可以写入256个字节
			}
			else
			{
				pageremain=NumByteToWrite;        //不够256个字节了
			}
		}
	}
} 

/*
	* @brief 写数据
  	* @param write_addr 目标地址
  	* @param pbuffer 将要写入数据
  	* @param len 将要写入数据长度 
*/
//写SPIFLASH  
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)                                                    
//NumByteToWrite:要写入的字节数(最大65535)   

void W25Q128_Spi_flash_buffer_write(uint8_t* pbuffer, uint32_t write_addr, uint16_t NumByteToWrite)
{
	#if 0
	uint16_t num_of_page = 0, num_of_single = 0, addr = 0, count = 0, temp = 0;
  
	addr          = write_addr % SPI_FLASH_PAGE_SIZE;
	count         = SPI_FLASH_PAGE_SIZE - addr; //计算当前页剩余多少个字节空间
	num_of_page   = len / SPI_FLASH_PAGE_SIZE;//计算需要写多少页
	num_of_single = len % SPI_FLASH_PAGE_SIZE;//计算不满一页的字节数量
  
	if(0 == addr)//位于页边界
	{
		if(0 == num_of_page)//所写字节数量不满一页,num_byte_to_write < SPI_FLASH_PAGE_SIZE
		{
			W25Q128_Spi_flash_page_write(pbuffer,write_addr, len);
		} 
		else//所写字节数量超过一页,num_byte_to_write > SPI_FLASH_PAGE_SIZE
		{
			while(num_of_page--)
			{
				W25Q128_Spi_flash_page_write(pbuffer,write_addr,SPI_FLASH_PAGE_SIZE);
				write_addr += SPI_FLASH_PAGE_SIZE;
				pbuffer += SPI_FLASH_PAGE_SIZE;
			}
			W25Q128_Spi_flash_page_write(pbuffer,write_addr,num_of_single);
		}
	}
	else
	{
		if(0 == num_of_page)//所写字节数量不满一页
		{
			if(num_of_single > count)//超过当前页
			{
				temp = num_of_single - count;//计算跨页的字节数量
				W25Q128_Spi_flash_page_write(pbuffer,write_addr,count);
				write_addr += count;//修改Flash地址
				pbuffer += count;   //修改指针
				W25Q128_Spi_flash_page_write(pbuffer,write_addr,temp);
			}
			else//没有超过当前页
			{
				W25Q128_Spi_flash_page_write(pbuffer,write_addr,len);
			}
		}
		else//所写字节数量超过一页
		{
			len -= count;//计算写当前页后的剩余字节总数
			num_of_page = len / SPI_FLASH_PAGE_SIZE;  //剩余字节总数需要多少页
			num_of_single = len % SPI_FLASH_PAGE_SIZE;//剩余字节总数写完整页后的剩余字节数量
  
			W25Q128_Spi_flash_page_write(pbuffer,write_addr, count);//向当前页写入count字节,凑成1整页
			write_addr += count;//修改Flash地址
			pbuffer += count;   //修改指针
  
			while(num_of_page--)
			{
				W25Q128_Spi_flash_page_write(pbuffer,write_addr,SPI_FLASH_PAGE_SIZE);
				write_addr += SPI_FLASH_PAGE_SIZE;//修改Flash地址
				pbuffer += SPI_FLASH_PAGE_SIZE;   //修改指针
			}
  
			if(0 != num_of_single)//最后写剩余的字节
			{
				W25Q128_Spi_flash_page_write(pbuffer,write_addr,num_of_single);
			}
		}
	}
	#else
	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;           
	uint16_t i;    
	uint8_t j = 0;
	uint8_t * W25QXX_BUF;   
	W25QXX_BUF=guc_W25Q128_Buf;       
	for(j=0; j<2; j++)
	{
		if(write_addr < SYSTEM_RECORD_START_ADDR)
		{
			if(j == 0)
			{
				secpos=write_addr/4096;//扇区地址  
				secoff=write_addr%4096;//在扇区内的偏移
				secremain=4096-secoff;//扇区剩余空间大小   
			}
			else
			{
				write_addr = write_addr + BACKUP_DATA_AREA_ADDR;
				secpos=write_addr/4096;//扇区地址  
				secoff=write_addr%4096;//在扇区内的偏移
				secremain=4096-secoff;//扇区剩余空间大小   
			}
		}
		else
		{
			j = 1;
			secpos=write_addr/4096;//扇区地址  
			secoff=write_addr%4096;//在扇区内的偏移
			secremain=4096-secoff;//扇区剩余空间大小   
		}
		//printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//测试用
		if(NumByteToWrite<=secremain)
		{
			secremain=NumByteToWrite;//不大于4096个字节
		}
		while(1) 
		{        
			W25Q128_Get_ReadDataBytes(secpos*4096,W25QXX_BUF,4096);//读出整个扇区的内容
			for(i=0;i<secremain;i++)//校验数据
			{
				if(W25QXX_BUF[secoff+i] != 0XFF)
				{
					break;//需要擦除      
				}
			}
			if(i<secremain)//需要擦除
			{
				W25Q128_Spi_flash_sector_erase(secpos);//擦除这个扇区
				//W25Q128_Get_ReadDataBytes(secpos*4096,W25QXX_BUF,4096);//读出整个扇区的内容
				for(i=0;i<secremain;i++)     //复制
				{
					W25QXX_BUF[i+secoff]=pbuffer[i];           
				}
				W25Q128_Write_NoCheck(W25QXX_BUF,secpos*4096,4096);//写入整个扇区  

			}
			else
			{
				W25Q128_Write_NoCheck(pbuffer,write_addr,secremain);//写已经擦除了的,直接写入扇区剩余区间.      
			}                                   
			if(NumByteToWrite==secremain)
			{
				break;//写入结束了
			}
			else//写入未结束
			{
				secpos++;//扇区地址增1
				secoff=0;//偏移位置为0        

				pbuffer+=secremain;  //指针偏移
				write_addr+=secremain;//写地址偏移      
				NumByteToWrite-=secremain;                                    //字节数递减
				if(NumByteToWrite>4096)
				{
					secremain=4096;  //下一个扇区还是写不完
				}
				else
				{
					secremain=NumByteToWrite;                   //下一个扇区可以写完了
				}
			}        
			
		};       
	}
   	
	//W25Q128_Get_ReadDataBytes(secpos*4096,W25QXX_BUF,4096);//读出整个扇区的内容
	#endif
} 

#endif // NEW_W25Q128_DRIVER

//保存设备参数到存储器中
//输入参数：eCMD:保存参数命令号;  cDataArr:保存数据指针
//返回参数：1->保存成功；0->保存失败
unsigned char func_Save_Device_Parameter(en_SaveParaCMD eCMD, unsigned char *cDataArr)
{
	unsigned char ucResult = 1;
	unsigned char ucTmpData = 0;
	unsigned short usTmpData = 0;
	int nTmpData = 0;
	float *fTmpData;
	unsigned char i,j;

	switch (eCMD)
	{
	case DEV_ID:
		ucTmpData = strlen((char*)cDataArr);
		if(ucTmpData > 16)
		{
			return 0;
		}
		else
		{
			strcpy(&pst_W25Q128SystemPara->DevicePara.cDeviceID[0],(char*)cDataArr);
			pst_W25Q128SystemPara->DevicePara.cDeviceRegisterFlag = 0;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceID[0],SYSTEM_PARA_ADDR,16);
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceRegisterFlag,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cDeviceRegisterFlag-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_IDEN_FLAG:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 1)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cDeviceIdenFlag = ucTmpData;
			pst_W25Q128SystemPara->DeviceRunPara.c4GInitFlag = 0;
			#ifndef NEW_W25Q128_DRIVER
            W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceIdenFlag,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cDeviceIdenFlag-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			if(pst_W25Q128SystemPara->DevicePara.cDeviceIdenFlag == 1)
			{
				pst_W25Q128SystemPara->DevicePara.cDeviceRegisterFlag = 0;
				W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceRegisterFlag,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cDeviceRegisterFlag-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			}
			#else
			if(pst_W25Q128SystemPara->DevicePara.cDeviceIdenFlag == 1)
			{
				pst_W25Q128SystemPara->DevicePara.cDeviceRegisterFlag = 0;
			}
			#endif
		}
		break;
	case DEV_REG_FLAG:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 1)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cDeviceRegisterFlag = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceRegisterFlag,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cDeviceRegisterFlag-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_PD_DATE:
		ucTmpData = strlen((char*)cDataArr);
		if(ucTmpData > 9)
		{
			return 0;
		}
		else
		{
			strcpy(&pst_W25Q128SystemPara->DevicePara.cDevicePDDate[0],(char*)cDataArr);
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDevicePDDate[0],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cDevicePDDate[0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),9);
			#endif
		}
		break;
	case DEV_SAMPLE_GAP:
		nTmpData = *((int *)cDataArr);  
		if ((nTmpData < 1) || (nTmpData > 600))
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.nDeviceSampleGapCnt = nTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.nDeviceSampleGapCnt,SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.nDeviceSampleGapCnt-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_RECORD_GAP:
		nTmpData = *((int *)cDataArr);  
		if ((nTmpData < 1) || (nTmpData > 600))
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.nDeviceSaveRecordCnt = nTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.nDeviceSaveRecordCnt,SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.nDeviceSaveRecordCnt-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_UPLOAD_GAP:
		nTmpData = *((int *)cDataArr);  
		if ((nTmpData < 1) || (nTmpData > 600))
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.nDeviceUploadCnt = nTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.nDeviceUploadCnt,SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.nDeviceUploadCnt-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_HIS_RECORD:
		nTmpData = *((int *)cDataArr);  
		//if (nTmpData > 0x270F) 
		//{ 
		//	return(0);
		//}
		//else
		{
			pst_W25Q128SystemPara->DevicePara.nDeviceRecordCnt = nTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.nDeviceRecordCnt,SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.nDeviceRecordCnt-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_SENSOR_ENABLE_1:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 1)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cMeasSensorEnableFlag[0] = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cMeasSensorEnableFlag[0],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cMeasSensorEnableFlag[0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_SENSOR_ENABLE_2:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 1)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cMeasSensorEnableFlag[1] = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cMeasSensorEnableFlag[1],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cMeasSensorEnableFlag[1]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_SENSOR_CNT_1:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 10)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cMeasSensorCount[0] = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cMeasSensorCount[0],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cMeasSensorCount[0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
			pst_W25Q128SystemPara->DeviceRunPara.cTotalSensorCnt = pst_W25Q128SystemPara->DevicePara.cMeasSensorCount[0] + pst_W25Q128SystemPara->DevicePara.cMeasSensorCount[1];
		}
		break;
	case DEV_SENSOR_CNT_2:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 10)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cMeasSensorCount[1] = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cMeasSensorCount[1],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cMeasSensorCount[1]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
			pst_W25Q128SystemPara->DeviceRunPara.cTotalSensorCnt = pst_W25Q128SystemPara->DevicePara.cMeasSensorCount[0] + pst_W25Q128SystemPara->DevicePara.cMeasSensorCount[1];
		}
		break;
	case DEV_CHN1_SENSOR1_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][0] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][0],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CHN1_SENSOR2_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][1] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][1],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][1]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CHN1_SENSOR3_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][2] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][2],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][2]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CHN1_SENSOR4_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][3] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][3],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][3]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CHN1_SENSOR5_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][4] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][4],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][4]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CHN1_SENSOR6_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][5] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][5],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][5]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CHN1_SENSOR7_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][6] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][6],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][6]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CHN1_SENSOR8_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][7] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][7],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][7]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CHN1_SENSOR9_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][8] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][8],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][8]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CHN1_SENSOR10_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][9] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][9],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][9]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CHN2_SENSOR1_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][0] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][0],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CHN2_SENSOR2_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][1] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][1],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][1]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CHN2_SENSOR3_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][2] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][2],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][2]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CHN2_SENSOR4_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][3] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][3],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][3]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CHN2_SENSOR5_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][4] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][4],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][4]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CHN2_SENSOR6_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][5] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][5],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][5]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CHN2_SENSOR7_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][6] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][6],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][6]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CHN2_SENSOR8_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][7] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][7],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][7]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CHN2_SENSOR9_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][8] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][8],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][8]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CHN2_SENSOR10_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_NULL))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][9] = (EMeasSensorType)ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][9],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][9]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_TOTAL_VOLUME:
		//memcpy(&pst_W25Q128SystemPara->DevicePara.fTotal_Volume, cDataArr, sizeof(float));
		fTmpData = ((float*)cDataArr);
		pst_W25Q128SystemPara->DevicePara.fTotal_Volume = *fTmpData;
		#ifndef NEW_W25Q128_DRIVER
		#pragma diag_suppress=Pa039
		W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.fTotal_Volume,SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.fTotal_Volume-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(float));
		#pragma diag_warning=Pa039
		#endif
		break;
	case DEV_SOFTRESET:	//设备软复位
		func_System_Soft_Reset();
		return 1;
		break;	
	case DEV_STATUS_UPLOAD_GAP:
		ucTmpData = *cDataArr;     
		if((ucTmpData < 1) || (ucTmpData > 200))
		{
			return 0;
		}
		else{
			pst_W25Q128SystemPara->DevicePara.ucUploadStatusGap = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.ucUploadStatusGap,SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.ucUploadStatusGap-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(unsigned char));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_IP_ADDRESS1:
		ucTmpData = strlen((char*)cDataArr);
		if(ucTmpData > 15)
		{
			return 0;
		}
		else
		{
			memset(&pst_W25Q128SystemPara->DevicePara.cServerIP[0][0],0,16);
			memcpy(&pst_W25Q128SystemPara->DevicePara.cServerIP[0][0],(char*)cDataArr,strlen((char*)cDataArr));
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cServerIP[0][0],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cServerIP[0][0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),16);
			#endif
		}
		break;
	case DEV_IP_ADDRESS2:
		ucTmpData = strlen((char*)cDataArr);
		if(ucTmpData > 15)
		{
			return 0;
		}
		else
		{
			memset(&pst_W25Q128SystemPara->DevicePara.cServerIP[1][0],0,16);
			memcpy(&pst_W25Q128SystemPara->DevicePara.cServerIP[1][0],(char*)cDataArr,strlen((char*)cDataArr));
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cServerIP[1][0],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cServerIP[1][0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),16);
			#endif
		}
		break;
	case DEV_IP_PORT1:
		usTmpData = *((unsigned short *)cDataArr);
		if(usTmpData > 60000)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.usServerPort[0] = usTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usServerPort[0],SYSTEM_PARA_ADDR+((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usServerPort[0]-(uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(unsigned short));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_IP_PORT2:
		usTmpData = *((unsigned short *)cDataArr);
		if(usTmpData > 60000)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.usServerPort[1] = usTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usServerPort[1],SYSTEM_PARA_ADDR+((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usServerPort[1]-(uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(unsigned short));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_DEBUG_MODEL:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 1)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DeviceRunPara.cDebugModel = ucTmpData;
			//W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDebugModel,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cDebugModel-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_INSTALL_HEIGHT:
		//memcpy(&pst_W25Q128SystemPara->DevicePara.fTotal_Volume, cDataArr, sizeof(float));
		fTmpData = ((float*)cDataArr);
		if(((double)*fTmpData < 0.2) || (((double)*fTmpData > 10.0)))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.fInit_Height = *fTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.fInit_Height,SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.fInit_Height-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(float));
			#pragma diag_warning=Pa039
			#endif
		}
		
		break;
	case DEV_SENSOR_BAUDRATE:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 5)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cSensorBaudRate = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cSensorBaudRate,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cSensorBaudRate-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_LONGPOWER_MODEL:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 1)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DeviceRunPara.cLongPowerModel = ucTmpData;
			pst_W25Q128SystemPara->DeviceRunPara.usLongPowerModelWaitCnt = 0;
			//W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDebugModel,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cDebugModel-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_PRESSURE_SENSOR_CALIBRATION:
		fTmpData = ((float*)cDataArr);
		if(((double)*fTmpData < -10.0) || (((double)*fTmpData > 10.0)))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.fPressureSensorCalibration = *fTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.fPressureSensorCalibration,SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.fPressureSensorCalibration-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(float));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_PIPE_INSTALL_HEIGHT:
		fTmpData = ((float*)cDataArr);
		if(((double)*fTmpData < 0.0) || (((double)*fTmpData > 30.0)))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.fInstall_Height = *fTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.fInstall_Height,SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.fInstall_Height-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(float));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_LEVELALARMCNTS:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 5)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cLevelAlarmCnts = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cLevelAlarmCnts,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cLevelAlarmCnts-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_LEVELALARMPER1:
		fTmpData = ((float*)cDataArr);
		*fTmpData = *fTmpData / 100;
		if(((double)*fTmpData < 0.0) || (((double)*fTmpData > 1.0)))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.fLevelAlarmPer[0] = *fTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.fLevelAlarmPer[0],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.fLevelAlarmPer[0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(float));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_LEVELALARMPER2:
		fTmpData = ((float*)cDataArr);
		*fTmpData = *fTmpData / 100;
		if(((double)*fTmpData < 0.0) || (((double)*fTmpData > 1.0)))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.fLevelAlarmPer[1] = *fTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.fLevelAlarmPer[1],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.fLevelAlarmPer[1]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(float));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_LEVELALARMPER3:
		fTmpData = ((float*)cDataArr);
		*fTmpData = *fTmpData / 100;
		if(((double)*fTmpData < 0.0) || (((double)*fTmpData > 1.0)))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.fLevelAlarmPer[2] = *fTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.fLevelAlarmPer[2],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.fLevelAlarmPer[2]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(float));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_LEVELALARMPER4:
		fTmpData = ((float*)cDataArr);
		*fTmpData = *fTmpData / 100;
		if(((double)*fTmpData < 0.0) || (((double)*fTmpData > 1.0)))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.fLevelAlarmPer[3] = *fTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.fLevelAlarmPer[3],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.fLevelAlarmPer[3]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(float));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_LEVELALARMPER5:
		fTmpData = ((float*)cDataArr);
		*fTmpData = *fTmpData / 100;
		if(((double)*fTmpData < 0.0) || (((double)*fTmpData > 1.0)))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.fLevelAlarmPer[4] = *fTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.fLevelAlarmPer[4],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.fLevelAlarmPer[4]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(float));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_LEVELALARMLEV1:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 5)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cLevelAlarmLev[0] = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cLevelAlarmLev[0],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cLevelAlarmLev[0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_LEVELALARMLEV2:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 5)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cLevelAlarmLev[1] = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cLevelAlarmLev[1],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cLevelAlarmLev[1]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_LEVELALARMLEV3:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 5)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cLevelAlarmLev[2] = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cLevelAlarmLev[2],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cLevelAlarmLev[2]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_LEVELALARMLEV4:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 5)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cLevelAlarmLev[3] = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cLevelAlarmLev[3],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cLevelAlarmLev[3]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_LEVELALARMLEV5:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 5)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cLevelAlarmLev[4] = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cLevelAlarmLev[4],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cLevelAlarmLev[4]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_ALARMSAMP1:
		usTmpData = *((unsigned short *)cDataArr);  
		if ((usTmpData < 1) || (usTmpData > 600))
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.usAlarmSamp[0] = usTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usAlarmSamp[0],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.usAlarmSamp[0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_ALARMSAMP2:
		usTmpData = *((unsigned short *)cDataArr);  
		if ((usTmpData < 1) || (usTmpData > 600))
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.usAlarmSamp[1] = usTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usAlarmSamp[1],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.usAlarmSamp[1]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_ALARMSAMP3:
		usTmpData = *((unsigned short *)cDataArr);  
		if ((usTmpData < 1) || (usTmpData > 600))
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.usAlarmSamp[2] = usTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usAlarmSamp[2],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.usAlarmSamp[2]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_ALARMSAMP4:
		usTmpData = *((unsigned short *)cDataArr);  
		if ((usTmpData < 1) || (usTmpData > 600))
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.usAlarmSamp[3] = usTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usAlarmSamp[3],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.usAlarmSamp[3]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_ALARMSAMP5:
		usTmpData = *((unsigned short *)cDataArr);  
		if ((usTmpData < 1) || (usTmpData > 600))
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.usAlarmSamp[4] = usTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usAlarmSamp[4],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.usAlarmSamp[4]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_ALARMUPLOAD1:
		usTmpData = *((unsigned short *)cDataArr);  
		if ((usTmpData < 1) || (usTmpData > 600))
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.usAlarmUpload[0] = usTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usAlarmUpload[0],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.usAlarmUpload[0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_ALARMUPLOAD2:
		usTmpData = *((unsigned short *)cDataArr);  
		if ((usTmpData < 1) || (usTmpData > 600))
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.usAlarmUpload[1] = usTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usAlarmUpload[1],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.usAlarmUpload[1]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_ALARMUPLOAD3:
		usTmpData = *((unsigned short *)cDataArr);  
		if ((usTmpData < 1) || (usTmpData > 600))
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.usAlarmUpload[2] = usTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usAlarmUpload[2],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.usAlarmUpload[2]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_ALARMUPLOAD4:
		usTmpData = *((unsigned short *)cDataArr);  
		if ((usTmpData < 1) || (usTmpData > 600))
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.usAlarmUpload[3] = usTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usAlarmUpload[3],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.usAlarmUpload[3]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_ALARMUPLOAD5:
		usTmpData = *((unsigned short *)cDataArr);  
		if ((usTmpData < 1) || (usTmpData > 600))
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.usAlarmUpload[4] = usTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usAlarmUpload[4],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.usAlarmUpload[4]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_WEATHER:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 1)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cWeatherFlag = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cWeatherFlag,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cWeatherFlag-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_SCENARIO:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 1)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cScenario = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cScenario,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cScenario-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CONALARMCNTS:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 5)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cCondAlarmCnts = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cCondAlarmCnts,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cCondAlarmCnts-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CONDALARMLEV1:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 5)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cCONDAlarmLev[0] = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cCONDAlarmLev[0],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cCONDAlarmLev[0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CONDALARMLEV2:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 5)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cCONDAlarmLev[1] = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cCONDAlarmLev[1],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cCONDAlarmLev[1]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CONDALARMLEV3:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 5)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cCONDAlarmLev[2] = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cCONDAlarmLev[2],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cCONDAlarmLev[2]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CONDALARMLEV4:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 5)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cCONDAlarmLev[3] = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cCONDAlarmLev[3],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cCONDAlarmLev[3]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CONDALARMLEV5:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 5)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cCONDAlarmLev[4] = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cCONDAlarmLev[4],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cCONDAlarmLev[4]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_CONDALARMVAL1:
		usTmpData = *((unsigned short *)cDataArr);  
		if ((usTmpData < 1) || (usTmpData > 65000))
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.usCONDAlarmValue[0] = usTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usCONDAlarmValue[0],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.usCONDAlarmValue[0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_CONDALARMVAL2:
		usTmpData = *((unsigned short *)cDataArr);  
		if ((usTmpData < 1) || (usTmpData > 65000))
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.usCONDAlarmValue[1] = usTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usCONDAlarmValue[1],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.usCONDAlarmValue[1]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_CONDALARMVAL3:
		usTmpData = *((unsigned short *)cDataArr);  
		if ((usTmpData < 1) || (usTmpData > 65000))
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.usCONDAlarmValue[2] = usTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usCONDAlarmValue[2],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.usCONDAlarmValue[2]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_CONDALARMVAL4:
		usTmpData = *((unsigned short *)cDataArr);  
		if ((usTmpData < 1) || (usTmpData > 65000))
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.usCONDAlarmValue[3] = usTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usCONDAlarmValue[3],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.usCONDAlarmValue[3]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_CONDALARMVAL5:
		usTmpData = *((unsigned short *)cDataArr);  
		if ((usTmpData < 1) || (usTmpData > 65000))
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.usCONDAlarmValue[4] = usTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usCONDAlarmValue[4],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.usCONDAlarmValue[4]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_PLAN_ENABLE:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 1)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cPlanEnableFlag = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cPlanEnableFlag,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cPlanEnableFlag-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_ALARM_ENABLE:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 1)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cAlarmEnableFlag = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cAlarmEnableFlag,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cAlarmEnableFlag-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_FLOW_ALARM_ENABLE:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 1)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cFlowAlarmEnableFlag = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cFlowAlarmEnableFlag,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cFlowAlarmEnableFlag-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_FLOW_ALARM_CNTS:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 2)
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cFlowAlarmCnts = ucTmpData;
			#ifndef NEW_W25Q128_DRIVER
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cFlowAlarmCnts,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cFlowAlarmCnts-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			#endif
		}
		break;
	case DEV_FLOW_ALARM_VAL1:
		fTmpData = ((float*)cDataArr);
		if(((double)*fTmpData < 0.0) || (((double)*fTmpData > 1000.0)))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.fFlowAlarmValue[0] = *fTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.fFlowAlarmValue[0],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.fFlowAlarmValue[0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(float));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_FLOW_ALARM_VAL2:
		fTmpData = ((float*)cDataArr);
		if(((double)*fTmpData < 0.0) || (((double)*fTmpData > 1000.0)))
		{
			return 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.fFlowAlarmValue[1] = *fTmpData;
			#ifndef NEW_W25Q128_DRIVER
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.fFlowAlarmValue[1],SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.fFlowAlarmValue[1]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(float));
			#pragma diag_warning=Pa039
			#endif
		}
		break;
	case DEV_FACTORY_RESET:
		func_Device_Parameter_Factory_Reset();
		break;
	default:
		break;
	}

	if((eCMD >= DEV_CHN1_SENSOR1_TYPE) && (eCMD <= DEV_CHN2_SENSOR10_TYPE))
	{
		//判断当前设备是否为HX压力液位计
		for(j=0; j<2; j++)
		{
			for(i=0; i<pst_W25Q128SystemPara->DevicePara.cMeasSensorCount[j]; i++)
			{
				if(pst_W25Q128SystemPara->DevicePara.eMeasSensor[j][i] == Meas_HX_Pressure_Level)
				{
					pst_W25Q128SystemPara->DeviceRunPara.cHXPressureLevelFlag = 1;
					break;
				}
			}
		}
	}
	pst_W25Q128SystemPara->DeviceRunPara.cSaveParaFlag = 1;
	return ucResult;
}

//保存设备测量数据
unsigned char func_Save_Device_MeasData(void)
{
	#ifdef NEW_W25Q128_DRIVER
	unsigned char i=0;
	unsigned char ucResult = 1;
	DevMeasRecordDataSt tMeasData;
	for(i=0; i<3; i++)
	{
		//写入记录后，读取记录，判断是否写入成功
		RecordData_Write(pst_W25Q128SystemPara->DevicePara.nDeviceRecordCnt, &gSt_DevMeasRecordData);
		RecordData_Read(pst_W25Q128SystemPara->DevicePara.nDeviceRecordCnt, &tMeasData);
		if((tMeasData.cWater_Immersion_Status == gSt_DevMeasRecordData.cWater_Immersion_Status) && (tMeasData.nAttitude_SC7A == gSt_DevMeasRecordData.nAttitude_SC7A))
		{
			ucResult = 0;
		    break;
		}
	}
	return ucResult;
	#else
	W25Q128_Spi_flash_buffer_write((uint8_t *)&gSt_DevMeasRecordData.fWaterLevel,SYSTEM_RECORD_START_ADDR+pst_W25Q128SystemPara->DevicePara.nDeviceRecordCnt*SYSTEM_RECORD_SIZE,SYSTEM_RECORD_SIZE);
	#endif
}

//读取设备测量数据
void func_Get_Device_MeasData_Record(int nRecordIndex, DevMeasRecordDataSt *pstMeasData)
{
	#ifdef NEW_W25Q128_DRIVER
	RecordData_Read(nRecordIndex, pstMeasData);
	#else
	W25Q128_Get_ReadDataBytes(SYSTEM_RECORD_START_ADDR+nRecordIndex*SYSTEM_RECORD_SIZE, (uint8_t *)pstMeasData, SYSTEM_RECORD_SIZE);
	#endif
}

 /**
 *******************************************************************************
 ** \brief  GD25Q128E init function
 **
 ** \param [in]  None
 **
 ** \retval 0: init success; 1: init failed
 **
 ******************************************************************************/
uint8_t drv_Storage_W25Q128_Init(void)
{
	stc_port_init_t stcPortInit;
	uint8_t ucDataArr[3] = {0};
	uint16_t usDeviceId = 0;

    /* configure structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);

    /* Flash NSS */
    stcPortInit.enPinMode = Pin_Mode_Out;
    PORT_Init(SPI_W25Q128_NSS_PORT, SPI_W25Q128_NSS_PIN, &stcPortInit);
	#ifndef SPI_HART
	PORT_Init(SPI_GD25_SCK_PORT, SPI_GD25_SCK_PIN, &stcPortInit);
	//PORT_Init(SPI_GD25_MOSI_PORT, SPI_GD25_MOSI_PIN, &stcPortInit);
	PORT_Init(SPI_GD25_MISO_PORT, SPI_GD25_MISO_PIN, &stcPortInit);
	stcPortInit.enPinMode = Pin_Mode_In;
	PORT_Init(SPI_GD25_MOSI_PORT, SPI_GD25_MOSI_PIN, &stcPortInit);
	#endif
    SPI_W25Q128_NSS_HIGH();
	pst_W25Q128SystemPara = GetSystemPara();
	W25Q128_Get_Identifica(ucDataArr);
	usDeviceId = W25Q128_Get_DeviceId();
	if(usDeviceId != 0xEF17)
	{
		return 1;
	}
	return 0;
}

//开机读取存储的设备参数结构体，以及对参数版本进行判断和必要的更新
void func_Device_Parameter_Init(void)
{
	//SystemPataSt stTemp;

	//读取FLASH中设备参数信息
	W25Q128_Get_ReadDataBytes(SYSTEM_PARA_ADDR,(uint8_t*)&pst_W25Q128SystemPara->DevicePara.cDeviceID[0],sizeof(SysDeviceParaSt));
    //W25Q128_Get_ReadDataBytes(SYSTEM_BACKUP_PARA_ADDR,(uint8_t*)&stTemp.DevicePara.cDeviceID[0],sizeof(SysDeviceParaSt));
	//判断版本号是否相同
	if(pst_W25Q128SystemPara->DevicePara.sEEP_Version != EEP_VERSION)
	{
		//读取备份区数据
		W25Q128_Get_ReadDataBytes(BACKUP_DATA_AREA_ADDR,(uint8_t*)&pst_W25Q128SystemPara->DevicePara.cDeviceID[0],sizeof(SysDeviceParaSt));
		if(pst_W25Q128SystemPara->DevicePara.sEEP_Version != EEP_VERSION)
		{
			//记录备份数据
			//W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceID[0],SYSTEM_BACKUP_PARA_ADDR,sizeof(gs_DeviceDefaultPara));
			//采用默认参数
			memcpy(&pst_W25Q128SystemPara->DevicePara.cDeviceID[0], &gs_DeviceDefaultPara, sizeof(gs_DeviceDefaultPara));
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceID[0],SYSTEM_PARA_ADDR,sizeof(gs_DeviceDefaultPara));
		}
	}
	if( strcmp( (char*)&pst_W25Q128SystemPara->DevicePara.cDeviceSWVersion[0], (char *)&gs_DeviceDefaultPara.cDeviceSWVersion[0]) )
	{
		memcpy(&pst_W25Q128SystemPara->DevicePara.cDeviceSWVersion[0],&gs_DeviceDefaultPara.cDeviceSWVersion[0],10);
		W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceSWVersion[0],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cDeviceSWVersion[0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),10);
	}
}

//复位设备参数为出厂设置
void func_Device_Parameter_Factory_Reset(void)
{
	char cDeviceID[17] = {0};
	char cDevicePDDate[10] = {0};
	//保存原有的设备ID和生产日期
	memcpy(&cDeviceID[0], &pst_W25Q128SystemPara->DevicePara.cDeviceID[0], 17);
	memcpy(&cDevicePDDate[0], &pst_W25Q128SystemPara->DevicePara.cDevicePDDate[0], 10);
	
	memcpy(&pst_W25Q128SystemPara->DevicePara.cDeviceID[0], &gs_DeviceDefaultPara, sizeof(gs_DeviceDefaultPara));
	
	memcpy(&pst_W25Q128SystemPara->DevicePara.cDeviceID[0], &cDeviceID[0], 17);
	memcpy(&pst_W25Q128SystemPara->DevicePara.cDevicePDDate[0], &cDevicePDDate[0], 10);
	#ifndef NEW_W25Q128_DRIVER
	W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceID[0],SYSTEM_PARA_ADDR,sizeof(gs_DeviceDefaultPara));
	#endif
	pst_W25Q128SystemPara->DeviceRunPara.cTotalSensorCnt = pst_W25Q128SystemPara->DevicePara.cMeasSensorCount[0] + pst_W25Q128SystemPara->DevicePara.cMeasSensorCount[1];
	pst_W25Q128SystemPara->DeviceRunPara.cDebugModel = 0;
	pst_W25Q128SystemPara->DeviceRunPara.cLongPowerModel = 0;
	pst_W25Q128SystemPara->DeviceRunPara.usLongPowerModelWaitCnt = 0;
	//func_System_Soft_Reset();
}

/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
