/****************************************************************************/
/**
 *  @file KVSMLObjectGlyph.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KVSMLObjectGlyph.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "KVSMLObjectGlyph.h"

namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new KVSML point object class.
 */
/*===========================================================================*/
KVSMLObjectGlyph::KVSMLObjectGlyph()
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new KVSML point object class.
 *  @param  filename [in] filename
 */
/*===========================================================================*/

KVSMLObjectGlyph::KVSMLObjectGlyph( const std::string& filename )
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

KVSMLObjectGlyph::KVSMLObjectGlyph( const vismodule::ValueArray<vismodule::Real32> coords, const vismodule::ValueArray<vismodule::UInt8>  colors,      
                                    const vismodule::ValueArray<vismodule::Real32> directions, const vismodule::ValueArray<vismodule::Real32> sizes ) 
{
    setCoords(coords);
    setColors(colors);
    setDirections(directions);
    setSizes(sizes);
}

/*===========================================================================*/
/**
 *  @brief  Destructs the KVSML point object class.
 */
/*===========================================================================*/
KVSMLObjectGlyph::~KVSMLObjectGlyph( void )
{
}
//
///*===========================================================================*/
///**
// *  @brief  Returns the KVSML tag.
// *  @return KVSML tag
// */
///*===========================================================================*/
//const vismodule::kvsml::KVSMLTag& KVSMLObjectGlyph::KVSMLTag( void ) const
//{
//    return( m_kvsml_tag );
//}
//
///*===========================================================================*/
///**
// *  @brief  Returns the object tag.
// *  @return object tag
// */
///*===========================================================================*/
//const vismodule::kvsml::ObjectTag& KVSMLObjectGlyph::objectTag( void ) const
//{
//    return( m_object_tag );
//}

/*===========================================================================*/
/**
 *  @brief  Returns the coordinate value array.
 */
/*===========================================================================*/
const vismodule::ValueArray<vismodule::Real32>& KVSMLObjectGlyph::coords( void ) const
{
    return( m_coords );
}

/*===========================================================================*/
/**
 *  @brief  Returns the color value array.
 */
/*===========================================================================*/
const vismodule::ValueArray<vismodule::UInt8>& KVSMLObjectGlyph::colors( void ) const
{
    return( m_colors );
}

/*===========================================================================*/
/**
 *  @brief  Returns the normal vector array.
 */
/*===========================================================================*/
const vismodule::ValueArray<vismodule::Real32>& KVSMLObjectGlyph::normals( void ) const
{
    return( m_normals );
}

/*===========================================================================*/
/**
 *  @brief  Returns the direction vector array.
 */
/*===========================================================================*/
const vismodule::ValueArray<vismodule::Real32>& KVSMLObjectGlyph::directions( void ) const
{
    return( m_directions );
}

/*===========================================================================*/
/**
 *  @brief  Returns the size value array.
 */
/*===========================================================================*/
const vismodule::ValueArray<vismodule::Real32>& KVSMLObjectGlyph::sizes( void ) const
{
    return( m_sizes );
}

/*===========================================================================*/
/**
 *  @brief  Returns the size value array.
 */
/*===========================================================================*/
const float KVSMLObjectGlyph::colorMin() const
{
    return(m_color_min);
}

const float KVSMLObjectGlyph::colorMax() const
{
    return(m_color_max);
}

const float KVSMLObjectGlyph::sizeMin() const
{
    return(m_size_min);
}

const float KVSMLObjectGlyph::sizeMax() const
{
    return(m_size_max);
}



/*===========================================================================*/
/**
 *  @brief  Set a writing type.
 *  @param  writing_type [in] writing type
 */
/*===========================================================================*/
//void KVSMLObjectGlyph::setWritingDataType( const WritingDataType writing_type )
//{
//    m_writing_type = writing_type;
//}

/*===========================================================================*/
/**
 *  @brief  Set a coordinate value array.
 *  @param  coords [in] coordinate value array
 */
/*===========================================================================*/
void KVSMLObjectGlyph::setCoords( const vismodule::ValueArray<vismodule::Real32>& coords )
{
    m_coords = coords;
}

/*===========================================================================*/
/**
 *  @brief  Set a color value array.
 *  @param  colors [in] color value array
 */
/*===========================================================================*/
void KVSMLObjectGlyph::setColors( const vismodule::ValueArray<vismodule::UInt8>& colors )
{
    m_colors = colors;
}

/*===========================================================================*/
/**
 *  @brief  Set a normal vector array.
 *  @param  normals [in] normal vector array
 */
/*===========================================================================*/
void KVSMLObjectGlyph::setNormals( const vismodule::ValueArray<vismodule::Real32>& normals )
{
    m_normals = normals;
}

/*===========================================================================*/
/**
 *  @brief  Set a normal vector array.
 *  @param  directions [in] normal vector array
 */
/*===========================================================================*/
void KVSMLObjectGlyph::setDirections( const vismodule::ValueArray<vismodule::Real32>& directions )
{
    m_directions = directions;
}

/*===========================================================================*/
/**
 *  @brief  Set a size value array.
 *  @param  sizes [in] size value array
 */
/*===========================================================================*/
void KVSMLObjectGlyph::setSizes( const vismodule::ValueArray<vismodule::Real32>& sizes )
{
    m_sizes = sizes;
}

/*===========================================================================*/
/**
 *  @brief  Set a size value array.
 *  @param  sizes [in] size value array
 */
/*===========================================================================*/
void KVSMLObjectGlyph::setColorMin( const float color_min )
{
    m_color_min = color_min;
}

void KVSMLObjectGlyph::setColorMax( const float color_max )
{
    m_color_max = color_max;
}

void KVSMLObjectGlyph::setSizeMin( const float size_min )
{
    m_size_min = size_min;
}

void KVSMLObjectGlyph::setSizeMax( const float size_max )
{
    m_size_max = size_max;
}

void KVSMLObjectGlyph::clear()
{
    m_sizes.deallocate();
    m_directions.deallocate();
    m_coords.deallocate();
    m_colors.deallocate();
}


/*===========================================================================*/
/**
 *  @brief  Read a KVSMl point object file.
 *  @param  filename [in] filename
 *  @return true, if the reading process is successfully
 */
/*===========================================================================*/
const bool KVSMLObjectGlyph::read( const std::string& filename )
{
    std::ifstream file(filename);
    m_filename = filename;
    std::string line_buffer;
    
    std::getline(file, line_buffer);
    std::cout << line_buffer << std::endl; 
    int nPoints = std::stoi( line_buffer ); 

    // coord
    std::getline(file, line_buffer);
    std::cout << line_buffer << std::endl; 
    std::vector<std::string> data;
    data = this->split( line_buffer, ' ' );
    m_coords.allocate(data.size());
    for (int i =0; i< nPoints*3; i ++ )
    {
        m_coords[i] = std::stof(data[i]);
    }

    //color
    std::getline(file, line_buffer);
    std::cout << line_buffer << std::endl; 
    data.clear();
    data = this->split( line_buffer, ' ' );
    m_colors.allocate(data.size());
    for (int i =0; i< nPoints*3; i ++ )
    {
        m_colors[i] = (vismodule::UInt8)std::stoi( data[i] );
    }

    //diresctions
    std::getline(file, line_buffer);
    std::cout << line_buffer << std::endl; 
    data.clear();
    data = this->split( line_buffer, ' ' );
    m_directions.allocate(data.size());
    for (int i =0; i< nPoints*3; i ++ )
    {
        m_directions[i] = std::stof( data[i] );
    }

    //sizes
    std::getline(file, line_buffer);
    std::cout << line_buffer << std::endl; 
    data.clear();
    data = this->split( line_buffer, ' ' );
    m_sizes.allocate(data.size());
    for (int i =0; i< nPoints; i ++ )
    {
        m_sizes[i] = std::stof( data[i] );
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
const bool KVSMLObjectGlyph::write( const std::string& filename )
{
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "ファイルを開けません: " << filename << std::endl;
        return false;
    }

    int nPoints = m_sizes.size()  ; 
    file << nPoints << std::endl;
    for (int i =0; i < m_coords.size(); i++ )
    {
        file <<  m_coords[i]  <<  " " ;
    }
    file << std::endl;
    for (int i =0; i < m_colors.size(); i++ )
    {
        file <<  (int)m_colors[i] <<  " " ;
    }
    file << std::endl;
    for (int i =0; i < m_directions.size(); i++ )
    {
        file <<  m_directions[i]  <<  " " ;
    }
    file << std::endl;
    for (int i =0; i < m_sizes.size(); i++ )
    {
        file <<  m_sizes[i]  <<  " " ;
    }
    file << std::endl;
    return true;
}

/*===========================================================================*/
/**
 *  @brief  '='operator.
 *  @param  other [in] point object
 */
/*===========================================================================*/
KVSMLObjectGlyph& KVSMLObjectGlyph::operator = ( const KVSMLObjectGlyph& other )
{
    if ( this != &other )
    {
        setCoords(other.coords());
        setColors(other.colors());
        setDirections(other.directions());
        setSizes(other.sizes());
    }

    return *this;
}


/*===========================================================================*/
/**
 *  @brief  Output operator.
 *  @param  os [out] output stream
 *  @param  rhs [in] KVSML point object
 */
/*===========================================================================*/
std::ostream& operator <<( std::ostream& os, const KVSMLObjectGlyph& rhs )
{
    os << "Num. of vertices: " << rhs.m_coords.size() / 3;

    return( os );
}

} // end of namespace vismodule
