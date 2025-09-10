/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\driver\drv_device\drv_SC7A20H\drv_SC7A20H.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-21       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "drv_SC7A20H.h"
#include "stdio.h"
#include "string.h"
#include "mainloop.h"
#include "hc32f460_exint_nmi_swi.h"
#include "hc32f460_interrupts.h"
#include "math.h"

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
static SystemPataSt *pst_SC7A20SystemPara;
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
  /**
 *******************************************************************************
 ** \brief ExtInt08 callback function
 **
 ** \param  None
 **
 ** \retval None
 **
 ******************************************************************************/
uint16_t gus_ExtiCh04 = 0;
void ExtInt04_Callback(void)
{
    if (Set == EXINT_IrqFlgGet(ExtiCh04))
    {
		gus_ExtiCh04++;
		//func_SC7A20H_Read_FIFO_Buf(&pst_SC7A20SystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A[0],&pst_SC7A20SystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A[1],&pst_SC7A20SystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A[2]);
		pst_SC7A20SystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A = SC7A20_Task();
        
        /* clear int request flag */
        EXINT_IrqFlgClr(ExtiCh04);
    }
}
/**
*******************************************************************************
** \brief SC7A20H IIC Init function
**
** \param [in]  None
**
** \retval 0: init success; 1: init failed.
**
** \note 
******************************************************************************/
uint8_t drv_Attitude_Sensor_SC7A20H_Init(void)
{
	uint8_t ucRes = 0;
    uint8_t id1=0;
    //uint8_t id2=0;
	//uint8_t ucDataValue = 0;
	//uint8_t ucDataArr[2] = {0};
	//配置为中断模式
	//uint8_t writeValue[5] = {0x47,0x00,0x10,0x88,0x00};
	//配置为轮询模式
	uint8_t writeValue[5] = {0x47,0x00,0x10,0x88,0x00};
	uint8_t readValue[4] = {0};
	//stc_exint_config_t stcExtiConfig;
	//stc_irq_regi_conf_t stcIrqRegiConf;
    //stc_port_init_t stcPortInit;

	pst_SC7A20SystemPara = GetSystemPara();

	#if 1
	ucRes = drv_mcu_SC7A_IIC_Init();

	//drv_SC7A_ReadData(0x0F, &id1, 1);
	Ddl_Delay1ms(10);

	//drv_SC7A_ReadData(0x70, &id2, 1);

	id1 = drv_SC7A_Read_Byte(0x0F);
	//id2 = drv_SC7A_Read_Byte(0x70);
	drv_SC7A_Write_nByte(0xA0,&writeValue[0],5);
	Ddl_Delay1ms(100);
	SC7A20TR_Read_nByte(0xA0,readValue, 5);
	#if 0	//配置为中断模式
	ucDataValue = 0;
	drv_SC7A_Write_nByte (0x25, &ucDataValue, 1);
	#endif
	#else
	drv_SC7A20_HARTIIC_Init();
	Ddl_Delay1ms(10);

	I2C_SC7A20_Master_Receive(DEVICE_SC7A20_ADDRESS,0x0F, &id1, 1, TIMEOUT);
	I2C_SC7A20_Master_Receive(DEVICE_SC7A20_ADDRESS,0x70, &id2, 1, TIMEOUT);

	I2C_SC7A20_Master_Transmit(DEVICE_SC7A20_ADDRESS,writeValue,5, TIMEOUT);
	Ddl_Delay1ms(100);
	I2C_SC7A20_Master_Receive(DEVICE_SC7A20_ADDRESS,0xA2,readValue, 4, TIMEOUT);
	#endif

	

    //if ((id1 != 0x11) || (id2 != 0x28))
	if(id1 != 0x11)
    {
		return 1;
    }
	#if 0
	ucDataValue = 0x77;
	drv_SC7A_WriteData(0x20, &ucDataValue, 1);
	//50Hz+正常模式xyz使能
	ucDataValue = 0X88;
	drv_SC7A_WriteData(0x21, &ucDataValue, 1);
	//关闭滤波器，手册上面没有滤波器截止频率设置说明，开启后无法测量静止状态下的重力加            速度
	ucDataValue = 0x10;
	drv_SC7A_WriteData(0x22, &ucDataValue, 1);
		//配置数据准备就绪时触发中断
	ucDataValue = 0x88;
	drv_SC7A_WriteData (0x23, &ucDataValue, 1);
		//读取完成再更新，小端模式，、2g+正常模式，高精度模式
	ucDataValue = 0;
	drv_SC7A_WriteData (0x25, &ucDataValue, 1);
	#else
	#if 0
	ucDataValue = 0x77;
	ucDataArr[0] = 0x20;	//CTRL_REG1
	ucDataArr[1] = ucDataValue;	//50Hz+正常模式xyz使能
	I2C_SC7A20_Master_Transmit(DEVICE_SC7A20_ADDRESS,ucDataArr,2, TIMEOUT);  //400Hz+正常模式xyz使能
	ucDataValue = 0x88;
	ucDataArr[0] = 0x21;	//CTRL_REG2
	ucDataArr[1] = ucDataValue;	
	I2C_SC7A20_Master_Transmit(DEVICE_SC7A20_ADDRESS,ucDataArr,2, TIMEOUT); //高通滤波：正常模式,00+滤波使能
	ucDataArr[0] = 0x22;	//CTRL_REG3
	ucDataArr[1] = 0x10;	
	I2C_SC7A20_Master_Transmit(DEVICE_SC7A20_ADDRESS,ucDataArr,2, TIMEOUT);	//配置数据准备就绪时触发中断
	ucDataArr[0] = 0x23;
	ucDataArr[1] = 0x88;	
	I2C_SC7A20_Master_Transmit(DEVICE_SC7A20_ADDRESS,ucDataArr,2, TIMEOUT);  //读取完成再更新、2g+正常模式
	ucDataArr[0] = 0x25;
	ucDataArr[1] = 0x00;	
	I2C_SC7A20_Master_Transmit(DEVICE_SC7A20_ADDRESS,ucDataArr,2, TIMEOUT); //INT中断为高电平
	#endif
	#endif
	#if 0	//中断配置
	MEM_ZERO_STRUCT(stcExtiConfig);
    MEM_ZERO_STRUCT(stcIrqRegiConf);
    MEM_ZERO_STRUCT(stcPortInit);

	
	    /**************************************************************************/
    /* External Int Ch.7                                                      */
    /**************************************************************************/
    stcExtiConfig.enExitCh = ExtiCh04;

    /* Filter setting */
    stcExtiConfig.enFilterEn = Enable;
    stcExtiConfig.enFltClk = Pclk3Div8;
    stcExtiConfig.enExtiLvl = ExIntRisingEdge;
    EXINT_Init(&stcExtiConfig);

    /* Set External Int */
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enExInt = Enable;
    PORT_Init(SCINT1_GPIO_PORT, SCINT1_GPIO_PIN, &stcPortInit);

    /* Select External Int Ch.7 */
    stcIrqRegiConf.enIntSrc = INT_PORT_EIRQ4;

    /* Register External Int to Vect.No.000 */
    stcIrqRegiConf.enIRQn = Int003_IRQn;

    /* Callback function */
    stcIrqRegiConf.pfnCallback = &ExtInt04_Callback;

    /* Registration IRQ */
    enIrqRegistration(&stcIrqRegiConf);

    /* Clear pending */
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);

    /* Set priority */
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);

    /* Enable NVIC */
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
	#endif
	return ucRes;
}

/*
SC7A20获取加速度值
accd_lsb：加速度值低寄存器地址
accd_msb：加速度值高寄存器地址
返回值accd_f：加速度值，单位mg
*/
int SC7A20_Get_ACCD(uint8_t accd_lsb,uint8_t accd_msb)
{
	uint8_t accd_l,accd_m = 0;
	unsigned int temp = 0;
	int accd;
	#if 0
	drv_SC7A_ReadData(accd_lsb,&accd_l,1);
	drv_SC7A_ReadData(accd_msb,&accd_m,1);
	#else
	I2C_SC7A20_Master_Receive(DEVICE_SC7A20_ADDRESS,accd_lsb, &accd_l, 1, TIMEOUT);
	I2C_SC7A20_Master_Receive(DEVICE_SC7A20_ADDRESS,accd_msb, &accd_m, 1, TIMEOUT);
	#endif
	temp &= 0x0000;
	temp |= accd_m;
	temp <<= 8;
	temp &= 0xff00;
	temp |= accd_l;	
	if(temp&0x8000)
	{
		temp >>= 4;
		temp |= 0xf000;
	}
	else
	{
		temp >>= 4;
		temp &= 0x0fff;
	}
	accd = temp*1;
	return accd;
}


//读取姿态传感器数据
uint8_t func_SC7A20H_Read_FIFO_Buf(int *x_buf,int *y_buf,int *z_buf)
{
    unsigned char  i=0;
	signed int x =0;
	signed int y = 0;
	signed int z = 0;
	unsigned char  sc7a20_data[7] = {0};
	#if 0
    
    unsigned char  SL_FIFO_ACCEL_NUM;
    uint8_t t3=0x4F;
	uint8_t ucDataArr[2] = {0};
	uint16_t usValue = 0;
	

    drv_SC7A_ReadData(0x2F,  &SL_FIFO_ACCEL_NUM, 1);
	//I2C_SC7A20_Master_Receive(DEVICE_SC7A20_ADDRESS,0x2F, &SL_FIFO_ACCEL_NUM, 1, TIMEOUT);

    if (SL_FIFO_ACCEL_NUM & 0x40)
	{
		SL_FIFO_ACCEL_NUM = 32;
	}
    else
	{
		SL_FIFO_ACCEL_NUM = SL_FIFO_ACCEL_NUM&0x1f;
	}
        
    for(i=0;i<3;i++)
    {
		#if 1
		drv_SC7A_ReadData( 0x27,  &sc7a20_data[0], 1);
        drv_SC7A_ReadData( 0xA8,  &sc7a20_data[1], 6);
		#else
		I2C_SC7A20_Master_Receive(DEVICE_SC7A20_ADDRESS,0x27, &sc7a20_data[0], 1, TIMEOUT);
		I2C_SC7A20_Master_Receive(DEVICE_SC7A20_ADDRESS,0xA8, &sc7a20_data[1], 6, TIMEOUT);
		#endif
		usValue = (sc7a20_data[2] << 8) | sc7a20_data[1];
		x += (signed short int)usValue;
		usValue = (sc7a20_data[4] << 8) | sc7a20_data[3];
		y += (signed short int)usValue;
		usValue = (sc7a20_data[6] << 8) | sc7a20_data[5];
		z += (signed short int)usValue;
		Ddl_Delay1ms(10);
        //x +=(signed short int)(((unsigned char)sc7a20_data[2] * 256 ) + (unsigned char)sc7a20_data[1]);
        //y +=(signed short int)(((unsigned char)sc7a20_data[4] * 256 ) + (unsigned char)sc7a20_data[3]);
        //z +=(signed short int)(((unsigned char)sc7a20_data[6] * 256 ) + (unsigned char)sc7a20_data[5]);

    }
	pst_SC7A20SystemPara->DeviceRunPara.esDeviceSensorsData.cDev_Attitude_SC7A_Status = sc7a20_data[0];
	x /= 3;
	y /= 3;
	z /= 3;

	*x_buf = x;
	*y_buf = y;
	*z_buf = z;

	ucDataArr[0] = 0X2E;	
	ucDataArr[1] = 0X00;	
	#if 1
	drv_SC7A_WriteData( 0X2E, 0X00, 1);
	drv_SC7A_WriteData( 0X2E, &t3, 1);
	#else
	I2C_SC7A20_Master_Transmit(DEVICE_SC7A20_ADDRESS,ucDataArr,2, TIMEOUT);
	ucDataArr[1] = t3;	
	I2C_SC7A20_Master_Transmit(DEVICE_SC7A20_ADDRESS,ucDataArr,2, TIMEOUT);
	#endif
	return SL_FIFO_ACCEL_NUM;
	#else
	
	//drv_SC7A_ReadData( 0x27,  &sc7a20_data[0], 1);
	I2C_SC7A20_Master_Receive(DEVICE_SC7A20_ADDRESS,0x27, &sc7a20_data[0], 1, TIMEOUT);
	for(i=0; i<8; i++)
	{
		x += SC7A20_Get_ACCD(0x28,0x29);
		y += SC7A20_Get_ACCD(0x2A,0x2B);
		z += SC7A20_Get_ACCD(0x2C,0x2D);
		Ddl_Delay1ms(10);
	}
	//pst_SC7A20SystemPara->DeviceRunPara.esDeviceSensorsData.cDev_Attitude_SC7A_Status = sc7a20_data[0];
	
	x /= 8;
	y /= 8;
	z /= 8;

	*x_buf = x;
	*y_buf = y;
	*z_buf = z;

	return 0;
	#endif

  	
}

static void SC7A20TR_Get_XYZ(int *x, int *y, int *z)
{
	uint8_t u8Temp[6] = {0};
	uint16_t u16Temp[3] = {0};

	//I2C_SC7A20_Master_Receive(DEVICE_SC7A20_ADDRESS,0x28 | 0x80, u8Temp, 6, TIMEOUT);
	SC7A20TR_Read_nByte( 0xA8,  u8Temp, 6);

	u16Temp[0] = (u8Temp[1] << 8) | u8Temp[0];
	u16Temp[0] = (u16Temp[0] >> 4) & 0x0fff;	/* 高12位数据有效，补码显示 */
	u16Temp[1] = (u8Temp[3] << 8) | u8Temp[2];
	u16Temp[1] = (u16Temp[1] >> 4) & 0x0fff;	/* 高12位数据有效，补码显示 */
	u16Temp[2] = (u8Temp[5] << 8) | u8Temp[4];
	u16Temp[2] = (u16Temp[2] >> 4) & 0x0fff;	/* 高12位数据有效，补码显示 */

	if(u16Temp[0] & 0x0800)
	{
		u16Temp[0] = ~(u16Temp[0] - 1) & 0x0fff;
		*x = (int)u16Temp[0] * (-1);
	}
	else
	{
		*x = (int)u16Temp[0];
	}

	if(u16Temp[1] & 0x0800)
	{
		u16Temp[1] = ~(u16Temp[1] - 1) & 0x0fff;
		*y = (int)u16Temp[1] * (-1);
	}
	else
	{
		*y = (int)u16Temp[1];
	}

	if(u16Temp[2] & 0x0800)
	{
		u16Temp[2] = ~(u16Temp[2] - 1) & 0x0fff;
		*z = (int)u16Temp[2] * (-1);
	}
	else
	{
		*z = (int)u16Temp[2];
	}
}

static int calculate_tilt_angle(void)
{
	int sx = 0, sy = 0, sz = 0;
	double fx = 0, fy = 0, fz = 0;
	char SC7A20TR_FILTERING_TICK =8;
	double M_PI = 3.141592654;
	int nValue = 0;

	/* 获取角速度初始值 */
	for(uint8_t i = 0; i < SC7A20TR_FILTERING_TICK; i++)
	{
		SC7A20TR_Get_XYZ(&sx, &sy, &sz);
		fx += sx;
		fy += sy;
		fz += sz;
	}

	/* 平均滤波 */
	fx /= SC7A20TR_FILTERING_TICK;
	fy /= SC7A20TR_FILTERING_TICK;
	fz /= SC7A20TR_FILTERING_TICK;

	/* 得到单位为G的加速度值-2G量程 */
	fx *= 2.0 / 4096;
	fy *= 2.0 / 4096;
	fz *= 2.0 / 4096;

	// 计算 z 轴的倾角
	double z_angle = atan(sqrt(fx * fx + fy * fy) / fz);
	// 转换为角度
	nValue = (int)(z_angle * (180 / M_PI));
	return nValue; // radians to degrees
}

/**
  *****************************************************************************
  * @Name   : SC7A20TR任务
  * @Brief  : None
  * @Return : None
  *****************************************************************************
**/
int SC7A20_Task(void)
{
	int angle = calculate_tilt_angle();	/* 获取倾角加速度 */

	if(angle < 0)
	{
		angle = 180 + angle; /* 切换量程0~180 */
	}
	return angle;
}
/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
