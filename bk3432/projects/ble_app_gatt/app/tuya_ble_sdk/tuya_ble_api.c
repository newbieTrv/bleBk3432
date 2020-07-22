/**
 * \file tuya_ble_api.c
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
#include "tuya_ble_api.h"
#include "tuya_ble_port.h"
#include "tuya_ble_type.h"
#include "tuya_ble_log.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_main.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_internal_config.h"
#include "ke_timer.h"
#include "tuya_ble_gatt_send_queue.h"
#include "tuya_ble_app_production_test.h"
#include "tuya_ble_sdk_version.h"

static uint8_t m_callback_numbers = 0;


extern tuya_ble_callback_t m_cb_table[TUYA_BLE_MAX_CALLBACKS];
/**
 * @brief   Function for registe call back functions.
 *
 * @note    appliction should receive the message from the call back registed by this function.
 *
 * */
tuya_ble_status_t tuya_ble_callback_queue_register(tuya_ble_callback_t cb)
{
    tuya_ble_status_t ret;

    tuya_ble_device_enter_critical();
    if (m_callback_numbers == TUYA_BLE_MAX_CALLBACKS) {
        ret = TUYA_BLE_ERR_RESOURCES;
    } else {
        m_cb_table[m_callback_numbers] = cb;
        m_callback_numbers++;

        ret = TUYA_BLE_SUCCESS;
    }
    tuya_ble_device_exit_critical();

    return ret;
}

/**
 * @brief   Function for executing all enqueued tasks.
 *
 * @note    This function must be called from within the main loop. It will
 * execute all events scheduled since the last time it was called.
 * */

//__WEAK void tuya_ble_main_tasks_exec(void)
//{

//}



/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_gatt_receive_data(uint8_t *p_data,uint16_t len)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_MTU_DATA_RECEIVE;

    if(len>TUYA_BLE_DATA_MTU_MAX)
    {
        event.mtu_data.len = TUYA_BLE_DATA_MTU_MAX;
    }
    else
    {
        event.mtu_data.len = len;
    }
    memcpy(event.mtu_data.data,p_data,event.mtu_data.len);
    if(tuya_ble_event_send(&event)!=0)
    {
        TUYA_BLE_LOG_ERROR("tuya_event_send ble data error,data len = %d ", event.mtu_data.len);
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}


/*
 *@brief Function for receive uart data.
 *@param
 *
 *@note
 *
 * */

typedef enum {
    TUYA_BLE_UART_REV_STATE_FOUND_NULL,
    TUYA_BLE_UART_REV_STATE_FOUND_HEAD,
    TUYA_BLE_UART_REV_STATE_FOUND_CMD,
    TUYA_BLE_UART_REV_STATE_FOUND_LEN_H,
    TUYA_BLE_UART_REV_STATE_FOUND_LEN_L,
    TUYA_BLE_UART_REV_STATE_FOUND_DATA,
    TUYA_BLE_UART_REV_STATE_UNKOWN,
} tuya_ble_uart_rev_state_type_t;

#define UART_RX_BUFFER_MAX   300
#define UART_RX_DATA_LEN_MAX 249
static volatile tuya_ble_uart_rev_state_type_t current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_NULL;
static uint8_t UART_RX_Buffer[UART_RX_BUFFER_MAX];
static uint8_t UART_RX_Buffer_temp[3] = {0};
static uint16_t uart_data_len =  0;
static volatile uint16_t UART_RX_Count = 0;


static bool tuya_ble_common_uart_data_unpack(uint8_t data)
{
    bool ret = false;

    UART_RX_Buffer_temp[0] = UART_RX_Buffer_temp[1];
    UART_RX_Buffer_temp[1] = UART_RX_Buffer_temp[2];
    UART_RX_Buffer_temp[2] = data;

    if(((UART_RX_Buffer_temp[0]==0x55)||(UART_RX_Buffer_temp[0]==0x66))&&(UART_RX_Buffer_temp[1]==0xAA)&&((UART_RX_Buffer_temp[2]==0x00)||(UART_RX_Buffer_temp[2]==0x01)))
    {
        memset(UART_RX_Buffer,0,sizeof(UART_RX_Buffer));
        memcpy(UART_RX_Buffer,UART_RX_Buffer_temp,3);
        memset(UART_RX_Buffer_temp,0,3);
        UART_RX_Count = 3;
        current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_HEAD;
        uart_data_len = 0;
        return ret;
    }

    switch(current_uart_rev_state_type)
    {
    case TUYA_BLE_UART_REV_STATE_FOUND_NULL:
        break;
    case TUYA_BLE_UART_REV_STATE_FOUND_HEAD:
        UART_RX_Buffer[UART_RX_Count++] = data;
        current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_CMD;
        break;
    case TUYA_BLE_UART_REV_STATE_FOUND_CMD:
        UART_RX_Buffer[UART_RX_Count++] = data;
        current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_LEN_H;
        break;
    case TUYA_BLE_UART_REV_STATE_FOUND_LEN_H:
        UART_RX_Buffer[UART_RX_Count++] = data;
        uart_data_len = (UART_RX_Buffer[UART_RX_Count-2]<<8)|UART_RX_Buffer[UART_RX_Count-1];
        if(uart_data_len>UART_RX_DATA_LEN_MAX)
        {
            memset(UART_RX_Buffer_temp,0,3);
            memset(UART_RX_Buffer,0,sizeof(UART_RX_Buffer));
            UART_RX_Count = 0;
            current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_NULL;
            uart_data_len = 0;
        }
        else if(uart_data_len>0)
        {
            current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_LEN_L;
        }
        else
        {
            current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_DATA;
        }
        break;
    case TUYA_BLE_UART_REV_STATE_FOUND_LEN_L:
        UART_RX_Buffer[UART_RX_Count++] = data;   //DATA
        uart_data_len--;
        if(uart_data_len==0)
        {
            current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_DATA;
        }
        break;
    case TUYA_BLE_UART_REV_STATE_FOUND_DATA:
        UART_RX_Buffer[UART_RX_Count++] = data;  //sum data
        ret = true;
        break;
    default:
        memset(UART_RX_Buffer_temp,0,3);
        memset(UART_RX_Buffer,0,sizeof(UART_RX_Buffer));
        UART_RX_Count = 0;
        current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_NULL;
        uart_data_len = 0;
        break;
    };

    return ret;

}

tuya_ble_status_t tuya_ble_common_uart_receive_data(uint8_t *p_data,uint16_t len)
{
    tuya_ble_status_t ret = TUYA_BLE_ERR_NOT_FOUND;
    tuya_ble_evt_param_t event;
    // uint16_t rev_len,data_len;
    uint8_t* uart_evt_buffer;
    uint16_t i;

    for(i=0; i<len; i++)
    {
        if(tuya_ble_common_uart_data_unpack(p_data[i]))
        {

            uart_evt_buffer=(uint8_t*)tuya_ble_malloc(UART_RX_Count);

            if(uart_evt_buffer==NULL)
            {
                TUYA_BLE_LOG_ERROR("tuya_MemGet uart evt buffer fail.");
                ret = TUYA_BLE_ERR_NO_MEM;
            }
            else
            {
                event.hdr.event = TUYA_BLE_EVT_UART_CMD;

                event.uart_cmd_data.data_len = UART_RX_Count;

                event.uart_cmd_data.p_data = uart_evt_buffer;

                memcpy(event.uart_cmd_data.p_data,&UART_RX_Buffer[0],event.uart_cmd_data.data_len);

                if(tuya_ble_event_send(&event)!=0)
                {
                    TUYA_BLE_LOG_ERROR("tuya_event_send uart data error.");
                    tuya_ble_free(uart_evt_buffer);
                    ret = TUYA_BLE_ERR_BUSY;
                }
                else
                {
                    ret = TUYA_BLE_SUCCESS;
                }
            }
            memset(UART_RX_Buffer_temp,0,3);
            memset(UART_RX_Buffer,0,sizeof(UART_RX_Buffer));
            UART_RX_Count = 0;
            current_uart_rev_state_type = TUYA_BLE_UART_REV_STATE_FOUND_NULL;
            uart_data_len = 0;

        }
    }

    return ret;
}


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
tuya_ble_status_t tuya_ble_common_uart_send_full_instruction_received(uint8_t *p_data,uint16_t len)
{
    tuya_ble_status_t ret = TUYA_BLE_SUCCESS;
    tuya_ble_evt_param_t event;
    uint16_t rev_len,data_len;
    uint8_t* uart_evt_buffer;

    if(len<7)
    {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    if((p_data[0]!=0x55)&&(p_data[0]!=0x66))
    {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    uart_evt_buffer=(uint8_t*)tuya_ble_malloc(len);

    if(uart_evt_buffer==NULL)
    {
        TUYA_BLE_LOG_ERROR("tuya_MemGet uart evt buffer fail.");
        ret = TUYA_BLE_ERR_NO_MEM;
    }
    else
    {
        event.hdr.event = TUYA_BLE_EVT_UART_CMD;

        event.uart_cmd_data.data_len = len;

        event.uart_cmd_data.p_data = uart_evt_buffer;

        memcpy(event.uart_cmd_data.p_data,p_data,len);

        if(tuya_ble_event_send(&event)!=0)
        {
            TUYA_BLE_LOG_ERROR("tuya_event_send uart full cmd error.");
            tuya_ble_free(uart_evt_buffer);
            ret = TUYA_BLE_ERR_BUSY;
        }
    }

    return ret;
}




/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_device_update_product_id(tuya_ble_product_id_type_t type, uint8_t len, uint8_t* p_buf)
{
    tuya_ble_evt_param_t event;

    if(len==0)
    {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    if(len>TUYA_BLE_PRODUCT_ID_MAX_LEN)
    {
        len = TUYA_BLE_PRODUCT_ID_MAX_LEN;
    }

    event.hdr.event = TUYA_BLE_EVT_DEVICE_INFO_UPDATE;

    if(type==TUYA_BLE_PRODUCT_ID_TYPE_PID)
    {
        event.device_info_data.type = DEVICE_INFO_TYPE_PID;
    }
    else
    {
        event.device_info_data.type = DEVICE_INFO_TYPE_PRODUCT_KEY;
    }

    event.device_info_data.len = len;
    memcpy(event.device_info_data.data,p_buf,len);
    if(tuya_ble_event_send(&event)!=0)
    {
        TUYA_BLE_LOG_ERROR("tuya_event_send product id update error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}

/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_dp_data_report(uint8_t *p_data,uint32_t len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer = NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if(tuya_ble_connect_status_get()!=BONDING_CONN)
    {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if((len>TUYA_BLE_REPORT_MAX_DP_DATA_LEN)||(len==0))
    {
        TUYA_BLE_LOG_ERROR("report dp data len error,data len = %d , max data len = %d",len,TUYA_BLE_REPORT_MAX_DP_DATA_LEN);
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_data,len,&list,0);
    if(MTP_OK != ret)
    {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);

    ble_evt_buffer=(uint8_t *)tuya_ble_malloc(len);
    if(ble_evt_buffer==NULL)
    {
        return TUYA_BLE_ERR_NO_MEM;
    }
    else
    {
        memcpy(ble_evt_buffer,p_data,len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_REPORTED;
    evt.reported_data.p_data = ble_evt_buffer;
    evt.reported_data.data_len = len;

    if(tuya_ble_event_send(&evt)!=0)
    {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_dp_data_with_flag_report(uint16_t sn,tuya_ble_report_mode_t mode,uint8_t *p_data,uint32_t len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer = NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if(tuya_ble_connect_status_get()!=BONDING_CONN)
    {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if((len>(TUYA_BLE_REPORT_MAX_DP_DATA_LEN-3))||(len==0))
    {
        TUYA_BLE_LOG_ERROR("report flag dp data len error,data len = %d , max data len = %d",len,(TUYA_BLE_REPORT_MAX_DP_DATA_LEN-3));
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_data,len,&list,0);
    if(MTP_OK != ret)
    {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);

    ble_evt_buffer=(uint8_t *)tuya_ble_malloc(len+3);
    if(ble_evt_buffer==NULL)
    {
        return TUYA_BLE_ERR_NO_MEM;
    }
    else
    {
        ble_evt_buffer[0] = sn>>8;
        ble_evt_buffer[1] = sn;
        ble_evt_buffer[2] = mode;
        memcpy(ble_evt_buffer+3,p_data,len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_FLAG_REPORTED;
    evt.flag_reported_data.sn = sn;
    evt.flag_reported_data.mode = mode;
    evt.flag_reported_data.p_data = ble_evt_buffer;
    evt.flag_reported_data.data_len = len+3;

    if(tuya_ble_event_send(&evt)!=0)
    {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

/*
 *@brief
 *@param
 *
 *@note
 *
 * */

tuya_ble_status_t tuya_ble_dp_data_with_time_report(uint32_t timestamp,uint8_t *p_data,uint32_t len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer=NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if(tuya_ble_connect_status_get()!=BONDING_CONN)
    {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if((len>(TUYA_BLE_REPORT_MAX_DP_DATA_LEN-5))||(len==0))
    {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_data,len,&list,0);
    if(MTP_OK != ret)
    {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);
    ble_evt_buffer=(uint8_t *)tuya_ble_malloc(len);
    if(ble_evt_buffer==NULL)
    {
        return TUYA_BLE_ERR_NO_MEM;
    }
    else
    {
        memcpy(ble_evt_buffer,p_data,len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_TIME_REPORTED;
    evt.reported_with_time_data.timestamp = timestamp;
    evt.reported_with_time_data.p_data = ble_evt_buffer;
    evt.reported_with_time_data.data_len = len;

    if(tuya_ble_event_send(&evt)!=0)
    {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}
/*
 *@brief
 *@param
 *
 *@note
 *
 * */

tuya_ble_status_t tuya_ble_dp_data_with_flag_and_time_report(uint16_t sn,tuya_ble_report_mode_t mode,uint32_t timestamp,uint8_t *p_data,uint32_t len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer=NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if(tuya_ble_connect_status_get()!=BONDING_CONN)
    {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if((len>(TUYA_BLE_REPORT_MAX_DP_DATA_LEN-8))||(len==0))
    {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_data,len,&list,0);
    if(MTP_OK != ret)
    {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);
    ble_evt_buffer=(uint8_t *)tuya_ble_malloc(len);
    if(ble_evt_buffer==NULL)
    {
        return TUYA_BLE_ERR_NO_MEM;
    }
    else
    {
        memcpy(ble_evt_buffer,p_data,len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_FLAG_AND_TIME_REPORTED;
    evt.flag_reported_with_time_data.sn = sn;
    evt.flag_reported_with_time_data.mode = mode;
    evt.flag_reported_with_time_data.timestamp = timestamp;
    evt.flag_reported_with_time_data.p_data = ble_evt_buffer;
    evt.flag_reported_with_time_data.data_len = len;

    if(tuya_ble_event_send(&evt)!=0)
    {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

/*
 *@brief
 *@param
 *
 *@note  time_string: 13-byte millisecond string ,for example ,"0000000123456";
 *
 * */

tuya_ble_status_t tuya_ble_dp_data_with_time_ms_string_report(uint8_t *time_string,uint8_t *p_data,uint32_t len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer=NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if(tuya_ble_connect_status_get()!=BONDING_CONN)
    {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if((len>(TUYA_BLE_REPORT_MAX_DP_DATA_LEN-14))||(len==0))
    {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_data,len,&list,0);
    if(MTP_OK != ret)
    {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);
    ble_evt_buffer=(uint8_t *)tuya_ble_malloc(len);
    if(ble_evt_buffer==NULL)
    {
        return TUYA_BLE_ERR_NO_MEM;
    }
    else
    {
        memcpy(ble_evt_buffer,p_data,len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_TIME_STRING_REPORTED;
    memcpy(evt.reported_with_time_string_data.time_string,time_string,13);
    evt.reported_with_time_string_data.p_data = ble_evt_buffer;
    evt.reported_with_time_string_data.data_len = len;

    if(tuya_ble_event_send(&evt)!=0)
    {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

/*
 *@brief
 *@param
 *
 *@note  time_string: 13-byte millisecond string ,for example ,"0000000123456";
 *
 * */
tuya_ble_status_t tuya_ble_dp_data_with_flag_and_time_ms_string_report(uint16_t sn,tuya_ble_report_mode_t mode,uint8_t *time_string,uint8_t *p_data,uint32_t len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer=NULL;
    mtp_ret ret;
    klv_node_s *list = NULL;

    if(tuya_ble_connect_status_get()!=BONDING_CONN)
    {
        return TUYA_BLE_ERR_INVALID_STATE;
    }

    if((len>(TUYA_BLE_REPORT_MAX_DP_DATA_LEN-17))||(len==0))
    {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }
    ret = data_2_klvlist(p_data,len,&list,0);
    if(MTP_OK != ret)
    {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }
    free_klv_list(list);
    ble_evt_buffer=(uint8_t *)tuya_ble_malloc(len);
    if(ble_evt_buffer==NULL)
    {
        return TUYA_BLE_ERR_NO_MEM;
    }
    else
    {
        memcpy(ble_evt_buffer,p_data,len);
    }

    evt.hdr.event = TUYA_BLE_EVT_DP_DATA_WITH_FLAG_AND_TIME_STRING_REPORTED;
    evt.flag_reported_with_time_string_data.sn = sn;
    evt.flag_reported_with_time_string_data.mode  = mode;
    memcpy(evt.flag_reported_with_time_string_data.time_string,time_string,13);
    evt.flag_reported_with_time_string_data.p_data = ble_evt_buffer;
    evt.flag_reported_with_time_string_data.data_len = len;

    if(tuya_ble_event_send(&evt)!=0)
    {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;
}

/**
 * @brief   Function for response the production test instruction asynchronous.
 *
 * @param   [in]channel: 0-uart ,1 - ble.
 *          [in]p_data  : pointer to production test cmd data(Complete instruction,include0x66 0xaa and checksum.)
 *          [in]len    : Number of bytes of pdata.
 * @note
 *.
 * */
tuya_ble_status_t tuya_ble_production_test_asynchronous_response(uint8_t channel,uint8_t *p_data,uint32_t len)
{
    tuya_ble_evt_param_t evt;
    uint8_t *ble_evt_buffer;

    if(len>TUYA_BLE_TRANSMISSION_MAX_DATA_LEN)
    {
        return TUYA_BLE_ERR_INVALID_LENGTH;
    }

    ble_evt_buffer=(uint8_t *)tuya_ble_malloc(len);
    if(ble_evt_buffer==NULL)
    {
        return TUYA_BLE_ERR_NO_MEM;
    }

    memcpy(ble_evt_buffer,(uint8_t *)p_data,len);
    evt.hdr.event = TUYA_BLE_EVT_PRODUCTION_TEST_RESPONSE;
    evt.prod_test_res_data.p_data = ble_evt_buffer;
    evt.prod_test_res_data.data_len = len;
    evt.prod_test_res_data.channel = channel;

    if(tuya_ble_event_send(&evt)!=0)
    {
        tuya_ble_free(ble_evt_buffer);
        return TUYA_BLE_ERR_NO_EVENT;
    }

    return TUYA_BLE_SUCCESS;

}

/*
 *@brief
 *@param
 *
 *@note
 *
 * */

tuya_ble_status_t tuya_ble_device_factory_reset(void)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_FACTORY_RESET;

    if(tuya_ble_event_send(&event)!=0)
    {
        TUYA_BLE_LOG_ERROR("tuya_event_send factory reset error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}


/*
 *@brief
 *@param time_type: 0-13-byte millisecond string ,1 - normal time format
 *
 *@note
 *
 * */

tuya_ble_status_t tuya_ble_time_req(uint8_t time_type)
{
    tuya_ble_evt_param_t event;

    if(time_type>1)
    {
        return TUYA_BLE_ERR_INVALID_PARAM;
    }

    event.hdr.event = TUYA_BLE_EVT_TIME_REQ;
    event.time_req_data.time_type = time_type;

    if(tuya_ble_event_send(&event)!=0)
    {
        TUYA_BLE_LOG_ERROR("tuya_event_send time req error");
        return TUYA_BLE_ERR_INTERNAL;
    }

    return TUYA_BLE_SUCCESS;
}



void tuya_ble_vtimer_connect_monitor_callback(void)
{
    tuya_ble_connect_status_t connect_state = tuya_ble_connect_status_get();

    if( (connect_state == UNBONDING_UNAUTH_CONN)||(connect_state == BONDING_UNAUTH_CONN) )
    {
        TUYA_BLE_LOG_DEBUG("ble disconncet because monitor timer timeout.");
        tuya_ble_gap_disconnect();
    }

}


extern tuya_ble_parameters_settings_t tuya_ble_current_para;

/*
 *@brief
 *@param
 *
 *@note
 *
 * */
void tuya_ble_connected_handler(void)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_CONNECT_STATUS_UPDATE;
    event.connect_change_evt = TUYA_BLE_CONNECTED;

    if(tuya_ble_event_send(&event)!=0)
    {
        TUYA_BLE_LOG_ERROR("tuya_event_send connect handler error");
    }
}

/*
 *@brief
 *@param
 *
 *@note
 *
 * */
void tuya_ble_disconnected_handler(void)
{
    tuya_ble_evt_param_t event;

    event.hdr.event = TUYA_BLE_EVT_CONNECT_STATUS_UPDATE;
    event.connect_change_evt = TUYA_BLE_DISCONNECTED;

    if(tuya_ble_event_send(&event)!=0)
    {
        TUYA_BLE_LOG_ERROR("tuya_event_send disconnect handler error");
    }

}

/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_sdk_init(tuya_ble_device_param_t * param_data)
{
    tuya_ble_cb_evt_param_t event;
    uint8_t device_id_temp[16];
    uint8_t device_id_temp2[20];

    tuya_ble_storage_init();

    if(param_data->product_id_len>0)       //
    {
        tuya_ble_current_para.pid_type = param_data->p_type;

        tuya_ble_current_para.pid_len = param_data->product_id_len;

        if(tuya_ble_current_para.pid_len>TUYA_BLE_PRODUCT_ID_MAX_LEN)
        {
            tuya_ble_current_para.pid_len = TUYA_BLE_PRODUCT_ID_MAX_LEN;
        }

        memcpy(tuya_ble_current_para.pid,param_data->product_id,tuya_ble_current_para.pid_len);
    }
    else
    {
        #if TUYA_BLE_AUC_WRITE_PID 
        
        tuya_ble_current_para.pid_type = tuya_ble_current_para.auth_settings.pid_type;
        tuya_ble_current_para.pid_len = tuya_ble_current_para.auth_settings.pid_len;
        memcpy(tuya_ble_current_para.pid,tuya_ble_current_para.auth_settings.factory_pid,tuya_ble_current_para.pid_len);
        
        #endif
    }

    if((param_data->device_id_len==16)||(param_data->device_id_len==20))
    {

        if(param_data->device_id_len == 20)
        {
            TUYA_BLE_LOG_HEXDUMP_DEBUG("device_id_20 ",param_data->device_id, 20);
            tuya_ble_device_id_20_to_16(param_data->device_id,device_id_temp);
            TUYA_BLE_LOG_HEXDUMP_DEBUG("device_id_16 ", device_id_temp, 16);
            tuya_ble_device_id_16_to_20(device_id_temp,device_id_temp2);
            TUYA_BLE_LOG_HEXDUMP_DEBUG("device_id_20 ", device_id_temp2, 20);
            memcpy(tuya_ble_current_para.auth_settings.device_id,device_id_temp,DEVICE_ID_LEN);
        }
        else
        {
            memcpy(tuya_ble_current_para.auth_settings.device_id,param_data->device_id,DEVICE_ID_LEN);
        }

        memcpy(tuya_ble_current_para.auth_settings.auth_key,param_data->auth_key,AUTH_KEY_LEN);
        
        memcpy(tuya_ble_current_para.auth_settings.mac,param_data->mac_addr.addr,6);

    }


    tuya_ble_set_device_version(param_data->firmware_version,param_data->hardware_version);


    if(tuya_ble_current_para.sys_settings.bound_flag==1)
    {
        tuya_ble_connect_status_set(BONDING_UNCONN);
    }
    else
    {
        tuya_ble_connect_status_set(UNBONDING_UNCONN);
    }

    tuya_ble_adv_change();

    tuya_ble_event_init();

    tuya_ble_gatt_send_queue_init();

    tuya_ble_common_uart_init();

    tuya_ble_connect_monitor_timer_init();

    TUYA_BLE_LOG_HEXDUMP_DEBUG("mac", tuya_ble_current_para.auth_settings.mac, MAC_LEN);
    TUYA_BLE_LOG_HEXDUMP_DEBUG("product_id", tuya_ble_current_para.pid, tuya_ble_current_para.pid_len);
    TUYA_BLE_LOG_HEXDUMP_DEBUG("device_id", tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN);
    TUYA_BLE_LOG_HEXDUMP_DEBUG("auth_key", tuya_ble_current_para.auth_settings.auth_key, AUTH_KEY_LEN);

    TUYA_BLE_LOG_DEBUG("bond_flag = %d", tuya_ble_current_para.sys_settings.bound_flag);
    
    TUYA_BLE_LOG_INFO("tuya ble sdk version : "TUYA_BLE_SDK_VERSION_STR);

    return TUYA_BLE_SUCCESS;
}








