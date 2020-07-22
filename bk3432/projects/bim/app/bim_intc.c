/**
 ****************************************************************************************
 *
 * @file bim_intc.c
 *
 * @brief Definition of the Interrupt Controller (INTCTRL) API.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */ 

#include "bim_intc.h"
#include <string.h>
#include "bim_uart.h"
typedef void (*FUNCPTR_T)(void);

extern void bim_wdt_enable(uint16_t wdt_cnt);


void Undefined_Exception(void)
{
    bim_wdt_enable(0X10);
	while(1)
	{
		//UART_PRINTF("%s \r\n",__func__);
		//uart_putchar("Undefined_Exception\r\n");
	}

}
void SoftwareInterrupt_Exception(void)
{
    bim_wdt_enable(0X10);
	while(1)
	{  
		//UART_PRINTF("%s \r\n",__func__);
		//uart_putchar("SoftwareInterrupt_Exception\r\n");
	}

}
void PrefetchAbort_Exception(void)
{
    bim_wdt_enable(0X10);
	while(1)
	{ 
		//UART_PRINTF("%s \r\n",__func__);
		//uart_putchar("PrefetchAbort_Exception\r\n");
	}

}
void DataAbort_Exception(void)
{
    bim_wdt_enable(0X10);
	while(1)
	{    
		//UART_PRINTF("%s \r\n",__func__);
		//uart_putchar("DataAbort_Exception\r\n");
	}

} 
	
void Reserved_Exception(void)
{
    bim_wdt_enable(0X10);
	while(1)
	{ 
		//UART_PRINTF("%s \r\n",__func__);
		//uart_putchar("Reserved_Exception\r\n");
	}

}

#pragma ARM
__IRQ void Irq_Exception(void)
{
    uint32_t IntStat=REG_AHB0_ICU_INT_FLAG;
    
    if(IntStat & INT_STATUS_UART_bit)
        bim_uart_isr();


    REG_AHB0_ICU_INT_FLAG=IntStat;

}



