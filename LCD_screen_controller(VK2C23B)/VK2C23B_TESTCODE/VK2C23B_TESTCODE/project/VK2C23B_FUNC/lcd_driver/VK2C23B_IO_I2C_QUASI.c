/**
  ******************************************************************************
  * @file    VK2C23B.c
  * @author  kevin_guo
  * @version V1.2
  * @date    10-12-2024
  * @brief   This file contains all the VK2C23B functions. 
	*          ���ļ������� VK2C23B
  ******************************************************************************
  * @attention
  ******************************************************************************
  */	
/* Includes ------------------------------------------------------------------*/
#include "VK2C23B_IO_I2C_QUASI.h"
  
/* extern variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define VK2C23B_CLK 10 //SCL�ź���Ƶ��,��delay_nusʵ�� 50->10kHz 10->50kHz 5->100kHz
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//segtab[]�����Ӧʵ�ʵ�оƬ��LCD���ߣ����߼�-VK2C23B�ο���·
//VK2C23B 4com 
unsigned char VK2C23B_segtab[VK2C23B_SEGNUM]={
	4,5,6,7,8,9,										//SEG4-SEG9
	10,11,12,13,14,15,16,17,18,19,	//SEG10-SEG19
	20,21,22,23,24,25,26,27,28,29,	//SEG20-SEG29
	30,31,32,33,34,35,36,37,38,     //SEG30-SEG38
};
//����LCDʵ��ֻ����SEG10��SEG21ʹ����������
//.h�ļ�����VK2C23B_SEGNUM=12
//const unsigned char VK2C23B_segtab[VK2C23B_SEGNUM]={	
//	10,11,12,13,14,15,16,17,18,19,	//SEG10-SEG19
//	20,21,													//SEG20-SEG21
//};
//VK2C23B_dispram��ӦоƬ����ʾRAM
//8com
unsigned char VK2C23B_dispram[VK2C23B_SEGNUM];//8COMʱÿ���ֽ����ݶ�Ӧ1��SEG
//��Ӧ����VK2C23B_segtab[VK2C23B_SEGNUM]
//��ʾRAM bufferΪ8λ��Ӧ1��SEG��bit7->com7,bit6->com6,...,bit0->com0

unsigned char VK2C23B_segi,VK2C23B_comi;
unsigned char VK2C23B_maxcom;//������com��VK2C23B��8com
unsigned char VK2C23B_readbuf[35];
/* Private function prototypes -----------------------------------------------*/
void VK2C23B_InitSequence(void);
/* Private function ----------------------------------------------------------*/
/*******************************************************************************
* Function Name  : delay_nus
* Description    : ��ʱ1uS����
* Input          : n->��ʱʱ��nuS
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
		{//nopָ����ݵ�Ƭ������Ӧ���޸�
			__nop();
		}
	}
}
/*******************************************************************************
* Function Name  : delay_nms
* Description    : ��ʱ1mS����
* Input          : n->��ʱʱ��nmS
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
* Description    : ʱ���߸�ʱ���������ɸߵ��͵����䣬��ʾI2C��ʼ�ź�
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
* Description    : ʱ���߸�ʱ���������ɵ͵��ߵ����䣬��ʾI2Cֹͣ�ź�
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
* Description    : I2C�ӻ��豸Ӧ���ѯ
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
  VK2C23B_SCL_H();//��9��sclk������
  TimeOut = 10000;
  while( TimeOut-- > 0 )
  {
    if( VK2C23B_GET_SDA()!=0 )//��ȡack
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
* Description    : I2C��������ACK
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
* Description    : I2C��������NACK
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
* Description    : I2Cдһ�ֽ�,���͸�λ
* Input          : byte-Ҫд�������
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
* Description    : д1�ֽ����������
* Input          : cmd-Ҫд�������
*                : data-Ҫд��Ĳ���
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
 
  return 0;    //���ز����ɰܱ�־
}
/*******************************************************************************
* Function Name  : I2CRDDatByte
* Description    : I2C��һ�ֽ�����,�������͵�λ
* Input          : None
* Output         : None
* Return         : ������1�ֽ�����
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
* Description    : д������ݵ���ʾRAM
* Input          : Addr-д��ram����ʼ��ַ
*                : Databuf->д��ram������bufferָ��
*                : Cnt->д��ram�����ݸ���
* Output         : None
* Return         : 0-ok 1-fail
*******************************************************************************/
unsigned char  WritenDataVK2C23B(unsigned char Addr,unsigned char *Databuf,unsigned char Cnt)
{
	unsigned char n;
	
	//START�ź�	
	VK2C23B_I2CStart(); 									
	//SLAVE��ַ
	VK2C23B_I2CWriteByte(VK2C23B_ADDR); 	
	if( 1 == VK2C23B_I2CSlaveAck() )
	{
		VK2C23B_I2CStop();													
		return 0;										
	}
	//д��ʾRAM����
	VK2C23B_I2CWriteByte(VK2C23B_RWRAM); 						
	if( 1 == VK2C23B_I2CSlaveAck() )
	{
		VK2C23B_I2CStop();													
		return 0;
	}
	//��ʾRAM��ʼ��ַ
	VK2C23B_I2CWriteByte(Addr); 						
	if( 1 == VK2C23B_I2CSlaveAck() )
	{
		VK2C23B_I2CStop();													
		return 0;
	}
	//����Cnt�����ݵ���ʾRAM
	for(n=0;n<Cnt;n++)
	{ 
		VK2C23B_I2CWriteByte(*Databuf++);
		if( VK2C23B_I2CSlaveAck()==1 )
		{
			VK2C23B_I2CStop();													
			return 0;
		}
	}
	//STOP�ź�
	 VK2C23B_I2CStop();											
	 return 0;    
}
/*******************************************************************************
* Function Name  : ReadnData
* Description    : ����ʾRAM���������
* Input          : Addr-��ram����ʼ��ַ
*                : Databuf->��ram������bufferָ��
*                : Cnt->��ram�����ݸ���
* Output         : None
* Return         : 0-ok 1-fail
*******************************************************************************/
unsigned char  ReadnDataVK2C23B(unsigned char Addr,unsigned char *Databuf,unsigned char Cnt)
{
	unsigned char n;
	
	//START�ź�	
	VK2C23B_I2CStart(); 									
	//SLAVE��ַд
	VK2C23B_I2CWriteByte(VK2C23B_ADDR); 	
	if( 1 == VK2C23B_I2CSlaveAck() )
	{
		VK2C23B_I2CStop();													
		return 0;										
	}      
  //д��ʾRAM����
	VK2C23B_I2CWriteByte(VK2C23B_RWRAM); 						
	if( 1 == VK2C23B_I2CSlaveAck() )
	{
		VK2C23B_I2CStop();													
		return 0;
	}	
	//��ʾRAM��ʼ��ַ
	VK2C23B_I2CWriteByte(Addr); 						
	if( 1 == VK2C23B_I2CSlaveAck() )
	{
		VK2C23B_I2CStop();													
		return 0;
	}
	
	//STOP�ź�
	VK2C23B_I2CStop();	
	//START�ź�	
	VK2C23B_I2CStart(); 									
	//SLAVE��ַ��
	VK2C23B_I2CWriteByte(VK2C23B_ADDRRD); 	
		if( 1 == VK2C23B_I2CSlaveAck() )
	{
		VK2C23B_I2CStop();													
		return 0;										
	}  
	//��Cnt�����ݵ���ʾRAM
	for(n=0;n<Cnt-1;n++)
	{ 
		*Databuf++=VK2C23B_I2CRDDat();
		VK2C23B_I2CSendAck();
	}
	*Databuf++=VK2C23B_I2CRDDat();
	VK2C23B_I2CSendNAck();
	//STOP�ź�
	VK2C23B_I2CStop();											
	 return 0;    
}
/*******************************************************************************
* Function Name  : VK2C23B_DisAll
* Description    : ����SEG��ʾͬһ�����ݣ�bit7/bit3-COM3 bit6/bit2-COM2 bit5/bit1-COM1 bit4/bit0-COM0
* 					     : ���磺0xffȫ�� 0x00ȫ�� 0x55�������� 0xaa�������� 0x33�������� 
* Input          ��dat->д��ram������(1���ֽ����ݶ�Ӧ2��SEG)  
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
	WritenDataVK2C23B(0,VK2C23B_dispram,35);//������8bit���ݶ�Ӧ1��SEG��ÿ8bit���ݵ�ַ��1��ÿ8λ����1��ACK
}
/*******************************************************************************
* Function Name  : DisSegComOn
* Description    : ����1����(1��seg��1��com�����Ӧ����ʾ��)
* Input          ��seg->���Ӧ��seg��  
* 		           ��com->���Ӧcom��  
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_DisDotOn(unsigned char seg,unsigned char com)
{
	unsigned char addrbit,tempdat;
	
	addrbit=(1<<com);
	tempdat=VK2C23B_dispram[seg]|addrbit;
	VK2C23B_dispram[seg]=tempdat;
	WritenDataVK2C23B(seg,&tempdat,1);//������8λ���ݵ�4bit��Ч��ÿ8bit���ݵ�ַ��1��ÿ8λ����1��ACK
}
/*******************************************************************************
* Function Name  : DisSegComOff
* Description    : �ر�1����(1��seg��1��com�����Ӧ����ʾ��)
* Input          ��seg->���Ӧ��seg��  
* 		           ��com->���Ӧcom��  
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
* Description    : �������͹���ģʽ,��������ʾ
* Input          ��None 
* Output         : None
* Return         : 0-ok 1-fail
*******************************************************************************/
void VK2C23B_Enter_PowerOff(void)
{		
	WriteCmdVK2C23B(VK2C23B_SYSSET,SYSOFF_LCDOFF);
}
/*******************************************************************************
* Function Name  : Exit_Standby
* Description    : �˳�����͹���ģʽ
* Input          ��None 
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_Exit_PowerOff(void)
{	
	WriteCmdVK2C23B(VK2C23B_SYSSET,SYSON_LCDON);
}

/*******************************************************************************
* Function Name  : Lowlevel_Init
* Description    : ����ͨ����GPIO
* Input          ��None 
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_Lowlevel_Init(void)
{
	//ͨ���ߵ�ƽ��ͬ������ӵ�ƽת����·
	//�˺������ݿͻ���Ƭ������Ӧ���޸�	
	GPIO_SetMode(VK2C23B_SCL_PORT, VK2C23B_SCL_PIN, GPIO_MODE_OUTPUT);
	GPIO_SetMode(VK2C23B_SDA_PORT, VK2C23B_SDA_PIN, GPIO_MODE_QUASI);
		      
	VK2C23B_SCL_H();  
	VK2C23B_SDA_H(); 	
}
/*******************************************************************************
* Function Name  : Init
* Description    : ��ʼ������
* Input          ��None 
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_InitSequence(void)
{	
	//�ܽ����ø��ݿͻ���Ƭ������Ӧ���޸�
	VK2C23B_Lowlevel_Init();
	WriteCmdVK2C23B(VK2C23B_MODESET,CCOM_1_3__8); //ģʽ����  1/3 Bais 1/8 Duty
//	WriteCmdVK2C23B(VK2C23B_MODESET,CCOM_1_4__8); //ģʽ����  1/4 Bais 1/8 Duty
	VK2C23B_maxcom=8;
	WriteCmdVK2C23B(VK2C23B_SYSSET,SYSON_LCDON); 		//�ڲ�ϵͳ��������lcd����ʾ
	WriteCmdVK2C23B(VK2C23B_FRAMESET,FRAME_80HZ); 	//֡Ƶ��80Hz
//	WriteCmdVK2C23B(VK2C23B_FRAMESET,FRAME_160HZ);//֡Ƶ��160Hz
	WriteCmdVK2C23B(VK2C23B_BLINKSET,BLINK_OFF); 		//��˸�ر�	
//	WriteCmdVK2C23B(VK2C23B_BLINKSET,BLINK_2HZ); 		//��˸2HZ
//	WriteCmdVK2C23B(VK2C23B_BLINKSET,BLINK_1HZ); 		//��˸1HZ
//	WriteCmdVK2C23B(VK2C23B_BLINKSET,BLINK_0D5HZ); 	//��˸0.5HZ
	//SEG/VLCD���ý���ΪVLCD���ڲ���ѹ�������ܹر�,VLCD��VDD�̽�VR=0ƫ�õ�ѹ=VDD
//	WriteCmdVK2C23B(VK2C23B_IVASET,VLCDSEL_IVAOFF_R0); 
	//SEG/VLCD���ý���ΪVLCD���ڲ���ѹ�������ܹر�,VLCD��VDD���ӵ���VR>0ƫ�õ�ѹ=VLCD
//	WriteCmdVK2C23B(VK2C23B_IVASET,VLCDSEL_IVAOFF_R1); 
	//SEG/VLCD���ý���ΪSEG���ڲ�ƫ�õ�ѹ������1/3bias=0.652VDD 1/4bias=0.714VDD
	WriteCmdVK2C23B(VK2C23B_IVASET,SEGSEL_IVA00H);	
}
/*******************************************************************************
* Function Name  : Init
* Description    : ��ʼ������
* Input          ��None 
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_Init(void)
{	
	//�ܽ����ø��ݿͻ���Ƭ������Ӧ���޸�
	VK2C23B_Lowlevel_Init();
	//��ʼ��ʱ��
	VK2C23B_InitSequence();
}
/*******************************************************************************
* Function Name  : test_Main
* Description    : ����������
* Input          ��None 
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_Main(void)
{	
	VK2C23B_Init();
	VK2C23B_DisAll(0x00);
	while(1)
	{
		VK2C23B_DisAll(0xff);			//LCDȫ��
		delay_nms(3000);					//��ʱ1S
	
		VK2C23B_DisAll(0x00);			//LCDȫ��
		delay_nms(3000);					//��ʱ1S
		
		//LCD����
		VK2C23B_DisAll(0xAA);			
		ReadnDataVK2C23B(0,VK2C23B_readbuf,35);	//����RAM����
		delay_nms(1500);					//��ʱ
		VK2C23B_DisAll(0x55);			
		ReadnDataVK2C23B(0,VK2C23B_readbuf,35);	//����RAM����
		delay_nms(1500);					//��ʱ
		
		//����
		VK2C23B_DisAll(0x00);			//LCDȫ��		
		VK2C23B_Enter_PowerOff();	//�������ģʽ
		delay_nms(5000);					//��ʱ5S		
		VK2C23B_Exit_PowerOff();	//�˳�����ģʽ
		
		VK2C23B_DisAll(0x00);			//LCDȫ��
		delay_nms(500);					  //��ʱ
		for(VK2C23B_segi=0;VK2C23B_segi<VK2C23B_SEGNUM;VK2C23B_segi++)//seg
		{
			for(VK2C23B_comi=0;VK2C23B_comi<VK2C23B_maxcom;VK2C23B_comi++)//com
			{
					VK2C23B_DisDotOn(VK2C23B_segtab[VK2C23B_segi]-4,VK2C23B_comi);	//LCD�������
				delay_nms(300);				//��ʱ300mS
				VK2C23B_DisDotOff(VK2C23B_segtab[VK2C23B_segi]-4,VK2C23B_comi);	  //������ϵ����������ʵ�ֵ�����������δ���˳�����
			}
		}
		
		VK2C23B_DisAll(0xff);			//LCDȫ��
		delay_nms(500);					  //��ʱ
		for(VK2C23B_segi=0;VK2C23B_segi<VK2C23B_SEGNUM;VK2C23B_segi++)//seg
		{
			for(VK2C23B_comi=0;VK2C23B_comi<VK2C23B_maxcom;VK2C23B_comi++)//com
			{
					VK2C23B_DisDotOff(VK2C23B_segtab[VK2C23B_segi]-4,VK2C23B_comi);	//LCD����ر�
				delay_nms(300);				//��ʱ300mS
				VK2C23B_DisDotOn(VK2C23B_segtab[VK2C23B_segi]-4,VK2C23B_comi);  //������ϵ���رպ���ʵ�ֵ���رգ����δ���˳��ر�
			}
		}
		delay_nms(1000);					  //��ʱ
	}
}
/************************END OF FILE****/
