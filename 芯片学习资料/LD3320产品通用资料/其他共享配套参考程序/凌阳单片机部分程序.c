#include <3320.h>
#include <sound.h>

uint8  nLD_Mode = LD_MODE_IDLE;		//	ÓÃÀ´¼ÇÂ¼µ±Ç°ÊÇÔÚ½øĞĞASRÊ¶±ğ»¹ÊÇÔÚ²¥·ÅMP3						//	ÓÃÀ´¼ÇÂ¼²¥·ÅMP3µÄ×´Ì¬
uint8  ucRegVal;
uint8  ucHighInt;
uint8  ucLowInt;
uint8  ucStatus;
uint8  ucSPVol=15; // MAX=15 MIN=0 	//	SpeakerÀ®°ÈÊä³öµÄÒôÁ¿

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
	LD_WriteReg(0x17, 0x35); //Ğ´35H ¶ÔLD3320 ½øĞĞÈí¸´Î»£¨Soft Reset£©
	delay(10);
	LD_ReadReg(0x06);  

	LD_WriteReg(0x89, 0x03);  //Ä£ÄâµçÂ·¿ØÖÆ   ³õÊ¼»¯Ê±Ğ´ 03H 
	delay(5);
	LD_WriteReg(0xCF, 0x43);   // ÄÚ²¿Ê¡µçÄ£Ê½ÉèÖÃ ,³õÊ¼»¯Ê± Ğ´Èë 43H 
	delay(5);
	LD_WriteReg(0xCB, 0x02);	//¶ÁÈ¡ ASR½á¹û£¨ºò²¹ 4£©
		
	/*PLL setting*/
	LD_WriteReg(0x11, LD_PLL_11);		//Ê±ÖÓÆµÂÊÉèÖÃ1
	if (nLD_Mode == LD_MODE_MP3)
	{
		LD_WriteReg(0x1E, 0x00); 
		LD_WriteReg(0x19, LD_PLL_MP3_19);	
		LD_WriteReg(0x1B, LD_PLL_MP3_1B);	
		LD_WriteReg(0x1D, LD_PLL_MP3_1D);
	}
	else
	{
		LD_WriteReg(0x1E,0x00); //ADC ×¨ÓÃ¿ØÖÆ£¬Ó¦³õÊ¼»¯Îª 00H 
		LD_WriteReg(0x19, LD_PLL_ASR_19); //Ê±ÖÓÆµÂÊÉèÖÃ 2 
		LD_WriteReg(0x1B, LD_PLL_ASR_1B);//Ê±ÖÓÆµÂÊÉèÖÃ3	
		LD_WriteReg(0x1D, LD_PLL_ASR_1D);//Ê±ÖÓÆµÂÊÉèÖÃ 4
	}
	delay(10);
	
	LD_WriteReg(0xCD, 0x04);//DSP ĞİÃßÉèÖÃ £¬ ³õÊ¼»¯Ê±Ğ´Èë 04H ÔÊĞíDSP ĞİÃß
	LD_WriteReg(0x17, 0x4c); //Ğ´4C ¿ÉÒÔÊ¹DSP ĞİÃß£¬±È½ÏÊ¡µç
	delay(5);					//0xb9 µ±Ç°Ìí¼ÓÊ¶±ğ¾äµÄ×Ö·û´®³¤¶È£¨Æ´Òô×Ö·û´®£©
	LD_WriteReg(0xB9, 0x00);//,³õÊ¼»¯Ê±Ğ´Èë 00H,Ã¿Ìí¼ÓÒ»ÌõÊ¶±ğ¾äºóÒªÉè¶¨Ò»´Î
	LD_WriteReg(0xCF, 0x4F); // MP3 ³õÊ¼»¯ºÍ ASR³õÊ¼»¯Ê±Ğ´Èë 4FH 
	LD_WriteReg(0x6F, 0xFF); //¶ÔĞ¾Æ¬½øĞĞ³õÊ¼»¯Ê±ÉèÖÃÎª 0xFF 
	
	//*P_Watchdog_Clear=0x0001;
}
		
void LD_Init_ASR()
{
	nLD_Mode=LD_MODE_ASR_RUN;
	LD_Init_Common();

	LD_WriteReg(0xBD, 0x00);	//Ğ´Èë 00H£»È»ºóÆô¶¯£»Îª ASR Ä£¿é£»
	LD_WriteReg(0x17, 0x48);	//Ğ´48H ¿ÉÒÔ¼¤»î DSP£» 
	delay( 10 );

	LD_WriteReg(0x3C, 0x80);	//FIFO_EXT ÏÂÏŞµÍ 8Î»£¨LowerBoundary L£©
	LD_WriteReg(0x3E, 0x07);	//FIFO_EXT ÏÂÏŞ¸ß 8Î»£¨LowerBoundary H£©
	LD_WriteReg(0x38, 0xff);   //FIFO_EXT ÉÏÏŞµÍ 8Î»£¨UpperBoundary L£©
	LD_WriteReg(0x3A, 0x07);	//FIFO_EXTÉÏÏŞ¸ß 8Î»£¨UpperBoundary H£© 
	
	LD_WriteReg(0x40, 0);			//FIFO_EXT MCUË®ÏßµÍ 8Î»£¨MCU water mark L£© 
	LD_WriteReg(0x42, 8);		//FIFO_EXT MCUË®Ïß¸ß 8Î»£¨MCU water mark H£©
	LD_WriteReg(0x44, 0);		//FIFO_EXT DSPË®ÏßµÍ 8Î»£¨DSP water mark L£© 
	LD_WriteReg(0x46, 8);		//FIFO_EXT DSPË®Ïß¸ß 8Î»£¨DSP water mark H£©
	delay( 1 );
}

	
void ProcessInt0()
{
	uint8 nAsrResCount=0;

	asm("irq off");
	ucRegVal = LD_ReadReg(0x2B);
	if(nLD_Mode == LD_MODE_ASR_RUN)
	{
		// ÓïÒôÊ¶±ğ²úÉúµÄÖĞ¶Ï
		// £¨ÓĞÉùÒôÊäÈë£¬²»ÂÛÊ¶±ğ³É¹¦»òÊ§°Ü¶¼ÓĞÖĞ¶Ï£©
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
		LD_WriteReg(0x1C,0);// Ğ´ 00H ADC²»¿ÉÓÃ
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
		if (LD_ReadReg(0xb2) == 0x21)// 0x21 ±íÊ¾ÏĞ£¬²éÑ¯µ½ÎªÏĞ×´Ì¬¿ÉÒÔ½øĞĞÏÂÒ»²½ASR ¶¯×÷
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
	LD_WriteReg(0x35, MIC_VOL);//ADC ÔöÒæ£¬»ò¿ÉÒÔÀí½âÎªÂó¿Ë·ç£¨MIC£©ÒôÁ¿
	LD_WriteReg(0x1C, 0x09);	// Ğ´ 09H Reserve ±£ÁôÃüÁî×Ö£¬¾ßÌåÊ¹ÓÃ°´ÕÕ¸ø³öµÄ²Î¿¼³ÌĞò´úÂëÊ¹ÓÃ
	LD_WriteReg(0xBD, 0x20);	//Ğ´Èë 02H£»È»ºóÆô¶¯£»Îª MP3 Ä£¿é£»
	LD_WriteReg(0x08, 0x01);	//µÚ0 Î»£ºĞ´Èë1¡úÇå³ıFIFO_DATA ,µÚ2 Î»£ºĞ´Èë1¡úÇå³ıFIFO_EXT 
	delay( 1 );
	LD_WriteReg(0x08, 0x00);	//Çå³ıÖ¸¶¨ FIFO ºóÔÙĞ´ÈëÒ»´Î00H
	delay( 1 );

	if(LD_Check_ASRBusyFlag_b2() == 0)
		return 0;

	LD_WriteReg(0xB2, 0xff);	// 0x21 ±íÊ¾ÏĞ£¬²éÑ¯µ½ÎªÏĞ×´Ì¬¿ÉÒÔ½øĞĞÏÂÒ»²½ASR ¶¯×÷
	LD_WriteReg(0x37, 0x06);	//Ğ´06H£ºÍ¨ÖªDSP ¿ªÊ¼Ê¶±ğÓïÒô¡
	LD_WriteReg(0x37, 0x06);
	delay( 5 );
	//LD_ReadReg(0x37);
	//LD_ReadReg(0xbf);
	LD_WriteReg(0x1C, 0x0b);	// Ğ´ 0BH Âó¿Ë·çÊäÈë ADCÍ¨µÀ¿ÉÓÃ 
	LD_WriteReg(0x29, 0x10);	//µÚ 4Î»£ºÍ¬²½ÖĞ¶ÏÔÊĞí£¬1 ±íÊ¾ÔÊĞí£»0±íÊ¾²»ÔÊĞí¡£
	
	LD_WriteReg(0xBD, 0x00);	//Ğ´Èë 00H£»È»ºóÆô¶¯£»Îª ASR Ä£¿é£» 
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
	for (i=0; i<5; i++)			//	·ÀÖ¹ÓÉÓÚÓ²¼şÔ­Òòµ¼ÖÂLD3320Ğ¾Æ¬¹¤×÷²»Õı³££¬ËùÒÔÒ»¹²³¢ÊÔ5´ÎÆô¶¯ASRÊ¶±ğÁ÷³Ì
	{
		//*P_Watchdog_Clear=0x0001;
		LD_Init_ASR();
		delay(100);
		if (LD_AsrAddFixed()==0)
		{
			LD_reset();			//	LD3320Ğ¾Æ¬ÄÚ²¿³öÏÖ²»Õı³££¬Á¢¼´ÖØÆôLD3320Ğ¾Æ¬
			delay(100);			//	²¢´Ó³õÊ¼»¯¿ªÊ¼ÖØĞÂASRÊ¶±ğÁ÷³Ì
			continue;
		}
		delay(10);
		if (LD_AsrRun() == 0)
		{
			LD_reset();			//	LD3320Ğ¾Æ¬ÄÚ²¿³öÏÖ²»Õı³££¬Á¢¼´ÖØÆôLD3320Ğ¾Æ¬
			delay(100);			//	²¢´Ó³õÊ¼»¯¿ªÊ¼ÖØĞÂASRÊ¶±ğÁ÷³Ì
			continue;
		}

		asrflag=1;
		break;					//	ASRÁ÷³ÌÆô¶¯³É¹¦£¬ÍË³öµ±Ç°forÑ­»·¡£¿ªÊ¼µÈ´ıLD3320ËÍ³öµÄÖĞ¶ÏĞÅºÅ
	}
	return asrflag;
}

// Return 1: success.
//	Ìí¼ÓÊ¶±ğ¹Ø¼ü´ÊÓï£¬¿ª·¢Õß¿ÉÒÔÑ§Ï°"ÓïÒôÊ¶±ğĞ¾Æ¬LD3320¸ß½×ÃØ¼®.pdf"ÖĞ¹ØÓÚÀ¬»ø´ÊÓïÎüÊÕ´íÎóµÄÓÃ·¨
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
			
		LD_WriteReg(0xc1, pCode[k] );//ASR£ºÊ¶±ğ×Ö Index£¨00H¡ªFFH£©
		LD_WriteReg(0xc3, 0 );			//ASR£ºÊ¶±ğ×ÖÌí¼ÓÊ±Ğ´Èë 00 
		LD_WriteReg(0x08, 0x04);		//µÚ2 Î»£ºĞ´Èë1¡úÇå³ıFIFO_EXT 
		delay(1);
		LD_WriteReg(0x08, 0x00);		//Çå³ıÖ¸¶¨ FIFO ºóÔÙĞ´ÈëÒ»´Î00H
		delay(1);
	
		for (nAsrAddLength=0; nAsrAddLength<20; nAsrAddLength++)
		{
			if (sRecog[k][nAsrAddLength] == 0)
				break;
			LD_WriteReg(0x5, sRecog[k][nAsrAddLength]);//FIFO_EXT Êı¾İ¿Ú 
			*P_Watchdog_Clear=0x0001;
		}
		LD_WriteReg(0xb9, nAsrAddLength);//µ±Ç°Ìí¼ÓÊ¶±ğ¾äµÄ×Ö·û´®³¤¶È£¨Æ´Òô×Ö·û´®£©
		LD_WriteReg(0xb2, 0xff);		//DSPÃ¦ÏĞ×´Ì¬
		LD_WriteReg(0x37, 0x04);		//Ğ´04H£ºÍ¨ÖªDSP ÒªÌí¼ÓÒ»ÏîÊ¶±ğ¾ä¡£ 
	}
	return flag;
}
