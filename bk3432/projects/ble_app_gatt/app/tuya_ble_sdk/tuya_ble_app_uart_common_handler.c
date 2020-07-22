/**
 * \file tuya_ble_app_uart_common_handler.c
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
#include "tuya_ble_internal_config.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_main.h"
#include "tuya_ble_storage.h"
#include "tuya_ble_app_uart_common_handler.h"
#include "tuya_ble_log.h"
#include "custom_app_uart_common_handler.h"


#define TUYA_BLE_UART_COMMON_MCU_OTA_DATA_LENGTH_MAX  200


#define TUYA_BLE_UART_COMMON_MCU_OTA_REQUEST			    0xEA
#define TUYA_BLE_UART_COMMON_MCU_OTA_FILE_INFO			    0xEB
#define TUYA_BLE_UART_COMMON_MCU_OTA_FILE_OFFSET	        0xEC
#define TUYA_BLE_UART_COMMON_MCU_OTA_DATA 			        0xED
#define TUYA_BLE_UART_COMMON_MCU_OTA_END			        0xEE


__TUYA_BLE_WEAK void tuya_ble_custom_app_uart_common_process(uint8_t *p_in_data,uint16_t in_len)
{
    uint8_t cmd = p_in_data[3];
    uint16_t data_len = (p_in_data[4]<<8) + p_in_data[5];
    uint8_t *data_buffer = p_in_data+6;

    switch(cmd)
    {

    default:
        break;
    };

}



void tuya_ble_uart_common_process(uint8_t *p_in_data,uint16_t in_len)
{
    uint8_t cmd = p_in_data[3];
    uint16_t data_len = (p_in_data[4]<<8) + p_in_data[5];
    uint8_t *data_buffer = p_in_data+6;

    switch(cmd)
    {
    case TUYA_BLE_UART_COMMON_MCU_OTA_REQUEST:
    case TUYA_BLE_UART_COMMON_MCU_OTA_FILE_INFO:
    case TUYA_BLE_UART_COMMON_MCU_OTA_FILE_OFFSET:
    case TUYA_BLE_UART_COMMON_MCU_OTA_DATA:
    case TUYA_BLE_UART_COMMON_MCU_OTA_END:
        TUYA_BLE_LOG_WARNING("unsupport mcu ota!");
        break;
    default:
        tuya_ble_custom_app_uart_common_process(p_in_data,in_len);
        break;
    };

}


