/**
 ****************************************************************************************
 *
 * @file app_tuya.h
 *
 * @brief tuya Application 
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
#ifndef APP_TUYA_H_
#define APP_TUYA_H_
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

#include <stdint.h>          // Standard Integer Definition
#include "ke_task.h"         // Kernel Task Definition

/*
 * STRUCTURES DEFINITION
 ****************************************************************************************
 */

#define APP_PRODUCT_ID "przwh7nj" 

#define APP_BUILD_FIRMNAME  "KF_SW_BK3432_TEST"

//
#define TY_APP_VER_NUM       0x0101
#define TY_APP_VER_STR	     "1.1" 	


#define TY_HARD_VER_NUM       0x0100
#define TY_HARD_VER_STR	     "1.0" 	

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
void app_tuya_init(void);


/**
 ****************************************************************************************
 * @brief Send a  level value
 ****************************************************************************************
 */



#endif // APP_TUYA_SERVICE_H_
