#include "eom.h"
#include "eom_softwaretimer.h"

#include <string.h>

#include <stm32f4xx.h>

EOM * programGlobal = 0;

static void configure_pll(unsigned int freq)
{
	uint32_t PLLM = 4;
	uint32_t PLLN = freq;
	uint32_t PLLP = 0;  // divider==2 for this configuration
	// SYSCLK = (HSE or HSI ) / PLLM * PLLN / PLLP

	/* Disable the main PLL */
	RCC->CR &= (uint32_t) ((uint32_t) ~(RCC_CR_PLLON));
	/* Wait till the main PLL is ready */
	while ((RCC->CR & RCC_CR_PLLRDY) == 1)
	{

	};

	RCC->PLLCFGR = PLLM | (PLLN << 6) | PLLP << 16;
	//use HSE as source for PLL
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE;

	/* Enable the main PLL */
	RCC->CR |= RCC_CR_PLLON;

	/* Wait till the main PLL is ready */
	while ((RCC->CR & RCC_CR_PLLRDY) == 0)
	{

	};

};



static void clkreset(unsigned int freq)
{
	uint32_t HSIStatus = 0;
	uint32_t HSEStatus = 0;

	/* Enable HSE */
	RCC->CR |= ((uint32_t) RCC_CR_HSEON);

	/* Wait till HSE is ready */
	do
	{
		HSEStatus = RCC->CR & RCC_CR_HSERDY;
	} while (HSEStatus == 0);

	/* Select the main HSE as system clock source */
	/*(we must do it to have ability to reconfigure PLL */
	RCC->CFGR &= (uint32_t) ((uint32_t) ~(RCC_CFGR_SW));
	RCC->CFGR |= RCC_CFGR_SW_HSE;

	/* Wait till the main HSE is used as system clock source */
	while ((RCC->CFGR & (uint32_t) RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE)
	{
	}

	/* Enable HSI */
	RCC->CR |= ((uint32_t) RCC_CR_HSION);
	/* Wait till HSI is ready */
	do
	{
		HSIStatus = RCC->CR & RCC_CR_HSIRDY;
	} while (HSIStatus == 0);

	configure_pll(freq);

	RCC->CFGR &= (uint32_t) ((uint32_t) ~(RCC_CFGR_SW));
	RCC->CFGR |= RCC_CFGR_SW_PLL;

	/* Wait till the main PLL is used as system clock source */
	while ((RCC->CFGR & (uint32_t) RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
		;

}


static void processTimers(EOM * program)
{

	//if (__LDREXW(&program->timerListsLocked) != 0)
	//	return;  //list changing in progress

	if (program->timerListsLocked )
		return;

	EOMSoftwareTimer * curr = program->timerListHead;
	while (curr)
	{
		eom_software_timer_increment_value(curr);
		curr = curr->nextTimer;

	};

};

void SysTick_Handler()
{
	++programGlobal->timeCounter;
	processTimers(programGlobal);
};


void eom_init(EOM * program, uint8_t freq)
{
   assert_param(freq > 0 && freq<=168);
   assert_param(programGlobal == 0);
   memset(program, 0, sizeof(EOM));
   clkreset(freq);
   
   program->initialized = true;
   program->freq = freq;
   program->timerListHead = 0;
   program->timerListsLocked = 0;
   programGlobal = program;

   uint32_t ticks = freq * 1000000 / 1000 / 8; //every 0.125ms
   SysTick_Config(ticks);
};

EOM * eom_get_program(void)
{
	return programGlobal;
};



void eom_timers_lock(EOM * program)
{
    //__DMB(); // Ensure memory operations completed before
	program->timerListsLocked = 1;

};



void eom_timers_unlock(EOM * program)
{
	program->timerListsLocked = 0;
};


