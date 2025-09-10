/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\driver\drv_device\drv_IIC\drv_IIC.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-21       Joe             First version
 @endverbatim

 */

/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __DRV_IIC_H
#define __DRV_IIC_H

#include "hc32f460_gpio.h"
#include "hc32_ddl.h"
#include "User_Data.h"
#include "hc32f460_i2c.h"
#include "hc32f460.h"
#include "hc32f460_pwc.h"

#define Successful        0

#define  IICNACK           0xE2
#define  IICRadAddrNACK    0xE3
#define  IICSWError        0xE4
#define  IICRadLenError    0xE5
#define  SCL_TimeOut       0xE6

//加密芯片LKT4202
#ifdef HW_VERSION_V1_1
#define SM4_IIC_SDA_PORT  (PortC)
#define SM4_IIC_SDA_PIN   (Pin13)
#else
#define SM4_IIC_SDA_PORT  (PortA)
#define SM4_IIC_SDA_PIN   (Pin07)
#endif
#define  SM4_IIC_SDA_H            (PORT_SetBits(SM4_IIC_SDA_PORT, SM4_IIC_SDA_PIN))
#define  SM4_IIC_SDA_L            (PORT_ResetBits(SM4_IIC_SDA_PORT, SM4_IIC_SDA_PIN))

#ifdef HW_VERSION_V1_1
#define  SM4_IIC_SCL_PORT  (PortH)
#define  SM4_IIC_SCL_PIN   (Pin02)
#else
#define  SM4_IIC_SCL_PORT  (PortC)
#define  SM4_IIC_SCL_PIN   (Pin04)
#endif
#define  SM4_IIC_SCL_H            (PORT_SetBits(SM4_IIC_SCL_PORT, SM4_IIC_SCL_PIN))
#define  SM4_IIC_SCL_L            (PORT_ResetBits(SM4_IIC_SCL_PORT, SM4_IIC_SCL_PIN))

#ifdef HW_VERSION_V1_1
#define  SM4_IIC_RESET_PORT (PortE)
#define  SM4_IIC_RESET_PIN  (Pin06)
#else
#define  SM4_IIC_RESET_PORT (PortE)
#define  SM4_IIC_RESET_PIN  (Pin07)
#endif
#define  SM4_IIC_REST_H           (PORT_SetBits(SM4_IIC_RESET_PORT, SM4_IIC_RESET_PIN))
#define  SM4_IIC_REST_L           (PORT_ResetBits(SM4_IIC_RESET_PORT, SM4_IIC_RESET_PIN))

#define  SM4_IIC_SdaState             PORT_GetBit(SM4_IIC_SDA_PORT,SM4_IIC_SDA_PIN)
#define  SM4_IIC_SclState             PORT_GetBit(SM4_IIC_SCL_PORT,SM4_IIC_SCL_PIN)

//姿态传感器SC7A
#define  SC7A_IIC_SDA_PORT  (PortB)
#define  SC7A_IIC_SDA_PIN   (Pin05)
#define  SC7A_IIC_SDA_H            (PORT_SetBits(SC7A_IIC_SDA_PORT, SC7A_IIC_SDA_PIN))
#define  SC7A_IIC_SDA_L            (PORT_ResetBits(SC7A_IIC_SDA_PORT, SC7A_IIC_SDA_PIN))

#define  SC7A_IIC_SCL_PORT  (PortB)
#define  SC7A_IIC_SCL_PIN   (Pin06)
#define  SC7A_IIC_SCL_H            (PORT_SetBits(SC7A_IIC_SCL_PORT, SC7A_IIC_SCL_PIN))
#define  SC7A_IIC_SCL_L            (PORT_ResetBits(SC7A_IIC_SCL_PORT, SC7A_IIC_SCL_PIN))

#define  SC7A_IIC_SdaState             PORT_GetBit(SC7A_IIC_SDA_PORT,SC7A_IIC_SDA_PIN)
#define  SC7A_IIC_SclState             PORT_GetBit(SC7A_IIC_SCL_PORT,SC7A_IIC_SCL_PIN)

//光照传感器 XYC_ALS
#ifdef HW_VERSION_V1_1
#define  XYC_ALS_IIC_SDA_PORT  (PortB)
#define  XYC_ALS_IIC_SDA_PIN   (Pin02)
#else
#define  XYC_ALS_IIC_SDA_PORT  (PortB)
#define  XYC_ALS_IIC_SDA_PIN   (Pin01)
#endif
#define  XYC_ALS_IIC_SDA_H            (PORT_SetBits(XYC_ALS_IIC_SDA_PORT, XYC_ALS_IIC_SDA_PIN))
#define  XYC_ALS_IIC_SDA_L            (PORT_ResetBits(XYC_ALS_IIC_SDA_PORT, XYC_ALS_IIC_SDA_PIN))

#ifdef HW_VERSION_V1_1
#define  XYC_ALS_IIC_SCL_PORT  (PortB)
#define  XYC_ALS_IIC_SCL_PIN   (Pin00)
#else
#define  XYC_ALS_IIC_SCL_PORT  (PortB)
#define  XYC_ALS_IIC_SCL_PIN   (Pin02)
#endif
#define  XYC_ALS_IIC_SCL_H            (PORT_SetBits(XYC_ALS_IIC_SCL_PORT, XYC_ALS_IIC_SCL_PIN))
#define  XYC_ALS_IIC_SCL_L            (PORT_ResetBits(XYC_ALS_IIC_SCL_PORT, XYC_ALS_IIC_SCL_PIN))

#define  XYC_ALS_IIC_SdaState             PORT_GetBit(XYC_ALS_IIC_SDA_PORT,XYC_ALS_IIC_SDA_PIN)
#define  XYC_ALS_IIC_SclState             PORT_GetBit(XYC_ALS_IIC_SCL_PORT,XYC_ALS_IIC_SCL_PIN)

//OLED显示
#define  OLED_IIC_SDA_PORT  (PortD)
#define  OLED_IIC_SDA_PIN   (Pin01)
#define  OLED_IIC_SDA_H            (PORT_SetBits(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN))
#define  OLED_IIC_SDA_L            (PORT_ResetBits(OLED_IIC_SDA_PORT, OLED_IIC_SDA_PIN))

#define  OLED_IIC_SCL_PORT  (PortD)
#define  OLED_IIC_SCL_PIN   (Pin00)
#define  OLED_IIC_SCL_H            (PORT_SetBits(OLED_IIC_SCL_PORT, OLED_IIC_SCL_PIN))
#define  OLED_IIC_SCL_L            (PORT_ResetBits(OLED_IIC_SCL_PORT, OLED_IIC_SCL_PIN))

#define  OLED_IIC_SdaState             PORT_GetBit(OLED_IIC_SDA_PORT,OLED_IIC_SDA_PIN)
#define  OLED_IIC_SclState             PORT_GetBit(OLED_IIC_SCL_PORT,OLED_IIC_SCL_PIN)

#define I2C_XYC_ALS_UNIT                        (M4_I2C1)
/* Define slave device address for example */
#define DEVICE_XYC_ALS_ADDRESS                  (0x38u)
//#define I2C_10BITS_ADDRESS              (1u)

/* Define port and pin for SDA and SCL */
#define I2C_GPIO_XYC_ALS_SCL_FUNC               (Func_I2c1_Scl)
#define I2C_GPIO_XYC_ALS_SDA_FUNC               (Func_I2c1_Sda)

#define I2C_XYC_ALS_FCG_USE                     (PWC_FCG1_PERIPH_I2C1)

#define TIMEOUT                         (0x10000ul)

/* Define Write and read data length for the example */
/* Define i2c baudrate */
#define I2C_XYC_ALS_BAUDRATE                    (400000ul)

#define I2C_SC7A20_UNIT                        (M4_I2C3)
/* Define slave device address for example */
#define DEVICE_SC7A20_ADDRESS                  (0x18u)
//#define I2C_10BITS_ADDRESS              (1u)

/* Define port and pin for SDA and SCL */
#define I2C_GPIO_SC7A20_SCL_FUNC               (Func_I2c3_Scl)
#define I2C_GPIO_SC7A20_SDA_FUNC               (Func_I2c3_Sda)

#define I2C_SC7A20_FCG_USE                     (PWC_FCG1_PERIPH_I2C3)
#define I2C_SC7A20_BAUDRATE                    (100000ul)

//加密芯片LKT4202
extern uint8_t drv_mcu_SM4_IIC_Init(void);
extern void drv_SM4_IIC_Sendbyte(unsigned char *Sdata);
extern uint8_t drv_SM4_IIC_Recbyte(unsigned  char *Rdata);
extern uint8_t drv_SM4_IIC_WriteData(char  *Sendbuf , uint16_t len);
extern uint8_t drv_SM4_IIC_ReadData(char  *RecBuf, uint16_t* len);
extern void drv_SM4_IIC_Reset(void);
extern void drv_SM4_IIC_Start(void);
extern void drv_SM4_IIC_Stop(void);

extern void drv_SM4_IIC_Ack(void);
extern void drv_SM4_IIC_NAck(void);
extern void drv_SM4_IIC_Checkack(void);
extern uint8_t drv_SM4_IIC_SendApdu(uint16_t Sendlen,char* Sendbuf,char* RecBuf,uint16_t* Relen);

//姿态传感器SC7A
extern uint8_t drv_mcu_SC7A_IIC_Init(void);
extern void drv_SC7A_IIC_Start(void);
extern void drv_SC7A_IIC_Stop(void);
extern void drv_SC7A_IIC_ACKorNACK(uint8_t uAck);
extern uint8_t drv_SC7A_IIC_Checkack(void);

extern uint8_t drv_SC7A_IIC_Sendbyte(unsigned char *Sdata);
extern uint8_t drv_SC7A_IIC_Recbyte(unsigned  char *Rdata);

extern uint8_t drv_SC7A_WriteData(uint8_t ucCMD, uint8_t *ucDataArr, uint16_t usDataLen);
extern uint8_t drv_SC7A_ReadData(uint8_t ucCMD, uint8_t *ucDataArr, uint16_t usDataLen);

//光照传感器 XYC_ALS
extern uint8_t drv_mcu_XYC_ALS_IIC_Init(void);
extern void drv_XYC_ALS_IIC_Start(void);
extern void drv_XYC_ALS_IIC_Stop(void);
extern void drv_XYC_ALS_IIC_ACKorNACK(uint8_t uAck);
extern uint8_t drv_XYC_ALS_IIC_Checkack(void);

extern uint8_t drv_XYC_ALS_IIC_Sendbyte(unsigned char *Sdata);
extern uint8_t drv_XYC_ALS_IIC_Recbyte(unsigned  char *Rdata);

extern uint8_t drv_XYC_ALS_WriteData(uint8_t ucCMD, uint8_t *ucDataArr, uint16_t usDataLen);
extern uint8_t drv_XYC_ALS_ReadData(uint8_t ucCMD, uint8_t *ucDataArr, uint16_t usDataLen);

//OLED显示
extern uint8_t drv_mcu_OLED_IIC_Init(void);
extern void drv_OLED_IIC_Start(void);
extern void drv_OLED_IIC_Stop(void);
extern void drv_OLED_IIC_ACKorNACK(uint8_t uAck);
extern uint8_t drv_OLED_IIC_Checkack(void);

extern uint8_t drv_OLED_IIC_Sendbyte(unsigned char Sdata);

extern uint8_t drv_XYC_ALS_HARTIIC_Init(void);
extern en_result_t I2C_XYC_ALS_Master_Transmit(uint16_t u16DevAddr, uint8_t *pu8TxData, uint32_t u32Size, uint32_t u32TimeOut);
extern en_result_t I2C_XYC_ALS_Master_Receive(uint16_t u16DevAddr,uint8_t ucCMD,uint8_t *pu8RxData, uint32_t u32Size, uint32_t u32TimeOut);


extern uint8_t drv_SC7A20_HARTIIC_Init(void);
extern en_result_t I2C_SC7A20_Master_Transmit(uint16_t u16DevAddr, uint8_t *pu8TxData, uint32_t u32Size, uint32_t u32TimeOut);
extern en_result_t I2C_SC7A20_Master_Receive(uint16_t u16DevAddr,uint8_t ucCMD,uint8_t *pu8RxData, uint32_t u32Size, uint32_t u32TimeOut);

extern void drv_SC7A_Write_nByte(uint8_t REG_ADD, uint8_t *pBuff, uint8_t num);
extern void SC7A20TR_Read_nByte(unsigned char REG_ADD, unsigned char *pBuff, unsigned char num);
extern uint8_t drv_SC7A_Read_Byte(uint8_t REG_ADD);

#endif /* __DRV_IIC_H */

/************************END OF FILE*******************************************/


