#ifndef _TIMER_SIMPLE_H_INCLUDE
#define _TIMER_SIMPLE_H_INCLUDE

#define PBVR_TIMER_COUNT_NUM 9

#ifdef _TIMER_
#define PBVR_TIMER_INIT()
#define PBVR_TIMER_FIN()
#define PBVR_TIMER_STA(id)
#define PBVR_TIMER_END(id)
#define PBVR_TIMER_BAR(id)
#else
#define PBVR_TIMER_INIT()
#define PBVR_TIMER_FIN()
#define PBVR_TIMER_STA(id)
#define PBVR_TIMER_END(id)
#define PBVR_TIMER_BAR(id)
#endif

#endif
