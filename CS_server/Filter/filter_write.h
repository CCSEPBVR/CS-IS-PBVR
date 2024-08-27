
#ifndef FILTER_WRITE_H
#define	FILTER_WRITE_H
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
                       Int64 );
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
        Int64 rank_nodes);

void output_value_multi(FILE *ifs,
        int nkind,
        int component_id,
        Int64 volume_nodes,
        Int64 rank_nodes,
        int k);
#endif	/* FILTER_WRITE_H */

