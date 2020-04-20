/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */


#include "includes.h"
#include "driver/uart.h"
#include "spi_flash.h"
#include "server.h"

#define Project "transparent transmission"


#if ((SPI_FLASH_SIZE_MAP == 0) || (SPI_FLASH_SIZE_MAP == 1))
#error "The flash map is not supported"
#elif (SPI_FLASH_SIZE_MAP == 2)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0xfb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0xfc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0xfd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR           0x7c000
#elif (SPI_FLASH_SIZE_MAP == 3)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x1fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR           0x7c000
#elif (SPI_FLASH_SIZE_MAP == 4)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR           0x7c000
#elif (SPI_FLASH_SIZE_MAP == 5)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x1fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR           0xfc000
#elif (SPI_FLASH_SIZE_MAP == 6)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR           0xfc000
#else
#error "The flash map is not supported"
#endif

#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM                SYSTEM_PARTITION_CUSTOMER_BEGIN

uint32 priv_param_start_sec;

static const partition_item_t at_partition_table[] = {
    { SYSTEM_PARTITION_BOOTLOADER, 						0x0, 												0x1000},
    { SYSTEM_PARTITION_OTA_1,   						0x1000, 											SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_OTA_2,   						SYSTEM_PARTITION_OTA_2_ADDR, 						SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_RF_CAL,  						SYSTEM_PARTITION_RF_CAL_ADDR, 						0x1000},
    { SYSTEM_PARTITION_PHY_DATA, 						SYSTEM_PARTITION_PHY_DATA_ADDR, 					0x1000},
    { SYSTEM_PARTITION_SYSTEM_PARAMETER, 				SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 			0x3000},
    { SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM,             SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR,          0x1000},
};

void ICACHE_FLASH_ATTR user_pre_init(void)
{
    if(!system_partition_table_regist(at_partition_table, sizeof(at_partition_table)/sizeof(at_partition_table[0]),SPI_FLASH_SIZE_MAP)) {
		os_printf("system_partition_table_regist fail\r\n");
		while(1);
	}
}
ETSTimer station_check;
/***************************接收到station的连接每2s进入一次**********************************/
void ICACHE_FLASH_ATTR check_station(void *arg){
	struct	ip_info	ap_ip;
	struct station_info*st_info;
	os_timer_disarm(&station_check);
	//获取ESP8266连接的station的IP
	st_info=wifi_softap_get_station_info();
	if(st_info){
		/*
		//获取ESP8266本机IP
		wifi_get_ip_info(STATION_IF,&ap_ip);
				os_printf("IP:"IPSTR"\n\r",IP2STR(&ap_ip.ip));
		//my_station_init(&(st_info->ip),&ap_ip.ip,8888);//注意第一个参数的赋值
		os_printf("IP:"IPSTR"",IP2STR(&ap_ip.ip));*/
		server_init(TCP_PORT);
		os_printf("TCP initialization success\n\r");
		os_free(st_info);
	}
	else{
		os_timer_arm(&station_check,2000,NULL);
	}

}
/*************************WiFi连接事件回调函数***********************************/
void ICACHE_FLASH_ATTR AP_be_connected_cb(System_Event_t *evt){

	switch(evt->event){
	case EVENT_SOFTAPMODE_STACONNECTED:
		os_timer_disarm(&station_check);
		/*对于同一个 timer，os_timer_arm 或 os_timer_arm_us 不能重复调用，必须先 os_timer_disarm。
		 *os_timer_setfn 必须在 timer 未使能的情况下调用，在 os_timer_arm 或 os_timer_arm_us之前或者 os_timer_disarm之后。*/
		os_timer_setfn(&station_check,check_station,NULL);
		os_timer_arm(&station_check,2000,NULL);
		break;
	default:
		break;
	}
}
void to_scan(void) {
	struct	ip_info	info;
	server_init(TCP_PORT);
	os_printf("First TCP initialization success\n\r");

	wifi_set_event_handler_cb(AP_be_connected_cb);
}
u16 N_Data_FLASH_SEC=0x77;
/*字符串必须存在以char生成的数组*/
u8 A_W_Data[16]="0123456789";
u32 A_R_Data[16]={0};//缓存读取Flash的数据
/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_init(void)
{
	struct softap_config apConfig;

	uart_init(115200,115200);
	os_printf("\r\n--------------------------------\r\n");
	os_printf("SDK version:%s", system_get_sdk_version());//串口打印
	os_printf("\r\n--------------------------------\r\n");

	spi_flash_erase_sector(0x77);//写入前要对相应扇区进行擦除
	spi_flash_write(0x77*4096,(uint32*)A_W_Data,sizeof(A_W_Data));
	os_printf("\r\n----------Write Flash Data OVER-----------\r\n");

	spi_flash_read(0x77*4096,(uint32*)A_R_Data,sizeof(A_R_Data));
	os_printf("Read Data=%s",A_R_Data);
	os_printf("\r\n-------------Read Flash OVER--------------\r\n");

	wifi_set_opmode_current(STATIONAP_MODE);
	os_strcpy(apConfig.ssid, "PVDF_AP");
	apConfig.ssid_len = strlen("PVDF_AP");
	os_strcpy(apConfig.password,"12345678");
	apConfig.authmode = AUTH_WPA_WPA2_PSK;
	apConfig.max_connection =4;//最大连接数量，最大支持四个，默认四个
	apConfig.beacon_interval = 100;//信标间隔，默认为100
	apConfig.channel = 1;//信道，共支持1~13个信道
	apConfig.ssid_hidden = 0;//隐藏SSID，0：不隐藏  1：隐藏
	wifi_softap_set_config(&apConfig);


	system_init_done_cb(to_scan);
}

