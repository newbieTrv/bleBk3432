/**
 ****************************************************************************************
 *
 * @file app_tuya_service.c
 *
 * @brief findt Application Module entry point
 *
 * @auth  yonghui.gao
 *
 * @date  2019.01.09
 *
 * 
 *
 *
 ****************************************************************************************
 */
#ifndef APP_TUYA_SERVICE_H_
#define APP_TUYA_SERVICE_H_
/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief TUYA Application Module entry point
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#include "tuya_ble_type.h"
#include <stdint.h>          // Standard Integer Definition
#include "ke_task.h"         // Kernel Task Definition

/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */

/// tuyas Application Module Environment Structure
struct app_tuya_env_tag
{
    /// Connection handle
    uint8_t conidx;
    /// Current value Level
    uint8_t lvl;
};

struct app_tuya_ble_evt_param_t
{
    tuya_ble_evt_param_t evt;
};


/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

/// tuyas Application environment
extern struct app_tuya_env_tag app_tuya_env;

/// Table of message handlers
extern const struct ke_state_handler app_tuya_table_handler;
/*
 * FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 *
 * tuyas Application Functions
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize tuyas Application Module
 ****************************************************************************************
 */
void app_tuya_service_init(void);
/**
 ****************************************************************************************
 * @brief Add a tuya Service instance in the DB
 ****************************************************************************************
 */
void app_tuya_add_service(void);
/**
 ****************************************************************************************
 * @brief Enable the tuya Service
 ****************************************************************************************
 */
//void app_tuya_enable_prf(uint8_t conidx);
/**
 ****************************************************************************************
 * @brief Send a  level value
 ****************************************************************************************
 */
void app_tuya_send_data(uint8_t* buf, uint8_t len);


#endif // APP_TUYA_SERVICE_H_
