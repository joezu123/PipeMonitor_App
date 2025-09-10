 /************************************************************/
//2014.03.06修改版
/*************************************************************/

#include "fm175xx.h"
#include <string.h>	
#include "stdint.h"
#include "drv_USART.h"
#include "hc32f460_utility.h"

#define MI_NOTAGERR 0xEE
#define MAXRLEN 64
#define Anticollision   0x02

/*************************************************************/
/*函数名：	    Set_BitMask									 */
/*功能：	    置位寄存器操作								 */
/*输入参数：	reg_add，寄存器地址；mask，寄存器置位		 */
/*返回值：	    TRUE											 */
/*				FALSE										 */
/*************************************************************/
unsigned char Set_BitMask(unsigned char reg_add,unsigned char mask)
{
    //unsigned char data result;
    drv_RF_NFC_SendData(reg_add,Read_Reg(reg_add) | mask);  // set bit mask
	return TRUE;
}
/********************************************
函数名：	    Clear_BitMask
功能：	    清除位寄存器操作
				
输入参数：	reg_add，寄存器地址；mask，寄存器清除位
返回值：	    TRUE
				FALSE
********************************************/
unsigned char Clear_BitMask(unsigned char reg_add,unsigned char mask)
{
    unsigned char result;
    result=Write_Reg(reg_add,Read_Reg(reg_add) & ~mask);  // clear bit mask
	return result;
}

/*************************************************************
函数名：	    Read_Reg
功能：	      读寄存器操作
输入参数：	  reg_add，寄存器地址
返回值：	    寄存器数值
*************************************************************/
uint8_t guc_FM17550_ReadValue = 0;
uint8_t guc_FM17550_ReadCMD = 0;
unsigned char Read_Reg(unsigned char reg_add)
{
	unsigned char  reg_value;	   
	reg_value = drv_RF_NFC_ReceiveData(reg_add);
	guc_FM17550_ReadValue = reg_value;
	guc_FM17550_ReadCMD = reg_add;
 	return reg_value;
}
/*************************************************************
函数名：	    Read_Reg All
功能：	    读64个寄存器操作
输入参数：	无
输出参数		*reg_value，寄存器数值指针
返回值：	    TRUE
************************************************************/
unsigned char Read_Reg_All(unsigned char *reg_value)
{
	unsigned char i;
	for (i=0;i<64;i++)	   
	{
		*(reg_value+i)=drv_RF_NFC_ReceiveData(i);
		//drv_RF_NFC_ReceiveData(i, (reg_value+i), 1);
	}
		
 	return TRUE;
}

/*************************************************************
函数名：	    Write_Reg									 
功能：	      写寄存器操作								 

输入参数：	  reg_add，寄存器地址；reg_value，寄存器数值	 
返回值：	    TRUE											 
				      FALSE										 
*************************************************************/
unsigned char Write_Reg(unsigned char reg_add,unsigned char reg_value)
{
	drv_RF_NFC_SendData(reg_add,reg_value);
	return TRUE;
}
/*************************************************************
函数名：	    Read_FIFO									                   
功能：	    读取FIFO  									                   
															                            
输入参数：	length，读取数据长度					                
输出参数：	*fifo_data，数据存放指针		             			
返回值：	    											                      
*************************************************************/
void Read_FIFO(unsigned char length,unsigned char *fifo_data)
{	 
	uint8_t i;
	for(i=0; i<length; i++)
	{
		fifo_data[i] = drv_RF_NFC_ReceiveData(FIFODataReg);
	}
	//SPIRead_Sequence(length,FIFODataReg,fifo_data);
	return;
}
/*************************************************************/
/*函数名：	    Write_FIFO									 */
/*功能：	    写入FIFO  									 */
/*															 */
/*输入参数：	length，读取数据长度；*fifo_data，数据存放指针*/
/*输出参数：												 */
/*返回值：	    											 */
/*															 */
/*************************************************************/
void Write_FIFO(unsigned char length,unsigned char *fifo_data)
{
	uint8_t i;
	for(i=0; i<length; i++)
	{
		drv_RF_NFC_SendData(FIFODataReg, fifo_data[i]);
	}
	//SPIWrite_Sequence(length,FIFODataReg,fifo_data);
	return;
}
/*************************************************************
函数名：	    Clear_FIFO					 
功能：	      清空FIFO  				
															
输入参数：	  无								
输出参数：										
返回值：	    TRUE						
				      FALSE									
************************************************************/
unsigned char Clear_FIFO(void)
{
	Set_BitMask(FIFOLevelReg,0x80);//清除FIFO缓冲
	if (drv_RF_NFC_ReceiveData(FIFOLevelReg)==0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
		
}

/*********************************************/
/*函数名：	    Set_RF
功能：	    设置射频输出
				
输入参数：	mode，射频输出模式
				0，关闭输出
        1，仅打开TX1输出
        2，仅打开TX2输出
				3，TX1，TX2打开输出，TX2为反向输出
返回值：	    TRUE
				FALSE
********************************************/

unsigned char Set_Rf(unsigned char mode)
{
	unsigned char result;
	if	((Read_Reg(TxControlReg)&0x03)==mode)
	{
		return TRUE;
	}
	if (mode==0)
	{
		result=Clear_BitMask(TxControlReg,0x03); //关闭TX1，TX2输出
	}
	if (mode==1)
	{
		result=Clear_BitMask(TxControlReg,0x01); //仅打开TX1输出
	}
	if (mode==2)
	{
		result=Clear_BitMask(TxControlReg,0x02); //仅打开TX2输出
	}
	if (mode==3)
	{
		result=Set_BitMask(TxControlReg,0x03); //打开TX1，TX2输出
	}
	Ddl_Delay1ms(100);
	return result;
}
/*********************************************
 函数名：	    Pcd_Comm （不利用IRQ管脚的情况）
 功能：	    读卡器通信
				
 输入参数：	Command，通信操作命令；
				pInData，发送数据数组；
				InLenByte，发送数据数组字节长度；
				pOutData，接收数据数组；
				pOutLenBit，接收数据的位长度
 返回值：	    TRUE
				FALSE
 ********************************************/  
unsigned char Pcd_Comm(	unsigned char Command, 
                 		unsigned char *pInData, 
                 		unsigned char InLenByte,
                 		unsigned char *pOutData, 
                 		unsigned int *pOutLenBit)
{
   	uint8_t status  = FALSE;
    uint8_t irqEn   = 0x00;
    uint8_t waitFor = 0x00;
    uint8_t lastBits;
	uint8_t n,sendLen,sendPi,revLen,fifoLen,errorReg,collPos;
    uint32_t i;
	sendPi = 0;
	Write_Reg(ComIrqReg, 0x7F);               //清楚IRQ标记
	Write_Reg(TModeReg,0x80);                 //设置TIMER自动启动
    switch (Command) 
	{
		case MFAuthent:                                                  /* Mifare认证                   */
			irqEn   = 0x12;
			waitFor = 0x10;
		break;
		case Transceive:                                               /* 发送FIFO中的数据到天线，传输后激活接收电路*/
			irqEn   = 0x77;
			waitFor = 0x30;
		break;
		default:
		break;
    }
   
    Write_Reg(ComIEnReg, irqEn | 0x80);
   // Clear_BitMask(ComIrqReg, 0x80);
    Write_Reg(CommandReg, Idle);
    Set_BitMask(FIFOLevelReg, 0x80);
    
    for (i=0; i < InLenByte; i++) 
	{
        Write_Reg(FIFODataReg, pInData[i]);
    }
    Write_Reg(CommandReg, Command);

    if (Command == Transceive) 
	{
        Set_BitMask(BitFramingReg, 0x80);
    }

    i = 300;                                                            /* 根据时钟频率调整，操作M1卡最大等待时间25ms*/

    do 
	{
        n = Read_Reg(ComIrqReg);
		fifoLen = drv_RF_NFC_ReceiveData(FIFOLevelReg);
		if(( InLenByte >0 )&&( fifoLen <0x10 ))                            
		{
				sendLen = InLenByte>30?30:InLenByte;
				Write_FIFO(sendLen, &pInData[sendPi]);
				sendPi += sendLen;
				InLenByte -=sendLen;
		}
		if( ( InLenByte == 0)&&( fifoLen >0x28 ) )
		{
					Read_FIFO(fifoLen,&pOutData[revLen]);
				revLen += fifoLen;
		}
        i--;
    } while ((i != 0) && !(n & 0x03) && !(n & waitFor));         //i==0表示延时到了，n&0x01!=1表示PCDsettimer时间未到
		                                                             //n&waitFor!=1表示指令执行完成

    Clear_BitMask(BitFramingReg, 0x80);

    if (i != 0) 
	{
		errorReg = Read_Reg(ErrorReg);
        if(!(errorReg & 0x1B)) 
		{
            status = TRUE;
            if (n & irqEn & 0x01) // n & 0x77 & 0x01 ==> n & 0x01 ; 因此n的最低位不能为1
			{
                status = MI_NOTAGERR;
            }
			else
			{
				lastBits = 1;
			}
            if (Command == Transceive) 
			{
                n = Read_Reg(FIFOLevelReg);
                lastBits = Read_Reg(ControlReg) & 0x07;
                if (lastBits) 
				{
                    *pOutLenBit = (n - 1) * 8 + lastBits;
                } 
				else 
				{
                    *pOutLenBit = n * 8;
                }
                if (n == 0) 
				{
                    n = 1;
                }
                if (n > MAXRLEN) 
				{
                    n = MAXRLEN;
                }
                for (i = 0; i < n; i++) 
				{
                    pOutData[i] = Read_Reg(FIFODataReg);
                }
            }
        } 
		else if(errorReg & 0x08)                                  /* 冲突                               */
        {
			Write_Reg(ErrorReg,~(0x08));                                         /* 清除接收中断 */
			collPos = Read_Reg(CollReg);                                        /* 获取冲突位置 */
			collPos &= 0x1f;
			*pOutLenBit = (collPos == 0 )?32:collPos;
			fifoLen =*pOutLenBit/8 +(*pOutLenBit%8?1:0);
			Read_FIFO(fifoLen,&pOutData[revLen]);
			status = Anticollision;
		}					
		else 
		{
            status = FALSE;
        }
    }
 	Clear_BitMask(BitFramingReg,0x80);//关闭发送
    return status;
}
 /********************************************
 函数名：	    Pcd_SetTimer
 功能：	    设置接收延时
 输入参数：	delaytime，延时时间（单位为毫秒）
 返回值：	    TRUE
 ********************************************/
 unsigned char Pcd_SetTimer(unsigned long delaytime)//设定超时时间（ms）
{
	unsigned long  TimeReload;
	unsigned int Prescaler;

	Prescaler=0;
	TimeReload=0;
	while(Prescaler<0xfff)
	{
		TimeReload = ((delaytime*(long)13560)-1)/(Prescaler*2+1);
		
		if( TimeReload<0xffff)
			break;
		Prescaler++;
	}
	TimeReload=TimeReload&0xFFFF;
	Set_BitMask(TModeReg,Prescaler>>8);
	Write_Reg(TPrescalerReg,Prescaler&0xFF);					
	Write_Reg(TReloadMSBReg,TimeReload>>8);
	Write_Reg(TReloadLSBReg,TimeReload&0xFF);
	return TRUE;
}
 /********************************************
 函数名：	    Pcd_ConfigISOType
 功能：	    设置操作协议
 输入参数：	type 0，ISO14443A协议；
 					 1，ISO14443B协议；
 返回值：	    TRUE
 ********************************************/
unsigned char Pcd_ConfigISOType(unsigned char type)
{
   	if (type == 0)                     //ISO14443_A
   	{ 
   		Set_BitMask(ControlReg, 0x10); //ControlReg 0x0C 设置reader模式
		Set_BitMask(TxAutoReg, 0x40); //TxASKReg 0x15 设置100%ASK有效
	    Write_Reg(TxModeReg, 0x00);  //TxModeReg 0x12 设置TX CRC无效，TX FRAMING =TYPE A
	    Write_Reg(RxModeReg, 0x00); //RxModeReg 0x13 设置RX CRC无效，RX FRAMING =TYPE A
   	}
   	if (type == 1)                     //ISO14443_B
	{ 
   		//Write_Reg(ControlReg, 0x10); //ControlReg 0x0C 设置reader模式
	    //Write_Reg(TxModeReg, 0x83); //TxModeReg 0x12 设置TX CRC有效，TX FRAMING =TYPE B
		 // Write_Reg(RxModeReg, 0x83); //RxModeReg 0x13 设置RX CRC有效，RX FRAMING =TYPE B
			//Write_Reg(GsNReg, 0xF4); //GsNReg 0x27 设置ON电导
			//Write_Reg(GsNOffReg, 0xF4); //GsNOffReg 0x23 设置OFF电导
			//Write_Reg(TxAutoReg, 0x00);// TxASKReg 0x15 设置100%ASK无效
			
		Write_Reg(ControlReg,0x10);
		Write_Reg(TxModeReg,0x83);//BIT1~0 = 2'b11:ISO/IEC 14443B
		Write_Reg(RxModeReg,0x83);//BIT1~0 = 2'b11:ISO/IEC 14443B
		Write_Reg(TxAutoReg,0x00);
		Write_Reg(RxThresholdReg,0x55);
		Write_Reg(RFCfgReg,0x48);//?????????????
		Write_Reg(TxBitPhaseReg,0x87);//默认值	
		Write_Reg(GsNReg,0x83);		//0x83, 0XF8
		Write_Reg(CWGsPReg,0x10);	//0X10, 0X3F
		Write_Reg(GsNOffReg,0x38);
		Write_Reg(ModGsPReg,0x10);	//0x10, 0x08
	
	}
	Ddl_Delay1ms(1);
   	return TRUE;
}
/********************************************
 函数名：	    FM175X_SoftReset
 功能：	    软复位操作
				
 输入参数：	
				

 返回值：	    TRUE
				FALSE
 ********************************************/

unsigned char  FM175X_SoftReset(void)
{	
	Write_Reg(CommandReg,SoftReset);//
	return	Set_BitMask(ControlReg,0x10);//17520初始值配置
}

/*********************************************
 函数名：	    FM175X_SoftPowerdown
 功能：	    软件低功耗操作
 输入参数：	
 返回值：	    TRUE，进入低功耗模式；
 				FALSE，退出低功耗模式；
 ********************************************/
unsigned char FM175X_SoftPowerdown(void)
{
	if (Read_Reg(CommandReg)&0x10)
	{
		Clear_BitMask(CommandReg,0x10);//退出低功耗模式
		return FALSE;
	}
	else
	{
		Set_BitMask(CommandReg,0x10);//进入低功耗模式
		return TRUE;
	}
}
/*********************************************
 函数名：	    FM175X_HardPowerdown
 功能：	    硬件低功耗操作
 输入参数：	
 返回值：	    TRUE，进入低功耗模式；
 				FALSE，退出低功耗模式；
*********************************************/
unsigned char FM175X_HardPowerdown(void)
{	
	//NPD=~NPD;
	//if(NPD==1)
		return TRUE; //进入低功耗模式
//	else
		//return FALSE;//退出低功耗模式
}
/********************************************
 函数名：	    Read_Ext_Reg
 功能：	    读取扩展寄存器
 输入参数：	reg_add，寄存器地址
 返回值：	    寄存器数值
 ********************************************/
unsigned char Read_Ext_Reg(unsigned char reg_add)
{
 	Write_Reg(0x0F,0x80+reg_add);
 	return Read_Reg(0x0F);
}
 /********************************************
 函数名：	    Write_Ext_Reg
 功能：	    写入扩展寄存器
 输入参数：	reg_add，寄存器地址；reg_value，寄存器数值
 返回值：	    TRUE
				FALSE
 ********************************************/
unsigned char Write_Ext_Reg(unsigned char reg_add,unsigned char reg_value)
{
	Write_Reg(0x0F,0x40+reg_add);
	return (Write_Reg(0x0F,0xC0+reg_value));
}


