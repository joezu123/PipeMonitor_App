/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\Photosensitive_XYC_ALS\Photosensitive_XYC_ALS.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-14       Joe             First version
 @endverbatim

 */
#ifndef __PHOTOSENSITIVE_XYC_ALS_H__
#define __PHOTOSENSITIVE_XYC_ALS_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "drv_IIC.h"
/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define SYSM_CTRL_REG   0x00  //系统控制寄存器:bit7->1:复位;  bit6->1：允许两次测量间加入等待时间; bit5->0:启用断电复位电路；bit1->1: 启用ALS功能一次;  bit0->1：启用ALS功能
#define INT_CTRL_REG    0x01  //中断引脚控制寄存器：bit4->1：当ALS中断产生时，测量将暂停到清除中断之前； bit0->1: ALS中断能影响INT引脚
#define INT_FLAG_REG    0x02  //中断状态寄存器：bit7->1:上电或复位时，INT引脚产生中断; bit6(只读)->0：此次数据合法; bit0->1:ALS中断触发
#define WAIT_TIME_REG   0x03  //等待时间寄存器:bit7:bit6->0x00->1*8ms; 0x03(max)->8*8ms; bit5:bit0:等待时间:0x00->1*(bit7:bit6);0x3f(max)->64*(bit7:bit6); 总等待时间=bit7:bit6 * bit5:bit0 * 8ms
#define ALS_GAIN_REG    0x04  //增益控制寄存器:bit7:PD增益选择；1->x2;  bit4:bit0：ALS传感增益:0x01->x1; 0x10(max)->x256
#define ALS_TIME_REG    0x05  //ALS综合时间寄存器
#define ALS_PER_REG     0x0B  //ALS持续性寄存器:bit4->0:选择ALS数据；bit3:bit0->持续多少次事件引起中断; 0->每次; 1->1次； 0xf(max):15次
#define ALS_THRES_LL    0x0C  //ALS比较下限阈值低位
#define ALS_THRES_LH    0x0D  //ALS比较下限阈值高位
#define ALS_THRES_HL    0x0E  //ALS比较上限阈值低位
#define ALS_THRES_HH    0x0F  //ALS比较上限阈值高位
#define DATA_STATUS_R   0x17  //只读; bit7->1:数据已更新，读取后清零； 
#define ALS_DATA_L_R    0x1E  //只读；ALS传感器转换完成后的数据低位
#define ALS_DATA_H_R    0x1F  //只读；ALS传感器转换完成后的数据高位


#ifdef HW_VERSION_V1_1
#define LTINT_PORT		PortB
#define LTINT_PIN		Pin01
#else
#define LTINT_PORT		PortE
#define LTINT_PIN		Pin08
#endif

#ifdef HW_VERSION_V1_1
#define VLT_GPIO_PORT    PortE
#define VLT_GPIO_PIN     Pin07
#else
#define VLT_GPIO_PORT    PortC
#define VLT_GPIO_PIN     Pin05
#endif

#define VLT_PIN_OPEN()    PORT_SetBits(VLT_GPIO_PORT, VLT_GPIO_PIN)
#define VLT_PIN_CLOSE()   PORT_ResetBits(VLT_GPIO_PORT, VLT_GPIO_PIN)

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/




/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern uint8_t drv_Photosensitive_XYC_ALS_Init(void);
extern uint16_t func_ReadPhoto_XYC_ALS_Data();

#ifdef __cplusplus
}
#endif

#endif /* __PHOTOSENSITIVE_XYC_ALS_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
