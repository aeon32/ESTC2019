#include "main.h"
#include <stdint.h>
#include <stdlib.h>
#include <stm32f4xx_tim.h>

#include <eom.h>
#include <eom_clkreset.h>

#define SYSCLK_FREQ 60

#define SWITCH_DELAY 50000

#define SWITCH_COUNTER_DELAY 500

#define BRIGHT_BUTTON_PIN GPIO_Pin_0
#define SWITCH_BUTTON_PIN GPIO_Pin_1
#define BUTTON_PORT GPIOE
#define BUTTON_PORT_PERIPH RCC_AHB1Periph_GPIOE

#define LED_PORT GPIOA
#define LED_PORT_PERIPH RCC_AHB1Periph_GPIOA

#define RED_LED_GPIO GPIO_Pin_8
#define GREEN_LED_GPIO GPIO_Pin_9
#define BLUE_LED_GPIO GPIO_Pin_10

#define RED_LED_AF_SOURCE GPIO_PinSource8
#define GREEN_LED_AF_SOURCE GPIO_PinSource9
#define BLUE_LED_AF_SOURCE GPIO_PinSource10


#define TIMER TIM1
#define TIMER_PERIPH RCC_APB2Periph_TIM1

#define TIMER_PERIOD 60000000

#define TIMER_APR 999

#define BRIGHT_STEPS 6




void setupGPIO()
{
	GPIO_InitTypeDef GPIO_InitStructure =
	{ 0 };

	/* Enable peripheral clock for LEDs port */
	RCC_AHB1PeriphClockCmd(LED_PORT_PERIPH, ENABLE);

    GPIO_PinAFConfig(LED_PORT, RED_LED_AF_SOURCE , GPIO_AF_TIM1);
    GPIO_PinAFConfig(LED_PORT, GREEN_LED_AF_SOURCE, GPIO_AF_TIM1);
    GPIO_PinAFConfig(LED_PORT, BLUE_LED_AF_SOURCE, GPIO_AF_TIM1);


	GPIO_InitStructure.GPIO_Pin = RED_LED_GPIO | GREEN_LED_GPIO | BLUE_LED_GPIO;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(LED_PORT, &GPIO_InitStructure);


	/* Enable peripheral clock for buttons port */
	RCC_AHB1PeriphClockCmd(BUTTON_PORT_PERIPH, ENABLE);

	GPIO_InitStructure.GPIO_Pin = BRIGHT_BUTTON_PIN | SWITCH_BUTTON_PIN ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(BUTTON_PORT, &GPIO_InitStructure);

}

void setupTimer()
{
	//TIM_CLK = 2 * SYSCLK / AHB_PRESC / APB2_PRESC
	// = 2 * 60Mhz / 1 /2 = 60 Mhz



	RCC_APB2PeriphClockCmd(TIMER_PERIPH, ENABLE);
	TIM_TimeBaseInitTypeDef tim_struct;

	tim_struct.TIM_Period = TIMER_APR ;
	//tim_struct.TIM_Prescaler = 2999;  //20hz
	//tim_struct.TIM_Prescaler = 1199;  //50hz
	tim_struct.TIM_Prescaler = 599;  //10hz
	tim_struct.TIM_CounterMode = TIM_CounterMode_Up;

    tim_struct.TIM_ClockDivision     = TIM_CKD_DIV1;
    tim_struct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIMER, &tim_struct);
};

void setupPWM()
{
    TIM_OCInitTypeDef tim_oc_init;

    TIM_OCStructInit(&tim_oc_init);

    tim_oc_init.TIM_OCMode      = TIM_OCMode_PWM1;
    tim_oc_init.TIM_Pulse       = 0;
    tim_oc_init.TIM_OutputState = TIM_OutputState_Enable;
    tim_oc_init.TIM_OCPolarity  = TIM_OCPolarity_Low;


    TIM_OC1Init(TIMER, &tim_oc_init);
    TIM_OC2Init(TIMER, &tim_oc_init);
    TIM_OC3Init(TIMER, &tim_oc_init);

    TIM_OC1PreloadConfig(TIMER, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIMER, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIMER, TIM_OCPreload_Enable);

    TIM_CtrlPWMOutputs(TIMER, ENABLE);



};

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

typedef void (*SetPWMValueFunc) (TIM_TypeDef*,uint32_t);


int main(void)
{
	EOM eomProgram;
	eom_init(&eomProgram);

	eom_clkreset(&eomProgram, SYSCLK_FREQ);


	setupTimer();
	setupPWM();
	setupGPIO();

    TIM_SetCompare1(TIMER, 0);
    TIM_SetCompare2(TIMER, 0);
    TIM_SetCompare3(TIMER, 0);

	TIM_Cmd(TIMER, ENABLE);




	uint32_t oldTimerCounter = 0;
	ButtonState brightnessButtonState = {0};
	ButtonState ledButtonState = {0};


	SetPWMValueFunc LEDS_BRIGHTNESS_FUNCS[] = { &TIM_SetCompare1, &TIM_SetCompare2, &TIM_SetCompare3 };
	const size_t LEDS_BRIGHTNESS_FUNCS_SIZE = sizeof(LEDS_BRIGHTNESS_FUNCS) / sizeof(LEDS_BRIGHTNESS_FUNCS[0]);

	uint16_t PWM_VALUES[BRIGHT_STEPS];
	for (int i = 0; i < BRIGHT_STEPS; i++)
	{
		PWM_VALUES[i] = i* (TIMER_APR / BRIGHT_STEPS);
	};
    int8_t current_brights[] = {0, 0, 0};

	int8_t current_led = 0;


	while (1)
	{

		int i;
		/* Switch the LED on */

		processButtonState(BUTTON_PORT, BRIGHT_BUTTON_PIN, &brightnessButtonState);
		processButtonState(BUTTON_PORT, SWITCH_BUTTON_PIN, &ledButtonState);
		if (brightnessButtonState.triggered && brightnessButtonState.switched_on)
		{
			current_brights[current_led]++;
			if (current_brights[current_led] == BRIGHT_STEPS)
				current_brights[current_led] = 0;
			LEDS_BRIGHTNESS_FUNCS[current_led](TIMER, PWM_VALUES[current_brights[current_led]] );


		};
		if (ledButtonState.triggered && ledButtonState.switched_on)
		{
			//LEDS_BRIGHTNESS_FUNCS[current_led](TIMER, 0 );
			current_led++;
			if (current_led == LEDS_BRIGHTNESS_FUNCS_SIZE)
				current_led= 0;
			LEDS_BRIGHTNESS_FUNCS[current_led](TIMER, PWM_VALUES[current_brights[current_led]] );


		};


	};

}

