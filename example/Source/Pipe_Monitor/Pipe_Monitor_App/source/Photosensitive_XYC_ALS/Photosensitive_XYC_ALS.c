/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\Photosensitive_XYC_ALS\Photosensitive_XYC_ALS.c
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
#include "Photosensitive_XYC_ALS.h"
#include "hc32f460_exint_nmi_swi.h"
#include "hc32f460_interrupts.h"
#include "hc32f460.h"
/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
static SystemPataSt *pst_PhotoSystemPara;
/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

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
#ifdef HW_VERSION_V1_1
void ExtInt01_Callback(void)
{
	uint8_t ucReadData = 0;
	uint8_t ucWriteData[2] = {0};
    if (Set == EXINT_IrqFlgGet(ExtiCh01))
    {
		//drv_XYC_ALS_ReadData(INT_FLAG_REG,&ucReadData,1);
		I2C_XYC_ALS_Master_Receive(DEVICE_XYC_ALS_ADDRESS, INT_FLAG_REG,&ucReadData, 1, TIMEOUT);
		if(ucReadData & 0x80)	//上电;VDD<1.4V; 软件重启导致的中断
		{
			ucWriteData[0] = INT_FLAG_REG;	//中断状态寄存器
			ucWriteData[1] = ucReadData;
			ucWriteData[1] &= 0x7F;	//清除该中断标志
			//drv_XYC_ALS_WriteData(INT_FLAG_REG,&ucWriteData,1);
			I2C_XYC_ALS_Master_Transmit(DEVICE_XYC_ALS_ADDRESS, ucWriteData, 2, TIMEOUT);
		}
		if(ucReadData & 0x40)	//转换数据非法
		{

		}
		if(ucReadData & 0x01)	//ALS中断
		{
			pst_PhotoSystemPara->DeviceRunPara.esDeviceSensorsData.cPhotosensitive_XYC_ALS_Status = 1;
			ucWriteData[0] = INT_FLAG_REG;	//中断状态寄存器
			ucWriteData[1] = ucReadData;
			ucWriteData[1] &= 0xFE;	//清除该中断标志
			//drv_XYC_ALS_WriteData(INT_FLAG_REG,&ucWriteData,1);
			I2C_XYC_ALS_Master_Transmit(DEVICE_XYC_ALS_ADDRESS, ucWriteData, 2, TIMEOUT);
		}
        
        /* clear int request flag */
        EXINT_IrqFlgClr(ExtiCh01);
    }
}
#else
void ExtInt08_Callback(void)
{
	uint8_t ucReadData = 0;
	uint8_t ucWriteData[2] = {0};
    if (Set == EXINT_IrqFlgGet(ExtiCh08))
    {
		//drv_XYC_ALS_ReadData(INT_FLAG_REG,&ucReadData,1);
		I2C_XYC_ALS_Master_Receive(DEVICE_XYC_ALS_ADDRESS, INT_FLAG_REG,&ucReadData, 1, TIMEOUT);
		if(ucReadData & 0x80)	//上电;VDD<1.4V; 软件重启导致的中断
		{
			ucWriteData[0] = INT_FLAG_REG;	//中断状态寄存器
			ucWriteData[1] = ucReadData;
			ucWriteData[1] &= 0x7F;	//清除该中断标志
			//drv_XYC_ALS_WriteData(INT_FLAG_REG,&ucWriteData,1);
			I2C_XYC_ALS_Master_Transmit(DEVICE_XYC_ALS_ADDRESS, ucWriteData, 2, TIMEOUT);
		}
		if(ucReadData & 0x40)	//转换数据非法
		{

		}
		if(ucReadData & 0x01)	//ALS中断
		{
			pst_PhotoSystemPara->DeviceRunPara.esDeviceSensorsData.cPhotosensitive_XYC_ALS_Status = 1;
			ucWriteData[0] = INT_FLAG_REG;	//中断状态寄存器
			ucWriteData[1] = ucReadData;
			ucWriteData[1] &= 0xFE;	//清除该中断标志
			//drv_XYC_ALS_WriteData(INT_FLAG_REG,&ucWriteData,1);
			I2C_XYC_ALS_Master_Transmit(DEVICE_XYC_ALS_ADDRESS, ucWriteData, 2, TIMEOUT);
		}
        
        /* clear int request flag */
        EXINT_IrqFlgClr(ExtiCh08);
    }
}
#endif

uint8_t drv_Photosensitive_XYC_ALS_Init(void)
{
	pst_PhotoSystemPara = GetSystemPara();
	stc_exint_config_t stcExtiConfig;
    stc_irq_regi_conf_t stcIrqRegiConf;
    stc_port_init_t stcPortInit;
	uint8_t ucData[2] = {0};

    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcExtiConfig);
    MEM_ZERO_STRUCT(stcIrqRegiConf);
    MEM_ZERO_STRUCT(stcPortInit);

	MEM_ZERO_STRUCT(stcPortInit);
	stcPortInit.enPinMode = Pin_Mode_Out;
	stcPortInit.enExInt = Disable;
	PORT_Init(VLT_GPIO_PORT, VLT_GPIO_PIN, &stcPortInit);

	VLT_PIN_OPEN();	//VLT引脚拉高，给XYC_ALS供电
	Ddl_Delay1ms(100);

    /**************************************************************************/
    /* External Int Ch.7                                                      */
    /**************************************************************************/
	#ifdef HW_VERSION_V1_1
	stcExtiConfig.enExitCh = ExtiCh01;
	#else
    stcExtiConfig.enExitCh = ExtiCh08;
	#endif

    /* Filter setting */
    stcExtiConfig.enFilterEn = Enable;
    stcExtiConfig.enFltClk = Pclk3Div8;
    stcExtiConfig.enExtiLvl = ExIntFallingEdge;
    EXINT_Init(&stcExtiConfig);

    /* Set External Int */
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enExInt = Enable;
    PORT_Init(LTINT_PORT, LTINT_PIN, &stcPortInit);

    /* Select External Int Ch.7 */
	#ifdef HW_VERSION_V1_1
	stcIrqRegiConf.enIntSrc = INT_PORT_EIRQ1;
	#else
    stcIrqRegiConf.enIntSrc = INT_PORT_EIRQ8;
	#endif

    /* Register External Int to Vect.No.000 */
    stcIrqRegiConf.enIRQn = Int001_IRQn;

    /* Callback function */
	#ifdef HW_VERSION_V1_1
	stcIrqRegiConf.pfnCallback = &ExtInt01_Callback;
	#else
    stcIrqRegiConf.pfnCallback = &ExtInt08_Callback;
	#endif

    /* Registration IRQ */
    enIrqRegistration(&stcIrqRegiConf);

    /* Clear pending */
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);

    /* Set priority */
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_01);

    /* Enable NVIC */
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

	#if 0
	drv_mcu_XYC_ALS_IIC_Init();

	//初始化命令配置
	ucData = 0x80;	//设备复位
	drv_XYC_ALS_WriteData(SYSM_CTRL_REG,&ucData,1);
	Ddl_Delay1ms(100);
	ucData = 0x41;	//允许两次测量之间加入等待时间; 启用ALS功能
	drv_XYC_ALS_WriteData(SYSM_CTRL_REG,&ucData,1);
	ucData = 0x11;	//当ALS中断产生时，测量将暂停到清除中断之前; 允许ALS中断影响INT引脚
	drv_XYC_ALS_WriteData(INT_CTRL_REG,&ucData,1);
	ucData = 0xD3;	//配置等待时间为8*8*20 = 1280ms
	drv_XYC_ALS_WriteData(WAIT_TIME_REG,&ucData,1);
	ucData = 0x01;	//增益控制，目前先使用默认值，不放大增益
	drv_XYC_ALS_WriteData(ALS_GAIN_REG,&ucData,1);
	ucData = 0x03;	//ALS时间配置，目前先使用默认值，最大输出值65535
	drv_XYC_ALS_WriteData(ALS_TIME_REG,&ucData,1);
	ucData = 0x0A;	//ALS持续次数达到10次后触发报警,总报警时间为10*1280ms= 12.8s
	drv_XYC_ALS_WriteData(ALS_PER_REG,&ucData,1);
	ucData = 0x10;	//预设低位报警阈值为10000(0x2710)；低位报警阈值低位：0x10
	drv_XYC_ALS_WriteData(ALS_THRES_LL,&ucData,1);
	ucData = 0x27;	//预设低位报警阈值为10000(0x2710)；低位报警阈值高位：0x27
	drv_XYC_ALS_WriteData(ALS_THRES_LH,&ucData,1);
	ucData = 0x50;	//预设高位报警阈值为50000(0xC350)；高位报警阈值低位：0x50
	drv_XYC_ALS_WriteData(ALS_THRES_HL,&ucData,1);
	ucData = 0xC3;	//预设高位报警阈值为50000(0xC350)；高位报警阈值高位：0xC3
	drv_XYC_ALS_WriteData(ALS_THRES_HH,&ucData,1);
	#else
	drv_XYC_ALS_HARTIIC_Init();
	#endif
	ucData[0] = SYSM_CTRL_REG;	
	ucData[1] = 0x80;	//设备复位
	I2C_XYC_ALS_Master_Transmit(DEVICE_XYC_ALS_ADDRESS, ucData, 2, TIMEOUT);
	Ddl_Delay1ms(100);
	ucData[0] = SYSM_CTRL_REG;	
	ucData[1] = 0x41;	//允许两次测量之间加入等待时间; 启用ALS功能
	I2C_XYC_ALS_Master_Transmit(DEVICE_XYC_ALS_ADDRESS, ucData, 2, TIMEOUT);
	ucData[0] = INT_CTRL_REG;	
	ucData[1] = 0x11;	//当ALS中断产生时，测量将暂停到清除中断之前; 允许ALS中断影响INT引脚
	I2C_XYC_ALS_Master_Transmit(DEVICE_XYC_ALS_ADDRESS, ucData, 2, TIMEOUT);
	ucData[0] = WAIT_TIME_REG;	
	ucData[1] = 0xD3;	//配置等待时间为8*8*20 = 1280ms
	I2C_XYC_ALS_Master_Transmit(DEVICE_XYC_ALS_ADDRESS, ucData, 2, TIMEOUT);
	ucData[0] = ALS_GAIN_REG;	
	ucData[1] = 0x01;	//增益控制，目前先使用默认值，不放大增益
	I2C_XYC_ALS_Master_Transmit(DEVICE_XYC_ALS_ADDRESS, ucData, 2, TIMEOUT);
	ucData[0] = ALS_TIME_REG;	
	ucData[1] = 0x03;	//ALS时间配置，目前先使用默认值，最大输出值65535
	I2C_XYC_ALS_Master_Transmit(DEVICE_XYC_ALS_ADDRESS, ucData, 2, TIMEOUT);
	ucData[0] = ALS_PER_REG;	
	//ucData[1] = 0x0A;	//ALS持续次数达到10次后触发报警,总报警时间为10*1280ms= 12.8s
	ucData[1] = 0x01;	//ALS持续次数达到10次后触发报警,总报警时间为10*1280ms= 12.8s
	I2C_XYC_ALS_Master_Transmit(DEVICE_XYC_ALS_ADDRESS, ucData, 2, TIMEOUT);
	ucData[0] = ALS_THRES_LL;	
	//ucData[1] = 0x10;	//预设低位报警阈值为10000(0x2710)；低位报警阈值低位：0x10
	ucData[1] = 0x00;	//预设低位报警阈值为0
	I2C_XYC_ALS_Master_Transmit(DEVICE_XYC_ALS_ADDRESS, ucData, 2, TIMEOUT);
	ucData[0] = ALS_THRES_LH;	
	//ucData[1] = 0x27;	//预设低位报警阈值为10000(0x2710)；低位报警阈值高位：0x27
	ucData[1] = 0x00;	//预设低位报警阈值为0
	I2C_XYC_ALS_Master_Transmit(DEVICE_XYC_ALS_ADDRESS, ucData, 2, TIMEOUT);
	ucData[0] = ALS_THRES_HL;	
	//ucData[1] = 0xD0;	//预设高位报警阈值为20000(0x07D0)；高位报警阈值低位：0xD0
	ucData[1] = 0x1E;	//预设高位报警阈值为30
	I2C_XYC_ALS_Master_Transmit(DEVICE_XYC_ALS_ADDRESS, ucData, 2, TIMEOUT);
	ucData[0] = ALS_THRES_HH;	
	//ucData[1] = 0x07;	//预设高位报警阈值为20000(0x07D0)；高位报警阈值高位：0x07
	ucData[1] = 0x00;	//预设高位报警阈值为30
	I2C_XYC_ALS_Master_Transmit(DEVICE_XYC_ALS_ADDRESS, ucData, 2, TIMEOUT);


	//ucData[0] = 0xBC;

	//I2C_XYC_ALS_Master_Receive(DEVICE_XYC_ALS_ADDRESS,0xBC, &ucData[0], 1, TIMEOUT);
	//I2C_XYC_ALS_Master_Receive(DEVICE_XYC_ALS_ADDRESS,0xBD, &ucData[1], 1, TIMEOUT);
	return 0;
}

uint16_t func_ReadPhoto_XYC_ALS_Data()
{
	uint16_t usData = 0;
	uint8_t ucData1[2] = {0};
	I2C_XYC_ALS_Master_Receive(DEVICE_XYC_ALS_ADDRESS,0x17, &ucData1[0], 1, TIMEOUT);

	I2C_XYC_ALS_Master_Receive(DEVICE_XYC_ALS_ADDRESS,0x1E, &ucData1[0], 1, TIMEOUT);
	I2C_XYC_ALS_Master_Receive(DEVICE_XYC_ALS_ADDRESS,0x1F, &ucData1[1], 1, TIMEOUT);
	usData = (ucData1[1] << 8) | ucData1[0];
	return usData;
}
/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
