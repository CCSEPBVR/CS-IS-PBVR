/*****************************************************************************/
/**
 *  @file   DataWriter.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DataWriter.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__KVSML__DATA_WRITER_H_INCLUDE
#define KVS__KVSML__DATA_WRITER_H_INCLUDE

#include <kvs/ValueArray>
#include <kvs/XMLNode>


namespace kvs
{

namespace kvsml
{

enum WritingDataType
{
    Ascii = 0,     ///< ascii data type
    ExternalAscii, ///< external ascii data type
    ExternalBinary ///< external binary data type
};

const bool WriteCoordData(
    kvs::XMLNode::SuperClass* parent,
    const kvs::kvsml::WritingDataType writing_type,
    const std::string& filename,
    const kvs::ValueArray<kvs::Real32>& coords );

const bool WriteColorData(
    kvs::XMLNode::SuperClass* parent,
    const kvs::kvsml::WritingDataType writing_type,
    const std::string& filename,
    const kvs::ValueArray<kvs::UInt8>& colors );

const bool WriteNormalData(
    kvs::XMLNode::SuperClass* parent,
    const kvs::kvsml::WritingDataType writing_type,
    const std::string& filename,
    const kvs::ValueArray<kvs::Real32>& normals );

const bool WriteSizeData(
    kvs::XMLNode::SuperClass* parent,
    const kvs::kvsml::WritingDataType writing_type,
    const std::string& filename,
    const kvs::ValueArray<kvs::Real32>& sizes );

const bool WriteConnectionData(
    kvs::XMLNode::SuperClass* parent,
    const kvs::kvsml::WritingDataType writing_type,
    const std::string& filename,
    const kvs::ValueArray<kvs::UInt32>& connections );

const bool WriteOpacityData(
    kvs::XMLNode::SuperClass* parent,
    const kvs::kvsml::WritingDataType writing_type,
    const std::string& filename,
    const kvs::ValueArray<kvs::UInt8>& opacities );

} // end of namespace kvsml

} // end of namespace kvs

#endif // KVS__KVSML__DATA_WRITER_H_INCLUDE
