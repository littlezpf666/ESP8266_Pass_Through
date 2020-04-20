/*
 * server.h
 *
 *  Created on: 2019Äê6ÔÂ12ÈÕ
 *      Author: pengfeizhao
 */

#ifndef APP_USER_SERVER_H_
#define APP_USER_SERVER_H_
#include "includes.h"
#include "espconn.h"
#define TCP_PORT 888
struct command
{
	char MSgId;

	char opmode;
	char* device_ip;
	char GET_SSID_INFO;
	char MsgObj[9];
	char CON_STATUS;
	char ssid[32];
	char password[64];
};
void  ICACHE_FLASH_ATTR server_init(int port);

#endif /* APP_USER_SERVER_H_ */
