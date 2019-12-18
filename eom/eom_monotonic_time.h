/**
  ******************************************************************************
  * @file    eom_monotonic_time.h
  * @brief   This file provides monotonic time functions
  ******************************************************************************
 **/


#ifndef  EOM_MONOTONIC_TIME_H
#define  EOM_MONOTONIC_TIME_H

#include "eom.h"

#ifdef __cplusplus
 extern "C" {
#endif 

 /**
   *  @brief Function returns monotonic time
   *  @retval Time in ms since CPU has been started
  **/

uint32_t eom_monotonic_time(EOM * program);

/**
  *  @brief Function returns elapsed time with potential overlflow considerations
  *  @param currentTime current time value
  *  @param lastTime previous time value
  *  @retval elapsed time. Just difference in simple cases.
 **/
uint32_t eom_monotonic_time_elapsed(uint32_t currentTime, uint32_t lastTime);

#ifdef __cplusplus
}
#endif

#endif

