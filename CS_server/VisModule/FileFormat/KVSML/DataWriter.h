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
#ifndef VIS_MODULE__KVSML__DATA_WRITER_H_INCLUDE
#define VIS_MODULE__KVSML__DATA_WRITER_H_INCLUDE

#include <vismodule/ValueArray>
#include <vismodule/XMLNode>


namespace vismodule
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
    vismodule::XMLNode::SuperClass* parent,
    const vismodule::kvsml::WritingDataType writing_type,
    const std::string& filename,
    const vismodule::ValueArray<vismodule::Real32>& coords );

const bool WriteColorData(
    vismodule::XMLNode::SuperClass* parent,
    const vismodule::kvsml::WritingDataType writing_type,
    const std::string& filename,
    const vismodule::ValueArray<vismodule::UInt8>& colors );

const bool WriteNormalData(
    vismodule::XMLNode::SuperClass* parent,
    const vismodule::kvsml::WritingDataType writing_type,
    const std::string& filename,
    const vismodule::ValueArray<vismodule::Real32>& normals );

const bool WriteSizeData(
    vismodule::XMLNode::SuperClass* parent,
    const vismodule::kvsml::WritingDataType writing_type,
    const std::string& filename,
    const vismodule::ValueArray<vismodule::Real32>& sizes );

const bool WriteConnectionData(
    vismodule::XMLNode::SuperClass* parent,
    const vismodule::kvsml::WritingDataType writing_type,
    const std::string& filename,
    const vismodule::ValueArray<vismodule::UInt32>& connections );

const bool WriteOpacityData(
    vismodule::XMLNode::SuperClass* parent,
    const vismodule::kvsml::WritingDataType writing_type,
    const std::string& filename,
    const vismodule::ValueArray<vismodule::UInt8>& opacities );

} // end of namespace kvsml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__DATA_WRITER_H_INCLUDE
