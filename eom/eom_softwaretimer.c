#include "eom_softwaretimer.h"
#include "eom.h"
#include <stm32f4xx.h>

void eom_software_timer_add(EOMSoftwareTimer * softwareTimer,EOM * program,
						 uint32_t aprValue, uint32_t compareValue,
						 EOMSoftwareTimerCallBack callBack, void * userData )
{

  assert_param(program != 0);
  assert_param(softwareTimer != 0);

  eom_timers_lock(program);
  EOMSoftwareTimer * curr = program->timerListHead;
  bool alreadyExists = false;

  //check for existence in list
  while (curr && !alreadyExists)
  {
	  alreadyExists = (curr == softwareTimer);
	  curr = curr->nextTimer;

  };

  if (!alreadyExists)
  {
	  EOMSoftwareTimer * oldhead = program->timerListHead;
	  program->timerListHead = softwareTimer;
	  softwareTimer->nextTimer = oldhead;
  };
  eom_timers_unlock(program);

  softwareTimer->program = program;
  softwareTimer->compareValue = compareValue;
  softwareTimer->newCompareValue = compareValue;
  softwareTimer->aprValue = aprValue;
  softwareTimer->currentValue = aprValue;  //raise edge on first execute
  softwareTimer->callBack = callBack;
  softwareTimer->userData = userData;


};


void eom_software_timer_remove(EOMSoftwareTimer * softwareTimer)
{
	assert_param(softwareTimer != 0);
	assert_param(softwareTimer->program != 0);

	eom_timers_lock(softwareTimer->program);
	EOMSoftwareTimer * previous = softwareTimer->program->timerListHead;
	EOMSoftwareTimer * curr = previous;
	bool finded = false;

	while (curr && !finded)
	{
		if (curr == softwareTimer)
			finded = true;
		else
		{
			previous = curr;
			curr = curr->nextTimer;
		};

	};

	if (finded)
	{
		if (softwareTimer == softwareTimer->program->timerListHead)
		{
			softwareTimer->program->timerListHead = softwareTimer->nextTimer;
		} else {
			previous->nextTimer = softwareTimer->nextTimer;

		};

		softwareTimer->nextTimer = 0;

	};
	eom_timers_unlock(softwareTimer->program);


};

void eom_software_timer_increment_value(EOMSoftwareTimer * softwareTimer)
{
	if (softwareTimer->currentValue == softwareTimer->aprValue)
	{
		softwareTimer->currentValue = 0;
		softwareTimer->compareValue = softwareTimer->newCompareValue;
		if (softwareTimer->compareValue > 0)
			softwareTimer->callBack(softwareTimer, true, softwareTimer->userData);
	} else {
		softwareTimer->currentValue++;
	};


	if (softwareTimer->currentValue == softwareTimer->compareValue && softwareTimer->compareValue > 0)
	{
		//call callback!
		softwareTimer->callBack(softwareTimer, false, softwareTimer->userData);
	};
}

void eom_software_timer_set_compare_value(EOMSoftwareTimer * softwareTimer, uint32_t newCompareValue)
{
   softwareTimer->newCompareValue = newCompareValue;

};
