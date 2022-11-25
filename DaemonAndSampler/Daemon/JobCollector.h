#ifndef MPI_JOB_COLLECTOR_H_INCLUDE
#define MPI_JOB_COLLECTOR_H_INCLUDE

#include "JobDispatcher.h"
#include "PointObject.h"
#include "VariableRange.h"

class JobCollector
{

    JobDispatcher* jd;

    bool           batch;
    size_t         pack_size;
    float*         pack_coords;
    unsigned char* pack_colors;
    float*         pack_normals;

    size_t              nvertices_list_size;
    std::vector<size_t> nvertices_list;

    int    pack_count;
    size_t pack_head;

public:
    JobCollector( JobDispatcher* pjd );
    ~JobCollector();

    int JobCollect( pbvr::PointObject* object, VariableRange* vr, int* wid = NULL );
    void SetBatch( bool stat )
    {
        batch = stat;
    };
};

void test_JobCollector();

#endif //  MPI_JOB_COLLECTOR_H_INCLUDE

