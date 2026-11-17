/****************************************************************************/
/**
 *  @file AnyValueArray.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: AnyValueArray.cpp 837 2011-06-02 09:32:56Z naohisa.sakamoto@gmail.com $
 */
/****************************************************************************/
#include "AnyValueArray.h"


namespace vismodule
{

AnyValueArray::AnyValueArray( void )
    : m_counter( 0 )
    , m_type_info( 0 )
    , m_size_of_value( 0 )
    , m_nvalues( 0 )
    , m_values( 0 )
{
    this->create_counter();
}

AnyValueArray::AnyValueArray( const AnyValueArray& other )
    : m_counter( 0 )
    , m_type_info( 0 )
    , m_size_of_value( 0 )
    , m_nvalues( 0 )
    , m_values( 0 )
{
    this->shallowCopy( other );
}

AnyValueArray::~AnyValueArray( void )
{
    this->deallocate();
}


template<> AnyValueArray::AnyValueArray( const vismodule::Int8* values, const std::size_t nvalues );
template<> AnyValueArray::AnyValueArray( const vismodule::UInt8* values, const std::size_t nvalues );
template<> AnyValueArray::AnyValueArray( const vismodule::Int16* values, const std::size_t nvalues );
template<> AnyValueArray::AnyValueArray( const vismodule::UInt16* values, const std::size_t nvalues );
template<> AnyValueArray::AnyValueArray( const vismodule::Int32* values, const std::size_t nvalues );
template<> AnyValueArray::AnyValueArray( const vismodule::UInt32* values, const std::size_t nvalues );
template<> AnyValueArray::AnyValueArray( const vismodule::Int64* values, const std::size_t nvalues );
template<> AnyValueArray::AnyValueArray( const vismodule::UInt64* values, const std::size_t nvalues );
template<> AnyValueArray::AnyValueArray( const vismodule::Real32* values, const std::size_t nvalues );
template<> AnyValueArray::AnyValueArray( const vismodule::Real64* values, const std::size_t nvalues );
template<> AnyValueArray::AnyValueArray( const std::string* values, const std::size_t nvalues );

template<> AnyValueArray::AnyValueArray( const std::vector<vismodule::Int8>& values );
template<> AnyValueArray::AnyValueArray( const std::vector<vismodule::UInt8>& values );
template<> AnyValueArray::AnyValueArray( const std::vector<vismodule::Int16>& values );
template<> AnyValueArray::AnyValueArray( const std::vector<vismodule::UInt16>& values );
template<> AnyValueArray::AnyValueArray( const std::vector<vismodule::Int32>& values );
template<> AnyValueArray::AnyValueArray( const std::vector<vismodule::UInt32>& values );
template<> AnyValueArray::AnyValueArray( const std::vector<vismodule::Int64>& values );
template<> AnyValueArray::AnyValueArray( const std::vector<vismodule::UInt64>& values );
template<> AnyValueArray::AnyValueArray( const std::vector<vismodule::Real32>& values );
template<> AnyValueArray::AnyValueArray( const std::vector<vismodule::Real64>& values );
template<> AnyValueArray::AnyValueArray( const std::vector<std::string>& values );

template<> AnyValueArray::AnyValueArray( const vismodule::ValueArray<vismodule::Int8>& values );
template<> AnyValueArray::AnyValueArray( const vismodule::ValueArray<vismodule::UInt8>& values );
template<> AnyValueArray::AnyValueArray( const vismodule::ValueArray<vismodule::Int16>& values );
template<> AnyValueArray::AnyValueArray( const vismodule::ValueArray<vismodule::UInt16>& values );
template<> AnyValueArray::AnyValueArray( const vismodule::ValueArray<vismodule::Int32>& values );
template<> AnyValueArray::AnyValueArray( const vismodule::ValueArray<vismodule::UInt32>& values );
template<> AnyValueArray::AnyValueArray( const vismodule::ValueArray<vismodule::Int64>& values );
template<> AnyValueArray::AnyValueArray( const vismodule::ValueArray<vismodule::UInt64>& values );
template<> AnyValueArray::AnyValueArray( const vismodule::ValueArray<vismodule::Real32>& values );
template<> AnyValueArray::AnyValueArray( const vismodule::ValueArray<vismodule::Real64>& values );
template<> AnyValueArray::AnyValueArray( const vismodule::ValueArray<std::string>& values );

template<> vismodule::Int8&   AnyValueArray::at<vismodule::Int8>( const std::size_t index );
template<> vismodule::UInt8&  AnyValueArray::at<vismodule::UInt8>( const std::size_t index );
template<> vismodule::Int16&  AnyValueArray::at<vismodule::Int16>( const std::size_t index );
template<> vismodule::UInt16& AnyValueArray::at<vismodule::UInt16>( const std::size_t index );
template<> vismodule::Int32&  AnyValueArray::at<vismodule::Int32>( const std::size_t index );
template<> vismodule::UInt32& AnyValueArray::at<vismodule::UInt32>( const std::size_t index );
template<> vismodule::Int64&  AnyValueArray::at<vismodule::Int64>( const std::size_t index );
template<> vismodule::UInt64& AnyValueArray::at<vismodule::UInt64>( const std::size_t index );
template<> vismodule::Real32& AnyValueArray::at<vismodule::Real32>( const std::size_t index );
template<> vismodule::Real64& AnyValueArray::at<vismodule::Real64>( const std::size_t index );
template<> std::string& AnyValueArray::at<std::string>( const std::size_t index );

template<> const vismodule::Int8&   AnyValueArray::at<vismodule::Int8>( const std::size_t index ) const;
template<> const vismodule::UInt8&  AnyValueArray::at<vismodule::UInt8>( const std::size_t index ) const;
template<> const vismodule::Int16&  AnyValueArray::at<vismodule::Int16>( const std::size_t index ) const ;
template<> const vismodule::UInt16& AnyValueArray::at<vismodule::UInt16>( const std::size_t index ) const;
template<> const vismodule::Int32&  AnyValueArray::at<vismodule::Int32>( const std::size_t index ) const;
template<> const vismodule::UInt32& AnyValueArray::at<vismodule::UInt32>( const std::size_t index ) const;
template<> const vismodule::Int64&  AnyValueArray::at<vismodule::Int64>( const std::size_t index ) const;
template<> const vismodule::UInt64& AnyValueArray::at<vismodule::UInt64>( const std::size_t index ) const;
template<> const vismodule::Real32& AnyValueArray::at<vismodule::Real32>( const std::size_t index ) const;
template<> const vismodule::Real64& AnyValueArray::at<vismodule::Real64>( const std::size_t index ) const;
template<> const std::string& AnyValueArray::at<std::string>( const std::size_t index ) const;

template<> const vismodule::Int8   AnyValueArray::to<vismodule::Int8>( const std::size_t index ) const;
template<> const vismodule::UInt8  AnyValueArray::to<vismodule::UInt8>( const std::size_t index ) const;
template<> const vismodule::Int16  AnyValueArray::to<vismodule::Int16>( const std::size_t index ) const;
template<> const vismodule::UInt16 AnyValueArray::to<vismodule::UInt16>( const std::size_t index ) const;
template<> const vismodule::Int32  AnyValueArray::to<vismodule::Int32>( const std::size_t index ) const;
template<> const vismodule::UInt32 AnyValueArray::to<vismodule::UInt32>( const std::size_t index ) const;
template<> const vismodule::Int64  AnyValueArray::to<vismodule::Int64>( const std::size_t index ) const;
template<> const vismodule::UInt64 AnyValueArray::to<vismodule::UInt64>( const std::size_t index ) const;
template<> const vismodule::Real32 AnyValueArray::to<vismodule::Real32>( const std::size_t index ) const;
template<> const vismodule::Real64 AnyValueArray::to<vismodule::Real64>( const std::size_t index ) const;
template<> const std::string AnyValueArray::to<std::string>( const std::size_t index ) const;

template<> const vismodule::Int8*   AnyValueArray::pointer<vismodule::Int8>( void ) const;
template<> const vismodule::UInt8*  AnyValueArray::pointer<vismodule::UInt8>( void ) const;
template<> const vismodule::Int16*  AnyValueArray::pointer<vismodule::Int16>( void ) const;
template<> const vismodule::UInt16* AnyValueArray::pointer<vismodule::UInt16>( void ) const;
template<> const vismodule::Int32*  AnyValueArray::pointer<vismodule::Int32>( void ) const;
template<> const vismodule::UInt32* AnyValueArray::pointer<vismodule::UInt32>( void ) const;
template<> const vismodule::Int64*  AnyValueArray::pointer<vismodule::Int64>( void ) const;
template<> const vismodule::UInt64* AnyValueArray::pointer<vismodule::UInt64>( void ) const;
template<> const vismodule::Real32* AnyValueArray::pointer<vismodule::Real32>( void ) const;
template<> const vismodule::Real64* AnyValueArray::pointer<vismodule::Real64>( void ) const;
template<> const std::string* AnyValueArray::pointer<std::string>( void ) const;

template<> vismodule::Int8*   AnyValueArray::pointer<vismodule::Int8>( void );
template<> vismodule::UInt8*  AnyValueArray::pointer<vismodule::UInt8>( void );
template<> vismodule::Int16*  AnyValueArray::pointer<vismodule::Int16>( void );
template<> vismodule::UInt16* AnyValueArray::pointer<vismodule::UInt16>( void );
template<> vismodule::Int32*  AnyValueArray::pointer<vismodule::Int32>( void );
template<> vismodule::UInt32* AnyValueArray::pointer<vismodule::UInt32>( void );
template<> vismodule::Int64*  AnyValueArray::pointer<vismodule::Int64>( void );
template<> vismodule::UInt64* AnyValueArray::pointer<vismodule::UInt64>( void );
template<> vismodule::Real32* AnyValueArray::pointer<vismodule::Real32>( void );
template<> vismodule::Real64* AnyValueArray::pointer<vismodule::Real64>( void );
template<> std::string* AnyValueArray::pointer<std::string>( void );

template<> void* AnyValueArray::allocate<vismodule::Int8>( const std::size_t nvalues );
template<> void* AnyValueArray::allocate<vismodule::UInt8>( const std::size_t nvalues );
template<> void* AnyValueArray::allocate<vismodule::Int16>( const std::size_t nvalues );
template<> void* AnyValueArray::allocate<vismodule::UInt16>( const std::size_t nvalues );
template<> void* AnyValueArray::allocate<vismodule::Int32>( const std::size_t nvalues );
template<> void* AnyValueArray::allocate<vismodule::UInt32>( const std::size_t nvalues );
template<> void* AnyValueArray::allocate<vismodule::Int64>( const std::size_t nvalues );
template<> void* AnyValueArray::allocate<vismodule::UInt64>( const std::size_t nvalues );
template<> void* AnyValueArray::allocate<vismodule::Real32>( const std::size_t nvalues );
template<> void* AnyValueArray::allocate<vismodule::Real64>( const std::size_t nvalues );
template<> void* AnyValueArray::allocate<std::string>( const std::size_t nvalues );

template<> void AnyValueArray::shallowCopy<vismodule::Int8>( const vismodule::ValueArray<vismodule::Int8>& values );
template<> void AnyValueArray::shallowCopy<vismodule::UInt8>( const vismodule::ValueArray<vismodule::UInt8>& values );
template<> void AnyValueArray::shallowCopy<vismodule::Int16>( const vismodule::ValueArray<vismodule::Int16>& values );
template<> void AnyValueArray::shallowCopy<vismodule::UInt16>( const vismodule::ValueArray<vismodule::UInt16>& values );
template<> void AnyValueArray::shallowCopy<vismodule::Int32>( const vismodule::ValueArray<vismodule::Int32>& values );
template<> void AnyValueArray::shallowCopy<vismodule::UInt32>( const vismodule::ValueArray<vismodule::UInt32>& values );
template<> void AnyValueArray::shallowCopy<vismodule::Int64>( const vismodule::ValueArray<vismodule::Int64>& values );
template<> void AnyValueArray::shallowCopy<vismodule::UInt64>( const vismodule::ValueArray<vismodule::UInt64>& values );
template<> void AnyValueArray::shallowCopy<vismodule::Real32>( const vismodule::ValueArray<vismodule::Real32>& values );
template<> void AnyValueArray::shallowCopy<vismodule::Real64>( const vismodule::ValueArray<vismodule::Real64>& values );
template<> void AnyValueArray::shallowCopy<std::string>( const vismodule::ValueArray<std::string>& values );

template<> void AnyValueArray::deepCopy<vismodule::Int8>( const vismodule::Int8* values, const std::size_t nvalues );
template<> void AnyValueArray::deepCopy<vismodule::UInt8>( const vismodule::UInt8* values, const std::size_t nvalues );
template<> void AnyValueArray::deepCopy<vismodule::Int16>( const vismodule::Int16* values, const std::size_t nvalues );
template<> void AnyValueArray::deepCopy<vismodule::UInt16>( const vismodule::UInt16* values, const std::size_t nvalues );
template<> void AnyValueArray::deepCopy<vismodule::Int32>( const vismodule::Int32* values, const std::size_t nvalues );
template<> void AnyValueArray::deepCopy<vismodule::UInt32>( const vismodule::UInt32* values, const std::size_t nvalues );
template<> void AnyValueArray::deepCopy<vismodule::Int64>( const vismodule::Int64* values, const std::size_t nvalues );
template<> void AnyValueArray::deepCopy<vismodule::UInt64>( const vismodule::UInt64* values, const std::size_t nvalues );
template<> void AnyValueArray::deepCopy<vismodule::Real32>( const vismodule::Real32* values, const std::size_t nvalues );
template<> void AnyValueArray::deepCopy<vismodule::Real64>( const vismodule::Real64* values, const std::size_t nvalues );
template<> void AnyValueArray::deepCopy<std::string>( const std::string* values, const std::size_t nvalues );

} // end of namespace vismodule
