/* 
 * File:   filter_io_file_reader.h
 * Author: Martin Andersson, V.I.C 
 *
 * Created on September 29, 2015, 10:03 AM
 */

#ifndef FILTER_IO_FILE_READER_H
#define	FILTER_IO_FILE_READER_H
#include <stdio.h>
#include "filter_io_types.h"

typedef struct GridFile
{
    FILE* handle;
    ReadMode rMode;
    bool needsByteSwap;
    Int64 OpenFblockEnd;
    const char* filePath;
    bool usebytecount;
    DataTypeId blockDataTypeId;
} GridFile;

int filter_io_open_DataFile(GridFile* gFile, const char* filepath, ReadMode rMode);
int filter_io_close_dataFile(GridFile* gFile);
int filter_io_readObject(GridFile* gFile, void* var, DataTypeId type);
int filter_io_readBuffer(GridFile* gFile, void* var, Int64 count, DataTypeId typeId);
void filter_io_close_FortranBlock(GridFile * gFile);
Int64 filter_io_open_FortranBlock(GridFile * gFile);
int filter_io_checkFileEndian(GridFile* gFile);
void filter_io_skipToOffset(GridFile* gFile,Int64 bytes);
#endif	/* FILTER_IO_FILE_READER_H */

