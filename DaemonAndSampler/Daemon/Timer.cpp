#include <stdio.h>
#include <stdlib.h>        /* getenv,atoi,malloc,free */
#include <sys/time.h>      /* gettimeofday */
#include <sys/resource.h>  /* getrusage */
//#include <sys/types.h>
//#include <unistd.h>
#include "Timer.h"

#define N_TIM (700+1)    /* num of timers. PLEASE MODIFY. */
#define FILENAME "timer.out" /* NULL(stdout) or filename(ex."timer.out") */
#define IGN_ZERO 1     /* 1: timer result is not printed if ncount[i] equals 0 */

static void mygettod_( double* t );
static double timers[N_TIM], tsta[N_TIM];
static double ttot[3];
static long ncount[N_TIM], ncnt_e[N_TIM];
//static int iregion_id, in_region;
//static int iregion_id;
static char* name[N_TIM];
static int   readed_time_step = -1;

/**********************************************************************/

void TimerInitialize()
{
    int i, id_env;
    char* env;

    /* clear counters */
    for ( i = 0; i < N_TIM; i++ )
    {
        timers[i] = 0.0;
        ncount[i] = 0;
        ncnt_e[i] = 0;
        name[i]   = NULL;
    }

    /* set timer name. PLEASE MODIFY. */
    name[0]   = "dummy";  /* Don't use counter ID 0 */
    name[1]   = "total-------------------------------------------";
    name[2]   = "monitor_check-----------------------------------";
    name[3]   = "assign_histogram--------------------------------";
    name[4]   = "sendMessage-------------------------------------";
    name[5]   = "monitor_check/m_status_file.read----------------";
    name[6]   = "monitor_check/setParameterFromFile--------------";
    name[7]   = "monitor_check/m.rticle_file.generatePointObject-";
    name[8]   = "sendMessage/pack--------------------------------";
    name[9]   = "sendMessage/send--------------------------------";

    name[10]  = "outside_total-----------------------------------";
    name[11]  = "outside_total/sendMessage-----------------------";

    /* real, user, sys of total */
    mygettod_( &ttot[0] );

    return;
}

/**********************************************************************/
void TimerFinish( const int time_step )
{
    FILE* fp;
    int i, n_err;
    int m, n_err_g;
    int myrank, nprocs, resultlen;
    double rarray[2];

    if( readed_time_step != time_step )
    {
        readed_time_step = time_step;
    }
    else
    {
        return;
    }

    /* convert usec to sec */
    for ( i = 0; i < N_TIM; i++ )
    {
        timers[i] *= 1.0e-6;
    }

    /* check count of timer call */
    n_err = 0;
    for ( i = 0; i < N_TIM; i++ )
    {
        if ( ncount[i] != ncnt_e[i] )
        {
            printf( "Error: TimerFinish: not pair Timer(Start|Stop), ID = %d\n", i );
            n_err++;
        }
    }

    if ( n_err > 0 )
    {
        return;
    }

    /* print result */
    fp = stdout;
    if ( FILENAME )
    {
        char str[128];
        int x;
        sprintf( str, "%s.%05d", FILENAME, time_step );
        fp = fopen( str, "w" );
    }
    if ( fp == NULL )
    {
        return;
    }

    /* print timers */
    for ( i = 1; i < N_TIM; i++ )
    {
        if ( IGN_ZERO != 1 || ncount[i] != 0 )
        {
            fprintf( fp, "%3d %-24s %13.3f %8ld\n",
                     i, name[i], timers[i], ncount[i] );
        }
    }

    fprintf( fp, "\n" );

    if ( fp != stdout )
    {
        fclose( fp );
    }

    return;
}

/**********************************************************************/
void TimerStart( int id )
{
    mygettod_( &tsta[id] );
    ncount[id]++;

    return;
}

/**********************************************************************/
void TimerStop( int id )
{
    double tend;

    mygettod_( &tend );
    timers[id] += tend - tsta[id];
    ncnt_e[id]++;

    return;
}

/**********************************************************************/
static void mygettod_( double* t )
{
    struct timeval tm;

    gettimeofday( &tm, NULL );
    *t = tm.tv_sec * 1.0e6 + tm.tv_usec;

    return;
}

