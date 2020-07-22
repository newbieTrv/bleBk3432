
#ifndef _BIM_FLASH_H_
#define _BIM_FLASH_H_

#include <stdint.h>
#include "BK3432_reg.h"


enum 
{
	FLASH_SPACE_TYPE_MAIN = 0x00,
	FLASH_SPACE_TYPE_NVR = 0x01,
	FLASH_SPACE_TYPE_INVALID = 0XFF,
};


#define FLASH_TYPE_EMBEN        FLASH_SPACE_TYPE_NVR
#define FLASH_MAIN_SIZE              0x00028000
#define FLASH_NVR_SIZE               0x00000200
#define FLASH_WRITE_ECC_DATA0 0XA3
#define FLASH_WRITE_ECC_DATA1 0X56


void bim_flash_init(void);
void bim_flash_erase_sector(uint8_t type,uint32_t address);
void bim_flash_erase(uint8_t flash_type, uint32_t address, uint32_t size);
void bim_flash_read(uint8_t flash_space, uint32_t address, uint32_t length, uint8_t * buffer);
void bim_flash_write(uint8_t flash_space, uint32_t address, uint32_t length, uint8_t * buffer);
void bim_wdt_enable(uint16_t wdt_cnt);
void bim_wdt_disable(void);

#endif





