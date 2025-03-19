/****************************************************************************/
/**
 *  @file TagTable.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TagTable.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__TIFF__TAG_TABLE_H_INCLUDE
#define VIS_MODULE__TIFF__TAG_TABLE_H_INCLUDE

#include "Tag.h"


namespace vismodule
{

namespace tiff
{

const vismodule::tiff::Tag TagTable[] =
{
    vismodule::tiff::Tag( 254, "subfile data descriptor" ),
    vismodule::tiff::Tag( 255, "kind of data in subfile" ),
    vismodule::tiff::Tag( 256, "image width in pixels" ),
    vismodule::tiff::Tag( 257, "image height in pixels" ),
    vismodule::tiff::Tag( 258, "bits per channel (sample)" ),
    vismodule::tiff::Tag( 259, "data compression technique" ),
    vismodule::tiff::Tag( 262, "photometric interpretation" ),
    vismodule::tiff::Tag( 263, "thresholding used on data" ),
    vismodule::tiff::Tag( 264, "dithering matrix width" ),
    vismodule::tiff::Tag( 265, "dithering matrix height" ),
    vismodule::tiff::Tag( 266, "data order within a byte" ),
    vismodule::tiff::Tag( 269, "name of doc. image is from" ),
    vismodule::tiff::Tag( 270, "info about image" ),
    vismodule::tiff::Tag( 271, "scanner manufacturer name" ),
    vismodule::tiff::Tag( 272, "scanner model name/number" ),
    vismodule::tiff::Tag( 273, "offsets to data strips" ),
    vismodule::tiff::Tag( 274, "image orientation" ),
    vismodule::tiff::Tag( 277, "samples per pixel" ),
    vismodule::tiff::Tag( 278, "rows per strip of data" ),
    vismodule::tiff::Tag( 279, "bytes counts for strips" ),
    vismodule::tiff::Tag( 280, "minimum sample value" ),
    vismodule::tiff::Tag( 281, "maximum sample value" ),
    vismodule::tiff::Tag( 282, "pixels/resolution in x" ),
    vismodule::tiff::Tag( 283, "pixels/resolution in y" ),
    vismodule::tiff::Tag( 284, "storage organization" ),
    vismodule::tiff::Tag( 285, "page name image is from" ),
    vismodule::tiff::Tag( 286, "x page offset of image lhs" ),
    vismodule::tiff::Tag( 287, "y page offset of image lhs" ),
    vismodule::tiff::Tag( 288, "byte offset to free block" ),
    vismodule::tiff::Tag( 289, "sizes of free blocks" ),
    vismodule::tiff::Tag( 290, "gray scale curve accuracy" ),
    vismodule::tiff::Tag( 291, "gray scale response curve" ),
    vismodule::tiff::Tag( 292, "32 flag bits" ),
    vismodule::tiff::Tag( 293, "32 flag bits" ),
    vismodule::tiff::Tag( 296, "units of resolutions" ),
    vismodule::tiff::Tag( 297, "page numbers of multi-page" ),
    vismodule::tiff::Tag( 300, "color curve accuracy" ),
    vismodule::tiff::Tag( 301, "colorimetry info" ),
    vismodule::tiff::Tag( 305, "name & release" ),
    vismodule::tiff::Tag( 306, "creation date and time" ),
    vismodule::tiff::Tag( 315, "creator of image" ),
    vismodule::tiff::Tag( 316, "machine where created" ),
    vismodule::tiff::Tag( 317, "prediction scheme w/ LZW" ),
    vismodule::tiff::Tag( 318, "image white point" ),
    vismodule::tiff::Tag( 319, "primary chromaticities" ),
    vismodule::tiff::Tag( 320, "RGB map for pallette image" ),
    vismodule::tiff::Tag( 321, "highlight+shadow info" ),
    vismodule::tiff::Tag( 322, "rows/data tile" ),
    vismodule::tiff::Tag( 323, "cols/data tile" ),
    vismodule::tiff::Tag( 324, "offsets to data tiles" ),
    vismodule::tiff::Tag( 325, "byte counts for tiles" ),
    vismodule::tiff::Tag( 326, "lines w/ wrong pixel count" ),
    vismodule::tiff::Tag( 327, "regenerated line info" ),
    vismodule::tiff::Tag( 328, "max consecutive bad lines" ),
    vismodule::tiff::Tag( 330, "subimage descriptors" ),
    vismodule::tiff::Tag( 332, "inks in separated image" ),
    vismodule::tiff::Tag( 333, "ascii names of inks" ),
    vismodule::tiff::Tag( 334, "number of inks" ),
    vismodule::tiff::Tag( 336, "dot range" ),
    vismodule::tiff::Tag( 337, "separation target" ),
    vismodule::tiff::Tag( 338, "info about extra samples" ),
    vismodule::tiff::Tag( 339, "data sample format" ),
    vismodule::tiff::Tag( 340, "variable MinSampleValue" ),
    vismodule::tiff::Tag( 341, "variable MaxSampleValue" ),
    vismodule::tiff::Tag( 347, "JPEG table stream" )
};

const size_t TagTableSize = sizeof( TagTable ) / sizeof( vismodule::tiff::Tag );

} // end of namespace tiff

} // end of namespace vismodule

#endif // VIS_MODULE__TIFF__TAG_TABLE_H_INCLUDE
