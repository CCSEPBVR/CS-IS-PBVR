/* 140319 for client stop by Ctrl+c */
bool SigServer = false;
void sig_handler( int sig )
{
    SigServer = true;
}
/* 140319 for client stop by Ctrl+c */


