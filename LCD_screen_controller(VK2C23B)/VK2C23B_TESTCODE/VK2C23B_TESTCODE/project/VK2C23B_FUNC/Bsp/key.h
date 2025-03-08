/**
  ******************************************************************************
  * @file    key.h
  * @author  kevin_guo
  * @version V1.0.0
  * @date    09-06-2020
  * @brief   This file contains definitions for key.
  ******************************************************************************
  * @attention
  ******************************************************************************  
  */
#ifndef __KEY_H__
#define __KEY_H__

#include "M451Series.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/ 
#define KEY_NONE		0
#define KEY_START		1
#define KEY_SETSEL	2
/* Exported functions ------------------------------------------------------- */
void Key_Init(void);
void Get_Keyval(void);
#endif  /*__KEY_H__*/

/************************END OF FILE****/
