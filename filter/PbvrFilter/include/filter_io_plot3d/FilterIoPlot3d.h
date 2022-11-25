/*
 * FilterIoPlot3d.h
 *
 *  Created on: 2016/11/24
 *      Author: hira
 */

#ifndef FILTER_IO_PLOT3D_FILTERIOPLOT3D_H_
#define FILTER_IO_PLOT3D_FILTERIOPLOT3D_H_

#include "PbvrFilter.h"
#include "filter_utils.h"
#include "filter_log.h"

#include "filter_io_plot3d/filter_io_file_reader.h"
#include "filter_io_plot3d/filter_io_types.h"
#include "filter_io_plot3d/filter_io_plot3d_config.h"
#include "filter_io_plot3d/filter_io_plot3d_dataSet.h"
#include "filter_io_plot3d/filter_io_plot3d_solutionBlock.h"
#include "filter_io_plot3d/filter_io_plot3d_coordinateBlock.h"
#include "filter_io_plot3d/filter_io_plot3d_functionBlock.h"


namespace pbvr {
namespace filter {

typedef struct allocationItem {
    void* address; //Pointer
    int size; //Size
    int line_use; //Line where allocated
    int line_free; //Line where freed
    const char* file_name_use; //Filname where allocated
    const char* file_name_free; //Filename where freed
    const char* ptr_name; //Name of pointer
    bool in_use; // Currently in use (i.e. not freed)
} allocationItem;

class FilterIoPlot3d {
public:
    FilterIoPlot3d(PbvrFilterInfo *filter_info)
        : m_filter_info(filter_info),
          data_set(NULL),
          allocationListLength(-1),
		  allocationIndex(0),
		  allocationItemList(NULL) {};
    virtual ~FilterIoPlot3d() {};

    // filter_io_plot.cpp
    int plot3d_sequentialConvert(int argc, char *argv[]);
    int plot3d_setBlockParameters(void);
    int plot3d_getBlockCoordinates();
    int plot3d_getBlockValuesForTstep(int step);
    int read_plot3d_config(void);
    int plot3d_readGridCoordinateFile();
    int plot3d_initOffsetValueArray();

    // filter_io_file_reader.cpp
    int filter_io_open_DataFile(GridFile* gFile, const char* filepath, ReadMode rMode);
    int filter_io_close_dataFile(GridFile* gFile);
    int filter_io_readObject(GridFile* gFile, void* var, DataTypeId type);
    int filter_io_readBuffer(GridFile* gFile, void* var, Int64 count, DataTypeId typeId);
    void filter_io_close_FortranBlock(GridFile * gFile);
    Int64 filter_io_open_FortranBlock(GridFile * gFile);
    int filter_io_checkFileEndian(GridFile* gFile);
    void filter_io_skipToOffset(GridFile* gFile,Int64 bytes);

    // filter_io_types.cpp
    /** Allocation List Methods **/
    void* allocationList_malloc(void* target, char* name, int size, char* file, int line);
    void allocationList_free(void* ptr, const char* name, const char* file, int line);
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

    // filter_io_plot3d_coordinateBlock.cpp
    void plot3d_coordinateBlock_setDimension(Plot3DCoordinateBlock* cGrid, int i, int j, int k);
    void plot3d_coordinateBlock_allocateMemory(Plot3DCoordinateBlock* cGrid, bool hasIBlank, DataTypeId typeId);
    void plot3d_coordinateBlock_deAllocateMemory(Plot3DCoordinateBlock* cGrid);
    int plot3d_coordinateBlock_readWhole(Plot3DCoordinateBlock* cGrid, GridFile* gFile, bool hasIBlanks);
    void plot3d_coordinateBlock_writeToArray(Plot3DCoordinateBlock* cGrid, int offset, int skip);
    void plot3d_coordinateBlock_dump(Plot3DCoordinateBlock* cGrid, FILE* file);
    int setDiskByteSize(Plot3DCoordinateBlock* cGrid, GridFile* gFile, bool hasIBlanks) ;
    void plot3d_coordinateBlock_find_dims(Plot3DCoordinateBlock* cBlock, DataTypeId dataTypeId);

    // filter_io_plot3d_dataSet.cpp
    Plot3Ddataset* plot3d_dataSet_allocateNew(int* err_state, plot3d_params* params);
    void plot3d_dataSet_closeAllFiles(Plot3Ddataset* data_set);
    int plot3d_dataSet_getBlockCoordinates(Plot3Ddataset* data_set, float* target, int block);
    int plot3d_dataSet_getStepData(Plot3Ddataset* data_set, float* target, int block);
    int plot3d_dataSet_readStepData(Plot3Ddataset* data_set, float* target, int step, int block);
    void plot3d_dataSet_indexStepFiles(Plot3Ddataset* data_set);
    void plot3d_dataSet_deallocate(Plot3Ddataset* data_set);
    int plot3d_dataSetG_openDataFile(Plot3Ddataset* data_set);
    int plot3d_dataSetG_closeDataFile(Plot3Ddataset* data_set);
    int plot3d_dataSetG_allocateBlockTypes(Plot3Ddataset* data_set, int nGrids);
    int plot3d_dataSetG_readBlockCount(Plot3Ddataset* data_set);
    void plot3d_dataSetG_dumpBlocks(Plot3Ddataset *data_set, const char* filePath);
    int plot3d_dataSetG_readBlockDimensions(Plot3Ddataset * data_set);
    int plot3d_dataSetG_readBlockData(Plot3Ddataset * data_set);
    int plot3d_dataSetG_deAllocateMemory(Plot3Ddataset * data_set);
    int plot3d_dataSetG_skipToBlock(Plot3Ddataset* data_set, int block);
    int plot3d_dataSetG_read3DSingleBlock(Plot3Ddataset* data_set, int block);
    int plot3d_dataSetG_read3DMultiBlock(Plot3Ddataset* data_set);
    int plot3d_dataSetG_copyToArray(Plot3Ddataset* data_set, int step, int offset, int skip);
    int plot3d_dataSetG_readCopyCoords(Plot3Ddataset* data_set, int block, int step);
    void plot3d_dataSetG_findGridDimensions(Plot3Ddataset * data_set);
    void plot3d_dataSetF_findValueRanges(Plot3Ddataset * data_set);
    int plot3d_dataSetF_openStepFiles(Plot3Ddataset* data_set);
    int plot3d_dataSetF_closeStepFiles(Plot3Ddataset* data_set);
    void plot3d_dataSetS_closeStepFile(Plot3Ddataset* data_set, int step);
    int plot3d_dataSetF_allocateBlockTypes(Plot3Ddataset* data_set, int nGrids);
    int plot3d_dataSetF_readBlockCounts(Plot3Ddataset* data_set);
    void plot3d_dataSetF_dumpBlocks(Plot3Ddataset *data_set, const char* filePath);
    int plot3d_dataSetF_indexBlocks(Plot3Ddataset* data_set);
    int plot3d_dataSetF_readBlockData(Plot3Ddataset* data_set, int step);
    int plot3d_dataSetF_deAllocateMemory(Plot3Ddataset* data_set);
    int plot3d_dataSetF_skipToBlock(Plot3Ddataset* data_set, int block, int step);
    int plot3d_dataSetF_read3DSingleBlock(Plot3Ddataset* data_set, int block, int step);
    int plot3d_dataSetF_read3DMultiBlock(Plot3Ddataset* data_set, int step);
    int plot3d_dataSetF_copyToArray(Plot3Ddataset* data_set, int step, int offset, int skip);
    int plot3d_dataSetF_readCopyFunctions(Plot3Ddataset* data_set, int step, int block);
    int plot3d_dataSetF_readCopyFunctionBlock(Plot3Ddataset* data_set, float* target, int step, int block, int offset);
    int plot3d_dataSetF_getCopyFunctionBlock(Plot3Ddataset* data_set, float* target, int block, int offset);
    int plot3d_dataSetS_openStepFiles(Plot3Ddataset* data_set);
    int plot3d_dataSetS_closeStepFiles(Plot3Ddataset* data_set);
    int plot3d_dataSetS_allocateBlockTypes(Plot3Ddataset* data_set, int nGrids);
    int plot3d_dataSetS_readBlockCounts(Plot3Ddataset* data_set);
    void plot3d_dataSetS_dumpBlocks(Plot3Ddataset *data_set, const char* filePath);
    int plot3d_dataSetS_indexBlocks(Plot3Ddataset* data_set);
    int plot3d_dataSetS_readBlockData(Plot3Ddataset* data_set, int step);
    int plot3d_dataSetS_deAllocateMemory(Plot3Ddataset* data_set);
    int plot3d_dataSetS_skipToBlock(Plot3Ddataset* data_set, int block, int step);
    int plot3d_dataSetS_read3DSingleBlock(Plot3Ddataset* data_set, int block, int step);
    int plot3d_dataSetS_read3DMultiBlock(Plot3Ddataset* data_set, int step);
    int plot3d_dataSetS_copyToArray(Plot3Ddataset* data_set, int step, int offset, int skip);
    int plot3d_dataSetS_readCopySolution(Plot3Ddataset* data_set, int step, int block);
    int plot3d_dataSetS_readCopySolutionBlock(Plot3Ddataset* data_set, float* target, int step, int block, int skip);

    // filter_io_plot3d_functionBlock.cpp
    void plot3d_functionBlock_setDimension(Plot3DFunctionBlock* fGrid, int i, int j, int k, int nvars);
    void plot3d_functionBlock_allocateMemory(Plot3DFunctionBlock* fGrid, DataTypeId typeId);
    void plot3d_functionBlock_deAllocateMemory(Plot3DFunctionBlock* fGrid);
    int plot3d_functionBlock_readWhole(Plot3DFunctionBlock* fGrid, GridFile* gFile);
    void plot3d_functionBlock_writeToArray(Plot3DFunctionBlock* fGrid, int offset, int skip);
    void plot3d_functionBlock_dump(Plot3DFunctionBlock* fGrid, FILE* file);
    Int64 plot3d_functionBlock_getDiskByteSize(Plot3DFunctionBlock* fBlock, GridFile* gFile);

    // filter_io_plot3d_solutionBlock.cpp
    void plot3d_solutionBlock_setDimension(Plot3DSolutionBlock* sBlock, int i, int j, int k);
    void plot3d_solutionBlock_allocateMemory(Plot3DSolutionBlock* sGrid, DataTypeId typeId);
    void plot3d_solutionBlock_deAllocateMemory(Plot3DSolutionBlock* sGrid);
    int plot3d_solutionBlock_readWhole(Plot3DSolutionBlock* sGrid, GridFile* gFile);
    void plot3d_solutionBlock_writeToArray(Plot3DSolutionBlock* sGrid,int offset, int skip);
    void plot3d_solutionBlock_dump(Plot3DSolutionBlock* sGrid, FILE* file);
    Int64 plot3d_solutionBlock_getDiskByteSize(Plot3DSolutionBlock* cGrid, GridFile* gFile);

    // filter_io_plot3d_config.cpp
    int plot3d_config_read();

public:
    PbvrFilterInfo *m_filter_info;
    Plot3Ddataset* data_set;

protected:
    // float varTypeDataComponent_getDoubleValue(void* dataComponent, int tuple);
    // float varTypeDataComponent_getFloatValue(void* dataComponent, int tuple);
    float varTypeDataArray_getDoubleValue(DataArray* dataArray, int component, int tuple);
    float varTypeDataArray_getFloatValue(DataArray* dataArray, int component, int tuple);
    void filter_io_swap_bufferEndian(void* var, int size, int count);
    int filter_io_readBuffer_ASCII(GridFile* gFile, char* buffer, const char* format, int size, int count);
    int filter_io_readBuffer_Binary(GridFile* gFile, void* buffer, int size, Int64 count);
    int filter_io_readObject_Binary(GridFile* gFile, void* var, int size);
    int filter_io_readObject_ASCII(GridFile* gFile, void* var, const char* format);
    int plot3d_coordinateBlock_setDiskByteSize(Plot3DCoordinateBlock* cBlock, GridFile* gFile, bool hasIBlanks);
    int plot3d_coordinateBlock_readPlanes(Plot3DCoordinateBlock* cBlock, GridFile* gFile, bool hasIBlanks);
    int plot3d_dataSetF_openStepFile(Plot3Ddataset* data_set, int step);
    void plot3d_dataSetF_closeStepFile(Plot3Ddataset* data_set, int step);
    int plot3d_dataSetS_openStepFile(Plot3Ddataset* data_set, int step);
    int plot3d_dataSetS_getCopySolutionBlock(Plot3Ddataset* data_set, float* target, int block, int skip);
    int plot3d_functionBlock_readPlanes(Plot3DFunctionBlock* fBlock, GridFile* gFile);
    void plot3d_config_initializeConfig();
    void plot3d_config_dumpConfigBlock(grid_params* pBlock);

private:
    int allocationListLength;
    int allocationIndex;
    // List of allocated items
    allocationItem* allocationItemList;

    static const bool irregular = true;
    plot3d_params sParams;

    //DataTypes Array for dynamic lookup
    static const DataType dataTypes[4];
};

} /* namespace filter */
} /* namespace pbvr */

#endif /* FILTER_IO_PLOT3D_FILTERIOPLOT3D_H_ */
