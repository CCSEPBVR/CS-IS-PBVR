/****************************************************************************/
/**
 *  @file Item.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Item.h 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__DCM__ITEM_H_INCLUDE
#define KVS__DCM__ITEM_H_INCLUDE

#include <iostream>
#include <fstream>
#include <string>


namespace kvs
{

namespace dcm
{

/*===========================================================================*/
/**
 *  @brief  Item class.
 */
/*===========================================================================*/
class Item
{
protected:

    unsigned int   m_data_length; ///< data length
    unsigned int   m_item_length; ///< item length
    unsigned short m_item_tag[2]; ///< item tag
    std::string    m_values;      ///< value string

public:

    Item( void );

    Item( const unsigned int data_length );

    virtual ~Item( void );

public:

    const unsigned int dataLength( void ) const;

    const std::string& values( void ) const;

public:

    const bool read( std::ifstream& ifs, const bool swap = false );

private:

    const bool read_item_tag( std::ifstream& ifs, const bool swap = false );

    const bool read_item_length( std::ifstream& ifs, const bool swap = false );

    const bool seek_item_delimitation( std::ifstream& ifs, const bool swap = false );

    const bool read_undefined_length_item( std::ifstream& ifs, const bool swap = false );
};

} // end of namespace dcm

} // end of namespace kvs

#endif // KVS__ITEM_H_INCLUDE
