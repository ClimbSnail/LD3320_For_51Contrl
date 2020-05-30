/******************** 乐声电子 ***************
* File Name          : usart.h
* Author             : http://yuesheng001.taobao.com/
* Version						 : zds0.0.3
* Date First Issued  : 10/12/2011
* Description        : public header file
*******************************************************/
#ifndef __USART_H
#define __USART_H


void UartIni(void);//串口初始化
void UARTSendByte(uint8_t DAT);	//串口发送一字节数据
void PrintCom(uint8_t *DAT); //打印串口字符串数据


#endif
