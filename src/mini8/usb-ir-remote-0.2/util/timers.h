/*********************************************************************
 * Copyright 2003                                                    *
 * J�rgen Birkler (birkler@yahoo.com)                                *
 *********************************************************************/

#ifndef _TIMERS_SIMPLE_H__
#define _TIMERS_SIMPLE_H__

// **********************************************
// * Timers
// **********************************************
#include "timer_ids.h"

typedef unsigned int milliseconds_t;
typedef unsigned int milliseconds_x256_t;
typedef unsigned char TimerId_t;



#define TIMER_HAS_EXPIRED (0)

#define TIMER_RESET ((milliseconds_t)(-1))


milliseconds_t Timers_DecreaseAll(const milliseconds_x256_t delta);


int Timer_HasExpired(TimerId_t TimerId);

void Timer_Reset(const TimerId_t TimerId);

void Timer_Set(const TimerId_t TimerId,const milliseconds_t setTimeout);

void Timer_SetExpired(const TimerId_t TimerId);

void Timers_Init(void);

#endif //_TIMERS_SIMPLE_H__

