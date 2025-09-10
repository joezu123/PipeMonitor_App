/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\OLED\OLED.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-04-02       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "OLED.h"
#include "Display.h"
#include "Font.h"
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
SystemPataSt *pst_OLEDSystemPara;
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
void OLED_Write_CMD(uint8_t ucmd)
{
	uint8_t i;

	LCMCS_PIN_RESET;
	LCMRS_PIN_RESET;
	for(i=0; i<8; i++)
	{
		OLED_IIC_SCL_L;
		//Ddl_Delay1us(2);
		if(ucmd & 0x80)
		{
			OLED_IIC_SDA_H;
		}
		else
		{
			OLED_IIC_SDA_L;
		}
		//Ddl_Delay1us(2);
		OLED_IIC_SCL_H;
		//Ddl_Delay1us(4);
		ucmd = ucmd << 1;
	}
	LCMCS_PIN_SET;
}

void OLED_Write_Data(uint8_t uData)
{
	uint8_t i;
	LCMCS_PIN_RESET;
	LCMRS_PIN_SET;
	for(i=0; i<8; i++)
	{
		OLED_IIC_SCL_L;
		//Ddl_Delay1us(2);
		if(uData & 0x80)
		{
			OLED_IIC_SDA_H;
		}
		else
		{
			OLED_IIC_SDA_L;
		}
		//Ddl_Delay1us(2);
		OLED_IIC_SCL_H;
		//Ddl_Delay1us(4);
		uData = uData << 1;
	}
	LCMCS_PIN_SET;
}

uint8_t drv_OLED_Init(void)
{
	stc_port_init_t stcPortInit;

	MEM_ZERO_STRUCT(stcPortInit);

	stcPortInit.enPinMode = Pin_Mode_Out;

    PORT_Init(PWRLCM_GPIO_PORT, PWRLCM_GPIO_PIN, &stcPortInit);

	PORT_Init(LCMRS_PORT, LCMRS_PIN, &stcPortInit);

	PORT_Init(LCMRST_PORT, LCMRST_PIN, &stcPortInit);

	PORT_Init(LCMCS_PORT, LCMCS_PIN, &stcPortInit);

	drv_mcu_OLED_IIC_Init();

	pst_OLEDSystemPara = GetSystemPara();

	memset(guc_OLED_Buf, 0, sizeof(guc_OLED_Buf));

	//开启OLED电源
	PWRLCM_PIN_CLOSE();
	Ddl_Delay1ms(100);
	PWRLCM_PIN_OPEN();

	LCMCS_PIN_RESET;

	//复位芯片
	LCMRST_PIN_RESET;
	Ddl_Delay1ms(100);
	LCMRST_PIN_SET;
	Ddl_Delay1ms(100);

	OLED_Write_CMD(0xAE);	//关显示
	OLED_Write_CMD(0x40);	//起始行
	OLED_Write_CMD(0x81);	//微调对比度，不可更改
	OLED_Write_CMD(0x32);	//微调对比度的值，可设置范围0x00~0xff ,默认0x32
	OLED_Write_CMD(0xc8); 	//行扫描顺序：从上到下 
	OLED_Write_CMD(0xa1); 	//列扫描顺序：从左到右 
	OLED_Write_CMD(0xa6); 	//正常显示模式 
	OLED_Write_CMD(0xa8); 	//duty 设置 
	OLED_Write_CMD(0x3f); 	//duty=1/64 
	OLED_Write_CMD(0xd3); 	//显示偏移 
	OLED_Write_CMD(0x00); 
	OLED_Write_CMD(0xd5); 	//晶振频率 
	OLED_Write_CMD(0xa0); 	//0x80 
	OLED_Write_CMD(0xd9); 	//Set Pre-Charge Period 
	OLED_Write_CMD(0xf1); 
	OLED_Write_CMD(0xda); 	//sequential configuration 
	OLED_Write_CMD(0x12); 
 	OLED_Write_CMD(0x91); 
 	OLED_Write_CMD(0x3f); 
 	OLED_Write_CMD(0x3f); 
 	OLED_Write_CMD(0x3f); 
 	OLED_Write_CMD(0x3f); 
 	OLED_Write_CMD(0xaf); 	//开显示

	return 0;
}

//OLED模块开启电源及初始化配置
void func_OLED_PowerUp_Init(void)
{
	stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Out;
    PORT_Init(LCMRST_PORT, LCMRST_PIN, &stcPortInit);
	PORT_Init(LCMCS_PORT, LCMCS_PIN, &stcPortInit);
	PORT_Init(LCMRS_PORT, LCMRS_PIN, &stcPortInit);
	PORT_Init(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN, &stcPortInit);
	PORT_Init(OLED_IIC_SCL_PORT, OLED_IIC_SCL_PIN, &stcPortInit);

	//开启OLED电源
	PWRLCM_PIN_CLOSE();
	Ddl_Delay1ms(100);
	PWRLCM_PIN_OPEN();

	LCMCS_PIN_RESET;

	//复位芯片
	LCMRST_PIN_RESET;
	Ddl_Delay1ms(100);
	LCMRST_PIN_SET;
	Ddl_Delay1ms(100);

	OLED_Write_CMD(0xAE);	//关显示
	OLED_Write_CMD(0x40);	//起始行
	OLED_Write_CMD(0x81);	//微调对比度，不可更改
	OLED_Write_CMD(0x32);	//微调对比度的值，可设置范围0x00~0xff ,默认0x32
	OLED_Write_CMD(0xc8); 	//行扫描顺序：从上到下 
	OLED_Write_CMD(0xa1); 	//列扫描顺序：从左到右 
	OLED_Write_CMD(0xa6); 	//正常显示模式 
	OLED_Write_CMD(0xa8); 	//duty 设置 
	OLED_Write_CMD(0x3f); 	//duty=1/64 
	OLED_Write_CMD(0xd3); 	//显示偏移 
	OLED_Write_CMD(0x00); 
	OLED_Write_CMD(0xd5); 	//晶振频率 
	OLED_Write_CMD(0xa0); 	//0x80 
	OLED_Write_CMD(0xd9); 	//Set Pre-Charge Period 
	OLED_Write_CMD(0xf1); 
	OLED_Write_CMD(0xda); 	//sequential configuration 
	OLED_Write_CMD(0x12); 
 	OLED_Write_CMD(0x91); 
 	OLED_Write_CMD(0x3f); 
 	OLED_Write_CMD(0x3f); 
 	OLED_Write_CMD(0x3f); 
 	OLED_Write_CMD(0x3f); 
 	OLED_Write_CMD(0xaf); 	//开显示
}

void func_OLED_PowerDown_DeInit(void)
{
	stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Ana;
	PORT_Init(LCMRST_PORT, LCMRST_PIN, &stcPortInit);
	PORT_Init(LCMCS_PORT, LCMCS_PIN, &stcPortInit);
	PORT_Init(LCMRS_PORT, LCMRS_PIN, &stcPortInit);
	PORT_Init(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN, &stcPortInit);
	PORT_Init(OLED_IIC_SCL_PORT, OLED_IIC_SCL_PIN, &stcPortInit);
	//LCMCS_PIN_SET;
	PWRLCM_PIN_CLOSE();	//关闭OLED电源
}

void lcd_address(uint8_t page,uint8_t column) 
{ 
 	column=column-1; //我们平常所说的第 1 列，在 LCD 驱动 IC 里是第 0 列。所以在这里减去1. 
 	page=page-1; 
 	OLED_Write_CMD(0xb0+page); //设置页地址。每页是 8 行。一个画面的 64 行被分成 8 个页。我们平常所说的第 1 页，在LCD 驱动 IC 里是第 0 页，所以在这里减去 1 
 	OLED_Write_CMD(((column>>4)&0x0f)+0x10); //设置列地址的高 4 位 
 	OLED_Write_CMD(column&0x0f); //设置列地址的低 4 位 
}

//全屏清屏 
void clear_screen() 
{ 
 	//unsigned char i,j; 
	#if 1
	stc_port_init_t stcPortInit;
	MEM_ZERO_STRUCT(stcPortInit);
	stcPortInit.enPinMode = Pin_Mode_Out;
	//PORT_Init(LCMRST_PORT, LCMRST_PIN, &stcPortInit);
	PORT_Init(LCMCS_PORT, LCMCS_PIN, &stcPortInit);
	PORT_Init(LCMRS_PORT, LCMRS_PIN, &stcPortInit);
	PORT_Init(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN, &stcPortInit);
	PORT_Init(OLED_IIC_SCL_PORT, OLED_IIC_SCL_PIN, &stcPortInit);
	#endif
	#if 0
 	for(j=0;j<8;j++) 
 	{ 
 		lcd_address(1+j,1); 
 		for(i=0;i<128;i++) 
 		{ 
 			OLED_Write_Data(0x00); 
 		} 
 	} 
	#endif
	memset(guc_OLED_Buf, 0, sizeof(guc_OLED_Buf));
} 
 
#if 1
//显示 128x64 点阵图像 
void display_128x64(uint8_t *dp) 
{ 
 	uint8_t i,j; 
	stc_port_init_t stcPortInit;
	MEM_ZERO_STRUCT(stcPortInit);
	stcPortInit.enPinMode = Pin_Mode_Ana;
	
 	for(j=0;j<8;j++) 
 	{ 
 		lcd_address(j+1,1); 
 		for (i=0;i<128;i++) 
 		{ 
			OLED_Write_Data(*dp); //写数据到 LCD,每写完一个 8 位的数据后列地址自动加 1 
 			dp++; 
 		} 
 	} 
	#if 1
	//PORT_Init(LCMRST_PORT, LCMRST_PIN, &stcPortInit);
	PORT_Init(LCMCS_PORT, LCMCS_PIN, &stcPortInit);
	PORT_Init(LCMRS_PORT, LCMRS_PIN, &stcPortInit);
	PORT_Init(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN, &stcPortInit);
	PORT_Init(OLED_IIC_SCL_PORT, OLED_IIC_SCL_PIN, &stcPortInit);
	#endif
}
#endif

void func_Display_128x64(unsigned char ucType)
{
	uint8_t i,j; 
	//uint8_t *dp = (uint8_t *)&g_sSKLogo_New[0];
	//uint8_t *dp = (uint8_t *)&guc_OLED_Buf[0];
	stc_port_init_t stcPortInit;
	MEM_ZERO_STRUCT(stcPortInit);
	stcPortInit.enPinMode = Pin_Mode_Ana;

	#if 0
	for(j=0xb0; j<0xb8; j++)
	{
		/*选择LCD的地址*/
		OLED_Write_CMD(j);
		OLED_Write_CMD(0x10);
		OLED_Write_CMD(0x00);

		for(i=0; i<128; i++) 
		{
			//if(ucType == 0)
			{
			OLED_Write_Data(guc_OLED_Buf[i][j-0xb0]);
			}
			//else
			//{
			//	OLED_Write_Data(*dp); //写数据到 LCD,每写完一个 8 位的数据后列地址自动加 1 
			//	dp++; 
			//}
		}
	}
	#else
	for(j=0;j<8;j++) 
 	{ 
 		lcd_address(j+1,1); 
 		for (i=0;i<128;i++) 
 		{ 
			//OLED_Write_Data(*dp); //写数据到 LCD,每写完一个 8 位的数据后列地址自动加 1 
 			//dp++; 
			 OLED_Write_Data(guc_OLED_Buf[i][j]);
 		} 
 	} 
	#endif

	#if 1
	//PORT_Init(LCMRST_PORT, LCMRST_PIN, &stcPortInit);
	PORT_Init(LCMCS_PORT, LCMCS_PIN, &stcPortInit);
	PORT_Init(LCMRS_PORT, LCMRS_PIN, &stcPortInit);
	PORT_Init(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN, &stcPortInit);
	PORT_Init(OLED_IIC_SCL_PORT, OLED_IIC_SCL_PIN, &stcPortInit);
	#endif
}

#if 0
//显示 32x32 点阵图像、汉字、生僻字或 32x32 点阵的其他图标 
void display_graphic_32x32(uint8_t page,uint8_t column,uint8_t *dp) 
{ 
	uint8_t i,j; 
 	for(j=0;j<4;j++) 
 	{ 
 		lcd_address(page+j,column); 
 		for (i=0;i<32;i++) 
 		{ 
			OLED_Write_Data(*dp); //写数据到 LCD,每写完一个 8 位的数据后列地址自动加 1 
 			dp++; 
 		} 
	} 
} 

 
//显示 16x16 点阵图像、汉字、生僻字或 16x16 点阵的其他图标 
void display_graphic_16x16(uint8_t page,uint8_t column,uint8_t *dp) 
{ 
	uint8_t i,j; 
 	for(j=0;j<2;j++) 
 	{ 
 		lcd_address(page+j,column); 
 		for (i=0;i<16;i++) 
 		{ 
			OLED_Write_Data(*dp); //写数据到 LCD,每写完一个 8 位的数据后列地址自动加 1 
 			dp++; 
		} 
 	} 
} 

//显示 8x16 点阵图像、ASCII, 或 8x16 点阵的自造字符、其他图标 
void display_graphic_8x16(uint8_t page,uint8_t column,uint8_t *dp) 
{ 
	uint8_t i,j; 
 	for(j=0;j<2;j++) 
 	{ 
 		lcd_address(page+j,column); 
 		for (i=0;i<8;i++) 
 		{ 
			OLED_Write_Data(*dp); //写数据到 LCD,每写完一个 8 位的数据后列地址自动加 1 
 			dp++; 
 		} 
 	} 
} 
#endif
void OLED_Test(uint8_t ucType)
{
	uint8_t i,j;
	//uint16_t k;
	uint8_t ucValue = 0;
	signed short sTestArr[11] = {0};
	clear_screen();
	memset(guc_OLED_Buf,0,sizeof(guc_OLED_Buf));
	switch (ucType)
	{
	case 0:
		for(i=0; i<4; i++)
		{
			for(j=0; j<128; j++)
			{
				guc_OLED_Buf[j][i] = 0xFF;
			}
		}
		func_Display_128x64(0);
		break;
	case 1:
		for(i=4; i<8; i++)
		{
			for(j=0; j<128; j++)
			{
				guc_OLED_Buf[j][i] = 0xFF;
			}
		}
		func_Display_128x64(0);
		break;
	case 2:
		for(i=0; i<8; i++)
		{
			for(j=0; j<64; j++)
			{
				guc_OLED_Buf[j][i] = 0xFF;
			}
		}
		func_Display_128x64(0);
		break;
	case 3:
		for(i=0; i<8; i++)
		{
			for(j=64; j<128; j++)
			{
				guc_OLED_Buf[j][i] = 0xFF;
			}
		}
		func_Display_128x64(0);
		break;
	case 4:
		for(i=0; i<8; i++)
		{
			for(j=0; j<128; j++)
			{
				guc_OLED_Buf[j][i] = 0xFF;
			}
		}
		func_Display_128x64(0);
		break;
	case 5:	//SK LOGO
		display_128x64((uint8_t *)&g_sSKLogo[0]);
		break;
	case 6:	
		//姿态数据显示
		memset(sTestArr,0,sizeof(sTestArr));
		for(i=1; i<4; i++)
		{
			sTestArr[i] = 0x0000 + i;
		}
		sTestArr[4] = 0xFFFF;
		func_display_string(0,0,&sTestArr[0]);

		memset(sTestArr,0,sizeof(sTestArr));
		ucValue = gc_SystemPosi / 100;
		sTestArr[0] = 0x0500 + ucValue;
		ucValue = (gc_SystemPosi / 10) % 10;
		sTestArr[1] = 0x0500 + ucValue;
		ucValue = gc_SystemPosi % 10;
		sTestArr[2] = 0x0500 + ucValue;
		sTestArr[3] = 0xFFFF;
		func_display_string(60,0,&sTestArr[0]);

		//电量数据
		memset(sTestArr,0,sizeof(sTestArr));
		sTestArr[0] = 0x0004;
		sTestArr[1] = 0x0005;
		sTestArr[2] = 0x0002;
		sTestArr[3] = 0x0003;
		sTestArr[4] = 0xFFFF;

		func_display_string(0,24,&sTestArr[0]);

		memset(sTestArr,0,sizeof(sTestArr));
		func_sprintf_metricsystem(&sTestArr[0],pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent,3,5);
		func_display_string(60,24,&sTestArr[0]);

		//光照数据
		memset(sTestArr,0,sizeof(sTestArr));
		sTestArr[0] = 0x0006;
		sTestArr[1] = 0x0007;
		sTestArr[2] = 0x0002;
		sTestArr[3] = 0x0003;
		sTestArr[4] = 0xFFFF;
		func_display_string(0,48,&sTestArr[0]);
		ucValue = pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.sPhotosensitive_XYC_ALS_Data / 100;
		sTestArr[0] = 0x0500 + ucValue;
		ucValue = (pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.sPhotosensitive_XYC_ALS_Data / 10) % 10;
		sTestArr[1] = 0x0500 + ucValue;
		ucValue = pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.sPhotosensitive_XYC_ALS_Data % 10;
		sTestArr[2] = 0x0500 + ucValue;
		sTestArr[3] = 0xFFFF;
		func_display_string(60,48,&sTestArr[0]);
		break;
	case 7:	
		//蓝牙数据
		memset(sTestArr,0,sizeof(sTestArr));
		sTestArr[0] = 0x0008;
		sTestArr[1] = 0x0009;
		sTestArr[2] = 0x0002;
		sTestArr[3] = 0x0003;
		sTestArr[4] = 0xFFFF;
		func_display_string(0,0,&sTestArr[0]);
		ucValue = pst_OLEDSystemPara->DeviceRunPara.usBTRecValue / 100;
		sTestArr[0] = 0x0500 + ucValue;
		ucValue = (pst_OLEDSystemPara->DeviceRunPara.usBTRecValue / 10) % 10;
		sTestArr[1] = 0x0500 + ucValue;
		ucValue = pst_OLEDSystemPara->DeviceRunPara.usBTRecValue % 10;
		sTestArr[2] = 0x0500 + ucValue;
		sTestArr[3] = 0xFFFF;
		func_display_string(60,0,&sTestArr[0]);

		//水浸数据
		memset(sTestArr,0,sizeof(sTestArr));
		sTestArr[0] = 0x000A;
		sTestArr[1] = 0x000B;
		sTestArr[2] = 0x0002;
		sTestArr[3] = 0x0003;
		sTestArr[4] = 0xFFFF;

		func_display_string(0,24,&sTestArr[0]);

		memset(sTestArr,0,sizeof(sTestArr));
		func_sprintf_metricsystem(&sTestArr[0],pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.fWater_Immersion_Level,3,5);
		func_display_string(60,24,&sTestArr[0]);

		//磁棒数据
		memset(sTestArr,0,sizeof(sTestArr));
		sTestArr[0] = 0x000C;
		sTestArr[1] = 0x000D;
		sTestArr[2] = 0x0002;
		sTestArr[3] = 0x0003;
		sTestArr[4] = 0xFFFF;
		func_display_string(0,48,&sTestArr[0]);
		ucValue = pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.cMagnetic_Bar_Status;
		sTestArr[0] = 0x0500 +ucValue;
		//sTestArr[1] = 0x0500 + (gus_BarCnt % 10);
		//sTestArr[2] = 0x050A;
		//sTestArr[3] = 0x0500 + (gus_BarCnt1 / 10);
		//sTestArr[4] = 0x0500 + (gus_BarCnt1 % 10);
		sTestArr[1] = 0xFFFF;
		func_display_string(60,48,&sTestArr[0]);
		break;
	case 8:	//显示BD数据
		for(i=0; i<6; i++)
		{
			sTestArr[i] = (guc_TextBDData[i]) - 0x30 | 0x0500;
		}
		sTestArr[6] = 0xFFFF;
		func_display_string(0,0,&sTestArr[0]);
		sTestArr[0] = (guc_TextBDData[11] - 0x33) | 0x0500;
		sTestArr[1] = 0xFFFF;
		func_display_string(0,24,&sTestArr[0]);
		break;
	default:
		break;
	}
	func_Display_128x64(0);
}




/* -------------------------------------------------------------------------------------------------------------
 *
 *         LCD 坐标原点位于左上角, 纵向向下为Y 8个字节, 横向128列
 * @brief  根据菜单文字编码数组将字库点阵存入显示屏RAM
 * @param   X_dot:0..127
			Y_dot:0..63
			in_DisplayString_pt:菜单文字编码数组
 * @retval 
 --------------------------------------------------------------------------------------------------------------- */
void func_display_string(uint8_t X_dot,uint8_t Y_dot,signed short *text)
{
	signed short j,sPosi;
	signed short sSize,sCode;
	signed short sWidth,Y_Bias,fbm19h;
	//unsigned char ucX_dot_H, ucX_dot_L;
	//unsigned short s_xSum = 0;
	signed char fbm1h,fbm2h,fbm3h;

	sPosi = 0;
	while(text[sPosi] != -1)
	{        
		//根据高8位判断显示像素大小，进行显示
		sSize = (text[sPosi]>>8)&0x00FF;	
		sCode =	text[sPosi]&0x00FF;
		#if 0
		ucX_dot_H = (( X_dot + s_xSum + 4  )>>4)|0x10;
		ucX_dot_L = ( X_dot + s_xSum + 4  )&0x0f;
		#endif
		switch (sSize)
		{
		case 0:	//汉字
			#if 0
			for(j=0;j<2;j++)//一个显示含有几个行（几个page）---2=16/8 i<page
			{
				OLED_Write_CMD(0xb0 + j + Y_dot/8);//pa页地址
				OLED_Write_CMD(ucX_dot_H);//高位
				OLED_Write_CMD(ucX_dot_L);//低位

				for(i=0;i<12;i++) //i<column
				{
					OLED_Write_Data(g_sint8_Font_16x12_12864[sCode][2*i + j]); //i*pag
					//delay_ms(5);
				}
			}
			if(sCode == 0x17)	//,
			{	
				s_xSum += 6;
			}
			else
			{
				s_xSum += 12;
			}
			#else
			j = 0;
			sWidth = 12;
			while(j < sWidth)
			{
				Y_Bias = Y_dot % 8;
				//fbm13h = (x_fbm5h<<3)	+ (y_fbm7h/8);
				if(Y_Bias < 4)
				{
					fbm1h = g_sint8_Font_16x12_12864[sCode][j*2]<<(Y_Bias);
					
					guc_OLED_Buf[X_dot][Y_dot/8] |= fbm1h;		
					
					fbm1h = ((unsigned char)g_sint8_Font_16x12_12864[sCode][j*2])>>(8-Y_Bias)|(g_sint8_Font_16x12_12864[sCode][j*2+1] << Y_Bias);	
					
					guc_OLED_Buf[X_dot][Y_dot/8+1] |= fbm1h;	
					
				}
				else
				{
					fbm1h = g_sint8_Font_16x12_12864[sCode][j*2] << Y_Bias;
					
					guc_OLED_Buf[X_dot][Y_dot/8] |= fbm1h;
					
					fbm1h = ((unsigned char)g_sint8_Font_16x12_12864[sCode][j*2])>>(8-Y_Bias) | (g_sint8_Font_16x12_12864[sCode][j*2+1]<<Y_Bias);
					
					guc_OLED_Buf[X_dot][Y_dot/8+1] |= fbm1h;	
					
					fbm1h = ((unsigned char)g_sint8_Font_16x12_12864[sCode][j*2+1])>>(8-Y_Bias);
					
					guc_OLED_Buf[X_dot][Y_dot/8+2] |= fbm1h;			
					
				}
				X_dot++;
				j++;
			}
			#endif
			break;
		case 5:	//数字+字母
			#if 0
			for(j=0;j<2;j++)
			{
				/*选择LCD的地址*/
				OLED_Write_CMD(0xb0 + j + Y_dot/8);//pa页地址
				OLED_Write_CMD(ucX_dot_H);//高位
				OLED_Write_CMD(ucX_dot_L);//低位

				for(i=0;i<10;i++)
				{
					OLED_Write_Data(g_sint8_Font_16x10_12864[sCode][2*i + j]); 
				}
			}
			if(sCode == 0x32 || sCode == 0x33)// '.' ' '
			{
				s_xSum += 4;
			}
			else if(sCode == 0x2A || sCode == 0x2C) // ()
			{
				s_xSum += 5;
			}
			else if(sCode == 0x37)// '
			{
				s_xSum += 3;
			}
			else if(sCode == 0x38)// "
			{
				s_xSum += 6;
			}
			else
			{
				s_xSum += 0x0A;
			}
			#else
			j = 0;
			if(sCode == 0x32 || sCode == 0x33)// '.' ' '
			{
				sWidth = 4;
			}
			else if(sCode == 0x46)	// ':'
			{
				sWidth = 7;
			}
			else
			{
				sWidth = 10;
			}
			
			while(j < sWidth)
			{	
				if(guc_LcdDipRevesAttr != 0)
				{
					fbm2h = (~g_sint8_Font_16x10_12864[sCode][j * 2])&0xFC;
					fbm3h = (~g_sint8_Font_16x10_12864[sCode][j * 2+1])&0x7F;
				}
				else
				{
					fbm2h = g_sint8_Font_16x10_12864[sCode][j * 2];
					fbm3h = g_sint8_Font_16x10_12864[sCode][j * 2+1];
				}
				//		[0x0045,0x004B] is GODA logo
				if(sCode < 0x0045 || sCode> 0x004B)
				{
					fbm2h = (((unsigned char)fbm2h)>>2) | (fbm3h << 6) ;		
					fbm3h = ((unsigned char)fbm3h)>>2;
				}
				
				Y_Bias = Y_dot % 8;
				
				if(Y_Bias <= 3)
				{
					fbm19h = 0x00FF;
					
				
					fbm1h = (((unsigned short)fbm19h) >> (8-Y_Bias)) & guc_OLED_Buf[X_dot][Y_dot/8];
						
					fbm1h |= fbm2h<<Y_Bias;
					fbm1h = (((unsigned short)fbm19h) >> (8-Y_Bias)) & guc_OLED_Buf[X_dot][Y_dot/8];
						
					fbm1h |= fbm2h<<Y_Bias;
					guc_OLED_Buf[X_dot][Y_dot/8] = fbm1h;		
					fbm1h = (guc_OLED_Buf[X_dot][Y_dot/8+1] & (fbm19h << (13-(8-Y_Bias))));
				
					
					fbm1h |= ( (((unsigned char)fbm2h) >> (8-Y_Bias)) | (fbm3h << Y_Bias));
				
					guc_OLED_Buf[X_dot][Y_dot/8 + 1] = fbm1h;		
				
				}
				else
				{
					fbm19h = 0x00FF;
				
					fbm1h = (((unsigned short)fbm19h) >> (8-Y_Bias)) & guc_OLED_Buf[X_dot][Y_dot/8];
					fbm1h |= fbm2h<<Y_Bias;			
					guc_OLED_Buf[X_dot][Y_dot/8] = fbm1h;

					fbm1h = ( (((unsigned char)fbm2h) >> (8-Y_Bias)) | (fbm3h << Y_Bias));	
					guc_OLED_Buf[X_dot][Y_dot/8 + 1] = fbm1h;			

					fbm1h = guc_OLED_Buf[X_dot][Y_dot/8 + 2] & (fbm19h << (5-(8-Y_Bias)));
							
					fbm1h |= (((unsigned char)fbm3h) >> (8-Y_Bias));
					guc_OLED_Buf[X_dot][Y_dot/8 + 2] = fbm1h;	
				}

				X_dot++;
				j++;	
			}
			#endif
			break;
		case 7:	//ASCII 6X12
			j = 0;

			if(sCode == 0x00 || sCode == 0x0E || sCode == 0x1A)// '.' ' '
			{
				sWidth = 4;
			}
			else
			{
				sWidth = 6;
			}
			
			while(j < sWidth)
			{	
				fbm2h = g_sint8_Font_6x12_AsciiDot[sCode][j * 2];
				fbm3h = g_sint8_Font_6x12_AsciiDot[sCode][j * 2+1];
				
				//		[0x0045,0x004B] is GODA logo
				//if(sCode < 0x0045 || sCode> 0x004B)
				{
				//	fbm2h = (((unsigned char)fbm2h)>>2) | (fbm3h << 6) ;		
				//	fbm3h = ((unsigned char)fbm3h)>>2;
				}
				
				Y_Bias = Y_dot % 8;
				
				if(Y_Bias <= 3)
				{
					fbm19h = 0x00FF;
					
				
					fbm1h = (((unsigned short)fbm19h) >> (8-Y_Bias)) & guc_OLED_Buf[X_dot][Y_dot/8];
						
					fbm1h |= fbm2h<<Y_Bias;
					fbm1h = (((unsigned short)fbm19h) >> (8-Y_Bias)) & guc_OLED_Buf[X_dot][Y_dot/8];
						
					fbm1h |= fbm2h<<Y_Bias;
					guc_OLED_Buf[X_dot][Y_dot/8] = fbm1h;		
					fbm1h = (guc_OLED_Buf[X_dot][Y_dot/8+1] & (fbm19h << (13-(8-Y_Bias))));
				
					
					fbm1h |= ( (((unsigned char)fbm2h) >> (8-Y_Bias)) | (fbm3h << Y_Bias));
				
					guc_OLED_Buf[X_dot][Y_dot/8 + 1] = fbm1h;		
				
				}
				else
				{
					fbm19h = 0x00FF;
				
					fbm1h = (((unsigned short)fbm19h) >> (8-Y_Bias)) & guc_OLED_Buf[X_dot][Y_dot/8];
					fbm1h |= fbm2h<<Y_Bias;			
					guc_OLED_Buf[X_dot][Y_dot/8] = fbm1h;

					fbm1h = ( (((unsigned char)fbm2h) >> (8-Y_Bias)) | (fbm3h << Y_Bias));	
					guc_OLED_Buf[X_dot][Y_dot/8 + 1] = fbm1h;			

					fbm1h = guc_OLED_Buf[X_dot][Y_dot/8 + 2] & (fbm19h << (5-(8-Y_Bias)));
							
					fbm1h |= (((unsigned char)fbm3h) >> (8-Y_Bias));
					guc_OLED_Buf[X_dot][Y_dot/8 + 2] = fbm1h;	
				}

				X_dot++;
				j++;	
			}
			break;
		default:
			break;
		}
		
		sPosi++;
	}
}

#if 0
//显示 8x16 的点阵的字符串，括号里的参数分别为（页,列，字符串指针） 
void display_string_8x16(uint8_t page,uint8_t column,uint8_t *text) 
{ 
	uint8_t i=0,j,k,n; 
 	if(column>123) 
 	{ 
 		column=1; 
 		page+=2; 
 	} 
 	while(text[i]>0x00) 
 	{ 
 		if((text[i]>=0x20)&&(text[i]<=0x7e)) 
 		{ 
 			j=text[i]-0x20; 
 			for(n=0;n<2;n++) 
 			{ 
 				lcd_address(page+n,column); 
 				for(k=0;k<8;k++) 
 				{ 
					OLED_Write_Data(ascii_table_8x16[j][k+8*n]); //写数据到 LCD,每写完 1 字节的数据后列地址自动加 1 
 				} 
 			} 
 			i++; 
 			column+=8; 
 		} 
 		else 
		{
			i++; 
		}
 	} 
} 
 
//显示 5x8 的点阵的字符串，括号里的参数分别为（页,列，字符串指针） 
void display_string_5x8(uint8_t page,uint8_t column,uint8_t reverse,uint8_t *text) 
{ 
	uint8_t i=0,j,k,disp_data; 
 	while(text[i]>0x00) 
 	{ 
 		if((text[i]>=0x20)&&(text[i]<=0x7e)) 
 		{ 
 			j=text[i]-0x20; 
 			lcd_address(page,column); 
 			for(k=0;k<5;k++) 
 			{ 
 				if(reverse==1) 
 				{ 
 					disp_data=~ascii_table_5x8[j][k]; 
 				} 
 				else 
 				{ 
 					disp_data=ascii_table_5x8[j][k]; 
 				} 
 
 				OLED_Write_Data(disp_data); //写数据到 LCD,每写完 1 字节的数据后列地址自动加 1 
 			} 
 			if(reverse==1)  //写入一列空白列，使得 5x8 的字符与字符之间有一列间隔，更美观 
			{
				OLED_Write_Data(0xff);
			}
 			else  //写入一列空白列，使得 5x8 的字符与字符之间有一列间隔，更美观 
			{
				OLED_Write_Data(0x00);
			}
 			i++; 
 			column+=6; 
 			if(column>123) 
 			{ 
 				column=1; 
 				page++; 
 			} 
 		} 
 		else
		{
			i++; 
		}
 	} 
} 
 
//写入一组 16x16 点阵的汉字字符串（字符串表格中需含有此字） 
//括号里的参数：(页，列，汉字字符串） 
void display_string_16x16(uint8_t page,uint8_t column,uint8_t *text) 
{ 
	uint8_t i,j,k; 
 	uint32_t address; 
 
 	j = 0; 
 	while(text[j] != '\0') 
 	{ 
		 i = 0; 
 		address = 1; 
 		while(Chinese_text_16x16[i] > 0x7e) // >0x7f 即说明不是 ASCII 码字符 
 		{ 
 			if(Chinese_text_16x16[i] == text[j]) 
 			{ 
 				if(Chinese_text_16x16[i + 1] == text[j + 1]) 
 				{ 
 					address = i * 16; 
 					break; 
 				} 
 			} 
 			i += 2; 
 		} 
 
 		if(column > 113) 
 		{ 
 			column = 0; 
 			page += 2; 
 		} 
 
 		if(address != 1)// 显示汉字 
 		{ 
 
 			for(k=0;k<2;k++) 
 			{ 
 				lcd_address(page+k,column); 
 				for(i = 0; i < 16; i++) 
 				{ 
					OLED_Write_Data(Chinese_code_16x16[address]); 
 					address++; 
 				} 
 			} 
 			j += 2; 
 		} 
 		else //显示空白字符 
 		{ 
 			for(k=0;k<2;k++) 
 			{ 
 				lcd_address(page+k,column); 
 				for(i = 0; i < 16; i++) 
 				{ 
					OLED_Write_Data(0x00); 
 				} 
 			} 
 
 			j++; 
 		} 
 
 		column+=16; 
 	} 
} 
 
//显示 16x16 点阵的汉字或者 ASCII 码 8x16 点阵的字符混合字符串 
//括号里的参数：(页，列，字符串） 
void disp_string_8x16_16x16(uint8_t page,uint8_t column,uint8_t *text) 
{ 
	uint8_t temp[3]; 
 	uint8_t i = 0; 
 
 	while(text[i] != '\0') 
 	{ 
 		if(text[i] > 0x7e) 
 		{ 
 			temp[0] = text[i]; 
 			temp[1] = text[i + 1]; 
 			temp[2] = '\0'; //汉字为两个字节 
 			display_string_16x16(page,column,temp); //显示汉字 
 			column += 16; 
 			i += 2; 
 		} 
 		else 
 		{ 
 			temp[0] = text[i]; 
 			temp[1] = '\0'; //字母占一个字节 
 			display_string_8x16(page, column, temp); //显示字母 
 			column += 8; 
 			i++; 
 		} 
 	} 
}

void OLED_Test(void)
{
	while(1) 
 	{ 
 		clear_screen(); //清屏 
 
		//演示 32x32 点阵的汉字，16x16 点阵的汉字，8x16 点阵的字符，5x8 点阵的字符 
 		display_string_5x8(1,1,0,"{(5x8dot ASCII char)}");//显示字符串，括号里的参数分别为（PAGE,列,字符串指针） 

 		display_string_5x8(2,1,0,"{[(<~!@#$%^&*_+=?>)]}"); 
 		disp_string_8x16_16x16(3,1,"标准 16x16dot 汉字"); //显示 16x16 点阵汉字串或 8x16 点阵的字符串，括号里的参数分别为（页,列,字符串指针） 
 		display_graphic_32x32 (5,1+32*0,jing1); //显示单个 32x32 点阵的汉字，括号里的参数分别为（PAGE,列,字符指针） 
 		display_graphic_32x32 (5,1+32*1,lian1); 
 		display_graphic_32x32 (5,1+32*2,xun1); 
 		disp_string_8x16_16x16(5,1+32*3,"JLX:"); 
 		disp_string_8x16_16x16(7,1+32*3,"OLED"); 

		//下一页
 
		//演示显示一页纯英文的 5x8 点阵的菜单界面 
 		clear_screen(); //clear all dots 
 		display_string_5x8(1,1,1,"012345678901234567890"); 
 		display_string_5x8(1,1,1," MENU "); //显示 5x8 点阵的字符串，括号里的参数分别为（页，列，是否反显，数据指针） 
 		display_string_5x8(3,1,0,"Select>>>>"); 
 		display_string_5x8(3,64,1,"1.Graphic "); 
 		display_string_5x8(4,64,0,"2.Chinese " ); 
 		display_string_5x8(5,64,0,"3.Movie "); 
 		display_string_5x8(6,64,0,"4.Contrast"); 
 		display_string_5x8(7,64,0,"5.Mirror "); 
 		display_string_5x8(8,1,1,"PRE USER DEL NEW"); 
 		display_string_5x8(8,19,0," "); 
 		display_string_5x8(8,65,0," "); 
 		display_string_5x8(8,97,0," "); 
 		waitkey(); 
 		clear_screen(); //clear all dots 
 		display_128x64(bmp1); 
 		waitkey(); 
 		clear_screen(); //clear all dots 
 		display_128x64(bmp2); 
 		waitkey(); 
 	}
}
#endif
/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
