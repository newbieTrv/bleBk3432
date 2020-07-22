/**
 * \file tuya_ble_api.h
 *
 * \brief 
 */
/*
 *  Copyright (C) 2014-2019, Tuya Inc., All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of tuya ble sdk 
 */


#ifndef TUYA_BLE_API_H__
#define TUYA_BLE_API_H__

#include "tuya_ble_type.h"
#include "tuya_ble_port.h"


/**
 * @brief   Function for transmit ble data from peer devices to tuya sdk.
 *
 * @note    This function must be called from where the ble data is received. 
 *.
 * */
tuya_ble_status_t tuya_ble_gatt_receive_data(uint8_t *p_data,uint16_t len);

/**
 * @brief   Function for transmit ble data from peer devices to tuya sdk.
 *
 * @note    This function must be called from where the ble data is received. 
 *.
 * */
tuya_ble_status_t tuya_ble_common_uart_receive_data(uint8_t *p_data,uint16_t len);

/**
 * @brief   Function for send the full instruction received from uart to the sdk.
 *
 * @param
 *          [in]p_data  : pointer to full instruction data(Complete instruction,include0x55 or 0x66 0xaa and checksum.)
 *          [in]len     : Number of bytes of pdata.
 *
 * @note    If the application uses a custom uart parsing algorithm to obtain the full uart instruction,then call this function to send the full instruction.
 *
 * */
tuya_ble_status_t tuya_ble_common_uart_send_full_instruction_received(uint8_t *p_data,uint16_t len);


/**
 * @brief   Function for update the device id to tuya sdk.
 *
 * @note    the following id of the device must be update immediately when changed.
 *.
 * */

tuya_ble_status_t tuya_ble_device_update_product_id(tuya_ble_product_id_type_t type, uint8_t len, uint8_t* p_buf);

/**
 * @brief   Function for initialize the tuya sdk.
 *
 * @note    appliction should call this after all platform init complete.
 *.
 * */

tuya_ble_status_t tuya_ble_sdk_init(tuya_ble_device_param_t * param_data);



/**
 * @brief   Function for report the dp point data.
 *
 * @note    
 *.
 * */

tuya_ble_status_t tuya_ble_dp_data_report(uint8_t *p_data,uint32_t len); 

/**
 * @brief   Function for report the dp point data with time.
 *
 * @note    
 *.
 * */

tuya_ble_status_t tuya_ble_dp_data_with_time_report(uint32_t timestamp,uint8_t *p_data,uint32_t len);

/**
 * @brief   Function for report the dp point data with time.
 *
 * @note    time_string: 13-byte millisecond string ,for example ,"0000000123456";
 *.
 * */

tuya_ble_status_t tuya_ble_dp_data_with_time_ms_string_report(uint8_t *time_string,uint8_t *p_data,uint32_t len);


/**
 * @brief   Function for report the dp point data with flag.
 *
 * @note    
 *.
 * */

tuya_ble_status_t tuya_ble_dp_data_with_flag_report(uint16_t sn,tuya_ble_report_mode_t mode,uint8_t *p_data,uint32_t len); 

/**
 * @brief   Function for report the dp point data with flag and time.
 *
 * @note    
 *.
 * */

tuya_ble_status_t tuya_ble_dp_data_with_flag_and_time_report(uint16_t sn,tuya_ble_report_mode_t mode,uint32_t timestamp,uint8_t *p_data,uint32_t len);

/**
 * @brief   Function for report the dp point data with flag and time.
 *
 * @note    time_string: 13-byte millisecond string ,for example ,"0000000123456";
 *.
 * */
tuya_ble_status_t tuya_ble_dp_data_with_flag_and_time_ms_string_report(uint16_t sn,tuya_ble_report_mode_t mode,uint8_t *time_string,uint8_t *p_data,uint32_t len);


/**
 * @brief   Function for response the production test instruction asynchronous.
 *
 * @param   [in]channel: 0-uart ,1 - ble.
 *          [in]p_data  : pointer to production test cmd data(Complete instruction,include0x66 0xaa and checksum.)
 *          [in]len    : Number of bytes of pdata.
 * @note
 *.
 * */
tuya_ble_status_t tuya_ble_production_test_asynchronous_response(uint8_t channel,uint8_t *p_data,uint32_t len);
/**
 * @brief   Function for process the internal state of tuya sdk, application should  call this in connect handler.
 *
 * @note    
 *.
 * */
void tuya_ble_connected_handler(void);


/**
 * @brief   Function for process the internal state of tuya sdk, application should  call this in disconnect handler.
 *
 * @note    
 *.
 * */
void tuya_ble_disconnected_handler(void);

/**
 * @brief   Function for get the ble connet status.
 *
 * @note    
 *.
 * */

tuya_ble_connect_status_t tuya_ble_connect_status_get(void);

/**
 * @brief   Function for notify the sdk the device has resumes factory Settings.
 *
 * @note    When the device resumes factory Settings,shoule notify the sdk.
 *.
 * */

tuya_ble_status_t tuya_ble_device_factory_reset(void);


/**
 * @brief   Function for Request update time.
 *
 * @param time_type: 0-13-byte millisecond string ,1 - normal time format
 * @note    
 *.
 * */
tuya_ble_status_t tuya_ble_time_req(uint8_t time_type);
	
/**
 * @brief   Function for registe call back functions.
 *
 * @note    appliction should receive the message from the call back registed by this function.
 * 
 * */	
tuya_ble_status_t tuya_ble_callback_queue_register(tuya_ble_callback_t cb);

/**
 * @brief   Function for connect monitor timeout functions.
 *
 * @note    
 * 
 * */	
void tuya_ble_vtimer_connect_monitor_callback(void);

#endif

