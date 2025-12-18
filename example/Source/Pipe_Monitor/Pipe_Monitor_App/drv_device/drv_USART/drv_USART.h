/**
 *******************************************************************************
 * @file  Pipe_Monitor_App\drivers\device_drv\drv_USART.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-13       Joe             First version
 @endverbatim

 */
#ifndef __DRV_USART_H__
#define __DRV_USART_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32f460_gpio.h"
#include "hc32f460_usart.h"
#include "hc32f460_dmac.h"
#include "hc32f460_interrupts.h"
#include "hc32f460_pwc.h"
#include "hc32f460.h"
#include "drv_Timer.h"
#include "User_Data.h"

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
/*DMA define */
#define DMA1_CH0_BTCINT_NUM                (INT_DMA1_BTC0)
#define DMA1_CH0_BTCINT_IRQn               (Int020_IRQn)

#define DMA1_CH1_BTCINT_NUM                (INT_DMA1_BTC1)
#define DMA1_CH1_BTCINT_IRQn               (Int021_IRQn)

#define DMA1_CH2_BTCINT_NUM                (INT_DMA1_BTC2)
#define DMA1_CH2_BTCINT_IRQn               (Int022_IRQn)

#define DMA1_CH3_BTCINT_NUM                (INT_DMA1_BTC3)
#define DMA1_CH3_BTCINT_IRQn               (Int023_IRQn)

/*USART define*/

#ifdef HW_VERSION_V1_1
#ifdef LORA_TP1109
#define USART1_RX_PORT                  (PortA)
#define USART1_RX_PIN                   (Pin08)
#else
#define USART1_RX_PORT                  (PortA)
#define USART1_RX_PIN                   (Pin07)
#define USART1_RX_FUNC                  (Func_Usart1_Tx)
#endif
#else
#define USART1_RX_PORT                  (PortC)
#define USART1_RX_PIN                   (Pin00)
#define USART1_RX_FUNC                  (Func_Usart1_Rx)
#endif


#ifdef HW_VERSION_V1_1
#ifdef LORA_TP1109
#define USART1_TX_PORT                  (PortC)
#define USART1_TX_PIN                   (Pin09)
#else
#define USART1_TX_PORT                  (PortC)
#define USART1_TX_PIN                   (Pin04)
#define USART1_TX_FUNC                  (Func_Usart1_Rx)
#endif
#else
#define USART1_TX_PORT                  (PortC)
#define USART1_TX_PIN                   (Pin01)
#define USART1_TX_FUNC                  (Func_Usart1_Tx)
#endif


#ifdef LORA_TP1109
#define USART1_BAUDRATE                 (9600ul)
#else
#define USART1_BAUDRATE                 (115200ul)
#endif

#define USART1_RI_NUM                  (INT_USART1_RI)
#define USART1_RI_IRQn                 (Int010_IRQn)

#define USART1_EI_NUM                   (INT_USART1_EI)
#define USART1_EI_IRQn                  (Int080_IRQn)
#define USART1_RTO_NUM                  (INT_USART1_RTO)
#define USART1_RTO_IRQn                 (Int081_IRQn)
#define USART1_TI_NUM                   (INT_USART1_TI)
#define USART1_TI_IRQn                  (Int082_IRQn)
#define USART1_TCI_NUM                  (INT_USART1_TCI)
#define USART1_TCI_IRQn                 (Int083_IRQn)

#define USART2_RX_PORT                  (PortA)
#define USART2_RX_PIN                   (Pin06)
#define USART2_RX_FUNC                  (Func_Usart2_Rx)

#define USART2_TX_PORT                  (PortA)
#define USART2_TX_PIN                   (Pin05)
#define USART2_TX_FUNC                  (Func_Usart2_Tx)

#define USART2_BAUDRATE                 (115200ul)
#define USART2_EI_NUM                   (INT_USART2_EI)
#define USART2_EI_IRQn                  (Int084_IRQn)
#define USART2_RI_NUM                   (INT_USART2_RI)
#define USART2_RTO_NUM                  (INT_USART2_RTO)
#define USART2_RTO_IRQn                 (Int085_IRQn)
#define USART2_TI_NUM                   (INT_USART2_TI)
#define USART2_TI_IRQn                  (Int024_IRQn)
#define USART2_TCI_NUM                  (INT_USART2_TCI)
#define USART2_TCI_IRQn                 (Int025_IRQn)

#ifdef HW_VERSION_V1_1
#define USART3_RX_BD_PORT                  (PortE)
#define USART3_RX_BD_PIN                   (Pin00)
#else
#define USART3_RX_BD_PORT                  (PortC)
#define USART3_RX_BD_PIN                   (Pin08)
#endif

#define USART3_RX_RFID_PORT                  (PortE)
#define USART3_RX_RFID_PIN                   (Pin13)

#define USART3_RX_FUNC                  (Func_Usart3_Rx)

#ifdef HW_VERSION_V1_1
#define USART3_TX_BD_PORT                  (PortB)
#define USART3_TX_BD_PIN                   (Pin09)
#else
#define USART3_TX_BD_PORT                  (PortC)
#define USART3_TX_BD_PIN                   (Pin07)
#endif

#define USART3_TX_RFID_PORT                  (PortE)
#define USART3_TX_RFID_PIN                   (Pin12)

#define USART3_TX_FUNC                  (Func_Usart3_Tx)

#define USART3_BAUDRATE                 (9600ul)
#define USART3_EI_NUM                   (INT_USART3_EI)
#define USART3_EI_IRQn                  (Int086_IRQn)
#define USART3_RI_NUM                  (INT_USART3_RI)
#define USART3_RTO_NUM                  (INT_USART3_RTO)
#define USART3_RTO_IRQn                 (Int087_IRQn)
#define USART3_TI_NUM                   (INT_USART3_TI)
#define USART3_TI_IRQn                  (Int088_IRQn)
#define USART3_TCI_NUM                  (INT_USART3_TCI)
#define USART3_TCI_IRQn                 (Int089_IRQn)

#define USART4_RX_485_1_PORT                  (PortE)
#define USART4_RX_485_1_PIN                   (Pin14)

#ifdef HW_VERSION_V1_1
#define USART4_RX_485_2_PORT                  (PortD)
#define USART4_RX_485_2_PIN                   (Pin10)
#else
#define USART4_RX_485_2_PORT                  (PortD)
#define USART4_RX_485_2_PIN                   (Pin11)
#endif

#define USART4_RX_FUNC                  (Func_Usart4_Rx)

#define USART4_TX_485_1_PORT                  (PortE)
#define USART4_TX_485_1_PIN                   (Pin15)

#ifdef HW_VERSION_V1_1
#define USART4_TX_485_2_PORT                  (PortD)
#define USART4_TX_485_2_PIN                   (Pin11)
#else
#define USART4_TX_485_2_PORT                  (PortD)
#define USART4_TX_485_2_PIN                   (Pin10)
#endif

#define USART4_TX_FUNC                  (Func_Usart4_Tx)

#define USART4_BAUDRATE                 (9600ul)
#define USART4_EI_NUM                   (INT_USART4_EI)
#define USART4_EI_IRQn                  (Int090_IRQn)
#define USART4_RTO_NUM                  (INT_USART4_RTO)
#define USART4_RTO_IRQn                 (Int091_IRQn)
#define USART4_RI_NUM                   (INT_USART4_RI)
#define USART4_TI_NUM                   (INT_USART4_TI)
#define USART4_TI_IRQn                  (Int026_IRQn)
#define USART4_TCI_NUM                  (INT_USART4_TCI)
#define USART4_TCI_IRQn                 (Int027_IRQn)

//RFID_NFC  复位等引脚定义
#ifdef HW_VERSION_V1_1
#define PWRRF_GPIO_PORT	PortE
#define PWRRF_GPIO_PIN	Pin09
#else
#define PWRRF_GPIO_PORT	PortD
#define PWRRF_GPIO_PIN	Pin05
#endif
#define PWRRF_PIN_OPEN()   PORT_SetBits(PWRRF_GPIO_PORT,PWRRF_GPIO_PIN)
#define PWRRF_PIN_CLOSE() PORT_ResetBits(PWRRF_GPIO_PORT,PWRRF_GPIO_PIN)

#ifdef HW_VERSION_V1_1
#define RFRST_PORT  PortE
#define RFRST_PIN   Pin10
#else
#define RFRST_PORT  PortE
#define RFRST_PIN   Pin11
#endif
#define RFRST_PIN_SET   PORT_SetBits(RFRST_PORT,RFRST_PIN)
#define RFRST_PIN_RESET PORT_ResetBits(RFRST_PORT,RFRST_PIN)

//BD模块引脚定义
#ifdef HW_VERSION_V1_1
#define PWRBD_GPIO_PORT  PortD
#define PWRBD_GPIO_PIN   Pin06
#else
#define PWRBD_GPIO_PORT  PortB
#define PWRBD_GPIO_PIN   Pin09
#endif

#define PWRBD_PIN_OPEN() PORT_SetBits(PWRBD_GPIO_PORT, PWRBD_GPIO_PIN)
#define PWRBD_PIN_CLOSE() PORT_ResetBits(PWRBD_GPIO_PORT, PWRBD_GPIO_PIN)

#ifdef HW_VERSION_V1_1
#define BDRST_GPIO_PORT  PortD
#define BDRST_GPIO_PIN   Pin05
#else
#define BDRST_GPIO_PORT  PortD
#define BDRST_GPIO_PIN   Pin15
#endif

#define BDRST_PIN_SET   PORT_SetBits(BDRST_GPIO_PORT, BDRST_GPIO_PIN)
#define BDRST_PIN_RESET PORT_ResetBits(BDRST_GPIO_PORT, BDRST_GPIO_PIN)

#ifdef HW_VERSION_V1_1
#define RTS1_GPIO_PORT  PortB
#define RTS1_GPIO_PIN   Pin10
#else
#define RTS1_GPIO_PORT  PortE
#define RTS1_GPIO_PIN   Pin09
#endif

#define RST1_PIN_OPEN() PORT_SetBits(RTS1_GPIO_PORT,RTS1_GPIO_PIN)
#define RST1_PIN_CLOSE()  PORT_ResetBits(RTS1_GPIO_PORT,RTS1_GPIO_PIN)

#define RTS2_GPIO_PORT  PortD
#define RTS2_GPIO_PIN   Pin12

#define RST2_PIN_OPEN() PORT_SetBits(RTS2_GPIO_PORT,RTS2_GPIO_PIN)
#define RST2_PIN_CLOSE()  PORT_ResetBits(RTS2_GPIO_PORT,RTS2_GPIO_PIN)
/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/

extern void drv_mcu_USART_Init(void);
extern void drv_mcu_USART_SendData(en_usart_device_t e_Module_Type, uint8_t *pu8Buff, uint16_t u16Len);
extern void drv_mcu_SetUSART3_Baud(void);
extern void drv_mcu_ChangeUSART3_Source(en_usart_device_t e_Module_Type);
extern void drv_mcu_ChangeUSART4_Source(en_usart_device_t e_Module_Type,unsigned long ulBaud);
extern void drv_mcu_ChangeUSART4_Baud(unsigned long ulValue);

extern void drv_RF_NFC_SendData(uint8_t ucCMD, uint8_t ucData);
extern uint8_t drv_RF_NFC_ReceiveData(uint8_t ucCMD);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_USART_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
