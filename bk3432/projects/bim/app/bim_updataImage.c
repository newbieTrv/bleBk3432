

#include "bim_updataImage.h"
#include "bim_flash.h"
#include "bim_uart.h"

#include <string.h>


img_hdr_t hdr_img;
img_hdr_t hdr_back;


uint32_t bim_get_psec_image_header(void)
{
	uint32_t sec_image_oad_header_fddr;
	
	if(hdr_back.uid == OAD_APP_PART_UID)
	{
		sec_image_oad_header_fddr = SEC_IMAGE_OAD_HEADER_APP_FADDR;
	}
	else if(hdr_back.uid == OAD_APP_STACK_UID)
	{
		sec_image_oad_header_fddr = SEC_IMAGE_OAD_HEADER_STACK_FADDR;
	}
    bim_printf("sec_image_oad_header_fddr = ", sec_image_oad_header_fddr,16 );
    bim_uart_write("\r\n");
    bim_flash_read(FLASH_SPACE_TYPE_MAIN, sec_image_oad_header_fddr/4, sizeof(img_hdr_t), (uint8_t *)&hdr_img);

	//bim_printf("hdr_img.crc = ",hdr_img.crc,16);
    //bim_uart_write("\r\n");
/*	bim_printf("hdr_img.crc_status = ",hdr_img.crc_status,16);
    bim_uart_write("\r\n");
	bim_printf("hdr_img.len = ",hdr_img.len,16);
    bim_uart_write("\r\n");
	bim_printf("hdr_img.rom_ver = ",hdr_img.rom_ver,16);
    bim_uart_write("\r\n");
	bim_printf("hdr_img.sec_status = ",hdr_img.sec_status,16);
    bim_uart_write("\r\n");
	bim_printf("hdr_img.ver = ",hdr_img.ver,16);
    bim_uart_write("\r\n");
	bim_printf("hdr_img.uid = ",hdr_img.uid,16);
    bim_uart_write("\r\n");
	*/
   	return 0;
}
 
uint32_t bim_get_psec_backup_header(void)
{
	UART_PRINTF("udi_get_psec_backup_header addr = 0x%x\r\n",SEC_BACKUP_OAD_HEADER_FADDR);
    bim_flash_read(FLASH_SPACE_TYPE_MAIN, SEC_BACKUP_OAD_HEADER_FADDR/4, sizeof(img_hdr_t), (uint8_t *)&hdr_back);
	
	//bim_printf("hdr_back.crc = ",hdr_back.crc, 16);
	//bim_uart_write("\r\n");
/*	bim_printf("hdr_back.crc_status = ",hdr_back.crc_status, 16);
	bim_uart_write("\r\n");
	bim_printf("hdr_back.len = ",hdr_back.len, 16);
	bim_uart_write("\r\n");
	bim_printf("hdr_img.rom_ver = ",hdr_back.rom_ver, 16);
	bim_uart_write("\r\n");
	bim_printf("hdr_back.sec_status = ",hdr_back.sec_status, 16);
	bim_uart_write("\r\n");
	bim_printf("hdr_back.ver = ",hdr_back.ver, 16);
	bim_uart_write("\r\n");
	bim_printf("hdr_back.uid = ",hdr_back.uid, 16);
	bim_uart_write("\r\n");
	*/
	return 0;
}

int make_crc32_table(void);
uint32_t make_crc32(uint32_t crc,unsigned char *string,uint32_t size);
uint32_t calc_image_sec_crc(void)
{
	
	UART_PRINTF("%s\r\n",__func__);
	uint8_t data[BLOCK_SIZE];
	uint8_t tmp_data[BLOCK_SIZE];
	uint32_t block_total;
	uint32_t read_addr;
	uint32_t calcuCrc = 0xffffffff;
	make_crc32_table();
	block_total =  hdr_img.len / 4 - 1;// not clac HDR

	if(hdr_img.uid == OAD_APP_PART_UID)
	{
		read_addr = SEC_IMAGE_RUN_APP_FADDR;   // 108k
	}
	else if(hdr_img.uid == OAD_APP_STACK_UID)  //8k
	{
		read_addr = SEC_IMAGE_RUN_STACK_FADDR;
	}
	UART_PRINTF("read start addr = 0x%x\r\n",read_addr);
	for(uint32_t i = 0;i < block_total; i++)
	{
        bim_flash_read(FLASH_SPACE_TYPE_MAIN, read_addr/4,BLOCK_SIZE, data);
        bim_flash_read(FLASH_SPACE_TYPE_MAIN, read_addr/4,BLOCK_SIZE, tmp_data);
		calcuCrc = make_crc32(calcuCrc,data,BLOCK_SIZE);
		
		if(memcmp(data,tmp_data,BLOCK_SIZE) != 0)
		{
			UART_PRINTF("read_addr error = 0x08%x\r\n",read_addr);
			for(int a=0;a<BLOCK_SIZE;a++)
			{
				UART_PRINTF("tmp_data = %02x,data = %02x \r\n",tmp_data[a],data[a]);
			}
		}
		read_addr+= BLOCK_SIZE;
	}
	UART_PRINTF("read end addr = 0x%x,calcuCrc = 0x%08x\r\n",read_addr,calcuCrc);
	
	return calcuCrc;
}

uint32_t calc_backup_sec_crc(void)
{
	UART_PRINTF("%s\r\n",__func__);
	uint8_t data[BLOCK_SIZE];
	uint8_t tmp_data[BLOCK_SIZE];
	uint16_t block_total;
	uint32_t read_addr;
	uint32_t calcuCrc = 0xffffffff;
	make_crc32_table();
	block_total = hdr_back.len / 4 - 1;
	read_addr = SEC_BACKUP_OAD_IMAGE_FADDR;
	UART_PRINTF("read start addr = 0x%x\r\n",read_addr);
	bim_printf("block_total=", block_total, 10);
	bim_uart_write("\r\n");
	for(uint32_t i = 0;i < block_total; i++)
	{
        bim_flash_read(FLASH_SPACE_TYPE_MAIN, read_addr/4,BLOCK_SIZE, data);
        bim_flash_read(FLASH_SPACE_TYPE_MAIN, read_addr/4,BLOCK_SIZE, tmp_data);
		calcuCrc = make_crc32(calcuCrc,data,BLOCK_SIZE);
		read_addr+= BLOCK_SIZE;
		
		if(memcmp(data,tmp_data,BLOCK_SIZE) != 0)
		{
			UART_PRINTF("read_addr error = 0x08%x\r\n",read_addr);
			for(int a=0;a<BLOCK_SIZE;a++)
			{
				UART_PRINTF("tmp_data = %02x,data = %02x \r\n",tmp_data[a],data[a]);
			}
		}
	}
	UART_PRINTF("read end addr = 0x%x,calcuCrc = 0x%08x\r\n",read_addr,calcuCrc);

	bim_printf("crc=", calcuCrc, 16);
	bim_uart_write("\r\n");
	
	return calcuCrc;
}

//const uint32_t ROM_VER = 0x0005;

uint8_t bim_check_image_sec_status(void)
{	
	bim_get_psec_image_header();

	if(hdr_img.uid == OAD_APP_PART_UID)
	{
		if(CRC_UNCHECK == hdr_img.crc_status) // image not crc check and image is exist ,do crc calc
		{
			if(hdr_img.len != 0xffff && (hdr_img.len / 4)<= SEC_MAX_FSIZE_APP_BLOCK)
			{
				if(hdr_img.crc == calc_image_sec_crc()) // crc ok
				{
					bim_uart_write("check crc OK!!!\r\n");
					hdr_img.crc_status = CRC_CHECK_OK;
					hdr_img.sec_status = SECT_NORMAL;
                    bim_flash_write(FLASH_SPACE_TYPE_MAIN, SEC_IMAGE_OAD_HEADER_APP_FADDR/4, sizeof(img_hdr_t),(uint8_t *)&hdr_img);
                    bim_flash_read(FLASH_SPACE_TYPE_MAIN, SEC_IMAGE_OAD_HEADER_APP_FADDR/4,sizeof(img_hdr_t), (uint8_t *)&hdr_img);
                    bim_get_psec_image_header();				
					return SSTATUS_SECT_NORMAL;
				}
	            else
	            {
	            	bim_uart_write("check crc fail!!!\r\n");
					hdr_img.crc_status = CRC_CHECK_FAIL;
					hdr_img.sec_status = SECT_ABNORMAL;	
					bim_flash_write(FLASH_SPACE_TYPE_MAIN, SEC_IMAGE_OAD_HEADER_APP_FADDR/4, sizeof(img_hdr_t),(uint8_t *)&hdr_img);
                    bim_flash_read(FLASH_SPACE_TYPE_MAIN, SEC_IMAGE_OAD_HEADER_APP_FADDR/4,sizeof(img_hdr_t), (uint8_t *)&hdr_img);
					return SSTATUS_SECT_ABNORMAL;
				}
			}
			else if(hdr_img.rom_ver == 0xffff)
			{
				return SSTATUS_SECT_ERASED;
			}
	        else
			{
				hdr_img.crc_status = CRC_CHECK_FAIL;
				hdr_img.sec_status = SECT_ABNORMAL;
                bim_flash_write(FLASH_SPACE_TYPE_MAIN, SEC_IMAGE_OAD_HEADER_APP_FADDR/4, sizeof(img_hdr_t),(uint8_t *)&hdr_img);
                bim_flash_read(FLASH_SPACE_TYPE_MAIN, SEC_IMAGE_OAD_HEADER_APP_FADDR/4,sizeof(img_hdr_t), (uint8_t *)&hdr_img);
				return SSTATUS_SECT_ABNORMAL;
			}
			
		}    
		else if(CRC_CHECK_FAIL == hdr_img.crc_status)
		{
		  	hdr_img.crc_status = CRC_CHECK_FAIL;
			hdr_img.sec_status = SECT_ABNORMAL;	
            bim_flash_write(FLASH_SPACE_TYPE_MAIN, SEC_IMAGE_OAD_HEADER_APP_FADDR/4, sizeof(img_hdr_t),(uint8_t *)&hdr_img);
            bim_flash_read(FLASH_SPACE_TYPE_MAIN, SEC_IMAGE_OAD_HEADER_APP_FADDR/4,sizeof(img_hdr_t), (uint8_t *)&hdr_img);
			return SSTATUS_SECT_ABNORMAL;
		}
		else if(CRC_CHECK_OK == hdr_img.crc_status)
		{
		  	hdr_img.crc_status = CRC_CHECK_OK;
			hdr_img.sec_status = SECT_NORMAL;
            bim_flash_write(FLASH_SPACE_TYPE_MAIN, SEC_IMAGE_OAD_HEADER_APP_FADDR/4, sizeof(img_hdr_t),(uint8_t *)&hdr_img);
            bim_flash_read(FLASH_SPACE_TYPE_MAIN, SEC_IMAGE_OAD_HEADER_APP_FADDR/4,sizeof(img_hdr_t), (uint8_t *)&hdr_img);
			return SSTATUS_SECT_NORMAL;
		}else
		{
		  	hdr_img.crc_status = CRC_CHECK_FAIL;
			hdr_img.sec_status = SECT_ABNORMAL;
            bim_flash_write(FLASH_SPACE_TYPE_MAIN, SEC_IMAGE_OAD_HEADER_APP_FADDR/4, sizeof(img_hdr_t),(uint8_t *)&hdr_img);
            bim_flash_read(FLASH_SPACE_TYPE_MAIN, SEC_IMAGE_OAD_HEADER_APP_FADDR/4,sizeof(img_hdr_t), (uint8_t *)&hdr_img);
			return SSTATUS_SECT_ABNORMAL;
		}
	}
	else 
	{
		bim_uart_write("bim_check_image_sec_status: ERROR, UNKNOWN UID\r\n");
		return SSTATUS_SECT_UNKOWN;
	}
}

 
uint8_t bim_check_backup_sec_status(void)//NOT WRITE INFO TO FLASH
{	
	bim_get_psec_backup_header();

    if(hdr_back.rom_ver == 0xffff || hdr_back.ver == 0xffff)
	{
		return SSTATUS_SECT_ERASED;
	}
	
	if(hdr_back.uid == OAD_APP_PART_UID)
	{
		if(CRC_UNCHECK == hdr_back.crc_status) // image not crc check and image is exist ,do crc calc
		{
			if(hdr_back.len != 0xffff && (hdr_back.len / 4) <= SEC_MAX_FSIZE_APP_BLOCK)
			{
				if(hdr_back.crc == calc_backup_sec_crc()) // crc ok
				{
					bim_uart_write("crc ok\r\n");
					hdr_back.crc_status = CRC_CHECK_OK;
					hdr_back.sec_status = SECT_NORMAL;	
					return SSTATUS_SECT_NORMAL;
				}else
				{	
					bim_uart_write("crc error\r\n");
					hdr_back.crc_status = CRC_CHECK_FAIL;
					hdr_back.sec_status = SECT_ABNORMAL;
					return SSTATUS_SECT_ABNORMAL;
				}
			}
	        else
			{
				hdr_back.crc_status = CRC_CHECK_FAIL;
				hdr_back.sec_status = SECT_ABNORMAL;		
				return SSTATUS_SECT_ABNORMAL;
			}
		}
		else if(CRC_CHECK_FAIL == hdr_back.crc_status)
		{
			hdr_back.crc_status = CRC_CHECK_FAIL;
			hdr_back.sec_status = SECT_ABNORMAL;	
			return SSTATUS_SECT_ABNORMAL;
		}
		else if(CRC_CHECK_OK == hdr_back.crc_status)
		{
			hdr_back.crc_status = CRC_CHECK_OK;
			hdr_back.sec_status = SECT_NORMAL;
			return SSTATUS_SECT_NORMAL;
		}else
		{
			hdr_back.crc_status = CRC_CHECK_FAIL;
			hdr_back.sec_status = SECT_ABNORMAL;
			return SSTATUS_SECT_ABNORMAL;
		}
	}
	else 
	{
		UART_PRINTF("bim_check_backup_sec_status: ERROR, UNKNOWN UID\r\n");
		return SSTATUS_SECT_UNKOWN;
	}
}



void bim_erase_image_sec(void)  
{
	if(hdr_back.uid == OAD_APP_PART_UID)  //41k
	{
		bim_uart_write("udi_erase_image_sec \r\n");

        //bim_flash_erase(FLASH_SPACE_TYPE_MAIN, SEC_IMAGE_ALLOC_START_APP_FADDR/4, 25*1024/4);
        uint32_t earse_addr = SEC_IMAGE_ALLOC_START_APP_FADDR;
        for(uint8_t i=0; i<82; i++)
        {
            bim_flash_erase_sector(FLASH_SPACE_TYPE_MAIN, earse_addr/4);
            earse_addr += 512;
        }
        bim_uart_write("udi_erase_image_sec end\r\n");
	}
	else
	{
		UART_PRINTF("bim_erase_image_sec: ERROR, UNKNOWN UID\r\n");
	}
}



void bim_erase_backup_sec(void)
{
	bim_uart_write("udi_erase_backup_sec \r\n");
    
	//bim_flash_erase(FLASH_SPACE_TYPE_MAIN, SEC_BACKUP_ALLOC_START_FADDR/4, 26*1024/4);
    uint32_t earse_addr = SEC_BACKUP_ALLOC_START_FADDR;
    for(uint8_t i=0; i<82; i++)//
    {
        bim_flash_erase_sector(FLASH_SPACE_TYPE_MAIN, earse_addr/4);
        earse_addr += 512;
    }
    bim_uart_write("udi_erase_backup_sec end\r\n");
}



void bim_updata_backup_to_image_sec(void)
{
	bim_uart_write("udi_updata_backup_to_image_sec\r\n");
	uint8_t data[BLOCK_SIZE];
	uint32_t backup_size = hdr_back.len * 4;
	uint32_t read_end_addr = SEC_BACKUP_OAD_HEADER_FADDR + backup_size;
	uint32_t write_addr;

	if(hdr_back.uid == OAD_APP_PART_UID) // only app part
	{
		write_addr = SEC_IMAGE_OAD_HEADER_APP_FADDR;
	}
	else if(hdr_back.uid == OAD_APP_STACK_UID) //app and stack
	{
		write_addr = SEC_IMAGE_OAD_HEADER_STACK_FADDR;
	}
	bim_printf("write_addr = ",write_addr, 16);
    bim_uart_write("\r\n");
	
	for(uint32_t read_addr = SEC_BACKUP_OAD_HEADER_FADDR;read_addr < read_end_addr;)
	{
        bim_flash_read(FLASH_SPACE_TYPE_MAIN, read_addr/4,BLOCK_SIZE, data);
        bim_flash_write(FLASH_SPACE_TYPE_MAIN, write_addr/4, BLOCK_SIZE, data);
		write_addr += BLOCK_SIZE;
		read_addr += BLOCK_SIZE;
	
	}
	bim_uart_write("udi_updata_backup_to_image_sec end\r\n");
}	
     
				
uint8_t bim_select_sec(void)
{
	uint8_t bsec_status;
	uint8_t status = 0;
	bsec_status = bim_check_backup_sec_status();

	switch(bsec_status)
	{
		case SSTATUS_SECT_NORMAL: // 1:I NORMAL ,B NORMAL,updata B -> I,RUN I 
		{
			if(hdr_back.uid == OAD_APP_PART_UID)
			{
				//bim_fflash_wr_protect_128k();
			}
            bim_erase_image_sec();
            bim_updata_backup_to_image_sec();
            if(SSTATUS_SECT_NORMAL == bim_check_image_sec_status())
            {
                bim_erase_backup_sec();
                status = 1;
            }
            else
            {
                bim_wdt_enable(100);//reset
            } 
            bim_uart_write("TYPE = 1\r\n");
            //bim_fflash_wp_ALL();
		}break;
		
		case SSTATUS_SECT_ABNORMAL://://2     :I NORMAL,B ABNORMAL ,ERASE B,RUN I
		{
			//bim_fflash_wr_protect_256k();
			bim_erase_backup_sec();
			status = 1;
			bim_uart_write("TYPE = 2\r\n");
			//bim_fflash_wp_ALL();
            
		}break;
		
		case SSTATUS_SECT_ERASED://://3:I NORMAL,B ERASED,RUN I
		{
			status = 1;
			bim_uart_write("TYPE = 3\r\n");
		}break;
		
		case SSTATUS_SECT_DIFF_ROM_VER:////4:I DIFF_ROM,B ERASED,NOT HAPPEN
		{
            //bim_fflash_wr_protect_256k();
			status = 1;
			bim_erase_backup_sec();
			bim_uart_write("TYPE = 4 SSTATUS_SECT_DIFF_ROM_VER\r\n");
            //bim_fflash_wp_ALL();
		}break;
		
		default:
        {
            //bim_fflash_wr_protect_256k();
			status = 1;
			//bim_erase_backup_sec();
			bim_uart_write(" SSTATUS_SECT_UNKOWN\r\n");
            //bim_fflash_wp_ALL();
        }break;
				
	}


	return status ;

}


//uint32_t check_data_crc(uint8_t *data,uint32_t size)
//{
//    uint32_t calcuCrc = 0xffffffff;
//    calcuCrc = make_crc32(calcuCrc,data,size);
////  uart_print_int_debug(CALCUCRC_VALUE,calcuCrc);
//    return calcuCrc;
//}


