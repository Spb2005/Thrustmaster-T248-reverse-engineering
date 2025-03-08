/**
  ******************************************************************************
  * @file    main.h
  * @author  kevin_guo
  * @version V1.0.0
  * @date    10-09-2024
  * @brief   This file contains all files for main.
  ******************************************************************************
  * @attention
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "M451Series.h"	
#include "board.h"				

//#include "VK2C23B_IO_I2C_QUASI.h"			//单片机SDA脚设为双向IO
//#include "VK2C23B_IO_I2C_DIR.h"				//单片机SDA脚设为单向IO
#include "VK2C23B_IO_I2C.h"								//单片机硬件I2C

/* extern variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/	
/* Private variables ---------------------------------------------------------*/
typedef void (*I2C_FUNC)(unsigned int i2cstatus);
static I2C_FUNC i2c0handlerflag = NULL;
unsigned char i2cwrbuf[64];
unsigned char i2cwrbuf[64];
unsigned char i2crdtx[64];
unsigned char i2crdrx;
unsigned char rxdummy;
unsigned char txlen,txcnt;
unsigned char rxlen,rxcnt;
unsigned char endflag= 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/*  I2C0 IRQ Handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void I2C0_IRQHandler(void)
{
    unsigned int status;

    status = I2C_GET_STATUS(I2C0);
    if(I2C_GET_TIMEOUT_FLAG(I2C0))
    {
        /* Clear I2C0 Timeout Flag */
        I2C_ClearTimeoutFlag(I2C0);
    }
    else
    {
        if(i2c0handlerflag != NULL)
            i2c0handlerflag(status);
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C Rx Callback Function                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
void I2C_MasterRx(unsigned int rxstatus)
{
    if(rxstatus == 0x08)                       /* START has been transmitted and prepare SLA+W */
    {
        I2C_SET_DATA(I2C0, (VK2C23B_ADDR << 1));    /* Write SLA+W to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI);
    }
    else if(rxstatus == 0x18)                  /* SLA+W has been transmitted and ACK has been received */
    {
        I2C_SET_DATA(I2C0, i2crdtx[txcnt++]);
        I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI);
    }
    else if(rxstatus == 0x20)                  /* SLA+W has been transmitted and NACK has been received */
    {
        I2C_STOP(I2C0);
        I2C_START(I2C0);
    }
    else if(rxstatus == 0x28)                  /* DATA has been transmitted and ACK has been received */
    {
			I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STA_SI);  //数据·发送完SR START
    }
    else if(rxstatus == 0x10)                  /* Repeat START has been transmitted and prepare SLA+R */
    {
        I2C_SET_DATA(I2C0, ((VK2C23B_ADDR << 1) | 0x01));   /* Write SLA+R to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI);
    }
    else if(rxstatus == 0x40)                  /* SLA+R has been transmitted and ACK has been received */
    {
        I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI);
    }
    else if(rxstatus == 0x58)                  /* DATA has been received and NACK has been returned */
    {
			i2crdrx = (unsigned char) I2C_GET_DATA(I2C0);
			I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STO_SI); //发送STOP
			endflag = 1;
    }
    else
    {
        /* TO DO */
        //rxstatusis NOT processed
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C Tx Callback Function                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
void I2C_MasterTx(unsigned int txstatus)
{
    if(txstatus == 0x08)                       /* START has been transmitted */
    {
        I2C_SET_DATA(I2C0, VK2C23B_ADDR << 1);    /* Write SLA+W to Register I2CDAT */
        I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI);
    }
    else if(txstatus == 0x18)                  /* SLA+W has been transmitted and ACK has been received */
    {
        I2C_SET_DATA(I2C0, i2cwrbuf[txcnt++]);
        I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI);
    }
    else if(txstatus == 0x20)                  /* SLA+W has been transmitted and NACK has been received */
    {
        I2C_STOP(I2C0);
        I2C_START(I2C0);
    }
    else if(txstatus == 0x28)                  /* DATA has been transmitted and ACK has been received */
    {
        if(txcnt != txlen)
        {
            I2C_SET_DATA(I2C0, i2cwrbuf[txcnt++]);
            I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI);
        }
        else
        {
            I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STO_SI);
            endflag = 1;
        }
    }
    else
    {
       /* TO DO */
      //txstatus is NOT processed
    }
}
/*******************************************************************************
* Function Name  : VK2C23B_I2C_WRCmd
* Description    : i2c向寄存器发送命令
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_I2C_WRCmd(unsigned char Addr,unsigned char Cmd)
{
	i2cwrbuf[0] = Addr;
	i2cwrbuf[1] = Cmd;  
	txcnt = 0;
	txlen = 2;
	endflag = 0;

	/* I2C function to write data to slave */
	i2c0handlerflag = (I2C_FUNC)I2C_MasterTx;

	/* I2C as master sends START signal */
	I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STA);
	
	/* Wait I2C Tx Finish */
	while(endflag == 0);
	endflag = 0;
}
/*******************************************************************************
* Function Name  : VK2C23B_I2C_Dat
* Description    : i2c发送数据
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_I2C_WRDat(unsigned char Addr,unsigned char *Databuf,unsigned char cnt)
{
	unsigned char i;  
  i2cwrbuf[0] = VK2C23B_RWRAM;	
	i2cwrbuf[1] = Addr;
	for(i=0;i<cnt;i++)
	{
		i2cwrbuf[2+i] = *Databuf++;  
	}
	txcnt = 0;
	txlen=cnt+2; 
	endflag = 0;

	/* I2C function to write data to slave */
	i2c0handlerflag = (I2C_FUNC)I2C_MasterTx;

	/* I2C as master sends START signal */
	I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STA);

	/* Wait I2C Tx Finish */
	while(endflag == 0);
	endflag = 0;
}

/*******************************************************************************
* Function Name  : VK2C23B_I2C_RDDat
* Description    : i2c接收数据
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void VK2C23B_I2C_RDDat(unsigned char Addr,unsigned char *Databuf,unsigned char cnt)
{	
	unsigned char i,rxnum;
	
	rxnum=cnt;
	for(i=0;i<rxnum;i++)
	{
		//I2C发送地址
		i2crdtx[0]=VK2C23B_RWRAM; 
		i2crdtx[1]= (Addr&0x1f)+i;
		txcnt = 0;
		txlen=2;
		endflag = 0;
		//I2C接收数据
	/* I2C function to read data from slave */
		i2c0handlerflag = (I2C_FUNC)I2C_MasterRx;

		rxcnt = 0;
		rxlen=1;

		I2C_SET_CONTROL_REG(I2C0, I2C_CTL_STA);

		/* Wait I2C Rx Finish */
		while(endflag == 0);
		*Databuf++=i2cwrbuf[txlen+i]; //显示数据是低位先读
	}
}

/*******************************************************************************
* Function Name  : main
* Description    : 主程序
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main(void)
{
	/* Unlock protected registers */
	SYS_UnlockReg();
	SYS_Init();
	/* Lock protected registers */
	SYS_LockReg();
	
	//配置PD5(SCL),PD4(SDA)脚为硬件I2C,频率100khz
	/* Open I2C module and set bus clock */
	I2C_Open(I2C0, 30000);  //100khz
	/* Set I2C Slave Addresses */
	I2C_SetSlaveAddr(I2C0, 0, VK2C23B_ADDR, 0);   
	/* Enable I2C interrupt */
	I2C_EnableInt(I2C0);
	NVIC_EnableIRQ(I2C0_IRQn);
		
	VK2C23B_Main();
	
	while(1)
	{					
	}
}	
/************************END OF FILE****/

