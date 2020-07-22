
#include "bim_flash.h"
#include <string.h>
#include "BK3432_reg.h"


/// Specific to EMBED BEKEN FLASH  (type 1)
#define FLASH_SECT_SIZE1         0x00000200  // 512 Byte
#define FLASH_SECT_SIZE2         0x00001000  // 4 KByte
#define FLASH_BOUNDARY256B       0x00000100  // 256 Byte

/// Flash environment structure
struct flash_env_tag
{
    uint32_t    length[2];
    uint8_t     space_type[2];
};

static struct flash_env_tag bim_flash_env;

static void bim_flash_unlock(void)
{
    // Unlock all sectors
    REG_AHB3_MFC_WP0 = 0xA5;
    REG_AHB3_MFC_WP1 = 0xC3;
}
 
static void bim_flash_writeKey(void)
{
    // write control key
    REG_AHB3_MFC_KEYWORD = 0x58A9;
    REG_AHB3_MFC_KEYWORD = 0xA958;
}

static void bim_flash_clearKey(void)
{
    // write control key
    REG_AHB3_MFC_KEYWORD = 0x0;
    REG_AHB3_MFC_KEYWORD = 0x0;
}
 
static void bim_flash_start(void)
{
    REG_AHB3_MFC_CONTROL |= 0x01;
}

static uint8_t bim_flash_status(void)
{
    return REG_AHB3_MFC_CONTROL & 0x01;
}

 void bim_flash_init(void)
 {
     // Init flash environment    
     bim_flash_env.length[0]    =  FLASH_MAIN_SIZE;
     bim_flash_env.space_type[0]  = FLASH_SPACE_TYPE_MAIN; 
     
     bim_flash_env.length[1]    =  FLASH_NVR_SIZE;
     bim_flash_env.space_type[1]  = FLASH_SPACE_TYPE_NVR;
  
 }
 

 uint32_t bim_flash_readword(uint8_t type,uint32_t address)
{
    uint32_t reg;
    uint32_t data;
    

    REG_AHB3_MFC_ADDR = address;

    bim_flash_writeKey(); // write key
    bim_flash_unlock(); //wipe off WP

    reg = REG_AHB3_MFC_CONTROL;
    reg &= ~((0x07 << 2) | (0x03 << 5)); //clear control mode and space
    reg  |= ( MFC_CONTROL_READ | (type << 5)); // set write mode and control space
    REG_AHB3_MFC_CONTROL = reg;

    bim_flash_start();

    while(bim_flash_status() == 1);

    data = REG_AHB3_MFC_DATA;
    bim_flash_clearKey();
    return data;
}

 void bim_flash_writeword(uint8_t type,uint32_t address, uint32_t data)
{
    uint32_t reg;
   
    REG_AHB3_MFC_ADDR = address;
    /* Write the data                                                                   */
    REG_AHB3_MFC_DATA = data;
    bim_flash_writeKey(); // write key
    bim_flash_unlock(); //wipe off WP
    reg = REG_AHB3_MFC_CONTROL;
    reg &= ~((0x07 << 2) | (0x03 << 5)); //clear control mode and space
    reg  |= ( MFC_CONTROL_WRITE | (type << 5)); // set write mode and control space
    REG_AHB3_MFC_CONTROL = reg;
    bim_flash_start();
    while(bim_flash_status()) ;
    bim_flash_clearKey();
	
}

void bim_flash_erase_sector(uint8_t type,uint32_t address)
{

    uint32_t reg;

    REG_AHB3_MFC_ADDR = address;

    bim_flash_writeKey(); // write key
    bim_flash_unlock(); //wipe off WP

    reg = REG_AHB3_MFC_CONTROL;
    reg &= ~((0x07 << 2) | (0x03 << 5)); //clear control mode and space
    reg  |= ( MFC_CONTROL_SECT_ERASE | (type << 5)); // set write mode and control space
    REG_AHB3_MFC_CONTROL = reg;

    bim_flash_start();

    while(bim_flash_status());
    bim_flash_clearKey();
}


void bim_flash_erase(uint8_t flash_type, uint32_t address, uint32_t size)
{
    uint32_t end_address;
    end_address = address + size;

    // Check parameters
    if((flash_type > FLASH_SPACE_TYPE_NVR))
    {		
        return ;
    }

    address = address & ~(FLASH_SECT_SIZE1 - 1);
    while(address < end_address)
    {
        bim_flash_erase_sector(flash_type,address);
        
       /* Calculate next sector address (below 0x000100) */
        address += FLASH_SECT_SIZE1/4;
    }

    return;
}

void bim_flash_write(uint8_t flash_space, uint32_t address, uint32_t length, uint8_t *buffer)
{
    uint32_t data = 0xffffffff;
    uint8_t  cnt;
   
    // Check parameters
    if ( (flash_space > FLASH_SPACE_TYPE_NVR) || (( length) > bim_flash_env.length[flash_space]) )
    {
        return ;
    }

    while(length >= 4)
    {
        data = (buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24)); 
        bim_flash_writeword(flash_space,address , data);

        address += 1;
        buffer += 4;
        length -= 4;
    }

    if(length != 0)
    { /* write last mutil byte */
        data = 0xffffffff;
        for(cnt = 0; cnt < length;cnt++)
        {
            data &= ~(0xff << (8 * cnt));
            data |= (buffer[cnt] << (8 * cnt )) ;
        }
        bim_flash_writeword(flash_space,address, data);
    }

    return ;
}

void bim_flash_read(uint8_t flash_space, uint32_t address, uint32_t length, uint8_t *buffer)
{
    uint32_t data = 0xffffffff;
    uint8_t cnt;
    
    // Check parameters
    if (  length > bim_flash_env.length[flash_space] )
    {        
        return ;
    }

    if ( flash_space > FLASH_SPACE_TYPE_NVR )
    {        
        return ;
    }


    while(length >= 4)
    {
        data = bim_flash_readword(flash_space,address);	
        memcpy(buffer, &data, 4);
		address += 1;
        buffer += 4;
        length -= 4;
    }

    if(length != 0)
    {
        /* read last byte */
        data = bim_flash_readword(flash_space,address);

        for(cnt = 0; cnt < length;cnt++)
        {
            *buffer++ = ( data >> (cnt  * 8) & 0xFF );
        }
    }

    return ;
}

void bim_wdt_disable(void)
{
    REG_AHB0_ICU_WDTCLKCON = 0x1 ; // Step1. WDT clock enable
    REG_APB0_WDT_CFG = 0x005A0000;// Step3. Feed dog. Write WDT key: 0x5A firstly and 0xA5 secondly.
    REG_APB0_WDT_CFG = 0x00A50000;
}
void bim_wdt_enable(uint16_t wdt_cnt)
{
    //the timer is 48uS @1 CLK
    //0xffff: 3s
    REG_AHB0_ICU_WDTCLKCON = 0x0 ; // Step1. WDT clock enable,16M
    REG_APB0_WDT_CFG  = wdt_cnt;   // Step2. Set WDT period=0xFF
    // Do two things together: 1. Set WDT period. 2. Write WDT key to feed dog.
    // Write WDT key: 0x5A firstly and 0xA5 secondly.
    REG_APB0_WDT_CFG = (0x5A<<16) + wdt_cnt;
    REG_APB0_WDT_CFG = (0xA5<<16) + wdt_cnt;
}





