/*****************************************************************************/
/**
 *  @file   Data.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Data.h 874 2011-07-28 03:06:54Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__GF__DATA_H_INCLUDE
#define KVS__GF__DATA_H_INCLUDE

#include <kvs/Type>
#include <kvs/ValueArray>
#include <string>
#include <iostream>


namespace kvs
{

namespace gf
{

/*===========================================================================*/
/**
 *  @brief  GF Data class.
 */
/*===========================================================================*/
class Data
{
protected:

    std::string m_array_type_header; ///< array type header
    std::string m_keyword; ///< keyword
    std::string m_comment; ///< comment (data name)
    kvs::Int32 m_num; ///< num (vector length)
    kvs::Int32 m_num2; ///< num2 (number of elements)
    kvs::ValueArray<kvs::Real32> m_flt_array; ///< data array (float type)
    kvs::ValueArray<kvs::Int32> m_int_array; ///< data array (int type)

public:

    Data( void );

public:

    friend std::ostream& operator << ( std::ostream& os, const Data& d );

public:

    const std::string& arrayTypeHeader( void ) const;

    const std::string& keyword( void ) const;

    const std::string& comment( void ) const;

    const kvs::Int32 num( void ) const;

    const kvs::Int32 num2( void ) const;

    const kvs::ValueArray<kvs::Real32>& fltArray( void ) const;

    const kvs::ValueArray<kvs::Int32>& intArray( void ) const;

    void deallocate( void );

public:

    const bool readAscii( FILE* fp, const std::string tag );

    const bool readBinary( FILE* fp, const bool swap = false );
};

} // end of namespace gf

} // end of namespace kvs

#endif // KVS__GF__DATA_H_INCLUDE
