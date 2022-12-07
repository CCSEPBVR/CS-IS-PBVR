
#ifndef FILTER_WRITE_H
#define	FILTER_WRITE_H

#include "PbvrFilter.h"
#include "FilterDivision.h"

namespace pbvr
{
namespace filter
{

class FilterWrite
{
public:
    FilterWrite(PbvrFilterInfo *filter_info) : m_filter_info(filter_info) {};
    virtual ~FilterWrite() {};

    int write_filter();
    int write_kvsml_xml( int      ,
                         int      ,
                         Int64 ,
                         Int64 );
    int write_kvsml_xmls( int      ,
                          int      ,
                          int      ,
                          Int64 ,
                          Int64 );
    int write_kvsml_geom( int        ,
                          int        ,
                          int        ,
                          Int64   ,
                          Int64   ,
                          Int64   ,
                          Int64   );
    int write_kvsml_value( int      ,
                           int      ,
                           int      ,
                           Int64 ,
                           Int64 ,
                           int node_offset = 0);		// add by @hira at 2017/04/01
    int write_value_per_vol( int      ,
                             Int64 ,
                             Int64 );
    int write_value_tmp1( FILE *   ,
                          int      ,
                          Int64 ,
                          Int64 );
    int write_pfi( NodeInf ,
                   NodeInf );
    int write_pfi_value( void );

    int set_geom( int      ,
                  Int64 ,
                  Int64 ,
                  Int64 ,
                  Int64 );
    int set_value( Int64   ,
                   Int64   );

    int write_pfi_value_single(void);
    int write_pfi_value_multi(void);
    int write_pfi_multi(NodeInf min, NodeInf max);
    int write_pfi_single(NodeInf min, NodeInf max);
    int write_kvsml_xmls_single(int step,
            int sub,
            int all,
            Int64 subvol_nelems,
            Int64 subvol_nnodes);
    int write_kvsml_xmls_multi(int step,
            int sub,
            int all,
            Int64 subvol_nelems,
            Int64 subvol_nnodes);

    void output_value_single(FILE *ifs,
            int nkind,
            int component_id,
            Int64 volume_nodes,
            Int64 rank_nodes,
            int node_offset = 0);		// add by @hira at 2017/04/01

    void output_value_multi(FILE *ifs,
            int nkind,
            int component_id,
            Int64 volume_nodes,
            Int64 rank_nodes,
            int k,
            int node_offset = 0);		// add by @hira at 2017/04/01
    int write_nodes_split();
    int write_nodes_subvolume();
    int write_nodes_step(
#ifdef MPI_EFFECT
        MPI_Comm mpi_comm
#endif
                        );
    int write_values_split(int step_loop);
    int write_valuetmp1_subvolume(int step_loop);
    int write_values_subvolume();
    int write_values_step(
                    int step_loop
#ifdef MPI_EFFECT
                    ,MPI_Comm  mpi_comm
#endif
                        );
    int remove_tmpfile(int m_elementtypeno,
                const char *m_out_prefix,
                int step_loop);

    void setFilterDivision(FilterDivision *division);

protected:
    int write_kvsml_value_single(int step_loop,
            int sub,
            int all,
            Int64 volume_nodes,
            Int64 rank_nodes,
            int node_offset = 0);		// add by @hira at 2017/04/01
    int write_kvsml_value_multi(int step_loop,
            int sub,
            int all,
            Int64 volume_nodes,
            Int64 rank_nodes,
            int node_offset = 0);		// add by @hira at 2017/04/01
private:
    PbvrFilterInfo *m_filter_info;
    FilterDivision *m_filter_division;
    void copyNodeValue(float* dest, float* src, int size, int subvolume);
};

} // end of namespace filter
} // end of namespace pbvr

#endif	/* FILTER_WRITE_H */

