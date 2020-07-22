/**
 ****************************************************************************************
 *
 * @file tuya_service.h
 *
 * @brief Header file - TUYA Service Server Role
 *
 * Copyright (C) beken 2009-2015
 *
 *
 ****************************************************************************************
 */
#ifndef _TUYA_SERVICE_H_
#define _TUYA_SERVICE_H_

/**
 ****************************************************************************************
 * @addtogroup  TUYA 'Profile' Server
 * @ingroup TUYA
 * @brief TUYA 'Profile' Server
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "rwprf_config.h"

#if (BLE_TUYA_SERVER)

#include "tuya_service_task.h"
#include "atts.h"
#include "prf_types.h"
#include "prf.h"

/*
 * DEFINES
 ****************************************************************************************
 */

#define TUYAS_CFG_FLAG_MANDATORY_MASK        (0x3F)
#define TUYAS_CFG_FLAG_NTF_SUP_MASK          (0x08)
#define TUYAS_CFG_FLAG_MTP_FFF1_MASK         (0x40)

#define CHAR_NOTIFY_LVL_MAX               			(100)

#define CHAR_NOTIFY_FLAG_NTF_CFG_BIT               (0x02)



enum
{		
		ATT_USER_SERVER_TUYA 						= ATT_UUID_16(0x1910),
	  
		ATT_USER_SERVER_CHAR_NOTIFY                 = ATT_UUID_16(0x2B10),
		
		ATT_USER_SERVER_CHAR_WRITE					= ATT_UUID_16(0x2B11),
		
};

/// Battery Service Attributes Indexes
enum
{
	TUYAS_IDX_SVC,

	TUYAS_IDX_CHAR_WRITE_LVL_CHAR,
	TUYAS_IDX_CHAR_WRITE_LVL_VAL,

	TUYAS_IDX_CHAR_NOTIFY_LVL_CHAR,
	TUYAS_IDX_CHAR_NOTIFY_LVL_VAL,
	TUYAS_IDX_CHAR_NOTIFY_LVL_NTF_CFG,

	TUYAS_IDX_NB,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


///  'Profile' Server environment variable
struct tuyas_env_tag
{
    /// profile environment
    prf_env_t prf_env;
   
    /// On-going operation
    struct ke_msg * operation;
    /// TUYA Services Start Handle
    uint16_t start_hdl;
    /// Level of the char notify
    uint8_t n_value[TUYA_CHAR_NOTIFY_DATA_LEN];
	
	uint8_t w_value[TUYA_CHAR_WRITE_DATA_LEN];
    ///  task state
    ke_state_t state[TUYAS_IDX_MAX];
    /// Notification configuration of peer devices.
    uint8_t ntf_cfg[BLE_CONNECTION_MAX];
    /// Database features
    uint8_t features;

};



/**
 ****************************************************************************************
 * @brief Retrieve tuya service profile interface
 *
 * @return tuya service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* tuyas_prf_itf_get(void);

uint16_t tuyas_get_att_handle(uint8_t att_idx);

uint8_t  tuyas_get_att_idx(uint16_t handle, uint8_t *att_idx);

void tuyas_notify_value_lvl(struct tuyas_env_tag* tuyas_env, struct tuyas_char_notify_level_upd_req const *param);

#endif /* #if (BLE_TUYA_SERVER) */



#endif /*  _TUYA_SERVICE_H_ */



