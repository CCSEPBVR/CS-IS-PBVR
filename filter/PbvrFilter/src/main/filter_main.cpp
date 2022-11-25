#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <float.h>
#include <iostream>
#include <unistd.h>

#include "PbvrFilter.h"
#include "filter_utils.h"

#ifdef _OPENMP
#include "omp.h"
#endif

using namespace std;
using namespace pbvr::filter;


/*
 * フィルターメインプログラム
 * Filter main program
 */
int main(int argc, char *argv[]) {
    int status = RTC_NG;
    char* param_filepath;
    // current_block = 0;
    /* 引数チェック */
    if (2 != argc) {
        fprintf(stderr, "usage:%s [paramfile].\n", argv[0]);
        return (-1);
    }

#ifdef MPI_EFFECT
    MPI_Init(&argc, &argv);
#endif

#ifdef _DEBUG
    sleep(10);
#endif

    param_filepath = argv[1];
    PbvrFilter *filter = new PbvrFilter();
    filter->setIsWrite(true);

    DIVISION_TYPE type = PARAM_DIVISION;
    filter->setFilterDivision(type, 0);
    status = filter->loadPbvrFilter(param_filepath);

    PbvrFilterInfo *info = filter->getPbvrFilterInfo();
    release_PbvrFilterInfo(info);


    delete filter;

    if (status == RTC_NG) {
#ifdef MPI_EFFECT
        MPI_Abort(MPI_COMM_WORLD, 99999);
#endif
    }
#ifdef MPI_EFFECT
    MPI_Finalize();
#endif

    return status;

}
