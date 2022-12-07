

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <math.h>
#include <float.h>
#include <sys/stat.h>
#include <stdint.h>

#ifdef _OPENMP
#include "omp.h"
#endif

#include "filter_io_stl/FilterIoStl.h"

namespace pbvr {
namespace filter {

#define STL_DELIMITERS " \t"


enum cmd_codes {
    STL_BAD_CMD = 0,
    STL_VERTEX_CMD = 1,
    STL_FACET_START_CMD = 2,
    STL_LOOP_END_CMD = 3,
    STL_FACET_END_CMD = 4,
    STL_LOOP_START_CMD = 5,
    STL_SOLID_START_CMD = 6,
    STL_SOLID_END_CMD = 7,
    STL_NO_CMD = 8
};

const stl_cmd FilterIoStl::stl_cmd_map[7] = {
    { (char*)"vertex", STL_VERTEX_CMD},
    { (char*)"facet normal", STL_FACET_START_CMD},
    { (char*)"endloop", STL_LOOP_END_CMD},
    { (char*)"endfacet", STL_FACET_END_CMD},
    { (char*)"outer loop", STL_LOOP_START_CMD},
    { (char*)"solid", STL_SOLID_START_CMD},
    { (char*)"endsolid", STL_SOLID_END_CMD}
};

int FilterIoStl::stl_ascii_scan_command(const char *str) {
    char ch;
    int i;
    stl_cmd c;
    for (i = 0; i < 7; i++) {
    c = stl_cmd_map[i];
    if (stringBeginsWith(str, c.cmd_string)) {
        return c.cmd_code; // Early return on match
    }
    }
    // No match, check for allowed non command

    ch = str[0];
    if (ch == '\n' || ch == '#') {
    return STL_NO_CMD;
    }
    return STL_BAD_CMD;
}

/*
 * STL read geometries from ASCII file format
 *
 */
int FilterIoStl::stl_ascii_read_geom() {
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char *filename = info->filename;

    sprintf(filename, "%s/%s", param->in_dir, param->geom_file);

    //int state = RTC_NG;
    char line [LINELEN_MAX];
    int f_index = 0;
    //int f_count = 0;
    int cmd_code = STL_NO_CMD;

    FILE *ifs = NULL;
    //facet *active_facet;

    ifs = fopen(filename, "r");
    if (ifs == NULL) {
        LOGOUT(FILTER_ERR, (char*)"ERROR: Can't open the STL ASCII input file: %s\n", filename);
        goto garbage;
    }
    DEBUG(3, "#### Reading STL ASCII input:%s \n", filename);
    while (fgets(line, LINELEN_MAX, ifs) != NULL) {
        if (stringBeginsWith(line, "facet normal")) {
            number_of_facets++;
        }
    }

    // Allocate the memory for the coordinates read from the file
    info->m_coords = (float *) malloc(number_of_facets * sizeof (float)* 3 * 3);

    if (info->m_coords == NULL) {
        LOGOUT(FILTER_ERR, (char*)"ERROR: Allocating memory for the coordinates array size in function stl_ascii_read_geom %i\n", sizeof (facet) * number_of_facets);
        goto garbage;
    }

    //facets = (facet *) malloc(sizeof (facet) * f_count);
    rewind(ifs);
    LOGOUT(FILTER_LOG, (char*)"Preliminary facet Count :%i   .\n", number_of_facets);

    //  main loop to read in the ASCII data from the file
    while (fgets(line, LINELEN_MAX, ifs) != NULL) {

        cmd_code = stl_ascii_scan_command(line);
        if (cmd_code == STL_FACET_START_CMD) {
            //active_facet = &(facets[f_index]);
            //active_facet->normal = stringToVertex(line, STL_DELIMITERS, 1); // returns a vertex3
            // read the normal vector but do not save it (TODO implement option to save it inside the value array
            stringToVertex(line, STL_DELIMITERS, 1); // returns a vertex3
            if (read_ascii_facet(ifs, &info->m_coords[f_index]) == RTC_OK) {
            // every read_ascii_facet call reads in a triangle (3 vertices (each vertex has a x,y,z component)), so move index forward 9 values after the read
            f_index += 9;
            } else {
            goto garbage;
            }
        } else if (cmd_code == STL_SOLID_END_CMD) {
            break;
        } else if (cmd_code == STL_BAD_CMD) {
            LOGOUT(FILTER_LOG, (char*)" Bad STL command in facet  %s.\n", f_index);
            goto garbage;
        }
    }

    LOGOUT(FILTER_LOG, (char*)"Extracted %i facets %i .\n", f_index / 9, number_of_facets);


    return RTC_OK;
garbage:
    LOGOUT(FILTER_ERR, (char*)"ERROR: The stl_ascii_read function did not execute correctly:%i   .\n", f_index);
    if (NULL != ifs) fclose(ifs);
    return (RTC_NG);
}

int FilterIoStl::read_ascii_facet(FILE* ifs, float *active_facet) {
    int v_count = 0;
    char line [LINELEN_MAX];
    int cmd_code = STL_NO_CMD;
    while (fgets(line, LINELEN_MAX, ifs) != NULL) {
        cmd_code = stl_ascii_scan_command(line);
        if (cmd_code == STL_VERTEX_CMD) {
            stringToArrayPosition(line, STL_DELIMITERS, 0, &active_facet[v_count]);
            //LOGOUT(FILTER_LOG, (char*)"Check %f %f %f \n", active_facet[v_count+0], active_facet[v_count+1], active_facet[v_count+2]);
            v_count += 3;
        }
        if (cmd_code == STL_FACET_END_CMD) {
            return RTC_OK;
        }
        if (cmd_code == STL_SOLID_END_CMD) {
            LOGOUT(FILTER_ERR, (char*)"Unexpected 'endsolid'\n");
            return RTC_NG;
        }
    }
    LOGOUT(FILTER_ERR, (char*)"Unexpected end of file\n");
    return RTC_NG;
}




/*****************************************************************************/

/*
 * STL read geometries from binary file format
 *
 * Format:
    UINT8[80] – Header
    UINT32 – Number of triangles

    foreach triangle
    REAL32[3] – Normal vector
    REAL32[3] – Vertex 1
    REAL32[3] – Vertex 2
    REAL32[3] – Vertex 3
    UINT16 – Attribute byte count
    end
 */
#define B4 0xff000000
#define B3 0x00ff0000
#define B2 0x0000ff00
#define B1 0x000000ff
#define SWAP_ENDIAN_32(V)  ((((V) & B4) >> 24)| (((V) & B3) >>  8) | (((V) & B2) <<  8) | (((V) & B1) << 24))

/**
 * fread with integer endian conversion
 * @param buf_ptr
 * @param size
 * @param num
 * @param ifs
 * @return
 */
size_t FilterIoStl::fread_ei( int* buf_ptr, size_t size, size_t num,FILE* ifs) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;

    size_t bcount=fread(buf_ptr, size, num, ifs);
    size_t n;
    if (info->ENDIAN) {
        for ( n = 0; n < num; n++) {
            buf_ptr[n]=ConvertEndianI(buf_ptr[n]);
        }
    }
    return bcount;
}
/**
 * fread with float endian conversion
 * @param buf_ptr
 * @param size
 * @param num
 * @param ifs
 * @return
 */
size_t FilterIoStl::fread_ef( float* buf_ptr, size_t size, size_t num,FILE* ifs) {

    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;

    size_t bcount = fread(buf_ptr, size, num, ifs);
    size_t n;
    if (info->ENDIAN) {
        for ( n = 0; n < num; n++) {
            buf_ptr[n]=ConvertEndianF(buf_ptr[n]);
        }
    }
    return bcount;
}

int FilterIoStl::stl_bin_read_geom() {
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    char *filename = info->filename;

    sprintf(filename, "%s/%s", param->in_dir, param->geom_file);

    //int state = RTC_NG;
    int f;
    //uint32_t num_facets;
    //uint16_t atb_count;

    FILE *ifs = NULL;
    char *header[80];

    ifs = fopen(filename, "rb");
    if (ifs == NULL) {
        LOGOUT(FILTER_ERR, (char*)"Can't STL Binary input: %s\n", filename);
        goto garbage;
    }
    DEBUG(3, "#### Reading STL Binary input:%s \n", filename);

    fread(&header, 80, 1, ifs);
    fread_ei(&number_of_facets, 4, 1, ifs); // read the total number of facets (triangles) in the file


    LOGOUT(FILTER_LOG, (char*)"Header : %s\n", header);
    LOGOUT(FILTER_LOG, (char*)"Number of facets : %i \n", number_of_facets);
    //facets = (facet *) malloc(sizeof (facet) * num_facets);
    // Allocate the memory for the coordinates
    // m_coords is a very important array it is a gloabl variable and used inside filter_convert.c main loop
    info->m_coords = (float *) malloc(number_of_facets * sizeof (float)* 3 * 3);

    if (info->m_coords == NULL) {
        LOGOUT(FILTER_ERR, (char*)"Error, allocating memory for the coordinates array size %i\n", sizeof (facet) * number_of_facets);
        goto garbage;
    }

    // main loop for reading the data from the file
    for (f = 0; f < number_of_facets; f++) {
    fseek(ifs, 12, SEEK_CUR); // Do not read in the normal vector data (3 float values)
    fread_ef(&info->m_coords[9 * f], 4, 9, ifs); // read three vertices (3 * sizeof(float) * 3)
    fseek(ifs, 2, SEEK_CUR); // Do not read in two bytes at the end of the triangle (meta information)

    }

    return RTC_OK;

garbage:
    if (ifs != NULL) fclose(ifs);
    return RTC_NG;

}

/*****************************************************************************
 *
 * STL read data from binary file format
 *
 */
int FilterIoStl::stl_bin_read_data(int step_loop) {//  free(value);
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    char *filename = info->filename;

    int state = RTC_NG;
    FILE *ifs = NULL;
    ifs = fopen(filename, "r");
    if (ifs == NULL) {
    LOGOUT(FILTER_ERR, (char*)"Can't open  STL file : %s\n", filename);
    return state;
    }
    fclose(ifs);
    state = RTC_OK;
    return state;
}

bool FilterIoStl::stringBeginsWith(const char *str, const char *pre) {
    size_t lenpre = strlen(pre),
        lenstr = strlen(str);

    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

void FilterIoStl::stringToArrayPosition(char *str, const char *delimiters, int skip, float *pvertices) {
    char* pch;
    (void) pch;
    char* d;
    (void) d;
    pch = strtok(str, delimiters);
    while (skip > 0) {
    d = strtok(NULL, delimiters);
    skip--;
    }

    pvertices[0] = strtof(strtok(NULL, delimiters), NULL);
    pvertices[1] = strtof(strtok(NULL, delimiters), NULL);
    pvertices[2] = strtof(strtok(NULL, delimiters), NULL);
    //LOGOUT(FILTER_LOG, (char*)"Check %f %f %f \n", vertices[0], vertices[1], vertices[2]);
}

vertex3 FilterIoStl::stringToVertex(char *str, const char *delimiters, int skip) {
    vertex3 v;
    char* pch;
    (void) pch;
    char* d;
    (void) d;
    pch = strtok(str, delimiters);
    while (skip > 0) {
    d = strtok(NULL, delimiters);
    skip--;
    }

    v.x = strtod(strtok(NULL, delimiters), NULL);
    v.y = strtod(strtok(NULL, delimiters), NULL);
    v.z = strtod(strtok(NULL, delimiters), NULL);
    return v;
}

void FilterIoStl::printVertex(vertex3 *v) {
    printf("\t x:%f\t y:%f\t z:%f\t \n", v->x, v->y, v->z);
}

void FilterIoStl::printFacet(facet *f) {
    printf("----------------------------\n");
    printf("f->normal:");
    printVertex(&f->normal);
    printf("f->vertices[0]:");
    printVertex(&f->vertices[0]);
    printf("f->vertices[1]:");
    printVertex(&f->vertices[1]);
    printf("f->vertices[2]:");
    printVertex(&f->vertices[2]);
}



// this function is called inside the filter_convert.c file, it will setup several
// data structures for the STL data format, this function will be called after
// the STL files (Binary or ASCII) have been read

int FilterIoStl::setup_stl_geom() {
    if (this->m_filter_info == NULL) return RTC_NG;
    if (this->m_filter_info->m_param == NULL) return RTC_NG;
    PbvrFilterInfo *info = this->m_filter_info;
    FilterParam *param = this->m_filter_info->m_param;
    NodeInf *min = &info->node_min;
    NodeInf *max = &info->node_max;

    int count = 0;
    int nd;

    param->mult_element_type = (char) 0;
    info->m_nnodes = number_of_facets * 3;
    info->m_nelements = number_of_facets;
    info->m_connectsize = number_of_facets * 3;
    info->m_elemcoms = 3;
    info->m_element_type = 2;
    info->m_nkinds = 1;
    info->m_nullflags = (int *) malloc(sizeof (int)*info->m_nkinds);
    info->m_nulldatas = (float *) malloc(sizeof (float)*info->m_nkinds);
    LOGOUT(FILTER_LOG, (char*)"Try to allocate %i (m_ids).\n", (number_of_facets * 3));

    if (info->m_ids != NULL) free(info->m_ids);
    info->m_ids = (int *) malloc(sizeof (int)*number_of_facets * 3); // Number of triangles (TODO  NOT VERFIED, MUST BE CHECKED)

    if (NULL == info->m_ids) {
        LOGOUT(FILTER_ERR, (char*)"can not allocate memory for m_ids in function setup_stl_geom .\n");
        goto garbage;
    }

#if defined(_OPENMP) && !defined(_WIN32) && !defined(_WIN64)
    int myth, numth, i;
    float *min_x, *min_y, *min_z;
    float *max_x, *max_y, *max_z;
#pragma omp parallel private(myth,i)
    {
    myth = omp_get_thread_num();
    numth = omp_get_num_threads();
#pragma omp master
    {
        min_x = (float *) malloc(sizeof (float)*numth);
        min_y = (float *) malloc(sizeof (float)*numth);
        min_z = (float *) malloc(sizeof (float)*numth);
        max_x = (float *) malloc(sizeof (float)*numth);
        max_y = (float *) malloc(sizeof (float)*numth);
        max_z = (float *) malloc(sizeof (float)*numth);
    }
#pragma omp barrier
    min_x[myth] = FLT_MAX;
    min_y[myth] = FLT_MAX;
    min_z[myth] = FLT_MAX;
    max_x[myth] = -FLT_MAX;
    max_y[myth] = -FLT_MAX;
    max_z[myth] = -FLT_MAX;
#pragma omp for
    for (i = 0; i < info->m_nnodes; i++) {
        if (min_x[myth] > info->m_coords[i * 3 ]) min_x[myth] = info->m_coords[i * 3 ];
        if (min_y[myth] > info->m_coords[i * 3 + 1]) min_y[myth] = info->m_coords[i * 3 + 1];
        if (min_z[myth] > info->m_coords[i * 3 + 2]) min_z[myth] = info->m_coords[i * 3 + 2];

        if (max_x[myth] < info->m_coords[i * 3 ]) max_x[myth] = info->m_coords[i * 3 ];
        if (max_y[myth] < info->m_coords[i * 3 + 1]) max_y[myth] = info->m_coords[i * 3 + 1];
        if (max_z[myth] < info->m_coords[i * 3 + 2]) max_z[myth] = info->m_coords[i * 3 + 2];
    }
#pragma omp master
    {
        min->x = min_x[0];
        min->y = min_y[0];
        min->z = min_z[0];
        max->x = max_x[0];
        max->y = max_y[0];
        max->z = max_z[0];
        for (i = 1; i < numth; i++) {
        if (min->x > min_x[i]) min->x = min_x[i];
        if (min->y > min_y[i]) min->y = min_y[i];
        if (min->z > min_z[i]) min->z = min_z[i];

        if (max->x < max_x[i]) max->x = max_x[i];
        if (max->y < max_y[i]) max->y = max_y[i];
        if (max->z < max_z[i]) max->z = max_z[i];
        }
        free(max_x);
        free(max_y);
        free(max_z);
        free(min_x);
        free(min_y);
        free(min_z);
    }
    }
#else
    min->x = max->x = info->m_coords[0];
    min->y = max->y = info->m_coords[1];
    min->z = max->z = info->m_coords[2];
    for (nd = 1; nd < info->m_nnodes; nd++) {

    if (min->x > info->m_coords[nd * 3 ]) min->x = info->m_coords[nd * 3 ];
    if (min->y > info->m_coords[nd * 3 + 1]) min->y = info->m_coords[nd * 3 + 1];
    if (min->z > info->m_coords[nd * 3 + 2]) min->z = info->m_coords[nd * 3 + 2];

    if (max->x < info->m_coords[nd * 3 ]) max->x = info->m_coords[nd * 3 ];
    if (max->y < info->m_coords[nd * 3 + 1]) max->y = info->m_coords[nd * 3 + 1];
    if (max->z < info->m_coords[nd * 3 + 2]) max->z = info->m_coords[nd * 3 + 2];
    }
#endif


#ifdef _OPENMP
#pragma omp parallel for default(shared) private(count)
#endif
    for (count = 0; count < number_of_facets * 3; count++) {
        //LOGOUT(FILTER_LOG, (char*)"ID %i (m_ids).\n", count);
        info->m_ids[count] = count + 1;
    }

    if (info->m_elementids != NULL) free(info->m_elementids);
    if (info->m_materials != NULL) free(info->m_materials);
    if (info->m_elemtypes != NULL) free(info->m_elemtypes);
    if (info->m_connections != NULL) free(info->m_connections);
    info->m_elementids = (int *) malloc(sizeof (int)*number_of_facets);
    info->m_materials = (int *) malloc(sizeof (int)*number_of_facets);
    info->m_elemtypes = (char *) malloc(sizeof (char)*number_of_facets);
    //m_connections = (int *)malloc(sizeof(int)*number_of_facets*20);
    info->m_connections = (int *) malloc(sizeof (int)*number_of_facets * 3);

    if (NULL == info->m_connections) {
        LOGOUT(FILTER_ERR, (char*)"Can not allocate memory for m_connections in function setup_stl_geom.\n");
        goto garbage;
    }

#ifdef _OPENMP
#pragma omp parallel for default(shared) private(count)
#endif
    for (count = 0; count < number_of_facets; count++) {
        info->m_elementids[count] = count;
        info->m_elemtypes[count] = 2;
        info->m_connections[count * 3] = count * 3 + 1;
        info->m_connections[count * 3 + 1] = count * 3 + 2;
        info->m_connections[count * 3 + 2] = count * 3 + 3;
        //LOGOUT(FILTER_LOG," PRINT X Y Z -> %f, %f, %f\n",x,y,z);
    }

    info->m_types[2] = number_of_facets;

    LOGOUT(FILTER_LOG, (char*)"NUMBER OF FACETS: %i\n", count);

    // TODO (It must be verified what this code is doing)
    param->nodesearch = (char) 0;
    for (nd = 0; nd < info->m_nnodes; nd++) {
        if (info->m_ids[nd] != (nd + 1)) {
            param->nodesearch = (char) 1;
            LOGOUT(FILTER_LOG, (char*)"     mids[%10d]        : %d\n", nd, info->m_ids[nd]);
            LOGOUT(FILTER_LOG, (char*)"     param->nodesearch : %d\n", param->nodesearch);
            break;
        }
    }


    LOGOUT(FILTER_LOG, (char*)"|||| max -> %8.2f, %8.2f, %8.2f\n", max->x, max->y, max->z);
    LOGOUT(FILTER_LOG, (char*)"|||| min -> %8.2f, %8.2f, %8.2f\n", min->x, min->y, min->z);

    LOGOUT(FILTER_LOG, (char*)"|||| m_nelements       : %10d\n", info->m_nelements);
    LOGOUT(FILTER_LOG, (char*)"|||| m_connectsize     : %10d\n", info->m_connectsize);
    LOGOUT(FILTER_LOG, (char*)"|||| Trai    (type 2)  : %10d\n", info->m_types[ 2]);
    LOGOUT(FILTER_LOG, (char*)"|||| Quad    (type 3)  : %10d\n", info->m_types[ 3]);
    LOGOUT(FILTER_LOG, (char*)"|||| Tetra   (type 4)  : %10d\n", info->m_types[ 4]);
    LOGOUT(FILTER_LOG, (char*)"|||| Pyramid (type 5)  : %10d\n", info->m_types[ 5]);
    LOGOUT(FILTER_LOG, (char*)"|||| Prism   (type 6)  : %10d\n", info->m_types[ 6]);
    LOGOUT(FILTER_LOG, (char*)"|||| Hexa    (type 7)  : %10d\n", info->m_types[ 7]);
    LOGOUT(FILTER_LOG, (char*)"|||| Trai2   (type 9)  : %10d\n", info->m_types[ 9]);
    LOGOUT(FILTER_LOG, (char*)"|||| Quad2   (type10)  : %10d\n", info->m_types[10]);
    LOGOUT(FILTER_LOG, (char*)"|||| Tetra2  (type11)  : %10d\n", info->m_types[11]);
    LOGOUT(FILTER_LOG, (char*)"|||| Pyramid2(type12)  : %10d\n", info->m_types[12]);
    LOGOUT(FILTER_LOG, (char*)"|||| Prism2  (type13)  : %10d\n", info->m_types[13]);
    LOGOUT(FILTER_LOG, (char*)"|||| Hexa2   (type14)  : %10d\n", info->m_types[14]);

    //state = RTC_OK;
    return RTC_OK;
garbage:
    return (RTC_NG);
}


} /* namespace filter */
} /* namespace pbvr */

