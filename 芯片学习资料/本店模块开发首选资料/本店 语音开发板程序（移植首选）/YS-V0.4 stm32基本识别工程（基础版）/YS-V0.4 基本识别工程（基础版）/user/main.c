/***************************乐声电子科技有限公司****************************
**  工程名称：YS-V0.4语音识别开发板程序
**	CPU: STM32f103RCT6
**	晶振：8MHZ
**	波特率：9600 bit/S
**	配套产品信息：YS-V0.4语音识别开发板
**                http://yuesheng001.taobao.com
**  作者：zdings
**  联系：751956552@qq.com
**  修改日期：2012.4.11
**  说明：本程序 具备语音识别、串口通信、开发板基本功能演示。
/*************端口信息********************
 * 接线说明
 * RST      PB6
 * CS   	  PB8
 * WR/SPIS  PB9
 * P2/SDCK  PB3
 * P1/SDO   PB4
 * P0/SDI   PB5
 * IRQ      PC1
 * CLK  与单片机的晶振共用

***************************乐声电子科技有限公司******************************/
#include "includes.h"

/* Private function prototypes -----------------------------------------------*/
void RCC_Configuration(void);
//void NVIC_Configuration(void);
void  USART_Configuration(void);
void PrintCom(USART_TypeDef* USARTx, uint8_t *Data);

extern void  LD3320_main(void);
/***********************************************************
* 名    称： 主函数
* 功    能：	程序入口
* 入口参数：  无
* 出口参数：	无
* 说    明：	无
* 调用方法： 	无
**********************************************************/ 
int main(void)
{
  /* System Clocks Configuration */
  RCC_Configuration();  //外设时钟配置
  USART_Configuration();//USART 配置	
	LD3320_main();				//LD3320执行函数
	while (1)
  {	
	}
	
}
/***********************************************************
* 名    称： void  USART_Configuration(void)
* 功    能： 串口配置
* 入口参数：  
* 出口参数：
* 说    明：
* 调用方法： 
**********************************************************/ 
void  USART_Configuration(void)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

  /* Configure USART1 */
  USART_Init(USART1, &USART_InitStructure);	//类似与GPIO口，配置完后初始化一次
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  //使能接收 中断 
  USART_ITConfig(USART1, USART_IT_TXE, ENABLE);  //使能发送 中断 
  USART_Cmd(USART1, ENABLE);
	
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;	         //USART1 TX
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;    //复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);		    //A端口 

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	         //USART1 RX
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   //复用开漏输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);		         //A端口 
}
/***********************************************************
* 名    称：void PrintCom(USART_TypeDef* USARTx, uint8_t *Data)
* 功    能：串口数据打印
* 入口参数：  
* 出口参数：
* 说    明：
* 调用方法： 
**********************************************************/ 
void PrintCom(USART_TypeDef* USARTx, uint8_t *Data)
{ 
	
		while(*Data)
		{
			USART_SendData(USARTx, *Data++);    /*发送单个数据 */
			while(USART_GetFlagStatus(USARTx, USART_FLAG_TC)==RESET);/* 检查指定的USART标志位即RESET＝1时发送完成*/
		}													
}

/***********************************************************
* 名    称：void RCC_Configuration(void)
* 功    能：配置外设时钟
* 入口参数：  
* 出口参数：
* 说    明：
* 调用方法： 
**********************************************************/ 
void RCC_Configuration(void)
{
  SystemInit();  
  /* Enable USART1, GPIOA, GPIOx and AFIO clocks */
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1 |RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                         RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
                         RCC_APB2Periph_GPIOE, ENABLE); //开启相关的AHP外设时钟
  /* Enable USART2 clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
}

/**
  * @brief  Configures the different GPIO ports.
  * @param  None
  * @retval : None
  */



