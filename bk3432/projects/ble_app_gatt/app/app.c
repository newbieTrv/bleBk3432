/**
 ****************************************************************************************
 *
 * @file app.c
 *
 * @brief Application entry point
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"             // SW configuration

#if (BLE_APP_PRESENT)
#include <string.h>
//#include "rwapp_config.h"
#include "app_task.h"                // Application task Definition
#include "app.h"                     // Application Definition
#include "gap.h"                     // GAP Definition
#include "gapm_task.h"               // GAP Manager Task API
#include "gapc_task.h"               // GAP Controller Task API

#include "co_bt.h"                   // Common BT Definition
#include "co_math.h"                 // Common Maths Definition
#include "ke_timer.h"
//#include "app_fff0.h"                 // Application security Definition
#include "app_tuya_service.h"  
#include "app_tuya.h"  
//#include "app_dis.h"                 // Device Information Service Application Definitions
//#include "app_batt.h"                // Battery Application Definitions
//#include "app_oads.h"                 // Application oads Definition
#if (NVDS_SUPPORT)
#include "nvds.h"                    // NVDS Definitions
#endif
#include "rf.h"
#include "uart.h"
#include "adc.h"
#include "gpio.h"
#include "wdt.h"
#include "rtc.h"
#include "tuya_ble_main.h"
/*
 * DEFINES
 ****************************************************************************************
 */
#define APP_DEVICE_NAME_LENGTH_MAX      (18)


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

typedef void (*appm_add_svc_func_t)(void);

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of service to add in the database
enum appm_svc_list
{
    APPM_SVC_TUYA,
    APPM_SVC_LIST_STOP ,
};

/*
 * LOCAL VARIABLES DEFINITIONS
 ****************************************************************************************
 */

/// Application Task Descriptor
static const struct ke_task_desc TASK_DESC_APP = {NULL, &appm_default_handler,
                                                  appm_state, APPM_STATE_MAX, APP_IDX_MAX};

/// List of functions used to create the database
static const appm_add_svc_func_t appm_add_svc_func_list[APPM_SVC_LIST_STOP] =
{
    (appm_add_svc_func_t)app_tuya_add_service,
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Application Environment Structure
struct app_env_tag app_env;

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void appm_init()
{
    // Reset the application manager environment
    memset(&app_env, 0, sizeof(app_env));

    // Create APP task
    ke_task_create(TASK_APP, &TASK_DESC_APP);

    // Initialize Task state
    ke_state_set(TASK_APP, APPM_INIT);

	app_env.dev_name_len = sizeof(APP_DFLT_DEVICE_NAME);
	memcpy(&app_env.dev_name[0], APP_DFLT_DEVICE_NAME, sizeof(APP_DFLT_DEVICE_NAME));

#if (NVDS_SUPPORT)
	  uint8_t key_len = KEY_LEN;
    if (nvds_get(NVDS_TAG_LOC_IRK, &key_len, app_env.loc_irk) != NVDS_OK)
#endif        
    {
        uint8_t counter;

        // generate a new IRK
        for (counter = 0; counter < KEY_LEN; counter++)
        {
            app_env.loc_irk[counter]    = (uint8_t)co_rand_word();
        }
#if (NVDS_SUPPORT)
        // Store the generated value in NVDS
        if (nvds_put(NVDS_TAG_LOC_IRK, KEY_LEN, (uint8_t *)&app_env.loc_irk) != NVDS_OK)
        {
            ASSERT_INFO(0, NVDS_TAG_LOC_IRK, 0);
        }
#endif        
    }


    /*------------------------------------------------------
     * INITIALIZE ALL MODULES
     *------------------------------------------------------*/   
    
    app_tuya_init();
}

bool appm_add_svc(void)
{
    // Indicate if more services need to be added in the database
    bool more_svc = false;

    // Check if another should be added in the database
    if (app_env.next_svc != APPM_SVC_LIST_STOP)
    {
        ASSERT_INFO(appm_add_svc_func_list[app_env.next_svc] != NULL, app_env.next_svc, 1);

        // Call the function used to add the required service
        appm_add_svc_func_list[app_env.next_svc]();

        // Select following service to add
        app_env.next_svc++;
        more_svc = true;
    }

    return more_svc;
}


/*设备主动断开连接函数*/
void appm_disconnect(void)
{
    struct gapc_disconnect_cmd *cmd = KE_MSG_ALLOC(GAPC_DISCONNECT_CMD,
                                                   KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                                                   gapc_disconnect_cmd);

    cmd->operation = GAPC_DISCONNECT;
    cmd->reason    = CO_ERROR_REMOTE_USER_TERM_CON;

    // Send the message
    ke_msg_send(cmd);
}

extern uint8_t adv_data[TUYA_BLE_ADV_DATA_LEN];
extern uint8_t scan_rsp_data[TUYA_BLE_SCAN_RSP_DATA_LEN];

void appm_start_advertising(void)
{	
    // Check if the advertising procedure is already is progress
    if (ke_state_get(TASK_APP) == APPM_READY)
    {				
        // Prepare the GAPM_START_ADVERTISE_CMD message
        struct gapm_start_advertise_cmd *cmd = KE_MSG_ALLOC(GAPM_START_ADVERTISE_CMD,
                                                            TASK_GAPM, TASK_APP,
                                                            gapm_start_advertise_cmd);

        cmd->op.addr_src    = GAPM_STATIC_ADDR;
        cmd->channel_map    = APP_ADV_CHMAP;
        cmd->intv_min 		= APP_ADV_INT_MIN;
        cmd->intv_max 		= APP_ADV_INT_MAX;	
        cmd->op.code        = GAPM_ADV_UNDIRECT;
		
        cmd->info.host.mode = GAP_GEN_DISCOVERABLE;

 		/*-----------------------------------------------------------------------------------
         * Set the Advertising Data and the Scan Response Data
         *---------------------------------------------------------------------------------*/
        // Flag value is set by the GAP
        cmd->info.host.adv_data_len       = ADV_DATA_LEN;
        cmd->info.host.scan_rsp_data_len  = SCAN_RSP_DATA_LEN;

#if (NVDS_SUPPORT)
        // Advertising Data
        if(nvds_get(NVDS_TAG_APP_BLE_ADV_DATA, &cmd->info.host.adv_data_len,
                    &cmd->info.host.adv_data[0]) != NVDS_OK)
#endif                    
        {
            cmd->info.host.adv_data_len = 0;
					
            memcpy(&cmd->info.host.adv_data[0],
                   adv_data, TUYA_BLE_ADV_DATA_LEN);
            cmd->info.host.adv_data_len += TUYA_BLE_ADV_DATA_LEN;
        }

#if (NVDS_SUPPORT)
        // Scan Response Data
        if(nvds_get(NVDS_TAG_APP_BLE_SCAN_RESP_DATA, &cmd->info.host.scan_rsp_data_len,
                    &cmd->info.host.scan_rsp_data[0]) != NVDS_OK)
#endif                    
        {
            cmd->info.host.scan_rsp_data_len = 0;

			memcpy(&cmd->info.host.scan_rsp_data[cmd->info.host.scan_rsp_data_len],
                   scan_rsp_data, TUYA_BLE_SCAN_RSP_DATA_LEN);
            cmd->info.host.scan_rsp_data_len += TUYA_BLE_SCAN_RSP_DATA_LEN;
        }

        // Send the message
        ke_msg_send(cmd);
	 	UART_PRINTF("appm start advertising\r\n");

		wdt_enable(0x3fff);

        // Set the state of the task to APPM_ADVERTISING
        ke_state_set(TASK_APP, APPM_ADVERTISING);	
                
		//ke_timer_set(APP_PERIOD_TIMER, TASK_APP, 1500);	
    }
	
    // else ignore the request
}


/* 设备主动停止广播函数*/
void appm_stop_advertising(void)
{
    if (ke_state_get(TASK_APP) == APPM_ADVERTISING)
    {
        // Go in ready state
        ke_state_set(TASK_APP, APPM_READY);

        // Prepare the GAPM_CANCEL_CMD message
        struct gapm_cancel_cmd *cmd = KE_MSG_ALLOC(GAPM_CANCEL_CMD,
                                                   TASK_GAPM, TASK_APP,
                                                   gapm_cancel_cmd);
        cmd->operation = GAPM_CANCEL;

        // Send the message
        ke_msg_send(cmd);

		wdt_disable_flag = 1;
    }
    // else ignore the request
}



void appm_update_param(struct gapc_conn_param *conn_param)
{
    // Prepare the GAPC_PARAM_UPDATE_CMD message
    struct gapc_param_update_cmd *cmd = KE_MSG_ALLOC(GAPC_PARAM_UPDATE_CMD,
                                                     KE_BUILD_ID(TASK_GAPC, app_env.conidx), TASK_APP,
                                                     gapc_param_update_cmd);

    cmd->operation  = GAPC_UPDATE_PARAMS;
    cmd->intv_min   = conn_param->intv_min;
    cmd->intv_max   = conn_param->intv_max;
    cmd->latency    = conn_param->latency;
    cmd->time_out   = conn_param->time_out;

    // not used by a slave device
    cmd->ce_len_min = 0xFFFF;
    cmd->ce_len_max = 0xFFFF;
		
    UART_PRINTF("intv_min = %d,intv_max = %d,latency = %d,time_out = %d\r\n",
		cmd->intv_min,cmd->intv_max,cmd->latency,cmd->time_out);
	
    // Send the message
    ke_msg_send(cmd);
}

void appm_update_adv_data( uint8_t* adv_buff, uint8_t adv_len, uint8_t* scan_buff, uint8_t scan_len)
{
	if (ke_state_get(TASK_APP) == APPM_ADVERTISING 
                 && (adv_len <= ADV_DATA_LEN) && (scan_len <= ADV_DATA_LEN))
	{
		struct gapm_update_advertise_data_cmd *cmd =  KE_MSG_ALLOC(
            		            GAPM_UPDATE_ADVERTISE_DATA_CMD,
            		            TASK_GAPM,
            		            TASK_APP,
            		            gapm_update_advertise_data_cmd);

		cmd->operation = GAPM_UPDATE_ADVERTISE_DATA;
		cmd->adv_data_len = adv_len;
		cmd->scan_rsp_data_len = scan_len;

		//memcpy
		memcpy(&cmd->adv_data[0], adv_buff, adv_len);
		memcpy(&cmd->scan_rsp_data[0], scan_buff, scan_len);
        
		// Send the message
		ke_msg_send(cmd);
	}
}


uint8_t appm_get_dev_name(uint8_t* name)
{
    // copy name to provided pointer
    memcpy(name, app_env.dev_name, app_env.dev_name_len);
    // return name length
    return app_env.dev_name_len;
}


#endif //(BLE_APP_PRESENT)

/// @} APP


