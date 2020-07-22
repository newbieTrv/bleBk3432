/**
 * \file tuya_ble_main.c
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

#include "tuya_ble_stdlib.h"
#include "tuya_ble_type.h"
#include "tuya_ble_api.h"
#include "tuya_ble_port.h"
#include "tuya_ble_main.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_event_handler.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_log.h"

tuya_ble_parameters_settings_t  tuya_ble_current_para;


static volatile tuya_ble_connect_status_t tuya_ble_connect_status = UNKNOW_STATUS;

tuya_ble_callback_t m_cb_table[TUYA_BLE_MAX_CALLBACKS];


void tuya_ble_connect_status_set(tuya_ble_connect_status_t status)
{
    tuya_ble_device_enter_critical();
    tuya_ble_connect_status = status;
    tuya_ble_device_exit_critical();

}

tuya_ble_connect_status_t tuya_ble_connect_status_get(void)
{
    return  tuya_ble_connect_status;
}


void tuya_ble_event_init(void)
{
    for(uint8_t i= 0; i<TUYA_BLE_MAX_CALLBACKS; i++)
    {
        m_cb_table[i] = NULL;
    }

}


uint8_t tuya_ble_event_send(tuya_ble_evt_param_t *evt)
{

    if(tuya_ble_message_send(evt)==TUYA_BLE_SUCCESS)
    {
        return 0;
    }
    else
    {
        return 1;
    }

}


uint8_t tuya_ble_custom_event_send(tuya_ble_custom_evt_t evt)
{
    static tuya_ble_evt_param_t event;
    uint8_t ret = 0;

    tuya_ble_device_enter_critical();
    event.hdr.event = TUYA_BLE_EVT_CUSTOM;
    event.custom_evt = evt;

    if(tuya_ble_message_send(&event)==TUYA_BLE_SUCCESS)
    {
        ret = 0;
    }
    else
    {        
        ret = 1;
    }    
    tuya_ble_device_exit_critical();
    return ret;

}



tuya_ble_status_t tuya_ble_inter_event_response(tuya_ble_cb_evt_param_t *param)
{

    switch (param->evt)
    {
    case TUYA_BLE_CB_EVT_CONNECTE_STATUS:
        break;
    case TUYA_BLE_CB_EVT_DP_WRITE:
        if(param->dp_write_data.p_data)
        {
            tuya_ble_free(param->dp_write_data.p_data);
        }
        break;
    case TUYA_BLE_CB_EVT_DP_QUERY:
        if(param->dp_query_data.p_data)
        {
            tuya_ble_free(param->dp_query_data.p_data);
        }
        break;

    case TUYA_BLE_CB_EVT_TIME_STAMP:
        break;
    case TUYA_BLE_CB_EVT_TIME_NORMAL:
        break;
    case TUYA_BLE_CB_EVT_DATA_PASSTHROUGH:

        if(param->ble_passthrough_data.p_data)
        {
            tuya_ble_free(param->ble_passthrough_data.p_data);
        }
        break;
    default:
        break;
    }

    return TUYA_BLE_SUCCESS;
}



uint8_t tuya_ble_cb_event_send(tuya_ble_cb_evt_param_t *evt)
{
    tuya_ble_callback_t fun;
    if(m_cb_table[0])
    {
        fun = m_cb_table[0];
        fun(evt);
        tuya_ble_inter_event_response(evt);
    }

    return 0;

}



void tuya_ble_event_process(tuya_ble_evt_param_t *tuya_ble_evt)
{
    switch(tuya_ble_evt->hdr.event)
    {
    case TUYA_BLE_EVT_DEVICE_INFO_UPDATE:
        tuya_ble_handle_device_info_update_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_DP_DATA_REPORTED:
        tuya_ble_handle_dp_data_reported_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_DP_DATA_WITH_FLAG_REPORTED:
        tuya_ble_handle_dp_data_with_flag_reported_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_FACTORY_RESET:
        tuya_ble_handle_factory_reset_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_OTA_RESPONSE:
        
        break;
    case TUYA_BLE_EVT_DATA_PASSTHROUGH:
       
        break;
    case TUYA_BLE_EVT_PRODUCTION_TEST_RESPONSE:
        tuya_ble_handle_data_prod_test_response_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_MTU_DATA_RECEIVE:       
        tuya_ble_handle_ble_data_evt(tuya_ble_evt->mtu_data.data,tuya_ble_evt->mtu_data.len);
        break;
    case TUYA_BLE_EVT_DP_DATA_WITH_TIME_REPORTED:
        tuya_ble_handle_dp_data_with_time_reported_evt(tuya_ble_evt);
        break;
    
    case TUYA_BLE_EVT_DP_DATA_WITH_FLAG_AND_TIME_REPORTED:
        tuya_ble_handle_dp_data_with_flag_and_time_reported_evt(tuya_ble_evt);
        break;
    
    case TUYA_BLE_EVT_DP_DATA_WITH_TIME_STRING_REPORTED:
        tuya_ble_handle_dp_data_with_time_string_reported_evt(tuya_ble_evt);
        break;
    
    case TUYA_BLE_EVT_DP_DATA_WITH_FLAG_AND_TIME_STRING_REPORTED:
        tuya_ble_handle_dp_data_with_flag_and_time_string_reported_evt(tuya_ble_evt);
        break;
    
    case TUYA_BLE_EVT_UART_CMD:
        tuya_ble_handle_uart_cmd_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_BLE_CMD:
        tuya_ble_handle_ble_cmd_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_NET_CONFIG_RESPONSE:
        break;
    case TUYA_BLE_EVT_CUSTOM:
        tuya_ble_evt->custom_evt.custom_event_handler(tuya_ble_evt->custom_evt.evt_id,tuya_ble_evt->custom_evt.data);
        break;
    case TUYA_BLE_EVT_CONNECT_STATUS_UPDATE:
        tuya_ble_handle_connect_change_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_TIME_REQ:
        tuya_ble_handle_time_request_evt(tuya_ble_evt);
        break;
    case TUYA_BLE_EVT_UNBOUND_RESPONSE:
  
        break;
    case TUYA_BLE_EVT_ANOMALY_UNBOUND_RESPONSE:
        break;
    case TUYA_BLE_EVT_DEVICE_RESET_RESPONSE:
        break;
    
    case TUYA_BLE_EVT_GATT_SEND_DATA:
        tuya_ble_evt->hdr.event_handler((void *)tuya_ble_evt);
        break;

    default:
        break;
    }
}




#if (TUYA_BLE_PROTOCOL_VERSION_HIGN==3)

/** @brief  GAP - Advertisement data (max size = 31 bytes, best kept short to conserve power) */



static const uint8_t adv_data_const[TUYA_BLE_ADV_DATA_LEN] =
{
    0x02,
    0x01,
    0x06,
    0x03,
    0x02,
    0x01, 0xA2,
    0x14,
    0x16,
    0x01, 0xA2,
    0x00,         //id type 00-pid 01-product key
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/** @brief  GAP - scan response data (max size = 31 bytes) */


static const uint8_t scan_rsp_data_const[TUYA_BLE_SCAN_RSP_DATA_LEN] =
{
    0x03,
    0x09,
    0x54, 0x59,
    0x19,             /* length */
    0xFF,
    0xD0,
    0x07,
    0x00, //bond flag bit7 （8）
    0x03, //protocol version
    0x01, //Encry Mode （10）
    0x00,0x00, //communication way bit0-mesh bit1-wifi bit2-zigbee bit3-NB
    0x00, //data type
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

uint8_t adv_data[TUYA_BLE_ADV_DATA_LEN];
uint8_t scan_rsp_data[TUYA_BLE_SCAN_RSP_DATA_LEN];


void tuya_ble_adv_change(void)
{
    uint8_t *aes_buf = NULL;
    uint8_t aes_key[16];
    uint8_t encry_device_id[DEVICE_ID_LEN];

    memcpy(adv_data,adv_data_const,TUYA_BLE_ADV_DATA_LEN);
    memcpy(&scan_rsp_data,scan_rsp_data_const,TUYA_BLE_SCAN_RSP_DATA_LEN);

    adv_data[7] = 4+tuya_ble_current_para.pid_len;
    adv_data[11] = tuya_ble_current_para.pid_type;
    
    if(TUYA_BLE_DEVICE_SHARED)
    {
        scan_rsp_data[8] |=0x02 ;
    }
    else
    {
        scan_rsp_data[8] &=(~0x02);
    }

    scan_rsp_data[9] = TUYA_BLE_PROTOCOL_VERSION_HIGN;

    scan_rsp_data[10] = TUYA_BLE_SECURE_CONNECTION_TYPE;

    scan_rsp_data[11] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY>>8;
    scan_rsp_data[12] = TUYA_BLE_DEVICE_COMMUNICATION_ABILITY;
    
    if(tuya_ble_current_para.sys_settings.bound_flag == 1)
    {
        scan_rsp_data[8] |=0x80 ;
        //
        memcpy(aes_key,tuya_ble_current_para.sys_settings.login_key,LOGIN_KEY_LEN);
        memcpy(aes_key+LOGIN_KEY_LEN,tuya_ble_current_para.auth_settings.device_id,16-LOGIN_KEY_LEN);

        aes_buf = tuya_ble_malloc(200);

        if(aes_buf==NULL)
        {
            return;
        }
        else
        {
            memset(aes_buf,0,200);
        }

        tuya_ble_encrypt_old_with_key(aes_key,tuya_ble_current_para.auth_settings.device_id,DEVICE_ID_LEN,aes_buf);

        memcpy(&adv_data[12],(uint8_t *)(aes_buf+1),tuya_ble_current_para.pid_len);

        tuya_ble_free(aes_buf);

        memset(aes_key,0,sizeof(aes_key));
        memcpy(aes_key,&adv_data[12],tuya_ble_current_para.pid_len);

        tuya_ble_device_id_encrypt(aes_key,tuya_ble_current_para.pid_len,tuya_ble_current_para.auth_settings.device_id,DEVICE_ID_LEN,encry_device_id);

        memcpy(&scan_rsp_data[14],encry_device_id,DEVICE_ID_LEN);

    }
    else
    {
        scan_rsp_data[8] &=(~0x80);

        memcpy(&adv_data[12],tuya_ble_current_para.pid,tuya_ble_current_para.pid_len);
        tuya_ble_device_id_encrypt(tuya_ble_current_para.pid,tuya_ble_current_para.pid_len,tuya_ble_current_para.auth_settings.device_id,DEVICE_ID_LEN,encry_device_id);


        memcpy(&scan_rsp_data[14],encry_device_id,DEVICE_ID_LEN);
    }

       
    TUYA_BLE_LOG_INFO("adv data changed ,current bound flag = %d",tuya_ble_current_para.sys_settings.bound_flag);

    tuya_ble_gap_advertising_adv_data_update(adv_data,sizeof(adv_data),scan_rsp_data,sizeof(scan_rsp_data));

}



#else

//

#endif



