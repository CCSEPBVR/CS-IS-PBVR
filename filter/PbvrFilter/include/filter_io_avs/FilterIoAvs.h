/*
 * FilterIoAvs.h
 *
 *  Created on: 2016/11/24
 *      Author: hira
 */

#ifndef FILTER_IO_AVS_FILTERIOAVS_H_
#define FILTER_IO_AVS_FILTERIOAVS_H_

#include "PbvrFilter.h"
#include "filter_utils.h"
#include "filter_log.h"

namespace pbvr {
namespace filter {

class FilterIoAvs {
public:
    FilterIoAvs(PbvrFilterInfo *filter_info)
        : m_filter_info(filter_info), m_multi_info(NULL) {};
    ~FilterIoAvs(){
		if (m_filter_info && m_filter_info->m_ucd_ascii_fp) {
			fclose(m_filter_info->m_ucd_ascii_fp);
			m_filter_info->m_ucd_ascii_fp = NULL;
		}
	};

    // filter_avsfld.cpp
    int read_fld_file(void);
    int read_geom( void );
    int read_coord( void );
    int read_variables_per_step(int step_loop);
    int read_variables( int step_loop );
    int read_coord_1( int step );
    int read_variables_2( FILE *ifs, int step_loop );
    int read_variables_3( int step_loop );

    // filter_ucdbin.cpp
    int read_ucdbin_geom( void );
    int read_ucdbin_data( int step_loop );
    int write_ucdbin( int      step_loop     ,
                      int      volm_loop     ,
                      int      subvol_no     ,
                      Int64 subvol_nelems ,
                      Int64 subvol_nnodes );
    int write_ucdbin_elem( int      step_loop     ,
                      int      elem_id       ,
                      Int64 subvol_nelems ,
                      Int64 subvol_nnodes ,
                      Int64 rank_nodes    ,
                      Int64 rank_elems    );
    // filter_ucdinp.cpp
    int read_inp_file( void );
    int read_ucd_ascii_geom( void );
    int read_ucd_ascii_value( int step_loop );
    int skip_ucd_ascii( int step );
    int skip_ucd_ascii_geom( void );
    int read_elementtypes( void );
    int read_unstructer_multi_geom(int element_type_num);
    int read_unstructer_multi_value(int element_type_num, int step_loop);
    int write_ucdbin_data(
                        int      step_loop     ,
                        int      elem_id       ,
                        Int64 subvol_nelems ,
                        Int64 subvol_nnodes ,
                        Int64 rank_nodes    ,
                        Int64 rank_elems    );

protected:
    int ChangeChar(char Text[], char Chr1, char Chr2);
    int alloc_stepfileinfo(void);

private:
    PbvrFilterInfo *m_filter_info;
    PbvrFilterInfo *m_multi_info;		// add by @hira at 2017/04/15
};

} /* namespace filter */
} /* namespace pbvr */

#endif /* FILTER_IO_AVS_FILTERIOAVS_H_ */
