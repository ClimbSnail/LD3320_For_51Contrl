/************************************************************************************
**	CPU: STC89LE52
**	晶振：22.1184MHZ
**	波特率：9600 bit/S
**	配套产品信息：LD3320 非特定语音模块 开发板
**                http://yuesheng001.taobao.com/
**  版本：zds0.0.3
**  修改日期：2011.12.10
/************************************************************************************/
#include "config.h"


#ifdef HARD_PARA_PORT

		#define LD_INDEX_PORT		(*((volatile unsigned char xdata*)(0x8100))) 
		#define LD_DATA_PORT		(*((volatile unsigned char xdata*)(0x8000))) 
		
		//评估板上 MCU的A8 连接到 LD芯片的AD
		//         MCU的A14 连接到 LD芯片的CSB
		//         MCU的RD、WR 连接 LD芯片的RD、WR (xdata 读写时自动产生低信号)
		//
		//0x8100的二进制是10000001 00000000		CSB=0 AD=1
		//                 ^     ^
		//0x8000的二进制是10000000 00000000		CSB=0 AD=0
		//                 ^     ^
		
		void LD_WriteReg( unsigned char address, unsigned char dataout )
		{
			LD_INDEX_PORT  = address;         
			LD_DATA_PORT = dataout;          
		}
		
		unsigned char LD_ReadReg( unsigned char address )
		{
			LD_INDEX_PORT = address;         
			return (unsigned char)LD_DATA_PORT;     
		}
#endif


//软件模拟并行方式读写
#ifdef SOFT_PARA_PORT

		#define DELAY_NOP	_nop_();_nop_();_nop_();_nop_(); _nop_(); 

		sbit	LD_WR = P3^6;
		sbit	LD_RD	= P3^7;
		sbit	LD_CS	= P2^1;
		sbit	LD_A0	= P2^0;
	void LD_WriteReg( unsigned char address, unsigned char dataout )
		{
			P0 = address;
			LD_A0 = 1;
			LD_CS = 0;
			LD_WR = 0;
			DELAY_NOP;
		
			LD_WR = 1;
			LD_CS = 1;
			DELAY_NOP;
		
			P0 = dataout;
			LD_A0 = 0;
			LD_CS = 0;
			LD_WR = 0;
			DELAY_NOP;
		
			LD_WR = 1;
			LD_CS = 1;
			DELAY_NOP;
		}
		
		unsigned char LD_ReadReg( unsigned char address )
		{
			unsigned char datain;
		
			P0 = address;
			LD_A0 = 1;
			LD_CS = 0;
			LD_WR = 0;
			DELAY_NOP;
		
			LD_WR = 1;
			LD_CS = 1;
			DELAY_NOP;
		
			LD_A0 = 0;
			LD_CS = 0;
			LD_RD = 0;
			DELAY_NOP;

			datain = P0;
			LD_RD = 1;
			LD_CS = 1;
			DELAY_NOP;
		
			return datain;
		}
#endif


//软件模拟SPI方式读写
#ifdef SOFT_SPI_PORT
		#define DELAY_NOP	_nop_();_nop_();_nop_();
		
		sbit SCS=P2^1;    //芯片片选信号
		sbit SDCK=P0^2;   //SPI 时钟信号
		sbit SDI=P0^0;    //SPI 数据输入
		sbit SDO=P0^1;    //SPI 数据输出
		sbit SPIS=P3^5;   //SPI模式设置：低有效。

		void LD_WriteReg(unsigned char address,unsigned char dataout)
		{
			unsigned char i = 0;
			unsigned char command=0x04;
			SPIS =0;
			SCS = 0;
			DELAY_NOP;
		
			//write command
			for (i=0;i < 8; i++)
			{
				if ((command & 0x80) == 0x80) 
					SDI = 1;
				else
					SDI = 0;
				
				DELAY_NOP;
				SDCK = 0;
				command = (command << 1);  
				DELAY_NOP;
				SDCK = 1;  
			}
			//write address
			for (i=0;i < 8; i++)
			{
				if ((address & 0x80) == 0x80) 
					SDI = 1;
				else
					SDI = 0;
				DELAY_NOP;
				SDCK = 0;
				address = (address << 1); 
				DELAY_NOP;
				SDCK = 1;  
			}
			//write data
			for (i=0;i < 8; i++)
			{
				if ((dataout & 0x80) == 0x80) 
					SDI = 1;
				else
					SDI = 0;
				DELAY_NOP;
				SDCK = 0;
				dataout = (dataout << 1); 
				DELAY_NOP;
				SDCK = 1;  
			}
			DELAY_NOP;
			SCS = 1;
		}

		unsigned char LD_ReadReg(unsigned char address)
		{
			unsigned char i = 0; 
			unsigned char datain =0 ;
			unsigned char temp = 0; 
			unsigned char command=0x05;
			SPIS =0;
			SCS = 0;
			DELAY_NOP;
		
			//write command
			for (i=0;i < 8; i++)
			{
				if ((command & 0x80) == 0x80) 
					SDI = 1;
				else
					SDI = 0;
				DELAY_NOP;
				SDCK = 0;
				command = (command << 1);  
				DELAY_NOP;
				SDCK = 1;  
			}

			//write address
			for (i=0;i < 8; i++)
			{
				if ((address & 0x80) == 0x80) 
					SDI = 1;
				else
					SDI = 0;
				DELAY_NOP;
				SDCK = 0;
				address = (address << 1); 
				DELAY_NOP;
				SDCK = 1;  
			}
			DELAY_NOP;

			//Read data
			for (i=0;i < 8; i++)
			{
				datain = (datain << 1);
				temp = SDO;
				DELAY_NOP;
				SDCK = 0;  
				if (temp == 1)  
					datain |= 0x01; 
				DELAY_NOP;
				SDCK = 1;  
			}
		
			DELAY_NOP;
			SCS = 1;
			return datain;
		}

#endif
