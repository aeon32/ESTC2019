#include "main.h"
#include <stdint.h>
#include <stdlib.h>
#include <stm32f4xx_tim.h>

#define SWITCH_DELAY 50000

#define SWITCH_COUNTER_DELAY 500

#define BUTTON_PIN GPIO_Pin_0
#define BUTTON_PORT GPIOE
#define BUTTON_PORT_PERIPH RCC_AHB1Periph_GPIOE

#define LED_PORT GPIOA
#define LED_PORT_PERIPH RCC_AHB1Periph_GPIOA

#define RED_LED_GPIO GPIO_Pin_8
#define GREEN_LED_GPIO GPIO_Pin_9
#define BLUE_LED_GPIO GPIO_Pin_10

#define TIMER TIM2
#define TIMER_IRQ TIM2_IRQn
#define TIMER_PERIPH RCC_APB1Periph_TIM2

#define TIMER_PERIOD 60000000

void configurePLL()
{
	uint32_t PLLM = 4;
	uint32_t PLLN = 60;
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

}
;

/**
 *  Use this function to achieve clear clock state
 *  (HSI ready, HSE ready, PLL is configured and set as SYSCLK source)
 */
void sysclkReset()
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

	configurePLL();

	RCC->CFGR &= (uint32_t) ((uint32_t) ~(RCC_CFGR_SW));
	RCC->CFGR |= RCC_CFGR_SW_PLL;

	/* Wait till the main PLL is used as system clock source */
	while ((RCC->CFGR & (uint32_t) RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
		;

}
;

void setupGPIO()
{
	GPIO_InitTypeDef GPIO_InitStructure =
	{ 0 };

	/* Enable peripheral clock for LEDs port */
	RCC_AHB1PeriphClockCmd(LED_PORT_PERIPH, ENABLE);

	GPIO_InitStructure.GPIO_Pin = RED_LED_GPIO | GREEN_LED_GPIO | BLUE_LED_GPIO;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(LED_PORT, &GPIO_InitStructure);

	/* Enable peripheral clock for buttons port */
	RCC_AHB1PeriphClockCmd(BUTTON_PORT_PERIPH, ENABLE);

	GPIO_InitStructure.GPIO_Pin = BUTTON_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(BUTTON_PORT, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = EXTI_Line0;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStruct);

	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

}

void setupTimer()
{
	TIM_TimeBaseInitTypeDef tim_struct;
	RCC_APB1PeriphClockCmd(TIMER_PERIPH, ENABLE);
	tim_struct.TIM_Period = 999;       //period = 1000
	tim_struct.TIM_Prescaler = 29999;  //presc = 30000
	tim_struct.TIM_ClockDivision = 0;
	tim_struct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIMER, &tim_struct);

	TIM_ITConfig(TIMER, TIM_IT_Update, ENABLE);

	NVIC_InitTypeDef nvic_struct;
	nvic_struct.NVIC_IRQChannel = TIM2_IRQn;
	nvic_struct.NVIC_IRQChannelPreemptionPriority = 0;
	nvic_struct.NVIC_IRQChannelSubPriority = 1;
	nvic_struct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_struct);
}
;

typedef struct _LedSwitcher
{
	int8_t direction;
	int8_t current_led;

} LedSwitcher;

void ledswitcher_switch_led(LedSwitcher *switcher)
{
	const uint16_t LEDS[] =
	{ RED_LED_GPIO, GREEN_LED_GPIO, BLUE_LED_GPIO };
	const size_t LEDS_SIZE = sizeof(LEDS) / sizeof(LEDS[0]);

	GPIO_SetBits(LED_PORT, LEDS[switcher->current_led]);

	switcher->current_led += switcher->direction;
	if (switcher->current_led == LEDS_SIZE)
		switcher->current_led = 0;
	if (switcher->current_led < 0)
		switcher->current_led = LEDS_SIZE - 1;
	GPIO_ResetBits(LED_PORT, LEDS[switcher->current_led]);

}
;

void ledswitcher_change_direction(LedSwitcher *switcher)
{
	switcher->direction = -switcher->direction;

}
;

void ledswitcher_init(LedSwitcher *switcher)
{
	switcher->direction = 1;
	switcher->current_led = 0;

}

LedSwitcher ledSwitcher;


void TIM2_IRQHandler(void)
{

	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		/* Switch the LED off */
		ledswitcher_switch_led(&ledSwitcher);
	}
}
;

void EXTI0_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line0) != RESET)
	{

		EXTI_ClearITPendingBit(EXTI_Line0);
		int state = GPIO_ReadInputDataBit(BUTTON_PORT, BUTTON_PIN);
		if (state == 0)
		{
			ledswitcher_change_direction(&ledSwitcher);
		};
	}
}

int main(void)
{

	sysclkReset();

	ledswitcher_init(&ledSwitcher);

	setupGPIO();
	setupTimer();

	/* Turn all the leds off */

	GPIO_SetBits(LED_PORT, RED_LED_GPIO | GREEN_LED_GPIO | BLUE_LED_GPIO);


	TIM_Cmd(TIMER, ENABLE);

	while (1)
	{
	};

}

