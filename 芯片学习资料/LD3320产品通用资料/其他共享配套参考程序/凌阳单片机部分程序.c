#include <3320.h>
#include <sound.h>

uint8  nLD_Mode = LD_MODE_IDLE;		//	用来记录当前是在进行ASR识别还是在播放MP3						//	用来记录播放MP3的状态
uint8  ucRegVal;
uint8  ucHighInt;
uint8  ucLowInt;
uint8  ucStatus;
uint8  ucSPVol=15; // MAX=15 MIN=0 	//	Speaker喇叭输出的音量

extern uint8 nAsrStatus;

void  delay(unsigned int uldata)
{
	unsigned int j  =  0;
	unsigned int g  =  0;
	for (j=0;j<5;j++)
	{
		for (g=0;g<uldata;g++)
		{
			__asm("nop");
			__asm("nop");
			__asm("nop");
			*P_Watchdog_Clear=0x0001;
		}
	}
}

void LD_reset()
{
	*P_IOB_Data=0xff1f;//IOB13(RST) TO LOW
	delay(2);
	*P_IOB_Data=0xff5f;//RST TO high
}

unsigned char LD_ReadReg( unsigned char address )
{
	unsigned char idata;
	
	address=address<<4;
	address=address<<4;
	address=address&0xff00;
	*P_IOB_Data=0x005f;
	*P_IOB_Data=0x005c|address;		//configure IOB3(A0) to high output, IOB1(CS) IOB0(WR) to low output
	asm("nop");
	*P_IOB_Data=0x005f|address;		//configure IOB1(CS) IOB0(WR) to HIGH output
	asm("nop");

	*P_IOB_Dir=0x00ff;			//configure IOB8~IOB15 to input
	*P_IOB_Attrib=0x00ff;
	*P_IOB_Data=0xff45;		//data operation configure IOB1(CS) IOB4(RD) to low output
	asm("nop");
	idata=*P_IOB_Data;
	*P_IOB_Data=0x0057;		//configure configure IOB1(CS) IOB4(RD) to HIGH output
	asm("nop");
	
	*P_IOB_Dir=0xffff;			//configure to output
	*P_IOB_Attrib=0xffff;
	*P_IOB_Data=0x005f;
	
	idata=idata>>4;
	idata=idata>>4;
	idata=idata&0x00ff;
	return idata;
}

void LD_WriteReg(unsigned char address, unsigned char dataout)
{
	address=address<<4;
	address=address<<4;
//	address=address&0xff00;
	*P_IOB_Data=0x005f;
	
	*P_IOB_Data=0x005c|address;		//configure IOB1(CS) IOB0(WR) to low output, to write the address
	asm("nop");
	*P_IOB_Data=0x005f|address;		//configure IOB1(CS) IOB0(WR) to HIGH output
//	asm("nop");

	dataout=dataout<<4;
	dataout=dataout<<4;
//	dataout=dataout&0xff00;
	*P_IOB_Data=0x005f;
	
	*P_IOB_Data=0x054|dataout;		//configure IOB1(CS) IOB0(WR) to low output
//	asm("nop");
	*P_IOB_Data=0x0057|dataout;		//configure IOB1(CS) IOB0(WR) to HIGH output
//	asm("nop");
}

void LD_Init_Common()
{
	LD_ReadReg(0x06);  
	LD_WriteReg(0x17, 0x35); //写35H 对LD3320 进行软复位（Soft Reset）
	delay(10);
	LD_ReadReg(0x06);  

	LD_WriteReg(0x89, 0x03);  //模拟电路控制   初始化时写 03H 
	delay(5);
	LD_WriteReg(0xCF, 0x43);   // 内部省电模式设置 ,初始化时 写入 43H 
	delay(5);
	LD_WriteReg(0xCB, 0x02);	//读取 ASR结果（候补 4）
		
	/*PLL setting*/
	LD_WriteReg(0x11, LD_PLL_11);		//时钟频率设置1
	if (nLD_Mode == LD_MODE_MP3)
	{
		LD_WriteReg(0x1E, 0x00); 
		LD_WriteReg(0x19, LD_PLL_MP3_19);	
		LD_WriteReg(0x1B, LD_PLL_MP3_1B);	
		LD_WriteReg(0x1D, LD_PLL_MP3_1D);
	}
	else
	{
		LD_WriteReg(0x1E,0x00); //ADC 专用控制，应初始化为 00H 
		LD_WriteReg(0x19, LD_PLL_ASR_19); //时钟频率设置 2 
		LD_WriteReg(0x1B, LD_PLL_ASR_1B);//时钟频率设置3	
		LD_WriteReg(0x1D, LD_PLL_ASR_1D);//时钟频率设置 4
	}
	delay(10);
	
	LD_WriteReg(0xCD, 0x04);//DSP 休眠设置 ， 初始化时写入 04H 允许DSP 休眠
	LD_WriteReg(0x17, 0x4c); //写4C 可以使DSP 休眠，比较省电
	delay(5);					//0xb9 当前添加识别句的字符串长度（拼音字符串）
	LD_WriteReg(0xB9, 0x00);//,初始化时写入 00H,每添加一条识别句后要设定一次
	LD_WriteReg(0xCF, 0x4F); // MP3 初始化和 ASR初始化时写入 4FH 
	LD_WriteReg(0x6F, 0xFF); //对芯片进行初始化时设置为 0xFF 
	
	//*P_Watchdog_Clear=0x0001;
}
		
void LD_Init_ASR()
{
	nLD_Mode=LD_MODE_ASR_RUN;
	LD_Init_Common();

	LD_WriteReg(0xBD, 0x00);	//写入 00H；然后启动；为 ASR 模块；
	LD_WriteReg(0x17, 0x48);	//写48H 可以激活 DSP； 
	delay( 10 );

	LD_WriteReg(0x3C, 0x80);	//FIFO_EXT 下限低 8位（LowerBoundary L）
	LD_WriteReg(0x3E, 0x07);	//FIFO_EXT 下限高 8位（LowerBoundary H）
	LD_WriteReg(0x38, 0xff);   //FIFO_EXT 上限低 8位（UpperBoundary L）
	LD_WriteReg(0x3A, 0x07);	//FIFO_EXT上限高 8位（UpperBoundary H） 
	
	LD_WriteReg(0x40, 0);			//FIFO_EXT MCU水线低 8位（MCU water mark L） 
	LD_WriteReg(0x42, 8);		//FIFO_EXT MCU水线高 8位（MCU water mark H）
	LD_WriteReg(0x44, 0);		//FIFO_EXT DSP水线低 8位（DSP water mark L） 
	LD_WriteReg(0x46, 8);		//FIFO_EXT DSP水线高 8位（DSP water mark H）
	delay( 1 );
}

	
void ProcessInt0()
{
	uint8 nAsrResCount=0;

	asm("irq off");
	ucRegVal = LD_ReadReg(0x2B);
	if(nLD_Mode == LD_MODE_ASR_RUN)
	{
		// 语音识别产生的中断
		// （有声音输入，不论识别成功或失败都有中断）
		LD_WriteReg(0x29,0) ;//clear
		LD_WriteReg(0x02,0) ;//clear
		if((ucRegVal & 0x10) &&LD_ReadReg(0xb2)==0x21 && LD_ReadReg(0xbf)==0x35)
		{
			nAsrResCount = LD_ReadReg(0xba);
			if(nAsrResCount>0 && nAsrResCount<=4) 
				nAsrStatus=LD_ASR_FOUNDOK;
			else
				nAsrStatus=LD_ASR_FOUNDZERO;
		}
		else
			nAsrStatus=LD_ASR_FOUNDZERO;

		LD_WriteReg(0x2b, 0);//clear???/
		LD_WriteReg(0x1C,0);// 写 00H ADC不可用
		asm("irq on");
		return;
	}	
}

// Return 1: success.
uint8 LD_Check_ASRBusyFlag_b2()
{
	uint8 j;
	uint8 flag = 0;
	for (j=0; j<10; j++)
	{
		if (LD_ReadReg(0xb2) == 0x21)// 0x21 表示闲，查询到为闲状态可以进行下一步ASR 动作
		{
			flag = 1;
			break;
		}
		delay(10);
		*P_Watchdog_Clear=0x0001;
	}
	return flag;
}
	
// Return 1: success.
uint8 LD_AsrRun()
{
	LD_WriteReg(0x35, MIC_VOL);//ADC 增益，或可以理解为麦克风（MIC）音量
	LD_WriteReg(0x1C, 0x09);	// 写 09H Reserve 保留命令字，具体使用按照给出的参考程序代码使用
	LD_WriteReg(0xBD, 0x20);	//写入 02H；然后启动；为 MP3 模块；
	LD_WriteReg(0x08, 0x01);	//第0 位：写入1→清除FIFO_DATA ,第2 位：写入1→清除FIFO_EXT 
	delay( 1 );
	LD_WriteReg(0x08, 0x00);	//清除指定 FIFO 后再写入一次00H
	delay( 1 );

	if(LD_Check_ASRBusyFlag_b2() == 0)
		return 0;

	LD_WriteReg(0xB2, 0xff);	// 0x21 表示闲，查询到为闲状态可以进行下一步ASR 动作
	LD_WriteReg(0x37, 0x06);	//写06H：通知DSP 开始识别语音�
	LD_WriteReg(0x37, 0x06);
	delay( 5 );
	//LD_ReadReg(0x37);
	//LD_ReadReg(0xbf);
	LD_WriteReg(0x1C, 0x0b);	// 写 0BH 麦克风输入 ADC通道可用 
	LD_WriteReg(0x29, 0x10);	//第 4位：同步中断允许，1 表示允许；0表示不允许。
	
	LD_WriteReg(0xBD, 0x00);	//写入 00H；然后启动；为 ASR 模块； 
	//EX0=1;
	
	//*P_INT_Ctrl|=C_IRQ3_EXT1;
	//*P_INT_Ctrl|=0x0004;
	//__asm("IRQ ON");
	return 1;
}

uint8 RunASR()
{
	uint8 i=0;
	uint8 asrflag=0;
	for (i=0; i<5; i++)			//	防止由于硬件原因导致LD3320芯片工作不正常，所以一共尝试5次启动ASR识别流程
	{
		//*P_Watchdog_Clear=0x0001;
		LD_Init_ASR();
		delay(100);
		if (LD_AsrAddFixed()==0)
		{
			LD_reset();			//	LD3320芯片内部出现不正常，立即重启LD3320芯片
			delay(100);			//	并从初始化开始重新ASR识别流程
			continue;
		}
		delay(10);
		if (LD_AsrRun() == 0)
		{
			LD_reset();			//	LD3320芯片内部出现不正常，立即重启LD3320芯片
			delay(100);			//	并从初始化开始重新ASR识别流程
			continue;
		}

		asrflag=1;
		break;					//	ASR流程启动成功，退出当前for循环。开始等待LD3320送出的中断信号
	}
	return asrflag;
}

// Return 1: success.
//	添加识别关键词语，开发者可以学习"语音识别芯片LD3320高阶秘籍.pdf"中关于垃圾词语吸收错误的用法
uint8 LD_AsrAddFixed()
{
	uint8 k, flag;
	uint8 nAsrAddLength;
	const char sRecog[22][20] = {"yi","yi","er","san","si","wu","liu","qi","ba","jiu","ling",
		"kai ji", "guan ji", "jing yin", "xuan tai","jie mu jia","jie mu jian","da sheng dian","xiao sheng dian","huang zhong hou","a a","en en"};
	const uint8 pCode[22] = {YI,YI,ER,SAN,SI,WU,LIU,QI,BA,JIU,LING,CODE_KAIJI, CODE_GUANJI, CODE_JINGYIN,
		CODE_XUANTAI,CODE_JIEMUJIA,CODE_JIEMUJIAN,CODE_DASD,CODE_XIAOSD,CODE_HZHH,CODE_NONE,CODE_NONE};
			
	flag = 1;
	for (k=0; k<22; k++)
	{
			
		if(LD_Check_ASRBusyFlag_b2() == 0)
		{
			flag = 0;
			break;
		}
			
		LD_WriteReg(0xc1, pCode[k] );//ASR：识别字 Index（00H—FFH）
		LD_WriteReg(0xc3, 0 );			//ASR：识别字添加时写入 00 
		LD_WriteReg(0x08, 0x04);		//第2 位：写入1→清除FIFO_EXT 
		delay(1);
		LD_WriteReg(0x08, 0x00);		//清除指定 FIFO 后再写入一次00H
		delay(1);
	
		for (nAsrAddLength=0; nAsrAddLength<20; nAsrAddLength++)
		{
			if (sRecog[k][nAsrAddLength] == 0)
				break;
			LD_WriteReg(0x5, sRecog[k][nAsrAddLength]);//FIFO_EXT 数据口 
			*P_Watchdog_Clear=0x0001;
		}
		LD_WriteReg(0xb9, nAsrAddLength);//当前添加识别句的字符串长度（拼音字符串）
		LD_WriteReg(0xb2, 0xff);		//DSP忙闲状态
		LD_WriteReg(0x37, 0x04);		//写04H：通知DSP 要添加一项识别句。 
	}
	return flag;
}
