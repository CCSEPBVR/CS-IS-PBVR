/****************************************************************************/
/**
 *  @file KVSMLObjectPlotOverTime.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KVSMLObjectPlotOverTime.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "KVSMLObjectPlotOverTime.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include "../../../Shared/json.hpp"

namespace
{

void PrintPlotOverTimeError( const char* function_name, const int line, const std::string& message )
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

}

namespace vismodule
{
/*===========================================================================*/
/**
 *  @brief  Constructs a new KVSML Object Plot Over Time class.
 */
/*===========================================================================*/
KVSMLObjectPlotOverTime::KVSMLObjectPlotOverTime()
{
    m_mask = false;
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new KVSML Object Plot Over Time class.
 *  @param  filename [in] filename
 */
/*===========================================================================*/

KVSMLObjectPlotOverTime::KVSMLObjectPlotOverTime( const std::string& filename )
{
    m_mask = false;
    if ( this->read( filename ) ) { m_is_success = true; }
    else { m_is_success = false; }
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new KVSML Object Plot Over Time class.
 *  @param  values_on_time [in] value on time
 *  @param  mask [in] true, if the getting value is successfully
 */
/*===========================================================================*/

KVSMLObjectPlotOverTime::KVSMLObjectPlotOverTime(
    const vismodule::ValueArray<float> values_on_time,
    const bool mask
)
{
    setValuesOnTime( values_on_time );
    setMask( mask );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the KVSML Object Plot Ovet Time class.
 */
/*===========================================================================*/
KVSMLObjectPlotOverTime::~KVSMLObjectPlotOverTime()
{
}

void KVSMLObjectPlotOverTime::setValuesOnTime( const vismodule::ValueArray<float>& values_on_time )
{
    m_values_on_time = values_on_time;
}

void KVSMLObjectPlotOverTime::setMask( const bool mask )
{
    m_mask = mask;
}

/*===========================================================================*/
/**
 *  @brief  Read a KVSMl Plot Ovet Time file.
 *  @param  filename [in] filename
 *  @return true, if the reading process is successfully
 */
/*===========================================================================*/
const bool KVSMLObjectPlotOverTime::read( const std::string& filename )
{
    std::ifstream file( filename );

    if ( !file.is_open() ) {
        std::cerr << "ERROR:" << __FILE__ << ", " << __func__ << ", " << __LINE__ << ", " << "Failed to open the file: " << filename << std::endl;
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
        if ( !root.contains( "mask" ) || !root.at( "mask" ).is_boolean() )
        {
            throw std::runtime_error( "missing boolean 'mask'" );
        }

        const bool mask = root.at( "mask" ).get<bool>();
        vismodule::ValueArray<float> values_on_time = FloatArray( root, "values_on_time" );

        m_filename = filename;
        m_mask = mask;
        m_values_on_time = values_on_time;
    }
    catch ( const std::exception& e )
    {
        PrintPlotOverTimeError( __func__, __LINE__, "Failed to load JSON file '" + filename + "': " + e.what() );
        return false;
    }

    return true;
}

/*===========================================================================*/
/**
 *  @brief  Writes the KVSML Plot Over Time file.
 *  @param  filename [in] filename
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool KVSMLObjectPlotOverTime::write( const std::string& filename )
{
    std::ofstream file( filename );

    if ( !file.is_open() ) {
        std::cerr << "Failed to open the file: " << filename << std::endl;
        return false;
    }

    file << "{\n";
    file << "    \"mask\": " << ( m_mask ? "true" : "false" ) << ",\n";
    file << "    \"values_on_time\": " << InlineFloatArray( m_values_on_time ) << "\n";
    file << "}\n";

    return true;
}

/*===========================================================================*/
/**
 *  @brief  Output operator.
 *  @param  os [out] output stream
 *  @param  rhs [in] KVSML Object Plot Over Time Class
 */
/*===========================================================================*/
std::ostream& operator <<( std::ostream& os, const KVSMLObjectPlotOverTime& rhs )
{
    // mask
    if ( rhs.m_mask ) os << "m_mask: TRUE" << std::endl;
    else os << "m_mask: FALSE" << std::endl;

    // values over time
    for ( std::size_t i = 0; i < rhs.m_values_on_time.size(); i++ )
    {
        os << rhs.m_values_on_time[i] << ",";
    }
    os << std::endl;

    return( os );
}

} // end of namespace vismodule
