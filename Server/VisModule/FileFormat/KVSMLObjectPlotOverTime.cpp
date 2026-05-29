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

#include <fstream>
#include <iostream>

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
    m_filename = filename;

    if ( !file.is_open() ) {
        std::cerr << "ERROR:" << __FILE__ << ", " << __func__ << ", " << __LINE__ << ", " << "Failed to open the file: " << filename << std::endl;
        return false;
    }

    std::string line_buffer;
    std::vector<std::string> data;

    // mask
    std::getline( file, line_buffer );
    if ( line_buffer == "TRUE" ) m_mask = true;
    else if ( line_buffer == "FALSE" ) m_mask = false;
    else
    {
        std::cerr << "ERROR:" << __FILE__ << ", " << __func__ << ", " << __LINE__ << ", " << "Failed to load mask" << std::endl;
        m_mask = false;
    }

    // values on time 
    std::getline( file, line_buffer );
    // std::cout << "INFO:" << __FILE__ << ", " << __func__ << ", " << __LINE__ << ", " << "line_buffer:" << line_buffer << std::endl;
    data = this->split( line_buffer, ' ' );
    m_values_on_time.allocate( data.size() );
    for (size_t i = 0; i < data.size(); i++ )
    {
        m_values_on_time[i] = std::stof( data[i] );
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

    // mask
    if ( m_mask ) file << "TRUE" << std::endl;
    else file << "FALSE" << std::endl;

    // values over time
    for ( std::size_t i = 0; i < m_values_on_time.size(); i++ )
    {
        file << m_values_on_time[i] << " ";
    }
    file << std::endl;

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