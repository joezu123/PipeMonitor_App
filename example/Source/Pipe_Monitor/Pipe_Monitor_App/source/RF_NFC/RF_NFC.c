/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\RF_NFC\RF_NFC.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-31       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "RF_NFC.h"
#include "hc32f460_gpio.h"
#include "hc32f460_exint_nmi_swi.h"
#include "hc32f460_interrupts.h"
#include "User_Data.h"
#include "drv_USART.h"
#include "hc32f460.h"
#include "hc32f460_utility.h"
#include "fm175xx.h"
#include "type_a.h"
#include "type_b.h"
#include "mifare_card.h"
/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

#ifdef HW_VERSION_V1_1
#define RFINT_PORT  PortE
#define RFINT_PIN   Pin08
#else
#define RFINT_PORT  PortD
#define RFINT_PIN   Pin06
#endif
/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
static SystemPataSt *pst_NFCSystemPara;
uint8_t guc_NFC_ID[20][5] = 
{
    {0x88,0x04,0x23,0x53,0xFC},
    {0x88,0x04,0x05,0xCF,0x46},
    {0}
};
//unsigned char  picc_atqa[2],picc_uid[15],picc_sak[3];
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
 /**
 *******************************************************************************
 ** \brief ExtInt06 callback function   RFINT
 **
 ** \param  None
 **
 ** \retval None
 **
 ******************************************************************************/
#ifdef HW_VERSION_V1_1
void ExtInt08_Callback(void)
{
    if (Set == EXINT_IrqFlgGet(ExtiCh08))
    {
        /* clear int request flag */
        EXINT_IrqFlgClr(ExtiCh08);
    }
}
#else
void ExtInt06_Callback(void)
{
    if (Set == EXINT_IrqFlgGet(ExtiCh06))
    {
        /* clear int request flag */
        EXINT_IrqFlgClr(ExtiCh06);
    }
}
#endif

uint8_t drv_RF_NFC_Module_Init(void)
{
    uint8_t u8Result = 0;
    stc_exint_config_t stcExtiConfig;
    stc_irq_regi_conf_t stcIrqRegiConf;
    stc_port_init_t stcPortInit;

    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcExtiConfig);
    MEM_ZERO_STRUCT(stcIrqRegiConf);
    MEM_ZERO_STRUCT(stcPortInit);

    /**************************************************************************/
    /* External Int Ch.6                                                      */
    /**************************************************************************/
    #ifdef HW_VERSION_V1_1
    stcExtiConfig.enExitCh = ExtiCh08;
    #else
    stcExtiConfig.enExitCh = ExtiCh06;
    #endif

    /* Filter setting */
    stcExtiConfig.enFilterEn = Enable;
    stcExtiConfig.enFltClk = Pclk3Div8;
    stcExtiConfig.enExtiLvl = ExIntFallingEdge;
    EXINT_Init(&stcExtiConfig);

    /* Set External Int */
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enExInt = Enable;
    PORT_Init(RFINT_PORT, RFINT_PIN, &stcPortInit);

    /* Select External Int Ch.6 */
    #ifdef HW_VERSION_V1_1
    stcIrqRegiConf.enIntSrc = INT_PORT_EIRQ8;
    #else
    stcIrqRegiConf.enIntSrc = INT_PORT_EIRQ6;
    #endif

    /* Register External Int to Vect.No.000 */
    stcIrqRegiConf.enIRQn = Int002_IRQn;

    /* Callback function */
    #ifdef HW_VERSION_V1_1
    stcIrqRegiConf.pfnCallback = &ExtInt08_Callback;
    #else
    stcIrqRegiConf.pfnCallback = &ExtInt06_Callback;
    #endif

    /* Registration IRQ */
    enIrqRegistration(&stcIrqRegiConf);

    /* Clear pending */
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);

    /* Set priority */
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_02);

    /* Enable NVIC */
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Disable;
    PORT_Init(PWRRF_GPIO_PORT, PWRRF_GPIO_PIN, &stcPortInit);
    PORT_Init(RFRST_PORT, RFRST_PIN, &stcPortInit);

    pst_NFCSystemPara = GetSystemPara();

    //开启供电电源
    PWRRF_PIN_CLOSE();
    Ddl_Delay1ms(100);
    PWRRF_PIN_OPEN();

    drv_mcu_ChangeUSART3_Source(MODULE_NFC_RFID);

    //RF模块复位
    RFRST_PIN_SET;
    Ddl_Delay1ms(100);
    RFRST_PIN_RESET;
    Ddl_Delay1ms(100);
    RFRST_PIN_SET;
    Ddl_Delay1ms(100);

    pst_NFCSystemPara->UsartData.usUsartxRecvDataLen[2] = 0;
    memset(pst_NFCSystemPara->UsartData.ucUsartxRecvDataArr[2], 0, USART_DATA_LEN_MAX);
    u8Result = drv_RF_NFC_MCU_TO_PCD_Test(); //测试模块是否正常工作
    //修改模块波特率
    //drv_RF_NFC_SendData(0x1F, 0x7A, 2); //修改波特率为115200;
    //Ddl_Delay1ms(100);
    //drv_mcu_SetUSART3_Baud();

    //u8Result = drv_RF_NFC_MCU_TO_PCD_Test(); //测试模块是否正常工作
	return u8Result;
}

//NFC模块开启电源及初始化配置操作
void func_RF_NFC_PowerUp_Init(void)
{
    //uint8_t ucResult = 0;
    stc_port_init_t stcPortInit;
    stc_exint_config_t stcExtiConfig;
    stc_irq_regi_conf_t stcIrqRegiConf;

    MEM_ZERO_STRUCT(stcExtiConfig);
    MEM_ZERO_STRUCT(stcIrqRegiConf);

    #ifdef HW_VERSION_V1_1
    stcExtiConfig.enExitCh = ExtiCh08;
    #else
    stcExtiConfig.enExitCh = ExtiCh06;
    #endif

    /* Filter setting */
    stcExtiConfig.enFilterEn = Enable;
    stcExtiConfig.enFltClk = Pclk3Div8;
    stcExtiConfig.enExtiLvl = ExIntFallingEdge;
    EXINT_Init(&stcExtiConfig);

    /* Set External Int */
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enExInt = Enable;
    PORT_Init(RFINT_PORT, RFINT_PIN, &stcPortInit);

    /* Select External Int Ch.6 */
    #ifdef HW_VERSION_V1_1
    stcIrqRegiConf.enIntSrc = INT_PORT_EIRQ8;
    #else
    stcIrqRegiConf.enIntSrc = INT_PORT_EIRQ6;
    #endif

    /* Register External Int to Vect.No.000 */
    stcIrqRegiConf.enIRQn = Int002_IRQn;

    /* Callback function */
    #ifdef HW_VERSION_V1_1
    stcIrqRegiConf.pfnCallback = &ExtInt08_Callback;
    #else
    stcIrqRegiConf.pfnCallback = &ExtInt06_Callback;
    #endif

    /* Registration IRQ */
    enIrqRegistration(&stcIrqRegiConf);

    /* Clear pending */
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);

    /* Set priority */
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_02);

    /* Enable NVIC */
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_Out;
    PORT_Init(RFRST_PORT, RFRST_PIN, &stcPortInit);
    
    //开启供电电源
    PWRRF_PIN_CLOSE();
    Ddl_Delay1ms(100);
    PWRRF_PIN_OPEN();

    drv_mcu_ChangeUSART3_Source(MODULE_NFC_RFID);

    //RF模块复位
    RFRST_PIN_SET;
    Ddl_Delay1ms(100);
    RFRST_PIN_RESET;
    Ddl_Delay1ms(100);
    RFRST_PIN_SET;
    Ddl_Delay1ms(100);

    pst_NFCSystemPara->UsartData.usUsartxRecvDataLen[2] = 0;
    memset(pst_NFCSystemPara->UsartData.ucUsartxRecvDataArr[2], 0, USART_DATA_LEN_MAX);

    //drv_RF_NFC_SendData(CommandReg, SoftReset);               // 读取控制寄存器
    drv_RF_NFC_MCU_TO_PCD_Test(); //测试模块是否正常工作
    Ddl_Delay1ms(200);
}

//关闭电源操作及引脚配置
void func_RF_NFC_PowerDown_DeInit(void)
{
    stc_port_init_t stcPortInit;
    MEM_ZERO_STRUCT(stcPortInit);
    PORT_ResetBits(RFRST_PORT,RFRST_PIN);
    
    //stcPortInit.enPinMode = Pin_Mode_Ana;
    
    
    //PORT_Init(RFRST_PORT, RFRST_PIN, &stcPortInit);
    //PORT_Init(RFINT_PORT, RFINT_PIN, &stcPortInit);
    //stcPortInit.enPinMode = Pin_Mode_In;
    PORT_SetFunc(USART3_RX_RFID_PORT, USART3_RX_RFID_PIN, USART3_RX_FUNC, Enable);
    PORT_SetFunc(USART3_TX_RFID_PORT, USART3_TX_RFID_PIN, USART3_TX_FUNC, Enable);
    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enPinSubFunc = Disable;
    PORT_Init(USART3_TX_RFID_PORT, USART3_TX_RFID_PIN, &stcPortInit);
    PORT_ResetBits(USART3_TX_RFID_PORT,USART3_TX_RFID_PIN);
    //PORT_Init(USART3_TX_RFID_PORT, USART3_TX_RFID_PIN, &stcPortInit);

    PWRRF_PIN_CLOSE();	//关闭RF模块电源
}

uint8_t drv_RF_NFC_MCU_TO_PCD_Test(void)
{
    uint8_t ucRegVal;
    //uint8_t ucTestArr[2] = {0xAA,0x55};
    //drv_RF_NFC_ReceiveData(0x04, &ucRegVal, 1); // 读取控制寄存器
    //drv_RF_NFC_ReceiveData(ControlReg, &ucRegVal, 1); // 读取控制寄存器
    drv_RF_NFC_SendData(CommandReg, SoftReset);               // 读取控制寄存器
    Ddl_Delay1ms(200);
    //drv_RF_NFC_ReceiveData(ControlReg, &ucRegVal, 1); // 读取控制寄存器
    //Ddl_Delay1ms(10);
    //guc_NFC_USART3_RecvCnt = 0;
    drv_RF_NFC_SendData(ControlReg, 0x10);               // 启动读写器模式
    //Ddl_Delay1ms(10);
	ucRegVal = drv_RF_NFC_ReceiveData(ControlReg); // 读取控制寄存器
    //drv_RF_NFC_ReceiveData(ControlReg, &ucRegVal, 1); // 读取控制寄存器
    //Ddl_Delay1ms(10);
    //drv_RF_NFC_ReceiveData(TxControlReg, &ucRegVal, 1);
    //drv_RF_NFC_SendData(TxControlReg, 0x03, 2); 
    //drv_RF_NFC_ReceiveData(TxControlReg, &ucRegVal, 1);

   
    //drv_RF_NFC_ReceiveData(ComIEnReg, &ucRegVal, 1); // 读取控制寄存器
    //drv_RF_NFC_SendData(ComIEnReg, 0x80,2);    
    //drv_RF_NFC_ReceiveData(ComIEnReg, &ucRegVal, 1); // 读取控制寄存器
    //Ddl_Delay1ms(100);
    //drv_RF_NFC_ReceiveData(ComIEnReg, &ucRegVal, 1); // 读取控制寄存器

    //drv_RF_NFC_ReceiveData(GsNReg, &ucRegVal, 1);
    drv_RF_NFC_SendData(GsNReg, 0xF4);            // CWGsN = 0xF; ModGsN = 0x4
    //Ddl_Delay1ms(10);
    ucRegVal = drv_RF_NFC_ReceiveData(GsNReg);
    //drv_RF_NFC_ReceiveData(GsNReg, &ucRegVal,
    //drv_RF_NFC_ReceiveData(GsNReg, &ucRegVal, 1);
    //ucRegVal = 0xAA;
    //drv_mcu_USART_SendData(MODULE_NFC_RFID, ucTestArr, 2);
    if(ucRegVal != 0xF4)                        // 验证接口正确
    {
        return 1;
    }
	return 0;
}

uint8_t     Send_Buff[100];                                       //数据发送缓冲
uint8_t     Recv_Buff[100];                                       //数据接收缓冲区
#define    DataLength     0                                           //数据帧的长度
#define    Data            1                                           //数据帧数据起始
uint8_t func_FindCar(void)
{
    unsigned char statues;
    unsigned char Card_Type;            //1:TypeA ;  2:TypeB
	//unsigned int Rec_len;               //卡片返回数据长度

    memset(picc_sak,0,3);
    Card_Type = 1;
    statues=TypeA_CardActive(picc_atqa,picc_uid,picc_sak);
    #if 0
    if(statues != TRUE)
    {
        Card_Type = 2;
        FM175X_SoftReset();
        Rec_len = 0;
        Pcd_ConfigISOType(1);             //设置TypeB
        Set_Rf(3);   //turn on radio
        statues=TypeB_WUP(&Rec_len,Send_Buff+Data,PUPI);//寻卡 Standard	 send request command Standard mode
        if(statues == TRUE)
        {
            statues=TypeB_Select(PUPI,&Rec_len,Send_Buff+Data);			
        }	 
    }
    #endif
    if(statues == TRUE)
    {
        //LED_RedOn();
        Ddl_Delay1ms(100);
        //LED_RedOff();
        Ddl_Delay1ms(100);
        if(Card_Type == 1)
        {
            memcpy(Send_Buff+Data,picc_uid,4);  //如果是4字节ID号卡则为4，如果是7字节ID号卡则为10
        }
        else   
        {
            memcpy(Send_Buff+Data,PUPI,4);
        }
        Send_Buff[DataLength]=4;	
        //Uart_Send();              //串口返回命令执行结果信息
    }					
    Set_Rf(0);   //turn off radio	
    return statues;
}


/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
