#ifndef _TIMER_SIMPLE_H_INCLUDE
#define _TIMER_SIMPLE_H_INCLUDE

#define TIMER_COUNT_NUM 9

#ifdef _TIMER_
#define TIMER_INIT()
#define TIMER_FIN()
#define TIMER_STA(id)
#define TIMER_END(id)
#define TIMER_BAR(id)
#else
#define TIMER_INIT()
#define TIMER_FIN()
#define TIMER_STA(id)
#define TIMER_END(id)
#define TIMER_BAR(id)
#endif

#endif
