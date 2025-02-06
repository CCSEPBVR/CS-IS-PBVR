#ifndef PBVR__JOB_COLLECTOR_H_INCLUDE
#define PBVR__JOB_COLLECTOR_H_INCLUDE

#include "JobDispatcher.h"
#include "PointObject.h"
#include "VariableRange.h"
#include "KVSMLObjectGlyph.h"

class JobCollector
{
private:
    JobDispatcher* m_jd;

    bool           m_batch;
    size_t         m_pack_size;
    float*         m_pack_coords;
    unsigned char* m_pack_colors;
    float*         m_pack_normals;
    float*         m_pack_directions;
    float*         m_pack_sizes;

    size_t              m_nvertices_list_size;
    std::vector<size_t> m_nvertices_list;

    int    m_pack_count;
    size_t m_pack_head;

public:
    JobCollector( JobDispatcher* pjd );
    ~JobCollector();

    void jobCollect( pbvr::PointObject* object, VariableRange* vr, bool* invalid, int* wid = NULL );
    void jobCollect_glyph( kvs::KVSMLObjectGlyph* object, bool* invalid, int* wid = NULL );
    void setBatch( const bool stat )
    {
        m_batch = stat;
    };
};

void TestJobCollector();

#endif //  PBVR__JOB_COLLECTOR_H_INCLUDE

