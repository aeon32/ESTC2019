#include "eom_antichatter.h"
#include "eom_monotonic_time.h"

void eom_antichatter_init( EOMAntiChatter * antichatter, EOM * program, GPIO_TypeDef * buttonPort,
		                  uint16_t buttonPin, uint16_t hysterTime, bool inverted)
{
	antichatter->program = program;
	antichatter->buttonPort = buttonPort;
	antichatter->buttonPin = buttonPin;
	antichatter->hysterTime = hysterTime;
	antichatter->inverted= inverted;
	antichatter->switchTime = 0;
	antichatter->lastTime = eom_monotonic_time(program);
	antichatter->switchedOn = false;
	antichatter->triggered = false;


};


bool eom_antichatter_process(EOMAntiChatter * antichatter)
{
	//we use counter to avoid contact chatter
	bool state = GPIO_ReadInputDataBit(antichatter->buttonPort, antichatter->buttonPin);
	if (antichatter->inverted)
		state = !state;

	uint32_t currentTime = eom_monotonic_time(antichatter->program);
	if (state)
	{
		uint32_t delta;
		if (antichatter->switchTime == 0)         //button pressed for first time
			delta = 1;
		else
			delta = eom_monotonic_time_elapsed(currentTime, antichatter->lastTime);

		antichatter->switchTime += delta;

		if (antichatter->switchTime > antichatter->hysterTime)
			antichatter->switchTime = antichatter->hysterTime;
	}
	else
	{
		uint32_t delta;

		if (antichatter->switchTime == antichatter->hysterTime)         //button depressed for first time
			delta = 1;
		else
			delta = eom_monotonic_time_elapsed(currentTime, antichatter->lastTime);

		if (antichatter->switchTime > delta)
			antichatter->switchTime -=delta;
		else
			antichatter->switchTime = 0;
	};

	antichatter->triggered = 0;
	if (!antichatter->switchedOn
			&& antichatter->switchTime == antichatter->hysterTime)
	{
		antichatter->switchedOn = true;
		antichatter->triggered = true;

	}
	else if (antichatter->switchedOn && antichatter->switchTime == 0)
	{
		antichatter->switchedOn = false;
		antichatter->triggered = true;

	};

	antichatter->lastTime = currentTime;
	return antichatter->triggered;


};


