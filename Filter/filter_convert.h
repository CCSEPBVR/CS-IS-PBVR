#ifndef FILTER_AVS_IO_H
#define	FILTER_AVS_IO_H
#include "filter.h"
#include "filter_write.h"

NodeInf max;
NodeInf min;
static const int    m_elementtypeno[] = { 0,     /*  0: point  */
                                          1,     /*  1: line   */
                                          2,     /*  2: tri    */
                                          3,     /*  3: quad   */
                                          4,     /*  4: tetra  */
                                          5,     /*  5: pyr    */
                                          6,     /*  6: prism  */
                                          7,     /*  7: hex    */
                                          8,     /*  8: line2  */
                                          9,     /*  9: tri2   */
                                         10,     /* 10: quad2  */
                                         11,     /* 11: tetra2 */
                                         12,     /* 12: pyr2   */
                                         13,     /* 13: prism2 */
                                         14};    /* 14: hex2   */

int filter_convert(int argc, char *argv[]);
int read_fld_file( void );
int read_geom( void );
int read_inp_file( void );
int read_coord( void );
int read_coord_1( int );
int read_variables_per_step( int );
int read_variables( int );
int read_variables_2( FILE* ,
                      int   );
int read_variables_3( int );

int read_ucdbin_geom( void );
int read_ucdbin_data( int );

int read_inp_file( void );
int skip_ucd_ascii( int );
int read_ucd_ascii_geom( void );
int skip_ucd_ascii_geom( void );
int read_ucd_ascii_value( int );
int write_ucdbin( int      ,
                  int      ,
                  int      ,
                  Int64 ,
                  Int64 );
int write_ucdbin_elem( int      ,
                       int      ,
                       Int64 ,
                       Int64 ,
                       Int64 ,
                       Int64 );
#endif	/* FILTER_AVS_IO_H */

