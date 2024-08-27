/****************************************************************************/
/**
 *  @file Element.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Element.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "Element.h"
#include "VR.h"
#include "Value.h"


namespace
{
kvs::dcm::Tag END_HEADER_TAG = kvs::dcm::Tag( 0x7FE0, 0x0010, kvs::dcm::VR_OW );
} // end of namesapce

namespace kvs
{

namespace dcm
{

/*===========================================================================*/
/**
 *  @brief  Constructor.
 */
/*===========================================================================*/
Element::Element( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructor.
 *  @param  ifs  [in] input file stream
 *  @param  swap [in] byte swap
 */
/*===========================================================================*/
Element::Element( std::ifstream& ifs, const bool swap )
{
    this->read( ifs, swap );
}

/*===========================================================================*/
/**
 *  @brief  Destructor.
 */
/*===========================================================================*/
Element::~Element( void )
{
}

const bool operator == ( const Element& e, const dcm::Tag t )
{
    return( e.m_tag == t );
}

const bool operator == ( const dcm::Tag t, const Element& e )
{
    return( t == e.m_tag );
}

const bool operator == ( const Element& a, const Element& b )
{
    return( a.m_tag == b.m_tag );
}

const bool operator != ( const Element& a, const Element& b )
{
    return( a.m_tag != b.m_tag );
}

const bool operator < ( const Element& a, const Element& b )
{
    return( a.m_tag < b.m_tag );
}

std::ostream& operator << ( std::ostream& os, const Element& e )
{
    os << e.m_tag  << std::endl;
    os << e.m_vr   << std::endl;
    os << "Value:          " << e.m_value;
    return( os );
}

std::ofstream& operator << ( std::ofstream& ofs, const Element& e )
{
    ofs << e.m_tag  << std::endl;
    ofs << e.m_vr   << std::endl;
    ofs << "Value:          " << e.m_value;
    return( ofs );
}

/*===========================================================================*/
/**
 *  @brief  Return the tag.
 *  @return tag
 */
/*===========================================================================*/
const dcm::Tag Element::tag( void ) const
{
    return( m_tag );
}

/*===========================================================================*/
/**
 *  @brief  Return the VR.
 *  @return VR
 */
/*===========================================================================*/
const dcm::VR Element::vr( void ) const
{
    return( m_vr );
}

/*===========================================================================*/
/**
 *  @brief  Return the value.
 *  @return value
 */
/*===========================================================================*/
const dcm::Value& Element::value( void ) const
{
    return( m_value );
}

/*===========================================================================*/
/**
 *  @brief  Check whether the element is known, or not.
 *  @return true, if the element is known
 */
/*===========================================================================*/
const bool Element::isKnown( void )
{
    return( !m_tag.name().empty() );
}

/*===========================================================================*/
/**
 *  @brief  Read the DICOM element.
 *  @param  ifs  [in] input file stream
 *  @param  swap [in] swap flag
 *  @return true, if the reading process is done succesfully
 */
/*===========================================================================*/
const bool Element::read( std::ifstream& ifs, const bool swap )
{
    // Read the tag.
    if( !m_tag.read( ifs, swap ) ) return( false );

    // Read the VR.
    m_vr = dcm::VR( m_tag.vrType() );
    if( !m_vr.read( ifs, swap ) ) return( false );

    // Stop reading the header infomation, if you read the "pixel-data" tag.
    if( m_tag == ::END_HEADER_TAG ) return( true );

    // Read the value.
    m_value = dcm::Value( m_vr.dataType(), m_vr.valueLength() );

    if( !m_value.read( ifs, swap ) ) return( false );

    return( true );
}

} // end of namespace dcm

} // end of namespace kvs
