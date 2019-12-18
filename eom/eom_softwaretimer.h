/**
  ******************************************************************************
  * @file    eom_softwaretimer.h
  * @brief   Software timer realization, based on systick interrupt
  ******************************************************************************
 **/

#ifndef  EOM_SOFTWARE_TIMER
#define  EOM_SOFTWARE_TIMER

#include "eom.h"


#ifdef __cplusplus
 extern "C" {
#endif 


 typedef void (*EOMSoftwareTimerCallBack) (EOMSoftwareTimer *, bool raiseEdge, void * userData);

 typedef struct _EOMSoftwareTimer
 {
	 EOMSoftwareTimer * nextTimer;   //single-linked list
	 EOM * program;
	 uint32_t aprValue;
	 uint32_t compareValue;
	 uint32_t currentValue;
	 uint32_t newCompareValue;
	 EOMSoftwareTimerCallBack callBack;
	 void * userData;

 } EOMSoftwareTimer;



 /**
   * @brief  Constructor of software timer. Initialize softwareTimer, adds it to list
   * @param  softwareTimer Structure to initialize
   * @param  program  Core program structure
   * @param  aprValue AutoReload counter value
   * @param  compareValue Value to determine duty cycle
   * @param  callback CallBack function, called on counter value overflow or on compareValue achievemnt
   * @param  userData Param for callback
   * @retval None
  **/
 void eom_software_timer_add(EOMSoftwareTimer * softwareTimer,EOM * program,
		                    uint32_t aprValue, uint32_t compareValue,
							EOMSoftwareTimerCallBack callBack, void * userData);
  /**
    * @brief  Detructor of software timer. Remove timer from list, free corresponding resources
    * @param  softwareTimer Structure to destroy
    * @retval None
   **/
 void eom_software_timer_remove(EOMSoftwareTimer * softwareTimer);

  /**
    * @brief  Function for setting new comparation (duty cycle) value
    * @param  softwareTimer Timer to manipulate
    * @param  newCompareValue New value of duty cycle
    * @retval None
    **/
 void eom_software_timer_set_compare_value(EOMSoftwareTimer * softwareTimer, uint32_t newCompareValue);

 /**
   * @brief  Function increments counter value and calls callback than overflow or compare value achieved
   * @param  softwareTimer Timer to manipulate
   * @retval None
   **/
 void eom_software_timer_increment_value(EOMSoftwareTimer * softwareTimer);

#ifdef __cplusplus
}
#endif

#endif

