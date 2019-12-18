#include "main.h"
#include <stdint.h>
#include <stdlib.h>
#include <stm32f4xx_tim.h>

#include <eom.h>
#include <eom_antichatter.h>
#include <eom_softwaretimer.h>
#include <eom_leddriver.h>
#include <eom_monotonic_time.h>


#define SYSCLK_FREQ 60

#define SWITCH_DELAY 50000

#define SWITCH_COUNTER_DELAY 500

#define BRIGHT_BUTTON_PIN GPIO_Pin_0
#define SWITCH_BUTTON_PIN GPIO_Pin_1
#define BUTTON_PORT GPIOE
#define BUTTON_PORT_PERIPH RCC_AHB1Periph_GPIOE

#define LED_PORT GPIOA
#define RED_LED_GPIO GPIO_Pin_8
#define GREEN_LED_GPIO GPIO_Pin_9
#define BLUE_LED_GPIO GPIO_Pin_10


#define LED_PORT_AUX GPIOD
#define RED_PIN_AUX  GPIO_Pin_13
#define GREEN_PIN_AUX GPIO_Pin_14
#define BLUE_PIN_AUX GPIO_Pin_15

#define MAX_BRIGHTNESS 255

#define BRIGHT_STEPS 6




void setupGPIO()
{
   GPIO_InitTypeDef GPIO_InitStructure =
	{ 0 };

	/* Enable peripheral clock for buttons port */
	RCC_AHB1PeriphClockCmd(BUTTON_PORT_PERIPH, ENABLE);

	GPIO_InitStructure.GPIO_Pin = BRIGHT_BUTTON_PIN | SWITCH_BUTTON_PIN ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(BUTTON_PORT, &GPIO_InitStructure);

}


int main(void)
{
	EOM eomProgram;
	eom_init(&eomProgram, SYSCLK_FREQ);
	
	setupGPIO();


	EOMAntiChatter brightnessButtonState;
	EOMAntiChatter ledButtonState;

	eom_antichatter_init(&brightnessButtonState, &eomProgram, BUTTON_PORT, BRIGHT_BUTTON_PIN, 30, true);
	eom_antichatter_init(&ledButtonState, &eomProgram,BUTTON_PORT, SWITCH_BUTTON_PIN, 30, true);



	uint16_t BRIGHT_VALUES[BRIGHT_STEPS];
	for (int i = 0; i < BRIGHT_STEPS; i++)
	{
		BRIGHT_VALUES[i] = i* (MAX_BRIGHTNESS / BRIGHT_STEPS);
	};
    int8_t current_brights_indexes[] = {0, 0, 0};

	int8_t current_led = 0;

    uint32_t lastTime = eom_monotonic_time(&eomProgram);

    EOMLedDriver ledDriver;
    eom_leddriver_init(&ledDriver, &eomProgram, LED_PORT_AUX, RED_PIN_AUX, GREEN_PIN_AUX, BLUE_PIN_AUX, false);
    uint8_t softwarePWMBrights[] = {0,100, 200};


    EOMLedDriver mainLedDriver;
    eom_leddriver_init(&mainLedDriver, &eomProgram, LED_PORT, RED_LED_GPIO, GREEN_LED_GPIO, BLUE_LED_GPIO, true);
    eom_leddriver_set_color(&mainLedDriver,1, 1, 1);

    while (1)
	{
		/* Switch the LED on */

		eom_antichatter_process( &brightnessButtonState);


		if (brightnessButtonState.triggered && brightnessButtonState.switchedOn)
		{
			current_brights_indexes[current_led]++;
			if (current_brights_indexes[current_led] == BRIGHT_STEPS)
				current_brights_indexes[current_led] = 0;

			eom_leddriver_set_color
			   (&mainLedDriver, BRIGHT_VALUES[current_brights_indexes[0]], BRIGHT_VALUES[current_brights_indexes[1]], BRIGHT_VALUES[current_brights_indexes[2]]);
		};

		eom_antichatter_process( &ledButtonState);
		if (ledButtonState.triggered && ledButtonState.switchedOn)
		{
			//LEDS_BRIGHTNESS_FUNCS[current_led](TIMER, 0 );
			current_led++;
			if (current_led == 3)
				current_led= 0;

			eom_leddriver_set_color
			   (&mainLedDriver, BRIGHT_VALUES[current_brights_indexes[0]], BRIGHT_VALUES[current_brights_indexes[1]], BRIGHT_VALUES[current_brights_indexes[2]]);



		};

		uint32_t currentTime = eom_monotonic_time(&eomProgram);
		if (currentTime >= (lastTime + 50))
		{
			for (int i = 0; i<3; i++)
			{
				softwarePWMBrights[i]+=1;
			};
			eom_leddriver_set_color(&ledDriver,softwarePWMBrights[0], softwarePWMBrights[1], softwarePWMBrights[2]);
			lastTime = currentTime;

		};


	};

}

