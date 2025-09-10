#include "drv_IIC.h"
#include "hc32f460_utility.h"




unsigned char SM4IICWriteAddr = 0x50;
unsigned char SM4IICReadAddr  = 0x51;

unsigned char SC7AIICWRITEADDR = 0x30;
unsigned char SC7AIICREADADDR  = 0x31;

unsigned char XYC_ALSIICWRITEADDR = 0x38;
unsigned char XYC_ALSIICREADADDR  = 0x39;

unsigned char nackFlag;		      //非应答标志
/**************************************************
*uint8_t drv_mcu_SM4_IIC_Init(void)
*入口参数：无
*出口参数：0: init success; 1: init failed
*功能说明：I2C 引脚初始化
***************************************************/
uint8_t drv_mcu_SM4_IIC_Init(void)  
{
	//初始化RST,SCL,SDA引脚 :RST输出 SDA输出 SCL输出 	
	stc_port_init_t stcPortInit;

    /* configure structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);

    /* Flash NSS */
    stcPortInit.enPinMode = Pin_Mode_Out;
	stcPortInit.enPinOType = Pin_OType_Od;
    PORT_Init(SM4_IIC_SDA_PORT, SM4_IIC_SDA_PIN, &stcPortInit);		
	PORT_Init(SM4_IIC_SCL_PORT, SM4_IIC_SCL_PIN, &stcPortInit);
	PORT_Init(SM4_IIC_RESET_PORT, SM4_IIC_RESET_PIN, &stcPortInit);        	  
	SM4_IIC_SDA_H;//SDA拉低
	SM4_IIC_SCL_H;//SCL拉高

	return 0;
} 

/*********************复位 LKT**********************/
void drv_SM4_IIC_Reset(void)
{
	SM4_IIC_REST_L;//RST拉低
	Ddl_Delay1ms(4);
	SM4_IIC_REST_H ;//RST拉高
	Ddl_Delay1ms(20);//(1000);
}

/*********************IIC停止信号***********************/
void drv_SM4_IIC_Stop(void)
{	
	SM4_IIC_SCL_L;  //
	Ddl_Delay1us(1);
	SM4_IIC_SDA_L;
	Ddl_Delay1us(3);
	SM4_IIC_SCL_H;
	Ddl_Delay1us(3);
  	SM4_IIC_SDA_H;
}

/*********************IIC起始信号***********************/
void drv_SM4_IIC_Start(void)
{
	SM4_IIC_SDA_H;
	Ddl_Delay1us(2);	
	SM4_IIC_SCL_H;
	Ddl_Delay1us(2);	
	SM4_IIC_SDA_L;//SDA拉低
	Ddl_Delay1us(4);	
	SM4_IIC_SCL_L;//SCL拉低
	Ddl_Delay1us(2);
}
	
/************MCU向LKT发送应答信号**********************/
void drv_SM4_IIC_Ack(void)
{	
	Ddl_Delay1us(4);
	SM4_IIC_SCL_H;//SCL拉高
	Ddl_Delay1us(4);	
	SM4_IIC_SCL_L;//SCL拉低
	SM4_IIC_SDA_H;      //SDA拉高
}

/************MCU向LKT发送非应答信号**********************/
void drv_SM4_IIC_NAck(void)
{
	SM4_IIC_SDA_H; //SDA拉高
	Ddl_Delay1us(3);  
	SM4_IIC_SCL_H; 
	Ddl_Delay1us(4);
	SM4_IIC_SCL_L; //SCL 拉低
 
}

/**************检测LKT是否返回应答信息**********************/
void drv_SM4_IIC_Checkack(void)
{  
	SM4_IIC_SCL_L; //SCL 拉低
	SM4_IIC_SDA_H; //SDA
	Ddl_Delay1us(4);//Delay_IIC_1us(10);	
	SM4_IIC_SCL_H; //SCL拉高
	nackFlag = 0;  	
	Ddl_Delay1us(4);	
	if(SM4_IIC_SdaState) 			// 若SDA=1表明非应答，置位非应答标志
	{
		nackFlag = 0x01;
	}
}


/*************** 发送一个字节数据子函数 **********************/
void drv_SM4_IIC_Sendbyte(unsigned char *Sdata)
{ 
	char temp,i;
	temp = *Sdata;
	for(i=0;i<8;i++)
	{
		SM4_IIC_SCL_L; 
		Ddl_Delay1us(2);	
    	if((temp&0x80) == 0x80)
		{
			SM4_IIC_SDA_H;
		}
		else
		{
			SM4_IIC_SDA_L;
		}
			
		Ddl_Delay1us(2);
    	SM4_IIC_SCL_H; 
		Ddl_Delay1us(4);
		temp = temp<<1;
	} 

}
/*************** 接收一字节子程序 ******************************/
uint8_t drv_SM4_IIC_Recbyte(unsigned  char *Rdata)
{
	char i;    		        // 从SDA线上读取一位数据字节，共8位
	uint8_t k=0;
//	set_sda_direction(IN);		//SDA输入
	*Rdata =0x00;
	for(i=0;i<8;i++)
	{
		SM4_IIC_SCL_L; 
		Ddl_Delay1us(4);	
		SM4_IIC_SCL_H;
		while(!SM4_IIC_SclState)
		{
			if(k>30)
			{
				return SCL_TimeOut;
			} 
			k++;
			Ddl_Delay1us(1);
		}
		Ddl_Delay1us(4);	
    	*Rdata = *Rdata<<1;		
    	if(SM4_IIC_SdaState)
		{
			*Rdata |=0x01;
		}
	}
	
	SM4_IIC_SCL_L; //SCL拉低
	SM4_IIC_SDA_L; //SDA拉低
	
  	return 0;
}

/****************发送n字节数据子程序 *****************************/
uint8_t drv_SM4_IIC_WriteData(char  *Sendbuf , uint16_t len)
{          
	drv_SM4_IIC_Start();					// 发送启动信号
	drv_SM4_IIC_Sendbyte(&SM4IICWriteAddr);		    //发送读地址	
	drv_SM4_IIC_Checkack();
	if (1 == nackFlag)
	{
		drv_SM4_IIC_Stop();
		return IICNACK;    			// 若非应答表明器件错误或已坏
	}
	while(len--)
	{ 
		Ddl_Delay1us(1);
		drv_SM4_IIC_Sendbyte((unsigned char*)Sendbuf);
		drv_SM4_IIC_Checkack();    			// 检查应答位
		if (1 == nackFlag)
		{
			drv_SM4_IIC_Stop();
			return IICNACK;    			// 若非应答表明器件错误或已坏
		}
		Sendbuf++;
	}
	if(len!=1)
	{
		drv_SM4_IIC_Stop();
	}
	return Successful;
	
}

/*****************接收n字节数据子程序 ******************************/
uint8_t drv_SM4_IIC_ReadData(char  *RecBuf, uint16_t* len)
{
	uint8_t res=0;
	int k,i=0,time=1500000;
	uint16_t usLen = 0;
	drv_SM4_IIC_Start();	
	drv_SM4_IIC_Sendbyte(&SM4IICReadAddr);		 //发送读地址	
	drv_SM4_IIC_Checkack();
	while(1 == nackFlag)
	{
		drv_SM4_IIC_Stop();
		if(i==time)
		{
			return IICRadAddrNACK ; 
		}
		Ddl_Delay1us(1);
		drv_SM4_IIC_Start();
		drv_SM4_IIC_Sendbyte(&SM4IICReadAddr);	
		drv_SM4_IIC_Checkack();
		SM4_IIC_SDA_H;     //SDA拉高
		i++;
	}	
	SM4_IIC_SDA_H;     //SDA拉高
	
	for(k=0;k<2;k++)
	{
		res=drv_SM4_IIC_Recbyte((unsigned char*)(RecBuf+k));
		if(res != Successful )
		{
			drv_SM4_IIC_Stop();
			return res;
		}
		drv_SM4_IIC_Ack();    		// 收到一个字节后发送一个应答位
		//*len = RecBuf[0]*0x100 + RecBuf[1];	
		usLen = RecBuf[0]*0x100 + RecBuf[1];	

	}	
	RecBuf +=2;

	*len = usLen;
	while((usLen)--)
	{ 
		res=drv_SM4_IIC_Recbyte((unsigned char*)(RecBuf));
		if(res != Successful )
		{
			drv_SM4_IIC_Stop();
			return res;
		}
		if((usLen)>0)
		{
			drv_SM4_IIC_Ack();    		// 收到一个字节后发送一个应答位
		}
		
		RecBuf++;
	}
	drv_SM4_IIC_NAck();    		   //收到最后一个字节后发送一个非应答位
	drv_SM4_IIC_Stop();
	
	return Successful;
}

uint8_t drv_SM4_IIC_SendApdu(uint16_t Sendlen,char* Sendbuf,char* RecBuf,uint16_t* Relen)
{
	uint8_t state=0xFF;

    state = drv_SM4_IIC_WriteData(Sendbuf,Sendlen); 
	if(state != Successful)
	{
		return state;
	}
			
	Ddl_Delay1us(10000);	
	state=drv_SM4_IIC_ReadData(RecBuf,Relen); 
	if(state != Successful)
	{
		return state;
	}
			
	return Successful;
}


/**************************************************
*uint8_t drv_mcu_SC7A_IIC_Init(void)
*入口参数：无
*出口参数：0: init success; 1: init failed
*功能说明：I2C 引脚初始化
***************************************************/
uint8_t drv_mcu_SC7A_IIC_Init(void)  
{
	//初始化RST,SCL,SDA引脚 :RST输出 SDA输出 SCL输出 	
	stc_port_init_t stcPortInit;

    /* configure structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);

    /* Flash NSS */
    stcPortInit.enPinMode = Pin_Mode_Out;
	stcPortInit.enPinOType = Pin_OType_Od;
    PORT_Init(SC7A_IIC_SDA_PORT, SC7A_IIC_SDA_PIN, &stcPortInit);		
	PORT_Init(SC7A_IIC_SCL_PORT, SC7A_IIC_SCL_PIN, &stcPortInit);
    	  
	//SC7A_IIC_SDA_H;//SDA拉低
	//SC7A_IIC_SCL_H;//SCL拉高

	return 0;
}

//设置IIC SDA 引脚为输出引脚
void drv_mcu_SC7A_IIC_SDA_PIN_SET_OUTPUT()
{
	//初始化RST,SCL,SDA引脚 :RST输出 SDA输出 SCL输出 	
	stc_port_init_t stcPortInit;

    /* configure structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);
	stcPortInit.enPinMode = Pin_Mode_Out;
	stcPortInit.enPinOType = Pin_OType_Od;
    PORT_Init(SC7A_IIC_SDA_PORT, SC7A_IIC_SDA_PIN, &stcPortInit);	
}

//设置IIC SDA 引脚为输入引脚
void drv_mcu_SC7A_IIC_SDA_PIN_SET_INPUT()
{
	//初始化RST,SCL,SDA引脚 :RST输出 SDA输出 SCL输出 	
	stc_port_init_t stcPortInit;

    /* configure structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);
	stcPortInit.enPinMode = Pin_Mode_In;
	//stcPortInit.enPinOType = Pin_OType_Od;
    PORT_Init(SC7A_IIC_SDA_PORT, SC7A_IIC_SDA_PIN, &stcPortInit);	
}

/*********************IIC停止信号***********************/
void drv_SC7A_IIC_Stop(void)
{	
	SC7A_IIC_SCL_L;  //
	Ddl_Delay1us(15);
	SC7A_IIC_SDA_L;
	Ddl_Delay1us(15);
	SC7A_IIC_SCL_H;
	Ddl_Delay1us(15);
	SC7A_IIC_SDA_H;
	Ddl_Delay1us(15);
}

/*********************IIC起始信号***********************/
void drv_SC7A_IIC_Start(void)
{
	SC7A_IIC_SDA_H;
	Ddl_Delay1us(15);
	SC7A_IIC_SCL_H;
	Ddl_Delay1us(15);	
	SC7A_IIC_SDA_L;//SDA拉低
	Ddl_Delay1us(15);
	SC7A_IIC_SCL_L;//SCL拉低
	Ddl_Delay1us(15);	
	
}

/************MCU向SC7A发送应答/非应答信号**********************/
void drv_SC7A_IIC_ACKorNACK(uint8_t uAck)
{	
	drv_mcu_SC7A_IIC_SDA_PIN_SET_OUTPUT(); //设置SDA为输出
	Ddl_Delay1us(14);
	SC7A_IIC_SCL_H;//SCL拉高
	Ddl_Delay1us(14);	
	SC7A_IIC_SCL_L;//SCL拉低
	SC7A_IIC_SDA_H;      //SDA拉高

	SC7A_IIC_SCL_L;        //SCL    ____________时钟线拉低，才允许改变数据位，防止SDA原来为低电平
    Ddl_Delay1us(15);	
    
    if(uAck==1) 
	{
        SC7A_IIC_SDA_H;
    } 
    else 
	{
        SC7A_IIC_SDA_L;
    }
    Ddl_Delay1us(10);
    SC7A_IIC_SCL_H;                  //    P90:TxD_SCL     _________|~~~~~~~~~~
    Ddl_Delay1us(10);

    SC7A_IIC_SCL_L;                //     ~~~~~|______________
    Ddl_Delay1us(15);
}

/****************************************************************************/
/**
  * @brief  I2C获取应答信号
  * @param   
  * @retval 1 表示没有停止信号 
  *         0 表示有停止信号
  */
/****************************************************************************/
uint8_t drv_SC7A_IIC_Checkack(void)
{  
	uint8_t tmp_flag = 0;

    SC7A_IIC_SCL_L;        //SCL    ____________时钟线拉低，才允许改变数据位，防止SDA原来为低电平
    Ddl_Delay1us(15);
    SC7A_IIC_SDA_H;          //SDA  ______/------ 
    Ddl_Delay1us(15);

///*---------------------------------------------------------------------------*/
    drv_mcu_SC7A_IIC_SDA_PIN_SET_INPUT();       //I/O 设置为输入
	
    SC7A_IIC_SCL_H;          //SCL   ------------
    Ddl_Delay1us(15);
	
    if(SC7A_IIC_SdaState) 
	{
        tmp_flag = 1;
    }

    SC7A_IIC_SCL_L;         //SCL   ------\______
    Ddl_Delay1us(15);
	
    drv_mcu_SC7A_IIC_SDA_PIN_SET_OUTPUT();

    return(tmp_flag);
}

uint8_t drv_SC7A_IIC_WaitACK(void)
{
	uint8_t re;
	
	SC7A_IIC_SDA_H;
	Ddl_Delay1us(15);

	SC7A_IIC_SCL_H;
	Ddl_Delay1us(15);

	if(SC7A_IIC_SdaState)
	{
		re = 1; //没有应答
	}
	else
	{
		re = 0; //有应答
	}

	SC7A_IIC_SCL_L;
	Ddl_Delay1us(15);
	return re;
}

void drv_SC7A_IIC_Ack(void)
{
	SC7A_IIC_SDA_L; //SDA拉低
	Ddl_Delay1us(15);
	SC7A_IIC_SCL_H; //SCL拉高
	Ddl_Delay1us(15);
	SC7A_IIC_SCL_L; //SCL拉低
	Ddl_Delay1us(15);
	SC7A_IIC_SDA_H; //SDA拉高
	Ddl_Delay1us(15);
}

void drv_SC7A_IIC_NAck(void)
{
	SC7A_IIC_SDA_H; //SDA拉高
	Ddl_Delay1us(15);

	SC7A_IIC_SCL_H; //SCL拉高
	Ddl_Delay1us(15);

	SC7A_IIC_SCL_L; //SCL拉低
	Ddl_Delay1us(15);
}


/*************** 发送一个字节数据子函数 **********************/
uint8_t drv_SC7A_IIC_Sendbyte(unsigned char *Sdata)
{ 
	char temp,i;
	uint8_t tmp_flag = 1;
	temp = *Sdata;
	for(i=0;i<8;i++)
	{
		
    	if((temp&0x80) == 0x80)
		{
			SC7A_IIC_SDA_H;
		}
		else
		{
			SC7A_IIC_SDA_L;
		}
			
		Ddl_Delay1us(20);
    	SC7A_IIC_SCL_H; 
		Ddl_Delay1us(20);
		SC7A_IIC_SCL_L; 
		
		if(i == 7)
		{
			SC7A_IIC_SDA_H;
		}
		temp = temp<<1;
		Ddl_Delay1us(20);	
	}

	//tmp_flag = drv_SC7A_IIC_Checkack();
    //tmp_flag = (!tmp_flag) & 0x01;

    return(tmp_flag);
}
/*************** 接收一字节子程序 ******************************/
//uint8_t guc_IICRecByte = 0;
uint8_t drv_SC7A_IIC_Recbyte(unsigned  char *Rdata)
{
	char i;    		        // 从SDA线上读取一位数据字节，共8位
	//uint8_t k=0;
	//SC7A_IIC_SDA_H; 
    //Ddl_Delay1us(10);
    //SC7A_IIC_SCL_L;
	//drv_mcu_SC7A_IIC_SDA_PIN_SET_INPUT();		//SDA输入
	*Rdata =0x00;
	uint8_t ucValue = 0;
	//guc_IICRecByte = 0;
	//guc_IICRecByte = 0;
	for(i=0;i<8;i++)
	{
		
    	//*Rdata = *Rdata<<1;
		ucValue = ucValue << 1;
		//guc_IICRecByte = guc_IICRecByte << 1;
		SC7A_IIC_SCL_H;
		Ddl_Delay1us(14);	
    	if(SC7A_IIC_SdaState)
		{
			//*Rdata ++;
			ucValue++;
			//guc_IICRecByte++;
			//guc_IICRecByte |= 0x01;
		}
		//guc_IICRecByte = guc_IICRecByte << 1;
		SC7A_IIC_SCL_L;
		Ddl_Delay1us(15);
	}
	*Rdata = ucValue;
	//drv_mcu_SC7A_IIC_SDA_PIN_SET_OUTPUT();
  	return 0;
}

void drv_SC7A_Write_Byte(uint8_t REG_ADD, uint8_t dat)
{
	drv_SC7A_IIC_Start();					// 发送启动信号
	drv_SC7A_IIC_Sendbyte(&SC7AIICWRITEADDR);		    //发送读地址
	drv_SC7A_IIC_WaitACK();	//等待应答

	drv_SC7A_IIC_Sendbyte(&REG_ADD);		    //发送命令号
	drv_SC7A_IIC_WaitACK();	//等待应答
	drv_SC7A_IIC_Sendbyte(&dat);		    //发送数据
	drv_SC7A_IIC_WaitACK();	//等待应答
	drv_SC7A_IIC_Stop();	//发送停止信号
}

uint8_t drv_SC7A_Read_Byte(uint8_t REG_ADD)
{
	uint8_t res=0;
	//uint8_t tmp_isOK;
	drv_SC7A_IIC_Start();					// 发送启动信号
	drv_SC7A_IIC_Sendbyte(&SC7AIICWRITEADDR);		    //发送读地址
	drv_SC7A_IIC_WaitACK();	//等待应答

	drv_SC7A_IIC_Sendbyte(&REG_ADD);		    //发送命令号
	drv_SC7A_IIC_WaitACK();	//等待应答

	drv_SC7A_IIC_Start();					// 发送启动信号
	drv_SC7A_IIC_Sendbyte(&SC7AIICREADADDR);		    //发送读地址
	drv_SC7A_IIC_WaitACK();	//等待应答

	drv_SC7A_IIC_Recbyte(&res);	//接收数据

	drv_SC7A_IIC_NAck();	//发送非应答信号
	drv_SC7A_IIC_Stop();	//发送停止信号
	
	return res;
}

void drv_SC7A_Write_nByte(uint8_t REG_ADD, uint8_t *pBuff, uint8_t num)
{
	unsigned char i = 0;

	drv_SC7A_IIC_Start();

	drv_SC7A_IIC_Sendbyte(&SC7AIICWRITEADDR);
	drv_SC7A_IIC_WaitACK();
	drv_SC7A_IIC_Sendbyte(&REG_ADD);
	drv_SC7A_IIC_WaitACK();

	for(i = 0; i < num; i++)
	{
		drv_SC7A_IIC_Sendbyte(pBuff);
		drv_SC7A_IIC_WaitACK();
		pBuff++;
	}

//	i2c_WaitAck();

	drv_SC7A_IIC_Stop();
}

void SC7A20TR_Read_nByte(unsigned char REG_ADD, unsigned char *pBuff, unsigned char num)
{
	unsigned char i = 0;

	drv_SC7A_IIC_Start();
	drv_SC7A_IIC_Sendbyte(&SC7AIICWRITEADDR);
	drv_SC7A_IIC_WaitACK();
	drv_SC7A_IIC_Sendbyte(&REG_ADD);
	drv_SC7A_IIC_WaitACK();

	drv_SC7A_IIC_Start();
	drv_SC7A_IIC_Sendbyte(&SC7AIICREADADDR);
	drv_SC7A_IIC_WaitACK();

	for(i = 0; i < num; i++)
	{
		drv_SC7A_IIC_Recbyte(pBuff); //读取数据

		if(i == (num - 1))
		{
			drv_SC7A_IIC_NAck(); //发送非应答信号
		}
		else
		{
			drv_SC7A_IIC_Ack(); //发送应答信号
		}

		pBuff++;
	}

	drv_SC7A_IIC_Stop();
}

uint8_t drv_SC7A_WriteData(uint8_t ucCMD, uint8_t *ucDataArr, uint16_t usDataLen)
{
	uint8_t tmp_isOK;
	drv_SC7A_IIC_Start();					// 发送启动信号
	tmp_isOK = drv_SC7A_IIC_Sendbyte(&SC7AIICWRITEADDR);		    //发送读地址	
	if (0 == tmp_isOK)
	{
		drv_SC7A_IIC_Stop();
		return IICNACK;    			// 若非应答表明器件错误或已坏
	}
	else
	{
		tmp_isOK = drv_SC7A_IIC_Sendbyte(&ucCMD);		    //发送命令号
		if(tmp_isOK == 1)
		{
			while(usDataLen--)
			{ 
				Ddl_Delay1us(1);
				tmp_isOK = drv_SC7A_IIC_Sendbyte(ucDataArr);
				if (0 == tmp_isOK)
				{
					drv_SC7A_IIC_Stop();
					return IICNACK;    			// 若非应答表明器件错误或已坏
				}
				ucDataArr++;
			}
			drv_SC7A_IIC_Stop();
		}
		else
		{
			drv_SC7A_IIC_Stop();
			return IICNACK;    			// 若非应答表明器件错误或已坏
		}
	}
	
	return Successful;
}

uint8_t drv_SC7A_ReadData(uint8_t ucCMD, uint8_t *ucDataArr, uint16_t usDataLen)
{
	uint8_t res=0;
	uint8_t tmp_isOK;
	drv_SC7A_IIC_Start();	
	tmp_isOK = drv_SC7A_IIC_Sendbyte(&SC7AIICWRITEADDR);		 //发送写地址	
	if(0 == tmp_isOK)
	{
		drv_SC7A_IIC_Stop();
		return IICRadAddrNACK ; 
	}	
	tmp_isOK = drv_SC7A_IIC_Sendbyte(&ucCMD);		 //发送命令号	
	if(tmp_isOK == 1)
	{
		drv_SC7A_IIC_Start();	
		tmp_isOK = drv_SC7A_IIC_Sendbyte(&SC7AIICREADADDR);		 //发送读地址
		if(tmp_isOK == 1)
		{
			while(usDataLen--)
			{ 
				res = drv_SC7A_IIC_Recbyte(ucDataArr);
				if(res != Successful )
				{
					drv_SC7A_IIC_Stop();
					return res;
				}
				if(usDataLen > 0)
				{
					drv_SC7A_IIC_ACKorNACK( 0 );  		// 收到一个字节后发送一个应答位
				}
				ucDataArr++;
			}
		}
		else
		{
			drv_SC7A_IIC_Stop();
			return IICRadAddrNACK ; 
		}
	}
	else
	{
		drv_SC7A_IIC_Stop();
		return IICRadAddrNACK ; 
	}
	
	drv_SC7A_IIC_ACKorNACK(1);    		   //收到最后一个字节后发送一个非应答位
	drv_SC7A_IIC_Stop();
	return Successful;
}


/**************************************************
*uint8_t drv_mcu_XYC_ALS_IIC_Init(void)
*入口参数：无
*出口参数：0: init success; 1: init failed
*功能说明：I2C 引脚初始化
***************************************************/
uint8_t drv_mcu_XYC_ALS_IIC_Init(void)  
{
	//初始化RST,SCL,SDA引脚 :RST输出 SDA输出 SCL输出 	
	stc_port_init_t stcPortInit;

    /* configure structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);

    /* Flash NSS */
    stcPortInit.enPinMode = Pin_Mode_Out;
	//stcPortInit.enPinOType = Pin_OType_Od;
    PORT_Init(XYC_ALS_IIC_SDA_PORT, XYC_ALS_IIC_SDA_PIN, &stcPortInit);		
	PORT_Init(XYC_ALS_IIC_SCL_PORT, XYC_ALS_IIC_SCL_PIN, &stcPortInit);
    	  
	XYC_ALS_IIC_SDA_H;//SDA拉低
	XYC_ALS_IIC_SCL_H;//SCL拉高

	return 0;
}

//设置IIC SDA 引脚为输出引脚
void drv_mcu_XYC_ALS_IIC_SDA_PIN_SET_OUTPUT()
{
	//初始化RST,SCL,SDA引脚 :RST输出 SDA输出 SCL输出 	
	stc_port_init_t stcPortInit;

    /* configure structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);
	stcPortInit.enPinMode = Pin_Mode_Out;
	//stcPortInit.enPinOType = Pin_OType_Od;
    PORT_Init(XYC_ALS_IIC_SDA_PORT, XYC_ALS_IIC_SDA_PIN, &stcPortInit);	
}

//设置IIC SDA 引脚为输入引脚
void drv_mcu_XYC_ALS_IIC_SDA_PIN_SET_INPUT()
{
	//初始化RST,SCL,SDA引脚 :RST输出 SDA输出 SCL输出 	
	stc_port_init_t stcPortInit;

    /* configure structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);
	stcPortInit.enPinMode = Pin_Mode_In;
	//stcPortInit.enPinOType = Pin_OType_Od;
    PORT_Init(XYC_ALS_IIC_SDA_PORT, XYC_ALS_IIC_SDA_PIN, &stcPortInit);	
}

/*********************IIC停止信号***********************/
void drv_XYC_ALS_IIC_Stop(void)
{	
	XYC_ALS_IIC_SCL_L;  //
	Ddl_Delay1us(15);
	XYC_ALS_IIC_SDA_L;
	Ddl_Delay1us(15);
	XYC_ALS_IIC_SCL_H;
	Ddl_Delay1us(15);
	XYC_ALS_IIC_SDA_H;
	Ddl_Delay1us(15);
}

/*********************IIC起始信号***********************/
void drv_XYC_ALS_IIC_Start(void)
{
	XYC_ALS_IIC_SDA_H;
	Ddl_Delay1us(15);
	XYC_ALS_IIC_SCL_H;
	Ddl_Delay1us(15);	
	XYC_ALS_IIC_SDA_L;//SDA拉低
	Ddl_Delay1us(15);
	XYC_ALS_IIC_SCL_L;//SCL拉低
	Ddl_Delay1us(15);	
	
}

/************MCU向XYC_ALS发送应答/非应答信号**********************/
void drv_XYC_ALS_IIC_ACKorNACK(uint8_t uAck)
{	
	Ddl_Delay1us(14);
	XYC_ALS_IIC_SCL_H;//SCL拉高
	Ddl_Delay1us(14);	
	XYC_ALS_IIC_SCL_L;//SCL拉低
	XYC_ALS_IIC_SDA_H;      //SDA拉高

	XYC_ALS_IIC_SCL_L;        //SCL    ____________时钟线拉低，才允许改变数据位，防止SDA原来为低电平
    Ddl_Delay1us(15);	
    
    if(uAck==1) 
	{
        XYC_ALS_IIC_SDA_H;
    } 
    else 
	{
        XYC_ALS_IIC_SDA_L;
    }
    Ddl_Delay1us(10);
    XYC_ALS_IIC_SCL_H;                  //    P90:TxD_SCL     _________|~~~~~~~~~~
    Ddl_Delay1us(10);

    XYC_ALS_IIC_SCL_L;                //     ~~~~~|______________
    Ddl_Delay1us(15);
}

/****************************************************************************/
/**
  * @brief  I2C获取应答信号
  * @param   
  * @retval 1 表示没有停止信号 
  *         0 表示有停止信号
  */
/****************************************************************************/
uint8_t drv_XYC_ALS_IIC_Checkack(void)
{  
	uint8_t tmp_flag = 0;

    XYC_ALS_IIC_SCL_L;        //SCL    ____________时钟线拉低，才允许改变数据位，防止SDA原来为低电平
    Ddl_Delay1us(1);
    XYC_ALS_IIC_SDA_H;          //SDA  ______/------ 
    Ddl_Delay1us(1);

///*---------------------------------------------------------------------------*/
    drv_mcu_XYC_ALS_IIC_SDA_PIN_SET_INPUT();       //I/O 设置为输入
	
    XYC_ALS_IIC_SCL_H;          //SCL   ------------
    Ddl_Delay1us(1);
	
    if(XYC_ALS_IIC_SdaState) 
	{
        tmp_flag = 1;
    }

    XYC_ALS_IIC_SCL_L;         //SCL   ------\______
    Ddl_Delay1us(1);
	
    drv_mcu_XYC_ALS_IIC_SDA_PIN_SET_OUTPUT();

    return(tmp_flag);
}

/*************** 发送一个字节数据子函数 **********************/
uint8_t drv_XYC_ALS_IIC_Sendbyte(unsigned char *Sdata)
{ 
	char temp,i;
	uint8_t tmp_flag = 0;
	temp = *Sdata;
	for(i=0;i<8;i++)
	{
		XYC_ALS_IIC_SCL_L; 
		Ddl_Delay1us(1);	
    	if((temp&0x80) == 0x80)
		{
			XYC_ALS_IIC_SDA_H;
		}
		else
		{
			XYC_ALS_IIC_SDA_L;
		}
			
		Ddl_Delay1us(1);
    	XYC_ALS_IIC_SCL_H; 
		Ddl_Delay1us(1);
		temp = temp<<1;
	}

	tmp_flag = drv_XYC_ALS_IIC_Checkack();
    tmp_flag = (!tmp_flag) & 0x01;

    return(tmp_flag);
}
/*************** 接收一字节子程序 ******************************/
uint8_t drv_XYC_ALS_IIC_Recbyte(unsigned  char *Rdata)
{
	char i;    		        // 从SDA线上读取一位数据字节，共8位
	uint8_t k=0;
	XYC_ALS_IIC_SDA_H;
    Ddl_Delay1us(10);
    XYC_ALS_IIC_SCL_L;
	drv_mcu_XYC_ALS_IIC_SDA_PIN_SET_INPUT();		//SDA输入
	*Rdata =0x00;
	for(i=0;i<8;i++)
	{
		XYC_ALS_IIC_SCL_H;
		Ddl_Delay1us(14);	
		while(!XYC_ALS_IIC_SclState)
		{
			if(k>30)
			{
				return SCL_TimeOut;
			} 
			k++;
			Ddl_Delay1us(10);
		}
		Ddl_Delay1us(14);	
    	*Rdata = *Rdata<<1;		
    	if(XYC_ALS_IIC_SdaState)
		{
			*Rdata |=0x01;
		}
		XYC_ALS_IIC_SCL_L;
		Ddl_Delay1us(15);
	}
	
	drv_mcu_XYC_ALS_IIC_SDA_PIN_SET_OUTPUT();
  	return 0;
}

uint8_t drv_XYC_ALS_WriteData(uint8_t ucCMD, uint8_t *ucDataArr, uint16_t usDataLen)
{
	uint8_t tmp_isOK;
	drv_XYC_ALS_IIC_Start();					// 发送启动信号
	tmp_isOK = drv_XYC_ALS_IIC_Sendbyte(&XYC_ALSIICWRITEADDR);		    //发送读地址	
	if (0 == tmp_isOK)
	{
		drv_XYC_ALS_IIC_Stop();
		return IICNACK;    			// 若非应答表明器件错误或已坏
	}
	else
	{
		tmp_isOK = drv_XYC_ALS_IIC_Sendbyte(&ucCMD);		    //发送命令号
		if(tmp_isOK == 1)
		{
			while(usDataLen--)
			{ 
				Ddl_Delay1us(1);
				tmp_isOK = drv_XYC_ALS_IIC_Sendbyte(ucDataArr);
				if (0 == tmp_isOK)
				{
					drv_XYC_ALS_IIC_Stop();
					return IICNACK;    			// 若非应答表明器件错误或已坏
				}
				ucDataArr++;
			}
			drv_XYC_ALS_IIC_Stop();
		}
		else
		{
			drv_XYC_ALS_IIC_Stop();
			return IICNACK;    			// 若非应答表明器件错误或已坏
		}
	}
	
	return Successful;
}

uint8_t drv_XYC_ALS_ReadData(uint8_t ucCMD, uint8_t *ucDataArr, uint16_t usDataLen)
{
	uint8_t res=0;
	uint8_t tmp_isOK;
	drv_XYC_ALS_IIC_Start();	
	tmp_isOK = drv_XYC_ALS_IIC_Sendbyte(&XYC_ALSIICWRITEADDR);		 //发送写地址	
	if(0 == tmp_isOK)
	{
		drv_XYC_ALS_IIC_Stop();
		return IICRadAddrNACK ; 
	}	
	tmp_isOK = drv_XYC_ALS_IIC_Sendbyte(&ucCMD);		 //发送命令号	
	if(tmp_isOK == 1)
	{
		drv_XYC_ALS_IIC_Start();	
		tmp_isOK = drv_XYC_ALS_IIC_Sendbyte(&XYC_ALSIICREADADDR);		 //发送读地址
		if(tmp_isOK == 1)
		{
			while(usDataLen--)
			{ 
				res = drv_XYC_ALS_IIC_Recbyte(ucDataArr);
				if(res != Successful )
				{
					drv_XYC_ALS_IIC_Stop();
					return res;
				}
				if(usDataLen > 0)
				{
					drv_XYC_ALS_IIC_ACKorNACK( 0 );  		// 收到一个字节后发送一个应答位
				}
				ucDataArr++;
			}
		}
		else
		{
			drv_XYC_ALS_IIC_Stop();
			return IICRadAddrNACK ; 
		}
	}
	else
	{
		drv_XYC_ALS_IIC_Stop();
		return IICRadAddrNACK ; 
	}
	
	drv_XYC_ALS_IIC_ACKorNACK(1);    		   //收到最后一个字节后发送一个非应答位
	drv_XYC_ALS_IIC_Stop();
	return Successful;
}

/**************************************************
*uint8_t drv_mcu_OLED_IIC_Init(void)
*入口参数：无
*出口参数：0: init success; 1: init failed
*功能说明：I2C 引脚初始化
***************************************************/
uint8_t drv_mcu_OLED_IIC_Init(void)  
{
	//初始化RST,SCL,SDA引脚 :RST输出 SDA输出 SCL输出 	
	stc_port_init_t stcPortInit;

    /* configure structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);

    /* Flash NSS */
    stcPortInit.enPinMode = Pin_Mode_Out;
	//stcPortInit.enPinOType = Pin_OType_Od;
    PORT_Init(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN, &stcPortInit);		
	PORT_Init(OLED_IIC_SCL_PORT, OLED_IIC_SCL_PIN, &stcPortInit);
    	  
	OLED_IIC_SDA_H;//SDA拉低
	OLED_IIC_SCL_H;//SCL拉高

	return 0;
}

//设置IIC SDA 引脚为输出引脚
void drv_mcu_OLED_IIC_SDA_PIN_SET_OUTPUT()
{
	//初始化RST,SCL,SDA引脚 :RST输出 SDA输出 SCL输出 	
	stc_port_init_t stcPortInit;

    /* configure structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);
	stcPortInit.enPinMode = Pin_Mode_Out;
	//stcPortInit.enPinOType = Pin_OType_Od;
    PORT_Init(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN, &stcPortInit);	
}

//设置IIC SDA 引脚为输入引脚
void drv_mcu_OLED_IIC_SDA_PIN_SET_INPUT()
{
	//初始化RST,SCL,SDA引脚 :RST输出 SDA输出 SCL输出 	
	stc_port_init_t stcPortInit;

    /* configure structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);
	stcPortInit.enPinMode = Pin_Mode_In;
	//stcPortInit.enPinOType = Pin_OType_Od;
    PORT_Init(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN, &stcPortInit);	
}

/*********************IIC停止信号***********************/
void drv_OLED_IIC_Stop(void)
{	
	OLED_IIC_SCL_L;  //
	Ddl_Delay1us(8);
	OLED_IIC_SDA_L;
	Ddl_Delay1us(8);
	OLED_IIC_SCL_H;
	Ddl_Delay1us(8);
	OLED_IIC_SDA_H;
}

/*********************IIC起始信号***********************/
void drv_OLED_IIC_Start(void)
{
	OLED_IIC_SDA_H;
	Ddl_Delay1us(8);
	OLED_IIC_SCL_H;
	Ddl_Delay1us(8);	
	OLED_IIC_SCL_L;//SCL拉低
	Ddl_Delay1us(8);	
	OLED_IIC_SDA_L;//SDA拉低
	Ddl_Delay1us(8);
}

/************MCU向XYC_ALS发送应答/非应答信号**********************/
void drv_OLED_IIC_ACKorNACK(uint8_t uAck)
{	
	Ddl_Delay1us(7);
	OLED_IIC_SCL_H;//SCL拉高
	Ddl_Delay1us(7);	
	OLED_IIC_SCL_L;//SCL拉低
	OLED_IIC_SDA_H;      //SDA拉高

	OLED_IIC_SCL_L;        //SCL    ____________时钟线拉低，才允许改变数据位，防止SDA原来为低电平
    Ddl_Delay1us(8);	
    
    if(uAck==1) 
	{
        OLED_IIC_SDA_H;
    } 
    else 
	{
        OLED_IIC_SDA_L;
    }
    Ddl_Delay1us(13);
    OLED_IIC_SCL_H;                  //    P90:TxD_SCL     _________|~~~~~~~~~~
    Ddl_Delay1us(13);

    OLED_IIC_SCL_L;                //     ~~~~~|______________
    Ddl_Delay1us(8);
}

/****************************************************************************/
/**
  * @brief  I2C获取应答信号
  * @param   
  * @retval 1 表示没有停止信号 
  *         0 表示有停止信号
  */
/****************************************************************************/
uint8_t drv_OLED_IIC_Checkack(void)
{  
	uint8_t tmp_flag = 0;

    OLED_IIC_SCL_L;        //SCL    ____________时钟线拉低，才允许改变数据位，防止SDA原来为低电平
    Ddl_Delay1us(8);
    OLED_IIC_SDA_H;          //SDA  ______/------ 
    Ddl_Delay1us(8);

///*---------------------------------------------------------------------------*/
    drv_mcu_OLED_IIC_SDA_PIN_SET_INPUT();       //I/O 设置为输入
	
    OLED_IIC_SCL_H;          //SCL   ------------
    Ddl_Delay1us(8);
	
    if(OLED_IIC_SdaState) 
	{
        tmp_flag = 1;
    }

    OLED_IIC_SCL_L;         //SCL   ------\______
    Ddl_Delay1us(8);
	
    drv_mcu_OLED_IIC_SDA_PIN_SET_OUTPUT();

    return(tmp_flag);
}

/*************** 发送一个字节数据子函数 **********************/
uint8_t drv_OLED_IIC_Sendbyte(unsigned char Sdata)
{ 
	char temp,i;
	uint8_t tmp_flag = 0;
	temp = Sdata;
	for(i=0;i<8;i++)
	{
		OLED_IIC_SCL_L; 
		Ddl_Delay1us(4);	
    	if((temp&0x80) == 0x80)
		{
			OLED_IIC_SDA_H;
		}
		else
		{
			OLED_IIC_SDA_L;
		}
			
		Ddl_Delay1us(4);
    	OLED_IIC_SCL_H; 
		Ddl_Delay1us(8);
		temp = temp<<1;
	}

	tmp_flag = drv_OLED_IIC_Checkack();
    tmp_flag = (!tmp_flag) & 0x01;

    return(tmp_flag);
}

/**
 ******************************************************************************
 ** \brief  Master transmit data
 **
 ** \param  u16DevAddr            The slave address
 ** \param  pu8TxData             Pointer to the data buffer
 ** \param  u32Size               Data size
 ** \param  u32TimeOut            Time out count
 ** \retval en_result_t           Enumeration value:
 **         - Ok:                 Success
 **         - ErrorTimeout:       Time out
 ******************************************************************************/
 en_result_t I2C_XYC_ALS_Master_Transmit(uint16_t u16DevAddr, uint8_t *pu8TxData, uint32_t u32Size, uint32_t u32TimeOut)
{
    en_result_t enRet;
    I2C_Cmd(I2C_XYC_ALS_UNIT, Enable);

    I2C_SoftwareResetCmd(I2C_XYC_ALS_UNIT, Enable);
    I2C_SoftwareResetCmd(I2C_XYC_ALS_UNIT, Disable);
    enRet = I2C_Start(I2C_XYC_ALS_UNIT,u32TimeOut);
    if(Ok == enRet)
    {
        enRet = I2C_TransAddr(I2C_XYC_ALS_UNIT, (uint8_t)u16DevAddr, I2CDirTrans, u32TimeOut);

        if(Ok == enRet)
        {
            enRet = I2C_TransData(I2C_XYC_ALS_UNIT, pu8TxData, u32Size,u32TimeOut);
        }
    }

    I2C_Stop(I2C_XYC_ALS_UNIT,u32TimeOut);
    I2C_Cmd(I2C_XYC_ALS_UNIT, Disable);

    return enRet;
}

/**
 ******************************************************************************
 ** \brief  Master receive data
 **
 ** \param  u16DevAddr            The slave address
 ** \param  pu8RxData             Pointer to the data buffer
 ** \param  u32Size               Data size
 ** \param  u32TimeOut            Time out count
 ** \retval en_result_t           Enumeration value:
 **         - Ok:                 Success
 **         - ErrorTimeout:       Time out
 ******************************************************************************/
 en_result_t I2C_XYC_ALS_Master_Receive(uint16_t u16DevAddr, uint8_t ucCMD,uint8_t *pu8RxData, uint32_t u32Size, uint32_t u32TimeOut)
{
    en_result_t enRet;

    I2C_Cmd(I2C_XYC_ALS_UNIT, Enable);
    I2C_SoftwareResetCmd(I2C_XYC_ALS_UNIT, Enable);
    I2C_SoftwareResetCmd(I2C_XYC_ALS_UNIT, Disable);
    enRet = I2C_Start(I2C_XYC_ALS_UNIT,u32TimeOut);
    if(Ok == enRet)
    {
        enRet = I2C_TransAddr(I2C_XYC_ALS_UNIT, (uint8_t)u16DevAddr, I2CDirTrans, u32TimeOut);

        if(Ok == enRet)
        {
            enRet = I2C_TransData(I2C_XYC_ALS_UNIT, (uint8_t *)&ucCMD, 1, u32TimeOut);
            if(Ok == enRet)
            {
                enRet = I2C_Restart(I2C_XYC_ALS_UNIT,u32TimeOut);
                if(Ok == enRet)
                {
                    if(1ul == u32Size)
                    {
                        I2C_AckConfig(I2C_XYC_ALS_UNIT, I2c_NACK);
                    }

                    enRet = I2C_TransAddr(I2C_XYC_ALS_UNIT, (uint8_t)u16DevAddr, I2CDirReceive, u32TimeOut);
                    if(Ok == enRet)
                    {
                        enRet = I2C_MasterDataReceiveAndStop(I2C_XYC_ALS_UNIT, pu8RxData, u32Size, u32TimeOut);
                    }

                    I2C_AckConfig(I2C_XYC_ALS_UNIT, I2c_ACK);
                }
            }
        }
    }

    if(Ok != enRet)
    {
        I2C_Stop(I2C_XYC_ALS_UNIT,u32TimeOut);
    }
    I2C_Cmd(I2C_XYC_ALS_UNIT, Disable);
    return enRet;
}


/**
 ******************************************************************************
 ** \brief   Initialize the I2C peripheral for master
 ** \param   None
 ** \retval en_result_t                Enumeration value:
 **          - Ok:                     Success
 **          - ErrorInvalidParameter:  Invalid parameter
 ******************************************************************************/
static en_result_t drv_XYC_ALS_IIC_Master_Initialize(void)
{
    en_result_t enRet;
    stc_i2c_init_t stcI2cInit;
    float32_t fErr;

    I2C_DeInit(I2C_XYC_ALS_UNIT);

    MEM_ZERO_STRUCT(stcI2cInit);
    stcI2cInit.u32ClockDiv = I2C_CLK_DIV2;
    stcI2cInit.u32Baudrate = I2C_XYC_ALS_BAUDRATE;
    stcI2cInit.u32SclTime = 0ul;
    enRet = I2C_Init(I2C_XYC_ALS_UNIT, &stcI2cInit, &fErr);

    I2C_BusWaitCmd(I2C_XYC_ALS_UNIT, Enable);

    return enRet;
}

uint8_t drv_XYC_ALS_HARTIIC_Init(void)
{
	PORT_SetFunc(XYC_ALS_IIC_SCL_PORT, XYC_ALS_IIC_SCL_PIN, I2C_GPIO_XYC_ALS_SCL_FUNC, Disable);
    PORT_SetFunc(XYC_ALS_IIC_SDA_PORT, XYC_ALS_IIC_SDA_PIN, I2C_GPIO_XYC_ALS_SDA_FUNC, Disable);

    /* Enable I2C Peripheral*/
    PWC_Fcg1PeriphClockCmd(I2C_XYC_ALS_FCG_USE, Enable);
    /* Initialize I2C peripheral and enable function*/
    drv_XYC_ALS_IIC_Master_Initialize();

	return 0;
}

/**
 ******************************************************************************
 ** \brief  Master transmit data
 **
 ** \param  u16DevAddr            The slave address
 ** \param  pu8TxData             Pointer to the data buffer
 ** \param  u32Size               Data size
 ** \param  u32TimeOut            Time out count
 ** \retval en_result_t           Enumeration value:
 **         - Ok:                 Success
 **         - ErrorTimeout:       Time out
 ******************************************************************************/
en_result_t I2C_SC7A20_Master_Transmit(uint16_t u16DevAddr, uint8_t *pu8TxData, uint32_t u32Size, uint32_t u32TimeOut)
{
    en_result_t enRet;
    I2C_Cmd(I2C_SC7A20_UNIT, Enable);

    I2C_SoftwareResetCmd(I2C_SC7A20_UNIT, Enable);
    I2C_SoftwareResetCmd(I2C_SC7A20_UNIT, Disable);
    enRet = I2C_Start(I2C_SC7A20_UNIT,u32TimeOut);
    if(Ok == enRet)
    {
        enRet = I2C_TransAddr(I2C_SC7A20_UNIT, (uint8_t)u16DevAddr, I2CDirTrans, u32TimeOut);

        if(Ok == enRet)
        {
            enRet = I2C_TransData(I2C_SC7A20_UNIT, pu8TxData, u32Size,u32TimeOut);
        }
    }

    I2C_Stop(I2C_SC7A20_UNIT,u32TimeOut);
    I2C_Cmd(I2C_SC7A20_UNIT, Disable);

    return enRet;
}

/**
 ******************************************************************************
 ** \brief  Master receive data
 **
 ** \param  u16DevAddr            The slave address
 ** \param  pu8RxData             Pointer to the data buffer
 ** \param  u32Size               Data size
 ** \param  u32TimeOut            Time out count
 ** \retval en_result_t           Enumeration value:
 **         - Ok:                 Success
 **         - ErrorTimeout:       Time out
 ******************************************************************************/
en_result_t I2C_SC7A20_Master_Receive(uint16_t u16DevAddr, uint8_t ucCMD,uint8_t *pu8RxData, uint32_t u32Size, uint32_t u32TimeOut)
{
    en_result_t enRet;

    I2C_Cmd(I2C_SC7A20_UNIT, Enable);
    I2C_SoftwareResetCmd(I2C_SC7A20_UNIT, Enable);
    I2C_SoftwareResetCmd(I2C_SC7A20_UNIT, Disable);
    enRet = I2C_Start(I2C_SC7A20_UNIT,u32TimeOut);
    if(Ok == enRet)
    {
        enRet = I2C_TransAddr(I2C_SC7A20_UNIT, (uint8_t)u16DevAddr, I2CDirTrans, u32TimeOut);

        if(Ok == enRet)
        {
            enRet = I2C_TransData(I2C_SC7A20_UNIT, (uint8_t *)&ucCMD, 1, u32TimeOut);
            if(Ok == enRet)
            {
                enRet = I2C_Restart(I2C_SC7A20_UNIT,u32TimeOut);
                if(Ok == enRet)
                {
                    if(1ul == u32Size)
                    {
                        I2C_AckConfig(I2C_SC7A20_UNIT, I2c_NACK);
                    }

                    enRet = I2C_TransAddr(I2C_SC7A20_UNIT, (uint8_t)u16DevAddr, I2CDirReceive, u32TimeOut);
                    if(Ok == enRet)
                    {
                        enRet = I2C_MasterDataReceiveAndStop(I2C_SC7A20_UNIT, pu8RxData, u32Size, u32TimeOut);
                    }

                    I2C_AckConfig(I2C_SC7A20_UNIT, I2c_ACK);
                }
            }
        }
    }

    if(Ok != enRet)
    {
        I2C_Stop(I2C_SC7A20_UNIT,u32TimeOut);
    }
    I2C_Cmd(I2C_SC7A20_UNIT, Disable);
    return enRet;
}

/**
 ******************************************************************************
 ** \brief   Initialize the I2C peripheral for master
 ** \param   None
 ** \retval en_result_t                Enumeration value:
 **          - Ok:                     Success
 **          - ErrorInvalidParameter:  Invalid parameter
 ******************************************************************************/
static en_result_t drv_SC7A20_IIC_Master_Initialize(void)
{
    en_result_t enRet;
    stc_i2c_init_t stcI2cInit;
    float32_t fErr;

    I2C_DeInit(I2C_SC7A20_UNIT);

    MEM_ZERO_STRUCT(stcI2cInit);
    stcI2cInit.u32ClockDiv = I2C_CLK_DIV8;
    stcI2cInit.u32Baudrate = I2C_SC7A20_BAUDRATE;
    stcI2cInit.u32SclTime = 0ul;
    enRet = I2C_Init(I2C_SC7A20_UNIT, &stcI2cInit, &fErr);

    I2C_BusWaitCmd(I2C_SC7A20_UNIT, Enable);

    return enRet;
}

uint8_t drv_SC7A20_HARTIIC_Init(void)
{
	PORT_SetFunc(SC7A_IIC_SCL_PORT, SC7A_IIC_SCL_PIN, I2C_GPIO_SC7A20_SCL_FUNC, Disable);
    PORT_SetFunc(SC7A_IIC_SDA_PORT, SC7A_IIC_SDA_PIN, I2C_GPIO_SC7A20_SDA_FUNC, Disable);

    /* Enable I2C Peripheral*/
    PWC_Fcg1PeriphClockCmd(I2C_SC7A20_FCG_USE, Enable);
    /* Initialize I2C peripheral and enable function*/
    drv_SC7A20_IIC_Master_Initialize();

	return 0;
}
