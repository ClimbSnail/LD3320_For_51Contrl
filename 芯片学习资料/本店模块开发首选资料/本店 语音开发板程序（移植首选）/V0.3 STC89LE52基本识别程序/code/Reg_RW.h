/************************************************************************************/
//	版权所有：Copyright (c) 2005 - 2010 ICRoute INC.
/************************************************************************************/

#ifndef REG_RW_H
#define REG_RW_H

//读写模式选择，开启定义
#define HARD_PARA_PORT
//#define SOFT_PARA_PORT	//	软件模拟并行读写
//#define SOFT_SPI_PORT		//	软件模拟SPI读写



//驱动端口定义
sbit	LD_MODE	=P3^5; /*读写模式选择*/
sbit RSTB=P3^3;	 /*复位端口*/
sbit CSB=P2^1;	 /*模块片选端口*/


//函数声明
void LD_WriteReg( unsigned char address, unsigned char dataout );
unsigned char LD_ReadReg( unsigned char address );


#endif
