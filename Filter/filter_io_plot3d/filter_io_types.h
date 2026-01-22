/* 
 * File:   filter_io_plot3d_types.h
 * Author: Martin Andersson, V.I.C 
 *
 * Created on September 29, 2015, 4:04 PM
 */

#ifndef FILTER_IO_PLOT3D_TYPES_H
#define	FILTER_IO_PLOT3D_TYPES_H
/** Windows MSVC as of this date still doesn't support standard inline def**/
#ifdef _MSC_VER
#define inline __inline
#define Int64 __int64
#else
//#define Int64 long int
#define Int64 long long
#endif
#include <stdbool.h>
#define integer int
#define real float
#define integer_size sizeof(integer)
#define real_size sizeof(real)
#define tVertex VertexD

/** Uncomment line to enable memory allocation ref listing  **/
//#define DEBUG_MEM

#ifdef DEBUG_MEM
#define ref_malloc(PTR,SIZE) PTR=allocationList_malloc(PTR, #PTR, SIZE,__FILE__,__LINE__)
#define ref_free(PTR) allocationList_free(PTR,#PTR,__FILE__,__LINE__)
#else
#define ref_malloc(PTR,SIZE) PTR=malloc(SIZE);
#define ref_free(PTR) free(PTR);
#endif

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

//DataTypes Array for dynamic lookup
static const DataType dataTypes[4] = {
    [INT_T] =
    {
        .onDiskSize = 4,
        .inMemSize = sizeof (int),
        .ptrSize = sizeof (int *),
        .scanFormat = "%d",
        .dumpFormat = "%d "
    },
    [LONG_T] =
    {
        .onDiskSize = 8,
        .inMemSize = sizeof (Int64),
        .ptrSize = sizeof (Int64 *),
        .scanFormat = "%ld",
        .dumpFormat = "%ld "
    },
    [FLOAT_T] =
    {
        .onDiskSize = 4,
        .inMemSize = sizeof (float),
        .ptrSize = sizeof (float *),
        .scanFormat = "%f",
        .dumpFormat = "%f "
    },
    [DOUBLE_T] =
    {
        .onDiskSize = 8,
        .inMemSize = sizeof (double),
        .ptrSize = sizeof (double *),
        .scanFormat = "%lf",
        .dumpFormat = "%lf "
    }
};

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

// DataComponent type definition
struct tDataComponent;
//typedef struct tDataComponent DataComponent; //Forward declared for func. pointer

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

/** Allocation List Methods **/
void* allocationList_malloc(void* target, char* name, int size, char* file, int line);
void allocationList_free(void* ptr, char* name, char* file, int line);
void allocationList_create(int size);
void allocationList_destruct();
void allocationList_displayActive();
void allocationList_displayFreed();

/** DataComponent  Methods **/
DataComponent DataComponent_create(Int64 i, DataTypeId typeId);
unsigned char* DataComponent_getPointerToTuple(DataComponent* dataComponent, int tuple);
Range DataComponent_getRange(DataComponent* dataComponent);
void DataComponent_destruct(DataComponent* dataComponent);

/** DataArray Methods **/
DataArray DataArray_create(Int64 nTuples);
//DataArray DataArray_create(Int64 iDim, Int64 jDim, DataTypeId typeId);
DataComponent* DataArray_addComponent(DataArray* dataArray, DataTypeId typeId);
void DataArray_addComponents(DataArray* dataArray, DataTypeId typeId, int numComponents);
void DataArray_copyAsFloat(DataArray* dataArray, float* arr, int offset, int skip);
void DataArray_getRange(DataArray* dataArray, float* min, float* max, int offset);
void varTypeDataArray_getComponentRanges(DataArray* dataArray, float* min, float* max, int offset);
void DataArray_updateRange(DataArray* dataArray);
void DataArray_dump(DataArray* dataArray);
void DataArray_destruct(DataArray* dataArray);

DataSet DataSet_create(Int64 nArrays);
void DataSet_destruct(DataSet* dataSet);
/** Range Methods **/
Range Range_copy(Range range_b);
void Range_extend(Range* range_a, Range range_b);

#endif /* FILTER_IO_PLOT3D_TYPES_H */

