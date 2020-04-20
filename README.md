# ESP8266_Pass-through
**透传程序**  
基于ESP8266 SDK version:3.1.0-dev(1ee4eb1)     
AP热点SSID： PVDF_AP；Password：12345678；   
Based on TCP server  
数据经串口输入与ESP8266相连接的client设备一对一透传。  
*Tips:   
1.编译时要注意载Github repository上的程序要以Project from Gits的方式导入,否则文本中会出现原本应隐藏字符;  
由于AiThinkerIDE_V1.0编译器改动,  
2.编译器选择Gross GCC;   
3.properties中Build command设置为make COMPILE=gcc BOOT=none APP=0 SPI_SPEED=40 SPI_MODE=DIO SPI_SIZE_MAP=2否则会编译出错;*
