/****************************************************************************/
/**
 *  @file Tag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Tag.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__DCM__TAG_H_INCLUDE
#define KVS__DCM__TAG_H_INCLUDE

#include "VRType.h"


namespace kvs
{

namespace dcm
{

/*===========================================================================*/
/**
 *  @brief  Dicom tag class.
 */
/*===========================================================================*/
class Tag
{
protected:

    unsigned short m_group_id;   ///< group ID
    unsigned short m_element_id; ///< element ID
    dcm::VRType    m_vr_type;    ///< VR type (initial value)
    std::string    m_name;       ///< element name

public:

    Tag( void );

    Tag( const unsigned short group_id,
         const unsigned short element_id,
         const dcm::VRType    vr_type     = dcm::VR_NONE,
         const std::string    name        = "" );

    Tag( const Tag& tag );

    Tag( std::ifstream& ifs, const bool swap = false );

    virtual ~Tag( void );

public:

    Tag& operator = ( const Tag& t );

    friend bool operator == ( const Tag& a, const Tag& b );

    friend bool operator != ( const Tag& a, const Tag& b );

    friend bool operator < ( const Tag& a, const Tag& b );

    friend std::ostream& operator << ( std::ostream& os, const Tag& t );

    friend std::ofstream& operator << ( std::ofstream& ofs, const Tag& t );

public:

    const unsigned short groupID( void ) const;

    const unsigned short elementID( void ) const;

    const dcm::VRType vrType( void ) const;

    const std::string& name( void ) const;

public:

    const bool read( std::ifstream& ifs, bool swap );
};

} // end of namespace dcm

} // end of namespace kvs

#endif // KVS__DCM_TAG_H_INCLUDE
