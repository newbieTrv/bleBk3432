#include <stddef.h>     // standard definition
#include "timer.h"      // timer definition
#include "bim_uart.h"       // uart definition
#include "BK3432_reg.h"
#include <stdlib.h>    // standard lib functions
#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>   // boolean definition
#include "string.h"
#include <stdlib.h>
#include  <stdarg.h>
#include  <stdio.h>
#include "bim_app.h"
#include "bim_flash.h"






#define Uart_Write_Byte(v)               (REG_APB3_UART_PORT=v)
#define UART_TX_FIFO_COUNT               (REG_APB3_UART_FIFO_STAT&0xff)
#define UART_RX_FIFO_COUNT               ((REG_APB3_UART_FIFO_STAT>>8)&0xff)
#define UART_TX_FIFO_FULL                (REG_APB3_UART_FIFO_STAT&0x00010000)
#define UART_TX_FIFO_EMPTY               (REG_APB3_UART_FIFO_STAT&0x00020000)
#define UART_RX_FIFO_FULL                (REG_APB3_UART_FIFO_STAT&0x00040000)
#define UART_RX_FIFO_EMPTY               (REG_APB3_UART_FIFO_STAT&0x00080000)
#define UART_TX_WRITE_READY              (REG_APB3_UART_FIFO_STAT&0x00100000)
#define UART_RX_READ_READY               (REG_APB3_UART_FIFO_STAT&0x00200000)
#define bit_UART_TXFIFO_NEED_WRITE        0x01
#define bit_UART_RXFIFO_NEED_READ         0x02
#define bit_UART_RXFIFO_OVER_FLOW         0x04
#define bit_UART_RX_PARITY_ERROR          0x08
#define bit_UART_RX_STOP_ERROR            0x10
#define bit_UART_TX_PACKET_END            0x20
#define bit_UART_RX_PACKET_END            0x40
#define bit_UART_RXD_WAKEUP_DETECT        0x80




#define uart_tx_en    0x1      // 0: Disable TX, 1: Enable TX 
#define uart_rx_en    0x1      // 0: Disable RX, 1: Enable RX
#define irda_mode     0x0      // 0: UART  MODE, 1: IRDA MODE
#define data_len      0x3      // 0: 5 bits, 1: 6 bits, 2: 7 bits, 3: 8 bits
#define parity_en     0x1      // 0: NO Parity, 1: Enable Parity
#define parity_mode   0x1      // 0: Odd Check, 1: Even Check 
#define stop_bits     0x0      // 0: 1 stop-bit, 1: 2 stop-bit 
#define uart_clks     16000000 // UART's Main-Freq, 26M 
#define baud_rate     115200 // UART's Baud-Rate,  1M 
#define BIM_REG_PL_RD(addr)              (*(volatile uint32_t *)(addr))


int bim_uart_printf(const char *fmt,...)
{
	return 0;
}
int bim_uart_printf_null(const char *fmt,...)
{
	return 0;
}

int bim_uart_putchar(char * st)
{
	uint8_t num = 0;

	while (*st)
	{
		if(UART_TX_WRITE_READY)
		{
			REG_APB3_UART_PORT = *st;
			st++;
			num++;
		}
	}

	return num;
}

void uart_send(unsigned char *buff, int len)
{
	while (len--)
	{
		while(!UART_TX_WRITE_READY);
		REG_APB3_UART_PORT = *buff++ ;
	}
}

uint8_t bim_uart_fifo_empty_getf(void)
{
	uint32_t localVal = BIM_REG_PL_RD(0x00806308);
	return ((localVal & ((uint32_t)0x00020000)) >> 17);
}

void bim_uart_send_byte( char data)
{
	while (!bim_uart_fifo_empty_getf());

	REG_APB3_UART_PORT = data ;
}

void bim_uart_write( char *buff)
{
#if 0
	uint8_t len = strlen((char*)buff);
	while (len--)
		bim_uart_send_byte(*buff++);
#endif
}



char* itoa_my(int v,char *str, int radix)
{
	char zm[37]="0123456789abcdefghijklmnopqrstuvwxyz";
	char aa[100]= {0};

	int sum = v;
	char *cp = str;
	int i = 0;

	if( radix<2||radix>36 )//增加了对错误的检测
	{
		return str;
	}

	if(v <0)
	{
		return str;
	}


	while( sum > 0 )
	{
		aa[i++]=zm[sum%radix];
		sum/=radix;
	}

	for(int j=i-1; j>=0; j--)
	{
		*cp++=aa[j];
	}
	*cp='\0';
	return str;
}


void bim_printf(char *title, uint32_t v, uint8_t radix)
{
#if 0
	uint8_t	tmpLen;
	char buf[128];
	uint32_t err;

	tmpLen = strlen( (char*)title);
	memcpy( buf, title, tmpLen );
	err = (uint32_t)(v);
	itoa_my( err, &buf[tmpLen], radix);
	bim_uart_write(&buf[0]);
#endif
}


void bim_uart_init(uint32_t baudrate)
{

	unsigned int baud_divisor ;
	REG_AHB0_ICU_UARTCLKCON   &= ~(0x1 << 0) ;  // Enable Uart's Clocks
	baud_divisor = 0x8a;//uart_clks/baud_rate ;
	baud_divisor = baud_divisor-1 ;
	REG_APB3_UART_CFG  = (baud_divisor<<8) +
	                     (stop_bits   <<7) +
	                     //(parity_mode <<6) +
	                     //(parity_en   <<5) +
	                     (data_len    <<3) +
	                     (irda_mode   <<2) +
	                     (uart_rx_en  <<1) +
	                     uart_tx_en       ;
	REG_APB3_UART_FIFO_CFG  = 0x00001001 ;  // Set Fifo threshold 8
	REG_APB3_UART_INT_ENABLE = ((0x01 << 1) | (0x01 << 6) | (0x01 << 7));  //need read / stop end /rxd wake up
	REG_APB3_UART_FLOW_CFG  = 0x00000000 ;  // No Flow Control
	REG_APB3_UART_WAKE_CFG  =  ((0x01 << 0 )| (0x01 << 20) |  (0x01 << 21)| (0x01 << 22));  // No Wake Control

	REG_APB5_GPIOA_CFG  &= ~((0x3<<BIT_GPIO_PULL_UP)  + (0x3<<BIT_GPIO_PERI_EN));
	REG_APB5_GPIOA_CFG  |= ((0x3<<BIT_GPIO_PULL_UP));
	REG_APB5_GPIOA_CFG  |=   (0x3<<BIT_GPIO_OUT_EN_N);

	REG_APB5_GPIOA_DATA &= ~ (0x3<<BIT_GPIO_INPUT_EN);

}



#if 1 //
void bim_uart_deinit( void )
{
	REG_AHB0_ICU_UARTCLKCON   = 0x1 ;  // disable Uart's Clocks

	REG_APB5_GPIOA_CFG  &= ~((0x3<<BIT_GPIO_PULL_UP)  + (0x3<<BIT_GPIO_PERI_EN));
	REG_APB5_GPIOA_CFG  |= ((0x3<<BIT_GPIO_PERI_EN));

}


uint8_t erase_fenable;
uint8_t bim_uart_rx_buf[BIM_UART0_RX_FIFO_MAX_COUNT];
uint8_t bim_uart_cmd[16];
uint8_t bim_uart_data[4096+8];
uint8_t read_data[512];
uint8_t uart_download_status=0;
uint16_t uart_buff_write;

void bim_uart_isr(void)
{
	unsigned int IntStat;

	IntStat = REG_APB3_UART_INT_STAT;
	if(IntStat & 0x42)
	{
		while((REG_APB3_UART_FIFO_STAT & (0x01 << 21)))
		{
			bim_uart_rx_buf[uart_buff_write++] = ((REG_APB3_UART_PORT>>8)&0xff);
			if( BIM_UART0_RX_FIFO_MAX_COUNT == uart_buff_write )
			{
				uart_buff_write = 0;
			}
		}

	}
	REG_APB3_UART_INT_STAT=IntStat;
}


uint32_t crc32_table[256];
int make_crc32_table(void)
{
	uint32_t c;
	int i = 0;
	int bit = 0;
	for(i = 0; i < 256; i++)
	{
		c = (uint32_t)i;
		for(bit = 0; bit < 8; bit++)
		{
			if(c&1)
			{
				c = (c>>1)^(0xEDB88320);
			}
			else
			{
				c = c >> 1;
			}
		}
		crc32_table[i] = c;

	}
	return 0;
}

uint32_t make_crc32(uint32_t crc,unsigned char *string,uint32_t size)
{
	while(size--)
	{
		crc = (crc >> 8)^(crc32_table[(crc^*string++)&0xff]);
	}
	return crc;
}

void bim_delay_ms(unsigned int tt)
{
	volatile unsigned int i, j;
	while(tt--)
	{
		for (j = 0; j < 1000/10; j++)
		{
			for (i = 0; i < 12; i++)
			{
				;
			}
		}
	}
}


#endif



