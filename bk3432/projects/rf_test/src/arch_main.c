/**
 ****************************************************************************************
 *
 * @file arch_main.c
 *
 * @brief Main loop of the application.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ******** ********************************************************************************
 */


/*
 * INCLUDES
 ****************************************************************************************
 */

#include "rwip_config.h" // RW SW configuration

#include "arch.h"      // architectural platform definitions
#include <stdlib.h>    // standard lib functions
#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>   // boolean definition
#include "boot.h"      // boot definition
#include "rwip.h"      // RW SW initialization
#include "syscntl.h"   // System control initialization
#include "emi.h"       // EMI initialization
#include "intc.h"      // Interrupt initialization
#include "timer.h"     // TIMER initialization
#include "icu.h"
#include "flash.h"
#include "uart_rf.h"      	// UART initialization
#include "flash.h"     // Flash initialization
//#include "led.h"       // Led initialization
#if (BLE_EMB_PRESENT || BT_EMB_PRESENT)
#include "rf.h"        // RF initialization
#endif // BLE_EMB_PRESENT || BT_EMB_PRESENT

#if (BLE_APP_PRESENT)
#endif // BLE_APP_PRESENT

#include "nvds.h"         // NVDS definitions

#include "reg_assert_mgr.h"
#include "BK3432_reg.h"
#include "RomCallFlash.h"
#include "gpio.h"
#include "user_config.h"
#include "rf_test.h"
#include "wdt.h"

/**
 ****************************************************************************************
 * @addtogroup DRIVERS
 * @{
 *
 *
 * ****************************************************************************************
 */



/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

// Creation of uart external interface api
struct rwip_eif_api uart_api;

/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */
#if ((UART_PRINTF_EN) &&(UART_DRIVER))
void assert_err(const char *condition, const char * file, int line)
{
	uart_printf("%s,condition %s,file %s,line = %d\r\n",__func__,condition,file,line);

}

void assert_param(int param0, int param1, const char * file, int line)
{
	uart_printf("%s,param0 = %d,param1 = %d,file = %s,line = %d\r\n",__func__,param0,param1,file,line);

}

void assert_warn(int param0, int param1, const char * file, int line)
{
	uart_printf("%s,param0 = %d,param1 = %d,file = %s,line = %d\r\n",__func__,param0,param1,file,line);

}

void dump_data(uint8_t* data, uint16_t length)
{
	uart_printf("%s,data = %d,length = %d,file = %s,line = %d\r\n",__func__,data,length);

}
#else
void assert_err(const char *condition, const char * file, int line)
{

}

void assert_param(int param0, int param1, const char * file, int line)
{

}

void assert_warn(int param0, int param1, const char * file, int line)
{

}

void dump_data(uint8_t* data, uint16_t length)
{

}
#endif //UART_PRINTF_EN

/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void platform_reset(uint32_t error)
{
	//void (*pReset)(void);

	UART_PRINTF("error = %x\r\n", error);

	// Disable interrupts
	GLOBAL_INT_STOP();

#if UART_PRINTF_EN
	// Wait UART transfer finished
	uart_finish_transfers();
#endif //UART_PRINTF_EN


	if(error == RESET_AND_LOAD_FW || error == RESET_TO_ROM)
	{
		// Not yet supported
	}
	else
	{
		//Restart FW
		//pReset = (void * )(0x0);
		//pReset();
		wdt_enable(10);
		while(1);
	}
}


void bdaddr_env_init(void)
{
	struct bd_addr co_bdaddr;
	flash_read(FLASH_SPACE_TYPE_MAIN, 0x27ff0/4, 6, &co_bdaddr.addr[0]);
	if(co_bdaddr.addr[0]!=0xff ||co_bdaddr.addr[1]!=0xff||
	        co_bdaddr.addr[2]!=0xff||co_bdaddr.addr[3]!=0xff||
	        co_bdaddr.addr[4]!=0xff||co_bdaddr.addr[5]!=0xff )
	{
		memcpy(&co_default_bdaddr,&co_bdaddr,6);
	}
}


void ble_clk_enable(void)
{
	REG_AHB0_ICU_BLECLKCON =  0;
}


/**
 *******************************************************************************
 * @brief RW main function.
 *
 * This function is called right after the booting process has completed.
 *
 * @return status   exit status
 *******************************************************************************
 */

extern struct rom_env_tag rom_env;

void rwip_eif_api_init(void);
void rw_main(void)
{
	/*
	 ***************************************************************************
	 * Platform initialization
	 ***************************************************************************
	 */
	icu_init();

	// Initialize random process
	srand(1);

	//get System sleep flag
	system_sleep_init();

	// Initialize the exchange memory interface
	emi_init();

	// Initialize timer module
	timer_init();

	rwip_eif_api_init();

	// Initialize the Interrupt Controller
	intc_init();
	// Initialize UART component
#if UART_DRIVER
	uart_init(115200);
#endif

#if PLF_NVDS
	// Initialize NVDS module
	struct nvds_env_tag env;
	env.flash_read = &flash_read;
	env.flash_write = &flash_write;
	env.flash_erase = &flash_erase;
	nvds_init(env);
#endif

	flash_init();
	rom_env_init(&rom_env);

	/*
	  ***************************************************************************
	  * RW SW stack initialization
	  ***************************************************************************
	  */
	//enable ble clock
	ble_clk_enable();

	// Initialize RW SW stack
	rwip_init(0);

	bdaddr_env_init();

	REG_AHB0_ICU_INT_ENABLE |= (0x01 << 15);
	REG_AHB0_ICU_IRQ_ENABLE = 0x03;

	// finally start interrupt handling
	GLOBAL_INT_START();

	UART_PRINTF("start\r\n");

	/*
	 ***************************************************************************
	 * Main loop
	 ***************************************************************************
	 */
	while(1)
	{
		rf_test_ready();
	}
}


void rwip_eif_api_init(void)
{
	uart_api.read = &uart_read;
	uart_api.write = &uart_write;
	uart_api.flow_on = &uart_flow_on;
	uart_api.flow_off = &uart_flow_off;
}

const struct rwip_eif_api* rwip_eif_get(uint8_t type)
{
	const struct rwip_eif_api* ret = NULL;
	switch(type)
	{
	case RWIP_EIF_AHI:
	{
		ret = &uart_api;
	}
	break;
#if (BLE_EMB_PRESENT) || (BT_EMB_PRESENT)
	case RWIP_EIF_HCIC:
	{
		ret = &uart_api;
	}
	break;
#elif !(BLE_EMB_PRESENT) || !(BT_EMB_PRESENT)
	case RWIP_EIF_HCIH:
	{
		ret = &uart_api;
	}
	break;
#endif
	default:
	{
		ASSERT_INFO(0, type, 0);
	}
	break;
	}
	return ret;
}


void rom_env_init(struct rom_env_tag *api)
{
	memset(&rom_env,0,sizeof(struct rom_env_tag));
	rom_env.prf_get_id_from_task = prf_get_id_from_task;
	rom_env.prf_get_task_from_id = prf_get_task_from_id;
	rom_env.prf_init = prf_init;
	rom_env.prf_create = prf_create;
	rom_env.prf_cleanup = prf_cleanup;
	rom_env.prf_add_profile = prf_add_profile;
	rom_env.rwble_hl_reset = rwble_hl_reset;
	rom_env.rwip_reset = rwip_reset;
#if SYSTEM_SLEEP
	rom_env.rwip_prevent_sleep_set = rwip_prevent_sleep_set;
	rom_env.rwip_prevent_sleep_clear = rwip_prevent_sleep_clear;
	rom_env.rwip_sleep_lpcycles_2_us = rwip_sleep_lpcycles_2_us;
	rom_env.rwip_us_2_lpcycles = rwip_us_2_lpcycles;
	rom_env.rwip_wakeup_delay_set = rwip_wakeup_delay_set;
#endif
	rom_env.platform_reset = platform_reset;
	rom_env.assert_err = assert_err;
	rom_env.assert_param = assert_param;
	rom_env.Read_Uart_Buf = Read_Uart_Buf;
	rom_env.uart_clear_rxfifo = uart_clear_rxfifo;

}

/// @} DRIVERS
