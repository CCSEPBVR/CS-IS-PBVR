/*
 * FilterConvertBase.h
 *
 *  Created on: 2016/11/21
 *      Author: hira
 */

#ifndef FILTERCONVERT_H_
#define FILTERCONVERT_H_

#include "PbvrFilter.h"

#include "filter_io_avs/FilterIoAvs.h"
#include "filter_io_plot3d/FilterIoPlot3d.h"
#include "filter_io_stl/FilterIoStl.h"
#ifdef VTK_ENABLED
#include "filter_io_vtk/FilterIoVtk.h"
#endif

namespace pbvr
{
namespace filter
{
class PbvrFilter;

class FilterConvert {
public:
    FilterConvert(PbvrFilterInfo *filter_info, PbvrFilter *parent);
    virtual ~FilterConvert();
    virtual int filter_convert();
    int separate_files();
    int read_inp_file( void );
    int createSubvolumeInfos();
    int read_unstructer_multi(int elemtype);

public:
    static const int    m_elementtypeno[];

protected:
    int realloc_work(Int64 *prov_len, Int64 new_len);
    int read_struct_grid(int numpe_f, int mype_f
#ifdef MPI_EFFECT
        , MPI_Comm mpi_comm
#endif
        );
    int connect_sort(void);
    int initialize_memory(void);
    int initialize_value(void);
#ifdef MPI_EFFECT
    void bcast_param(int mype);
#endif

protected:
    PbvrFilterInfo *m_filter_info;
    FilterIoPlot3d *m_io_plot3d;
    FilterIoAvs *m_io_avs;
    FilterIoStl *m_io_stl;
#ifdef VTK_ENABLED
    FilterIoVtk *m_io_vtk;
#endif

    PbvrFilter* m_parent;

};

} // end of namespace filter
} // end of namespace pbvr
#endif /* FILTERCONVERT_H_ */
