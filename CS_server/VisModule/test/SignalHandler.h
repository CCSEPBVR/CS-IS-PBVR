/* 140319 for client stop by Ctrl+c */
bool SigServer = false;
void SignalHandler( const int sig )
{
    SigServer = true;
}
