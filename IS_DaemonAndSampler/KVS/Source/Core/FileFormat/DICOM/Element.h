/****************************************************************************/
/**
 *  @file Element.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Element.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__DCM__ELEMENT_H_INCLUDE
#define KVS__DCM__ELEMENT_H_INCLUDE

#include <iostream>
#include <fstream>
#include <string>
#include "Tag.h"
#include "VR.h"
#include "Value.h"


namespace kvs
{

namespace dcm
{

/*===========================================================================*/
/**
 *  @brief  DICOM element class.
 */
/*===========================================================================*/
class Element
{
public:

    typedef dcm::Tag   Tag;
    typedef dcm::VR    VR;
    typedef dcm::Value Value;

protected:

    dcm::Tag   m_tag;   //!< tag
    dcm::VR    m_vr;    //!< value representation
    dcm::Value m_value; //!< data value

public:

    Element( void );

    Element( std::ifstream& ifs, const bool swap = false );

    virtual ~Element( void );

public:

    friend const bool operator == ( const Element& e, const dcm::Tag t );

    friend const bool operator == ( const dcm::Tag t, const Element& e );

    friend const bool operator == ( const Element& a, const Element& b );

    friend const bool operator != ( const Element& a, const Element& b );

    friend const bool operator < ( const Element& a, const Element& b );

    friend std::ostream& operator << ( std::ostream& os, const Element& e );

    friend std::ofstream& operator << ( std::ofstream& ofs, const Element& e );

public:

    const dcm::Tag tag( void ) const;

    const dcm::VR vr( void ) const;

    const dcm::Value& value( void ) const;

    const bool isKnown( void );

public:

    const bool read( std::ifstream& ifs, const bool swap );
};

} // end of namespace dcm

} // end of namespace kvs

#endif // KVS__DCM__ELEMENT_H_INCLUDE
