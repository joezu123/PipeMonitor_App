/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\driver\drv_device\drv_SPI\drv_SPI.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-20       Joe             First version
 @endverbatim

 */
#ifndef __DRV_SPI_H__
#define __DRV_SPI_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32f460.h"
#include "hc32f460_spi.h"
#include "hc32f460_gpio.h"
#include "hc32f460_pwc.h"
#include "hc32_ddl.h"
#include "User_Data.h"
/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define SPI_HART 1

/* SPI_SCK Port/Pin definition */
#ifdef HW_VERSION_V1_1
#define SPI_W25Q128_SCK_PORT                    (PortA)
#define SPI_W25Q128_SCK_PIN                     (Pin02)
#else
#define SPI_W25Q128_SCK_PORT                    (PortA)
#define SPI_W25Q128_SCK_PIN                     (Pin03)
#endif
#define SPI_W25Q128_SCK_FUNC                    (Func_Spi1_Sck)

/* SPI_NSS Port/Pin definition */
#ifdef HW_VERSION_V1_1
#define SPI_W25Q128_NSS_PORT                    (PortA)
#define SPI_W25Q128_NSS_PIN                     (Pin00)
#else
#define SPI_W25Q128_NSS_PORT                    (PortA)
#define SPI_W25Q128_NSS_PIN                     (Pin01)
#endif
#define SPI_W25Q128_NSS_FUNC                    (Func_Spi1_Nss0)

/* SPI_MOSI Port/Pin definition */
#ifdef HW_VERSION_V1_1
#define SPI_W25Q128_MISO_PORT                   (PortA)
#define SPI_W25Q128_MISO_PIN                    (Pin01)
#else
#define SPI_W25Q128_MISO_PORT                   (PortA)
#define SPI_W25Q128_MISO_PIN                    (Pin00)
#endif
#define SPI_W25Q128_MISO_FUNC                   (Func_Spi1_Miso)

/* SPI_MISO Port/Pin definition */
#ifdef HW_VERSION_V1_1
#define SPI_W25Q128_MOSI_PORT                   (PortA)
#define SPI_W25Q128_MOSI_PIN                    (Pin03)
#else
#define SPI_W25Q128_MOSI_PORT                   (PortA)
#define SPI_W25Q128_MOSI_PIN                    (Pin02)
#endif
#define SPI_W25Q128_MOSI_FUNC                   (Func_Spi1_Mosi)

#define SPI_W25_SCK_L                   (PORT_ResetBits(SPI_W25Q128_SCK_PORT, SPI_W25Q128_SCK_PIN))
#define SPI_W25_SCK_H                   (PORT_SetBits(SPI_W25Q128_SCK_PORT, SPI_W25Q128_SCK_PIN))

#define SPI_W25_SI_L                   (PORT_ResetBits(SPI_W25Q128_MOSI_PORT, SPI_W25Q128_MOSI_PIN))
#define SPI_W25_SI_H                   (PORT_SetBits(SPI_W25Q128_MOSI_PORT, SPI_W25Q128_MOSI_PIN))

#define SPI_W25_SO                   (PORT_GetBit(SPI_W25Q128_MISO_PORT, SPI_W25Q128_MISO_PIN))

/* SPI unit and clock definition */
#define SPI_W25Q128_UNIT                        (M4_SPI1)
#define SPI_W25Q128_UNIT_CLOCK                  (PWC_FCG1_PERIPH_SPI1)

#define SPI_W25Q128_NSS_HIGH()                  (PORT_SetBits(SPI_W25Q128_NSS_PORT, SPI_W25Q128_NSS_PIN))
#define SPI_W25Q128_NSS_LOW()                   (PORT_ResetBits(SPI_W25Q128_NSS_PORT, SPI_W25Q128_NSS_PIN))


#define SPI_LORA_RA02_SCK_PORT                    (PortB)
#define SPI_LORA_RA02_SCK_PIN                     (Pin10)
#define SPI_LORA_RA02_SCK_FUNC                    (Func_Spi3_Sck)

/* SPI_NSS Port/Pin definition */
#define SPI_LORA_RA02_NSS_PORT                    (PortD)
#define SPI_LORA_RA02_NSS_PIN                     (Pin08)
#define SPI_LORA_RA02_NSS_FUNC                    (Func_Spi3_Nss0)

/* SPI_MOSI Port/Pin definition */
#define SPI_LORA_RA02_MISO_PORT                   (PortB)
#define SPI_LORA_RA02_MISO_PIN                    (Pin14)
#define SPI_LORA_RA02_MISO_FUNC                   (Func_Spi3_Miso)

/* SPI_MISO Port/Pin definition */
#define SPI_LORA_RA02_MOSI_PORT                   (PortB)
#define SPI_LORA_RA02_MOSI_PIN                    (Pin15)
#define SPI_LORA_RA02_MOSI_FUNC                   (Func_Spi3_Mosi)

/* SPI unit and clock definition */
#define SPI_LORA_RA02_UNIT                        (M4_SPI3)
#define SPI_LORA_RA02_UNIT_CLOCK                  (PWC_FCG1_PERIPH_SPI3)

#define SPI_LORA_RA02_NSS_HIGH()                  (PORT_SetBits(SPI_LORA_RA02_NSS_PORT, SPI_LORA_RA02_NSS_PIN))
#define SPI_LORA_RA02_NSS_LOW()                   (PORT_ResetBits(SPI_LORA_RA02_NSS_PORT, SPI_LORA_RA02_NSS_PIN))

#define SPI_LORA_RA02_SCK_L                   (PORT_ResetBits(SPI_LORA_RA02_SCK_PORT, SPI_LORA_RA02_SCK_PIN))
#define SPI_LORA_RA02_SCK_H                   (PORT_SetBits(SPI_LORA_RA02_SCK_PORT, SPI_LORA_RA02_SCK_PIN))

#define SPI_LORA_RA02_SI_L                   (PORT_ResetBits(SPI_LORA_RA02_MISO_PORT, SPI_LORA_RA02_MISO_PIN))
#define SPI_LORA_RA02_SI_H                   (PORT_SetBits(SPI_LORA_RA02_MISO_PORT, SPI_LORA_RA02_MISO_PIN))

#define SPI_LORA_RA02_SO                   (PORT_GetBit(SPI_LORA_RA02_MOSI_PORT, SPI_LORA_RA02_MOSI_PIN))


/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/




/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/
extern uint8_t drv_mcu_SPI_Init(void);

#ifdef __cplusplus
} 
#endif

#endif /* __DRV_SPI_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
