#ifndef JUPITER_TIMER_H_INCLUDE
#define JUPITER_TIMER_H_INCLUDE

void TimerInitialize();
void TimerFinish( const int time_step );
void TimerStart( int id );
void TimerStop( int id );

#endif
