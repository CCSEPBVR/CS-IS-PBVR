/*****************************************************************************/
/**
 *  @file   DataValueTag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DataValueTag.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__KVSML__DATA_VALUE_H_INCLUDE
#define KVS__KVSML__DATA_VALUE_H_INCLUDE

#include <string>
#include <kvs/ValueArray>
#include <kvs/Tokenizer>
#include <kvs/XMLNode>
#include <kvs/XMLElement>
#include <kvs/XMLDocument>
#include "DataArray.h"
#include "TagBase.h"


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <DataValue>.
 */
/*===========================================================================*/
class DataValueTag : public kvs::kvsml::TagBase
{
public:

    typedef kvs::kvsml::TagBase BaseClass;

public:

    DataValueTag( void );

    virtual ~DataValueTag( void );

public:

    template <typename T>
    const bool read( const kvs::XMLNode::SuperClass* parent, const size_t nelements, kvs::ValueArray<T>* data );

    template <typename T>
    const bool write( kvs::XMLNode::SuperClass* parent, const kvs::ValueArray<T>& data );

private:

    const bool read( const kvs::XMLNode::SuperClass* parent );

    const bool write( kvs::XMLNode::SuperClass* parent );
};

template <typename T>
inline const bool DataValueTag::read(
    const kvs::XMLNode::SuperClass* parent,
    const size_t nelements,
    kvs::ValueArray<T>* data )
{
    const std::string tag_name = BaseClass::name();

    BaseClass::m_node = kvs::XMLNode::FindChildNode( parent, tag_name );
    if ( !BaseClass::m_node )
    {
        kvsMessageError( "Cannot find <%s>.", tag_name.c_str() );
        return( false );
    }

    const TiXmlText* array_text = kvs::XMLNode::ToText( m_node );
    if ( !array_text )
    {
        kvsMessageError( "No value in <%s>.", tag_name.c_str() );
        return( false );
    }

    const std::string delim(" \n");
    kvs::Tokenizer tokenizer( array_text->Value(), delim );
    if ( !kvs::kvsml::DataArray::ReadInternalData<T>( data, nelements, tokenizer ) )
    {
        kvsMessageError( "Cannot read the data in <%s>.", tag_name.c_str() );
        return( false );
    }

    return( true );
}

template <typename T>
inline const bool DataValueTag::write(
    kvs::XMLNode::SuperClass* parent,
    const kvs::ValueArray<T>& data )
{
    if ( data.size() == 0 ) return( true );

    const std::string tag_name = BaseClass::name();
    kvs::XMLElement element( tag_name );

    std::ostringstream oss( std::ostringstream::out );
    const size_t data_size = data.size();
    if ( typeid(T) == typeid(kvs::Int8) || typeid(T) == typeid(kvs::UInt8) )
    {
        for ( size_t i = 0; i < data_size; i++ ) oss << int(data.at(i)) << " ";
    }
    else
    {
        for ( size_t i = 0; i < data_size; i++ ) oss << data.at(i) << " ";
    }

    // Insert the data array as string-stream to the parent node.
    TiXmlText text;
    text.SetValue( oss.str() );

    kvs::XMLNode::SuperClass* node = parent->InsertEndChild( element );
    return( node->InsertEndChild( text ) != NULL );
}

} // end of namespace kvsml

} // end of namespace kvs

#endif // KVS__KVSML__DATA_VALUE_H_INCLUDE
