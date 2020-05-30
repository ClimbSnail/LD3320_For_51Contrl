/************************************************************************************
**	版权所有：Copyright (c) 2005 - 2010 ICRoute INC.
**	CPU: STC89LE52
**	晶振：22.1184MHZ
**	波特率：9600 bit/S
**	配套产品信息：LD3320 非特定语音模块 开发板
**                http://yuesheng001.taobao.com/
**  版本：zds0.0.3
**  修改日期：2011.12.10
**  说明：本工程只具有 语音识别功能。属基本驱动代码，
					附加串口、如不需要用到可
					屏蔽删除，根据需要自行移植到相关的单片机上。
/************************************************************************************/
#include "config.h"

/************************************************************************************/
//	nAsrStatus 用来在main主程序中表示程序运行的状态，不是LD3320芯片内部的状态寄存器
//	LD_ASR_NONE:		表示没有在作ASR识别
//	LD_ASR_RUNING：		表示LD3320正在作ASR识别中
//	LD_ASR_FOUNDOK:		表示一次识别流程结束后，有一个识别结果
//	LD_ASR_FOUNDZERO:	表示一次识别流程结束后，没有识别结果
//	LD_ASR_ERROR:		表示一次识别流程中LD3320芯片内部出现不正确的状态
/***********************************************************************************/
uint8 idata nAsrStatus=0;	
extern void   _nop_     (void); 
void MCU_init(); 
uint8 RunASR();
void ProcessInt0(); //识别处理函数
void  delay(unsigned long uldata);
/************************************************************************
功能描述： 主函数程序入口
入口参数：	none
返 回 值： 	none
其他说明：	none
**************************************************************************/
void  main()
{
	uint8 idata nAsrRes;
	uint8 i=0;

	MCU_init();
	LD_Reset();
	UartIni(); /*串口初始化*/
	nAsrStatus = LD_ASR_NONE;		//	初始状态：没有在作ASR
	PrintCom("串口运行中....\n"); /*text.....*/
	PrintCom("口令：1、代码测试\n"); /*text.....*/
	PrintCom("	2、开发板验证\n"); /*text.....*/
	PrintCom("	3、检查完毕\n"); /*text.....*/

	while(1)
	{
		switch(nAsrStatus)
		{
			case LD_ASR_RUNING:
			case LD_ASR_ERROR:		
				break;
			case LD_ASR_NONE:
			{
				nAsrStatus=LD_ASR_RUNING;
				if (RunASR()==0)	/*	启动一次ASR识别流程：ASR初始化，ASR添加关键词语，启动ASR运算*/
				{
					nAsrStatus = LD_ASR_ERROR;
				}
				break;
			}

			case LD_ASR_FOUNDOK: /*	一次ASR识别流程结束，去取ASR识别结果*/
			{
				
				nAsrRes = LD_GetResult();		/*获取结果*/
				PrintCom("\n识别码:"); /*text.....*/
        UARTSendByte(nAsrRes+0x30); /*text.....*/				
				 switch(nAsrRes)		   /*对结果执行相关操作,客户修改*/
				  {
					  case CODE_DMCS:			/*命令“测试”*/
							PrintCom("“代码测试”命令识别成功\n"); /*text.....*/
														 break;
						case CODE_KFBYZ:	 /*命令“全开”*/
							PrintCom("“开发板验证”命令识别成功\n"); /*text.....*/
														 break;
						case CODE_JCWB:		/*命令“复位”*/
				
							PrintCom("“检查完毕”命令识别成功\n"); /*text.....*/
														break;
						default:break;
					}				

				nAsrStatus = LD_ASR_NONE;
				break;
			}
			case LD_ASR_FOUNDZERO:
			default:
			{
				nAsrStatus = LD_ASR_NONE;
				break;
			}
		}// switch	 
	}// while

}
/************************************************************************
功能描述： 	单片机初始化
入口参数：
返 回 值： 
其他说明：
**************************************************************************/
void MCU_init()
{
	P0 = 0xff;
	P1 = 0xff;
	P2 = 0xff;
	P3 = 0xf7;

#if defined (SOFT_SPI_PORT)		//	软件模拟SPI读写
	LD_MODE = 1;				//	设置MD管脚为高
#elif defined (HARD_SPI_PORT)	//	硬件实现SPI读写
	LD_MODE = 1;				//	设置MD管脚为高
#else							//	并行读写 （硬件实现或者软件模拟）
	LD_MODE = 0;				//	设置MD管脚为低
#endif
	IE0=1;
	EX0=1;
	EA=1;
}

/************************************************************************
功能描述： 延时函数
调用函数： 
入口参数： 
返回函数： 
**************************************************************************/
void  delay(unsigned long uldata)
{
	unsigned int j  =  0;
	unsigned int g  =  0;
	for (j=0;j<5;j++)
	{
		for (g=0;g<uldata;g++)
		{
			_nop_();
			_nop_();
			_nop_();
		}
	}
}
/************************************************************************
功能描述： 	运行ASR识别流程
入口参数：	none
返 回 值：  asrflag：1->启动成功， 0—>启动失败
其他说明：	识别顺序如下:
						1、RunASR()函数实现了一次完整的ASR语音识别流程
						2、LD_AsrStart() 函数实现了ASR初始化
						3、LD_AsrAddFixed() 函数实现了添加关键词语到LD3320芯片中
						4、LD_AsrRun()	函数启动了一次ASR语音识别流程					
						任何一次ASR识别流程，都需要按照这个顺序，从初始化开始
**************************************************************************/
uint8 RunASR()
{
	uint8 i=0;
	uint8 asrflag=0;
	for (i=0; i<5; i++)			//	防止由于硬件原因导致LD3320芯片工作不正常，所以一共尝试5次启动ASR识别流程
	{
		LD_AsrStart();
		delay(100);
		if (LD_AsrAddFixed()==0)
		{
			LD_Reset();			//	LD3320芯片内部出现不正常，立即重启LD3320芯片
			delay(100);			//	并从初始化开始重新ASR识别流程
			continue;
		}
		delay(10);
		if (LD_AsrRun() == 0)
		{
			LD_Reset();			//	LD3320芯片内部出现不正常，立即重启LD3320芯片
			delay(100);			//	并从初始化开始重新ASR识别流程
			continue;
		}
		asrflag=1;
		break;					//	ASR流程启动成功，退出当前for循环。开始等待LD3320送出的中断信号
	}

	return asrflag;
}
/************************************************************************
功能描述： 中断处理函数
调用函数： 
入口参数： 
返回函数： 
**************************************************************************/
void ExtInt0Handler(void) interrupt 0  
{ 	
	ProcessInt0();				//	LD3320 送出中断信号，包括ASR和播放MP3的中断，需要在中断处理函数中分别处理
	PrintCom("进入中断/n"); /*text.....*/
}



