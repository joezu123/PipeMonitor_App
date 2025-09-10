/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\OLED\Display.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-04-23       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "Display.h"
#include "math.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "Font.h"
#include "OLED.h"
#include "drv_RTC.h"
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

void func_sprintf_metricsystem(signed short * buffer, float fb5h,char fb9h,char fbah)
{
	//signed short * ptr_fb2h = buffer;
	//signed short sBuf[20] = {0};
	short len_fbm4h;
	short i_fbm6h;
	float fbmeh;
	short int_fbmeh;
	float fbm16h;
	short int_fbm16h;
	float fbm1eh;
	char buf_fbm3c[30];
	unsigned char buf_fbm5a[30];
	float fValue;
	long lValue;
	
	if(fb9h > 3)
	{
		fb9h = 3;
	}
	
	switch(fb9h)
	{
	case 0:
		if(fb5h >= 0.00f)
		{
			fb5h = (0.5f + fb5h);
		}
		else
		{
			fb5h = (-0.5f + fb5h);
		}
		break;
	case 1:
		if(fb5h >= 0.00f)
		{
		fb5h = (0.05f + fb5h);
		}
		else
		{
			fb5h = (-0.05f + fb5h);
		}
		break;
	case 2:
		if(fb5h >= 0.00f)
		{
		fb5h = (0.005f + fb5h);
		}
		else
		{
			fb5h = (-0.005f + fb5h);
		}
		break;
	case 3:
		if(fb5h >= 0.00f)
		{
		fb5h = (0.0005f + fb5h);
		}
		else
		{
			fb5h = (-0.0005f + fb5h);
		}
		break;
	default:
		break;
	}

	if((9999999.000000f>=fb5h))
	{
		#if 1
		i_fbm6h = 0;
		while(i_fbm6h < 7)
		{
			fbmeh = (0.000100f +fb5h);		//fb5h+0.000100
			if((0.0f>fbmeh))
			{	// 0.0 > fbmeh, that is ,fbmeh < 0
				fbm1eh = -fbmeh;	
			}
			else
			{
				fbm1eh = fbmeh;				
			}
			fbm16h = fbm1eh;
			fValue = pow(10.0f,i_fbm6h);
			fValue = fbm16h / fValue;
			lValue = (long)fValue;
			lValue = lValue % 10;
			buf_fbm5a[i_fbm6h+5] = (char)lValue;
			//sBuf[i_fbm6h+5] = (char)lValue;
			i_fbm6h++;
		}
		#endif
		i_fbm6h = 3;
		while(i_fbm6h >= 0)
		{
			fbmeh = (0.000100f+ fb5h);
			if((0.0f>fbmeh))
			{
				fbm16h = -fbmeh;
			}
			else
			{
				fbm16h = fbmeh;
			}
			fbm1eh = fbm16h;
			fValue = pow(10.0f,4-i_fbm6h);
			fValue = fbm1eh * fValue;
			lValue = (long)fValue;
			lValue = lValue % 10;
			buf_fbm5a[i_fbm6h] = (char)lValue;
			//sBuf[i_fbm6h] = (char)lValue;
			//buf_fbm5a[i_fbm6h]  = (char)((long)((fbm1eh*(pow(10.0,4-i_fbm6h))) )%10);
			i_fbm6h--;
					
		}
		buf_fbm5a[4] = 0x2E;		//'.'
		i_fbm6h = 0x000B;
		while(i_fbm6h > 5)
		{
			if(buf_fbm5a[i_fbm6h] > 0)
			{
				break;
			}
			i_fbm6h--;
		}
		int_fbm16h = i_fbm6h;
		int_fbmeh = 0;
		if(!(0.0f<=fb5h))
		{
			buf_fbm3c[int_fbmeh++] = 0x2D;		//	'-'
		}
		i_fbm6h = int_fbm16h;
		while(i_fbm6h > 4)
		{
			buf_fbm3c[int_fbmeh] = buf_fbm5a[i_fbm6h]+0x30;			//0x03 + 0x30 == '3'
			i_fbm6h--;
			int_fbmeh++;			
		}
		if(fb9h > 0)
		{
			buf_fbm3c[int_fbmeh++] = 0x2E;			//'.'
		}
		i_fbm6h = 3;
		while(i_fbm6h >= (4 - fb9h))
		{
			buf_fbm3c[int_fbmeh] = buf_fbm5a[i_fbm6h]+0x30;
			int_fbmeh++;
			i_fbm6h--;
		}
		buf_fbm3c[int_fbmeh] = 0;
		len_fbm4h = int_fbmeh;
	}
	else
	{
		sprintf(&buf_fbm3c[0],"%.3e",(double)fb5h);
		len_fbm4h = strlen(&buf_fbm3c[0]);		
	}
	i_fbm6h = 0;
	while(i_fbm6h < len_fbm4h)
	{
		if(buf_fbm3c[i_fbm6h] == 0x2E)
		{		//'.'
			switch(fbah)
			{
			case 1:	
				buffer[i_fbm6h] = 0x031B;
				break;
			case 5:
				buffer[i_fbm6h] = 0x0532;
				break;
			case 4:
				buffer[i_fbm6h] = 0x040A;
				break;
			case 7:
				buffer[i_fbm6h] = 0x070E;
				break;
			case 0x0E:
				buffer[i_fbm6h] = 0x0E0A;
				break;
			default:				
				break;
			}
		}
		else if(buf_fbm3c[i_fbm6h] == 0x2D)
		{		//'-'
			switch(fbah)
			{
			case 1:	
				buffer[i_fbm6h] = 0x031F;
				break;
			case 5:
				buffer[i_fbm6h] = 0x050C;
				break;
			case 4:
				buffer[i_fbm6h] = 0x040E;
				break;
			case 7:
				buffer[i_fbm6h] = 0x070D;
				break;
			case 0x0E:
				buffer[i_fbm6h] = 0x0E0E;
				break;
			default:				
				break;
			}
		}
		else if(buf_fbm3c[i_fbm6h] == 0x65)
		{// 	'e'
			switch(fbah)
			{
			case 1:	
				buffer[i_fbm6h] = 0x0304;
				break;
			case 5:
				buffer[i_fbm6h] = 0x052E;
				break;
			case 4:
				buffer[i_fbm6h] = 0x052E;
				break;
			default:				
				break;
			}
		}
		else if(buf_fbm3c[i_fbm6h] >= 0x30 && buf_fbm3c[i_fbm6h] <= 0x39)
		{
			buffer[i_fbm6h] = (((short)fbah)<<8) | (buf_fbm3c[i_fbm6h]- 0x20);
		}
		else
		{
			buffer[i_fbm6h] = 0x031A;
		}
		i_fbm6h++;
	}
	buffer[i_fbm6h] = 0xFFFF;
}

/* -------------------------------------------------------------------------------------------
  * @brief  把 ASCII字串翻译成 5号字库的编码
  * @param  a 需转换的地址指针，  
  *         b 转换后的存放位置  
  *         flag                    0:from string to bitmap
  *                                 1:from bitmap to string
  * @retval 
     char gfunc_DateString_process(char *str, short* bitmap, short c);        
 -------------------------------------------------------------------------------------------- */
 void gfunc_DateString_process( char* a, signed short* b)
 {
	char *  str_fbm2 = (char *) a;
	signed short * bitmap_fbm4 = b;
	short len_fbm6;
	short i_fbm8;
	short j;

	//from string to bitmap
	len_fbm6 = strlen((char *) str_fbm2);
	if(len_fbm6 != 0)
	{
		i_fbm8 = 0;
		j = 0;
		while(i_fbm8<len_fbm6)
		{
			if(str_fbm2[i_fbm8]==0x20||str_fbm2[i_fbm8]==0)
			{
				bitmap_fbm4[j] = 0x0700;					
			}
			else
			{
				if(str_fbm2[i_fbm8]==0x2B)
				{
					bitmap_fbm4[j] = 0x070B;						
				}
				else if(str_fbm2[i_fbm8]==0x2D)
				{
					bitmap_fbm4[j] = 0x070D;
				}
				else if(str_fbm2[i_fbm8]==0x5F)
				{
					bitmap_fbm4[j] = 0x073F;
				}
				else if(str_fbm2[i_fbm8]==0x2E)
				{
					bitmap_fbm4[j] = 0x070E;
				}
				else if(str_fbm2[i_fbm8]>=0x30 && str_fbm2[i_fbm8]<=0x39)
				{	//digit
					bitmap_fbm4[j] = (str_fbm2[i_fbm8]-0x20)|0x0700;
				}
				else if(str_fbm2[i_fbm8]>=0x41 && str_fbm2[i_fbm8]<=0x5A)
				{
					bitmap_fbm4[j] = (str_fbm2[i_fbm8]-0x20)|0x0700;
				}
				else if(str_fbm2[i_fbm8]== 0x73)
				{
					bitmap_fbm4[j] = 0x0753;
				}
				else if(str_fbm2[i_fbm8]==0x65)
				{
					bitmap_fbm4[j] = 0x0745;
				}
				else if(str_fbm2[i_fbm8]==0x6E)
				{
					bitmap_fbm4[j] = 0x074E;			
				}
				else if(str_fbm2[i_fbm8]==0x6F)
				{
					bitmap_fbm4[j] = 0x074F;		
				}
				else if(str_fbm2[i_fbm8]==0x72)
				{
					bitmap_fbm4[j] = 0x0752;				
				}
				else
				{
					bitmap_fbm4[j] = 0x0700;
				}
			}
			i_fbm8++;
			j++;
			
		}
	}
	else
	{
		j = 0;
		while(j<8)
		{
			bitmap_fbm4[j] = 0x070D;
			j++;
		}
	}
	bitmap_fbm4[j] = 0xffff;
	return ;
	
}


//开机界面，显示SK LOGO
void func_display_PowerOn_Menu(void)
{
	clear_screen();
	display_128x64((uint8_t *)&g_sSKLogo_New[0]);
	//display_128x64(1);	//显示SK LOGO
}


//显示授权通过界面
void func_display_Authorize_Menu(void)
{
	uint8_t i = 0;
	signed short sTestArr[20] = {0};
	clear_screen();
	memset(guc_OLED_Buf,0,sizeof(guc_OLED_Buf));
	
	for(i=0; i<10; i++)
	{
		sTestArr[i] = i + 0x000E;
	}
	sTestArr[10] = 0xFFFF;
	func_display_string(0,0,&sTestArr[0]);

	for(i=0; i<4; i++)
	{
		sTestArr[i] = i + 0x0018;
	}
	sTestArr[4] = 0xFFFF;
	func_display_string(0,24,&sTestArr[0]);

	func_Display_128x64(0);
}

//每次磁棒唤醒设备后，界面显示要进行权限认证界面
void func_diplay_NFCCheck_Menu(void)
{
	signed short sTestArr[20] = {0};
	clear_screen();
	memset(guc_OLED_Buf,0,sizeof(guc_OLED_Buf));
	sTestArr[0] = 0x001E;	//请
	sTestArr[1] = 0x0039;	//将
	sTestArr[2] = 0x0018;	//认
	sTestArr[3] = 0x0019;	//证
	sTestArr[4] = 0x003A;	//卡
	sTestArr[5] = 0x003B;	//贴
	sTestArr[6] = 0x003C;	//近
	sTestArr[7] = 0x000F;	//设
	sTestArr[8] = 0x0010;	//备
	sTestArr[9] = 0xFFFF;
	func_display_string(0,24,&sTestArr[0]);

	func_Display_128x64(0);
}

//显示日期时间，电池电量等状态信息界面
void func_DateTime_Battery_Status_View_Show()
{
	signed short sTestArr[20] = {0};
	char cDateTimeArr[25] = {0};
	uint8_t i = 0;
	clear_screen();
	memset(guc_OLED_Buf,0,sizeof(guc_OLED_Buf));
	
	//显示日期时间
	drv_mcu_Get_RTC_Time(cDateTimeArr);
	sTestArr[0] = cDateTimeArr[2] - 0x30 + 0x0710;	//年
	sTestArr[1] = cDateTimeArr[3] - 0x30 + 0x0710;
	sTestArr[2] = 0x070D;	//'-'
	sTestArr[3] = cDateTimeArr[5] - 0x30 + 0x0710;	//月
	sTestArr[4] = cDateTimeArr[6] - 0x30 + 0x0710;
	sTestArr[5] = 0x070D;
	sTestArr[6] = cDateTimeArr[8] - 0x30 + 0x0710;	//日
	sTestArr[7] = cDateTimeArr[9] - 0x30 + 0x0710;
	sTestArr[8] = 0x0700;	//' '
	sTestArr[9] = cDateTimeArr[11] - 0x30 + 0x0710;	//时
	sTestArr[10] = cDateTimeArr[12] - 0x30 + 0x0710;
	sTestArr[11] = 0x071A;	//':'
	sTestArr[12] = cDateTimeArr[14] - 0x30 + 0x0710;	//分
	sTestArr[13] = cDateTimeArr[15] - 0x30 + 0x0710;
	sTestArr[14] = 0x071A;	//':'
	sTestArr[15] = cDateTimeArr[17] - 0x30 + 0x0710;	//秒
	sTestArr[16] = cDateTimeArr[18] - 0x30 + 0x0710;
	sTestArr[17] = 0xFFFF;
	func_display_string(0,0,&sTestArr[0]);
	
	//显示电池电量
	//pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent = 39.0;
	if(pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent >= 80.0f)
	{
		sTestArr[0] = 0x0760;
	}
	else if(pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent >= 60.0f)
	{
		sTestArr[0] = 0x0761;
	}
	else if(pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent >= 40.0f)
	{
		sTestArr[0] = 0x0762;
	}
	else if(pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent >= 20.0f)
	{
		sTestArr[0] = 0x0763;
	}
	else
	{
		sTestArr[0] = 0x0764;
	}
	if(pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent >= 99.0f)
	{
		pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent = 99.0f;
	}
	sTestArr[1] = ((uint8_t)(pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent) / 10) + 0x0710;
	sTestArr[2] = ((uint8_t)(pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent) % 10) + 0x0710;
	sTestArr[3] = 0x0705;	//'.'
	//sTestArr[4] = ((uint16_t)(pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.fBattery_Level_Percent * 10) % 10) + 0x0710;
	sTestArr[4] = 0xFFFF;
	func_display_string(102,0,&sTestArr[0]);
	
	//显示其他状态信息
	//...

	//显示状态栏下方横线
	for(i=0; i<128; i++)
	{
		guc_OLED_Buf[i][1] |= 0x10; 
	}
}

//屏幕最下方显示当前设备状态
void func_Device_Status_View_Show(unsigned char ucCnt)
{
	signed short sTestArr[20] = {0};
	uint8_t i = 0;

	//设备状态栏上方横线
	for(i=0; i<128; i++)
	{
		guc_OLED_Buf[i][6] |= 0x02; 
	}
	i=0;
	if(ucCnt == 0)	//正常情况下
	{
		//pst_OLEDSystemPara->DeviceRunPara.usDevStatus = 0XFF;
		//根据设备当前运行状态参数，显示相关信息
		
		if(pst_OLEDSystemPara->DeviceRunPara.usDevStatus == 0x0000)	//正常
		{
			sTestArr[i++] = 0x0026;
			sTestArr[i++] = 0x0027;
		}
		else	//存在异常
		{
			sTestArr[i++] = 0x0028;
			sTestArr[i++] = 0x0027;
			sTestArr[i++] = 0x071A;	//':'
			if((pst_OLEDSystemPara->DeviceRunPara.usDevStatus & 0x01) == 0x01)	//4G初始化异常
			{
				//sTestArr[i++] = 0x0504;
				sTestArr[i++] = 0x0514;
				sTestArr[i++] = 0x0533;
			}
			if((pst_OLEDSystemPara->DeviceRunPara.usDevStatus & 0x02) == 0x02)	//NFC初始化异常
			{
				sTestArr[i++] = 0x051B;
				//sTestArr[i++] = 0x0513;
				//sTestArr[i++] = 0X0510;
				sTestArr[i++] = 0x0533;
			}
			if((pst_OLEDSystemPara->DeviceRunPara.usDevStatus & 0x04) == 0x04)	//姿态
			{
				sTestArr[i++] = 0X0000;
				//sTestArr[i++] = 0X0001;
				sTestArr[i++] = 0x0533;
			}
			if((pst_OLEDSystemPara->DeviceRunPara.usDevStatus & 0x08) == 0x08)	//水浸	
			{
				//sTestArr[i++] = 0X000A;
				sTestArr[i++] = 0X000B;
				sTestArr[i++] = 0x0533;
			}
			if((pst_OLEDSystemPara->DeviceRunPara.usDevStatus & 0x10) == 0x10)	//光照
			{
				sTestArr[i++] = 0X0006;
				//sTestArr[i++] = 0X0007;
				sTestArr[i++] = 0x0533;
			}
			if((pst_OLEDSystemPara->DeviceRunPara.usDevStatus & 0x20) == 0x20)	//MODBUS通信异常
			{
				sTestArr[i++] = 0X051A;	//M
				sTestArr[i++] = 0x0533;
			}
		}
	}
	else	//表示磁棒长时间接触，要显示关机提示信息
	{
		//关机中Xs
		sTestArr[i++] = 0x002C;
		sTestArr[i++] = 0x002D;
		sTestArr[i++] = 0x002E;
		sTestArr[i++] = 0x071A;	//':'
		sTestArr[i++] = ucCnt + 0x0500;
		sTestArr[i++] = 0x052D;
	}

	sTestArr[i] = 0xFFFF;
	func_display_string(0,51,&sTestArr[0]);
}

//显示设备未经过认证界面
void func_Not_Certified_View_Show(void)
{
	signed short sTestArr[20] = {0};
	func_DateTime_Battery_Status_View_Show();
	//显示提示信息：此设备未经过认证，请联系售后人员
	sTestArr[0] = 0x000E;
	sTestArr[1] = 0x000F;
	sTestArr[2] = 0x0010;
	sTestArr[3] = 0x001C;
	sTestArr[4] = 0x001D;
	sTestArr[5] = 0x001B;
	sTestArr[6] = 0x0018;
	sTestArr[7] = 0x0019;
	sTestArr[8] = 0xFFFF;

	func_display_string(0,20,&sTestArr[0]);

	sTestArr[0] = 0x001E;
	sTestArr[1] = 0x001F;
	sTestArr[2] = 0x0020;
	sTestArr[3] = 0x0021;
	sTestArr[4] = 0x0022;
	sTestArr[5] = 0x0023;
	sTestArr[6] = 0x0024;
	sTestArr[7] = 0xFFFF;
	func_display_string(0,40,&sTestArr[0]);

	func_Display_128x64(0);
}

//与服务器通讯模块状态显示界面
void func_Connect_Server_Status_View_Show(void)
{
	signed short sTestArr[20] = {0};
	//显示当前是否在上传数据到服务器标志
	if(pst_OLEDSystemPara->DeviceRunPara.enUploadStatus == Status_Register)
	{
		sTestArr[0] = 0x051F;	//R
	}
	else if(pst_OLEDSystemPara->DeviceRunPara.enUploadStatus == Status_Upload)
	{
		sTestArr[0] = 0x0522;	//U
	}
	else if(pst_OLEDSystemPara->DeviceRunPara.enUploadStatus == Status_Err)
	{
		sTestArr[0] = 0x0512;	//E
	}
	else
	{
		sTestArr[0] = 0x051C;	//O
	}
	sTestArr[1] = 0xFFFF;
	guc_LcdDipRevesAttr = 1;
	func_display_string(115,51,&sTestArr[0]);
	guc_LcdDipRevesAttr = 0;
}

//显示测量界面：液位+流量
void func_Measure_WaterLevel_View_Show(unsigned char ucCnt)
{
	signed short sTestArr[20] = {0};
	uint8_t i = 0;
	uint8_t l = 0;
	float fLevel = 0.0;
	float fFlow = 0.0;
	func_DateTime_Battery_Status_View_Show();

	//pst_OLEDSystemPara->DeviceRunPara.esMeasData.fWaterLevel_Radar = 1.234;
	//pst_OLEDSystemPara->DeviceRunPara.esMeasData.fVolumeValue = 5.678;
	//水位:
	sTestArr[0] = 0x000A;
	sTestArr[1] = 0x0025;
	sTestArr[2] = 0x071A;	//':'
	sTestArr[3] = 0xFFFF;
	func_display_string(0,17,&sTestArr[0]);
	memset(sTestArr,0,sizeof(sTestArr));
	for(l=0; l<2; l++)
	{
		for(i=0; i<pst_OLEDSystemPara->DevicePara.cMeasSensorCount[l]; i++)
		{
			if(pst_OLEDSystemPara->DevicePara.eMeasSensor[l][i] == Meas_BY_Radar_Level)
			{
				fLevel = pst_OLEDSystemPara->DeviceRunPara.esMeasData.esBY_LevelData.fRadarWaterLevelValue;
				break;
			}
			else if((pst_OLEDSystemPara->DevicePara.eMeasSensor[l][i] == Meas_HZ_Radar_Level))
			{
				fLevel = pst_OLEDSystemPara->DeviceRunPara.esMeasData.esHZ_LevelData.fRadarWaterLevelValue;
				break;
			}
			else if((pst_OLEDSystemPara->DevicePara.eMeasSensor[l][i] == Meas_BY_Pressure_Level))
			{
				fLevel = pst_OLEDSystemPara->DeviceRunPara.esMeasData.esBY_LevelData.fPressureWaterLevelValue;
				break;
			}
		}
	}
	

	func_sprintf_metricsystem(&sTestArr[0],fLevel,3,7);
	for(i=0; i<20; i++)
	{
		if(sTestArr[i] == -1)
		{
			sTestArr[i] = 0x0700;	//' '
			sTestArr[i+1] = 0x074D;	//m
			sTestArr[i+2] = 0xFFFF;
			break;
		}
	}
	func_display_string(35,20,&sTestArr[0]);

	//流量:
	sTestArr[0] = 0x0029;
	sTestArr[1] = 0x0005;
	sTestArr[2] = 0x071A;	//':'
	sTestArr[3] = 0xFFFF;
	func_display_string(0,33,&sTestArr[0]);
	memset(sTestArr,0,sizeof(sTestArr));
	for(l=0; l<2; l++)
	{
		for(i=0; i<pst_OLEDSystemPara->DevicePara.cMeasSensorCount[l]; i++)
		{
			if((pst_OLEDSystemPara->DevicePara.eMeasSensor[l][i] == Meas_Flowmeter)
			|| (pst_OLEDSystemPara->DevicePara.eMeasSensor[l][i] == Meas_HZ_Radar_Ultrasonic_Flow)
			|| (pst_OLEDSystemPara->DevicePara.eMeasSensor[l][i] == Meas_HZ_Ultrasonic_Flow)
			|| (pst_OLEDSystemPara->DevicePara.eMeasSensor[l][i] == Meas_HX_Radar_Ultrasonic_Flow)
			|| (pst_OLEDSystemPara->DevicePara.eMeasSensor[l][i] == Meas_HX_Flowmeter))
			{
				fFlow = pst_OLEDSystemPara->DeviceRunPara.esMeasData.fVolumeValue;
				break;
			}
		}
	}

	func_sprintf_metricsystem(&sTestArr[0],fFlow,3,7);
	for(i=0; i<20; i++)
	{
		if(sTestArr[i] == -1)
		{
			sTestArr[i] = 0x0700;	//' '
			sTestArr[i+1] = 0x074D;	//m
			sTestArr[i+2] = 0x070F;	//'/'
			sTestArr[i+3] = 0x0753;	//s
			sTestArr[i+4] = 0xFFFF;
			break;
		}
	}
	func_display_string(35,36,&sTestArr[0]);
	func_Connect_Server_Status_View_Show();
	func_Device_Status_View_Show(ucCnt);
	func_Display_128x64(0);
}

//显示测量界面：水质: 电导率+COD
void func_Measure_Water_Quality_View_Show(unsigned char ucCnt)
{
	signed short sTestArr[20] = {0};
	float fCOD = 0.0;
	float fCOND = 0.0;
	uint8_t i = 0;
	uint8_t l = 0;
	func_DateTime_Battery_Status_View_Show();

	//pst_OLEDSystemPara->DeviceRunPara.esMeasData.fWaterQuality_DDValue = 100.235;
	//pst_OLEDSystemPara->DeviceRunPara.esMeasData.fWaterQuality_CODValue = 56.182;

	//电导率: uS/cm
	sTestArr[0] = 0x002F;
	sTestArr[1] = 0x0030;
	sTestArr[2] = 0x0031;
	sTestArr[3] = 0x071A;	//':'
	sTestArr[4] = 0xFFFF;
	func_display_string(0,17,&sTestArr[0]);
	memset(sTestArr,0,sizeof(sTestArr));
	for(l=0; l<2; l++)
	{
		for(i=0; i<pst_OLEDSystemPara->DevicePara.cMeasSensorCount[l]; i++)
		{
			if((pst_OLEDSystemPara->DevicePara.eMeasSensor[l][i] == Meas_BY_Integrated_Conductivity))
			{
				fCOND = pst_OLEDSystemPara->DeviceRunPara.esMeasData.esBY_IntegratedConductivityData.fConductivityValue;
				break;
			}
		}
	}
	//fCOND = 99999.999;
	func_sprintf_metricsystem(&sTestArr[0],fCOND,2,7);
	for(i=0; i<20; i++)
	{
		if(sTestArr[i] == -1)	
		{
			sTestArr[i] = 0x0700;	//' '
			sTestArr[i+1] = 0x0755;	//u
			sTestArr[i+2] = 0x0733;	//S
			sTestArr[i+3] = 0x070F;	///
			sTestArr[i+4] = 0x0743;	//c
			sTestArr[i+5] = 0x074D;	//m
			sTestArr[i+6] = 0xFFFF;
			break;
		}
	}
	func_display_string(47,20,&sTestArr[0]);

	//COD: mg/L
	sTestArr[0] = 0x0510;
	sTestArr[1] = 0x051C;
	sTestArr[2] = 0x0511;
	sTestArr[3] = 0x071A;	//':'
	sTestArr[4] = 0xFFFF;
	func_display_string(0,33,&sTestArr[0]);
	memset(sTestArr,0,sizeof(sTestArr));
	for(l=0; l<2; l++)
	{
		for(i=0; i<pst_OLEDSystemPara->DevicePara.cMeasSensorCount[l]; i++)
		{
			if(pst_OLEDSystemPara->DevicePara.eMeasSensor[l][i] == Meas_HX_WaterQuality_COD)
			{
				fCOD = pst_OLEDSystemPara->DeviceRunPara.esMeasData.fWaterQuality_CODValue;
				break;
			}
		}
	}
	//fCOD = 99999.99;
	func_sprintf_metricsystem(&sTestArr[0],fCOD,3,7);
	for(i=0; i<20; i++)
	{
		if(sTestArr[i] == -1)
		{
			sTestArr[i] = 0x0700;	//' '
			sTestArr[i+1] = 0x074D;	//m
			sTestArr[i+2] = 0x0747;	//'g'
			sTestArr[i+3] = 0x070F;	//'/'
			sTestArr[i+4] = 0x072C;	//L
			sTestArr[i+5] = 0xFFFF;
			break;
		}
	}
	func_display_string(47,36,&sTestArr[0]);

	func_Connect_Server_Status_View_Show();
	func_Device_Status_View_Show(ucCnt);
	func_Display_128x64(0);
}

//显示界面：姿态+光敏
void func_Meas_Sensor_Value_View_Show(unsigned char ucCnt)
{
	signed short sTestArr[20] = {0};
	char cID[10] = {0};
	//uint8_t i = 0;
	func_DateTime_Battery_Status_View_Show();
	//姿态
	sTestArr[0] = 0x0000;
	sTestArr[1] = 0x0001;
	sTestArr[2] = 0x071A;	//':'
	sTestArr[3] = 0xFFFF;
	func_display_string(0,17,&sTestArr[0]);	
	func_sprintf_metricsystem(&sTestArr[0],(float)pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.nDev_Attitude_SC7A,3,7);
	func_display_string(35,20,&sTestArr[0]);

	//光敏
	sTestArr[0] = 0x0006;
	sTestArr[1] = 0x0007;
	sTestArr[2] = 0x071A;	//':'
	sTestArr[3] = 0xFFFF;
	func_display_string(0,33,&sTestArr[0]);	
	func_sprintf_metricsystem(&sTestArr[0],(float)pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.sPhotosensitive_XYC_ALS_Data,3,7);
	func_display_string(35,36,&sTestArr[0]);

	func_Connect_Server_Status_View_Show();
	func_Device_Status_View_Show(ucCnt);
	func_Display_128x64(0);
}

//显示界面：定位信息
void func_GPSData_View_Show(unsigned char ucCnt)
{
	signed short sTestArr[20] = {0};
	float fValue = 0.0;
	char cID[10] = {0};
	//uint8_t i = 0;
	func_DateTime_Battery_Status_View_Show();
	if(pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ulLatitude < 1)
	{
		//未定位
		sTestArr[0] = 0x001C;
		sTestArr[1] = 0x0036;
		sTestArr[2] = 0x0025;
		sTestArr[3] = 0xFFFF;
		func_display_string(0,17,&sTestArr[0]);	
	}
	else
	{
		//经度
		sTestArr[0] = 0x001D;
		sTestArr[1] = 0x0038;
		sTestArr[2] = 0x071A;	//':'
		sTestArr[3] = 0xFFFF;
		func_display_string(0,17,&sTestArr[0]);	
		fValue = (float)pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ulLongitude / 1000.0f;
		func_sprintf_metricsystem(&sTestArr[0],fValue,3,7);
		func_display_string(35,20,&sTestArr[0]);

		//纬度
		sTestArr[0] = 0x0037;
		sTestArr[1] = 0x0038;
		sTestArr[2] = 0x071A;	//':'
		sTestArr[3] = 0xFFFF;
		func_display_string(0,33,&sTestArr[0]);	
		fValue = (float)pst_OLEDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ulLatitude / 1000.0f;
		func_sprintf_metricsystem(&sTestArr[0],fValue,3,7);
		func_display_string(35,36,&sTestArr[0]);
	}

	func_Connect_Server_Status_View_Show();
	func_Device_Status_View_Show(ucCnt);
	func_Display_128x64(0);
}

//显示界面：设备ID+生产日期
void func_Device_ID_PD_View_Show(unsigned char ucCnt)
{
	signed short sTestArr[20] = {0};
	char cID[10] = {0};
	//uint8_t i = 0;
	func_DateTime_Battery_Status_View_Show();
	
	//序号: 
	sTestArr[0] = 0x0032;
	sTestArr[1] = 0x0033;
	sTestArr[2] = 0x071A;	//':'
	sTestArr[3] = 0xFFFF;
	func_display_string(0,17,&sTestArr[0]);	
	memcpy(cID,&pst_OLEDSystemPara->DevicePara.cDeviceID[2],4);
	memcpy(&cID[4],&pst_OLEDSystemPara->DevicePara.cDeviceID[10],6);
	gfunc_DateString_process(cID,sTestArr);
	func_display_string(35,20,&sTestArr[0]);

	//日期：
	sTestArr[0] = 0x0034;
	sTestArr[1] = 0x0035;
	sTestArr[2] = 0x071A;	//':'
	sTestArr[3] = 0xFFFF;
	func_display_string(0,33,&sTestArr[0]);	
	gfunc_DateString_process(pst_OLEDSystemPara->DevicePara.cDevicePDDate,sTestArr);
	func_display_string(35,36,&sTestArr[0]);

	func_Connect_Server_Status_View_Show();
	func_Device_Status_View_Show(ucCnt);
	func_Display_128x64(0);
}

//显示界面：软件版本+硬件版本
void func_Device_SW_HW_View_Show(unsigned char ucCnt)
{
	signed short sTestArr[20] = {0};
	//uint8_t i = 0;
	func_DateTime_Battery_Status_View_Show();
	
	//SW: 
	sTestArr[0] = 0x0520;
	sTestArr[1] = 0x0524;
	sTestArr[2] = 0x071A;	//':'
	sTestArr[3] = 0xFFFF;
	func_display_string(0,17,&sTestArr[0]);	
	gfunc_DateString_process(pst_OLEDSystemPara->DevicePara.cDeviceSWVersion,sTestArr);
	func_display_string(35,20,&sTestArr[0]);

	//HW：
	sTestArr[0] = 0x0515;
	sTestArr[1] = 0x0524;
	sTestArr[2] = 0x071A;	//':'
	sTestArr[3] = 0xFFFF;
	func_display_string(0,33,&sTestArr[0]);	
	gfunc_DateString_process(pst_OLEDSystemPara->DevicePara.cDeviceHWVersion,sTestArr);
	func_display_string(35,36,&sTestArr[0]);

	func_Connect_Server_Status_View_Show();
	func_Device_Status_View_Show(ucCnt);
	func_Display_128x64(0);
}


//开机中
void func_PowerOn_View_Show(unsigned char ucCnt)
{
	signed short sTestArr[20] = {0};
	clear_screen();
	
	sTestArr[0] = 0x002B;
	sTestArr[1] = 0x002D;
	sTestArr[2] = 0x002E;
	sTestArr[3] = 0x071A;	//':'
	sTestArr[4] = ucCnt + 0x0500;
	sTestArr[5] = 0x052D;
	sTestArr[6] = 0xFFFF;
	func_display_string(0,33,&sTestArr[0]);
	func_Display_128x64(0);
}

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/



/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
