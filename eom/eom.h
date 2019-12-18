/**
  ******************************************************************************
  * @file    eom.h
  * @brief   This file provides basic program initialization function
  ******************************************************************************
 **/ 

#ifndef  EOM_H
#define  EOM_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
 extern "C" {
#endif 

 typedef struct _EOMSoftwareTimer EOMSoftwareTimer;


typedef struct _EOM 
{

	bool initialized;
	uint8_t freq;
	uint32_t timeCounter;
    EOMSoftwareTimer * timerListHead;
    volatile int timerListsLocked;

} EOM;


/**
  * @brief  Use this function to achieve clear clock state (HSI ready, HSE ready, PLL is configured and set as SYSCLK source)
  * @param  eomStruct program to the core program structure
  * @param  freq  Required SysCLK freq (in Mhz)
  * @retval None
 **/
void eom_init(EOM * program, uint8_t freq);


/**
  * @brief  Returns value of EOM struct global instance
  * @retval Global instance of EOM struct
 **/
EOM * eom_get_program();

/**
  * @brief  Lock mutex for timers list.
  * Must be called before any manipulation with timers list to prevent race in interrupt
  * @retval None
 **/
void eom_timers_lock(EOM * program);


/**
  * @brief  Unlock mutex for timers list
  * @retval None
 **/
void eom_timers_lock(EOM * program);

#ifdef __cplusplus
}
#endif

#endif

