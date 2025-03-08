/**
  ******************************************************************************
  * @file    led.h
  * @author  kevin_guo
  * @version V1.0.0
  * @date    09-06-2020
  * @brief   This file contains definitions for led.
  ******************************************************************************
  * @attention
  ******************************************************************************  
  */
#ifndef __LED_H__
#define __LED_H__

#include "M451Series.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/ 
#define LED_OK 		1
#define LED_NG 		2
#define LED_BUSY	3
/* Exported functions ------------------------------------------------------- */
void Led_Init(void);
void Led_On(unsigned char ledn);
void Led_Off(unsigned char ledn);
void Led_Toggle(unsigned char ledn);
#endif  /*__LED_H__*/

/************************END OF FILE****/
