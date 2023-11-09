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
#ifndef KVS__KVSML__DATA_READER_H_INCLUDE
#define KVS__KVSML__DATA_READER_H_INCLUDE

#include <kvs/ValueArray>
#include <kvs/XMLNode>


namespace kvs
{

namespace kvsml
{

const bool ReadCoordData(
    const kvs::XMLNode::SuperClass* parent,
    const size_t ncoords,
    kvs::ValueArray<kvs::Real32>* coords );

const bool ReadColorData(
    const kvs::XMLNode::SuperClass* parent,
    const size_t ncolors,
    kvs::ValueArray<kvs::UInt8>* colors );

const bool ReadNormalData(
    const kvs::XMLNode::SuperClass* parent,
    const size_t nnormals,
    kvs::ValueArray<kvs::Real32>* normals );

const bool ReadSizeData(
    const kvs::XMLNode::SuperClass* parent,
    const size_t nsizes,
    kvs::ValueArray<kvs::Real32>* sizes );

const bool ReadConnectionData(
    const kvs::XMLNode::SuperClass* parent,
    const size_t nconnections,
    kvs::ValueArray<kvs::UInt32>* connections );

const bool ReadOpacityData(
    const kvs::XMLNode::SuperClass* parent,
    const size_t nopacities,
    kvs::ValueArray<kvs::UInt8>* opacities );

} // end of namespace kvsml

} // end of namespace kvs

#endif // KVS__KVSML__DATA_READER_H_INCLUDE
