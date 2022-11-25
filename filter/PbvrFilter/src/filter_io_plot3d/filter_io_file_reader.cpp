
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "filter_io_plot3d/FilterIoPlot3d.h"

namespace pbvr {
namespace filter {

//#ifdef WIN32
#if 1
const char* modeStrings[3] = {
    "r",
    "rb",
	""
};
#else
const char* modeStrings[3] = {
    [ASCII] = "r",
    [BINARY] = "rb",
};
#endif

/**
 * openGridDataFile, Opens the grid data file (XYZ, Q or Function) and stores
 * the file handle in grid->pDataFile
 * @param gFile GridFile*  struct with additional information about the file
 * @param filepath
 * @return status, error code - 0 for OK, -1 for error
 */
int FilterIoPlot3d::filter_io_open_DataFile(GridFile* gFile, const char* filepath, ReadMode rMode) {
    gFile->rMode = rMode;
    gFile->handle = fopen(filepath, modeStrings[rMode]);
    gFile->filePath = filepath;
    gFile->OpenFblockEnd = 0;
    gFile->usebytecount = false;
    if (!gFile->handle) {
        LOGOUT(FILTER_ERR, (char*)"##filter_io_reader cannot open data file %s \n", filepath);
        return RTC_NG;
    }

    DEBUG(3,"##filter_io_reader opened %s in mode %s\n", filepath, modeStrings[rMode]);
    return RTC_OK;
}

/**
 * NOT USED!
 * Checks the endian of the intput file.
 * As this only works if the file has fortran style byte headers,
 * the endian is now specified using the configuration file instead.
 * @param gFile
 * @return
 */
int FilterIoPlot3d::filter_io_checkFileEndian(GridFile* gFile) {
    int status = 0;
    int cnt=0;
    UNUSED(cnt);
    if (gFile->rMode == ASCII) {
        gFile->needsByteSwap = false;
        return status;
    }

    char buffer[4];

    // Endians, 0 for BIG, 1 for LITTLE endian
    int sys_endian = check_endian();
    int file_endian;
    cnt= fread(&buffer, 4, 1, gFile->handle);
    if (buffer[0] == 4 && buffer[3] == 0) {
        DEBUG(3,"File seems to be BIG_ENDIAN\n");
        file_endian = 0;
    } else if (buffer[3] == 4 && buffer[0] == 0) {
        DEBUG(3,"File seems to be LITTLE_ENDIAN\n");
        file_endian = 1;
    } else {
        LOGOUT(FILTER_ERR,(char*)"ERROR: File Endian can not be determined\n");
        return -1;
    }
    gFile->needsByteSwap = file_endian == sys_endian ? false : true;
    rewind(gFile->handle);
    return status;
}

/**
 * filter_io_close_dataFile , does what you'd expect
 * @param gFile GridFile*  struct with additional information about the file
 * @return status, error code - 0 for OK, -1 for error
 */
int FilterIoPlot3d::filter_io_close_dataFile(GridFile* gFile) {
    Int64 cur_loc = ftell(gFile->handle);
    UNUSED(cur_loc);
    fseek(gFile->handle, 0, SEEK_END);
    Int64 sz = ftell(gFile->handle);
    UNUSED(sz);
    DEBUG(3,"##Closing data file with %ld bytes remaining\n", sz - cur_loc);
    fclose(gFile->handle);
    return 0;
}

/**
 * filter_io_swap_bufferEndian, swaps the byte order (endian) of a void* array
 * @param var *void Array buffer to be swapped
 * @param size int Size of the elements contained in the var array
 * @param count int The number of elements in the var array
 */
void FilterIoPlot3d::filter_io_swap_bufferEndian(void* var, int size, int count) {
    unsigned char* tmp0 = (unsigned char*)malloc(size*count);//[size * count];
    unsigned char* tmp1 = (unsigned char*)malloc(size*count);//[size * count];
    int i, c;
    memcpy(tmp0, var, size * count);
    for (c = 0; c < size * count; c += size) {
        for (i = 0; i < size; i++) {
            tmp1[c + i ] = tmp0[c + (size - 1) - i];
        }
    }
    memcpy(var, tmp1, size * count);
    free(tmp0);
    free(tmp1);
}

/**
 * filter_io_readBuffer_ASCII, reads a number of elements from an ASCII input file
 * @param gFile GridFile*  struct with additional information about the file
 * @param buffer char* the buffer to store the data
 * @param format char* the format specifier string
 * @param size   int   the size of each element
 * @param count  int   the count of elements to read
 * @return status, error code - 0 for OK, -1 for error
 */
int FilterIoPlot3d::filter_io_readBuffer_ASCII(
        GridFile* gFile, char* buffer, const char* format, int size, int count) {
    int status = 0;
    int index = 0;
    FILE* fh = gFile->handle;
    while (index < count) {
    if (fscanf(fh, format, &(buffer[index * size]))) {
        index++;
    } else {
        LOGOUT(FILTER_LOG, (char*)"##WARN: break at index %i in filter_io_readBuffer_ASCII\n", index);
        break;
    }
    }
    DEBUG(3,"##filter_io_readBuffer_ASCII read %i Int64 floats (%s)\n", index, format);

    //    if (count != objCount) {
    //	printf("ERROR: filter_io_readBuffer - Expected %i objects of size %i, but was only able to read %i \n", count, size, objCount);
    //	status = -1;
    //    }
    return status;
}

/**
 * filter_io_readBuffer_Binary,  reads a number of elements from a binary input file
 * @param gFile GridFile*  struct with additional information about the file
 * @param buffer char* the buffer to store the data
 * @param format char* the format specifier string
 * @param size   int   the size of each element
 * @param count  int   the count of elements to read
 * @return status, error code - 0 for OK, -1 for error
 */
int FilterIoPlot3d::filter_io_readBuffer_Binary(GridFile* gFile, void* buffer, int size, Int64 count) {

    Int64 objCount;
    FILE* fh = gFile->handle;
    int loc = ftell(fh);
    UNUSED(loc);
    objCount = fread(buffer, size, count, fh);
    if (count != objCount) {
        LOGOUT(FILTER_ERR, (char*)"##ERROR: filter_io_readBuffer - Expected %ld objects of size %d, but was only able to read %ld \n", count, size, objCount);
        return RTC_NG;
    } else {
        DEBUG(3,"## read %ld elements of size %i at location %i in file %p\n", objCount, size, loc, gFile->handle);
    }
    if (gFile->needsByteSwap) {
        filter_io_swap_bufferEndian(buffer, size, count);
    }
    return RTC_OK;
}

/**
 * filter_io_readObject_Binary, reads a single element from binary file
 * @param gFile GridFile*  struct with additional information about the file
 * @param var void* variable to write to
 * @param size the byte size of var
 * @return status, error code - 0 for OK, -1 for error
 */
int FilterIoPlot3d::filter_io_readObject_Binary(GridFile* gFile, void* var, int size) {
    int status = 0;
    FILE* fh = gFile->handle;
    Int64 objCount = fread(var, size, 1, fh);
    if (gFile->needsByteSwap) {
        filter_io_swap_bufferEndian(var, size, 1);
    }
    if (objCount != 1) {
        LOGOUT(FILTER_ERR, (char*)"##ERROR: readTuple - Expected 1 object of size %i, but was only able to read %ld \n", size, objCount);
    status = -1;
    }
    return status;
}

/**
 * filter_io_readObject_Binary, reads a single element from ASCII file
 * @param gFile GridFile*  struct with additional information about the file
 * @param var void* variable to write to
 * @param format char* the format specifier to use for reading
 * @return status, error code - 0 for OK, -1 for error
 */
int FilterIoPlot3d::filter_io_readObject_ASCII(GridFile* gFile, void* var, const char* format) {

    FILE* fh = gFile->handle;
    DEBUG(3,"##Reading Object from file using format string: '%s' \n", format);
    int objCount = fscanf(fh, format, var);
    if (objCount != 1) {
        LOGOUT(FILTER_ERR, (char*)"##ERROR: readTuple - Expected 1 object of type %s, but was only able to read %i \n", format, objCount);
        return RTC_NG;
    }
    return RTC_OK;
}

/**
 * filter_io_read_number, reads a single number from input file
 * @param gFile GridFile*  struct with additional information about the file
 * @param var void* variable to write to
 * @param type, the DataType to read
 * @return status, error code - 0 for OK, -1 for error
 */
int FilterIoPlot3d::filter_io_readObject(GridFile* gFile, void* var, DataTypeId typeId) {
    int status = RTC_OK;
    if (gFile->rMode == ASCII) {
        const char* format = dataTypes[typeId].scanFormat;
        status = filter_io_readObject_ASCII(gFile, var, format);
    } else {
        int size = dataTypes[typeId].onDiskSize;
        status = filter_io_readObject_Binary(gFile, var, size);
    }
    return status;
}

/**
 * filter_io_read_numbers, reads [count] amount of numbers from input file
 * @param gFile GridFile*  struct with additional information about the file
 * @param var void* variable to write to
 * @param type, the DataType to read
 * @return status, error code - 0 for OK, -1 for error
 */
int FilterIoPlot3d::filter_io_readBuffer(GridFile* gFile, void* var, Int64 count, DataTypeId typeId) {
    int err_state = RTC_OK;

    if (gFile->rMode == ASCII) {
        size_t size = dataTypes[typeId].inMemSize;
        const char* format = dataTypes[typeId].scanFormat;
        err_state = filter_io_readBuffer_ASCII(gFile, (char*)var, format, size, count);
    } else {
        int size = dataTypes[typeId].onDiskSize;
        err_state = filter_io_readBuffer_Binary(gFile, var, size, count);
    }
    return err_state;
}

/**
 * filter_io_open_FortranBlock, Opens a new FORTRAN array-block for reading.
 * Checks the byte-count header, and stores the expected
 * end of block position in the GridFile->OpenFblockSize
 *
 * @param gFile GridFile*  struct with additional information about the file
 * @return Int64, the byte size of the block
 */
Int64 FilterIoPlot3d::filter_io_open_FortranBlock(GridFile * gFile) {
    int cnt=0;
    UNUSED(cnt);
    if (gFile->rMode == ASCII || !gFile->usebytecount) {
        return 0;
    }
    Int64 blockSize = 0;
    if (gFile->OpenFblockEnd == 0) {
        Int64 loc = ftell(gFile->handle);
        UNUSED(loc);
        cnt=fread(&blockSize, 1, 4, gFile->handle);
        Int64 newloc = ftell(gFile->handle);
        UNUSED(newloc);
        if (gFile->needsByteSwap) {
            filter_io_swap_bufferEndian(&blockSize, 4, 1);
        }
        gFile->OpenFblockEnd = blockSize + loc + 4;
        DEBUG(3,"##FBLOCK: Begin at:%ld, data at: %ld, size: %ld, next block: %ld\n", loc, newloc, blockSize, newloc + blockSize + 4);
    } else {
        LOGOUT(FILTER_ERR,(char*)"##ERROR:!! FBLOCK ALREADY OPEN!!!\n");
        exit(0);
    }
    return blockSize;
}

void FilterIoPlot3d::filter_io_skipToOffset(GridFile * gFile, Int64 bytes) {
    fseek(gFile->handle, bytes, SEEK_SET);
}

/**
 * filter_io_close_FortranBlock, Closes the open FORTRAN array-block.
 * Checks the file location (ftell) and compares against the expected position
 * Produces a warning if they dont't match.
 * end of block position in the GridFile->OpenFblockSize
 *
 * @param gFile GridFile*  struct with additional information about the file
 */
void FilterIoPlot3d::filter_io_close_FortranBlock(GridFile * gFile) {
    int cnt;
    if (gFile->rMode == ASCII || !gFile->usebytecount) {
        return;
    }
    Int64 tmp = 0;
    Int64 loc = ftell(gFile->handle);
    cnt=fread(&tmp, 1, 4, gFile->handle);
    if (gFile->needsByteSwap) {
        filter_io_swap_bufferEndian(&tmp, 4, 1);
        DEBUG(3,"##FBLOCK: End at:%ld, tail size: %ld\n", loc, tmp);
    }
    if (loc != gFile->OpenFblockEnd || cnt == 0) {
        LOGOUT(FILTER_ERR, (char*)"##ERROR: !! FBLOCK: SIZE INCORRECT !!( End at %ld, but expected:%ld)\n", loc, gFile->OpenFblockEnd);
        ASSERT_FAIL("Incorrect FBLOCK size, verify _mode_usebytecount settings");
    }
    gFile->OpenFblockEnd = 0;
}

} /* namespace filter */
} /* namespace pbvr */
