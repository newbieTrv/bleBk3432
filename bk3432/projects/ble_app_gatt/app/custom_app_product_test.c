#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "app_tuya_service.h"          
#include "app_tuya.h" 
#include "app.h"                    
#include "app_task.h"            
#include "tuya_service_task.h"          
#include "co_bt.h"
#include "prf_types.h"             
#include "arch.h"                   
#include "prf.h"
#include "tuya_service.h"
#include "ke_timer.h"
#include "uart.h"
#include "tuya_ble_log.h"
#include "tuya_ble_type.h"
#include "tuya_ble_api.h"
#include "tuya_ble_main.h"
#include "tuya_ble_port.h"
#include "tuya_ble_internal_config.h"
#include "custom_app_product_test.h"
#include "app_tuya_ota.h"
#include "uart2.h"

tuya_ble_status_t tuya_ble_prod_beacon_scan_start(void)
{
    //
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_prod_beacon_scan_stop(void)
{
    //
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_prod_beacon_get_rssi_avg(int8_t *rssi)
{
    //
    *rssi = -30;
    return TUYA_BLE_SUCCESS;
}

tuya_ble_status_t tuya_ble_prod_gpio_test(void)
{
    //Add gpio test code here
    return TUYA_BLE_SUCCESS;
}

void tuya_ble_custom_app_production_test_process(uint8_t channel,uint8_t *p_in_data,uint16_t in_len)
{
    uint16_t cmd = 0;
    uint8_t *data_buffer = NULL;
    uint16_t data_len = ((p_in_data[4]<<8) + p_in_data[5]);
       
    if((p_in_data[6] != 3)||(data_len<3))
        return;
    
    cmd = (p_in_data[7]<<8) + p_in_data[8];
    data_len -= 3;
    if(data_len>0)
    {
        data_buffer = p_in_data+9;
    }
    
    switch(cmd)
    {   

        //
        default:
            break;
    };    
    
    
}


