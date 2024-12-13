#ifndef VIS_MODULE__JOB_COLLECTOR_H_INCLUDE
#define VIS_MODULE__JOB_COLLECTOR_H_INCLUDE

#include <vismodule/JobDispatcher>
#include <vismodule/PointObject>
#include "VariableRange.h"
#include "KVSMLObjectGlyph.h"
#include <vector>

class JobCollector
{
private:
    JobDispatcher* m_jd;

    bool           m_batch;
    size_t         m_pack_size;
    size_t         m_pack_size_div3;
    float*         m_pack_coords;
    unsigned char* m_pack_colors;
    float*         m_pack_normals;
    float*         m_pack_directions;
    float*         m_pack_sizes;

    float*         m_pack_axis;
    float*         m_pack_values;
    int*           m_pack_mask;

    size_t              m_nvertices_list_size;
    std::vector<size_t> m_nvertices_list;

    int    m_pack_count;
    size_t m_pack_head;

public:
    JobCollector( JobDispatcher* pjd );
    ~JobCollector();

    void jobCollect( vismodule::PointObject* object, VariableRange* vr, bool* invalid, int* wid = NULL );
    void jobCollect_glyph( vismodule::KVSMLObjectGlyph* object, bool* invalid, int* wid = NULL );
    void jobCollect_pol( std::vector<float>& axis, std::vector<int>& mask, std::vector<float>& values, bool* invalid, int* wid = NULL );
    void setBatch( const bool stat )
    {
        m_batch = stat;
    };
};

void TestJobCollector();

#endif //  VIS_MODULE__JOB_COLLECTOR_H_INCLUDE

