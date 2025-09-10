/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\LC86L_BD\LC86L_BD.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-04-03       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "LC86L_BD.h"
#include "hc32f460_utility.h"
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
static SystemPataSt *pst_BDSystemPara;
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
//从buf里面得到第cx个逗号所在的位置
//返回值:0~0XFE,代表逗号所在位置的偏移.
//       0XFF,代表不存在第cx个逗号
uint8_t NMEA_Comma_Pos(uint8_t *buf,uint8_t cx)
{
	uint8_t *p=buf;
  	while(cx)
  	{
    	if((*buf=='*') || (*buf<' ') || (*buf>'z'))//遇到'*'或者非法字符,则不存在第cx个逗号
		{
			return 0XFF;
		}
    	if(*buf==',')
		{
			cx--;
		}
    	buf++;
  	}
  	return (buf-p);
}

//m^n函数
//返回值:m^n次方.
uint32_t NMEA_Pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;
  	while(n--)
	{
		result*=m;
	}
  	return result;
}

//str转换为数字,以','或者'*'结束
//buf:数字存储区
//dx:小数点位数,返回给调用函数
//返回值:转换后的数值
/*遇到冒号以及竖杠、注释的斜杠的时候进行返回*/
int NMEA_Str2num(uint8_t *buf,uint8_t*dx)
{
	uint8_t *p=buf;
  	uint32_t ires=0,fres=0;
  	uint8_t ilen=0,flen=0,i;
  	uint8_t mask=0;
  	int res;
  	while(1) //得到整数和小数的长度
  	{
    	if(*p=='-')//是负数
		{
			mask |= 0X02;
			p++;
		}
    	if((*p==',') || (*p=='*') || (*p=='|') || (*p==':') || (*p=='!') || (*p=='/'))
		{
			break;//遇到结束了
		}
			
    	if(*p=='.')	//遇到小数点了
		{
			mask |= 0X01;
			p++;
		}
    	else if(*p == 0)//截至符 0
    	{
      		break;
    	}
    	else if(*p>'9'||(*p<'0'))  //有非法字符
    	{
      		ilen=0;
      		flen=0;
      		break;
    	}
    	if(mask&0X01)
		{
			flen++;
		}
    	else 
		{
			ilen++;
		}
    	p++;
  	}
  	if(mask&0X02)
	{
		buf++;  //去掉负号
	}
  	for(i=0;i<ilen;i++)  //得到整数部分数据
  	{
    	ires += NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');//
  	}
  	if(flen>5)  //最多取5位小数
	{
		flen = 5;
	}
	*dx = flen;       //小数点位数
  	for(i=0;i<flen;i++)  //得到小数部分数据
  	{
    	fres += NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
  	}
  	res = ires*NMEA_Pow(10,flen)+fres;
  	if(mask&0X02)
	{
		res=-res;
	}
  	return res;
}    


//分析GPRMC信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
//$GNRMC,081729.804,V,,,,,,,111022,,,N,V*21<CR><LF>
void NMEA_GPRMC_Analysis(uint8_t *buf)
{
	uint8_t *p1,dx;
	uint8_t posx;
  	uint32_t temp;
  	float rs;  
	dx = 1;
  	p1 = (uint8_t*)strstr((const char *)buf,"BDRMC"); //GNSS
  	if(p1 == NULL)
  	{
    	p1=(uint8_t*)strstr((const char *)buf,"GNRMC");//"$GPRMC",经常有&和GPRMC分开的情况,故只判断GPRMC.
  	}
	
	if(p1 != NULL)
	{
		posx=NMEA_Comma_Pos(p1, 1);                //得到UTC时间  hhmmss.ss
		if(posx!=0XFF)
		{
			temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);     //得到UTC时间,去掉ms
			pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.esUTC.ucHour = temp/10000;
			pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.esUTC.ucMin = (temp/100)%100;
			pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.esUTC.ucSec = temp%100;
		}
	
		posx=NMEA_Comma_Pos(p1,2);/*判断RMC数据状态,A=数据有效 V=数据无效*/
		if(posx!=0XFF)
		{
			uint8_t* p2=(uint8_t*)strstr((const char *)(p1+posx), "A");
			if(p2 == NULL)
			{
				posx = 0;  //数据无效 TODO
				//pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ucDataValidFlag = 0;
				return;
			}
		}
		
	
		posx = NMEA_Comma_Pos(p1,3);                //得到纬度 ddmm.mmmm
		if(posx!=0XFF)
		{
			temp = NMEA_Str2num(p1+posx,&dx);
			pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ulLatitude=temp/NMEA_Pow(10,dx+2);  //得到°
			rs = temp % NMEA_Pow(10,dx+2);        //得到'
			pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ulLatitude=pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ulLatitude*NMEA_Pow(10,5)+(uint32_t)(rs*NMEA_Pow(10,5-dx))/60;//转换为°
			pst_BDSystemPara->DeviceRunPara.esDeviceRunState.fDevLoca_lat = (double)pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ulLatitude / 100000.0;
			if(pst_BDSystemPara->DeviceRunPara.esDeviceRunState.fDevLoca_lat > 1.0)
			{
				pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ucDataValidFlag = 1;
			}
		}
		posx=NMEA_Comma_Pos(p1,4);                //南纬还是北纬
		if(posx!=0XFF)
		{
			pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ucNshemi=*(p1+posx);  
			if(pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ucNshemi == 'S')
			{
				pst_BDSystemPara->DeviceRunPara.esDeviceRunState.fDevLoca_lat = -1.0 * pst_BDSystemPara->DeviceRunPara.esDeviceRunState.fDevLoca_lat;
			}
		}
				
		posx = NMEA_Comma_Pos(p1,5);                //得到经度 dddmm.mmmm
		if(posx!=0XFF)
		{
			temp = NMEA_Str2num(p1+posx,&dx);
			pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ulLongitude = temp / NMEA_Pow(10,dx+2);  //得到°
			rs = temp % NMEA_Pow(10,dx+2);        //得到'
			pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ulLongitude
				= pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ulLongitude*NMEA_Pow(10,5)+(uint32_t)(rs*NMEA_Pow(10,5-dx))/60;//转换为°
			pst_BDSystemPara->DeviceRunPara.esDeviceRunState.fDevLoca_lng = (double)pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ulLongitude / 100000.0;
			
		}
		posx = NMEA_Comma_Pos(p1,6);                //东经还是西经
		if(posx!=0XFF)
		{
			pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ucEwhemi=*(p1+posx); 
			if(pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.ucEwhemi == 'W')
			{
				pst_BDSystemPara->DeviceRunPara.esDeviceRunState.fDevLoca_lng =  -1.0 * pst_BDSystemPara->DeviceRunPara.esDeviceRunState.fDevLoca_lng;
			}
		}
	
		#if 0
		posx=NMEA_Comma_Pos(p1,8);                //得到方位 度
		if(posx!=0XFF)
		{
			temp=NMEA_Str2num(p1+posx,&dx);
			pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.usCourse = temp*10;
		}
	
		posx=NMEA_Comma_Pos(p1, 9);                //得到UTC日期 ddmmyy
		if(posx!=0XFF)
		{
			temp=NMEA_Str2num(p1+posx, &dx);
			pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.esUTC.ucDate = temp/10000;
			pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.esUTC.ucMonth = (temp/100)%100;
			pst_BDSystemPara->DeviceRunPara.esDeviceSensorsData.esBD_NEMAData.esUTC.usYear  = 2000+temp%100;
		}
		#endif
	}
  	
}

void drv_LC86L_SendSDKCMD(uint8_t *ucDataArr, uint16_t usDataLen)
{
	int len = strlen( (char*)ucDataArr );
    uint8_t cTemp[50];
    
    memcpy( cTemp, ucDataArr,len );

    cTemp[len] = '\r';
    cTemp[len+1] = '\n';

	//memset(pst_BTSystemPara->UsartData.ucUsartxRecvDataArr[1], 0, USART_DATA_LEN_MAX);
	drv_mcu_USART_SendData(MODULE_BD, cTemp, usDataLen+2);
}

//static char g_AT_PQBAUD_CMD[30] = "$PQBAUD,W,9600*43";        //bd设置波特率
static char g_AT_SETBD_CMD[30] = "$PMTK353,1,0,0,0,1*2B";        //bd 设置卫星类型:GPS+BDS
//static char g_AT_PQTXT_CMD[30] = "$PQTXT,W,1,1*22";        //bd 设置文本输出
//static char g_AT_QGPSCFG_CMD[30] = "AT+QGPSCFG=?";        //bd 设置卫星类型:GPS+BDS
uint8_t drv_LC86L_BD_Init(void)
{
	#if 0
	stc_port_init_t stcPortInit;

	MEM_ZERO_STRUCT(stcPortInit);

	stcPortInit.enPinMode = Pin_Mode_Out; 

    
	PORT_Init(BDRST_GPIO_PORT, BDRST_GPIO_PIN, &stcPortInit);

	//开启供电
	PWRBD_PIN_CLOSE();
	Ddl_Delay1ms(100);
	PWRBD_PIN_OPEN();
	Ddl_Delay1ms(1000);
	
	

	//复位模块
	BDRST_PIN_SET;
	Ddl_Delay1ms(100);
	BDRST_PIN_RESET;
	Ddl_Delay1ms(1000);
	BDRST_PIN_SET;
	Ddl_Delay1ms(100);
	#endif
	pst_BDSystemPara = GetSystemPara();
	//Ddl_Delay1ms(2500);
	//drv_LC86L_SendSDKCMD((uint8_t *)g_AT_PQTXT_CMD,15);

	//drv_LC86L_SendSDKCMD((uint8_t *)g_AT_SETBD_CMD,21);
	//Ddl_Delay1ms(5000);

	return 0;
}

void TurnOff_LC86L_BD_Power(void)
{
	PWRBD_PIN_CLOSE();
}

void TurnOn_LC86L_BD_Power(void)
{
	PWRBD_PIN_OPEN();
}

void func_BD_Test(void)
{
	//drv_LC86L_SendSDKCMD((uint8_t *)g_AT_PQBAUD_CMD,17);
	//drv_LC86L_SendSDKCMD((uint8_t *)g_AT_SETBD_CMD,21);
	//Ddl_Delay1ms(500);
}

void func_BD_PowerUp_Init(void)
{
	stc_port_init_t stcPortInit;

	MEM_ZERO_STRUCT(stcPortInit);

	stcPortInit.enPinMode = Pin_Mode_Out;
	//BD 复位引脚
    PORT_Init(BDRST_GPIO_PORT, BDRST_GPIO_PIN, &stcPortInit);
	PORT_Init(USART3_RX_BD_PORT, USART3_RX_BD_PIN, &stcPortInit);
	PORT_Init(USART3_TX_BD_PORT, USART3_TX_BD_PIN, &stcPortInit);
}

void func_BD_PownDown_Deinit(void)
{
	stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Ana;
	PORT_Init(BDRST_GPIO_PORT, BDRST_GPIO_PIN, &stcPortInit);
	PORT_Init(USART3_RX_BD_PORT, USART3_RX_BD_PIN, &stcPortInit);
	PORT_Init(USART3_TX_BD_PORT, USART3_TX_BD_PIN, &stcPortInit);

	PWRBD_PIN_CLOSE();	//关闭北斗模块电源
}

/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
