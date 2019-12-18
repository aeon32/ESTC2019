#include "eom_leddriver.h"
#include <stm32f4xx.h>

#define APR_VALUE 100

static void setupGPIO(EOMLedDriver * ledDriver)
{
   uint32_t rcc_AHB1Periph;
   if (ledDriver->ledPort == GPIOA)
   {
	   rcc_AHB1Periph = RCC_AHB1Periph_GPIOA;
   } else if (ledDriver->ledPort == GPIOB)
   {
	   rcc_AHB1Periph = RCC_AHB1Periph_GPIOB;

   } else if (ledDriver->ledPort == GPIOC)
   {
	   rcc_AHB1Periph = RCC_AHB1Periph_GPIOC;

   } else if (ledDriver->ledPort == GPIOD)
   {
	   rcc_AHB1Periph = RCC_AHB1Periph_GPIOD;
   } else if (ledDriver->ledPort == GPIOE)
   {
	   rcc_AHB1Periph = RCC_AHB1Periph_GPIOE;
   };
   RCC_AHB1PeriphClockCmd(rcc_AHB1Periph, ENABLE);

   GPIO_InitTypeDef gpioInitStructure = { 0 };

   gpioInitStructure.GPIO_Pin = ledDriver->redLedPin | ledDriver->blueLedPin | ledDriver->greenLedPin;
   gpioInitStructure.GPIO_Mode = GPIO_Mode_OUT;
   gpioInitStructure.GPIO_OType = GPIO_OType_PP;
   gpioInitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   gpioInitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_Init(ledDriver->ledPort, &gpioInitStructure);


};


static void ledTimerCallBack(EOMSoftwareTimer * timer, bool raiseEdge, void * userData)
{

	EOMLedDriver * ledDriver = (EOMLedDriver * ) userData;
	bool switchOn =  ledDriver->inverted ? !raiseEdge : raiseEdge;
	uint16_t pin;
	if (timer == &ledDriver->redTimer)
		pin = ledDriver->redLedPin;
	else if (timer == &ledDriver->blueTimer)
		pin = ledDriver->blueLedPin;
	else
		pin = ledDriver->greenLedPin;
	if (switchOn)
		GPIO_SetBits(ledDriver->ledPort, pin);
	else
		GPIO_ResetBits(ledDriver->ledPort, pin);

};


void eom_leddriver_init(EOMLedDriver * ledDriver, EOM * program, GPIO_TypeDef * ledPort,
		               uint16_t redLedPin, uint16_t greenLedPin, uint16_t blueLedPin, bool inverted )
 {

	 ledDriver->program = program;
	 ledDriver->ledPort = ledPort;
	 ledDriver->redLedPin = redLedPin;
	 ledDriver->blueLedPin = blueLedPin;
	 ledDriver->greenLedPin = greenLedPin;
	 ledDriver->inverted = inverted;
	 setupGPIO(ledDriver);

	 eom_software_timer_add(&ledDriver->redTimer, program, APR_VALUE, 0, ledTimerCallBack,ledDriver);
	 eom_software_timer_add(&ledDriver->blueTimer, program, APR_VALUE, 0, ledTimerCallBack, ledDriver);
	 eom_software_timer_add(&ledDriver->greenTimer, program, APR_VALUE, 0, ledTimerCallBack, ledDriver);


 };

void eom_leddriver_set_color(EOMLedDriver * ledDriver, uint8_t R, uint8_t G, uint8_t B)
{

	eom_software_timer_set_compare_value(&ledDriver->redTimer,  R * 100 / 256);
	eom_software_timer_set_compare_value(&ledDriver->greenTimer,  G * 100 / 256);
	eom_software_timer_set_compare_value(&ledDriver->blueTimer,  B * 100 / 256);

};



