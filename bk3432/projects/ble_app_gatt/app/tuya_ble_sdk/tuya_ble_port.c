#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tuya_ble_port.h"
#include "tuya_ble_type.h"
#include "ke_mem.h"
#include "ll.h"
#include "arch.h"
#include "rf.h"
#include "flash.h"
#include "app.h"
#include "app_tuya_service.h"
#include "ke_msg.h"
#include "app_task.h"
#include "ke_timer.h"
#include "app_task.h"
#include "tuya_ble_config.h"
#include "uart2.h"

void tuya_hexdump(uint8_t *p_data,uint8_t len)
{
    for(uint8_t i = 0; i < len; i++)
	{
		UART_PRINTF("%02x ",p_data[i]);
        if((i+1)%8==0)
        {
            UART_PRINTF("\r\n");
        }
	}
}


tuya_ble_status_t tuya_ble_gap_advertising_adv_data_update(uint8_t * p_adv_data, uint8_t adv_len,uint8_t * p_scan_data, uint8_t scan_len)
{
    appm_update_adv_data(p_adv_data,adv_len,p_scan_data,scan_len);
    return TUYA_BLE_SUCCESS;
}


tuya_ble_status_t tuya_ble_gap_disconnect(void)
{
     appm_disconnect();
     return TUYA_BLE_SUCCESS;
}



tuya_ble_status_t tuya_ble_gatt_send_data(uint8_t *p_data,uint16_t len)
{
    app_tuya_send_data(p_data,len);
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_common_uart_init(void)
{    
    //uart_init();
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_common_uart_send_data(const uint8_t *p_data,uint16_t len)
{
    uart2_send((uint8_t *)p_data,len);
    return TUYA_BLE_SUCCESS;
}



void tuya_ble_device_delay_ms(uint32_t ms)
{
    Delay_ms(ms);
}


tuya_ble_status_t tuya_ble_rand_generator(uint8_t* p_buf, uint8_t len)
{
    uint32_t cnt=len/4;
    uint8_t  remain = len%4;
    int32_t temp;
    for(uint32_t i=0; i<cnt; i++)
    {
        temp = rand();
        memcpy(p_buf,(uint8_t *)&temp,4);
        p_buf += 4;
    }
    temp = rand();
    memcpy(p_buf,(uint8_t *)&temp,remain);

    return TUYA_BLE_SUCCESS;
}

/*
 *@brief
 *@param
 *
 *@note
 *
 * */
tuya_ble_status_t tuya_ble_device_reset(void)
{
    platform_reset(RESET_NO_ERROR);
    return TUYA_BLE_SUCCESS;
}


void tuya_ble_device_enter_critical(void)
{
    GLOBAL_INT_STOP();
}

void tuya_ble_device_exit_critical(void)
{
    GLOBAL_INT_START();
}


tuya_ble_status_t tuya_ble_rtc_get_timestamp(uint32_t *timestamp,int32_t *timezone)
{
    *timestamp = 0;
    *timezone = 0;
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_rtc_set_timestamp(uint32_t timestamp,int32_t timezone)
{

    return TUYA_BLE_SUCCESS;
}


tuya_ble_status_t tuya_ble_nv_init(void)
{    
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_nv_erase(uint32_t addr,uint32_t size)
{    
    tuya_ble_status_t result = TUYA_BLE_SUCCESS;
   
   // flash_erase(FLASH_SPACE_TYPE_MAIN,addr/4,size);
	
    uint32_t sec_cnt = size/TUYA_NV_ERASE_MIN_SIZE;

	uint32_t address = addr & ~(TUYA_NV_ERASE_MIN_SIZE - 1);

	for(uint8_t i=0;i<sec_cnt;i++)
	{

		flash_erase_sector(FLASH_SPACE_TYPE_MAIN,address/4);

		address += TUYA_NV_ERASE_MIN_SIZE;

	}
	
       
    return result;
}

tuya_ble_status_t tuya_ble_nv_write(uint32_t addr,const uint8_t *p_data, uint32_t size)
{  
    flash_write(FLASH_SPACE_TYPE_MAIN, addr/4, size, (uint8_t *)p_data);
    return TUYA_BLE_SUCCESS;
}


tuya_ble_status_t tuya_ble_nv_read(uint32_t addr,uint8_t *p_data, uint32_t size)
{
    flash_read(FLASH_SPACE_TYPE_MAIN,addr/4,size,p_data);
    return TUYA_BLE_SUCCESS;

}

void *tuya_ble_malloc(uint16_t size)
{
    uint8_t *ptr = NULL;
    ptr = ke_malloc(size,KE_MEM_KE_MSG);
    if(ptr)
    {
        memset(ptr,0x0,size);//allocate buffer need init
    }
    return ptr;
}

tuya_ble_status_t tuya_ble_free(void *ptr)
{
    if(ptr==NULL) 
        return TUYA_BLE_SUCCESS;

    ke_free(ptr);
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_message_send(tuya_ble_evt_param_t *evt)
{
    // Allocate the message
    struct app_tuya_ble_evt_param_t * req = KE_MSG_ALLOC(APP_TUYA_BLE_EVT,
	                                        TASK_APP,
	                                        TASK_APP,
	                                        app_tuya_ble_evt_param_t);
	// Fill in the parameter structure
	memcpy(&req->evt, evt, sizeof(tuya_ble_evt_param_t));

	// Send the message
	ke_msg_send(req);
    
    
    return TUYA_BLE_SUCCESS;
}



void tuya_ble_connect_monitor_timer_init(void)
{

}


void tuya_ble_connect_monitor_timer_start(void)
{
    ke_timer_set(APP_TUYA_CONNECT_MONITOR_TIMER,TASK_APP ,tuya_ble_connect_monitor_timeout_ms);
}

void tuya_ble_connect_monitor_timer_stop(void)
{
    ke_timer_clear(APP_TUYA_CONNECT_MONITOR_TIMER,TASK_APP);
}

void tuya_ble_device_reset_time_delay_ms(uint32_t ms)
{
	ke_timer_set(APP_TUYA_PROD_MONITOR_TIMER,TASK_APP ,ms/10);
}
