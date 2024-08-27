/* 
 * File:   filter_io_plot3d_types.c
 * Author: Martin Andersson, V.I.C 
 *
 * Created on September 29, 2015, 4:04 PM
 */
#include <stdlib.h>
#include <stdio.h>
#include "filter_io_types.h"
#include "../filter_log.h"

static int allocationListLength = -1;
static int allocationIndex = 0;

/*  Allocation Item type
 */
typedef struct allocationItem {
    void* address; //Pointer
    int size; //Size
    int line_use; //Line where allocated
    int line_free; //Line where freed
    char* file_name_use; //Filname where allocated
    char* file_name_free; //Filename where freed
    char* ptr_name; //Name of pointer 
    bool in_use; // Currently in use (i.e. not freed)
} allocationItem;

// List of allocated items
allocationItem* allocationItemList;

/**
 * Create an allocation item list
 * @param size int The capacity (number of items) of the list
 */
void allocationList_create(int size) {
    int i;
    allocationItemList = malloc(sizeof (allocationItem) * size);
    allocationListLength = size;
    for (i = 0; i < allocationListLength; i++) {
        allocationItemList[i].in_use = false;
        allocationItemList[i].address = 0;
        allocationItemList[i].line_use = 0;
        allocationItemList[i].line_free = 0;
        allocationItemList[i].file_name_free = "";
        allocationItemList[i].file_name_use = "";
        allocationItemList[i].ptr_name = "--";
    }
};

/**
 * Allocate new item and register it in the allocation list.
 * Typically called by the r_malloc macro which will automatically
 * fill in the name, file, and line parameters
 * @param target, the pointer to allocate
 * @param name, the name of the pointer 
 * @param size, the size of memory to allocate 
 * @param file, file name where allocation takes place
 * @param line, line where allocation takes place
 * @return 
 */
void* allocationList_malloc(void* target, char* name, int size, char* file, int line) {

    if (target != NULL) {
        filter_log(FILTER_ERR, "\n:::MEMORY:::ERROR target not null  for ptr %s    ::: line: %i in %s \n", name, line, file);
        fflush(stdout);
        exit(0);
    }
    void* ptr;
    ptr = malloc(size);
    if (ptr == NULL) {
        filter_log(FILTER_ERR, "\n:::MEMORY:::ERROR Could not allocate %i bytes for ptr %s    ::: line: %i in %s \n", size, name, line, file);
        fflush(stdout);
        exit(0);
    }
    if (allocationIndex < allocationListLength) {
        //    item= &allocation_list[allocation_index];
        allocationItemList[allocationIndex].address = ptr;
        allocationItemList[allocationIndex].size = size;
        allocationItemList[allocationIndex].in_use = true;
        allocationItemList[allocationIndex].line_use = line;
        allocationItemList[allocationIndex].file_name_use = file;
        allocationItemList[allocationIndex].ptr_name = name;
        allocationIndex++;
    } else {
        filter_log(FILTER_LOG, "\n:::MEMORY::WARNING allocation list to short (program execution, should continue normally, but memory no longer tracked)\n");
    }
    DEBUG(3,"\n:::MEMORY:::Allocated %i bytes at %p for ptr %s    ::: line: %i in %s \n", size, ptr, name, line, file);
    fflush(stdout);
    return ptr;
}

/**
 * Free an item that is registered in the Allocation item list
 * Typically called by the r_free macro which will fill in  the 
 * name,file, and line parameters
 * @param ptr,  the pointer to free
 * @param name, the name of the pointer 
 * @param file, the file where the pointer is freed
 * @param line, the line in the file where the pointer is freed
 */
void allocationList_free(void* ptr, char* name, char* file, int line) {
    int i;
    bool found_in_list = false;
    if (ptr == NULL) {
        return;
    }
    for (i = 0; i < allocationListLength; i++) {
        if (allocationItemList[i].address == ptr) {
            found_in_list = true;
            allocationItemList[i].in_use = false;
            allocationItemList[i].file_name_free = file;
            allocationItemList[i].line_free = line;
        }
    }
    if (!found_in_list) {
        filter_log(FILTER_LOG, "\n:::MEMORY:::WARNING Free on non tracked item %32s at %p. \t :::(line: %i in %s )\n", name, ptr, line, file);
    }
    DEBUG(3,":::MEMORY:::Freeing  %s at %p. \t :::(line: %i in %s )\n", name, ptr, line, file);
    free(ptr);
    ptr = NULL;
}

/**
 * Display the allocation list items which has been freed
 */
void allocationList_displayFreed() {
    int i;
    allocationItem item;
    printf("\n#### Listing freed allocations\n");
    for (i = 0; i < allocationIndex; i++) {
        item = allocationItemList[i];
        if (!item.in_use && item.address != 0) {
            printf("%4i, %p (%12i bytes) '%27s' in-use:%i (allocated at: %s %i) \t (freed at: %s %i) \n", i, item.address, item.size, item.ptr_name, item.in_use, item.file_name_use, item.line_use, item.file_name_free, item.line_free);
        }
    }
}

/**
 * Display the allocation list items which has not been freed
 */
void allocationList_displayActive() {
    int i;
    allocationItem item;
    printf("\n#### Listing active allocations\n");
    for (i = 0; i < allocationIndex; i++) {
        item = allocationItemList[i];
        if (item.in_use) {
            printf("%4i, %p (%8i bytes) '%24s' in-use:%i (allocated at: %s %i) \n", i, item.address, item.size, item.ptr_name, item.in_use, item.file_name_use, item.line_use);
        }
    }
}

/**
 * Destruct the allocation item list and free any remaining items
 */
void allocationList_destruct() {
    int i;
    int count = 0;
    allocationItem item;
    for (i = 0; i < allocationIndex; i++) {
        item = allocationItemList[i];
        if (item.in_use && item.address != NULL) {
            count++;
            allocationList_free(item.address, item.ptr_name, "Remaining allocation, freed by allocation list destructor", -1);
        }
    }
    free(allocationItemList);
    if (count > 0) {
        DEBUG(3,"#### Allocation list destructor found and freed %i remaining objects \n", count);
    }
}

/*
 Simple deep copy of Range object
 */
Range Range_copy(Range range_b) {
    Range range_a;
    range_a.max = range_b.max;
    range_a.min = range_b.min;
    return range_a;
}

/*
 Extend range a to include range b
 */
void Range_extend(Range* range_a, Range range_b) {
    range_a->max = range_a->max > range_b.max ? range_a->max : range_b.max;
    range_a->min = range_a->min < range_b.min ? range_a->min : range_b.min;
}

/*
 Get varTypeDataComponent tuple of type double and return as float
 */
static float varTypeDataComponent_getDoubleValue(void* dataComponent, int tuple) {
    DataComponent* dc = (DataComponent*) dataComponent;
    return (float) dc->dData[tuple];
}

/*
 Get varTypeDataComponent tuple of type float and return as float
 */
static float varTypeDataComponent_getFloatValue(void* dataComponent, int tuple) {
    DataComponent* dc = (DataComponent*) dataComponent;
    return (float) dc->fData[tuple];
}

/**
 * varTypeDataComponent Ctor 
 * @param nTuples, number of tuples
 * @param typeId, data type id
 * @return 
 */
DataComponent DataComponent_create(Int64 nTuples, DataTypeId typeId) {
    DataComponent dataComponent;
    dataComponent.ucBuffer = NULL;

    dataComponent.type = dataTypes[typeId];
    dataComponent.typeId = typeId;
    dataComponent.num_tumples = nTuples;
    Int64 size = nTuples * dataTypes[typeId].inMemSize;
    ref_malloc(dataComponent.ucBuffer, (size));
    dataComponent.dData = (double*) dataComponent.ucBuffer;
    dataComponent.fData = (float*) dataComponent.ucBuffer;
    dataComponent.iData = (int*) dataComponent.ucBuffer;
    if (typeId == DOUBLE_T) {
        dataComponent.getValueAsFloat = varTypeDataComponent_getDoubleValue;
    } else {
        dataComponent.getValueAsFloat = varTypeDataComponent_getFloatValue;
    }
    return dataComponent;
};

/**
 * Get pointer buffer location corresponding to tuple n;
 * @param dataComponent
 * @param tuple
 * @return 
 */
unsigned char* DataComponent_getPointerToTuple(DataComponent* dataComponent, int tuple) {
    int index = dataComponent->type.inMemSize * tuple;
    return &dataComponent->ucBuffer[index];
}

/**
 * Get value range of component
 * @param dataComponent
 * @return 
 */
Range DataComponent_getRange(DataComponent* dataComponent) {

    int t;

    float min, max;

    min = max = (*dataComponent->getValueAsFloat)(dataComponent, 0);
    for (t = 0; t < dataComponent->num_tumples; t++) {
        float val = (*dataComponent->getValueAsFloat)(dataComponent, t);
        if (val < min) min = val;
        if (val > max) max = val;
    }
    dataComponent->valueRange.max = max;
    dataComponent->valueRange.min = min;

    DEBUG(3,"DataComponent_getRange min %f and max %f  from %d tuples\n", min, max, dataComponent->num_tumples);
    return dataComponent->valueRange;
}

/**
 * varTypeDataComponent destructor
 * @param dataComponent
 */
void DataComponent_destruct(DataComponent* dataComponent) {
    if (dataComponent->ucBuffer == NULL) {
        return;
    }
    ref_free(dataComponent->ucBuffer);
    dataComponent->ucBuffer = NULL;
    dataComponent->fData = NULL;
    dataComponent->dData = NULL;
    dataComponent->iData = NULL;
    dataComponent->num_tumples = 0;
}

/**
 * Get varTypeDataArray tuple component of type double and return as float
 * @param dataArray
 * @param component component index
 * @param tuple tuple index
 * @return 
 */
static inline float varTypeDataArray_getDoubleValue(DataArray* dataArray, int component, int tuple) {
    return dataArray->components[component].getValueAsDouble(&dataArray->components[component], tuple);
}

/**
 * Get varTypeDataArray tuple component of type float and return as float
 * @param dataArray
 * @param component component index
 * @param tuple tuple index
 * @return 
 */
static inline float varTypeDataArray_getFloatValue(DataArray* dataArray, int component, int tuple) {
    return dataArray->components[component].getValueAsFloat(&dataArray->components[component], tuple);
    //    return (float) dataArray->components[component].fData[tuple];
}

/**
 * varTypeDataArray Ctor
 * @param nComponents
 * @param nTuples
 * @param typeId
 * @return 
 */
DataArray DataArray_create(Int64 nTuples) {
    int i;
    DataArray dataArray;
    dataArray.components = NULL;
    dataArray.num_components = 0;
    dataArray.num_tuples = nTuples;
    ref_malloc(dataArray.components, (16 * sizeof (DataComponent)));
    //    dataArray.getValueAsFloat = varTypeDataArray_getFloatValue;
    DEBUG(3,"### Allocated DataArray with %ld tuples\n", nTuples);
    return dataArray;
};

/**
 * Add component one to data array
 * @param dataArray array to add to
 * @param typeId    type of component
 * @return  returns reference to the added data component
 */
DataComponent* DataArray_addComponent(DataArray* dataArray, DataTypeId typeId) {

    int index = dataArray->num_components;
    DEBUG(3,"### DataArray_addComponent of type %d  at index %d\n", typeId, index);
    DataComponent comp = DataComponent_create(dataArray->num_tuples, typeId);
    dataArray->components[index] = comp;
    dataArray->num_components++;

    return &dataArray->components[index];
}

/**
 * Add multiple dataComponents of same type
 * @param dataArray     array to add to
 * @param typeId        type of component
 * @param numComponents number of components to add
 */
void DataArray_addComponents(DataArray* dataArray, DataTypeId typeId, int numComponents) {
    int i;
    for (i = 0; i < numComponents; i++) {
        DataArray_addComponent(dataArray, typeId);
    }
}

/**
 * Copy varTypeDataArray to float array
 * The offset and skip parameters can be used to interleave several
 * varTypeArrays into one target array.
 * @param dataArray, 
 * @param arr       The array pointer to copy to
 * @param offset    An offset before each copied tuple (group of components)
 * @param skip      A skip after each copied tuple
 */
void DataArray_copyAsFloat(DataArray* dataArray, float* arr, int offset, int skip) {
    int i;
    int j;
    int n = 0;
    DEBUG(3,"Copying  matrix of dimension %i x %i\n", dataArray->num_components, dataArray->num_tuples);
    for (j = 0; j < dataArray->num_tuples; j++) {
        n += offset;
        for (i = 0; i < dataArray->num_components; i++) {
            arr[n] = varTypeDataArray_getFloatValue(dataArray, i, j);
            n++;
        }
        n += skip;
    }
}

/**
 * Update range data, finds max and min after data change
 * @param dataArray
 */
void DataArray_updateRange(DataArray* dataArray) {
    int i;
    int j;
    int n = 0;
    float min, max;
    DEBUG(3,"DataArray_updateRange\n");

    for (i = 0; i < dataArray->num_components; i++) {
        DataComponent_getRange(&dataArray->components[i]);
    }
}

/**
 * Get Range from each component, and store into min and max array
 * @param dataArray
 * @param min       array pointer to array that will hold component min values
 * @param max       array pointer to array that will hold component max values
 * @param offset    offset, for interleaving data from several vtArrays
 */
void varTypeDataArray_getComponentRanges(DataArray* dataArray, float* min, float* max, int offset) {
    int t = 0;
    int c = 0;
    float val, fmax, fmin;
    DEBUG(3,"#$$# varTypeDataArray_getComponentRanges %p  %p %d\n", min, max, offset);


    for (c = 0; c < dataArray->num_components; c++) {
        fmax = varTypeDataArray_getFloatValue(dataArray, c, 0);
        fmin = varTypeDataArray_getFloatValue(dataArray, c, 0);
        for (t = 0; t < dataArray->num_tuples; t++) {
            val = varTypeDataArray_getFloatValue(dataArray, c, t);
            if (val > fmax) fmax = val;
            if (val < fmin) fmin = val;
        }
        DEBUG(3,"COMPONENT %d min: %f , max:%f\n", c, fmin, fmax);
        min[offset + c ] = fmin;
        max[offset + c ] = fmax;
    }


}

/**
 * Dump data of varTypeDataArray
 * @param dataArray
 */
void DataArray_dump(DataArray* dataArray) {
    int i;
    int j;

    DataComponent* dataComponent;

    for (i = 0; i < dataArray->num_components; i++) {
        dataComponent = &(dataArray->components[i]);
        printf("Dumping Matrix array: %i  (length:%i)\n", i, dataComponent->num_tumples);
        for (j = 0; j < dataArray->num_tuples; j++) {
            if (j % 5 == 0) {
                printf("%i\t:", j);
            }
            printf("%f\t", dataComponent->getValueAsFloat(dataComponent, j));
            if (j % 5 == 4) {
                printf("\n");
            }
        }
    }
}

/**
 * varTypeDataArray destructor
 * @param dataArray
 */
void DataArray_destruct(DataArray* dataArray) {
    if (dataArray->components == NULL) {
        return;
    }
    int i;
    for (i = 0; i < dataArray->num_components; i++) {
        DataComponent_destruct(&dataArray->components[i]);
    }
    ref_free(dataArray->components);

}

/**
 * varTypeDataSet Ctor
 * @param nArrays
 * @return 
 */
DataSet DataSet_create(Int64 nArrays) {
    int i;
    DataSet dataSet;
    dataSet.num_arrays = nArrays;
    ref_malloc(dataSet.arrays, (nArrays * sizeof (DataComponent)));
    DEBUG(3,"### Allocated DataSet with %ld arrays\n", nArrays);
    return dataSet;
};

/**
 * varTypeDataArray destructor
 * @param dataArray
 */
void DataSet_destruct(DataSet* dataSet) {
    if (dataSet->arrays == NULL) {
        return;
    }
    int i;
    for (i = 0; i < dataSet->num_arrays; i++) {
        DataArray_destruct(&dataSet->arrays[i]);
    }
    ref_free(dataSet->arrays);
}