/*
 * File:   filter_io_plot3d_types.h
 * Author: Martin Andersson, V.I.C
 *
 * Created on September 29, 2015, 4:04 PM
 */

#ifndef FILTER_IO_PLOT3D_TYPES_H
#define	FILTER_IO_PLOT3D_TYPES_H

/** Uncomment line to enable memory allocation ref listing  **/
//#define DEBUG_MEM

#ifdef DEBUG_MEM
#define ref_malloc(PTR,SIZE) PTR=allocationList_malloc(PTR, #PTR, SIZE,__FILE__,__LINE__)
#define ref_free(PTR) allocationList_free(PTR,#PTR,__FILE__,__LINE__)
#else
#define ref_malloc(PTR,SIZE) PTR=malloc(SIZE);
#define ref_free(PTR) free(PTR);
#endif

namespace pbvr {
namespace filter {

// Simple range object, with min and max

typedef struct Range {
    float min;
    float max;
} Range;

// ReadModes enumeration

typedef enum ReadMode {
    ASCII,
    BINARY,
} ReadMode;
// EndianTypes enumeration

typedef enum EndianType {
    LITTLE_ENDIAN_T,
    BIG_ENDIAN_T
} EndianType;
// DataTypeIds enumeration

typedef enum DataTypeId {
    INT_T,
    LONG_T,
    FLOAT_T,
    DOUBLE_T,
} DataTypeId;

// DataType struct type

typedef struct DataType {
    int onDiskSize;
    int inMemSize;
    int ptrSize;
    char scanFormat[4];
    char dumpFormat[5];
} DataType;

//Simple Dimension struct type

typedef struct Dimension {
    int i;
    int j;
    int k;
} Dimension;
//Simple Vertex struct type, float precision

typedef struct Vertex {
    float x;
    float y;
    float z;
} Vertex;
//Simple Vertex struct type, double precision

typedef struct VertexD {
    double x;
    double y;
    double z;
} VertexD;

//Variable type union

typedef union {
    float f;
    double d;
    char c;
    int i;
    Int64 l;
} var;

typedef struct tDataComponent {
    DataType type;
    DataTypeId typeId;
    //    Dimension dimensions;
    int num_tumples;
    unsigned char* ucBuffer;
    float* fData;
    double* dData;
    int* iData;
    Range valueRange;
    float (*getValueAsFloat)(void*, int);
    double (*getValueAsDouble)(void*, int);
} DataComponent;

// DataArray type definition

typedef struct DataArray {
    int num_components;
    int num_tuples;
    DataComponent* components;
} DataArray;
// DataSet type definition

typedef struct DataSet {
    int num_arrays;
    DataArray* arrays;
} DataSet;

} /* namespace filter */
} /* namespace pbvr */

#endif /* FILTER_IO_PLOT3D_TYPES_H */

