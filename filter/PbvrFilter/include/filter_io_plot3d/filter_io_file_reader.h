/*
 * File:   filter_io_file_reader.h
 * Author: Martin Andersson, V.I.C
 *
 * Created on September 29, 2015, 10:03 AM
 */

#ifndef FILTER_IO_FILE_READER_H
#define	FILTER_IO_FILE_READER_H

#include "filter_io_plot3d/filter_io_types.h"

namespace pbvr {
namespace filter {

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

} /* namespace filter */
} /* namespace pbvr */

#endif	/* FILTER_IO_FILE_READER_H */

