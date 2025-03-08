/**
  ******************************************************************************
  * @file    board.c
  * @author  kevin_guo
  * @version V1.1
  * @date    07-05-2024
  * @brief   This file contains all files for board.
  ******************************************************************************
  * @attention
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "M451Series.h"
#include "board.h"	

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define PLL_CLOCK           72000000
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private function ----------------------------------------------------------*/
/*******************************************************************************
* Function Name  : SYS_Init
* Description    : ³õÊ¼»¯clock
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SYS_Init(void)
{
	/*---------------------------------------------------------------------------------------------------------*/
	/* Init System Clock                                                                                       */
	/*---------------------------------------------------------------------------------------------------------*/

	/* Enable HIRC clock (Internal RC 22.1184MHz) */
	CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

	/* Wait for HIRC clock ready */
	CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

	/* Select HCLK clock source as HIRC and and HCLK source divider as 1 */
	CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

	/* Enable HXT clock (external XTAL 12MHz) */
	CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);

	/* Wait for HXT clock ready */
	CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

	/* Set core clock as PLL_CLOCK from PLL */
	CLK_SetCoreClock(PLL_CLOCK);

  /* Enable PWM1 module clock */
  CLK_EnableModuleClock(PWM1_MODULE);
	
    /* Enable I2C0 module clock */
    CLK_EnableModuleClock(I2C0_MODULE);
	/*---------------------------------------------------------------------------------------------------------*/
	/* Init I/O Multi-function                                                                                 */
	/*---------------------------------------------------------------------------------------------------------*/
	/* PWM clock frequency is set equal to HCLK: select PWM module clock source as PCLK */
	CLK_SetModuleClock(PWM1_MODULE, CLK_CLKSEL2_PWM1SEL_PCLK1, NULL);

	/* Reset PWM1 module */
	SYS_ResetModule(PWM1_RST);
	
	/* Set PC15 multi-function pins for PWM1 Channel0 */
	SYS->GPC_MFPH = (SYS->GPC_MFPH & (~SYS_GPC_MFPH_PC15MFP_Msk));
	SYS->GPC_MFPH |= SYS_GPC_MFPH_PC15MFP_PWM1_CH0;		
	
	/* Set I2C PD multi-function pins */
	SYS->GPD_MFPL &= ~(SYS_GPD_MFPL_PD4MFP_Msk | SYS_GPD_MFPL_PD5MFP_Msk);
	SYS->GPD_MFPL |= (SYS_GPD_MFPL_PD4MFP_I2C0_SDA | SYS_GPD_MFPL_PD5MFP_I2C0_SCL);

	/* I2C pins enable schmitt trigger */
	PD->SMTEN |= (GPIO_SMTEN_SMTEN2_Msk | GPIO_SMTEN_SMTEN3_Msk);
}

/************************END OF FILE****/
