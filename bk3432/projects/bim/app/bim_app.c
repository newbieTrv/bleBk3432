

#include <stdint.h>
#include "bim_flash.h"
#include "bim_app.h"
#include "BK3432_reg.h"
#include "bim_uart.h"
#include "bim_updataImage.h"



typedef void (*FUNCPTR)(void);

void  updata_memset32(void * dest, uint32_t value, uint32_t size)
{
	uint32_t *_u8_dest = (uint32_t *)dest;
	uint32_t *_u8_end  = (uint32_t *)dest+size*4;

	while (_u8_dest < _u8_end)
	{
		*_u8_dest++ = value;
	}
}


void bim_icu_init(uint8_t clk)
{
	REG_AHB0_ICU_FLASH &= ~(0xff << 16);
	REG_AHB0_ICU_FLASH |= (0x15 << 16);
	REG_AHB0_ICU_CPU_STATUS  = 0x771;

	REG_AHB0_ICU_DIGITAL_PWD = REG_AHB0_ICU_DIGITAL_PWD & (~0X02);
	REG_AHB0_ICU_CORECLKCON = 0X00;  //clk div 0
	REG_AHB0_ICU_CLKSRCSEL = 0X000001F9; //usr 16m, sel=0, dec=7,normal=7
	REG_AHB0_ICU_ANA_CTL &= ~(0X01 << 6);
}


#define MAIN_IMAGE_RUN_ADDR         0x1020


void bim_main(void)
{
	int32_t main_point = MAIN_IMAGE_RUN_ADDR;

	updata_memset32((uint8_t *)0x00400000, 1, 1);

	bim_icu_init(ICU_CLK_16M);

	bim_flash_init();

	//bim_uart_init(115200);

	//REG_AHB0_ICU_INT_ENABLE = (0x01 << 5);
	//REG_AHB0_ICU_IRQ_ENABLE = 0x03;
	//__enable_irq();
	//__disable_irq( );
	//REG_AHB0_ICU_INT_ENABLE = 0x00;
	//REG_AHB0_ICU_IRQ_ENABLE = 0x00;
	updata_memset32((uint8_t *)0x00400000, 0, 1);
	//bim_uart_deinit();	
	if(1 == bim_select_sec())
	{
		(*(FUNCPTR)main_point)();
	}


}

