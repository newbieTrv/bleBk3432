/**
 ****************************************************************************************
 *
 * @file bim_uart.h
 *
 * @brief UART Driver for HCI over UART operation.
 *
 * Copyright (C) Beken 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _BIM_UART_H_
#define _BIM_UART_H_

/**
 ****************************************************************************************
 * @defgroup UART UART
 * @ingroup DRIVERS
 * @brief UART driver
 *
 * @{
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdbool.h>          // standard boolean definitions
#include <stdint.h>           // standard integer functions



#if 0
#define UART_PRINTF bim_uart_printf  
#else
#define UART_PRINTF bim_uart_printf_null 
#endif // #if UART_PRINTF_EN
 

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initializes the UART to default values.
 *****************************************************************************************
 */

typedef enum _UART_CMD_STATE
{
    
    UART_CMD_STATE_HEAD,
    UART_CMD_STATE_OPCODE_ONE,
    UART_CMD_STATE_OPCODE_TWO,
    UART_CMD_STATE_LENGTH,
    UART_CMD_STATE_CMD,
    UART_CMD_STATE_CMD_FLASH,
    UART_CMD_STATE_LENGTH_FLASH_LEN0,
    UART_CMD_STATE_LENGTH_FLASH_LEN1,
    UART_CMD_STATE_LENGTH_FLASH_SCMD,
    UART_CMD_STATE_PAYLOAD,
    UART_CMD_STATE_ERROR_ONE,
    UART_CMD_STATE_ERROR_TWO,
    UART_CMD_STATE_ERROR_THREE,
    UART_CMD_STATE_ERROR_FOUR,
    UART_CMD_STATE_PACKET
    
} UART_CMD_STATE;

#define LINK_CHECK_CMD     0X00
#define CRC_CHECK_CMD      0X10
#define SET_RESET_CMD      0X0E
#define SET_BAUDRATE_CMD   0X0F
#define STAY_ROM_CMD       0XAA




#define BIM_UART0_RX_FIFO_MAX_COUNT  512








void bim_uart_init(uint32_t baudrate);

void bim_dbg_initial(void);

void bim_uart_write( char *buff);
void bim_printf(char *title, uint32_t v, uint8_t radix) ;
int bim_dbg_putchar(char * st);
int bim_uart_putchar(char * st);
int bim_uart_printf(const char *fmt,...);
int bim_uart_printf_null(const char *fmt,...);
int bim_dbg_printf(const char *fmt,...);
void bim_uart_print_int(unsigned int num);
uint8_t bim_check_uart_stop(void);

void bim_cpu_delay( volatile unsigned int times);
void uart_cmd_dispath(uint8_t *buff,uint8_t len);
void cmd_response( uint8_t cmd, uint8_t length, uint8_t *payload );
void operate_flash_cmd_response( uint8_t cmd, uint8_t status,uint16_t length, uint8_t *payload );
void bim_uart_data_callback( uint8_t * buff, uint16_t len);
void bim_uart_isr(void);
void bim_uart_deinit( void );


/// @} UART
#endif /* _BIM_UART_H_ */
