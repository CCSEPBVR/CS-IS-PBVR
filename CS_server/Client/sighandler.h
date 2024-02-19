#ifndef PBVR__SIG_HANDLER_H_INCLUDE
#define PBVR__SIG_HANDLER_H_INCLUDE

void InitializeSignalHandler();
void SignalHandler( const int sig );
bool IsSignal();

#endif    // PBVR__SIG_HANDLER_H_INCLUDE

