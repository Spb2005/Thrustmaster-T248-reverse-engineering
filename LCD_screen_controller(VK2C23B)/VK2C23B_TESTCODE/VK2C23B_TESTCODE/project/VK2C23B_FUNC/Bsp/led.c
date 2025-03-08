/**
  ******************************************************************************
  * @file    led.h
  * @author  kevin_guo
  * @version V1.0.0
  * @date    09-06-2020
  * @brief   This file contains all files for led.
  ******************************************************************************
  * @attention
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "global.h"

/* extern variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private function ----------------------------------------------------------*/
/*******************************************************************************
* Function Name  : Led_Init
* Description    : ≥ı ºªØled
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Led_Init(void)
{    
	/* Configure PC11, PC12, PC13 as Output mode */
  GPIO_SetMode(PC, BIT11, GPIO_MODE_OUTPUT);
  GPIO_SetMode(PC, BIT12, GPIO_MODE_OUTPUT);
  GPIO_SetMode(PC, BIT13, GPIO_MODE_OUTPUT);
	
	Led_Off(LED_OK);
	Led_Off(LED_NG);
	Led_Off(LED_BUSY);
}
/*******************************************************************************
* Function Name  : Led_On
* Description    : LED ON
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Led_On(unsigned char ledn)
{
//	if(ledn == LED_OK)
//		PC11=0;
//	else 	if(ledn == LED_NG)
//		PC12=0;
//	else 	if(ledn == LED_BUSY)
//		PC13=0;
}
/*******************************************************************************
* Function Name  : Led_Off
* Description    : LED OFF
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Led_Off(unsigned char ledn)
{
//	if(ledn == LED_OK)
//		PC11=1;
//	else 	if(ledn == LED_NG)
//		PC12=1;
//	else 	if(ledn == LED_BUSY)
//		PC13=1;
}
/*******************************************************************************
* Function Name  : Led_Toggle
* Description    : Led_Toggle
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Led_Toggle(unsigned char ledn)
{
//	if(ledn ==LED_OK)
//		PC11=!PC11;
//	else 	if(ledn ==LED_NG)
//		PC12=!PC12;
//	else 	if(ledn ==LED_BUSY)
//		PC13=!PC13;
}

/************************END OF FILE****/
