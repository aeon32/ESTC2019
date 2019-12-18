/**
  ******************************************************************************
  * @file    eom_antichatter.h
  * @brief   This file provides realization of antichatter algorithm
  ******************************************************************************
 **/ 

#ifndef  EOM_ANTICHATTER_H
#define  EOM_ANTICHATTER_H

#include <stdint.h>
#include <stdbool.h>
#include <stm32f4xx.h>

#include "eom.h"

#ifdef __cplusplus
 extern "C" {
#endif 

typedef struct _EOMAntiChatter
{
	uint32_t switchTime;
	bool switchedOn;
	bool triggered;
	EOM * program;
	GPIO_TypeDef * buttonPort;
	uint16_t buttonPin;
	uint16_t hysterTime;
	bool inverted;
	uint32_t lastTime;

} EOMAntiChatter;


/**
  * @brief  EOMAntichatter struct initializer
  * @param  program - eom core program structure
  * @param  antichatter - structure to initialize
  * @param  buttonPort - GPIO input port
  * @param  buttonPin - GPIO input pin
  * @param  hysterTime - hysteresis time, ms
  * @param  invertFlag - invert polarity
  * @retval None
 **/
void eom_antichatter_init(EOMAntiChatter * antichatter, EOM * program, GPIO_TypeDef * buttonPort, uint16_t buttonPin, uint16_t hysterTime, bool inverted);

/**
 * @brief  process antichatter
 * Call this function in loop.
 * @retval Returns true if button state has been changed. Actual state of button saved in switched_on flag
*/
bool eom_antichatter_process(EOMAntiChatter * antichatter);

#ifdef __cplusplus
}
#endif

#endif

