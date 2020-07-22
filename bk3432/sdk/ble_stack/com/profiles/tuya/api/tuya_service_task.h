/**
 ****************************************************************************************
 *
 * @file tuya_service_task.h
 *
 * @brief
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */


#ifndef _TUYA_SERVICE_TASK_H_
#define _TUYA_SERVICE_TASK_H_


#include "rwprf_config.h"
#if (BLE_TUYA_SERVER)
#include <stdint.h>
#include "rwip_task.h" // Task definitions
/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of TUYA Server task instances
#define TUYAS_IDX_MAX     0x01
///Maximal number of TUYA that can be added in the DB

#define  TUYA_CHAR_NOTIFY_DATA_LEN  20
#define  TUYA_CHAR_WRITE_DATA_LEN   20
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Possible states of the TUYAS task
enum tuyas_state
{
    /// Idle state
    TUYAS_IDLE,
    /// busy state
    TUYAS_BUSY,
    /// Number of defined states.
    TUYAS_STATE_MAX
};

/// Messages for TUYA Server
enum tuyas_msg_id
{
    /// Start the tuya Server - at connection used to restore bond data
	TUYAS_CREATE_DB_REQ   = TASK_FIRST_MSG(TASK_ID_TUYAS),
	
    ///  Level Value Update Request
    TUYAS_CHAR_NOTIFY_LEVEL_UPD_REQ,
    /// Inform APP if CHAR Level value has been notified or not
    TUYAS_CHAR_NOTIFY_LEVEL_UPD_RSP,
    /// Inform APP that CHAR Level Notification Configuration has been changed - use to update bond data
    TUYAS_CHAR_NOTIFY_LEVEL_NTF_CFG_IND,
	
	TUYAS_CHAR_WRITE_WRITER_REQ_IND,

	TUYAS_CHAR_NOTIFY_LEVEL_PERIOD_NTF
	
		
};

/// Features Flag Masks
enum tuyas_features
{
    /// char Level Characteristic doesn't support notifications
    TUYAS_CHAR_NOTIFY_LVL_NTF_NOT_SUP,
    /// char Level Characteristic support notifications
    TUYAS_CHAR_NOTIFY_LVL_NTF_SUP,
};
/*
 * APIs Structures
 ****************************************************************************************
 */

/// Parameters for the database creation
struct tuyas_db_cfg
{
    /// Number of tuya to add
    uint8_t tuya_nb;
    /// Features of each tuyas instance
    uint8_t features;
   };

/// Parameters of the @ref TUYAS_ENABLE_REQ message
struct tuyas_enable_req
{
    /// connection index
    uint8_t  conidx;
    /// Notification Configuration
    uint8_t  ntf_cfg;
    /// Old  Level used to decide if notification should be triggered
    uint8_t  old_fff1_lvl;
};


///Parameters of the @ref TUYAS_CHAR_NOTIFY_LEVEL_UPD_REQ message
struct tuyas_char_notify_level_upd_req
{
    ///  instance
    uint8_t conidx;
	
	uint8_t length;
    ///  Level
    uint8_t value[TUYA_CHAR_NOTIFY_DATA_LEN];
};

///Parameters of the @ref TUYAS_CHAR_NOTIFY_LEVEL_UPD_RSP message
struct tuyas_char_notify_level_upd_rsp
{
    ///status
    uint8_t status;
};

///Parameters of the @ref TUYAS_CHAR_NOTIFY_NTF_CFG_IND message
struct tuyas_char_notify_level_ntf_cfg_ind
{
    /// connection index
    uint8_t  conidx;
    ///Notification Configuration
    uint8_t  ntf_cfg;
};


/// Parameters of the @ref TUYAS_CHAR_WRITE_WRITER_REQ_IND message
struct tuyas_char_write_writer_ind
{
    /// Alert level
    uint8_t value[TUYA_CHAR_WRITE_DATA_LEN];
	
	uint8_t length;
    /// Connection index
    uint8_t conidx;
};


/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

extern const struct ke_state_handler tuyas_default_handler;
#endif // BLE_TUYA_SERVER


#endif /* _TUYA_SEVICE_TASK_H_ */

