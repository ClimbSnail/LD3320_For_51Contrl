/******************** 乐声电子 ***************
* File Name          : usart.c
* Author             : http://yuesheng001.taobao.com/
* Version						 : zds0.0.3
* Date First Issued  : 10/12/2011
* Description        : 串口API
											CPU: STC89LE52
											晶振：22.1184MHZ
											波特率：9600 bit/S	
*******************************************************/
#include "config.h"

/************************************************************************
函 数 名： 串口初始化
功能描述： STC10L08XE 单片机串口初始化函数
返回函数： none
其他说明： none
**************************************************************************/
void UartIni(void)
{
	
	SCON = 0x50 ; //SCON: serail mode 1, 8-bit UART, enable ucvr  
	
	TMOD |= 0x20 ; //TMOD: timer 1, mode 2, 8-bit reload 
	
	PCON |= 0x80 ; //SMOD=1; 
	
	TH1 = 0xF4 ; //Baud:9600 fosc=22.1148MHz 
	
	IE |= 0x90 ; //Enable Serial Interrupt 
	
	TR1 = 1 ; // timer 1 run 
	ES=1;
	EA=1;
}
/************************************************************************
功能描述： 	串口发送一字节数据
入口参数：	DAT:带发送的数据
返 回 值： 	none
其他说明：	none
**************************************************************************/
void UARTSendByte(uint8_t DAT)
{
	ES  =  0;
	TI=0;
	SBUF = DAT;
	while(TI==0);
	TI=0;
	ES = 1;
}

/************************************************************************
功能描述： 串口发送字符串数据
入口参数： 	*DAT：字符串指针
返 回 值： none
其他说明： API 供外部使用，直观！
**************************************************************************/
void PrintCom(uint8_t *DAT)
{
	while(*DAT)
	{
	 	UARTSendByte(*DAT++);
	}	
}

