/**
 * \file tuya_ble_port.h
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


#ifndef TUYA_BLE_PORT_H__
#define TUYA_BLE_PORT_H__


#include "tuya_ble_stdlib.h"
#include "tuya_ble_type.h"
#include "uart.h"

   
#define TUYA_BLE_PRINTF(...)            UART_PRINTF(__VA_ARGS__)
#define TUYA_BLE_HEXDUMP(...)           tuya_hexdump(__VA_ARGS__)

void tuya_hexdump(uint8_t *p_data,uint8_t len);
//tuya_ble_status_t tuya_ble_gap_adv_start(void);

tuya_ble_status_t tuya_ble_gap_advertising_adv_data_update(uint8_t * p_adv_data, uint8_t adv_len,uint8_t * p_scan_data, uint8_t scan_len);

//tuya_ble_status_t tuya_ble_gap_advertising_scan_rsp_data_update(uint8_t const *p_sr_data, uint8_t sr_len);

//tuya_ble_status_t tuya_ble_gap_adv_stop(void);

//tuya_ble_status_t tuya_ble_gap_update_conn_params(tuya_ble_gap_conn_param_t conn_params);

tuya_ble_status_t tuya_ble_gap_disconnect(void);

//tuya_ble_status_t tuya_ble_gap_address_get(tuya_ble_addr_t mac);

tuya_ble_status_t tuya_ble_gatt_send_data(uint8_t *p_data,uint16_t len);


/**
 * @brief Function for delaying execution for a number of milliseconds.
 *
 * @param ms_time Number of milliseconds to wait.
 */
void tuya_ble_device_delay_ms(uint32_t ms);


/**
 * @brief Function for delaying execution for a number of microseconds.
 *
 * @param us_time Number of microseconds to wait.
 */
void tuya_ble_device_delay_us(uint32_t us);



/**
 * @brief   Function for RESET device.
 *
 * @note    
 *.
 * */
tuya_ble_status_t tuya_ble_device_reset(void);

/**
 * @brief   Function for get mac addr.
 *
 * @note    
 *.
 * */
//tuya_ble_status_t tuya_ble_gap_addr_get(tuya_ble_gap_addr_t *p_addr);

/**
 * @brief   Function for update mac addr.
 *
 * @note    
 *.
 * */
//tuya_ble_status_t tuya_ble_gap_addr_set(tuya_ble_gap_addr_t *p_addr);


/**@brief Function for entering a critical region.
 *
 * @note Due to implementation details, there must exist one and only one call to
 *       tuya_ble_device_exit_critical() for each call to tuya_ble_device_enter_critical(), and they must be located
 *       in the same scope.
 */
void tuya_ble_device_enter_critical(void);

/**@brief Macro for leaving a critical region.
 *
 * @note Due to implementation details, there must exist one and only one call to
 *       tuya_ble_device_exit_critical() for each call to tuya_ble_device_enter_critical(), and they must be located
 *       in the same scope.
 */

void tuya_ble_device_exit_critical(void);

/**
 * @brief   Get ture random bytes .
 * @param   [out] p_buf: pointer to data
 *          [in] len: Number of bytes to take from pool and place in
 * p_buff
 * @return  TUYA_BLE_SUCCESS          The requested bytes were written to
 * p_buff
 *          TUYA_BLE_ERR_NO_MEM       No bytes were written to the buffer, because
 * there were not enough random bytes available.
 * @note    SHOULD use TRUE random num generator
 * */
tuya_ble_status_t tuya_ble_rand_generator(uint8_t* p_buf, uint8_t len);


/**
 * @brief   Function for get the unix timestamp.
 *
 * @note    timezone: 100 times the actual time zone
 *.
 * */

tuya_ble_status_t tuya_ble_rtc_get_timestamp(uint32_t *timestamp,int32_t *timezone);

/**
 * @brief   Function for set the unix timestamp.
 *
 * @note    timezone: 100 times the actual time zone,Eastern eight zones:8x100
 *. 
 * */

tuya_ble_status_t tuya_ble_rtc_set_timestamp(uint32_t timestamp,int32_t timezone);


/**
 * @brief Initialize the NV module.
 * @note  
 * @note  
 *
 * @param 
 * @param 
 *
 * @return result
 */
tuya_ble_status_t tuya_ble_nv_init(void);

/**
 * @brief Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
tuya_ble_status_t tuya_ble_nv_erase(uint32_t addr,uint32_t size);

/**
 * @brief Write data to flash.
 * 
 * @note This operation must after erase. @see tuya_ble_nv_erase.
 *
 * @param addr flash address
 * @param p_data the write data buffer
 * @param size write bytes size
 *
 * @return result
 */
tuya_ble_status_t tuya_ble_nv_write(uint32_t addr,const uint8_t * p_data, uint32_t size);



/**
 * @brief Read data from flash.
 * @note 
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
tuya_ble_status_t tuya_ble_nv_read(uint32_t addr,uint8_t * p_data, uint32_t size);


/**
 * @brief Initialize uart peripheral.
 * @note   UART_PARITY_NO_PARTY,UART_STOP_BITS_1,UART_WROD_LENGTH_8BIT;
 *         9600 baud rate.
 * @param No parameter.
 *
 * @return tuya_ble_status_t
 */
tuya_ble_status_t tuya_ble_common_uart_init(void);


/**
 * @brief Send data to uart.
 * @note 
 *
 * @param p_data the send data buffer
 * @param len to send bytes size
 *
 * @return result
 */
tuya_ble_status_t tuya_ble_common_uart_send_data(const uint8_t *p_data,uint16_t len);


void *tuya_ble_malloc(uint16_t size);


tuya_ble_status_t tuya_ble_free(void *ptr);

tuya_ble_status_t tuya_ble_message_send(tuya_ble_evt_param_t *evt);

void tuya_ble_connect_monitor_timer_init(void);

void tuya_ble_connect_monitor_timer_start(void);

void tuya_ble_connect_monitor_timer_stop(void);

void tuya_ble_device_reset_time_delay_ms(uint32_t ms);

#endif





