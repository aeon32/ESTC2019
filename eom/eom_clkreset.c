#include "eom_clkreset.h"
#include <stm32f4xx.h>

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



void eom_clkreset(unsigned int freq)
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