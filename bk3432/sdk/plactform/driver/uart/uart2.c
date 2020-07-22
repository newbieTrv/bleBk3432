/**
 ****************************************************************************************
 *
 * @file uart2.c
 *
 * @brief UART driver
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup UART
 * @{
 ****************************************************************************************
 */ 
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stddef.h>     // standard definition
#include "timer.h"      // timer definition
#include "uart2.h"       // uart definition
#include "BK3432_reg.h"
#include "reg_uart2.h"   // uart register
#include "rwip.h"       // SW interface
#include "h4tl.h"
#if (NVDS_SUPPORT)
#include "nvds.h"                    // NVDS Definitions
#endif
#include "dbg.h"
#include "tuya_ble_api.h"
/*
 * DEFINES
 *****************************************************************************************
 */

/// Max baudrate supported by this UART (in bps)
#define UART_BAUD_MAX      		  3500000
/// Min baudrate supported by this UART (in bps)
#define UART_BAUD_MIN      		  9600

/// Duration of 1 byte transfer over UART (10 bits) in us (for 921600 default baudrate)
#define UART_CHAR_DURATION        11

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */
 
 extern void uart_stack_register(void *cb);
 
 /*
 * common.c
 *
 *  Created on: 2013-5-7
 *  Author: pujie
 */

#include  <stdarg.h>
#include  <stdio.h>
#include "BK3432_reg.h"
#include "ea.h"


#define Uart2_Write_Byte(v)               (REG_APB3_UART2_PORT=v)

#define UART2_TX_FIFO_COUNT               (REG_APB3_UART2_FIFO_STAT&0xff)
#define UART2_RX_FIFO_COUNT               ((REG_APB3_UART2_FIFO_STAT>>8)&0xff)
#define UART2_TX_FIFO_FULL                (REG_APB3_UART2_FIFO_STAT&0x00010000)
#define UART2_TX_FIFO_EMPTY               (REG_APB3_UART2_FIFO_STAT&0x00020000)
#define UART2_RX_FIFO_FULL                (REG_APB3_UART2_FIFO_STAT&0x00040000)
#define UART2_RX_FIFO_EMPTY               (REG_APB3_UART2_FIFO_STAT&0x00080000) 
#define UART2_TX_WRITE_READY              (REG_APB3_UART2_FIFO_STAT&0x00100000) 
#define UART2_RX_READ_READY               (REG_APB3_UART2_FIFO_STAT&0x00200000) 
#define bit_UART2_TXFIFO_NEED_WRITE        0x01
#define bit_UART2_RXFIFO_NEED_READ         0x02
#define bit_UART2_RXFIFO_OVER_FLOW         0x04
#define bit_UART2_RX_PARITY_ERROR          0x08
#define bit_UART2_RX_STOP_ERROR            0x10
#define bit_UART2_TX_PACKET_END            0x20
#define bit_UART2_RX_PACKET_END            0x40
#define bit_UART2_RXD_WAKEUP_DETECT        0x80


#define uart_tx_en    0x1      // 0: Disable TX, 1: Enable TX 
#define uart_rx_en    0x1      // 0: Disable RX, 1: Enable RX
#define irda_mode     0x0      // 0: UART  MODE, 1: IRDA MODE
#define data_len      0x3      // 0: 5 bits, 1: 6 bits, 2: 7 bits, 3: 8 bits
#define parity_en     0x0      // 0: NO Parity, 1: Enable Parity
#define parity_mode   0x1      // 0: Odd Check, 1: Even Check 
#define stop_bits     0x0      // 0: 1 stop-bit, 1: 2 stop-bit 
#define uart_clks     16000000 // UART's Main-Freq, 26M 
#define baud_rate     115200   // UART's Baud-Rate,  1M 


unsigned char uart2_rx_buf[UART2_RX_FIFO_MAX_COUNT];
//unsigned char uart2_tx_buf[UART2_TX_FIFO_MAX_COUNT];
volatile bool uart2_rx_done = 0;
volatile unsigned long uart2_rx_index = 0;
uint8_t cur_read_buf_idx2 = 0;


#define UART2_READ_BYTE()                 (REG_APB3_UART2_PORT&0xff)


///UART Character format
enum UART_CHARFORMAT
{
    UART_CHARFORMAT_8 = 0,
    UART_CHARFORMAT_7 = 1
};

///UART Stop bit
enum UART_STOPBITS
{
    UART_STOPBITS_1 = 0,
    UART_STOPBITS_2 = 1  /* Note: The number of stop bits is 1.5 if a character format
                            with 5 bit is chosen*/
};

///UART Parity enable
enum UART_PARITY
{
    UART_PARITY_DISABLED = 0,
    UART_PARITY_ENABLED  = 1
};

///UART Parity type
enum UART_PARITYBIT
{
    UART_PARITYBIT_EVEN  = 0,
    UART_PARITYBIT_ODD   = 1,
    UART_PARITYBIT_SPACE = 2, // The parity bit is always 0.
    UART_PARITYBIT_MARK  = 3  // The parity bit is always 1.
};

///UART HW flow control
enum UART_HW_FLOW_CNTL
{
    UART_HW_FLOW_CNTL_DISABLED = 0,
    UART_HW_FLOW_CNTL_ENABLED = 1
};

///UART Input clock select
enum UART_INPUT_CLK_SEL
{
    UART_INPUT_CLK_SEL_0 = 0,
    UART_INPUT_CLK_SEL_1 = 1,
    UART_INPUT_CLK_SEL_2 = 2,
    UART_INPUT_CLK_SEL_3 = 3
};

///UART Interrupt enable/disable
enum UART_INT
{
    UART_INT_DISABLE = 0,
    UART_INT_ENABLE = 1
};

///UART Error detection
enum UART_ERROR_DETECT
{
    UART_ERROR_DETECT_DISABLED = 0,
    UART_ERROR_DETECT_ENABLED  = 1
};

/*
 * STRUCT DEFINITIONS
 *****************************************************************************************
 */
/* TX and RX channel class holding data used for asynchronous read and write data
 * transactions
 */
/// UART TX RX Channel
struct uart_txrxchannel
{
    /// call back function pointer
    void (*callback) (void*, uint8_t);
    /// Dummy data pointer returned to callback when operation is over.
    void* dummy;
};

/// UART environment structure
struct uart_env_tag
{
    /// tx channel
    struct uart_txrxchannel tx;
    /// rx channel
    struct uart_txrxchannel rx;
    /// error detect
    uint8_t errordetect;
    /// external wakeup
    bool ext_wakeup;
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// uart environment structure
struct uart_env_tag uart2_env;
//char uart2_buff[64];



static UART2_RX_CALLBACK_T uart2_rx_cb = NULL; 

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */


/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */
uint8_t Read_Uart2_Buf(void)
{
	return 0;//uart2_rx_buf[cur_read_buf_idx2++];
}	

uint8_t Uart2_Read_Byte(void)
{
	return (REG_APB3_UART2_PORT&0xff);
}	


int uart2_putchar(char * st)
{
	uint8_t num = 0;
    while (*st) 
    {
		if(UART2_TX_WRITE_READY)
		{
			REG_APB3_UART2_PORT = *st;
	    	st++;
			num++;
	    }
	} 
    return num;
}
/*
int uart2_printf(const char *fmt,...)
{ 
	int n;	
	
	va_list ap;
	va_start(ap, fmt);
	n=vsprintf(uart2_buff, fmt, ap);
	va_end(ap);
    uart2_putchar(uart2_buff);
	if(n > sizeof(uart2_buff))
	{
		uart2_putchar("buff full \r\n");
	}
		
    return n;
}

int uart2_printf_null(const char *fmt,...)
{
	return 0;
}


static char *hex2Str( uint8_t data)
{

  char hex[] = "0123456789ABCDEF";
  static char str[3];
  char *pStr = str;
 
  *pStr++ = hex[data >> 4];
  *pStr++ = hex[data & 0x0F];
  *pStr = 0;

  return str;
}

void uart2_print_int(unsigned int num)
{
	uint8_t i;
	uint8_t m;

	uart2_putchar((char *)"0x");
	for (i = 4;i > 0;i--)
	{
		m = ((num >> (8 * (i - 1)))& 0xff);
		uart2_putchar(hex2Str(m));
	}
	uart2_putchar("\r\n");
}

*/

void cpu2_delay( volatile unsigned int times)
{
    while(times--)
	{
		for(uint32_t i = 0;i < 1000;i++)
			;
	}				
}


void uart2_init(uint32_t baudrate)
{
    unsigned int baud_divisor ;
    REG_AHB0_ICU_UARTCLKCON   &= ~(0x1 << 0) ;  // Enable Uart's Clocks 
    switch(baudrate)
    {
    	case 9600:
		case 19200:
		case 38400:
		case 57600:
		case 115200:
			baud_divisor = 0x89;
			break;
		case 921600:
			baud_divisor = 0x10;
			break;
		default:
			baud_divisor = 0x89;
			break;
	}
    REG_APB3_UART2_CFG  = (baud_divisor<<8) + 
                          (stop_bits   <<7) + 
                          (data_len    <<3) + 
                          (irda_mode   <<2) + 
                          (uart_rx_en  <<1) + 
                           uart_tx_en ;
  
    REG_APB3_UART2_FIFO_CFG = (1<<BIT_TX_FIFO_THRESHOLD)|(16<<BIT_RX_FIFO_THRESHOLD)|(0x0 << BIT_STOP_DETECT_TIME);
	REG_APB3_UART2_INT_ENABLE = ((0x01 << 1) | (0x01 << 6) | (0x01 << 7)); //0x01 << 1
    REG_APB3_UART2_FLOW_CFG  = 0x00000000 ;  // No Flow Control   
    REG_APB3_UART2_WAKE_CFG  = ((0x01 << 0 )| (0x01 << 20) |  (0x01 << 21)| (0x01 << 22));  // No Wake Control
    	
    REG_APB5_GPIOB_CFG  &= ~((0xc0<<BIT_GPIO_PULL_UP)  + (0xc0<<BIT_GPIO_PERI_EN)); 
    REG_APB5_GPIOB_CFG  |= ((0xc0<<BIT_GPIO_PULL_UP)); 
    REG_APB5_GPIOB_CFG  |=   (0xc0<<BIT_GPIO_OUT_EN_N);
		
    REG_APB5_GPIOA_DATA &= ~ (0xc0<<BIT_GPIO_INPUT_EN);
		

    uart2_env.rx.callback = NULL;
    uart2_env.rx.dummy    = NULL;

	REG_AHB0_ICU_INT_ENABLE |= (0x01 << 16);
#if (UART_PRINTF_EN && UART_DRIVER)	
	//uart_stack_register(uart2_printf);
#endif
}	

/*

void uart2_flow_on(void)
{
    // Configure modem (HW flow control enable)
    // uart_flow_en_setf(0);
}

void uart2_clear_rxfifo(void)
{
	
	while(uart2_rx_fifo_rd_ready_getf())
	{
		Uart2_Read_Byte();
	}
	memset(uart2_rx_buf,0,UART2_RX_FIFO_MAX_COUNT);
	
}

bool uart2_flow_off(void)
{
	
	  return true;
	
}
*/
void uart2_finish_transfers(void)
{
    uart2_flow_en_setf(1);

    // Wait TX FIFO empty
    while(!uart2_tx_fifo_empty_getf());
}

/*
void uart2_read(uint8_t *bufptr, uint32_t size, void (*callback) (void*, uint8_t), void* dummy)
{
    // Sanity check
    ASSERT_ERR(bufptr != NULL);
    ASSERT_ERR(size != 0);
    ASSERT_ERR(callback != NULL);
    uart2_env.rx.callback = callback;

    uart2_env.rx.dummy    = dummy;
	
}



void uart2_write(uint8_t *bufptr, uint32_t size, void (*callback) (void*, uint8_t), void* dummy)
{
	// Sanity check
	ASSERT_ERR(bufptr != NULL);
	ASSERT_ERR(size != 0);
	ASSERT_ERR(callback != NULL);
	
	uint8_t len;
	len = size;
		
	uart2_env.tx.callback = callback;
	uart2_env.tx.dummy    = dummy;
	
	//Delay_ms(100);
	while(len--)
	{
		//cpu_delay(10);
		if(UART2_TX_WRITE_READY)
		{
			REG_APB3_UART2_PORT = *bufptr;
        	bufptr++;
    	}
	}
		
	if(callback != NULL)
	{
		// Clear callback pointer
		uart2_env.tx.callback = NULL;
		uart2_env.tx.dummy    = NULL;
		// Call handler
		callback(dummy, RWIP_EIF_STATUS_OK);
	}
}
*/
static void uart2_send_byte(unsigned char data)
{
	while (!uart2_tx_fifo_empty_getf());
    
	REG_APB3_UART2_PORT = data ;
}

void uart2_send(unsigned char *buff, int len)
{
    while (len--)
        uart2_send_byte(*buff++);
}

void uart2_cb_register(UART2_RX_CALLBACK_T cb)
{
	if(cb)
	{
		uart2_rx_cb = cb;
	}
}

void uart2_cb_clear(void)
{
	if(uart2_rx_cb)
	{
		uart2_rx_cb = NULL;
	}
}


void uart2_isr(void)
{		
   uint32_t IntStat;
	
 #if BLE_TESTER
	void (*callback) (void*, uint8_t) = NULL;
    void* data =NULL;
#endif
	
    IntStat = uart2_isr_stat_get();	 
	if(uart2_rx_fifo_need_rd_isr_getf() || uart2_rx_end_isr_getf()|| uart2_rxd_wakeup_isr_getf())
	{
		while((REG_APB3_UART2_FIFO_STAT & (0x01 << 21)))
		{
			uart2_rx_buf[uart2_rx_index++] = UART2_READ_BYTE();
			if( UART2_RX_FIFO_MAX_COUNT == uart2_rx_index )
			{
				uart2_rx_index = 0;
			}
		}

		uart2_rx_done = 1;	
		tuya_ble_common_uart_receive_data(uart2_rx_buf, uart2_rx_index);
		if(uart2_rx_cb)
		{
			(*uart2_rx_cb)(uart2_rx_buf, uart2_rx_index);
		}
		uart2_rx_index = 0;  
	}
	#if BLE_TESTER
	{
		callback = uart2_env.rx.callback;
		data = uart2_env.rx.dummy;
		if(callback != NULL)
		{
			// Clear callback pointer
			uart2_env.rx.callback = NULL;
			uart2_env.rx.dummy    = NULL;

			// Call handler
			UART_PRINTF("UART callback 0x%08x\r\n",callback);
	  	 	callback(data, RWIP_EIF_STATUS_OK);
		}
	}
	#endif // BLE_TESTER

     uart2_isr_stat_set(IntStat);
	
}

uint8_t check_uart2_stop(void)
{
	return uart2_tx_fifo_empty_getf();
}

/// @} UART
