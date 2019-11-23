#include "main.h"
#include <stdint.h>
#include <stdlib.h>

#define SWITCH_DELAY 50000

#define SWITCH_COUNTER_DELAY 500

#define BUTTON_PIN GPIO_Pin_0
#define BUTTON_PORT GPIOA
#define BUTTON_PORT_PERIPH RCC_AHB1Periph_GPIOA

#define LED_PORT GPIOD
#define LED_PORT_PERIPH RCC_AHB1Periph_GPIOD

#define RED_LED_GPIO GPIO_Pin_13
#define GREEN_LED_GPIO GPIO_Pin_14
#define BLUE_LED_GPIO GPIO_Pin_15


void configurePLL()
{
	uint32_t PLLM = 4;
	uint32_t PLLN = 84;
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
void clockReset()
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
	while ((RCC->CFGR & (uint32_t) RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);




};


void switchClockSource()
{
	uint32_t newClockSourceValue = RCC_CFGR_SW_PLL;
	uint32_t newClockSourceControlValue = RCC_CFGR_SWS_PLL;
	if (((RCC->CFGR & (uint32_t) RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL) )
	{
		newClockSourceValue = RCC_CFGR_SW_HSI;
		newClockSourceControlValue = RCC_CFGR_SWS_HSI;
	};




	RCC->CFGR &= (uint32_t) ((uint32_t) ~(RCC_CFGR_SW));
	RCC->CFGR |= newClockSourceValue;

	while ((RCC->CFGR & (uint32_t) RCC_CFGR_SWS) != newClockSourceControlValue);



};


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

}

typedef struct _ButtonState
{
	uint32_t switch_counter;
	uint8_t switched_on;
	uint8_t triggered;

} ButtonState;

void processButtonState(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
		ButtonState *buttonState)
{
	//we use counter to avoid contact chatter
	int state = GPIO_ReadInputDataBit(GPIOx, GPIO_Pin);
	if (state == 0)
	{
		buttonState->switch_counter++;
		if (buttonState->switch_counter > SWITCH_COUNTER_DELAY)
			buttonState->switch_counter = SWITCH_COUNTER_DELAY;
	}
	else
	{
		if (buttonState->switch_counter > 0)
			buttonState->switch_counter--;
	};

	buttonState->triggered = 0;
	if (!buttonState->switched_on
			&& buttonState->switch_counter == SWITCH_COUNTER_DELAY)
	{
		buttonState->switched_on = 1;
		buttonState->triggered = 1;

	}
	else if (buttonState->switched_on && buttonState->switch_counter == 0)
	{
		buttonState->switched_on = 0;
		buttonState->triggered = 1;

	};

}
;

int main(void)
{

	clockReset();
	setupGPIO();


	/* LEDs array to toggle between them */
	/* LED to toggle during iteration */
	int8_t current_led = 0;

	const uint16_t LEDS[] =
	{ RED_LED_GPIO, GREEN_LED_GPIO, BLUE_LED_GPIO };
	const size_t LEDS_SIZE = sizeof(LEDS) / sizeof(LEDS[0]);

	/* Turn all the leds off */

	GPIO_ResetBits(LED_PORT, RED_LED_GPIO | GREEN_LED_GPIO | BLUE_LED_GPIO);
	ButtonState buttonState =
	{ 0 };

	int8_t direction = 1;

	while (1)
	{

		int i;
		/* Switch the LED on */
		GPIO_SetBits(LED_PORT, LEDS[current_led]);

		for (i = 0; i < SWITCH_DELAY; i++)
		{

			processButtonState(BUTTON_PORT, BUTTON_PIN, &buttonState);
			if (buttonState.triggered && buttonState.switched_on)
			{
				direction = -direction;
				switchClockSource();

			};
		};

		/* Switch the LED off */
		GPIO_ResetBits(LED_PORT, LEDS[current_led]);

		current_led += direction;
		if (current_led == LEDS_SIZE)
			current_led = 0;
		if (current_led < 0)
			current_led = LEDS_SIZE - 1;
	}
}

