#ifndef FILTER_LOG_H_
#define FILTER_LOG_H_

#include <stdio.h>
#include <stdlib.h>
#ifdef MPI_EFFECT
#include "mpi.h"
#endif

/* LOGOUT MACRO */
#define LOGOUT     FilterLog::getInstance().filter_log

/*DEBUG MACRO*/
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0
#endif
#if DEBUG_LEVEL == 0
#define DEBUG( L, ... )
#else
#define DEBUG( L, ... )  if( L <= DEBUG_LEVEL ){FilterLog::getInstance().f_log(__LINE__,__FILE__,__func__); printf( __VA_ARGS__ ); fflush(stdout);};
#endif

#define UNUSED(x) (void)(x)
/* Windows MSVC cl does not define standard c99 __func__ */
#ifndef __func__
#define __func__ __FUNCTION__
#endif
/* 返却値 */
/* ASERTION MACROS added by VIC 2015-10-22*/
#define ASSERT_PASS
#define ASSERT_FAIL(MESS)  LOGOUT(FILTER_ERR,(char*)" ##!! Error: %s. !!## | Aborting at line %d in %s.\n",MESS,__LINE__,__FILE__);exit(0);
#define ASSERT_RTC_OK(ERR_STATE,MESS) if (ERR_STATE != RTC_OK) {ASSERT_FAIL(MESS)} else {ASSERT_PASS};
#define ASSERT_TRUE(CONDITION,MESS) if ((CONDITION)!= 1) {ASSERT_FAIL(MESS)} else {ASSERT_PASS};
#define ASSERT_ALLOC(PTR) if (PTR == NULL) {ASSERT_FAIL("Memory allocation error, pointer is null at:")} else {ASSERT_PASS};
#define CHK(...)  printf( __VA_ARGS__); printf(" at %d, in %s\n",__LINE__,__FILE__);getc(stdin);

/* ログ出力モード */
#define FILTER_ERR 0
#define FILTER_LOG 1

/* ログファイルデフォルト設定 */
#define FILTER_ERR_DEF "./pbvr_filter.err"
#define FILTER_LOG_DEF "./pbvr_filter.log"

namespace pbvr
{
namespace filter
{

class FilterLog
{
public:
    static FilterLog& getInstance() {
        static FilterLog log;
        return log;
    }
    char log_file[512];
    char log_func[512];
    int log_line;
    int myrank;

    void filter_log(int, char*, ...);
    void f_log(int line, const char* fname,const char* func);

private:
    FilterLog():log_line(0), myrank(0) {
#ifdef MPI_EFFECT
        MPI_Comm_rank(MPI_COMM_WORLD, &this->myrank);
#endif
    }

    void get_localtime(char*);
};

} // end of namespace filter
} // end of namespace pbvr

#endif /* FILTER_LOG_H_ */
