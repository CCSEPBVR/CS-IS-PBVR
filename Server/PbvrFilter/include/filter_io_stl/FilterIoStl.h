/*
 * FilterIoStl.h
 *
 *  Created on: 2016/11/24
 *      Author: hira
 */

#ifndef FILTER_IO_STL_FILTERIOSTL_H_
#define FILTER_IO_STL_FILTERIOSTL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "PbvrFilter.h"
#include "filter_utils.h"
#include "filter_log.h"

namespace pbvr {
namespace filter {

typedef  struct {
    float x;
    float y;
    float z;
} vertex3;

typedef struct {
    vertex3 normal;
    vertex3 vertices[3];
    int index;
} facet;

typedef struct stl_cmd {
    char *cmd_string;
    int cmd_code;
} stl_cmd;

class FilterIoStl {
public:
    FilterIoStl(PbvrFilterInfo *filter_info) : m_filter_info(filter_info), number_of_facets(0) {};
    virtual ~FilterIoStl() {};

    int  filter_io_stl(void);
    int stl_ascii_scan_command(const char *str);
    int stl_bin_read_geom();
    int stl_ascii_read_geom();
    int setup_stl_geom();
    bool stringBeginsWith(const char *str, const char *pre);
    vertex3 stringToVertex(char *str,const char *delimiters, int skip);
    void    stringToArrayPosition(char *str,const char *delimiters, int skip, float *vertices);
    void printVertex(vertex3 *v);
    void printFacet(facet *f);

public:
    PbvrFilterInfo *m_filter_info;

protected:
    int read_ascii_facet(FILE* ifs, float *active_facet);
    size_t fread_ei( int* buf_ptr, size_t size, size_t num,FILE* ifs);
    size_t fread_ef( float* buf_ptr, size_t size, size_t num,FILE* ifs);
    int stl_bin_read_data(int step_loop);

private:
    static const stl_cmd stl_cmd_map[7];
    int number_of_facets;
};

} /* namespace filter */
} /* namespace pbvr */

#endif /* FILTER_IO_STL_FILTERIOSTL_H_ */
