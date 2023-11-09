/****************************************************************************/
/**
 *  @file VRDictionary.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: VRDictionary.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__DCM__VR_DICTIONARY_H_INCLUDE
#define KVS__DCM__VR_DICTIONARY_H_INCLUDE

#include <string>
#include <map>
#include "VR.h"
#include "VRTable.h"


namespace kvs
{

namespace dcm
{

/*===========================================================================*/
/**
 *  @brief  VR dictionary class.
 */
/*===========================================================================*/
class VRDictionary
{
public:

    typedef std::map<std::string,dcm::VR>           Container;
    typedef std::map<std::string,dcm::VR>::iterator ContainerIterator;

protected:

    Container m_container;

public:

    VRDictionary( void );

    virtual ~VRDictionary( void );

public:

    dcm::VR operator [] ( const std::string& key );

protected:

    void create( void );

    void clear( void );
};

inline VRDictionary::VRDictionary( void )
{
    this->create();
}

inline VRDictionary::~VRDictionary( void )
{
    this->clear();
}

inline dcm::VR VRDictionary::operator [] ( const std::string& key )
{
    // Find the VR type from the map.
    ContainerIterator i = m_container.find( key );

    return( i == m_container.end() ?
            dcm::VR( dcm::VR_NONE, dcm::ELEM_IMPLICIT, dcm::DATA_STRING ) :
            i->second );
}

inline void VRDictionary::create( void )
{
    for( int i = 0; i < dcm::VR_TABLE_SIZE; i++ )
    {
        std::string  key = dcm::VR_TABLE[i].vr_string;
        dcm::VR val = dcm::VR( dcm::VR_TABLE[i].vr_type,
                               dcm::VR_TABLE[i].elem_type,
                               dcm::VR_TABLE[i].data_type );

        m_container[key] = val;
    }
}

inline void VRDictionary::clear( void )
{
    m_container.clear();
}

} // end of namespace dcm

} // end of namespace kvs

#endif // KVS__DCM__VR_DICTIONARY_H_INCLUDE
