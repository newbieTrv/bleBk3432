/**
 ****************************************************************************************
 *
 * @file app_tuya_service.c
 *
 * @brief tuya Application Module entry point
 *
 * @auth  yonghui.gao
 *
 * @date  2020.01.09
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



/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// fff0 Application Module Environment Structure
struct app_tuya_env_tag app_tuya_env;

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

#include "tuya_ble_secure.h"
static uint8_t test_buf[32];
static uint8_t key[16];
static uint8_t iv[16];
static uint32_t out_len;
tuya_ble_parameters_settings_t para;
void app_tuya_service_init(void)
{
	// Reset the environment
	memset(&app_tuya_env, 0, sizeof(struct app_tuya_env_tag));
    //tuya_ble_encryption(1,iv,test_buf,32,&out_len,test_buf,&para,key);
}

void app_tuya_add_service(void)
{

	struct tuyas_db_cfg *db_cfg;

	struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
	                                        TASK_GAPM, TASK_APP,
	                                        gapm_profile_task_add_cmd, sizeof(struct tuyas_db_cfg));
	// Fill message
	req->operation = GAPM_PROFILE_TASK_ADD;
	req->sec_lvl =   0;
	req->prf_task_id = TASK_ID_TUYAS;
	req->app_task = TASK_APP;
	req->start_hdl = 0; //req->start_hdl = 0; dynamically allocated


	// Set parameters
	db_cfg = (struct tuyas_db_cfg* ) req->param;

	// Sending of notifications is supported
	db_cfg->features = TUYAS_CHAR_NOTIFY_LVL_NTF_SUP;
	// Send the message
	ke_msg_send(req);
}


void app_tuya_send_data(uint8_t* buf, uint8_t len)
{
	// Allocate the message
	struct tuyas_char_notify_level_upd_req * req = KE_MSG_ALLOC(TUYAS_CHAR_NOTIFY_LEVEL_UPD_REQ,
	                                        prf_get_task_from_id(TASK_ID_TUYAS),
	                                        TASK_APP,
	                                        tuyas_char_notify_level_upd_req);
	// Fill in the parameter structure
	req->length = len;
	memcpy(req->value, buf, len);

	// Send the message
	ke_msg_send(req);
}


static int tuyas_char_notify_level_ntf_cfg_ind_handler(ke_msg_id_t const msgid,
        struct tuyas_char_notify_level_ntf_cfg_ind const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
	UART_PRINTF("param->ntf_cfg = %x\r\n",param->ntf_cfg);
//	if(param->ntf_cfg == PRF_CLI_STOP_NTFIND)
//	{
//		ke_timer_clear(TUYAS_CHAR_NOTIFY_LEVEL_PERIOD_NTF,dest_id);
//	}
//	else
//	{
//		ke_timer_set(TUYAS_CHAR_NOTIFY_LEVEL_PERIOD_NTF,dest_id ,1);
//	}

	return (KE_MSG_CONSUMED);
}

static int tuyas_char_notify_level_upd_handler(ke_msg_id_t const msgid,
                                  struct tuyas_char_notify_level_upd_rsp const *param,
                                  ke_task_id_t const dest_id,
                                  ke_task_id_t const src_id)
{
	if(param->status == GAP_ERR_NO_ERROR)
	{
//		uint8_t buf[20];
//		memset(buf, 0xcc, 20);
//		app_tuya_send_data(buf, 20);
	}

	return (KE_MSG_CONSUMED);
}



/**
 ****************************************************************************************
 * @brief
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GAP).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int app_tuya_msg_dflt_handler(ke_msg_id_t const msgid,
                                     void const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
	UART_PRINTF("%s\r\n", __func__);

	// Drop the message
	return (KE_MSG_CONSUMED);
}


static int tuyas_char_write_writer_req_handler(ke_msg_id_t const msgid,
                                   struct tuyas_char_write_writer_ind *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
	// Drop the message
	UART_PRINTF("write param->value = 0x ");

	for(uint8_t i = 0; i < param->length; i++)
	{
		UART_PRINTF("%02x ",param->value[i]);
	}
	UART_PRINTF("\r\n");

	return (KE_MSG_CONSUMED);
}


static int tuyas_char_notify_period_ntf_handler(ke_msg_id_t const msgid,
                                   struct tuyas_char_notify_level_ntf_cfg_ind const *param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
	uint8_t buf[20];
	memset(buf, 0xff, 20);
	app_tuya_send_data(buf, 20);
	//ke_timer_set(FFF0S_FFF1_LEVEL_PERIOD_NTF,dest_id , 100);

	return (KE_MSG_CONSUMED);
}



/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
const struct ke_msg_handler app_tuya_msg_handler_list[] =
{
	// Note: first message is latest message checked by kernel so default is put on top.
	{KE_MSG_DEFAULT_HANDLER,        (ke_msg_func_t)app_tuya_msg_dflt_handler},
	{TUYAS_CHAR_NOTIFY_LEVEL_NTF_CFG_IND,  (ke_msg_func_t)tuyas_char_notify_level_ntf_cfg_ind_handler},
	{TUYAS_CHAR_NOTIFY_LEVEL_UPD_RSP,      (ke_msg_func_t)tuyas_char_notify_level_upd_handler},
	{TUYAS_CHAR_WRITE_WRITER_REQ_IND,		(ke_msg_func_t)tuyas_char_write_writer_req_handler},
	{TUYAS_CHAR_NOTIFY_LEVEL_PERIOD_NTF,	(ke_msg_func_t)tuyas_char_notify_period_ntf_handler},
};

const struct ke_state_handler app_tuya_table_handler =
{&app_tuya_msg_handler_list[0], (sizeof(app_tuya_msg_handler_list)/sizeof(struct ke_msg_handler))};


