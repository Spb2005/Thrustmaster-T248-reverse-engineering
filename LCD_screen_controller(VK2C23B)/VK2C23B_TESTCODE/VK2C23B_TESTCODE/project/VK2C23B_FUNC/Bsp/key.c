/**
  ******************************************************************************
  * @file    key.h
  * @author  kevin_guo
  * @version V1.0.0
  * @date    09-06-2020
  * @brief   This file contains all files for key.
  ******************************************************************************
  * @attention
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "global.h"

/* extern variables ----------------------------------------------------------*/
extern unsigned char getpara_flag;
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
unsigned char key_val;
/* Private function prototypes -----------------------------------------------*/
/* Private function ----------------------------------------------------------*/
/*******************************************************************************
* Function Name  : Key_Init
* Description    : 初始化key
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Key_Init(void)
{    
	/* Configure PC9,PC10 as Input mode */
  GPIO_SetMode(PC, BIT9, GPIO_MODE_QUASI);
  GPIO_SetMode(PC, BIT10, GPIO_MODE_QUASI);
}
/*******************************************************************************
* Function Name  : Get_keyval
* Description    : 读取键值
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Get_Keyval(void)
{
	if(PC9 == 0)
	{
		key_val=KEY_START;
	}
	else 	if(PC10 == 0)
		key_val=KEY_SETSEL;
}

/************************END OF FILE****/
