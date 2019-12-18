#include "eom_monotonic_time.h"


uint32_t eom_monotonic_time(EOM * program)
{
	return program->timeCounter / 8;
};


uint32_t eom_monotonic_time_elapsed(uint32_t currentTime, uint32_t lastTime)
{
	if (currentTime >= lastTime)
		return currentTime - lastTime;
	uint32_t  maxuint = (1UL<<31) - 1;
	return maxuint - lastTime + currentTime + 1;

}
