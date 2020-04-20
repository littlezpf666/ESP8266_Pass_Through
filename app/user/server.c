/*
 * server.c
 *
 *  Created on: 2019年6月12日
 *      Author: pengfeizhao
 */
#include "server.h"

struct espconn user_tcp_conn;
void ICACHE_FLASH_ATTR server_recvcb(void*arg, char*pdata, unsigned short len) {
	struct espconn *pesp_conn = arg;
	os_printf("%s", pdata);
	/*
	 * espconn_send(pesp_conn, pdata, os_strlen(pdata));//回传
	 * os_printf("Recv：remote_port:%d,remote_IP:"IPSTR"\r\n", pesp_conn->proto.tcp->remote_port
	 ,IP2STR(pesp_conn->proto.tcp->remote_ip));
	 */
}
/*TCP发送回调函数，
 * 一般情况，请在前一包数据发送成功，进入 espconn_sent_callback 后，再调用 espconn_send 发送下一包数据*/
void ICACHE_FLASH_ATTR server_sentcb(void*arg) {
	os_printf("发送成功\r\n");
}
/*TCP断开回调函数，
 * WiFi的断开似乎并不会触发该回调*/
void ICACHE_FLASH_ATTR server_disconcb(void*arg) {

	os_printf("断开连接\r\n");
}
/*TCP连接回调函数*/
void ICACHE_FLASH_ATTR server_listen(void *arg) {
	struct espconn *pesp_conn = arg;

	os_printf("\r\nListen:remote_port:%d,remote_IP:"IPSTR"\r\n",
			pesp_conn->proto.tcp->remote_port
			,IP2STR(pesp_conn->proto.tcp->remote_ip));
	espconn_regist_recvcb((struct espconn*) arg, server_recvcb);
	espconn_regist_sentcb((struct espconn*) arg, server_sentcb);
	espconn_regist_disconcb((struct espconn*) arg, server_disconcb);
}
void ICACHE_FLASH_ATTR server_reconn(void *arg, sint8 err) {
	os_printf("连接错误，错误代码为%d\r\n", err);
}
void ICACHE_FLASH_ATTR server_init(int port) {

	//espconn参数配置
	user_tcp_conn.type = ESPCONN_TCP;
	user_tcp_conn.state = ESPCONN_NONE;
	user_tcp_conn.proto.tcp = (esp_tcp *) os_zalloc(sizeof(esp_tcp));
	user_tcp_conn.proto.tcp->local_port = port;

	//注册连接和重连接回调函数
	espconn_regist_connectcb(&user_tcp_conn, server_listen);
	espconn_regist_reconcb(&user_tcp_conn, server_reconn);

	//启动连接
	espconn_accept(&user_tcp_conn);
	espconn_regist_time(&user_tcp_conn, 0, 0); //超时断开连接时间，这个必须要设置否则会默认10s左右自动断开，写0永久连接

}

