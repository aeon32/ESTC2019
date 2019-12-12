#ifndef  EOM_CLKRESET_H
#define  EOM_CLKRESET_H

#include "eom.h"


#ifdef __cplusplus
 extern "C" {
#endif 


/**
 *  Use this function to achieve clear clock state
 *  (HSI ready, HSE ready, PLL is configured and set as SYSCLK source)
 *  
 */
void eom_clkreset(EOM * program, unsigned int freq);


#ifdef __cplusplus
}
#endif

#endif

