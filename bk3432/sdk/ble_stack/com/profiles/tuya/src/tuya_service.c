/**
 ****************************************************************************************
 *
 * @file tuya_service.c
 *
 * @brief tuya Server Implementation.
 *
 * Copyright (C) beken 2009-2015
 *
 *
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_TUYA_SERVER)
#include "attm.h"
#include "tuya_service.h"
#include "tuya_service_task.h"
#include "prf_utils.h"
#include "prf.h"
#include "ke_mem.h"

#include "uart.h"



/*
 * TUYA ATTRIBUTES DEFINITION
 ****************************************************************************************
 */
 
/// Full TUYA Database Description - Used to add attributes into the database
const struct attm_desc tuya_att_db[TUYAS_IDX_NB] =
{
    //  Service Declaration
    [TUYAS_IDX_SVC]            =   {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), 0, 0},

	[TUYAS_IDX_CHAR_WRITE_LVL_CHAR]  =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    //  Characteristic Value
    [TUYAS_IDX_CHAR_WRITE_LVL_VAL]   =   {ATT_USER_SERVER_CHAR_WRITE,PERM(WRITE_COMMAND, ENABLE), PERM(RI, ENABLE), TUYA_CHAR_WRITE_DATA_LEN *sizeof(uint8_t)},

	//  Level Characteristic Declaration
	[TUYAS_IDX_CHAR_NOTIFY_LVL_CHAR]  =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    //  Level Characteristic Value
    [TUYAS_IDX_CHAR_NOTIFY_LVL_VAL]   =   {ATT_USER_SERVER_CHAR_NOTIFY, PERM(WRITE_COMMAND, ENABLE) , PERM(RI, ENABLE), TUYA_CHAR_NOTIFY_DATA_LEN * sizeof(uint8_t)},

	//  Level Characteristic - Client Characteristic Configuration Descriptor
	[TUYAS_IDX_CHAR_NOTIFY_LVL_NTF_CFG] = {ATT_DESC_CLIENT_CHAR_CFG,  PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},

};/// Macro used to retrieve permission value from access and rights on attribute.


static uint8_t tuyas_init (struct prf_task_env* env, uint16_t* start_hdl, uint16_t app_task, uint8_t sec_lvl,  struct tuyas_db_cfg* params)
{
    uint16_t shdl;
    struct tuyas_env_tag* tuyas_env = NULL;
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    
    //-------------------- allocate memory required for the profile  ---------------------
    tuyas_env = (struct tuyas_env_tag* ) ke_malloc(sizeof(struct tuyas_env_tag), KE_MEM_ATT_DB);
    memset(tuyas_env, 0 , sizeof(struct tuyas_env_tag));

    // Service content flag
    uint8_t cfg_flag = TUYAS_CFG_FLAG_MANDATORY_MASK;

    // Save database configuration
    tuyas_env->features |= (params->features) ;
   
    // Check if notifications are supported
    if (params->features == TUYAS_CHAR_NOTIFY_LVL_NTF_SUP)
    {
        cfg_flag |= TUYAS_CFG_FLAG_NTF_SUP_MASK;
    }
    shdl = *start_hdl;

    //Create tuyas in the DB
    //------------------ create the attribute database for the profile -------------------
    status = attm_svc_create_db(&(shdl), ATT_USER_SERVER_TUYA, (uint8_t *)&cfg_flag,
            TUYAS_IDX_NB, NULL, env->task, &tuya_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)));
				


    //Set optional permissions
    if (status == GAP_ERR_NO_ERROR)
    {
        //Set optional permissions
        if(params->features == TUYAS_CHAR_NOTIFY_LVL_NTF_SUP)
        {
            //  Level characteristic value permissions
            uint16_t perm = PERM(NTF, ENABLE);

            attm_att_set_permission(shdl + TUYAS_IDX_CHAR_NOTIFY_LVL_VAL, perm, 0);
        }
    }

    //-------------------- Update profile task information  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {

        // allocate  required environment variable
        env->env = (prf_env_t*) tuyas_env;
        *start_hdl = shdl;
        tuyas_env->start_hdl = *start_hdl;
        tuyas_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        tuyas_env->prf_env.prf_task = env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        env->id                     = TASK_ID_TUYAS;
        env->desc.idx_max           = TUYAS_IDX_MAX;
        env->desc.state             = tuyas_env->state;
        env->desc.default_handler   = &tuyas_default_handler;

        // service is ready, go into an Idle state
        ke_state_set(env->task, TUYAS_IDLE);
    }
    else if(tuyas_env != NULL)
    {
        ke_free(tuyas_env);
    }
     
    return (status);
}


static void tuyas_destroy(struct prf_task_env* env)
{
    struct tuyas_env_tag* tuyas_env = (struct tuyas_env_tag*) env->env;

    // clear on-going operation
    if(tuyas_env->operation != NULL)
    {
        ke_free(tuyas_env->operation);
    }

    // free profile environment variables
    env->env = NULL;
    ke_free(tuyas_env);
}

static void tuyas_create(struct prf_task_env* env, uint8_t conidx)
{
    struct tuyas_env_tag* tuyas_env = (struct tuyas_env_tag*) env->env;
    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);

    // force notification config to zero when peer device is connected
    tuyas_env->ntf_cfg[conidx] = 0;
}


static void tuyas_cleanup(struct prf_task_env* env, uint8_t conidx, uint8_t reason)
{
    struct tuyas_env_tag* tuyas_env = (struct tuyas_env_tag*) env->env;

    ASSERT_ERR(conidx < BLE_CONNECTION_MAX);
    // force notification config to zero when peer device is disconnected
    tuyas_env->ntf_cfg[conidx] = 0;
}


void tuyas_notify_value_lvl(struct tuyas_env_tag* tuyas_env, struct tuyas_char_notify_level_upd_req const *param)
{
    // Allocate the GATT notification message
    struct gattc_send_evt_cmd *char_lvl = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            KE_BUILD_ID(TASK_GATTC, 0), prf_src_task_get(&(tuyas_env->prf_env),0),
            gattc_send_evt_cmd, sizeof(uint8_t)* (param->length));

    // Fill in the parameter structure
    char_lvl->operation = GATTC_NOTIFY;
    char_lvl->handle = tuyas_get_att_handle(TUYAS_IDX_CHAR_NOTIFY_LVL_VAL);
    // pack measured value in database
    char_lvl->length = param->length;
  	//fff1_lvl->value[0] = fff0s_env->fff1_lvl[0];
	memcpy(&char_lvl->value[0],&param->value[0],param->length);
    // send notification to peer device
    ke_msg_send(char_lvl);
}



/// tuya service Task interface required by profile manager
const struct prf_task_cbs tuyas_itf =
{
        (prf_init_fnct) tuyas_init,
        tuyas_destroy,
        tuyas_create,
        tuyas_cleanup,
};


const struct prf_task_cbs* tuyas_prf_itf_get(void)
{
   return &tuyas_itf;
}


uint16_t tuyas_get_att_handle( uint8_t att_idx)
{
		
    struct tuyas_env_tag *tuyas_env = PRF_ENV_GET(TUYAS, tuyas);
    uint16_t handle = ATT_INVALID_HDL;
   
    handle = tuyas_env->start_hdl;

    // increment index according to expected index
    if(att_idx < TUYAS_IDX_CHAR_NOTIFY_LVL_NTF_CFG)
    {
        handle += att_idx;
    }
    //  notification
    else if((att_idx == TUYAS_IDX_CHAR_NOTIFY_LVL_NTF_CFG) && (((tuyas_env->features ) & 0x01) == TUYAS_CHAR_NOTIFY_LVL_NTF_SUP))
    {
        handle += TUYAS_IDX_CHAR_NOTIFY_LVL_NTF_CFG;			
    }		      
    else
    {
        handle = ATT_INVALID_HDL;
    }
    

    return handle;
}

uint8_t tuyas_get_att_idx(uint16_t handle, uint8_t *att_idx)
{
    struct tuyas_env_tag* tuyas_env = PRF_ENV_GET(TUYAS, tuyas);
    uint16_t hdl_cursor = tuyas_env->start_hdl;
    uint8_t status = PRF_APP_ERROR;

    // Browse list of services
    // handle must be greater than current index 
    // check if it's a mandatory index
    if(handle <= (hdl_cursor + TUYAS_IDX_CHAR_NOTIFY_LVL_VAL))
    {
        *att_idx = handle -hdl_cursor;
        status = GAP_ERR_NO_ERROR;
        
    }
    hdl_cursor += TUYAS_IDX_CHAR_NOTIFY_LVL_VAL;

    // check if it's a notify index
    if(((tuyas_env->features ) & 0x01) == TUYAS_CHAR_NOTIFY_LVL_NTF_SUP)
    {
        hdl_cursor++;
        if(handle == hdl_cursor)
        {
            *att_idx = TUYAS_IDX_CHAR_NOTIFY_LVL_NTF_CFG;
            status = GAP_ERR_NO_ERROR;
        }
    }
    hdl_cursor++;
    
    return (status);
}


#endif // (BLE_TUYA_SERVER)


 
