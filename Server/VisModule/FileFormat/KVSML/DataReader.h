/*****************************************************************************/
/**
 *  @file   DataReader.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DataReader.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__KVSML__DATA_READER_H_INCLUDE
#define VIS_MODULE__KVSML__DATA_READER_H_INCLUDE

#include <vismodule/ValueArray>
#include <vismodule/XMLNode>


namespace vismodule
{

namespace kvsml
{

const bool ReadCoordData(
    const vismodule::XMLNode::SuperClass* parent,
    const std::size_t ncoords,
    vismodule::ValueArray<vismodule::Real32>* coords );

const bool ReadColorData(
    const vismodule::XMLNode::SuperClass* parent,
    const std::size_t ncolors,
    vismodule::ValueArray<vismodule::UInt8>* colors );

const bool ReadNormalData(
    const vismodule::XMLNode::SuperClass* parent,
    const std::size_t nnormals,
    vismodule::ValueArray<vismodule::Real32>* normals );

const bool ReadSizeData(
    const vismodule::XMLNode::SuperClass* parent,
    const std::size_t nsizes,
    vismodule::ValueArray<vismodule::Real32>* sizes );

const bool ReadConnectionData(
    const vismodule::XMLNode::SuperClass* parent,
    const std::size_t nconnections,
    vismodule::ValueArray<vismodule::UInt32>* connections );

const bool ReadOpacityData(
    const vismodule::XMLNode::SuperClass* parent,
    const std::size_t nopacities,
    vismodule::ValueArray<vismodule::UInt8>* opacities );

} // end of namespace kvsml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__DATA_READER_H_INCLUDE
