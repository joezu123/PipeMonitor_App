/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\LoRa_RA02\LoRa_RA02.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-04-01       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "LoRa_RA02.h"
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
LoRa myLoRa;
uint8_t read_data[128];
uint8_t send_data[128];
int			RSSI;
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/* ----------------------------------------------------------------------------- *\
		name        : newLoRa

		description : it's a constructor for LoRa structure that assign default values
									and pass created object (LoRa struct instanse)

		arguments   : Nothing

		returns     : A LoRa object whith these default values:
											----------------------------------------
										  |   carrier frequency = 433 MHz        |
										  |    spreading factor = 7				       |
											|           bandwidth = 125 KHz        |
											| 		    coding rate = 4/5            |
											----------------------------------------
\* ----------------------------------------------------------------------------- */
LoRa newLoRa()
{
	LoRa new_LoRa;

	new_LoRa.frequency             = 433       ;
	new_LoRa.spredingFactor        = SF_7      ;
	new_LoRa.bandWidth			   = BW_125KHz ;
	new_LoRa.crcRate               = CR_4_5    ;
	new_LoRa.power				   = POWER_20db;
	new_LoRa.overCurrentProtection = 100       ;
	new_LoRa.preamble			   = 8         ;

	return new_LoRa;
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_reset

		description : reset module

		arguments   :
			LoRa* LoRa --> LoRa object handler

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_reset(LoRa* _LoRa)
{
	PORT_ResetBits(_LoRa->reset_port, _LoRa->reset_pin);
	Ddl_Delay1ms(1);
	PORT_SetBits(_LoRa->reset_port, _LoRa->reset_pin);
	Ddl_Delay1ms(100);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_isvalid

		description : check the LoRa instruct values

		arguments   :
			LoRa* LoRa --> LoRa object handler

		returns     : returns 1 if all of the values were given, otherwise returns 0
\* ----------------------------------------------------------------------------- */
uint8_t LoRa_isvalid(LoRa* _LoRa)
{
	return 1;
}

/**
 *******************************************************************************
 ** \brief SPI flash write byte function
 **
 ** \param [in] u8Data                      SPI write data to flash
 **
 ** \retval uint8_t                         SPI receive data from flash
 **
 ******************************************************************************/
uint8_t LORA_RA02_SendRcvByte(uint8_t TxData)
{
	uint8_t Rxdata;
	#ifdef SPI_HART
	/* Wait tx buffer empty */
    while (Reset == SPI_GetFlag(SPI_LORA_RA02_UNIT, SpiFlagSendBufferEmpty))
    {
    }
    /* Send data */
    SPI_SendData8(SPI_LORA_RA02_UNIT, TxData);
    /* Wait rx buffer full */
    while (Reset == SPI_GetFlag(SPI_LORA_RA02_UNIT, SpiFlagReceiveBufferFull))
    {
    }
    /* Receive data */
    Rxdata = SPI_ReceiveData8(SPI_LORA_RA02_UNIT);
	#else
	uint8_t i=0;
	for(i=0;i<8;i++)
	{
		SPI_LORA_RA02_SCK_L;					
		if(TxData & 0x80)
		{
			SPI_LORA_RA02_SI_H; 
		} 
		else 
		{
			SPI_LORA_RA02_SI_L;
		}
		TxData<<=1;		
		Rxdata<<=1;
		SPI_LORA_RA02_SCK_H;  //时钟平时为低。 在上升沿送数据
		if(SPI_LORA_RA02_SO)
		{
			Rxdata |= 0x01;
		} 
	}
	SPI_LORA_RA02_SCK_L;
	#endif
	return Rxdata;
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_readReg

		description : read a register(s) by an address and a length,
									then store value(s) at outpur array.
		arguments   :
			LoRa* LoRa        --> LoRa object handler
			uint8_t* address  -->	pointer to the beginning of address array
			uint16_t r_length -->	detemines number of addresse bytes that
														you want to send
			uint8_t* output		--> pointer to the beginning of output array
			uint16_t w_length	--> detemines number of bytes that you want to read

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_readReg(LoRa* _LoRa, uint8_t* address, uint16_t r_length, uint8_t* output, uint16_t w_length)
{
	SPI_LORA_RA02_NSS_LOW();
	LORA_RA02_SendRcvByte(*address);

	while(w_length--)
	{	
		*output = LORA_RA02_SendRcvByte(DUMMY_BYTE);
		output++;
	}
	SPI_LORA_RA02_NSS_HIGH();
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_writeReg

		description : write a value(s) in a register(s) by an address

		arguments   :
			LoRa* LoRa        --> LoRa object handler
			uint8_t* address  -->	pointer to the beginning of address array
			uint16_t r_length -->	detemines number of addresse bytes that
														you want to send
			uint8_t* output		--> pointer to the beginning of values array
			uint16_t w_length	--> detemines number of bytes that you want to send

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_writeReg(LoRa* _LoRa, uint8_t* address, uint16_t r_length, uint8_t* values, uint16_t w_length)
{
	SPI_LORA_RA02_NSS_LOW();
	LORA_RA02_SendRcvByte(*address);

	while(w_length--)
	{	
		LORA_RA02_SendRcvByte(*values);
		values++;
	}
	SPI_LORA_RA02_NSS_HIGH();
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_read

		description : read a register by an address

		arguments   :
			LoRa*   LoRa        --> LoRa object handler
			uint8_t address     -->	address of the register e.g 0x1D

		returns     : register value
\* ----------------------------------------------------------------------------- */
uint8_t LoRa_read(LoRa* _LoRa, uint8_t address)
{
	uint8_t read_data;
	uint8_t data_addr;

	data_addr = address & 0x7F;
	LoRa_readReg(_LoRa, &data_addr, 1, &read_data, 1);
	//HAL_Delay(5);

	return read_data;
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_write

		description : write a value in a register by an address

		arguments   :
			LoRa*   LoRa        --> LoRa object handler
			uint8_t address     -->	address of the register e.g 0x1D
			uint8_t value       --> value that you want to write

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_write(LoRa* _LoRa, uint8_t address, uint8_t value)
{
	uint8_t data;
	uint8_t addr;

	addr = address | 0x80;
	data = value;
	LoRa_writeReg(_LoRa, &addr, 1, &data, 1);
	//HAL_Delay(5);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_BurstWrite

		description : write a set of values in a register by an address respectively

		arguments   :
			LoRa*   LoRa        --> LoRa object handler
			uint8_t address     -->	address of the register e.g 0x1D
			uint8_t *value      --> address of values that you want to write

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_BurstWrite(LoRa* _LoRa, uint8_t address, uint8_t *value, uint8_t length)
{
	uint8_t addr;
	addr = address | 0x80;

	//NSS = 1
	SPI_LORA_RA02_NSS_LOW();
	
	LORA_RA02_SendRcvByte(addr);

	while(length--)
	{	
		LORA_RA02_SendRcvByte(*value);
		value++;
	}
	//NSS = 0
	//HAL_Delay(5);
	SPI_LORA_RA02_NSS_HIGH();
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_setLowDaraRateOptimization

		description : set the LowDataRateOptimization flag. Is is mandated for when the symbol length exceeds 16ms.

		arguments   :
			LoRa*	LoRa         --> LoRa object handler
			uint8_t	value        --> 0 to disable, otherwise to enable

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_setLowDaraRateOptimization(LoRa* _LoRa, uint8_t value)
{
	uint8_t	data;
	uint8_t	read;

	read = LoRa_read(_LoRa, RegModemConfig3);
	
	if(value)
	{
		data = read | 0x08;
	}
	else
	{
		data = read & 0xF7;
	}

	LoRa_write(_LoRa, RegModemConfig3, data);
	Ddl_Delay1ms(10);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_setAutoLDO

		description : set the LowDataRateOptimization flag automatically based on the symbol length.

		arguments   :
			LoRa*	LoRa         --> LoRa object handler

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_setAutoLDO(LoRa* _LoRa)
{
	double BW[] = {7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125.0, 250.0, 500.0};

	LoRa_setLowDaraRateOptimization(_LoRa, (long)((1 << _LoRa->spredingFactor) / ((double)BW[_LoRa->bandWidth])) > 16.0);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_setFrequency

		description : set carrier frequency e.g 433 MHz

		arguments   :
			LoRa* LoRa        --> LoRa object handler
			int   freq        --> desired frequency in MHz unit, e.g 434

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_setFrequency(LoRa* _LoRa, int freq)
{
	uint8_t  data;
	uint32_t F;
	F = (freq * 524288)>>5;

	// write Msb:
	data = F >> 16;
	LoRa_write(_LoRa, RegFrMsb, data);
	Ddl_Delay1ms(5);

	// write Mid:
	data = F >> 8;
	LoRa_write(_LoRa, RegFrMid, data);
	Ddl_Delay1ms(5);

	// write Lsb:
	data = F >> 0;
	LoRa_write(_LoRa, RegFrLsb, data);
	Ddl_Delay1ms(5);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_setSpreadingFactor

		description : set spreading factor, from 7 to 12.

		arguments   :
			LoRa* LoRa        --> LoRa object handler
			int   SP          --> desired spreading factor e.g 7

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_setSpreadingFactor(LoRa* _LoRa, int SF)
{
	uint8_t	data;
	uint8_t	read;

	if(SF>12)
	{
		SF = 12;
	}
	if(SF<7)
	{
		SF = 7;
	}

	read = LoRa_read(_LoRa, RegModemConfig2);
	Ddl_Delay1ms(10);

	data = (SF << 4) + (read & 0x0F);
	LoRa_write(_LoRa, RegModemConfig2, data);
	Ddl_Delay1ms(10);
	
	LoRa_setAutoLDO(_LoRa);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_setPower

		description : set power gain.

		arguments   :
			LoRa* LoRa        --> LoRa object handler
			int   power       --> desired power like POWER_17db

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_setPower(LoRa* _LoRa, uint8_t power)
{
	LoRa_write(_LoRa, RegPaConfig, power);
	Ddl_Delay1ms(10);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_setOCP

		description : set maximum allowed current.

		arguments   :
			LoRa* LoRa        --> LoRa object handler
			int   current     --> desired max currnet in mA, e.g 120

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_setOCP(LoRa* _LoRa, uint8_t current)
{
	uint8_t	OcpTrim = 0;

	if(current<45)
		current = 45;
	if(current>240)
		current = 240;

	if(current <= 120)
		OcpTrim = (current - 45)/5;
	else if(current <= 240)
		OcpTrim = (current + 30)/10;

	OcpTrim = OcpTrim + (1 << 5);
	LoRa_write(_LoRa, RegOcp, OcpTrim);
	Ddl_Delay1ms(10);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_setTOMsb_setCRCon

		description : set timeout msb to 0xFF + set CRC enable.

		arguments   :
			LoRa* LoRa        --> LoRa object handler

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_setTOMsb_setCRCon(LoRa* _LoRa)    
{
	uint8_t read, data;

	read = LoRa_read(_LoRa, RegModemConfig2);

	data = read | 0x07;
	LoRa_write(_LoRa, RegModemConfig2, data);
	Ddl_Delay1ms(10);
}           

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_setTOMsb_setCRCon

		description : set timeout msb to 0xFF + set CRC enable.

		arguments   :
			LoRa* LoRa        --> LoRa object handler

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_setSyncWord(LoRa* _LoRa, uint8_t syncword)
{
	LoRa_write(_LoRa, RegSyncWord, syncword);
	Ddl_Delay1ms(10);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_gotoMode

		description : set LoRa Op mode

		arguments   :
			LoRa* LoRa    --> LoRa object handler
			mode	        --> select from defined modes

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_gotoMode(LoRa* _LoRa, int mode)
{
	uint8_t    read;
	uint8_t    data;

	read = LoRa_read(_LoRa, RegOpMode);
	data = read;

	if(mode == SLEEP_MODE)
	{
		data = (read & 0xF8) | 0x00;
		_LoRa->current_mode = SLEEP_MODE;
	}
	else if (mode == STNBY_MODE)
	{
		data = (read & 0xF8) | 0x01;
		_LoRa->current_mode = STNBY_MODE;
	}
	else if (mode == TRANSMIT_MODE)
	{
		data = (read & 0xF8) | 0x03;
		_LoRa->current_mode = TRANSMIT_MODE;
	}
	else if (mode == RXCONTIN_MODE)
	{
		data = (read & 0xF8) | 0x05;
		_LoRa->current_mode = RXCONTIN_MODE;
	}
	else if (mode == RXSINGLE_MODE)
	{
		data = (read & 0xF8) | 0x06;
		_LoRa->current_mode = RXSINGLE_MODE;
	}

	LoRa_write(_LoRa, RegOpMode, data);
	//HAL_Delay(10);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_startReceiving

		description : Start receiving continuously

		arguments   :
			LoRa*    LoRa     --> LoRa object handler

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
void LoRa_startReceiving(LoRa* _LoRa)
{
	LoRa_gotoMode(_LoRa, RXCONTIN_MODE);
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_transmit

		description : Transmit data

		arguments   :
			LoRa*    LoRa     --> LoRa object handler
			uint8_t  data			--> A pointer to the data you wanna send
			uint8_t	 length   --> Size of your data in Bytes
			uint16_t timeOut	--> Timeout in milliseconds
		returns     : 1 in case of success, 0 in case of timeout
\* ----------------------------------------------------------------------------- */
uint8_t LoRa_transmit(LoRa* _LoRa, uint8_t* data, uint8_t length, uint16_t timeout)
{
	uint8_t read;

	int mode = _LoRa->current_mode;
	LoRa_gotoMode(_LoRa, STNBY_MODE);
	read = LoRa_read(_LoRa, RegFiFoTxBaseAddr);
	LoRa_write(_LoRa, RegFiFoAddPtr, read);
	LoRa_write(_LoRa, RegPayloadLength, length);
	LoRa_BurstWrite(_LoRa, RegFiFo, data, length);
	LoRa_gotoMode(_LoRa, TRANSMIT_MODE);
	while(1)
	{
		read = LoRa_read(_LoRa, RegIrqFlags);
		if((read & 0x08)!=0)
		{
			LoRa_write(_LoRa, RegIrqFlags, 0xFF);
			LoRa_gotoMode(_LoRa, mode);
			return 1;
		}
		else{
			if(--timeout==0)
			{
				LoRa_gotoMode(_LoRa, mode);
				return 0;
			}
		}
		Ddl_Delay1ms(1);
	}
} 

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_Receive

		description : Read received data from module

		arguments   :
			LoRa*    LoRa     --> LoRa object handler
			uint8_t  data			--> A pointer to the array that you want to write bytes in it
			uint8_t	 length   --> Determines how many bytes you want to read

		returns     : The number of bytes received
\* ----------------------------------------------------------------------------- */
uint8_t LoRa_receive(LoRa* _LoRa, uint8_t* data, uint8_t length)
{
	uint8_t read;
	uint8_t number_of_bytes;
	uint8_t min = 0;

	for(int i=0; i<length; i++)
	{
		data[i]=0;
	}

	LoRa_gotoMode(_LoRa, STNBY_MODE);
	read = LoRa_read(_LoRa, RegIrqFlags);
	if((read & 0x40) != 0)
	{
		LoRa_write(_LoRa, RegIrqFlags, 0xFF);
		number_of_bytes = LoRa_read(_LoRa, RegRxNbBytes);
		read = LoRa_read(_LoRa, RegFiFoRxCurrentAddr);
		LoRa_write(_LoRa, RegFiFoAddPtr, read);
		min = length >= number_of_bytes ? number_of_bytes : length;
		for(int i=0; i<min; i++)
		{
			data[i] = LoRa_read(_LoRa, RegFiFo);
		}
	}
	LoRa_gotoMode(_LoRa, RXCONTIN_MODE);
    return min;
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_getRSSI

		description : initialize and set the right setting according to LoRa sruct vars

		arguments   :
			LoRa* LoRa        --> LoRa object handler

		returns     : Returns the RSSI value of last received packet.
\* ----------------------------------------------------------------------------- */
int LoRa_getRSSI(LoRa* _LoRa)
{
	uint8_t read;
	read = LoRa_read(_LoRa, RegPktRssiValue);
	return -164 + read;
}

/* ----------------------------------------------------------------------------- *\
		name        : LoRa_init

		description : initialize and set the right setting according to LoRa sruct vars

		arguments   :
			LoRa* LoRa        --> LoRa object handler

		returns     : Nothing
\* ----------------------------------------------------------------------------- */
uint16_t LoRa_init(LoRa* _LoRa)
{
	uint8_t    data;
	uint8_t    read;

	if(LoRa_isvalid(_LoRa))
	{
		// goto sleep mode:
		LoRa_gotoMode(_LoRa, SLEEP_MODE);
		Ddl_Delay1ms(10);

		// turn on LoRa mode:
		read = LoRa_read(_LoRa, RegOpMode);
		Ddl_Delay1ms(10);
		data = read | 0x80;
		LoRa_write(_LoRa, RegOpMode, data);
		Ddl_Delay1ms(100);

		// set frequency:
		LoRa_setFrequency(_LoRa, _LoRa->frequency);

		// set output power gain:
		LoRa_setPower(_LoRa, _LoRa->power);

		// set over current protection:
		LoRa_setOCP(_LoRa, _LoRa->overCurrentProtection);

		// set LNA gain:
		LoRa_write(_LoRa, RegLna, 0x23);

		// set spreading factor, CRC on, and Timeout Msb:
		LoRa_setTOMsb_setCRCon(_LoRa);
		LoRa_setSpreadingFactor(_LoRa, _LoRa->spredingFactor);

		// set Timeout Lsb:
		LoRa_write(_LoRa, RegSymbTimeoutL, 0xFF);

		// set bandwidth, coding rate and expilicit mode:
			// 8 bit RegModemConfig --> | X | X | X | X | X | X | X | X |
			//       bits represent --> |   bandwidth   |     CR    |I/E|
		data = 0;
		data = (_LoRa->bandWidth << 4) + (_LoRa->crcRate << 1);
		LoRa_write(_LoRa, RegModemConfig1, data);
		LoRa_setAutoLDO(_LoRa);

		// set preamble:
		LoRa_write(_LoRa, RegPreambleMsb, _LoRa->preamble >> 8);
		LoRa_write(_LoRa, RegPreambleLsb, _LoRa->preamble >> 0);

		// DIO mapping:   --> DIO: RxDone
		read = LoRa_read(_LoRa, RegDioMapping1);
		data = read | 0x3F;
		LoRa_write(_LoRa, RegDioMapping1, data);

		// goto standby mode:
		LoRa_gotoMode(_LoRa, STNBY_MODE);
		_LoRa->current_mode = STNBY_MODE;
		Ddl_Delay1ms(10);

		read = LoRa_read(_LoRa, RegVersion);
		if(read == 0x12)
		{
			return LORA_OK;
		}
		else
		{
			return LORA_NOT_FOUND;
		}
	}
	else 
	{
		return LORA_UNAVAILABLE;
	}
}

 /**
 *******************************************************************************
 ** \brief  GD25Q128E init function
 **
 ** \param [in]  None
 **
 ** \retval 0: init success; 1: init failed
 **
 ******************************************************************************/
uint8_t drv_LoRa_RA02_Init(void)
{
	stc_port_init_t stcPortInit;

    /* configure structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);

    /* Flash NSS */
    stcPortInit.enPinMode = Pin_Mode_Out;
	
    PORT_Init(LORA_RA02_RST_PORT, LORA_RA02_RST_PIN, &stcPortInit);
	PORT_Init(SPI_LORA_RA02_NSS_PORT, SPI_LORA_RA02_NSS_PIN, &stcPortInit);
	#ifndef SPI_HART
	PORT_Init(SPI_LORA_RA02_SCK_PORT, SPI_LORA_RA02_SCK_PIN, &stcPortInit);
	PORT_Init(SPI_LORA_RA02_MISO_PORT, SPI_LORA_RA02_MISO_PIN, &stcPortInit);
	stcPortInit.enPinMode = Pin_Mode_In;
	PORT_Init(SPI_LORA_RA02_MOSI_PORT, SPI_LORA_RA02_MOSI_PIN, &stcPortInit);
	#endif

	PWRLORA_PIN_CLOSE();
	Ddl_Delay1ms(100);
	PWRLORA_PIN_OPEN();
	Ddl_Delay1ms(100);

	SPI_LORA_RA02_NSS_LOW() ;
	Ddl_Delay1ms(100);
    SPI_LORA_RA02_NSS_HIGH();

	// MODULE SETTINGS ----------------------------------------------
	myLoRa = newLoRa();
	
	myLoRa.hSPIx                 = SPI_LORA_RA02_UNIT;
	myLoRa.CS_port               = SPI_LORA_RA02_NSS_PORT;
	myLoRa.CS_pin                = SPI_LORA_RA02_NSS_PIN;
	myLoRa.reset_port            = LORA_RA02_RST_PORT;
	myLoRa.reset_pin             = LORA_RA02_RST_PIN;
	//myLoRa.DIO0_port						 = DIO0_GPIO_Port;
	//myLoRa.DIO0_pin							 = DIO0_Pin;
	
	myLoRa.frequency             = 433;							  // default = 433 MHz
	myLoRa.spredingFactor        = SF_7;							// default = SF_7
	myLoRa.bandWidth			       = BW_125KHz;				  // default = BW_125KHz
	myLoRa.crcRate				       = CR_4_5;						// default = CR_4_5
	myLoRa.power					       = POWER_20db;				// default = 20db
	myLoRa.overCurrentProtection = 120; 							// default = 100 mA
	myLoRa.preamble				       = 10;		  					// default = 8;
	
	LoRa_reset(&myLoRa);
	LoRa_init(&myLoRa);
	
	// START CONTINUOUS RECEIVING -----------------------------------
	LoRa_startReceiving(&myLoRa);

	return 0;
}

//模块上电及初始化
void func_LoRa_RA02_PowerUp_Init(void)
{
	PWRLORA_PIN_CLOSE();
	Ddl_Delay1ms(100);
	PWRLORA_PIN_OPEN();
	Ddl_Delay1ms(100);

	SPI_LORA_RA02_NSS_LOW() ;
	Ddl_Delay1ms(100);
    SPI_LORA_RA02_NSS_HIGH();
}
/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
