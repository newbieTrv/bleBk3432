/**
 * \file tuya_ble_event_handler.c
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
#include "tuya_ble_data_handler.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_app_uart_common_handler.h"
#include "tuya_ble_app_production_test.h"
#include "tuya_ble_log.h"
#include "tuya_ble_event_handler.h"


void tuya_ble_handle_device_info_update_evt(tuya_ble_evt_param_t *evt)
{

    switch(evt->device_info_data.type)
    {
    case DEVICE_INFO_TYPE_PID:
        tuya_ble_current_para.pid_type = TUYA_BLE_PRODUCT_ID_TYPE_PID;
        tuya_ble_current_para.pid_len = evt->device_info_data.len;
        memcpy(tuya_ble_current_para.pid,evt->device_info_data.data,tuya_ble_current_para.pid_len);
        tuya_ble_adv_change();
        break;

    case DEVICE_INFO_TYPE_PRODUCT_KEY:
        tuya_ble_current_para.pid_type = TUYA_BLE_PRODUCT_ID_TYPE_PRODUCT_KEY;
        tuya_ble_current_para.pid_len = evt->device_info_data.len;
        memcpy(tuya_ble_current_para.pid,evt->device_info_data.data,tuya_ble_current_para.pid_len);
        tuya_ble_adv_change();
        break;
    case DEVICE_INFO_TYPE_LOGIN_KEY:

        break;
    case DEVICE_INFO_TYPE_BOUND:

        break;
    default:
        break;
    }

}


void tuya_ble_handle_dp_data_reported_evt(tuya_ble_evt_param_t *evt)
{
    uint8_t encry_mode = 0;
    if(tuya_ble_pair_rand_valid_get()==1)
    {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    }
    else
    {
        encry_mode = ENCRYPTION_MODE_KEY_4;
    }
    tuya_ble_commData_send(FRM_STAT_REPORT,0,evt->reported_data.p_data,evt->reported_data.data_len,encry_mode);

    if(evt->reported_data.p_data)
    {
        tuya_ble_free(evt->reported_data.p_data);
    }
}


void tuya_ble_handle_dp_data_with_flag_reported_evt(tuya_ble_evt_param_t *evt)
{
    uint8_t encry_mode = 0;
    if(tuya_ble_pair_rand_valid_get()==1)
    {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    }
    else
    {
        encry_mode = ENCRYPTION_MODE_KEY_4;
    }

    tuya_ble_commData_send(FRM_DATA_WITH_FLAG_REPORT,0,evt->flag_reported_data.p_data,evt->flag_reported_data.data_len,encry_mode);

    if(evt->flag_reported_data.p_data)
    {
        tuya_ble_free(evt->flag_reported_data.p_data);
    }
}


void tuya_ble_handle_dp_data_with_time_reported_evt(tuya_ble_evt_param_t *evt)
{
    uint8_t *data_buffer = NULL;
    uint16_t data_len;
    uint8_t encry_mode = 0;

    data_len = 5+evt->reported_with_time_data.data_len;

    data_buffer=(uint8_t*)tuya_ble_malloc(data_len);
    if(data_buffer==NULL)
    {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_with_time_reported_evt malloc failed.");
        if(evt->reported_with_time_data.p_data)
        {
            tuya_ble_free(evt->reported_with_time_data.p_data);
        }
        return;
    }
    else
    {
        memset(data_buffer,0,data_len);
    }

    if(tuya_ble_pair_rand_valid_get()==1)
    {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    }
    else
    {
        encry_mode = ENCRYPTION_MODE_KEY_4;
    }

    data_buffer[0] = 1;
    data_buffer[1] = evt->reported_with_time_data.timestamp>>24;
    data_buffer[2] = evt->reported_with_time_data.timestamp>>16;
    data_buffer[3] = evt->reported_with_time_data.timestamp>>8;
    data_buffer[4] = evt->reported_with_time_data.timestamp;

    memcpy(&data_buffer[5],evt->reported_with_time_data.p_data,evt->reported_with_time_data.data_len);

    tuya_ble_commData_send(FRM_STAT_WITH_TIME_REPORT,0,data_buffer,data_len,encry_mode);

    tuya_ble_free(data_buffer);

    if(evt->reported_with_time_data.p_data)
    {
        tuya_ble_free(evt->reported_with_time_data.p_data);
    }
}

void tuya_ble_handle_dp_data_with_flag_and_time_reported_evt(tuya_ble_evt_param_t *evt)
{
    uint8_t *data_buffer = NULL;
    uint16_t data_len;
    uint8_t encry_mode = 0;

    data_len = 8+evt->flag_reported_with_time_data.data_len;

    data_buffer=(uint8_t*)tuya_ble_malloc(data_len);
    if(data_buffer==NULL)
    {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_with_flag_and_time_reported_evt malloc failed.");
        if(evt->flag_reported_with_time_data.p_data)
        {
            tuya_ble_free(evt->flag_reported_with_time_data.p_data);
        }
        return;
    }
    else
    {
        memset(data_buffer,0,data_len);
    }

    if(tuya_ble_pair_rand_valid_get()==1)
    {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    }
    else
    {
        encry_mode = ENCRYPTION_MODE_KEY_4;
    }

    data_buffer[0] = evt->flag_reported_with_time_data.sn>>8;
    data_buffer[1] = evt->flag_reported_with_time_data.sn;
    data_buffer[2] = evt->flag_reported_with_time_data.mode;
    data_buffer[3] = 1;
    data_buffer[4] = evt->flag_reported_with_time_data.timestamp>>24;
    data_buffer[5] = evt->flag_reported_with_time_data.timestamp>>16;
    data_buffer[6] = evt->flag_reported_with_time_data.timestamp>>8;
    data_buffer[7] = evt->flag_reported_with_time_data.timestamp;

    memcpy(&data_buffer[8],evt->flag_reported_with_time_data.p_data,evt->flag_reported_with_time_data.data_len);

    tuya_ble_commData_send(FRM_DATA_WITH_FLAG_AND_TIME_REPORT,0,data_buffer,data_len,encry_mode);

    tuya_ble_free(data_buffer);

    if(evt->flag_reported_with_time_data.p_data)
    {
        tuya_ble_free(evt->flag_reported_with_time_data.p_data);
    }
}


void tuya_ble_handle_dp_data_with_time_string_reported_evt(tuya_ble_evt_param_t *evt)
{
    uint8_t *data_buffer = NULL;
    uint16_t data_len;
    uint8_t encry_mode = 0;

    data_len = 14+evt->reported_with_time_string_data.data_len;

    data_buffer=(uint8_t*)tuya_ble_malloc(data_len);
    if(data_buffer==NULL)
    {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_with_time_string_reported_evt malloc failed.");
        if(evt->reported_with_time_string_data.p_data)
        {
            tuya_ble_free(evt->reported_with_time_string_data.p_data);
        }
        return;
    }
    else
    {
        memset(data_buffer,0,data_len);
    }

    if(tuya_ble_pair_rand_valid_get()==1)
    {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    }
    else
    {
        encry_mode = ENCRYPTION_MODE_KEY_4;
    }

    data_buffer[0] = 0;
    memcpy(&data_buffer[1],evt->reported_with_time_string_data.time_string,13);

    memcpy(&data_buffer[14],evt->reported_with_time_string_data.p_data,evt->reported_with_time_string_data.data_len);

    tuya_ble_commData_send(FRM_STAT_WITH_TIME_REPORT,0,data_buffer,data_len,encry_mode);

    tuya_ble_free(data_buffer);

    if(evt->reported_with_time_string_data.p_data)
    {
        tuya_ble_free(evt->reported_with_time_string_data.p_data);
    }
}

void tuya_ble_handle_dp_data_with_flag_and_time_string_reported_evt(tuya_ble_evt_param_t *evt)
{
    uint8_t *data_buffer = NULL;
    uint16_t data_len;
    uint8_t encry_mode = 0;

    data_len = 17+evt->flag_reported_with_time_string_data.data_len;

    data_buffer=(uint8_t*)tuya_ble_malloc(data_len);
    if(data_buffer==NULL)
    {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_dp_data_with_flag_and_time_string_reported_evt malloc failed.");
        if(evt->flag_reported_with_time_string_data.p_data)
        {
            tuya_ble_free(evt->flag_reported_with_time_string_data.p_data);
        }
        return;
    }
    else
    {
        memset(data_buffer,0,data_len);
    }

    if(tuya_ble_pair_rand_valid_get()==1)
    {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    }
    else
    {
        encry_mode = ENCRYPTION_MODE_KEY_4;
    }

    data_buffer[0] = evt->flag_reported_with_time_string_data.sn>>8;
    data_buffer[1] = evt->flag_reported_with_time_string_data.sn;
    data_buffer[2] = evt->flag_reported_with_time_string_data.mode;
    data_buffer[3] = 0;
    memcpy(&data_buffer[4],evt->flag_reported_with_time_string_data.time_string,13);

    memcpy(&data_buffer[17],evt->flag_reported_with_time_string_data.p_data,evt->flag_reported_with_time_string_data.data_len);

    tuya_ble_commData_send(FRM_DATA_WITH_FLAG_AND_TIME_REPORT,0,data_buffer,data_len,encry_mode);

    tuya_ble_free(data_buffer);

    if(evt->flag_reported_with_time_string_data.p_data)
    {
        tuya_ble_free(evt->flag_reported_with_time_string_data.p_data);
    }
}



void tuya_ble_handle_factory_reset_evt(tuya_ble_evt_param_t *evt)
{
    tuya_ble_cb_evt_param_t event;

    memset(tuya_ble_current_para.sys_settings.device_virtual_id,0,DEVICE_VIRTUAL_ID_LEN);
    tuya_ble_device_unbond();

    event.evt = TUYA_BLE_CB_EVT_CONNECTE_STATUS;
    event.connect_status = tuya_ble_connect_status_get();

    if(tuya_ble_cb_event_send(&event)!=0)
    {
        TUYA_BLE_LOG_ERROR("tuya_ble_handle_factory_reset_evt-tuya ble send cb event (connect status update) failed.");
    }
    else
    {

    }
}

void tuya_ble_handle_data_prod_test_response_evt(tuya_ble_evt_param_t *evt)
{
    uint8_t encry_mode = 0;
    tuya_ble_connect_status_t connect_status;

    if(evt->prod_test_res_data.channel==0)
    {
        tuya_ble_common_uart_send_data(evt->prod_test_res_data.p_data,evt->prod_test_res_data.data_len);
    }
    else if(evt->prod_test_res_data.channel==1)
    {
        connect_status = tuya_ble_connect_status_get();
        if(connect_status==BONDING_CONN)
        {
            encry_mode = ENCRYPTION_MODE_SESSION_KEY;
        }
        else
        {
            encry_mode = 0;
        }

        tuya_ble_commData_send(FRM_FACTORY_TEST_RESP,0,evt->prod_test_res_data.p_data,evt->prod_test_res_data.data_len,encry_mode);
    }
    
    if(evt->prod_test_res_data.p_data)
    {
        tuya_ble_free(evt->prod_test_res_data.p_data);
    }

}

void tuya_ble_handle_uart_cmd_evt(tuya_ble_evt_param_t *evt)
{
    uint8_t sum;
    uint8_t *uart_send_buffer;
    uint16_t uart_send_len;

    TUYA_BLE_LOG_HEXDUMP_DEBUG("received uart cmd data",(uint8_t*)evt->uart_cmd_data.p_data,evt->uart_cmd_data.data_len);//

    if(evt->uart_cmd_data.data_len>0)
    {
        sum = tuya_ble_check_sum(evt->uart_cmd_data.p_data,evt->uart_cmd_data.data_len-1);
        if(sum==evt->uart_cmd_data.p_data[evt->uart_cmd_data.data_len-1])
        {

            switch(evt->uart_cmd_data.p_data[0])
            {
            case 0x55:
                tuya_ble_uart_common_process(evt->uart_cmd_data.p_data,evt->uart_cmd_data.data_len);
                break;
            case 0x66:
                tuya_ble_app_production_test_process(0,evt->uart_cmd_data.p_data,evt->uart_cmd_data.data_len);
                break;
            default:
                break;
            };

        }
        else
        {
            TUYA_BLE_LOG_ERROR("uart receive data check_sum error , receive sum = 0x%02x ; cal sum = 0x%02x",evt->uart_cmd_data.p_data[evt->uart_cmd_data.data_len-1],sum);
        }
    }

    if(evt->uart_cmd_data.p_data)
    {
        tuya_ble_free(evt->uart_cmd_data.p_data);
    }
}


void tuya_ble_handle_ble_cmd_evt(tuya_ble_evt_param_t *evt)
{

    tuya_ble_evt_process(evt->ble_cmd_data.cmd,evt->ble_cmd_data.p_data,evt->ble_cmd_data.data_len);
    if(evt->ble_cmd_data.p_data)
    {
        tuya_ble_free(evt->ble_cmd_data.p_data);
    }
}


void tuya_ble_handle_time_request_evt(tuya_ble_evt_param_t *evt)
{
    uint8_t encry_mode = 0;
    uint16_t cmd;
    if(tuya_ble_pair_rand_valid_get()==1)
    {
        encry_mode = ENCRYPTION_MODE_SESSION_KEY;
    }
    else
    {
        encry_mode = ENCRYPTION_MODE_KEY_4;
    }

    if(evt->time_req_data.time_type==0)
    {
        cmd = FRM_GET_UNIX_TIME_CHAR_MS_REQ;
    }
    else
    {
        cmd = FRM_GET_UNIX_TIME_CHAR_DATE_REQ;
    }

    tuya_ble_commData_send(cmd,0,NULL,0,encry_mode);

}


extern void tuya_ble_connect_monitor_timer_start(void);
void tuya_ble_handle_connect_change_evt(tuya_ble_evt_param_t *evt)
{
    tuya_ble_cb_evt_param_t event;
    uint8_t send_cb_flag = 1;

    if(evt->connect_change_evt==TUYA_BLE_CONNECTED)
    {
        TUYA_BLE_LOG_INFO("Connected!");
        tuya_ble_reset_ble_sn();
        if(tuya_ble_current_para.sys_settings.bound_flag!=1)
        {
            tuya_ble_connect_status_set(UNBONDING_UNAUTH_CONN);
        }
        else
        {
            tuya_ble_connect_status_set(BONDING_UNAUTH_CONN);
        }

        tuya_ble_connect_monitor_timer_start();

    }
    else if(evt->connect_change_evt==TUYA_BLE_DISCONNECTED)
    {
        TUYA_BLE_LOG_INFO("Disonnected");

        tuya_ble_reset_ble_sn();

        tuya_ble_pair_rand_clear();

        if(tuya_ble_current_para.sys_settings.bound_flag==1)
        {
            tuya_ble_connect_status_set(BONDING_UNCONN);
        }
        else
        {
            tuya_ble_connect_status_set(UNBONDING_UNCONN);
        }
    }
    else
    {
        TUYA_BLE_LOG_WARNING("unknown connect_change_evt!");
    }

    if(send_cb_flag)
    {
        event.evt = TUYA_BLE_CB_EVT_CONNECTE_STATUS;
        event.connect_status = tuya_ble_connect_status_get();
        if(tuya_ble_cb_event_send(&event)!=0)
        {
            TUYA_BLE_LOG_ERROR("tuya ble send cb event failed.");
        }
        else
        {
            TUYA_BLE_LOG_DEBUG("tuya ble send cb event succeed.");
        }

    }

}


void tuya_ble_handle_ble_data_evt(uint8_t *buf,uint16_t len)
{
    tuya_ble_commonData_rx_proc(buf,len);
}


