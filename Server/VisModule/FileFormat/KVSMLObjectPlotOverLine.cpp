/****************************************************************************/
/**
 *  @file KVSMLObjectPlotOverLine.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KVSMLObjectPlotOverLine.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "KVSMLObjectPlotOverLine.h"

#include <cmath>
#include <stdexcept>
#include "../../../Shared/json.hpp"

namespace
{

void PrintPlotOverLineError( const char* function_name, const int line, const std::string& message )
{
    std::cerr << "ERROR:" << __FILE__ << ", " << function_name << ", " << line << ", " << message << std::endl;
}

const nlohmann::json& RequiredArray( const nlohmann::json& root, const std::string& key )
{
    if ( !root.contains( key ) || !root.at( key ).is_array() )
    {
        throw std::runtime_error( "missing array '" + key + "'" );
    }
    return root.at( key );
}

vismodule::ValueArray<float> FloatArray( const nlohmann::json& root, const std::string& key )
{
    const nlohmann::json& array = RequiredArray( root, key );
    vismodule::ValueArray<float> values;
    values.allocate( array.size() );

    for ( std::size_t i = 0; i < array.size(); i++ )
    {
        if ( !array.at( i ).is_number() )
        {
            throw std::runtime_error( "'" + key + "' contains a non-number value" );
        }
        values[i] = array.at( i ).get<float>();
    }

    return values;
}

vismodule::ValueArray<bool> BoolArray( const nlohmann::json& root, const std::string& key )
{
    const nlohmann::json& array = RequiredArray( root, key );
    vismodule::ValueArray<bool> values;
    values.allocate( array.size() );

    for ( std::size_t i = 0; i < array.size(); i++ )
    {
        if ( !array.at( i ).is_boolean() )
        {
            throw std::runtime_error( "'" + key + "' contains a non-boolean value" );
        }
        values[i] = array.at( i ).get<bool>();
    }

    return values;
}

float JsonSafeFloat( const float value )
{
    return std::isfinite( value ) ? value : 0.0f;
}

std::string InlineFloatArray( const vismodule::ValueArray<float>& values )
{
    std::ostringstream stream;
    stream << '[';
    for ( std::size_t i = 0; i < values.size(); i++ )
    {
        if ( i > 0 ) stream << ',';
        stream << JsonSafeFloat( values[i] );
    }
    stream << ']';
    return stream.str();
}

std::string InlineBoolArray( const vismodule::ValueArray<bool>& values )
{
    std::ostringstream stream;
    stream << '[';
    for ( std::size_t i = 0; i < values.size(); i++ )
    {
        if ( i > 0 ) stream << ',';
        stream << ( values[i] ? "true" : "false" );
    }
    stream << ']';
    return stream.str();
}

}

namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new KVSML point object class.
 */
/*===========================================================================*/
KVSMLObjectPlotOverLine::KVSMLObjectPlotOverLine()
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new KVSML point object class.
 *  @param  filename [in] filename
 */
/*===========================================================================*/

KVSMLObjectPlotOverLine::KVSMLObjectPlotOverLine( const std::string& filename )
{
    if ( this->read( filename ) ) { m_is_success = true; }
    else { m_is_success = false; }
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new KVSML point object class.
 *  @param  filename [in] filename
 */
/*===========================================================================*/

KVSMLObjectPlotOverLine::KVSMLObjectPlotOverLine( const vismodule::ValueArray<float> values_on_line, const vismodule::ValueArray<float> x_axis,
                                    const vismodule::ValueArray<bool> mask ) 
{
    setValuesOnLine(values_on_line);
    setXAxis(x_axis);
    setMask(mask);
}

/*===========================================================================*/
/**
 *  @brief  Destructs the KVSML point object class.
 */
/*===========================================================================*/
KVSMLObjectPlotOverLine::~KVSMLObjectPlotOverLine( void )
{
}

const vismodule::ValueArray<float>& KVSMLObjectPlotOverLine:: values_on_line( void ) const
{
    return( m_values_on_line );
}

const vismodule::ValueArray<float>& KVSMLObjectPlotOverLine::x_axis( void ) const
{
    return( m_x_axis );
}

const vismodule::ValueArray<bool>& KVSMLObjectPlotOverLine::mask( void ) const
{
    return( m_mask );
}


void KVSMLObjectPlotOverLine::setValuesOnLine( const vismodule::ValueArray<float>& values_on_line )
{
    m_values_on_line = values_on_line;
}

void KVSMLObjectPlotOverLine::setXAxis( const vismodule::ValueArray<float>& x_axis )
{
    m_x_axis = x_axis;
}

void KVSMLObjectPlotOverLine::setMask( const vismodule::ValueArray<bool>& mask )
{
    m_mask = mask;
}

void KVSMLObjectPlotOverLine::setResolution(const int resolution)
{
    m_values_on_line.allocate(resolution);
    m_x_axis.allocate(resolution);
    m_mask.allocate(resolution);
}
/*===========================================================================*/
/**
 *  @brief  Read a KVSMl point object file.
 *  @param  filename [in] filename
 *  @return true, if the reading process is successfully
 */
/*===========================================================================*/
const bool KVSMLObjectPlotOverLine::read( const std::string& filename )
{
    std::ifstream file( filename );

    if ( !file.is_open() )
    {
        PrintPlotOverLineError( __func__, __LINE__, "Failed to open the file: " + filename );
        return false;
    }

    try
    {
        nlohmann::json root;
        file >> root;

        if ( !root.is_object() )
        {
            throw std::runtime_error( "root is not an object" );
        }

        vismodule::ValueArray<float> x_axis = FloatArray( root, "x_axis" );
        vismodule::ValueArray<bool> mask = BoolArray( root, "mask" );
        vismodule::ValueArray<float> values_on_line = FloatArray( root, "values_on_line" );

        if ( x_axis.size() != mask.size() || x_axis.size() != values_on_line.size() )
        {
            throw std::runtime_error( "array sizes do not match" );
        }

        m_filename = filename;
        m_x_axis = x_axis;
        m_mask = mask;
        m_values_on_line = values_on_line;
    }
    catch ( const std::exception& e )
    {
        PrintPlotOverLineError( __func__, __LINE__, "Failed to load JSON file '" + filename + "': " + e.what() );
        return false;
    }

    return true;
}

/*===========================================================================*/
/**
 *  @brief  Writes the KVSML point object.
 *  @param  filename [in] filename
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool KVSMLObjectPlotOverLine::write( const std::string& filename )
{
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "ファイルを開けません: " << filename << std::endl;
        return false;
    }

    file << "{\n";
    file << "    \"x_axis\": " << InlineFloatArray( m_x_axis ) << ",\n";
    file << "    \"mask\": " << InlineBoolArray( m_mask ) << ",\n";
    file << "    \"values_on_line\": " << InlineFloatArray( m_values_on_line ) << "\n";
    file << "}\n";

    return true;
}

/*===========================================================================*/
/**
 *  @brief  Output operator.
 *  @param  os [out] output stream
 *  @param  rhs [in] KVSML point object
 */
/*===========================================================================*/
std::ostream& operator <<( std::ostream& os, const KVSMLObjectPlotOverLine& rhs )
{
    os << "Num. of resolution: " << rhs.m_values_on_line.size();

    return( os );
}

} // end of namespace vismodule
