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
    std::ifstream file(filename);
    m_filename = filename;
    std::string line_buffer;
    
    //std::getline(file, line_buffer);
    //std::cout << line_buffer << std::endl; 
    //int nPoints = std::stoi( line_buffer ); 

    std::vector<std::string> data;
    // xaxis 
    std::getline(file, line_buffer);
    data.clear();
    data = this->split( line_buffer, ' ' );
    m_x_axis.allocate(data.size());
    for (int i =0; i< data.size(); i ++ )
    {
        m_x_axis[i] = std::stof(data[i]);
    }

    // mask 
    std::getline(file, line_buffer);
    data.clear();
    data = this->split( line_buffer, ' ' );
    m_mask.allocate(data.size());
    for (int i =0; i< data.size(); i ++ )
    {
        if(strcmp(data[i].c_str(), "1") ==0 ) m_mask[i] = true;
        else m_mask[i] = false;
    }

    // values on line 
    std::getline(file, line_buffer);
    data.clear();
    data = this->split( line_buffer, ' ' );
    m_values_on_line.allocate(data.size());
    for (int i =0; i< data.size(); i ++ )
    {
        m_values_on_line[i] = std::stof(data[i]);
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

    //int nPoints = m_sizes.size()  ; 
    for (int i =0; i < m_x_axis.size(); i++ )
    {
        file <<  m_x_axis[i] <<  " " ;
    }
    file << std::endl;
    for (int i = 0; i < m_mask.size(); i++ )
    {
        if ( m_mask[i] ) file << 1 <<  " ";
        else             file << 0 <<  " ";
    }
    file << std::endl;
    for (int i =0; i < m_values_on_line.size(); i++ )
    {
        file <<  m_values_on_line[i]  <<  " " ;
    }
    file << std::endl;

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
