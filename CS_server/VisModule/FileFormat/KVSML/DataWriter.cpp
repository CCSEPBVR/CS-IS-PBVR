/*****************************************************************************/
/**
 *  @file   DataWriter.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DataWriter.cpp 667 2011-02-22 16:07:54Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "DataWriter.h"
#include "TagBase.h"
#include "CoordTag.h"
#include "ColorTag.h"
#include "NormalTag.h"
#include "SizeTag.h"
#include "ConnectionTag.h"
#include "OpacityTag.h"
#include <vismodule/DataArrayTag>
#include "DataValueTag.h"
#include <vismodule/Message>
#include <vismodule/ValueArray>
#include <vismodule/XMLNode>


namespace vismodule
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Writes coordinate data to <Coord>.
 *  @param  parent [out] pointer to the parent node
 *  @param  writing_type [in] writing data type
 *  @param  filename [in] filename
 *  @param  coords [in] coordinate value array
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool WriteCoordData(
    vismodule::XMLNode::SuperClass* parent,
    const vismodule::kvsml::WritingDataType writing_type,
    const std::string& filename,
    const vismodule::ValueArray<vismodule::Real32>& coords )
{
    // <Coord>
    if ( coords.size() > 0 )
    {
        vismodule::kvsml::CoordTag coord_tag;
        if ( !coord_tag.write( parent ) )
        {
            visModuleMessageError( "Cannot write <%s>.", coord_tag.name().c_str() );
            return( false );
        }

        // <DataArray>
        vismodule::kvsml::DataArrayTag data_tag;
        if ( writing_type == vismodule::kvsml::ExternalAscii )
        {
            data_tag.setFile( vismodule::kvsml::DataArray::GetDataFilename( filename, "coord" ) );
            data_tag.setFormat( "ascii" );
        }
        else if ( writing_type == vismodule::kvsml::ExternalBinary )
        {
            data_tag.setFile( vismodule::kvsml::DataArray::GetDataFilename( filename, "coord" ) );
            data_tag.setFormat( "binary" );
        }

        const std::string pathname = vismodule::File( filename ).pathName();
        if ( !data_tag.write( coord_tag.node(), coords, pathname ) )
        {
            visModuleMessageError( "Cannot write <%s> for <%s>.",
                             data_tag.name().c_str(),
                             coord_tag.name().c_str() );
            return( false );
        }
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes color data to <Color>.
 *  @param  parent [out] pointer to the parent node
 *  @param  writing_type [in] writing data type
 *  @param  filename [in] filename
 *  @param  colors [in] color value array
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool WriteColorData(
    vismodule::XMLNode::SuperClass* parent,
    const vismodule::kvsml::WritingDataType writing_type,
    const std::string& filename,
    const vismodule::ValueArray<vismodule::UInt8>& colors )
{
    // <Color>
    if ( colors.size() > 0 )
    {
        vismodule::kvsml::ColorTag color_tag;
        if ( !color_tag.write( parent ) )
        {
            visModuleMessageError( "Cannot write <%s>.", color_tag.name().c_str() );
            return( false );
        }

        // <DataValue>
        if ( colors.size() == 3 )
        {
            vismodule::kvsml::DataValueTag data_tag;
            if ( !data_tag.write( color_tag.node(), colors ) )
            {
                visModuleMessageError( "Cannot write <%s> for <%s>.",
                                 data_tag.name().c_str(),
                                 color_tag.name().c_str() );
                return( false );
            }
        }
        // <DataArray>
        else
        {
            vismodule::kvsml::DataArrayTag data_tag;
            if ( writing_type == vismodule::kvsml::ExternalAscii )
            {
                data_tag.setFile( vismodule::kvsml::DataArray::GetDataFilename( filename, "color" ) );
                data_tag.setFormat( "ascii" );
            }
            else if ( writing_type == vismodule::kvsml::ExternalBinary )
            {
                data_tag.setFile( vismodule::kvsml::DataArray::GetDataFilename( filename, "color" ) );
                data_tag.setFormat( "binary" );
            }

            const std::string pathname = vismodule::File( filename ).pathName();
            if ( !data_tag.write( color_tag.node(), colors, pathname ) )
            {
                visModuleMessageError( "Cannot write <%s> for <%s>.",
                                 data_tag.name().c_str(),
                                 color_tag.name().c_str() );
                return( false );
            }
        }
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes normal data to <Coord>.
 *  @param  parent [out] pointer to the parent node
 *  @param  writing_type [in] writing data type
 *  @param  filename [in] filename
 *  @param  normals [in] normal value array
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool WriteNormalData(
    vismodule::XMLNode::SuperClass* parent,
    const vismodule::kvsml::WritingDataType writing_type,
    const std::string& filename,
    const vismodule::ValueArray<vismodule::Real32>& normals )
{
    // <Normal>
    if ( normals.size() > 0 )
    {
        vismodule::kvsml::NormalTag normal_tag;
        if ( !normal_tag.write( parent ) )
        {
            visModuleMessageError( "Cannot write <%s>.", normal_tag.name().c_str() );
            return( false );
        }

        // <DataValue>
        if ( normals.size() == 3 )
        {
            vismodule::kvsml::DataValueTag data_tag;
            if ( !data_tag.write( normal_tag.node(), normals ) )
            {
                visModuleMessageError( "Cannot write <%s> for <%s>.",
                                 data_tag.name().c_str(),
                                 normal_tag.name().c_str() );
                return( false );
            }
        }
        // <DataArray>
        else
        {
            vismodule::kvsml::DataArrayTag data_tag;
            if ( writing_type == vismodule::kvsml::ExternalAscii )
            {
                data_tag.setFile( vismodule::kvsml::DataArray::GetDataFilename( filename, "normal" ) );
                data_tag.setFormat( "ascii" );
            }
            else if ( writing_type == vismodule::kvsml::ExternalBinary )
            {
                data_tag.setFile( vismodule::kvsml::DataArray::GetDataFilename( filename, "normal" ) );
                data_tag.setFormat( "binary" );
            }

            const std::string pathname = vismodule::File( filename ).pathName();
            if ( !data_tag.write( normal_tag.node(), normals, pathname ) )
            {
                visModuleMessageError( "Cannot write <%s> for <%s>.",
                                 data_tag.name().c_str(),
                                 normal_tag.name().c_str() );
                return( false );
            }
        }
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes size data to <Coord>.
 *  @param  parent [out] pointer to the parent node
 *  @param  writing_type [in] writing data type
 *  @param  filename [in] filename
 *  @param  sizes [in] size value array
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool WriteSizeData(
    vismodule::XMLNode::SuperClass* parent,
    const vismodule::kvsml::WritingDataType writing_type,
    const std::string& filename,
    const vismodule::ValueArray<vismodule::Real32>& sizes )
{
    // <Size>
    if ( sizes.size() > 0 )
    {
        vismodule::kvsml::SizeTag size_tag;
        if ( !size_tag.write( parent ) )
        {
            visModuleMessageError( "Cannot write <%s>.", size_tag.name().c_str() );
            return( false );
        }

        // <DataValue>
        if ( sizes.size() == 1 )
        {
            vismodule::kvsml::DataValueTag data_tag;
            if ( !data_tag.write( size_tag.node(), sizes ) )
            {
                visModuleMessageError( "Cannot write <%s> for <%s>.",
                                 data_tag.name().c_str(),
                                 size_tag.name().c_str() );
                return( false );
            }
        }
        // <DataArray>
        else
        {
            vismodule::kvsml::DataArrayTag data_tag;
            if ( writing_type == vismodule::kvsml::ExternalAscii )
            {
                data_tag.setFile( vismodule::kvsml::DataArray::GetDataFilename( filename, "size" ) );
                data_tag.setFormat( "ascii" );
            }
            else if ( writing_type == vismodule::kvsml::ExternalBinary )
            {
                data_tag.setFile( vismodule::kvsml::DataArray::GetDataFilename( filename, "size" ) );
                data_tag.setFormat( "binary" );
            }

            const std::string pathname = vismodule::File( filename ).pathName();
            if ( !data_tag.write( size_tag.node(), sizes, pathname ) )
            {
                visModuleMessageError( "Cannot write <%s> for <%s>.",
                                 data_tag.name().c_str(),
                                 size_tag.name().c_str() );
                return( false );
            }
        }
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes connection data to <Coord>.
 *  @param  parent [out] pointer to the parent node
 *  @param  writing_type [in] writing data type
 *  @param  filename [in] filename
 *  @param  connection [in] connection value array
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool WriteConnectionData(
    vismodule::XMLNode::SuperClass* parent,
    const vismodule::kvsml::WritingDataType writing_type,
    const std::string& filename,
    const vismodule::ValueArray<vismodule::UInt32>& connections )
{
    // <Connection>
    if ( connections.size() > 0 )
    {
        vismodule::kvsml::ConnectionTag connection_tag;
        if ( !connection_tag.write( parent ) )
        {
            visModuleMessageError( "Cannot write <%s>.", connection_tag.name().c_str() );
            return( false );
        }

        // <DataArray>
        vismodule::kvsml::DataArrayTag data_tag;
        if ( writing_type == vismodule::kvsml::ExternalAscii )
        {
            data_tag.setFile( vismodule::kvsml::DataArray::GetDataFilename( filename, "connect" ) );
            data_tag.setFormat( "ascii" );
        }
        else if ( writing_type == vismodule::kvsml::ExternalBinary )
        {
            data_tag.setFile( vismodule::kvsml::DataArray::GetDataFilename( filename, "connect" ) );
            data_tag.setFormat( "binary" );
        }

        const std::string pathname = vismodule::File( filename ).pathName();
        if ( !data_tag.write( connection_tag.node(), connections, pathname ) )
        {
            visModuleMessageError( "Cannot write <%s> for <%s>.",
                             data_tag.name().c_str(),
                             connection_tag.name().c_str() );
            return( false );
        }
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes opacity data to <Coord>.
 *  @param  parent [out] pointer to the parent node
 *  @param  writing_type [in] writing data type
 *  @param  filename [in] filename
 *  @param  opacity [in] opacity value array
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool WriteOpacityData(
    vismodule::XMLNode::SuperClass* parent,
    const vismodule::kvsml::WritingDataType writing_type,
    const std::string& filename,
    const vismodule::ValueArray<vismodule::UInt8>& opacities )
{
    // <Opacity>
    if ( opacities.size() > 0 )
    {
        vismodule::kvsml::OpacityTag opacity_tag;
        if ( !opacity_tag.write( parent ) )
        {
            visModuleMessageError( "Cannot write <%s>.", opacity_tag.name().c_str() );
            return( false );
        }

        // <DataValue>
        if ( opacities.size() == 1 )
        {
            vismodule::kvsml::DataValueTag data_tag;
            if ( !data_tag.write( opacity_tag.node(), opacities ) )
            {
                visModuleMessageError( "Cannot write <%s> for <%s>.",
                                 data_tag.name().c_str(),
                                 opacity_tag.name().c_str() );
                return( false );
            }
        }
        // <DataArray>
        else
        {
            vismodule::kvsml::DataArrayTag data_tag;
            if ( writing_type == vismodule::kvsml::ExternalAscii )
            {
                data_tag.setFile( vismodule::kvsml::DataArray::GetDataFilename( filename, "opacity" ) );
                data_tag.setFormat( "ascii" );
            }
            else if ( writing_type == vismodule::kvsml::ExternalBinary )
            {
                data_tag.setFile( vismodule::kvsml::DataArray::GetDataFilename( filename, "opacity" ) );
                data_tag.setFormat( "binary" );
            }

            const std::string pathname = vismodule::File( filename ).pathName();
            if ( !data_tag.write( opacity_tag.node(), opacities, pathname ) )
            {
                visModuleMessageError( "Cannot write <%s> for <%s>.",
                                 data_tag.name().c_str(),
                                 opacity_tag.name().c_str() );
                return( false );
            }
        }
    }

    return( true );
}

} // end of namespace kvsml

} // end of namespace vismodule
