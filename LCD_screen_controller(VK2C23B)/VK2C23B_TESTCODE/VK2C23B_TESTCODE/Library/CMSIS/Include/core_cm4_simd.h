/**************************************************************************//**
 * @file     core_cm4_simd.h
 * @brief    CMSIS Cortex-M4 SIMD Header File
 * @version  V3.01
 * @date     06. March 2012
 *
 * @note
 * Copyright (C) 2010-2012 ARM Limited. All rights reserved.
 *
 * @par
 * ARM Limited (ARM) is supplying this software for use with Cortex-M
 * processor based microcontrollers.  This file can be freely distributed
 * within development tools that are supporting such ARM based processors.
 *
 * @par
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ******************************************************************************/

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef __CORE_CM4_SIMD_H
#define __CORE_CM4_SIMD_H


/*******************************************************************************
 *                Hardware Abstraction Layer
 ******************************************************************************/


/* ###################  Compiler specific Intrinsics  ########################### */
/** \defgroup CMSIS_SIMD_intrinsics CMSIS SIMD Intrinsics
  Access to dedicated SIMD instructions
  @{
*/

#if   defined ( __CC_ARM ) /*------------------RealView Compiler -----------------*/
/* ARM armcc specific functions */

/*------ CM4 SIMD Intrinsics -----------------------------------------------------*/
#define __SADD8                           __sadd8
#define __QADD8                           __qadd8
#define __SHADD8                          __shadd8
#define __UADD8                           __uadd8
#define __UQADD8                          __uqadd8
#define __UHADD8                          __uhadd8
#define __SSUB8                           __ssub8
#define __QSUB8                           __qsub8
#define __SHSUB8                          __shsub8
#define __USUB8                           __usub8
#define __UQSUB8                          __uqsub8
#define __UHSUB8                          __uhsub8
#define __SADD16                          __sadd16
#define __QADD16                          __qadd16
#define __SHADD16                         __shadd16
#define __UADD16                          __uadd16
#define __UQADD16                         __uqadd16
#define __UHADD16                         __uhadd16
#define __SSUB16                          __ssub16
#define __QSUB16                          __qsub16
#define __SHSUB16                         __shsub16
#define __USUB16                          __usub16
#define __UQSUB16                         __uqsub16
#define __UHSUB16                         __uhsub16
#define __SASX                            __sasx
#define __QASX                            __qasx
#define __SHASX                           __shasx
#define __UASX                            __uasx
#define __UQASX                           __uqasx
#define __UHASX                           __uhasx
#define __SSAX                            __ssax
#define __QSAX                            __qsax
#define __SHSAX                           __shsax
#define __USAX                            __usax
#define __UQSAX                           __uqsax
#define __UHSAX                           __uhsax
#define __USAD8                           __usad8
#define __USADA8                          __usada8
#define __SSAT16                          __ssat16
#define __USAT16                          __usat16
#define __UXTB16                          __uxtb16
#define __UXTAB16                         __uxtab16
#define __SXTB16                          __sxtb16
#define __SXTAB16                         __sxtab16
#define __SMUAD                           __smuad
#define __SMUADX                          __smuadx
#define __SMLAD                           __smlad
#define __SMLADX                          __smladx
#define __SMLALD                          __smlald
#define __SMLALDX                         __smlaldx
#define __SMUSD                           __smusd
#define __SMUSDX                          __smusdx
#define __SMLSD                           __smlsd
#define __SMLSDX                          __smlsdx
#define __SMLSLD                          __smlsld
#define __SMLSLDX                         __smlsldx
#define __SEL                             __sel
#define __QADD                            __qadd
#define __QSUB                            __qsub

#define __PKHBT(ARG1,ARG2,ARG3)          ( ((((unsigned int)(ARG1))          ) & 0x0000FFFFUL) |  \
                                           ((((unsigned int)(ARG2)) << (ARG3)) & 0xFFFF0000UL)  )

#define __PKHTB(ARG1,ARG2,ARG3)          ( ((((unsigned int)(ARG1))          ) & 0xFFFF0000UL) |  \
                                           ((((unsigned int)(ARG2)) >> (ARG3)) & 0x0000FFFFUL)  )


/*-- End CM4 SIMD Intrinsics -----------------------------------------------------*/



#elif defined ( __ICCARM__ ) /*------------------ ICC Compiler -------------------*/
/* IAR iccarm specific functions */

/*------ CM4 SIMD Intrinsics -----------------------------------------------------*/
#include <cmsis_iar.h>

/*-- End CM4 SIMD Intrinsics -----------------------------------------------------*/



#elif defined ( __TMS470__ ) /*---------------- TI CCS Compiler ------------------*/
/* TI CCS specific functions */

/*------ CM4 SIMD Intrinsics -----------------------------------------------------*/
#include <cmsis_ccs.h>

/*-- End CM4 SIMD Intrinsics -----------------------------------------------------*/



#elif defined ( __GNUC__ ) /*------------------ GNU Compiler ---------------------*/
/* GNU gcc specific functions */

/*------ CM4 SIMD Intrinsics -----------------------------------------------------*/
__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SADD8(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("sadd8 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __QADD8(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("qadd8 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SHADD8(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("shadd8 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __UADD8(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("uadd8 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __UQADD8(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("uqadd8 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __UHADD8(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("uhadd8 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}


__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SSUB8(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("ssub8 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __QSUB8(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("qsub8 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SHSUB8(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("shsub8 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __USUB8(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("usub8 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __UQSUB8(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("uqsub8 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __UHSUB8(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("uhsub8 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}


__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SADD16(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("sadd16 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __QADD16(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("qadd16 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SHADD16(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("shadd16 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __UADD16(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("uadd16 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __UQADD16(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("uqadd16 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __UHADD16(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("uhadd16 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SSUB16(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("ssub16 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __QSUB16(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("qsub16 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SHSUB16(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("shsub16 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __USUB16(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("usub16 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __UQSUB16(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("uqsub16 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __UHSUB16(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("uhsub16 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SASX(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("sasx %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __QASX(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("qasx %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SHASX(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("shasx %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __UASX(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("uasx %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __UQASX(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("uqasx %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __UHASX(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("uhasx %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SSAX(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("ssax %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __QSAX(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("qsax %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SHSAX(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("shsax %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __USAX(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("usax %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __UQSAX(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("uqsax %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __UHSAX(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("uhsax %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __USAD8(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("usad8 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __USADA8(unsigned int op1, unsigned int op2, unsigned int op3)
{
  unsigned int result;

  __ASM volatile ("usada8 %0, %1, %2, %3" : "=r" (result) : "r" (op1), "r" (op2), "r" (op3) );
  return(result);
}

#define __SSAT16(ARG1,ARG2) \
({                          \
  unsigned int __RES, __ARG1 = (ARG1); \
  __ASM ("ssat16 %0, %1, %2" : "=r" (__RES) :  "I" (ARG2), "r" (__ARG1) ); \
  __RES; \
 })

#define __USAT16(ARG1,ARG2) \
({                          \
  unsigned int __RES, __ARG1 = (ARG1); \
  __ASM ("usat16 %0, %1, %2" : "=r" (__RES) :  "I" (ARG2), "r" (__ARG1) ); \
  __RES; \
 })

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __UXTB16(unsigned int op1)
{
  unsigned int result;

  __ASM volatile ("uxtb16 %0, %1" : "=r" (result) : "r" (op1));
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __UXTAB16(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("uxtab16 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SXTB16(unsigned int op1)
{
  unsigned int result;

  __ASM volatile ("sxtb16 %0, %1" : "=r" (result) : "r" (op1));
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SXTAB16(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("sxtab16 %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SMUAD  (unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("smuad %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SMUADX (unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("smuadx %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SMLAD (unsigned int op1, unsigned int op2, unsigned int op3)
{
  unsigned int result;

  __ASM volatile ("smlad %0, %1, %2, %3" : "=r" (result) : "r" (op1), "r" (op2), "r" (op3) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SMLADX (unsigned int op1, unsigned int op2, unsigned int op3)
{
  unsigned int result;

  __ASM volatile ("smladx %0, %1, %2, %3" : "=r" (result) : "r" (op1), "r" (op2), "r" (op3) );
  return(result);
}

#define __SMLALD(ARG1,ARG2,ARG3) \
({ \
  unsigned int __ARG1 = (ARG1), __ARG2 = (ARG2), __ARG3_H = (unsigned int)((uint64_t)(ARG3) >> 32), __ARG3_L = (unsigned int)((uint64_t)(ARG3) & 0xFFFFFFFFUL); \
  __ASM volatile ("smlald %0, %1, %2, %3" : "=r" (__ARG3_L), "=r" (__ARG3_H) : "r" (__ARG1), "r" (__ARG2), "0" (__ARG3_L), "1" (__ARG3_H) ); \
  (uint64_t)(((uint64_t)__ARG3_H << 32) | __ARG3_L); \
 })

#define __SMLALDX(ARG1,ARG2,ARG3) \
({ \
  unsigned int __ARG1 = (ARG1), __ARG2 = (ARG2), __ARG3_H = (unsigned int)((uint64_t)(ARG3) >> 32), __ARG3_L = (unsigned int)((uint64_t)(ARG3) & 0xFFFFFFFFUL); \
  __ASM volatile ("smlaldx %0, %1, %2, %3" : "=r" (__ARG3_L), "=r" (__ARG3_H) : "r" (__ARG1), "r" (__ARG2), "0" (__ARG3_L), "1" (__ARG3_H) ); \
  (uint64_t)(((uint64_t)__ARG3_H << 32) | __ARG3_L); \
 })

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SMUSD  (unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("smusd %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SMUSDX (unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("smusdx %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SMLSD (unsigned int op1, unsigned int op2, unsigned int op3)
{
  unsigned int result;

  __ASM volatile ("smlsd %0, %1, %2, %3" : "=r" (result) : "r" (op1), "r" (op2), "r" (op3) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SMLSDX (unsigned int op1, unsigned int op2, unsigned int op3)
{
  unsigned int result;

  __ASM volatile ("smlsdx %0, %1, %2, %3" : "=r" (result) : "r" (op1), "r" (op2), "r" (op3) );
  return(result);
}

#define __SMLSLD(ARG1,ARG2,ARG3) \
({ \
  unsigned int __ARG1 = (ARG1), __ARG2 = (ARG2), __ARG3_H = (unsigned int)((ARG3) >> 32), __ARG3_L = (unsigned int)((ARG3) & 0xFFFFFFFFUL); \
  __ASM volatile ("smlsld %0, %1, %2, %3" : "=r" (__ARG3_L), "=r" (__ARG3_H) : "r" (__ARG1), "r" (__ARG2), "0" (__ARG3_L), "1" (__ARG3_H) ); \
  (uint64_t)(((uint64_t)__ARG3_H << 32) | __ARG3_L); \
 })

#define __SMLSLDX(ARG1,ARG2,ARG3) \
({ \
  unsigned int __ARG1 = (ARG1), __ARG2 = (ARG2), __ARG3_H = (unsigned int)((ARG3) >> 32), __ARG3_L = (unsigned int)((ARG3) & 0xFFFFFFFFUL); \
  __ASM volatile ("smlsldx %0, %1, %2, %3" : "=r" (__ARG3_L), "=r" (__ARG3_H) : "r" (__ARG1), "r" (__ARG2), "0" (__ARG3_L), "1" (__ARG3_H) ); \
  (uint64_t)(((uint64_t)__ARG3_H << 32) | __ARG3_L); \
 })

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __SEL  (unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("sel %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __QADD(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("qadd %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

__attribute__( ( always_inline ) ) __STATIC_INLINE unsigned int __QSUB(unsigned int op1, unsigned int op2)
{
  unsigned int result;

  __ASM volatile ("qsub %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

#define __PKHBT(ARG1,ARG2,ARG3) \
({                          \
  unsigned int __RES, __ARG1 = (ARG1), __ARG2 = (ARG2); \
  __ASM ("pkhbt %0, %1, %2, lsl %3" : "=r" (__RES) :  "r" (__ARG1), "r" (__ARG2), "I" (ARG3)  ); \
  __RES; \
 })

#define __PKHTB(ARG1,ARG2,ARG3) \
({                          \
  unsigned int __RES, __ARG1 = (ARG1), __ARG2 = (ARG2); \
  if (ARG3 == 0) \
    __ASM ("pkhtb %0, %1, %2" : "=r" (__RES) :  "r" (__ARG1), "r" (__ARG2)  ); \
  else \
    __ASM ("pkhtb %0, %1, %2, asr %3" : "=r" (__RES) :  "r" (__ARG1), "r" (__ARG2), "I" (ARG3)  ); \
  __RES; \
 })

/*-- End CM4 SIMD Intrinsics -----------------------------------------------------*/



#elif defined ( __TASKING__ ) /*------------------ TASKING Compiler --------------*/
/* TASKING carm specific functions */


/*------ CM4 SIMD Intrinsics -----------------------------------------------------*/
/* not yet supported */
/*-- End CM4 SIMD Intrinsics -----------------------------------------------------*/


#endif

/*@} end of group CMSIS_SIMD_intrinsics */


#endif /* __CORE_CM4_SIMD_H */

#ifdef __cplusplus
}
#endif
