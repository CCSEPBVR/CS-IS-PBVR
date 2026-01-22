/* 
 * File:   filter_io_stl.h
 * Author: Martin Andersson, V.I.C 
 *
 * Created on September 8, 2015, 1:04 PM
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#ifndef VIC_TOOL_SET_H
#define	VIC_TOOL_SET_H
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
#endif	/* VIC_TOOL_SET_H */

