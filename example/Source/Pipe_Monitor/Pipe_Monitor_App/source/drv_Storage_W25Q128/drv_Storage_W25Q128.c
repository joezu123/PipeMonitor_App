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
 
 
#define  SFLASH_ID       						0xC84018  //Flash ID

 
#define SPI_FLASH_NSS_ENABLE 	    PORT_ResetBits(SPI_W25Q128_NSS_PORT, SPI_W25Q128_NSS_PIN)
#define SPI_FLASH_NSS_DISENABLE 	PORT_SetBits(SPI_W25Q128_NSS_PORT, SPI_W25Q128_NSS_PIN)
 
 
#define SPI_FLASH_PAGE_SIZE    0x100 //GD25Qxx每页有256个字节
#define WIP_FLAG         0x01     /* write in progress(wip)flag */
 
#define DEVICE_SIZE 	0x1000000 	// 16M = 256 * BLOCK
#define BLOCK_SIZE		0x10000		// 64K	= 16 * SECTOR
#define SECTOR_SIZE		0x1000 		// 4K = 16 * PAGE
#define PAGE_SIZE		  0x100 		// 256Bytes


/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
SystemPataSt *pst_W25Q128SystemPara;
/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
union W25Q128_UINT32_DATA_TYPE
{
	uint8_t b[4];
    uint32_t Uint32_Data;
};
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
uint8_t guc_W25Q128_Buf[4096];
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
	uint8_t * W25QXX_BUF;   
	W25QXX_BUF=guc_W25Q128_Buf;       
   	secpos=write_addr/4096;//扇区地址  
	secoff=write_addr%4096;//在扇区内的偏移
	secremain=4096-secoff;//扇区剩余空间大小   
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
	//W25Q128_Get_ReadDataBytes(secpos*4096,W25QXX_BUF,4096);//读出整个扇区的内容
	#endif
} 

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

	switch (eCMD)
	{
	case DEV_ID:
		ucTmpData = strlen((char*)cDataArr);
		if(ucTmpData > 16)
		{
			ucResult = 0;
		}
		else
		{
			strcpy(&pst_W25Q128SystemPara->DevicePara.cDeviceID[0],(char*)cDataArr);
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceID[0],SYSTEM_PARA_ADDR,16);
			pst_W25Q128SystemPara->DevicePara.cDeviceRegisterFlag = 0;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceRegisterFlag,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cDeviceRegisterFlag-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_IDEN_FLAG:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 1)
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cDeviceIdenFlag = ucTmpData;
			pst_W25Q128SystemPara->DeviceRunPara.c4GInitFlag = 0;
            W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceIdenFlag,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cDeviceIdenFlag-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			if(pst_W25Q128SystemPara->DevicePara.cDeviceIdenFlag == 1)
			{
				pst_W25Q128SystemPara->DevicePara.cDeviceRegisterFlag = 0;
				W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceRegisterFlag,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cDeviceRegisterFlag-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
			}
			
		}
		break;
	case DEV_REG_FLAG:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 1)
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cDeviceRegisterFlag = ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceRegisterFlag,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cDeviceRegisterFlag-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_PD_DATE:
		ucTmpData = strlen((char*)cDataArr);
		if(ucTmpData > 9)
		{
			ucResult = 0;
		}
		else
		{
			strcpy(&pst_W25Q128SystemPara->DevicePara.cDevicePDDate[0],(char*)cDataArr);
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDevicePDDate[0],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cDevicePDDate[0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),9);
		}
	case DEV_SAMPLE_GAP:
		nTmpData = *((int *)cDataArr);  
		if (nTmpData > 0x270F) 
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.nDeviceSampleGapCnt = nTmpData;
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.nDeviceSampleGapCnt,SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.nDeviceSampleGapCnt-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
		}
		break;
	case DEV_RECORD_GAP:
		nTmpData = *((int *)cDataArr);  
		if (nTmpData > 0x270F) 
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.nDeviceSaveRecordCnt = nTmpData;
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.nDeviceSaveRecordCnt,SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.nDeviceSaveRecordCnt-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
		}
		break;
	case DEV_UPLOAD_GAP:
		nTmpData = *((int *)cDataArr);  
		if (nTmpData > 0x270F) 
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.nDeviceUploadCnt = nTmpData;
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.nDeviceUploadCnt,SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.nDeviceUploadCnt-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
		}
		break;
	case DEV_HIS_RECORD:
		nTmpData = *((int *)cDataArr);  
		if (nTmpData > 0x270F) 
		{ 
			return(0);
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.nDeviceRecordCnt = nTmpData;
			#pragma diag_suppress=Pa039
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.nDeviceRecordCnt,SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.nDeviceRecordCnt-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(int));
			#pragma diag_warning=Pa039
		}
		break;
	case DEV_SENSOR_ENABLE_1:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 1)
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cMeasSensorEnableFlag[0] = ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cMeasSensorEnableFlag[0],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cMeasSensorEnableFlag[0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_SENSOR_ENABLE_2:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 1)
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cMeasSensorEnableFlag[1] = ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cMeasSensorEnableFlag[1],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cMeasSensorEnableFlag[1]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_SENSOR_CNT_1:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 10)
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cMeasSensorCount[0] = ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cMeasSensorCount[0],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cMeasSensorCount[0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_SENSOR_CNT_2:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 10)
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cMeasSensorCount[1] = ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cMeasSensorCount[1],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cMeasSensorCount[1]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN1_SENSOR1_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][0] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][0],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN1_SENSOR2_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][1] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][1],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][1]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN1_SENSOR3_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][2] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][2],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][2]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN1_SENSOR4_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][3] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][3],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][3]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN1_SENSOR5_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][4] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][4],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][4]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN1_SENSOR6_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][5] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][5],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][5]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN1_SENSOR7_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][6] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][6],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][6]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN1_SENSOR8_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][7] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][7],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][7]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN1_SENSOR9_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][8] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][8],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][8]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN1_SENSOR10_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][9] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][9],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][9]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN2_SENSOR1_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][0] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][0],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN2_SENSOR2_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][1] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][1],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][1]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN2_SENSOR3_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][2] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][2],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][2]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN2_SENSOR4_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][3] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][3],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][3]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN2_SENSOR5_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][4] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][4],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][4]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN2_SENSOR6_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][5] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[0][5],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][5]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN2_SENSOR7_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][6] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][6],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][6]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN2_SENSOR8_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][7] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][7],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][7]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN2_SENSOR9_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][8] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][8],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][8]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_CHN2_SENSOR10_TYPE:
		ucTmpData = *cDataArr;     
		if ((ucTmpData > Meas_Max-1) || (ucTmpData < Meas_BY_Integrated_Conductivity))
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][9] = (EMeasSensorType)ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][9],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.eMeasSensor[1][9]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	case DEV_TOTAL_VOLUME:
		//memcpy(&pst_W25Q128SystemPara->DevicePara.fTotal_Volume, cDataArr, sizeof(float));
		fTmpData = ((float*)cDataArr);
		pst_W25Q128SystemPara->DevicePara.fTotal_Volume = *fTmpData;
		#pragma diag_suppress=Pa039
		W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.fTotal_Volume,SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.fTotal_Volume-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(float));
		#pragma diag_warning=Pa039
		break;
	case DEV_SOFTRESET:	//设备软复位
		func_System_Soft_Reset();
		break;	
	case DEV_STATUS_UPLOAD_GAP:
		ucTmpData = *cDataArr;     
		pst_W25Q128SystemPara->DevicePara.ucUploadStatusGap = ucTmpData;
		#pragma diag_suppress=Pa039
		W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.ucUploadStatusGap,SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.ucUploadStatusGap-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(unsigned char));
		#pragma diag_warning=Pa039
		break;
	case DEV_IP_ADDRESS1:
		ucTmpData = strlen((char*)cDataArr);
		if(ucTmpData > 15)
		{
			ucResult = 0;
		}
		else
		{
			strcpy(&pst_W25Q128SystemPara->DevicePara.cServerIP[0][0],(char*)cDataArr);
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cServerIP[0][0],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cServerIP[0][0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),16);
		}
		break;
	case DEV_IP_ADDRESS2:
		ucTmpData = strlen((char*)cDataArr);
		if(ucTmpData > 15)
		{
			ucResult = 0;
		}
		else
		{
			strcpy(&pst_W25Q128SystemPara->DevicePara.cServerIP[1][0],(char*)cDataArr);
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cServerIP[1][0],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cServerIP[1][0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),16);
		}
		break;
	case DEV_IP_PORT1:
	usTmpData = *((unsigned short *)cDataArr);
		if(usTmpData > 60000)
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.usServerPort[0] = usTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usServerPort[0],SYSTEM_PARA_ADDR+((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usServerPort[0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(unsigned short));
		}
		break;
	case DEV_IP_PORT2:
		usTmpData = *((unsigned short *)cDataArr);
		if(usTmpData > 60000)
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.usServerPort[1] = usTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usServerPort[1],SYSTEM_PARA_ADDR+((uint8_t *)&pst_W25Q128SystemPara->DevicePara.usServerPort[1]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(unsigned short));
		}
		break;
	case DEV_DEBUG_MODEL:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 1)
		{
			ucResult = 0;
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
		pst_W25Q128SystemPara->DevicePara.fInit_Height = *fTmpData;
		#pragma diag_suppress=Pa039
		W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.fInit_Height,SYSTEM_PARA_ADDR+((char*)&pst_W25Q128SystemPara->DevicePara.fInit_Height-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),sizeof(float));
		#pragma diag_warning=Pa039
		break;
	case DEV_SENSOR_BAUDRATE:
		ucTmpData = *cDataArr;     
		if (ucTmpData > 5)
		{
			ucResult = 0;
		}
		else
		{
			pst_W25Q128SystemPara->DevicePara.cSensorBaudRate = ucTmpData;
			W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cSensorBaudRate,SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cSensorBaudRate-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),1);
		}
		break;
	default:
		break;
	}
	return ucResult;
}

//保存设备测量数据
void func_Save_Device_MeasData(void)
{
	W25Q128_Spi_flash_buffer_write((uint8_t *)&gSt_DevMeasRecordData.fWaterLevel_Radar,SYSTEM_RECORD_START_ADDR+pst_W25Q128SystemPara->DevicePara.nDeviceRecordCnt*SYSTEM_RECORD_SIZE,SYSTEM_RECORD_SIZE);
}

//读取设备测量数据
void func_Get_Device_MeasData_Record(int nRecordIndex, DevMeasRecordDataSt *pstMeasData)
{
	W25Q128_Get_ReadDataBytes(SYSTEM_RECORD_START_ADDR+nRecordIndex*SYSTEM_RECORD_SIZE, (uint8_t *)pstMeasData, SYSTEM_RECORD_SIZE);
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
		//记录备份数据
		//W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceID[0],SYSTEM_BACKUP_PARA_ADDR,sizeof(gs_DeviceDefaultPara));
		//采用默认参数
		memcpy(&pst_W25Q128SystemPara->DevicePara.cDeviceID[0], &gs_DeviceDefaultPara, sizeof(gs_DeviceDefaultPara));
		W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceID[0],SYSTEM_PARA_ADDR,sizeof(gs_DeviceDefaultPara));
	}
	if( strcmp( (char*)&pst_W25Q128SystemPara->DevicePara.cDeviceSWVersion[0], (char *)&gs_DeviceDefaultPara.cDeviceSWVersion[0]) )
	{
		memcpy(&pst_W25Q128SystemPara->DevicePara.cDeviceSWVersion[0],&gs_DeviceDefaultPara.cDeviceSWVersion[0],10);
		W25Q128_Spi_flash_buffer_write((uint8_t *)&pst_W25Q128SystemPara->DevicePara.cDeviceSWVersion[0],SYSTEM_PARA_ADDR+(&pst_W25Q128SystemPara->DevicePara.cDeviceSWVersion[0]-&pst_W25Q128SystemPara->DevicePara.cDeviceID[0]),10);
	}
}

/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
