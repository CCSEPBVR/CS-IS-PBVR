#ifndef FILTER_UTILS_H
#define	FILTER_UTILS_H

#include "PbvrFilter.h"

namespace pbvr
{
namespace filter
{

char check_endian(void);
int ConvertEndianI(int);
float ConvertEndianF(float);

int set_element_in_node(PbvrFilterInfo *filter_info);
int create_hexahedron(PbvrFilterInfo* filter_info);
int create_tetrahedron(PbvrFilterInfo* filter_info);


int count_in_elements(int,
                      Int64 *,
                      Int64 *,
                      Int64 *,
                      Int64 *,
                      Int64,
                      Int64,
                      PbvrFilterInfo *filter_info);
int read_param_file(const char* filename, FilterParam* param);

int set_etc(PbvrFilterInfo* filter_info);
int count_in_subvolume(int,
                       Int64 *,
                       Int64 *,
                       Int64 *,
                       Int64 *,
                       Int64,
                       Int64,
                       bool mult_element_type,
                       PbvrFilterInfo *filter_info);
int count_in_subvolume_b(int,
                         Int64 *,
                         Int64 *,
                         Int64 *,
                         PbvrFilterInfo *filter_info);
int set_nodes_in_subvolume(Int64,
                           Int64,
                           PbvrFilterInfo *filter_info);
int search_node_id(int,
                   int,
                   int**);

int release_PbvrFilterInfo(PbvrFilterInfo *filter_info);
int pre_release_PbvrFilterInfo(PbvrFilterInfo *filter_info);
int initialize_PbvrFilterInfo(PbvrFilterInfo *filter_info);
char* getFilterCellTypeText(int element_type, char* cell_type);
void print_memory();
char *trim(char *s);


} // end of namespace filter
} // end of namespace pbvr

#endif	/* FILTER_UTILS_H */

