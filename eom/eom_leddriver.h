/**
  ******************************************************************************
  * @file    eom_leddriver.h
  * @brief   Functions for controlling RGB led
  ******************************************************************************
 **/
#ifndef  EOM_LEDDRIVER_H
#define  EOM_LEDDRIVER_H

#include "eom.h"
#include "eom_softwaretimer.h"
#include <stm32f4xx.h>

#ifdef __cplusplus
 extern "C" {
#endif 


 typedef struct _EOMLedDriver
 {
 	GPIO_TypeDef * ledPort;
 	uint16_t redLedPin;
 	uint16_t blueLedPin;
 	uint16_t greenLedPin;
 	bool inverted;
 	EOM * program;
 	EOMSoftwareTimer redTimer;
 	EOMSoftwareTimer blueTimer;
 	EOMSoftwareTimer greenTimer;
 	uint8_t R;
 	uint8_t G;
 	uint8_t B;

 } EOMLedDriver;

 /**
   * @brief  Constructor of led driver. Initialize led GPIO and software timers
   * @param  ledDriver Structure to initialize
   * @param  program  Core program structure
   * @param  ledPort GPIO port for led (GPIOA, GPIOB .. )
   * @param  redLedPin Number of red channel output pin
   * @param  greenLedPin Number of green channel output pin
   * @param  blueLedPin Number of blue channel output pin
   * @param  inverted Flag of invertation output voltage level
   * @retval None
  **/
 void eom_leddriver_init(EOMLedDriver * ledDriver, EOM * program, GPIO_TypeDef * ledPort,
		                 uint16_t redLedPin, uint16_t greenLedPin, uint16_t blueLedPin, bool inverted );

 /**
   * @brief  Setting color for led
   * @param  ledDriver LedDriver structure
   * @param  R Red channel brightness [0..255]
   * @param  G Green channel brightness [0..255]
   * @param  B Blue channel brightness [0..255]
   * @retval None
  **/
  void eom_leddriver_set_color(EOMLedDriver * ledDriver, uint8_t R, uint8_t G, uint8_t B);



#ifdef __cplusplus
}
#endif

#endif

