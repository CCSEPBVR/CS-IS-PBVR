/*****************************************************************************/
/**
 *  @file   DataReader.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DataReader.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "DataReader.h"
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
 *  @brief  Reads coordinate data from <Coord>.
 *  @param  parent  [in] pointer to the parent node
 *  @param  ncoords [in] number of coordinates
 *  @param  coords [out] pointer to the coordinate value array
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool ReadCoordData(
    const vismodule::XMLNode::SuperClass* parent,
    const std::size_t ncoords,
    vismodule::ValueArray<vismodule::Real32>* coords )
{
    // <Coord>
    vismodule::kvsml::CoordTag coord_tag;
    if ( coord_tag.isExisted( parent ) )
    {
        if ( !coord_tag.read( parent ) )
        {
            visModuleMessageError( "Cannot read <%s>.", coord_tag.name().c_str() );
            return( false );
        }

        // <DataArray>
        const std::size_t dimension = 3;
        const std::size_t nelements = ncoords * dimension;
        vismodule::kvsml::DataArrayTag data_tag;
        if ( !data_tag.read( coord_tag.node(), nelements, coords ) )
        {
            visModuleMessageError( "Cannot read <%s> for <%s>.",
                             data_tag.name().c_str(),
                             coord_tag.name().c_str() );
            return( false );
        }
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Reads color data from <Color>.
 *  @param  parent  [in] pointer to the parent node
 *  @param  ncoords [in] number of colors
 *  @param  colors [out] pointer to the color value array
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool ReadColorData(
    const vismodule::XMLNode::SuperClass* parent,
    const std::size_t ncolors,
    vismodule::ValueArray<vismodule::UInt8>* colors )
{
    // <Color>
    vismodule::kvsml::ColorTag color_tag;
    if ( color_tag.isExisted( parent ) )
    {
        if ( !color_tag.read( parent ) )
        {
            visModuleMessageError( "Cannot read <%s>.", color_tag.name().c_str() );
            return( false );
        }

        // <DataValue>
        if ( vismodule::XMLNode::FindChildNode( color_tag.node(), "DataValue" ) )
        {
            // In this case, 'ncolors' is equal to 1.
            const std::size_t nchannels = 3; // RGB
            const std::size_t nelements = nchannels; // = 1 * nchannels
            vismodule::kvsml::DataValueTag data_tag;
            if ( !data_tag.read( color_tag.node(), nelements, colors ) )
            {
                visModuleMessageError( "Cannot read <%s> for <%s>.",
                                 data_tag.name().c_str(),
                                 color_tag.name().c_str() );
                return( false );
            }
        }
        // <DataArray>
        else
        {
            const std::size_t nchannels = 3; // RGB
            const std::size_t nelements = ncolors * nchannels;
            vismodule::kvsml::DataArrayTag data_tag;
            if ( !data_tag.read( color_tag.node(), nelements, colors ) )
            {
                visModuleMessageError( "Cannot read <%s> for <%s>.",
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
 *  @brief  Reads normal data from <Color>.
 *  @param  parent  [in] pointer to the parent node
 *  @param  ncoords [in] number of normals
 *  @param  normals [out] pointer to the normal value array
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool ReadNormalData(
    const vismodule::XMLNode::SuperClass* parent,
    const std::size_t nnormals,
    vismodule::ValueArray<vismodule::Real32>* normals )
{
    // <Normal>
    vismodule::kvsml::NormalTag normal_tag;
    if ( normal_tag.isExisted( parent ) )
    {
        if ( !normal_tag.read( parent ) )
        {
            visModuleMessageError( "Cannot read <%s>.", normal_tag.name().c_str() );
            return( false );
        }

        // <DataValue>
        if ( vismodule::XMLNode::FindChildNode( normal_tag.node(), "DataValue" ) )
        {
            // In this case, 'nnormals' is equal to 1.
            const std::size_t dimension = 3;
            const std::size_t nelements = dimension; // = 1 * dimension
            vismodule::kvsml::DataValueTag data_tag;
            if ( !data_tag.read( normal_tag.node(), nelements, normals ) )
            {
                visModuleMessageError( "Cannot read <%s> for <%s>.",
                                 data_tag.name().c_str(),
                                 normal_tag.name().c_str() );
                return( false );
            }
        }
        // <DataArray>
        else
        {
            const std::size_t dimension = 3;
            const std::size_t nelements = nnormals * dimension;
            vismodule::kvsml::DataArrayTag data_tag;
            if ( !data_tag.read( normal_tag.node(), nelements, normals ) )
            {
                visModuleMessageError( "Cannot read <%s> for <%s>.",
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
 *  @brief  Reads size data from <Size>.
 *  @param  parent  [in] pointer to the parent node
 *  @param  ncoords [in] number of sizes
 *  @param  sizes [out] pointer to the size value array
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool ReadSizeData(
    const vismodule::XMLNode::SuperClass* parent,
    const std::size_t nsizes,
    vismodule::ValueArray<vismodule::Real32>* sizes )
{
    // <Size>
    vismodule::kvsml::SizeTag size_tag;
    if ( size_tag.isExisted( parent ) )
    {
        if ( !size_tag.read( parent ) )
        {
            visModuleMessageError( "Cannot read <%s>.", size_tag.name().c_str() );
            return( false );
        }

        // <DataValue>
        if ( vismodule::XMLNode::FindChildNode( size_tag.node(), "DataValue" ) )
        {
            const std::size_t nelements = 1;
            vismodule::kvsml::DataValueTag data_tag;
            if ( !data_tag.read( size_tag.node(), nelements, sizes ) )
            {
                visModuleMessageError( "Cannot read <%s> for <%s>.",
                                 data_tag.name().c_str(),
                                 size_tag.name().c_str() );
                return( false );
            }
        }
        // <DataArray>
        else
        {
            const std::size_t nelements = nsizes;
            vismodule::kvsml::DataArrayTag data_tag;
            if ( !data_tag.read( size_tag.node(), nelements, sizes ) )
            {
                visModuleMessageError( "Cannot read <%s> for <%s>.",
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
 *  @brief  Reads connection data from <Connection>.
 *  @param  parent  [in] pointer to the parent node
 *  @param  ncoords [in] number of connections
 *  @param  connections [out] pointer to the connection value array
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool ReadConnectionData(
    const vismodule::XMLNode::SuperClass* parent,
    const std::size_t nconnections,
    vismodule::ValueArray<vismodule::UInt32>* connections )
{
    // <Connection>
    vismodule::kvsml::ConnectionTag connection_tag;
    if ( connection_tag.isExisted( parent ) )
    {
        if ( !connection_tag.read( parent ) )
        {
            visModuleMessageError( "Cannot read <%s>.", connection_tag.name().c_str() );
            return( false );
        }

        // <DataArray>
        const std::size_t nelements = nconnections;
        vismodule::kvsml::DataArrayTag data_tag;
        if ( !data_tag.read( connection_tag.node(), nelements, connections ) )
        {
            visModuleMessageError( "Cannot read <%s> for <%s>.",
                             data_tag.name().c_str(),
                             connection_tag.name().c_str() );
            return( false );
        }
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Reads opacity data from <Opacity>.
 *  @param  parent  [in] pointer to the parent node
 *  @param  ncoords [in] number of opacities
 *  @param  opacities [out] pointer to the opacity value array
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool ReadOpacityData(
    const vismodule::XMLNode::SuperClass* parent,
    const std::size_t nopacities,
    vismodule::ValueArray<vismodule::UInt8>* opacities )
{
    // <Opacity>
    vismodule::kvsml::OpacityTag opacity_tag;
    if ( opacity_tag.isExisted( parent ) )
    {
        if ( !opacity_tag.read( parent ) )
        {
            visModuleMessageError( "Cannot read <%s>.", opacity_tag.name().c_str() );
            return( false );
        }

        // <DataValue>
        if ( vismodule::XMLNode::FindChildNode( opacity_tag.node(), "DataValue" ) )
        {
            const std::size_t nelements = 1;
            vismodule::kvsml::DataValueTag data_tag;
            if ( !data_tag.read( opacity_tag.node(), nelements, opacities ) )
            {
                visModuleMessageError( "Cannot read <%s> for <%s>.",
                                 data_tag.name().c_str(),
                                 opacity_tag.name().c_str() );
                return( false );
            }
        }
        // <DataArray>
        else
        {
            const std::size_t nelements = nopacities;
            vismodule::kvsml::DataArrayTag data_tag;
            if ( !data_tag.read( opacity_tag.node(), nelements, opacities ) )
            {
                visModuleMessageError( "Cannot read <%s> for <%s>.",
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
