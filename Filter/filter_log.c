#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <sys/timeb.h>

#include <stdarg.h>

#include "filter_log.h"

#define LOGNAME_MAX 256
/*****************************************************************************/

/* 
 * 現在時刻を取得する
 * 
 */
void
get_localtime(char* time_buff) {
#ifdef MSDOS
    struct _timeb tb;
#else
    struct timeb tb;
#endif

    time_t t;
    struct tm *lt;
    short mil;
    int year;

    /* 現在時刻の取得 */
#ifdef MSDOS
    _ftime(&tb);
#else
    ftime(&tb);
#endif
    t = tb.time;
    mil = tb.millitm;

    /* ローカル時間の取得 */
    lt = localtime(&t);

    /* 年/月/日 時：分：秒：ミリ秒 */
    year = 1900 + lt->tm_year;
    sprintf(time_buff, "%d/%02d/%02d %02d:%02d:%02d:%03d",
	    year, lt->tm_mon + 1, lt->tm_mday,
	    lt->tm_hour, lt->tm_min, lt->tm_sec, mil);
}
/*****************************************************************************/

/* 
 * デバッグログ
 * 
 */
void
filter_log(int logmode, char* format, ...) {
    FILE *fp_log = NULL;
    char logfile[LOGNAME_MAX];

    va_list p;
    char *env_p = NULL;
    char buff[LOGNAME_MAX];
    char time_buff[LOGNAME_MAX] = {0};

    /* エラーログかデバッグログかを切り替え */
    switch (logmode) {
	case FILTER_ERR:
	    env_p = getenv("FILTER_ERRFILE");
	    if (NULL == env_p) {
		sprintf(logfile, "%s.%04d", FILTER_ERR_DEF, myrank);
	    } else {
		sprintf(logfile, "%s.%04d", env_p, myrank);
	    }
	    break;
	case FILTER_LOG:
	    env_p = getenv("FILTER_LOGFILE");
	    if (NULL == env_p) {
		sprintf(logfile, "%s.%04d", FILTER_LOG_DEF, myrank);
	    } else {
		sprintf(logfile, "%s.%04d", env_p, myrank);
	    }
	    break;
    }

    /* 現在時刻を取得する */
    //    get_localtime(time_buff);

    /* コメント文字列セットアップ */
    va_start(p, format);
    vsprintf(buff, format, p);

    /* ログ出力 */
    fp_log = fopen(logfile, "a");
    if (NULL != fp_log) {
	fprintf(fp_log, "%s %s", time_buff, buff);
	fclose(fp_log);
    }
}

/*****************************************************************************/

void f_log(int line, char* fname, const char* func) {

    log_line = line;
    if (strcmp(log_file, fname)) {
#ifdef DEBUG_LEVEL
	printf("\n  **File|: %s :|**", fname);
#endif
	sprintf(log_file, "%s", fname);
    }
    if (strcmp(log_func, func)) {
#ifdef DEBUG_LEVEL
	printf("\nFunction|: %s :|**\n", func);
#endif
	sprintf(log_func, "%s", func);
    }
#ifdef DEBUG_LEVEL
    printf("   %5d|:", line);
#endif
}