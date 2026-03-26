/**
 *******************************************************************************
 * @file  Pipe_Monitor_App_V2\source\WatchDog\WatchDog.c
 * @brief This file provides firmware functions to manage the Communication
 *        Device driver.复位电阻51K，对应复位时间为373.162144s，约6分钟
 @verbatim
   Change Logs:
   Date             Author          Notes
   2025-03-31       Joe             First version
 @endverbatim
 
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "AlarmRule.h"
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
static SystemPataSt *pst_ARSystemPara;
/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
// ============================ 辅助函数 ============================
/**
 * @brief  求三个数的最大值
 * @param  a, b, c: 输入数值
 * @retval 最大值
 */
WarningLevel_TypeDef MAX3(WarningLevel_TypeDef a, WarningLevel_TypeDef b, WarningLevel_TypeDef c)
{
    WarningLevel_TypeDef max_val = a;
    if(b > max_val) max_val = b;
    if(c > max_val) max_val = c;
    return max_val;
}

// 电导率预警规则是否启用判断
void func_CondAlarmRule_Enable(void)
{
	if(pst_ARSystemPara->DevicePara.cAlarmEnableFlag == 1)	//预警开关：启用
	{
		if(pst_ARSystemPara->DevicePara.cScenario == 0)	//排口场景
		{
			if(pst_ARSystemPara->DevicePara.cPlanEnableFlag == 0)	//预案开关：未启用
			{
				g_WarningSystem.cond_config.warning_switch = 1; //开启电导率预警
			}
			else
			{
				g_WarningSystem.cond_config.warning_switch = 0; //关闭电导率预警
			}
		}
		else //污水管网场景
		{
			if(pst_ARSystemPara->DevicePara.cWeatherFlag == 0)	//雨旱天开关：旱天
			{
				g_WarningSystem.cond_config.warning_switch = 1; //开启电导率预警
			}
			else
			{
				g_WarningSystem.cond_config.warning_switch = 0; //关闭电导率预警
			}
		}
	}
	else
	{
		g_WarningSystem.cond_config.warning_switch = 0; //关闭电导率预警
	}
}
// ============================ 系统初始化函数 ============================
/**
 * @brief  预警系统初始化
 * @param  无
 * @retval 0-成功，1-失败
 */
uint8_t WarningSystem_Init(void)
{
	time_t now;
    pst_ARSystemPara = GetSystemPara();
    // 初始化配置参数（默认值）
    memset(&g_WarningSystem, 0, sizeof(WarningSystem_TypeDef));
    
    // 液位默认配置
    g_WarningSystem.level_config.well_depth = pst_ARSystemPara->DevicePara.fInstall_Height;        // 默认井深5米
    g_WarningSystem.level_config.warning_switch = pst_ARSystemPara->DevicePara.cAlarmEnableFlag;       // 默认开启预警
    g_WarningSystem.level_config.warning_group = WARNING_GROUP_COUNT;
    g_WarningSystem.level_config.level1_threshold = pst_ARSystemPara->DevicePara.fLevelAlarmPer[0]; // 70%井深为第1档位
    g_WarningSystem.level_config.level2_threshold = pst_ARSystemPara->DevicePara.fLevelAlarmPer[1]; // 80%井深为第2档位
    g_WarningSystem.level_config.level3_threshold = pst_ARSystemPara->DevicePara.fLevelAlarmPer[2]; // 90%井深为第3档位
    
    // 电导率默认配置
    //g_WarningSystem.cond_config.warning_switch = pst_ARSystemPara->DevicePara.cAlarmEnableFlag;        // 默认开启预警
	func_CondAlarmRule_Enable();
    g_WarningSystem.cond_config.warning_group = WARNING_GROUP_COUNT;
    g_WarningSystem.cond_config.level1_threshold = pst_ARSystemPara->DevicePara.usCONDAlarmValue[0];   // 300μS/cm为第1档位
    g_WarningSystem.cond_config.level2_threshold = pst_ARSystemPara->DevicePara.usCONDAlarmValue[1];   // 500μS/cm为第2档位
    g_WarningSystem.cond_config.level3_threshold = pst_ARSystemPara->DevicePara.usCONDAlarmValue[2];   // 800μS/cm为第3档位
    
    // 流速默认配置
    g_WarningSystem.flow_config.flow_switch = pst_ARSystemPara->DevicePara.cFlowAlarmEnableFlag;          // 默认开启流量
    g_WarningSystem.flow_config.warning_switch = pst_ARSystemPara->DevicePara.cAlarmEnableFlag;       // 默认开启预警
    g_WarningSystem.flow_config.warning_group = WARNING_GROUP_COUNT;
    g_WarningSystem.flow_config.level1_threshold = pst_ARSystemPara->DevicePara.fFlowAlarmValue[0];  // 0.5m/s为第1档位
    g_WarningSystem.flow_config.level2_threshold = pst_ARSystemPara->DevicePara.fFlowAlarmValue[1];  // 1.0m/s为第2档位
    //g_WarningSystem.flow_config.level3_threshold = pst_ARSystemPara->DevicePara.fFlowAlarmValue[2];  // 1.5m/s为第3档位
    
    // 初始化档位状态
    g_WarningSystem.level_level = WARNING_LEVEL_NORMAL;
    g_WarningSystem.cond_level = WARNING_LEVEL_NORMAL;
    g_WarningSystem.flow_level = WARNING_LEVEL_NORMAL;
    g_WarningSystem.system_level = WARNING_LEVEL_NORMAL;
    
    // 初始化时间记录
	now = func_Get_Linux_Time_Sec();
    g_WarningSystem.last_collect_time = (long)now;
    g_WarningSystem.last_upload_time = (long)now;
	//drv_mcu_Get_RTC_Minute(&g_WarningSystem.last_collect_time);
	//drv_mcu_Get_RTC_Minute(&g_WarningSystem.last_upload_time);
    
    // 初始化历史数据缓存
    for(uint8_t i=0; i<WARNING_GROUP_COUNT; i++)
    {
        g_WarningSystem.level_history[i] = 0.0f;
        g_WarningSystem.cond_history[i] = 0;
        g_WarningSystem.flow_history[i] = 0.0f;
    }
    g_WarningSystem.history_index = 0;
    
    g_WarningSystem.system_init_flag = 1;

    return 0;
}

/**
 * @brief  获取数据采集周期
 * @param  level: 当前预警档位
 * @retval 采集周期（秒）
 */
uint32_t Get_CollectInterval(WarningLevel_TypeDef level)
{
    switch(level)
    {
        case WARNING_LEVEL_1:
            return pst_ARSystemPara->DevicePara.usAlarmSamp[0];
        case WARNING_LEVEL_2:
            return pst_ARSystemPara->DevicePara.usAlarmSamp[1];
        case WARNING_LEVEL_3:
            return pst_ARSystemPara->DevicePara.usAlarmSamp[2];
        default:
            return pst_ARSystemPara->DevicePara.nDeviceSampleGapCnt;
    }
}

#if 0
/**
 * @brief  更新单个传感器预警档位
 * @param  sensor_type: 传感器类型
 * @param  new_level: 新档位
 * @retval 无
 */
void Update_WarningLevel(SensorType_TypeDef sensor_type, WarningLevel_TypeDef new_level)
{
    uint32_t current_time = Timer_GetSecCount();
    
    switch(sensor_type)
    {
        case SENSOR_TYPE_LEVEL:
            if(g_WarningSystem.level_level != new_level)
            {
                // 档位变化，重置持续时间
                g_WarningSystem.level_level = new_level;
                g_WarningSystem.level_duration = 0;
                UART_Printf("Level warning level changed to %d\\r\\n", new_level);
                
                // 推送未发送的数据
                Data_Upload(1);
            }
            else
            {
                // 档位不变，更新持续时间
                g_WarningSystem.level_duration = current_time - g_WarningSystem.last_collect_time;
                
                // 检查是否需要升级档位（持续时间判断）
                if(g_WarningSystem.level_duration >= WARNING_LEVEL_DURATION && new_level < WARNING_LEVEL_3)
                {
                    Update_WarningLevel(sensor_type, (WarningLevel_TypeDef)(new_level + 1));
                }
            }
            break;
            
        case SENSOR_TYPE_CONDUCTIVITY:
            if(g_WarningSystem.cond_level != new_level)
            {
                g_WarningSystem.cond_level = new_level;
                g_WarningSystem.cond_duration = 0;
                UART_Printf("Conductivity warning level changed to %d\\r\\n", new_level);
                Data_Upload(1);
            }
            else
            {
                g_WarningSystem.cond_duration = current_time - g_WarningSystem.last_collect_time;
                if(g_WarningSystem.cond_duration >= WARNING_LEVEL_DURATION && new_level < WARNING_LEVEL_3)
                {
                    Update_WarningLevel(sensor_type, (WarningLevel_TypeDef)(new_level + 1));
                }
            }
            break;
            
        case SENSOR_TYPE_FLOW_RATE:
            if(g_WarningSystem.flow_level != new_level)
            {
                g_WarningSystem.flow_level = new_level;
                g_WarningSystem.flow_duration = 0;
                UART_Printf("Flow rate warning level changed to %d\\r\\n", new_level);
                Data_Upload(1);
            }
            else
            {
                g_WarningSystem.flow_duration = current_time - g_WarningSystem.last_collect_time;
                if(g_WarningSystem.flow_duration >= WARNING_LEVEL_DURATION && new_level < WARNING_LEVEL_3)
                {
                    Update_WarningLevel(sensor_type, (WarningLevel_TypeDef)(new_level + 1));
                }
            }
            break;
    }
}

/**
 * @brief  更新系统综合预警档位（最高优先）
 * @param  无
 * @retval 无
 */
void Update_SystemWarningLevel(void)
{
    WarningLevel_TypeDef new_system_level = WARNING_LEVEL_NORMAL;
    
    // 取三个传感器中的最高档位
    new_system_level = MAX3(g_WarningSystem.level_level, 
                           g_WarningSystem.cond_level, 
                           g_WarningSystem.flow_level);
    
    if(g_WarningSystem.system_level != new_system_level)
    {
        g_WarningSystem.system_level = new_system_level;
        UART_Printf("System warning level changed to %d\\r\\n", new_system_level);
        
        // 档位变化时立即上传数据
        Data_Upload(1);
    }
}

/**
 * @brief  降档预判处理
 * @param  无
 * @retval 无
 */
void Downgrade_CheckProcess(void)
{
    static uint32_t last_check_time = 0;
    uint32_t current_time = Timer_GetSecCount();
    
    // 按周期进行降档检查
    if((current_time - last_check_time) < DOWNGRADE_CHECK_CYCLE)
    {
        return;
    }
    last_check_time = current_time;
    
    // 检查各传感器是否需要降档
    Check_Downgrade(SENSOR_TYPE_LEVEL);
    Check_Downgrade(SENSOR_TYPE_CONDUCTIVITY);
    Check_Downgrade(SENSOR_TYPE_FLOW_RATE);
    
    // 更新系统档位
    Update_SystemWarningLevel();
}

/**
 * @brief  单个传感器降档检查
 * @param  sensor_type: 传感器类型
 * @retval 无
 */
void Check_Downgrade(SensorType_TypeDef sensor_type)
{
    WarningLevel_TypeDef current_level = WARNING_LEVEL_NORMAL;
    WarningLevel_TypeDef new_level = WARNING_LEVEL_NORMAL;
    
    switch(sensor_type)
    {
        case SENSOR_TYPE_LEVEL:
            current_level = g_WarningSystem.level_level;
            if(current_level > WARNING_LEVEL_NORMAL)
            {
                float32_t level_percent = (g_WarningSystem.current_level / g_WarningSystem.level_config.well_depth) * 100;
                if(level_percent < g_WarningSystem.level_config.level1_threshold)
                {
                    new_level = WARNING_LEVEL_NORMAL;
                }
                else if(level_percent < g_WarningSystem.level_config.level2_threshold)
                {
                    new_level = WARNING_LEVEL_1;
                }
                else if(level_percent < g_WarningSystem.level_config.level3_threshold)
                {
                    new_level = WARNING_LEVEL_2;
                }
                
                if(new_level < current_level)
                {
                    Update_WarningLevel(sensor_type, new_level);
                }
            }
            break;
            
        case SENSOR_TYPE_CONDUCTIVITY:
            current_level = g_WarningSystem.cond_level;
            if(current_level > WARNING_LEVEL_NORMAL)
            {
                if(g_WarningSystem.current_conductivity < g_WarningSystem.cond_config.level1_threshold)
                {
                    new_level = WARNING_LEVEL_NORMAL;
                }
                else if(g_WarningSystem.current_conductivity < g_WarningSystem.cond_config.level2_threshold)
                {
                    new_level = WARNING_LEVEL_1;
                }
                else if(g_WarningSystem.current_conductivity < g_WarningSystem.cond_config.level3_threshold)
                {
                    new_level = WARNING_LEVEL_2;
                }
                
                if(new_level < current_level)
                {
                    Update_WarningLevel(sensor_type, new_level);
                }
            }
            break;
            
        case SENSOR_TYPE_FLOW_RATE:
            current_level = g_WarningSystem.flow_level;
            if(current_level > WARNING_LEVEL_NORMAL)
            {
                if(g_WarningSystem.current_flow_rate < g_WarningSystem.flow_config.level1_threshold)
                {
                    new_level = WARNING_LEVEL_NORMAL;
                }
                else if(g_WarningSystem.current_flow_rate < g_WarningSystem.flow_config.level2_threshold)
                {
                    new_level = WARNING_LEVEL_1;
                }
                else if(g_WarningSystem.current_flow_rate < g_WarningSystem.flow_config.level3_threshold)
                {
                    new_level = WARNING_LEVEL_2;
                }
                
                if(new_level < current_level)
                {
                    Update_WarningLevel(sensor_type, new_level);
                }
            }
            break;
    }
}

/**
 * @brief  检查数据是否匹配目标档位
 * @param  sensor_type: 传感器类型
 * @param  data: 传感器数据
 * @param  target_level: 目标档位
 * @retval 1-匹配，0-不匹配
 */
uint8_t Is_LevelMatch(SensorType_TypeDef sensor_type, float32_t data, WarningLevel_TypeDef target_level)
{
    switch(sensor_type)
    {
        case SENSOR_TYPE_LEVEL:
            if(target_level == WARNING_LEVEL_1)
                return (data >= g_WarningSystem.level_config.level1_threshold && data < g_WarningSystem.level_config.level2_threshold) ? 1 : 0;
            else if(target_level == WARNING_LEVEL_2)
                return (data >= g_WarningSystem.level_config.level2_threshold && data < g_WarningSystem.level_config.level3_threshold) ? 1 : 0;
            else if(target_level == WARNING_LEVEL_3)
                return (data >= g_WarningSystem.level_config.level3_threshold) ? 1 : 0;
            break;
            
        case SENSOR_TYPE_CONDUCTIVITY:
            if(target_level == WARNING_LEVEL_1)
                return (data >= g_WarningSystem.cond_config.level1_threshold && data < g_WarningSystem.cond_config.level2_threshold) ? 1 : 0;
            else if(target_level == WARNING_LEVEL_2)
                return (data >= g_WarningSystem.cond_config.level2_threshold && data < g_WarningSystem.cond_config.level3_threshold) ? 1 : 0;
            else if(target_level == WARNING_LEVEL_3)
                return (data >= g_WarningSystem.cond_config.level3_threshold) ? 1 : 0;
            break;
            
        case SENSOR_TYPE_FLOW_RATE:
            if(target_level == WARNING_LEVEL_1)
                return (data >= g_WarningSystem.flow_config.level1_threshold && data < g_WarningSystem.flow_config.level2_threshold) ? 1 : 0;
            else if(target_level == WARNING_LEVEL_2)
                return (data >= g_WarningSystem.flow_config.level2_threshold && data < g_WarningSystem.flow_config.level3_threshold) ? 1 : 0;
            else if(target_level == WARNING_LEVEL_3)
                return (data >= g_WarningSystem.flow_config.level3_threshold) ? 1 : 0;
            break;
            
        default:
            return 0;
    }
    return 0;
}


/**
 * @brief  检查历史数据是否满足预警条件
 * @param  sensor_type: 传感器类型
 * @param  target_level: 目标档位
 * @retval 1-满足，0-不满足
 */
uint8_t Check_HistoryData(SensorType_TypeDef sensor_type, WarningLevel_TypeDef target_level)
{
    if(target_level == WARNING_LEVEL_NORMAL)
    {
        // 非预警档位不需要历史数据判断
        return 1;
    }
    
    uint8_t count = 0;
    uint8_t required_group = g_WarningSystem.level_config.warning_group;
    
    switch(sensor_type)
    {
        case SENSOR_TYPE_LEVEL:
            required_group = g_WarningSystem.level_config.warning_group;
            for(uint8_t i=0; i<WARNING_GROUP_COUNT; i++)
            {
                float32_t level_percent = (g_WarningSystem.level_history[i] / g_WarningSystem.level_config.well_depth) * 100;
                if(Is_LevelMatch(sensor_type, level_percent, target_level))
                {
                    count++;
                }
            }
            break;
            
        case SENSOR_TYPE_CONDUCTIVITY:
            required_group = g_WarningSystem.cond_config.warning_group;
            for(uint8_t i=0; i<WARNING_GROUP_COUNT; i++)
            {
                if(Is_LevelMatch(sensor_type, g_WarningSystem.cond_history[i], target_level))
                {
                    count++;
                }
            }
            break;
            
        case SENSOR_TYPE_FLOW_RATE:
            required_group = g_WarningSystem.flow_config.warning_group;
            for(uint8_t i=0; i<WARNING_GROUP_COUNT; i++)
            {
                if(Is_LevelMatch(sensor_type, g_WarningSystem.flow_history[i], target_level))
                {
                    count++;
                }
            }
            break;
            
        default:
            return 0;
    }
    
    // 检查是否满足所需组数
    return (count >= required_group) ? 1 : 0;
}

/**
 * @brief  预警档位判断
 * @param  无
 * @retval 无
 */
void WarningLevel_Judge(void)
{
    // 1. 液位档位判断
    if(g_WarningSystem.level_config.warning_switch == 1)
    {
        WarningLevel_TypeDef new_level = WARNING_LEVEL_NORMAL;
        float32_t level_percent = (g_WarningSystem.current_level / g_WarningSystem.level_config.well_depth) * 100;
        
        // 检查是否达到各档位阈值
        if(level_percent >= g_WarningSystem.level_config.level3_threshold)
        {
            new_level = WARNING_LEVEL_3;
        }
        else if(level_percent >= g_WarningSystem.level_config.level2_threshold)
        {
            new_level = WARNING_LEVEL_2;
        }
        else if(level_percent >= g_WarningSystem.level_config.level1_threshold)
        {
            new_level = WARNING_LEVEL_1;
        }
        
        // 检查是否满足组数要求
        if(Check_HistoryData(SENSOR_TYPE_LEVEL, new_level))
        {
            Update_WarningLevel(SENSOR_TYPE_LEVEL, new_level);
        }
    }
    
    // 2. 电导率档位判断
    if(g_WarningSystem.cond_config.warning_switch == 1)
    {
        WarningLevel_TypeDef new_level = WARNING_LEVEL_NORMAL;
        
        if(g_WarningSystem.current_conductivity >= g_WarningSystem.cond_config.level3_threshold)
        {
            new_level = WARNING_LEVEL_3;
        }
        else if(g_WarningSystem.current_conductivity >= g_WarningSystem.cond_config.level2_threshold)
        {
            new_level = WARNING_LEVEL_2;
        }
        else if(g_WarningSystem.current_conductivity >= g_WarningSystem.cond_config.level1_threshold)
        {
            new_level = WARNING_LEVEL_1;
        }
        
        if(Check_HistoryData(SENSOR_TYPE_CONDUCTIVITY, new_level))
        {
            Update_WarningLevel(SENSOR_TYPE_CONDUCTIVITY, new_level);
        }
    }
    
    // 3. 流速档位判断
    if(g_WarningSystem.flow_config.warning_switch == 1 && g_WarningSystem.flow_config.flow_switch == 1)
    {
        WarningLevel_TypeDef new_level = WARNING_LEVEL_NORMAL;
        
        if(g_WarningSystem.current_flow_rate >= g_WarningSystem.flow_config.level3_threshold)
        {
            new_level = WARNING_LEVEL_3;
        }
        else if(g_WarningSystem.current_flow_rate >= g_WarningSystem.flow_config.level2_threshold)
        {
            new_level = WARNING_LEVEL_2;
        }
        else if(g_WarningSystem.current_flow_rate >= g_WarningSystem.flow_config.level1_threshold)
        {
            new_level = WARNING_LEVEL_1;
        }
        
        if(Check_HistoryData(SENSOR_TYPE_FLOW_RATE, new_level))
        {
            Update_WarningLevel(SENSOR_TYPE_FLOW_RATE, new_level);
        }
    }
    
    // 4. 系统综合档位判断（最高优先原则）
    Update_SystemWarningLevel();
}


// ============================ 数据采集处理函数 ============================
/**
 * @brief  传感器数据采集
 * @param  无
 * @retval 0-成功，1-失败
 */
uint8_t Sensor_DataCollect(void)
{
    if(g_WarningSystem.system_init_flag == 0)
    {
        return 1;
    }
    
    uint32_t current_time = Timer_GetSecCount();
    
    // 检查采集周期
    uint32_t collect_interval = Get_CollectInterval(g_WarningSystem.system_level);
    if((current_time - g_WarningSystem.last_collect_time) < collect_interval)
    {
        return 0; // 未到采集时间
    }
    
    // 读取传感器数据
    g_WarningSystem.current_level = gSt_DevMeasRecordData.fWaterLevel;//Sensor_ReadLevel();
    g_WarningSystem.current_conductivity = gSt_DevMeasRecordData.fWaterQuality_COND;//Sensor_ReadConductivity();
    g_WarningSystem.current_flow_rate = gSt_DevMeasRecordData.fWaterSpeed;//Sensor_ReadFlowRate();
    
    // 存储历史数据
    g_WarningSystem.level_history[g_WarningSystem.history_index] = g_WarningSystem.current_level;
    g_WarningSystem.cond_history[g_WarningSystem.history_index] = g_WarningSystem.current_conductivity;
    g_WarningSystem.flow_history[g_WarningSystem.history_index] = g_WarningSystem.current_flow_rate;
    
    // 更新历史数据索引
    g_WarningSystem.history_index = (g_WarningSystem.history_index + 1) % WARNING_GROUP_COUNT;
    
    // 更新采集时间
    g_WarningSystem.last_collect_time = current_time;
    
    // 进行预警档位判断
    WarningLevel_Judge();
    
    return 0;
}
#endif
/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
//static SystemPataSt *pst_MagSystemPara;
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/


/******************************************************************************
 * 
 * EOF (not truncated)
 *****************************************************************************/
