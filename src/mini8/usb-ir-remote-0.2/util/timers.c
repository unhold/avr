/*********************************************************************
 * Copyright 2003                                                    *
 * Jorgen Birkler (birkler@yahoo.com)                                *
 *********************************************************************/

#include "timers.h"
//#include <util/atomic.h>

static volatile milliseconds_t Timers[TIMER_LAST];

#define ATOMIC_BLOCK(x)

void Timers_Init(void)
{
	TimerId_t id;
	for (id=0; id < TIMER_LAST;id++)
	{
    Timers[id]=TIMER_RESET;
  }
}

milliseconds_t Timers_DecreaseAll(milliseconds_x256_t delta_x256)
{
	static unsigned char frag = 0;
	TimerId_t id;
	milliseconds_t nextTime = TIMER_RESET;
	delta_x256 += frag;
	frag = delta_x256 & 0xFF;
	milliseconds_t delta = delta_x256>>8;
	for (id=0; id < TIMER_LAST;id++)
	{
		milliseconds_t tempTime = Timers[id];
		if (tempTime != TIMER_RESET)
		{
			if (tempTime > delta) 
			{
				tempTime-=delta;

				if (tempTime < nextTime) 
				{
					nextTime = tempTime;
				}

				Timers[id] = tempTime;
			}
			else {
				Timers[id] = TIMER_HAS_EXPIRED;
			}
		}
	}
	return nextTime;
}


int Timer_HasExpired(const TimerId_t TimerId)
{
	int result;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if (Timers[TimerId]==TIMER_HAS_EXPIRED)
		{
			Timers[TimerId]=TIMER_RESET;
			result = 1;
		}
		else {
			result = 0;
		}
	}
	return result;
}

void Timer_Reset(const TimerId_t TimerId)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		Timers[TimerId]=TIMER_RESET;
	}
}
void Timer_SetExpired(const TimerId_t TimerId)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		Timers[TimerId]=TIMER_HAS_EXPIRED;
	}
}


void Timer_Set(const TimerId_t TimerId,const milliseconds_t setTimeout)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		Timers[TimerId]=setTimeout;
	}
}



