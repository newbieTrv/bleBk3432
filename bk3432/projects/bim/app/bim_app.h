#ifndef _BIM_APP_H_
#define _BIM_APP_H_

#include <stdint.h>

enum
{
	ICU_CLK_16M,
       ICU_CLK_32M,
	ICU_CLK_64M,

};

void bim_icu_init(uint8_t clk);

#endif
