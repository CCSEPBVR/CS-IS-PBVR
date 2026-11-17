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
#ifndef VIS_MODULE__KVSML__DATA_VALUE_H_INCLUDE
#define VIS_MODULE__KVSML__DATA_VALUE_H_INCLUDE

#include <string>
#include <vismodule/ValueArray>
#include <vismodule/Tokenizer>
#include <vismodule/XMLNode>
#include <vismodule/XMLElement>
#include <vismodule/XMLDocument>
#include "DataArray.h"
#include "TagBase.h"


namespace vismodule
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <DataValue>.
 */
/*===========================================================================*/
class DataValueTag : public vismodule::kvsml::TagBase
{
public:

    typedef vismodule::kvsml::TagBase BaseClass;

public:

    DataValueTag( void );

    virtual ~DataValueTag( void );

public:

    template <typename T>
    const bool read( const vismodule::XMLNode::SuperClass* parent, const std::size_t nelements, vismodule::ValueArray<T>* data );

    template <typename T>
    const bool write( vismodule::XMLNode::SuperClass* parent, const vismodule::ValueArray<T>& data );

private:

    const bool read( const vismodule::XMLNode::SuperClass* parent );

    const bool write( vismodule::XMLNode::SuperClass* parent );
};

template <typename T>
inline const bool DataValueTag::read(
    const vismodule::XMLNode::SuperClass* parent,
    const std::size_t nelements,
    vismodule::ValueArray<T>* data )
{
    const std::string tag_name = BaseClass::name();

    BaseClass::m_node = vismodule::XMLNode::FindChildNode( parent, tag_name );
    if ( !BaseClass::m_node )
    {
        visModuleMessageError( "Cannot find <%s>.", tag_name.c_str() );
        return( false );
    }

    const TiXmlText* array_text = vismodule::XMLNode::ToText( m_node );
    if ( !array_text )
    {
        visModuleMessageError( "No value in <%s>.", tag_name.c_str() );
        return( false );
    }

    const std::string delim(" \n");
    vismodule::Tokenizer tokenizer( array_text->Value(), delim );
    if ( !vismodule::kvsml::DataArray::ReadInternalData<T>( data, nelements, tokenizer ) )
    {
        visModuleMessageError( "Cannot read the data in <%s>.", tag_name.c_str() );
        return( false );
    }

    return( true );
}

template <typename T>
inline const bool DataValueTag::write(
    vismodule::XMLNode::SuperClass* parent,
    const vismodule::ValueArray<T>& data )
{
    if ( data.size() == 0 ) return( true );

    const std::string tag_name = BaseClass::name();
    vismodule::XMLElement element( tag_name );

    std::ostringstream oss( std::ostringstream::out );
    const std::size_t data_size = data.size();
    if ( typeid(T) == typeid(vismodule::Int8) || typeid(T) == typeid(vismodule::UInt8) )
    {
        for ( std::size_t i = 0; i < data_size; i++ ) oss << int(data.at(i)) << " ";
    }
    else
    {
        for ( std::size_t i = 0; i < data_size; i++ ) oss << data.at(i) << " ";
    }

    // Insert the data array as string-stream to the parent node.
    TiXmlText text;
    text.SetValue( oss.str() );

    vismodule::XMLNode::SuperClass* node = parent->InsertEndChild( element );
    return( node->InsertEndChild( text ) != NULL );
}

} // end of namespace kvsml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__DATA_VALUE_H_INCLUDE
