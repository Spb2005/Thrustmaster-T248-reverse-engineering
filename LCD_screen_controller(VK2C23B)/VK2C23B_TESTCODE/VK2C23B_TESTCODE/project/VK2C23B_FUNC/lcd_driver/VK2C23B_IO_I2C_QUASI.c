/**
  ******************************************************************************
  * @file    VK2C23B.c
  * @author  kevin_guo
  * @version V1.2
  * @date    10-12-2024
  * @brief   This file contains all the VK2C23B functions. 
	*          此文件适用于 VK2C23B
  ******************************************************************************
  * @attention
  ******************************************************************************
  */	
/* Includes ------------------------------------------------------------------*/
#include "VK2C23B_IO_I2C_QUASI.h"
  
/* extern variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define VK2C23B_CLK 10 //SCL信号线频率,由delay_nus实现 50->10kHz 10->50kHz 5->100kHz
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//segtab[]数组对应实际的芯片到LCD连线，连线见-VK2C23B参考电路
//VK2C23B 4com 
unsigned char VK2C23B_segtab[VK2C23B_SEGNUM]={
	4,5,6,7,8,9,										//SEG4-SEG9
	10,11,12,13,14,15,16,17,18,19,	//SEG10-SEG19
	20,21,22,23,24,25,26,27,28,29,	//SEG20-SEG29
	30,31,32,33,34,35,36,37,38,     //SEG30-SEG38
};
//例：LCD实际只接了SEG10到SEG21使用下面数组
//.h文件配置VK2C23B_SEGNUM=12
//const unsigned char VK2C23B_segtab[VK2C23B_SEGNUM]={	
//	10,11,12,13,14,15,16,17,18,19,	//SEG10-SEG19
//	20,21,													//SEG20-SEG21
//};
//VK2C23B_dispram对应芯片的显示RAM
//8com
unsigned char VK2C23B_dispram[VK2C23B_SEGNUM];//8COM时每个字节数据对应1个SEG
//对应数组VK2C23B_segtab[VK2C23B_SEGNUM]
//显示RAM buffer为8位对应1个SEG，bit7->com7,bit6->com6,...,bit0->com0

unsigned char VK2C23B_segi,VK2C23B_comi;
unsigned char VK2C23B_maxcom;//驱动的com数VK2C23B是8com
unsigned char VK2C23B_readbuf[35];
/* Private function prototypes -----------------------------------------------*/
void VK2C23B_InitSequence(void);
/* Private function ----------------------------------------------------------*/
/*******************************************************************************
* Function Name  : delay_nus
* Description    : 延时1uS程序
* Input          : n->延时时间nuS
* Output         : None
* Return         : None
*******************************************************************************/
void delay_nus(unsigned int n)	   
{
	unsigned char i;
	while(n--)
	{
		i=10;
		while(i--)
		{//nop指令根据单片机做相应的修改
			__nop();
		}
	}
}
/*******************************************************************************
* Function Name  : delay_nms
* Description    : 延时1mS程序
* Input          : n->延时时间nmS
* Output         : None
* Return         : None
*******************************************************************************/
void delay_nms(unsigned long int n)
{
	while(n--)
	{
		delay_nus(1000);
	}
}
/*******************************************************************************
* Function Name  : I2CStart
* Description    : 时钟线高时，数据线由高到低的跳变，表示I2C开始信号
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_I2CStart( void )
{
  VK2C23B_SCL_H();
  VK2C23B_SDA_H();
  delay_nus(VK2C23B_CLK);
  VK2C23B_SDA_L();
  delay_nus(VK2C23B_CLK);
}
/*******************************************************************************
* Function Name  : I2CStop
* Description    : 时钟线高时，数据线由低到高的跳变，表示I2C停止信号
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_I2CStop( void )
{
  VK2C23B_SCL_H();
  VK2C23B_SDA_L();
  delay_nus(VK2C23B_CLK);
  VK2C23B_SDA_H();
  delay_nus(VK2C23B_CLK);
}
/*******************************************************************************
* Function Name  : I2CSlaveAck
* Description    : I2C从机设备应答查询
* Input          : None
* Output         : None
* Return         : 0-ok 1-fail
*******************************************************************************/
unsigned char VK2C23B_I2CSlaveAck( void )
{
  unsigned int TimeOut;
  unsigned char RetValue;
	
  VK2C23B_SCL_L();
  delay_nus(VK2C23B_CLK);
  VK2C23B_SCL_H();//第9个sclk上升沿
  TimeOut = 10000;
  while( TimeOut-- > 0 )
  {
    if( VK2C23B_GET_SDA()!=0 )//读取ack
    {
      RetValue = 1;
    }
    else
    {
      RetValue = 0;
      break;
    }
  } 
	VK2C23B_SCL_L();
  
  return RetValue;
}
/*******************************************************************************
* Function Name  : I2CSendAck
* Description    : I2C主机发送ACK
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_I2CSendAck( void )
{
  VK2C23B_SCL_L();
  VK2C23B_SDA_L();
  delay_nus(VK2C23B_CLK*2);
  VK2C23B_SCL_H();
  delay_nus(VK2C23B_CLK*2);
  VK2C23B_SCL_L();
  VK2C23B_SDA_H();
}
/*******************************************************************************
* Function Name  : I2CSendNAck
* Description    : I2C主机发送NACK
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_I2CSendNAck( void )
{
  VK2C23B_SCL_L();
  VK2C23B_SDA_H();
  delay_nus(VK2C23B_CLK);
  VK2C23B_SCL_H();
  delay_nus(VK2C23B_CLK);
}
/*******************************************************************************
* Function Name  : I2CWriteByte
* Description    : I2C写一字节,先送高位
* Input          : byte-要写入的数据
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_I2CWriteByte( unsigned char byte )
{
	unsigned char i=8;
	while (i--)
	{ 
		VK2C23B_SCL_L();
		if(byte&0x80)
			VK2C23B_SDA_H();
		else
			VK2C23B_SDA_L();
		byte<<=1; 
		delay_nus(VK2C23B_CLK);
		VK2C23B_SCL_H();     
		delay_nus(VK2C23B_CLK);
	}
}

/*******************************************************************************
* Function Name  : WriteCmd
* Description    : 写1字节命令带参数
* Input          : cmd-要写入的命令
*                : data-要写入的参数
* Output         : None
* Return         : 0-ok 1-fail
*******************************************************************************/
unsigned char WriteCmdVK2C23B(unsigned char cmd, unsigned char data )
{
	VK2C23B_I2CStart();

	VK2C23B_I2CWriteByte( VK2C23B_ADDR|0x00 );
	if( 1 == VK2C23B_I2CSlaveAck() )
	{
		VK2C23B_I2CStop(); 
		return 0;   
	}
	VK2C23B_I2CWriteByte( cmd );
	if( 1 == VK2C23B_I2CSlaveAck() )
	{
		VK2C23B_I2CStop(); 
		return 0;   
	}
	VK2C23B_I2CWriteByte( data );
	if( 1 == VK2C23B_I2CSlaveAck() )
	{
		VK2C23B_I2CStop(); 
		return 0;   
	}
  VK2C23B_I2CStop();
 
  return 0;    //返回操作成败标志
}
/*******************************************************************************
* Function Name  : I2CRDDatByte
* Description    : I2C读一字节数据,数据先送低位
* Input          : None
* Output         : None
* Return         : 读出的1字节数据
*******************************************************************************/
unsigned char VK2C23B_I2CRDDat( void )
{
	unsigned char i,RetValue,bit;
	
	RetValue=0;	
  for( i=0; i<8; i++ )
  {
		VK2C23B_SCL_H(); 
    delay_nus(VK2C23B_CLK);
    if( 1 == VK2C23B_GET_SDA() )
      bit = 0X01;
    else
      bit = 0x00;
      
    RetValue = (RetValue<<1)|bit;
    VK2C23B_SCL_L();
    delay_nus(VK2C23B_CLK);
	}
  
  return RetValue;
}
/*******************************************************************************
* Function Name  : WritenData
* Description    : 写多个数据到显示RAM
* Input          : Addr-写入ram的起始地址
*                : Databuf->写入ram的数据buffer指针
*                : Cnt->写入ram的数据个数
* Output         : None
* Return         : 0-ok 1-fail
*******************************************************************************/
unsigned char  WritenDataVK2C23B(unsigned char Addr,unsigned char *Databuf,unsigned char Cnt)
{
	unsigned char n;
	
	//START信号	
	VK2C23B_I2CStart(); 									
	//SLAVE地址
	VK2C23B_I2CWriteByte(VK2C23B_ADDR); 	
	if( 1 == VK2C23B_I2CSlaveAck() )
	{
		VK2C23B_I2CStop();													
		return 0;										
	}
	//写显示RAM命令
	VK2C23B_I2CWriteByte(VK2C23B_RWRAM); 						
	if( 1 == VK2C23B_I2CSlaveAck() )
	{
		VK2C23B_I2CStop();													
		return 0;
	}
	//显示RAM起始地址
	VK2C23B_I2CWriteByte(Addr); 						
	if( 1 == VK2C23B_I2CSlaveAck() )
	{
		VK2C23B_I2CStop();													
		return 0;
	}
	//发送Cnt个数据到显示RAM
	for(n=0;n<Cnt;n++)
	{ 
		VK2C23B_I2CWriteByte(*Databuf++);
		if( VK2C23B_I2CSlaveAck()==1 )
		{
			VK2C23B_I2CStop();													
			return 0;
		}
	}
	//STOP信号
	 VK2C23B_I2CStop();											
	 return 0;    
}
/*******************************************************************************
* Function Name  : ReadnData
* Description    : 从显示RAM读多个数据
* Input          : Addr-读ram的起始地址
*                : Databuf->读ram的数据buffer指针
*                : Cnt->读ram的数据个数
* Output         : None
* Return         : 0-ok 1-fail
*******************************************************************************/
unsigned char  ReadnDataVK2C23B(unsigned char Addr,unsigned char *Databuf,unsigned char Cnt)
{
	unsigned char n;
	
	//START信号	
	VK2C23B_I2CStart(); 									
	//SLAVE地址写
	VK2C23B_I2CWriteByte(VK2C23B_ADDR); 	
	if( 1 == VK2C23B_I2CSlaveAck() )
	{
		VK2C23B_I2CStop();													
		return 0;										
	}      
  //写显示RAM命令
	VK2C23B_I2CWriteByte(VK2C23B_RWRAM); 						
	if( 1 == VK2C23B_I2CSlaveAck() )
	{
		VK2C23B_I2CStop();													
		return 0;
	}	
	//显示RAM起始地址
	VK2C23B_I2CWriteByte(Addr); 						
	if( 1 == VK2C23B_I2CSlaveAck() )
	{
		VK2C23B_I2CStop();													
		return 0;
	}
	
	//STOP信号
	VK2C23B_I2CStop();	
	//START信号	
	VK2C23B_I2CStart(); 									
	//SLAVE地址读
	VK2C23B_I2CWriteByte(VK2C23B_ADDRRD); 	
		if( 1 == VK2C23B_I2CSlaveAck() )
	{
		VK2C23B_I2CStop();													
		return 0;										
	}  
	//读Cnt个数据到显示RAM
	for(n=0;n<Cnt-1;n++)
	{ 
		*Databuf++=VK2C23B_I2CRDDat();
		VK2C23B_I2CSendAck();
	}
	*Databuf++=VK2C23B_I2CRDDat();
	VK2C23B_I2CSendNAck();
	//STOP信号
	VK2C23B_I2CStop();											
	 return 0;    
}
/*******************************************************************************
* Function Name  : VK2C23B_DisAll
* Description    : 所有SEG显示同一个数据，bit7/bit3-COM3 bit6/bit2-COM2 bit5/bit1-COM1 bit4/bit0-COM0
* 					     : 例如：0xff全亮 0x00全灭 0x55灭亮灭亮 0xaa亮灭亮灭 0x33灭灭亮亮 
* Input          ：dat->写入ram的数据(1个字节数据对应2个SEG)  
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_DisAll(unsigned char dat)
{
	unsigned char segi;
	
	for(segi=0;segi<35;segi++)
	{
		VK2C23B_dispram[segi]=dat;
	}
	WritenDataVK2C23B(0,VK2C23B_dispram,35);//这里送8bit数据对应1个SEG，每8bit数据地址加1，每8位数据1个ACK
}
/*******************************************************************************
* Function Name  : DisSegComOn
* Description    : 点亮1个点(1个seg和1个com交叉对应的显示点)
* Input          ：seg->点对应的seg脚  
* 		           ：com->点对应com脚  
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_DisDotOn(unsigned char seg,unsigned char com)
{
	unsigned char addrbit,tempdat;
	
	addrbit=(1<<com);
	tempdat=VK2C23B_dispram[seg]|addrbit;
	VK2C23B_dispram[seg]=tempdat;
	WritenDataVK2C23B(seg,&tempdat,1);//这里送8位数据低4bit有效，每8bit数据地址加1，每8位数据1个ACK
}
/*******************************************************************************
* Function Name  : DisSegComOff
* Description    : 关闭1个点(1个seg和1个com交叉对应的显示点)
* Input          ：seg->点对应的seg脚  
* 		           ：com->点对应com脚  
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_DisDotOff(unsigned char seg,unsigned char com)
{
	unsigned char addrbit,tempdat;

	addrbit=(1<<com);
	tempdat=VK2C23B_dispram[seg]&~addrbit;
	VK2C23B_dispram[seg]=tempdat;
	WritenDataVK2C23B(seg,&tempdat,1);
}
/*******************************************************************************
* Function Name  : Enter_Standby
* Description    : 进入掉电低功耗模式,掉电无显示
* Input          ：None 
* Output         : None
* Return         : 0-ok 1-fail
*******************************************************************************/
void VK2C23B_Enter_PowerOff(void)
{		
	WriteCmdVK2C23B(VK2C23B_SYSSET,SYSOFF_LCDOFF);
}
/*******************************************************************************
* Function Name  : Exit_Standby
* Description    : 退出掉电低功耗模式
* Input          ：None 
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_Exit_PowerOff(void)
{	
	WriteCmdVK2C23B(VK2C23B_SYSSET,SYSON_LCDON);
}

/*******************************************************************************
* Function Name  : Lowlevel_Init
* Description    : 配置通信线GPIO
* Input          ：None 
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_Lowlevel_Init(void)
{
	//通信线电平不同，建议加电平转换电路
	//此函数根据客户单片机做相应的修改	
	GPIO_SetMode(VK2C23B_SCL_PORT, VK2C23B_SCL_PIN, GPIO_MODE_OUTPUT);
	GPIO_SetMode(VK2C23B_SDA_PORT, VK2C23B_SDA_PIN, GPIO_MODE_QUASI);
		      
	VK2C23B_SCL_H();  
	VK2C23B_SDA_H(); 	
}
/*******************************************************************************
* Function Name  : Init
* Description    : 初始化配置
* Input          ：None 
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_InitSequence(void)
{	
	//管脚配置根据客户单片机做相应的修改
	VK2C23B_Lowlevel_Init();
	WriteCmdVK2C23B(VK2C23B_MODESET,CCOM_1_3__8); //模式设置  1/3 Bais 1/8 Duty
//	WriteCmdVK2C23B(VK2C23B_MODESET,CCOM_1_4__8); //模式设置  1/4 Bais 1/8 Duty
	VK2C23B_maxcom=8;
	WriteCmdVK2C23B(VK2C23B_SYSSET,SYSON_LCDON); 		//内部系统振荡器开，lcd开显示
	WriteCmdVK2C23B(VK2C23B_FRAMESET,FRAME_80HZ); 	//帧频率80Hz
//	WriteCmdVK2C23B(VK2C23B_FRAMESET,FRAME_160HZ);//帧频率160Hz
	WriteCmdVK2C23B(VK2C23B_BLINKSET,BLINK_OFF); 		//闪烁关闭	
//	WriteCmdVK2C23B(VK2C23B_BLINKSET,BLINK_2HZ); 		//闪烁2HZ
//	WriteCmdVK2C23B(VK2C23B_BLINKSET,BLINK_1HZ); 		//闪烁1HZ
//	WriteCmdVK2C23B(VK2C23B_BLINKSET,BLINK_0D5HZ); 	//闪烁0.5HZ
	//SEG/VLCD共用脚设为VLCD，内部电压调整功能关闭,VLCD和VDD短接VR=0偏置电压=VDD
//	WriteCmdVK2C23B(VK2C23B_IVASET,VLCDSEL_IVAOFF_R0); 
	//SEG/VLCD共用脚设为VLCD，内部电压调整功能关闭,VLCD和VDD串接电阻VR>0偏置电压=VLCD
//	WriteCmdVK2C23B(VK2C23B_IVASET,VLCDSEL_IVAOFF_R1); 
	//SEG/VLCD共用脚设为SEG，内部偏置电压调整：1/3bias=0.652VDD 1/4bias=0.714VDD
	WriteCmdVK2C23B(VK2C23B_IVASET,SEGSEL_IVA00H);	
}
/*******************************************************************************
* Function Name  : Init
* Description    : 初始化配置
* Input          ：None 
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_Init(void)
{	
	//管脚配置根据客户单片机做相应的修改
	VK2C23B_Lowlevel_Init();
	//初始化时序
	VK2C23B_InitSequence();
}
/*******************************************************************************
* Function Name  : test_Main
* Description    : 测试主程序
* Input          ：None 
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_Main(void)
{	
	VK2C23B_Init();
	VK2C23B_DisAll(0x00);
	while(1)
	{
		VK2C23B_DisAll(0xff);			//LCD全显
		delay_nms(3000);					//延时1S
	
		VK2C23B_DisAll(0x00);			//LCD全关
		delay_nms(3000);					//延时1S
		
		//LCD半显
		VK2C23B_DisAll(0xAA);			
		ReadnDataVK2C23B(0,VK2C23B_readbuf,35);	//读回RAM数据
		delay_nms(1500);					//延时
		VK2C23B_DisAll(0x55);			
		ReadnDataVK2C23B(0,VK2C23B_readbuf,35);	//读回RAM数据
		delay_nms(1500);					//延时
		
		//掉电
		VK2C23B_DisAll(0x00);			//LCD全关		
		VK2C23B_Enter_PowerOff();	//进入掉电模式
		delay_nms(5000);					//延时5S		
		VK2C23B_Exit_PowerOff();	//退出掉电模式
		
		VK2C23B_DisAll(0x00);			//LCD全关
		delay_nms(500);					  //延时
		for(VK2C23B_segi=0;VK2C23B_segi<VK2C23B_SEGNUM;VK2C23B_segi++)//seg
		{
			for(VK2C23B_comi=0;VK2C23B_comi<VK2C23B_maxcom;VK2C23B_comi++)//com
			{
					VK2C23B_DisDotOn(VK2C23B_segtab[VK2C23B_segi]-4,VK2C23B_comi);	//LCD单点点亮
				delay_nms(300);				//延时300mS
				VK2C23B_DisDotOff(VK2C23B_segtab[VK2C23B_segi]-4,VK2C23B_comi);	  //此行配合单点点亮函数实现单点点亮，屏蔽此行顺序点亮
			}
		}
		
		VK2C23B_DisAll(0xff);			//LCD全显
		delay_nms(500);					  //延时
		for(VK2C23B_segi=0;VK2C23B_segi<VK2C23B_SEGNUM;VK2C23B_segi++)//seg
		{
			for(VK2C23B_comi=0;VK2C23B_comi<VK2C23B_maxcom;VK2C23B_comi++)//com
			{
					VK2C23B_DisDotOff(VK2C23B_segtab[VK2C23B_segi]-4,VK2C23B_comi);	//LCD单点关闭
				delay_nms(300);				//延时300mS
				VK2C23B_DisDotOn(VK2C23B_segtab[VK2C23B_segi]-4,VK2C23B_comi);  //此行配合单点关闭函数实现单点关闭，屏蔽此行顺序关闭
			}
		}
		delay_nms(1000);					  //延时
	}
}
/************************END OF FILE****/
