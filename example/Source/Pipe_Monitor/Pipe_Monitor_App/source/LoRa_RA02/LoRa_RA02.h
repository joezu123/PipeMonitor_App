/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\LoRa_RA02\LoRa_RA02.h
 * @brief This file contains all the functions prototypes of the communication
 *        device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-04-01       Joe             First version
 @endverbatim

 */
#ifndef __LORA_RA02_H__
#define __LORA_RA02_H__

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "drv_SPI.h"
#include "hc32f460.h"
#include "hc32f460_gpio.h"
#include "User_Data.h"
/*******************************************************************************
 * Global pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define LORA_RA02_RST_PORT              (PortD)
#define LORA_RA02_RST_PIN               (Pin09)

#define PWRLORA_GPIO_PORT              (PortD)
#define PWRLORA_GPIO_PIN               (Pin13)

#define PWRLORA_PIN_OPEN()       PORT_SetBits(PWRLORA_GPIO_PORT, PWRLORA_GPIO_PIN)
#define PWRLORA_PIN_CLOSE()      PORT_ResetBits(PWRLORA_GPIO_PORT, PWRLORA_GPIO_PIN)

/*******************************************************************************
 * Global type definitions ('typedef')
 ******************************************************************************/
#define TRANSMIT_TIMEOUT		2000
#define RECEIVE_TIMEOUT			2000

//--------- MODES ---------//
#define SLEEP_MODE			0
#define	STNBY_MODE			1
#define TRANSMIT_MODE			3
#define RXCONTIN_MODE			5
#define RXSINGLE_MODE			6

//------- BANDWIDTH -------//
#define BW_7_8KHz			0
#define BW_10_4KHz			1
#define BW_15_6KHz			2
#define BW_20_8KHz			3
#define BW_31_25KHz			4
#define BW_41_7KHz			5
#define BW_62_5KHz			6
#define BW_125KHz			7
#define BW_250KHz			8
#define BW_500KHz			9

//------ CODING RATE ------//
#define CR_4_5				1
#define CR_4_6				2
#define CR_4_7				3
#define CR_4_8				4

//--- SPREADING FACTORS ---//
#define SF_7				7
#define SF_8				8
#define SF_9				9
#define SF_10				10
#define SF_11  				11
#define SF_12				12

//------ POWER GAIN ------//
#define POWER_11db			0xF6
#define POWER_14db			0xF9
#define POWER_17db			0xFC
#define POWER_20db			0xFF

//------- REGISTERS -------//
#define RegFiFo				0x00
#define RegOpMode			0x01
#define RegFrMsb			0x06
#define RegFrMid			0x07
#define RegFrLsb			0x08
#define RegPaConfig			0x09
#define RegOcp				0x0B
#define RegLna				0x0C
#define RegFiFoAddPtr			0x0D
#define RegFiFoTxBaseAddr		0x0E
#define RegFiFoRxBaseAddr		0x0F
#define RegFiFoRxCurrentAddr	0x10
#define RegIrqFlags				0x12
#define RegRxNbBytes			0x13
#define RegPktRssiValue			0x1A
#define	RegModemConfig1			0x1D
#define RegModemConfig2			0x1E
#define RegSymbTimeoutL			0x1F
#define RegPreambleMsb			0x20
#define RegPreambleLsb			0x21
#define RegPayloadLength		0x22
#define RegModemConfig3			0x26
#define RegSyncWord				0x39
#define RegDioMapping1			0x40
#define RegDioMapping2			0x41
#define RegVersion			0x42

//------ LORA STATUS ------//
#define LORA_OK				200
#define LORA_NOT_FOUND			404
#define LORA_LARGE_PAYLOAD		413
#define LORA_UNAVAILABLE		503

typedef struct LoRa_setting{
	
	// Hardware setings:
	en_port_t		CS_port;
	uint16_t		CS_pin;
	en_port_t		reset_port;
	uint16_t		reset_pin;
	//en_port_t*		DIO0_port;
	//uint16_t		DIO0_pin;
	M4_SPI_TypeDef*	hSPIx;
	
	// Module settings:
	int			current_mode;
	int 			frequency;
	uint8_t			spredingFactor;
	uint8_t			bandWidth;
	uint8_t			crcRate;
	uint16_t		preamble;
	uint8_t			power;
	uint8_t			overCurrentProtection;
	
} LoRa;

LoRa newLoRa(void);
void LoRa_reset(LoRa* _LoRa);
void LoRa_readReg(LoRa* _LoRa, uint8_t* address, uint16_t r_length, uint8_t* output, uint16_t w_length);
void LoRa_writeReg(LoRa* _LoRa, uint8_t* address, uint16_t r_length, uint8_t* values, uint16_t w_length);
void LoRa_gotoMode(LoRa* _LoRa, int mode);
uint8_t LoRa_read(LoRa* _LoRa, uint8_t address);
void LoRa_write(LoRa* _LoRa, uint8_t address, uint8_t value);
void LoRa_BurstWrite(LoRa* _LoRa, uint8_t address, uint8_t *value, uint8_t length);
uint8_t LoRa_isvalid(LoRa* _LoRa);

void LoRa_setLowDaraRateOptimization(LoRa* _LoRa, uint8_t value);
void LoRa_setAutoLDO(LoRa* _LoRa);
void LoRa_setFrequency(LoRa* _LoRa, int freq);
void LoRa_setSpreadingFactor(LoRa* _LoRa, int SP);
void LoRa_setPower(LoRa* _LoRa, uint8_t power);
void LoRa_setOCP(LoRa* _LoRa, uint8_t current);
void LoRa_setTOMsb_setCRCon(LoRa* _LoRa);
void LoRa_setSyncWord(LoRa* _LoRa, uint8_t syncword);
uint8_t LoRa_transmit(LoRa* _LoRa, uint8_t* data, uint8_t length, uint16_t timeout);
void LoRa_startReceiving(LoRa* _LoRa);
uint8_t LoRa_receive(LoRa* _LoRa, uint8_t* data, uint8_t length);
void LoRa_receive_IT(LoRa* _LoRa, uint8_t* data, uint8_t length);
int LoRa_getRSSI(LoRa* _LoRa);

uint16_t LoRa_init(LoRa* _LoRa);
uint8_t drv_LoRa_RA02_Init(void);
extern void func_LoRa_RA02_PowerUp_Init(void);

/*******************************************************************************
 * Global variable definitions ('extern')
 ******************************************************************************/

/*******************************************************************************
  Global function prototypes (definition in C source)
 ******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __LORA_RA02_H__ */

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
