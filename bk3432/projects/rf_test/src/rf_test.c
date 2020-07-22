
#include "rf_test.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "uart_rf.h"
#include "BK3432_reg.h"


//uint8_t freq_value,ch_status,freq,powerlevel,p_mode,d_mode=3, hopping;
uint8_t freq_value,ch_status = 0x02, freq = 40, powerlevel = 3,p_mode = 1, d_mode=0, hopping = 0;

uint8_t temp;
uint8_t index;



const uint8_t RF_CHANNEL_TABLE[80] =     //FOR ble 4.0
{   // 2402 -2480
    10,26,30,44,50, 72,18,56, 4,16, 
    78,80,76, 2,12, 34,40,54,74,38, 
    22,32,46,52,62, 60, 6,14,36,42, 
    72,18,56, 4,16, 10,26,30,44,50,
    34,40,54,74,38, 68,70,20,64,58,
    60, 6,14,36,42, 78,80,76, 2,12, 
    66,24, 8,28,48, 22,32,46,52,62,
    68,70,20,64,58, 66,24, 8,28,48,
 };



void BT_delayUs(int num)
{
    int x, y;
    for(y = 0; y < num; y ++ )
    {
        for(x = 0; x < 10; x++);
    }
}

uint8_t char_to_num(uint8_t ch)
{
    uint8_t value;
    if( (ch>='0')&&(ch<='9'))
        value=(ch-'0');
    else if( (ch>='a')&&(ch<='f'))
        value=(ch-'a'+10);
    else if ( (ch>='A')&&(ch<='F'))
        value=(ch-'A'+10);
    return value;
}


uint8_t fCheckValue(unsigned char *src, uint8_t *index)
{
	uint8_t i,j,data;
	i=char_to_num(*src); 
    if(src[1]!='+')
    {
    	src++;
		*index=3;
		if(*src=='"')
			return i;
        j=char_to_num(*src);
        data=((i*0x0a)+j);

    }
    else
    {
       data = i;
	   *index=2;
    }
	return data;
}


void BT_singleWaveCfg(uint8_t freq, uint8_t powerlevel)
{
	uint32_t val = 0;

	val |= freq;
    val |= ((powerlevel+12) << 7);
	if(XVR_REG24 == val)
    {
       XVR_REG24 = val;
    }
    else
    {
        XVR_REG25 &= 0xFFF00000 ;  // Clear XVR Test    //clear [19:0] 
        BT_delayUs(10);
        XVR_REG24 = val; 
    }
}


	   
void rf_test_ready(void)
{
	if(uart_rx_done && uart_rx_index > 17)
	{
		//the open and close Cmd should be ignored.
		if(uart_rx_buf[0] == 'C' && uart_rx_buf[uart_rx_index-1] == '"' &&  uart_rx_buf[uart_rx_index-2] == '"'&& 
			(uart_rx_buf[uart_rx_index-3] == 'N' || uart_rx_buf[uart_rx_index-3] == 'E'))
		{
			if(strncmp((const void*)&uart_rx_buf,SINGEWAVE_OPEN,sizeof(SINGEWAVE_OPEN)-1) ==0)
			{
				UART_PRINTF("Singewave Open\r\n");
			}
			else
			{
				UART_PRINTF("Singewave Close\r\n");
			}
			uart_rx_index = 0;
			uart_rx_done = 0;
			memset(uart_rx_buf, 0x00, UART0_RX_FIFO_MAX_COUNT);
		}
	}
	if(uart_rx_done && uart_rx_index > 27)
	{ 
		uart_rx_done = 0;

		//UART_PRINTF("uart_rx_index = %d\r\n", uart_rx_index);
		//UART_PRINTF("UART recv Cmd\r\n");
		if(uart_rx_buf[uart_rx_index-1] == '"' && uart_rx_buf[0] == 'C')
		{
			uart_rx_index = 0;

			//UART_PRINTF("UART Cmd Head OK\r\n");
			if(strncmp((const void*)&uart_rx_buf,SINGEWAVE_CONFIG,sizeof(SINGEWAVE_CONFIG)-1) ==0)
			{
				//UART_PRINTF("UART Config Cmd\r\n");
				temp=sizeof(SINGEWAVE_CONFIG)-1;

				// d_mode pn9 or singal
				d_mode = fCheckValue(&uart_rx_buf[temp],&index);
				// freq
				temp += index;
				//UART_PRINTF("t=%d,%x\r\n",temp,d_mode);
				freq = fCheckValue(&uart_rx_buf[temp],&index);
				// powerlevel
				temp += index;
				//UART_PRINTF("t=%d,%x\r\n",temp,freq);
				powerlevel = fCheckValue(&uart_rx_buf[temp],&index);
				// p_mode 0 :rx mode
				temp += index;
				//UART_PRINTF("t=%d,%x\r\n",temp,powerlevel);
				p_mode = fCheckValue(&uart_rx_buf[temp],&index);
				// hopping
				temp += index;
				//UART_PRINTF("t=%d,%x\r\n",temp,p_mode);
				hopping = fCheckValue(&uart_rx_buf[temp],&index);
				temp += index;
				//UART_PRINTF("t=%d,%x\r\n",temp,hopping);

				if(freq % 2)
	            {
	                ch_status = 0x01;
	                d_mode=3;
	                UART_PRINTF("Invalid commmand.\n");
	                UART_PRINTF("Please select double channel.\n");	                
	            }
				else
				{
					ch_status = 0x02;
					if(0 == p_mode)
					{
						//UART_PRINTF("d_mode=%x, freq=%x, powerlevel=%x, p_mode=%x\n, hopping=%x\r\n",d_mode,freq,powerlevel,p_mode,hopping);
					  	UART_PRINTF("RX mode!\n");
					}
					else if(1 == d_mode)  //PN9 
					{
						if (1 == p_mode)     //PN9_DH1;
						{
						  	//UART_PRINTF("d_mode=%x, freq=%x, powerlevel=%x, p_mode=%x\n, hopping=%x\r\n",d_mode,freq,powerlevel,p_mode,hopping);
						 	UART_PRINTF("PN9 DH1 mode!\n");
						}
						else if(2 == p_mode) //PN9_DH3;
						{
						  	//UART_PRINTF("d_mode=%x, freq=%x, powerlevel=%x, p_mode=%x\n, hopping=%x\r\n",d_mode,freq,powerlevel,p_mode,hopping);
						 	UART_PRINTF("PN9 DH3 mode!\n");
						}
						else if(3 == p_mode)  //PN9_DH5;
						{
						  	//UART_PRINTF("d_mode=%x, freq=%x, powerlevel=%x, p_mode=%x\n, hopping=%x\r\n",d_mode,freq,powerlevel,p_mode,hopping);
						 	UART_PRINTF("PN9 DH5 mode!\n");
						}
					}
					else if(0 == d_mode)  //ЕЅди
					{
					  	//UART_PRINTF("d_mode=%x, freq=%x, powerlevel=%x, p_mode=%x\n, hopping=%x\r\n",d_mode,freq,powerlevel,p_mode,hopping);
					  	UART_PRINTF("Singlewave mode!\n");
					}
				}
				
			}
		}
		else
		{
			UART_PRINTF("UART Cmd Error\r\n");
		}
		//Clear UART Rx buffer
		uart_rx_index = 0;
		memset(uart_rx_buf, 0x00, UART0_RX_FIFO_MAX_COUNT);
	}
	if(ch_status != 0x01)
	{
		if(0 == p_mode)
		{
			XVR_REG25 &= ~((0x1 << 11)|(0x1 << 12)|(0x1 << 13));
			BT_delayUs(625);
			XVR_REG25 |= (0x1<<10);
		}
		else if(1 == d_mode)  //PN9 
		{
			if (1 == p_mode)     //PN9_DH1;
			{
				if(hopping)
				{
					freq_value = RF_CHANNEL_TABLE[freq];
					BT_singleWaveCfg(freq_value, powerlevel);
					freq++;
					freq %= 79;	
				}
				else
				{
					BT_singleWaveCfg(freq,powerlevel);
				}
				XVR_REG25 |= (0x1<<11) | (0x1<<12) | (0x1<<13) ; //set [11]bit:PN9 send mode;set [12]:Radio in TX mode
				BT_delayUs(420); //48M,210 --> 170us ; 518 --> 420us
				XVR_REG25 &= 0xFFF00000;
				BT_delayUs(625); //48M,210 --> 170us  ;772 --> 625us   
			}
			else if(2 == p_mode) //PN9_DH3;
			{
				if(hopping)
            	{
               		freq_value = RF_CHANNEL_TABLE[freq];
               		BT_singleWaveCfg(freq_value,powerlevel);
	      			freq++;
	      			freq %= 79;
				}
				else
				{
					BT_singleWaveCfg(freq,powerlevel);
				}
				XVR_REG25 |= (0x1<<11) | (0x1<<12) | (0x1<<13)  ; //set [11]bit:PN9 send mode;set [12]:Radio in TX mode
				BT_delayUs(1670); //48M,210 --> 170us;2063 -->  1.67ms
				XVR_REG25 &= 0xFFF00000 ;  // Clear XVR Test  
				BT_delayUs(625); //48M,210 --> 170us  ;772 --> 625us 
			}
			else if(3 == p_mode)  //PN9_DH5;
			{
				if(hopping)
				{
				    freq_value = RF_CHANNEL_TABLE[freq];
				    BT_singleWaveCfg(freq_value,powerlevel);
					freq++;
					freq %= 79;
				}
				else
				{   
					BT_singleWaveCfg(freq,powerlevel);
				}
				XVR_REG25 |= (0x1<<11)  |  (0x1<<12) | (0x1<<13); //set [11]bit:PN9 send mode;set [12]:Radio in TX mode;set [9]bit :PN9 hold enable 
				BT_delayUs(2900); //48M,210 --> 170us;3582 --> 2.9ms
				XVR_REG25 &= 0xFFF00000 ;  // Clear XVR Test 
				BT_delayUs(625); //48M,210 --> 170us  ;772 --> 625us  
			}
		}
		else if(0 == d_mode)  //ЕЅди
		{
			if(hopping)
			{
				freq_value = RF_CHANNEL_TABLE[freq];
				BT_singleWaveCfg(freq_value,powerlevel);
				freq++;
				freq %= 79;
			}
			else
			{   
				BT_singleWaveCfg(freq,powerlevel);
			}
			BT_delayUs(625);  //625us
			XVR_REG25 |= (0x1<<12) |(0x1<<13);
			BT_delayUs(625);  //625us
		}
	}
}






