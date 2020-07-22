/**
 ****************************************************************************************
 *
 * @file app_tuya_ota.c
 *
 * @brief tuya Application
 *
 * @auth  yonghui.gao
 *
 * @date  2020.02.11
 *
 * 
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

#include "rwip_config.h"     // SW configuration

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>
#include "app_tuya_service.h"              // tuya Application Module Definitions
#include "app_tuya.h" 
#include "app_tuya_ota.h" 
#include "app.h"                    // Application Definitions
#include "app_task.h"             // application task definitions
#include "tuya_service_task.h"           // health thermometer functions
#include "co_bt.h"
#include "prf_types.h"             // Profile common types definition
#include "arch.h"                    // Platform Definitions
#include "prf.h"
#include "tuya_service.h"
#include "ke_timer.h"
#include "uart.h"
#include "tuya_ble_log.h"
#include "tuya_ble_type.h"
#include "tuya_ble_api.h"
#include "tuya_ble_main.h"
#include "tuya_ble_data_handler.h"
#include "tuya_ble_secure.h"
#include "tuya_ble_utils.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */


/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

//#define  DFU_SETTINGS_ADDR 0x800

static volatile tuya_ota_status_t tuya_ota_status;

typedef struct
{
    uint32_t firmware_file_version;
    uint32_t firmware_file_length;
    uint32_t firmware_file_crc;
    uint8_t  firmware_file_md5[16];
} dfu_firmware_file_info_t;

typedef struct
{
    uint32_t firmware_image_offset_last;
    uint32_t firmware_image_crc_last;
} dfu_firmware_progress_t;


typedef struct
{
    uint32_t            crc;                /**< CRC for the stored DFU settings, not including the CRC itself. If 0xFFFFFFF, the CRC has never been calculated. */
    uint32_t            settings_version;   /**< Version of the current DFU settings struct layout. */
    dfu_firmware_file_info_t file_info;
    uint32_t            write_offset;
    dfu_firmware_progress_t progress;
    uint32_t res[8];
} dfu_settings_t;


static dfu_settings_t dfu_settings;


#define DFU_MAX_CHECK_BUFFER_SIZE    256
#define DFU_TEMP_BUFFER_SIZE         512

#define	APP_NEW_FW_MAX_SIZE          0xA400  //41K

static uint8_t TempBufferHead[DFU_TEMP_BUFFER_SIZE];
static uint16_t ota_tmp_buf_used_size = 0;

#define MAX_DFU_DATA_LEN  DFU_MAX_CHECK_BUFFER_SIZE

//static uint8_t file_crc_check_in_flash(uint32_t length,uint32_t *crc);

static uint16_t current_package = 0;
static uint16_t last_package = 0;


uint32_t tuya_ota_init(void)
{
    tuya_ota_status = TUYA_OTA_STATUS_NONE;
    //uint32_t crc_check;
    current_package = 0;
    last_package = 0;
    memset(TempBufferHead,0,sizeof(TempBufferHead));
    ota_tmp_buf_used_size = 0;
    //memset(&dfuPara,0,sizeof(dfuPara));

	memset(&dfu_settings,0,sizeof(dfu_settings_t));

    return 0;
}



static void tuya_ota_start_req(uint8_t*recv_data,uint32_t recv_len)
{
    uint8_t p_buf[70];
    uint8_t payload_len = 0;
    uint32_t current_version = TY_APP_VER_NUM;
    uint32_t ack_sn = 0;

    ack_sn  = (recv_data[1]<<24)|(recv_data[2]<<16)|(recv_data[3]<<8)|recv_data[4];

    if(tuya_ota_status!=TUYA_OTA_STATUS_NONE)
    {
        TUYA_BLE_LOG_ERROR("current ota status is not TUYA_OTA_STATUS_NONE  and is : %d !",tuya_ota_status);
        return;
    }

    p_buf[0] = 0;    //允许升级
    p_buf[1] = TUYA_OTA_VERSION;
    p_buf[2] = 0;
    p_buf[3] = current_version>>24;
    p_buf[4] = current_version>>16;
    p_buf[5] = current_version>>8;
    p_buf[6] = current_version;
    p_buf[7] = MAX_DFU_DATA_LEN>>8;
    p_buf[8] = (MAX_DFU_DATA_LEN&0xFF);
    tuya_ota_status = TUYA_OTA_STATUS_START;
    payload_len = 9;
	
	tuya_ble_commData_send(FRM_OTA_START_RESP,ack_sn,p_buf,payload_len,ENCRYPTION_MODE_SESSION_KEY);

}


static void tuya_ota_file_info_req(uint8_t*recv_data,uint32_t recv_len)
{
    uint8_t p_buf[30];
    uint8_t payload_len = 0;
    uint8_t encry_mode = 0;
    uint32_t file_version;
    uint32_t file_length;
    uint32_t file_crc;
    bool file_md5;
    // uint8_t file_md5[16];
    uint8_t state = 0;
    uint32_t crc_temp  = 0;
	uint32_t ack_sn = 0;

    ack_sn  = (recv_data[1]<<24)|(recv_data[2]<<16)|(recv_data[3]<<8)|recv_data[4];
	

    //file_crc_check_in_flash(256,&file_crc);
    if(tuya_ota_status!=TUYA_OTA_STATUS_START)
    {
        TUYA_BLE_LOG_ERROR("current ota status is not TUYA_OTA_STATUS_START  and is : %d !",tuya_ota_status);
        return;
    }

    if(recv_data[13]!=TUYA_OTA_TYPE)
    {
        TUYA_BLE_LOG_ERROR("current ota fm type is not 0!");
        return;
    }

    file_version = recv_data[22]<<24;
    file_version += recv_data[23]<<16;
    file_version += recv_data[24]<<8;
    file_version += recv_data[25];

    file_length = recv_data[42]<<24;
    file_length += recv_data[43]<<16;
    file_length += recv_data[44]<<8;
    file_length += recv_data[45];

    file_crc = recv_data[46]<<24;
    file_crc += recv_data[47]<<16;
    file_crc += recv_data[48]<<8;
    file_crc += recv_data[49];
/*  
    TUYA_BLE_LOG_DEBUG("file_version = 0x%04x",file_version);
    TUYA_BLE_LOG_DEBUG("file_length = %d",file_length);
    TUYA_BLE_LOG_DEBUG("ffile_crc = 0x%08x",file_crc);
    TUYA_BLE_LOG_DEBUG("dfu_settings.progress.firmware_image_offset_last = %d ",dfu_settings.progress.firmware_image_offset_last);
    TUYA_BLE_LOG_DEBUG("dfu_settings.progress.firmware_image_crc_last = 0x%08x ",dfu_settings.progress.firmware_image_crc_last);
*/
    if((file_version > TY_APP_VER_NUM)&&(file_length <= APP_NEW_FW_MAX_SIZE))
    {
            memset(&dfu_settings, 0, sizeof(dfu_settings_t));
            dfu_settings.progress.firmware_image_crc_last = 0;
            dfu_settings.file_info.firmware_file_version = file_version;
            dfu_settings.file_info.firmware_file_length = file_length;
            dfu_settings.file_info.firmware_file_crc = file_crc;
            memcpy(dfu_settings.file_info.firmware_file_md5,&recv_data[30+TUYA_BLE_PRODUCT_ID_DEFAULT_LEN],16);
            //dfu_settings.write_offset = dfu_settings.progress.firmware_image_offset_last;
            state = 0;        

    }
    else
    {
        if(file_version <= TY_APP_VER_NUM)
        {
            TUYA_BLE_LOG_ERROR("ota file version error !");
            state = 2;
        }
        else
        {
            TUYA_BLE_LOG_ERROR("ota file length is bigger than rev space !");
            state = 3;
        }
    }

    memset(p_buf,0,sizeof(p_buf));
	
    p_buf[0] = TUYA_OTA_TYPE;
	
    p_buf[1] = state;
	
    if(state==0)
    {
        p_buf[2] = dfu_settings.progress.firmware_image_offset_last>>24;
        p_buf[3] = dfu_settings.progress.firmware_image_offset_last>>16;
        p_buf[4] = dfu_settings.progress.firmware_image_offset_last>>8;
        p_buf[5] = (uint8_t)dfu_settings.progress.firmware_image_offset_last;
        p_buf[6] = dfu_settings.progress.firmware_image_crc_last>>24;
        p_buf[7] = dfu_settings.progress.firmware_image_crc_last>>16;
        p_buf[8] = dfu_settings.progress.firmware_image_crc_last>>8;
        p_buf[9] = (uint8_t)dfu_settings.progress.firmware_image_crc_last;
        tuya_ota_status = TUYA_OTA_STATUS_FILE_INFO;
        current_package = 0;
        last_package = 0;
       // memset(TempBufferHead,0,sizeof(TempBufferHead));
       // ota_tmp_buf_used_size = 0;

        TUYA_BLE_LOG_DEBUG("ota file length  : %d",dfu_settings.file_info.firmware_file_length);
        TUYA_BLE_LOG_DEBUG("ota file  crc    : 0x%04x",dfu_settings.file_info.firmware_file_crc);
        TUYA_BLE_LOG_DEBUG("ota file version : 0x%04x",dfu_settings.file_info.firmware_file_version);
        // NRF_LOG_DEBUG("ota file md5 : 0x%04x",s_dfu_settings.progress.firmware_file_length);
        TUYA_BLE_LOG_DEBUG("ota firmware_image_offset_last : 0x%04x",dfu_settings.progress.firmware_image_offset_last);
        TUYA_BLE_LOG_DEBUG("ota firmware_image_crc_last    : 0x%04x",dfu_settings.progress.firmware_image_crc_last);
        TUYA_BLE_LOG_DEBUG("ota firmware   write offset    : 0x%04x",dfu_settings.write_offset);

    }
    payload_len = 26;

    tuya_ble_commData_send(FRM_OTA_FILE_INFOR_RESP,ack_sn,p_buf,payload_len,ENCRYPTION_MODE_SESSION_KEY);

}


static void tuya_ota_offset_req(uint8_t*recv_data,uint32_t recv_len)
{
    uint8_t p_buf[5];
    uint8_t payload_len = 0;
    uint8_t encry_mode = 0;
    uint32_t offset = 0,offset_temp = 0;
    uint32_t ack_sn = 0;

    ack_sn  = (recv_data[1]<<24)|(recv_data[2]<<16)|(recv_data[3]<<8)|recv_data[4];

    if(tuya_ota_status!=TUYA_OTA_STATUS_FILE_INFO)
    {
        TUYA_BLE_LOG_ERROR("current ota status is not TUYA_OTA_STATUS_FILE_INFO  and is : %d !",tuya_ota_status);
        return;
    }
	
	if(recv_data[13]!=TUYA_OTA_TYPE)
    {
        TUYA_BLE_LOG_ERROR("current ota fm type is not 0!");
        return;
    }

    offset  = recv_data[14]<<24;
    offset += recv_data[15]<<16;
    offset += recv_data[16]<<8;
    offset += recv_data[17];

    if((offset==0)&&(dfu_settings.progress.firmware_image_offset_last!=0))
    {
        dfu_settings.progress.firmware_image_crc_last = 0;
        dfu_settings.progress.firmware_image_offset_last = 0;
        dfu_settings.write_offset = dfu_settings.progress.firmware_image_offset_last;
 
    }

    p_buf[0] = TUYA_OTA_TYPE;
    p_buf[1] = dfu_settings.progress.firmware_image_offset_last>>24;
    p_buf[2] = dfu_settings.progress.firmware_image_offset_last>>16;
    p_buf[3] = dfu_settings.progress.firmware_image_offset_last>>8;
    p_buf[4] = (uint8_t)dfu_settings.progress.firmware_image_offset_last;

    tuya_ota_status = TUYA_OTA_STATUS_FILE_OFFSET;

    payload_len = 5;

    tuya_ble_commData_send(FRM_OTA_FILE_OFFSET_RESP,ack_sn,p_buf,payload_len,ENCRYPTION_MODE_SESSION_KEY);

}



static void tuya_ota_data_req(uint8_t*recv_data,uint32_t recv_len)
{
    uint8_t p_buf[2];
    uint8_t payload_len = 0;
    uint8_t state = 0;
    uint16_t data_length;
    uint8_t *p_value;
    uint32_t write_addr = 0;
    uint32_t ack_sn = 0;

    ack_sn  = (recv_data[1]<<24)|(recv_data[2]<<16)|(recv_data[3]<<8)|recv_data[4];

    if((tuya_ota_status!=TUYA_OTA_STATUS_FILE_OFFSET)&&(tuya_ota_status!=TUYA_OTA_STATUS_FILE_DATA))
    {
        TUYA_BLE_LOG_ERROR("current ota status is not TUYA_OTA_STATUS_FILE_OFFSET  or TUYA_OTA_STATUS_FILE_DATA and is : %d !",tuya_ota_status);
        return;
    }
	
	if(recv_data[13]!=TUYA_OTA_TYPE)
    {
        TUYA_BLE_LOG_ERROR("current ota fm type is not 0!");
        return;
    }

    state = 0;

    current_package = (recv_data[14]<<8)|recv_data[15];
    data_length = (recv_data[16]<<8)|recv_data[17];

    if((current_package!=(last_package+1))&&(current_package!=0))
    {
        TUYA_BLE_LOG_ERROR("ota received package number error.received package number : %d",current_package);
        state = 1;
        goto ota_data_response;
    }
    else  if(data_length>MAX_DFU_DATA_LEN)
    {
        TUYA_BLE_LOG_ERROR("ota received package data length error : %d",data_length);
        state = 5;
        goto ota_data_response;
    }
    else
    {

        if (dfu_settings.write_offset + ota_tmp_buf_used_size + data_length > dfu_settings.file_info.firmware_file_length)
        {
            TUYA_BLE_LOG_ERROR("dfu_service_handle_packet_req: p_dfu->cur_offset=%d, ota_temp_buf_used_size =%d, length= %d, image_total_length = %d ",
                               dfu_settings.write_offset,
                               ota_tmp_buf_used_size,
                               data_length,
                               dfu_settings.file_info.firmware_file_length
                              );
            state = 2;
            TUYA_BLE_LOG_ERROR("DFU_LENGTH_ERROR");
            goto ota_data_response;
        }
        else
        {
            memcpy(TempBufferHead+ota_tmp_buf_used_size,recv_data+20,data_length);
            ota_tmp_buf_used_size += data_length;

            if (ota_tmp_buf_used_size == DFU_TEMP_BUFFER_SIZE || dfu_settings.write_offset + ota_tmp_buf_used_size == dfu_settings.file_info.firmware_file_length)
            {
				write_addr = dfu_settings.write_offset + SEC_BACKUP_OAD_HEADER_FADDR;
				tuya_ble_nv_erase(write_addr,TUYA_NV_ERASE_MIN_SIZE);
				tuya_ble_nv_write(write_addr,(uint8_t *)TempBufferHead,ota_tmp_buf_used_size);
				
				dfu_settings.write_offset += ota_tmp_buf_used_size;
                dfu_settings.progress.firmware_image_crc_last = tuya_ble_crc32_compute(TempBufferHead, ota_tmp_buf_used_size, &dfu_settings.progress.firmware_image_crc_last);
                dfu_settings.progress.firmware_image_offset_last += ota_tmp_buf_used_size;
				
				ota_tmp_buf_used_size = 0;
				
            }

        }

    }

ota_data_response:

    p_buf[0] = TUYA_OTA_TYPE;
    p_buf[1] = state;

    tuya_ota_status = TUYA_OTA_STATUS_FILE_DATA;

    payload_len = 2;

    tuya_ble_commData_send(FRM_OTA_DATA_RESP,ack_sn,p_buf,payload_len,ENCRYPTION_MODE_SESSION_KEY);

    if(state!=0)//出错，恢复初始状态
    {
        TUYA_BLE_LOG_ERROR("ota error so free!");
        tuya_ota_status = TUYA_OTA_STATUS_NONE;
        tuya_ota_init_disconnect();
        //memset(&dfu_settings, 0, sizeof(dfu_settings_t));
        //dfu_settings_write();
    }
    else
    {
        last_package = current_package;
    }


}

static uint8_t file_crc_check_in_flash(uint32_t len,uint32_t *crc)
{
    uint8_t *buf = TempBufferHead;
    if(len == 0)
    {
        return 1;
    }
	memset(buf,0,256);
    uint32_t crc_temp = 0;
    uint32_t read_addr = SEC_BACKUP_OAD_HEADER_FADDR;
    uint32_t cnt = len/256;
    uint32_t remainder = len % 256;
    for(uint32_t i = 0; i<cnt; i++)
    {
        //memcpy(buf,(uint32_t *)read_addr,256);
		tuya_ble_nv_read(read_addr,buf,256);
        crc_temp = tuya_ble_crc32_compute(buf, 256, &crc_temp);
        read_addr += 256;
    }

    if(remainder>0)
    {
        tuya_ble_nv_read(read_addr,buf,remainder);
        crc_temp = tuya_ble_crc32_compute(buf, remainder, &crc_temp);
        read_addr += remainder;
    }

    *crc = crc_temp;

    return 0;
}



static void tuya_ota_end_req(uint8_t*recv_data,uint32_t recv_len)
{
    uint8_t p_buf[2];
    uint8_t payload_len = 0;
	uint32_t ack_sn = 0;
    bool check_result;
	uint8_t state;
    ack_sn  = (recv_data[1]<<24)|(recv_data[2]<<16)|(recv_data[3]<<8)|recv_data[4];

    if(tuya_ota_status==TUYA_OTA_STATUS_NONE)
    {
        TUYA_BLE_LOG_ERROR("current ota status is TUYA_OTA_STATUS_NONE!");
        return;
    }
	if(recv_data[13]!=TUYA_OTA_TYPE)
    {
        TUYA_BLE_LOG_ERROR("current ota fm type is not 0!");
        return;
    }

    if (dfu_settings.progress.firmware_image_offset_last == dfu_settings.file_info.firmware_file_length)
    {
        TUYA_BLE_LOG_DEBUG("Whole firmware image received. Postvalidating.");
        uint32_t crc_temp  = 0;
        if(file_crc_check_in_flash(dfu_settings.progress.firmware_image_offset_last,&crc_temp)==0)
        {
            if(dfu_settings.progress.firmware_image_crc_last != crc_temp)
            {
                TUYA_BLE_LOG_WARNING("file crc check in flash diff from crc_last. crc_temp = 0x%04x,crc_last = 0x%04x",crc_temp,dfu_settings.progress.firmware_image_crc_last);
                dfu_settings.progress.firmware_image_crc_last = crc_temp;
            }

        }

        if(dfu_settings.progress.firmware_image_crc_last!=dfu_settings.file_info.firmware_file_crc)
        {
            TUYA_BLE_LOG_ERROR("ota file crc check error,last_crc = 0x%04x ,file_crc = 0x%04x",dfu_settings.progress.firmware_image_crc_last,dfu_settings.file_info.firmware_file_crc);
            state = 2;
        }
        else
        {
            memset(&dfu_settings, 0, sizeof(dfu_settings_t));
            state = 0;
        }

    }
    else
    {
        state = 1;
    }

    p_buf[0] = TUYA_OTA_TYPE;
    p_buf[1] = state;
    tuya_ota_status = TUYA_OTA_STATUS_NONE;
    payload_len = 2;

	tuya_ble_commData_send(FRM_OTA_END_RESP,ack_sn,p_buf,payload_len,ENCRYPTION_MODE_SESSION_KEY);

    if(state==0)
    {
        TUYA_BLE_LOG_INFO("start reset~~~.");
        
		tuya_ble_device_reset_time_delay_ms(1500);
        //tuya_ble_device_delay_ms(1000);
        //tuya_ble_gap_disconnect();
        //tuya_ble_device_reset();
    }
    else
    {
        TUYA_BLE_LOG_ERROR("ota crc error!");
        tuya_ota_status = TUYA_OTA_STATUS_NONE;
        tuya_ota_init_disconnect();
    }    

}


void tuya_ble_handle_ota_req(uint16_t cmd,uint8_t*recv_data,uint32_t recv_len)
{
    TUYA_BLE_LOG_DEBUG("ota cmd : 0x%04x , recv_len : %d",cmd,recv_len);
    switch(cmd)
    {
    case FRM_OTA_START_REQ:
        tuya_ota_start_req(recv_data,recv_len);
        break;
    case FRM_OTA_FILE_INFOR_REQ:
        tuya_ota_file_info_req(recv_data,recv_len);
        break;
    case FRM_OTA_FILE_OFFSET_REQ:
        tuya_ota_offset_req(recv_data,recv_len);
        break;
    case FRM_OTA_DATA_REQ:
        tuya_ota_data_req(recv_data,recv_len);
        break;
    case FRM_OTA_END_REQ:
        tuya_ota_end_req(recv_data,recv_len);
        break;
    default:
        break;
    }

}

void tuya_ota_status_set(tuya_ota_status_t status)
{
    tuya_ota_status = status;
}


tuya_ota_status_t tuya_ota_status_get(void)
{
    return tuya_ota_status;
}

uint8_t tuya_ota_init_disconnect(void)
{
    if(tuya_ota_status != TUYA_OTA_STATUS_NONE)
    {
        //dfu_settings_write();
        tuya_ota_status = TUYA_OTA_STATUS_NONE;
        current_package = 0;
        last_package = 0;
    }

}


