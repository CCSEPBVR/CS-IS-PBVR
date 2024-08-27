/*****************************************************************************/
/**
 *  @file   DataArrayTag.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DataArrayTag.cpp 667 2011-02-22 16:07:54Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "DataArrayTag.h"
#include "DataArray.h"
#include <kvs/XMLNode>
#include <kvs/XMLElement>
#include <kvs/XMLDocument>
#include <kvs/File>
#include <kvs/ValueArray>
#include <kvs/AnyValueArray>
#include <kvs/IgnoreUnusedVariable>
#include <iostream>
#include <fstream>
#include <sstream>


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new data array tag class.
 */
/*===========================================================================*/
DataArrayTag::DataArrayTag( void ):
    kvs::kvsml::TagBase( "DataArray" ),
    m_has_type( false ),
    m_type( "" ),
    m_has_file( false ),
    m_file( "" ),
    m_has_format( false ),
    m_format( "" )
{
}

/*===========================================================================*/
/**
 *  @brief  Destructs the data array tag class.
 */
/*===========================================================================*/
DataArrayTag::~DataArrayTag( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the tag has 'type' or not.
 *  @return true, if the tag has 'type'
 */
/*===========================================================================*/
const bool DataArrayTag::hasType( void ) const
{
    return( m_has_type );
}

/*===========================================================================*/
/**
 *  @brief  Returns a type that is specfied by 'type'.
 *  @return type
 */
/*===========================================================================*/
const std::string& DataArrayTag::type( void ) const
{
    return( m_type );
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the tag has 'file' or not.
 *  @return true, if the tag has 'file'
 */
/*===========================================================================*/
const bool DataArrayTag::hasFile( void ) const
{
    return( m_has_file );
}

/*===========================================================================*/
/**
 *  @brief  Returns a external file name that is specfied by 'file'.
 *  @return external file name
 */
/*===========================================================================*/
const std::string& DataArrayTag::file( void ) const
{
    return( m_file );
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the tag has 'format' or not.
 *  @return true, if the tag has 'format'
 */
/*===========================================================================*/
const bool DataArrayTag::hasFormat( void ) const
{
    return( m_has_format );
}

/*===========================================================================*/
/**
 *  @brief  Returns a external file format that is specfied by 'format'.
 *  @return external file format
 */
/*===========================================================================*/
const std::string& DataArrayTag::format( void ) const
{
    return( m_format );
}

/*===========================================================================*/
/**
 *  @brief  Sets a external file name.
 *  @param  file [in] external file name
 */
/*===========================================================================*/
void DataArrayTag::setFile( const std::string& file )
{
    m_has_file = true;
    m_file = file;
}

/*===========================================================================*/
/**
 *  @brief  Sets a external file format.
 *  @param  format [in] external file format
 */
/*===========================================================================*/
void DataArrayTag::setFormat( const std::string& format )
{
    m_has_format = true;
    m_format = format;
}

/*===========================================================================*/
/**
 *  @brief  Reads a data array tag.
 *  @param  parent [in] pointer to the parent node
 *  @param  nelements [in] number of elements of the data array
 *  @param  data [out] data array
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool DataArrayTag::read(
    const kvs::XMLNode::SuperClass* parent,
    const size_t nelements,
    kvs::AnyValueArray* data )
{
    const std::string tag_name = BaseClass::name();

    BaseClass::m_node = kvs::XMLNode::FindChildNode( parent, tag_name );
    if( !BaseClass::m_node )
    {
        kvsMessageError( "Cannot find <%s>.", tag_name.c_str() );
        return( false );
    }

    this->read_attribute();

    return( this->read_data( nelements, data ) );
}

/*===========================================================================*/
/**
 *  @brief  Writes the data array.
 *  @param  parent [in] pointer to the paranet node for writing
 *  @param  data [in] data array
 *  @param  pathname [in] pathname
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool DataArrayTag::write(
    kvs::XMLNode::SuperClass* parent,
    const kvs::AnyValueArray& data,
    const std::string pathname )
{
    if ( data.size() == 0 ) return( true );

    const std::string tag_name = BaseClass::name();
    kvs::XMLElement element( tag_name );
    element.setAttribute( "type", kvs::kvsml::DataArray::GetDataType( data ) );

    // Internal data: <DataArray type="xxx">xxx</DataArray>
    if ( !m_has_file )
    {
        // Write the data array to string-stream.
        std::ostringstream oss( std::ostringstream::out );
        const std::type_info& data_type = data.typeInfo()->type();
        const size_t data_size = data.size();
        if ( data_type == typeid(kvs::Int8) )
        {
            const kvs::Int8* values = data.pointer<kvs::Int8>();
            for ( size_t i = 0; i < data_size; i++ ) oss << kvs::Int16(values[i]) << " ";
        }
        else if ( data_type == typeid(kvs::UInt8) )
        {
            const kvs::UInt8* values = data.pointer<kvs::UInt8>();
            for ( size_t i = 0; i < data_size; i++ ) oss << kvs::UInt16(values[i]) << " ";
        }
        else if ( data_type == typeid(kvs::Int16) )
        {
            const kvs::Int16* values = data.pointer<kvs::Int16>();
            for ( size_t i = 0; i < data_size; i++ ) oss << values[i] << " ";
        }
        else if ( data_type == typeid(kvs::UInt16) )
        {
            const kvs::UInt16* values = data.pointer<kvs::UInt16>();
            for ( size_t i = 0; i < data_size; i++ ) oss << values[i] << " ";
        }
        else if ( data_type == typeid(kvs::Int32) )
        {
            const kvs::Int32* values = data.pointer<kvs::Int32>();
            for ( size_t i = 0; i < data_size; i++ ) oss << values[i] << " ";
        }
        else if ( data_type == typeid(kvs::UInt32) )
        {
            const kvs::UInt32* values = data.pointer<kvs::UInt32>();
            for ( size_t i = 0; i < data_size; i++ ) oss << values[i] << " ";
        }
        else if ( data_type == typeid(kvs::Real32) )
        {
            const kvs::Real32* values = data.pointer<kvs::Real32>();
            for ( size_t i = 0; i < data_size; i++ ) oss << values[i] << " ";
        }
        else if ( data_type == typeid(kvs::Real64) )
        {
            const kvs::Real64* values = data.pointer<kvs::Real64>();
            for ( size_t i = 0; i < data_size; i++ ) oss << values[i] << " ";
        }
        else if ( data_type == typeid(kvs::Real128) )
        {
            const kvs::Real128* values = data.pointer<kvs::Real128>();
            for ( size_t i = 0; i < data_size; i++ ) oss << values[i] << " ";
        }

        // Insert the data array as string-stream to the parent node.
        TiXmlText text;
        text.SetValue( oss.str() );

        kvs::XMLNode::SuperClass* node = parent->InsertEndChild( element );
        if( !node )
        {
            kvsMessageError( "Cannot insert <%s>.", tag_name.c_str() );
            return( false );
        }

        return( node->InsertEndChild( text ) != NULL );
    }

    // External data: <DataArray type="xxx" format="xxx" file="xxx"/>
    else
    {
        if ( !m_has_format )
        {
            kvsMessageError( "'format' is not spcified in <%s>.", tag_name.c_str() );
            return( false );
        }

        element.setAttribute( "file", m_file );
        element.setAttribute( "format", m_format );
        parent->InsertEndChild( element );

        // Write the data to the external data file.
        const std::string filename = pathname + kvs::File::Separator() + m_file;
        return( kvs::kvsml::DataArray::WriteExternalData( data, m_file, m_format ) );
    }
}

/*===========================================================================*/
/**
 *  @brief  Reads attributes.
 */
/*===========================================================================*/
const void DataArrayTag::read_attribute( void )
{
    // Element
    const kvs::XMLElement::SuperClass* element = kvs::XMLNode::ToElement( m_node );

    // type="xxx"
    const std::string type = kvs::XMLElement::AttributeValue( element, "type" );
    if( type != "" )
    {
        m_has_type = true;
        m_type = type;
    }

    // file="xxx"
    const std::string file = kvs::XMLElement::AttributeValue( element, "file" );
    if( file != "" )
    {
        m_has_file = true;
        m_file = file;
    }

    // format="xxx"
    const std::string format = kvs::XMLElement::AttributeValue( element, "format" );
    if( format != "" )
    {
        m_has_format = true;
        m_format = format;
    }
}

/*===========================================================================*/
/**
 *  @brief  Reads a data array.
 *  @param  nelements [in] number of elements
 *  @param  data [out] pointer to the data array
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool DataArrayTag::read_data( const size_t nelements, kvs::AnyValueArray* data )
{
    const std::string tag_name = BaseClass::name();

    // Internal data.
    if ( m_file == "" )
    {
        const TiXmlText* array_text = kvs::XMLNode::ToText( m_node );
        if ( !array_text )
        {
            kvsMessageError( "No value in <%s>.", tag_name.c_str() );
            return( false );
        }

        // <DataArray type="xxx">xxx</DataArray>
        const std::string delim(" ,\t\n");
        kvs::Tokenizer t( array_text->Value(), delim );

        if( m_type == "char" )
        {
            if ( !kvs::kvsml::DataArray::ReadInternalData<kvs::Int8>( data, nelements, t ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return( false );
            }
        }
        else if( m_type == "unsigned char" || m_type == "uchar" )
        {
            if ( !kvs::kvsml::DataArray::ReadInternalData<kvs::UInt8>( data, nelements, t ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return( false );
            }
        }
        else if ( m_type == "short" )
        {
            if ( !kvs::kvsml::DataArray::ReadInternalData<kvs::Int16>( data, nelements, t ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return( false );
            }
        }
        else if ( m_type == "unsigned short" || m_type == "ushort" )
        {
            if ( !kvs::kvsml::DataArray::ReadInternalData<kvs::UInt16>( data, nelements, t ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return( false );
            }
        }
        else if ( m_type == "int" )
        {
            if ( !kvs::kvsml::DataArray::ReadInternalData<kvs::Int32>( data, nelements, t ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return( false );
            }
        }
        else if ( m_type == "unsigned int" || m_type == "uint" )
        {
            if ( !kvs::kvsml::DataArray::ReadInternalData<kvs::UInt32>( data, nelements, t ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return( false );
            }
        }
        else if ( m_type == "float" )
        {
            if ( !kvs::kvsml::DataArray::ReadInternalData<kvs::Real32>( data, nelements, t ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return( false );
            }
        }
        else if ( m_type == "double" )
        {
            if ( !kvs::kvsml::DataArray::ReadInternalData<kvs::Real64>( data, nelements, t ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return( false );
            }
        }
        else
        {
            kvsMessageError( "'type' is not specified or unknown data type in <%s>.", tag_name.c_str() );
            return( false );
        }
    }

    // External data.
    else
    {
        // <DataArray file="xxx" type="xxx" format="xxx"/>
        if( m_format == "" )
        {
            kvsMessageError( "'format' is not specified in <%s>.", tag_name.c_str() );
            return( false );
        }

        // Filename as an absolute path.
        const kvs::XMLDocument* document
            = reinterpret_cast<kvs::XMLDocument*>( m_node->GetDocument() );
        const std::string path = kvs::File( document->filename() ).pathName( true );
        const std::string filename = path + kvs::File::Separator() + m_file;

        if( m_type == "char" )
        {
            if ( !kvs::kvsml::DataArray::ReadExternalData<kvs::Int8>( data, nelements, filename, m_format ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return( false );
            }
        }
        else if( m_type == "unsigned char" || m_type == "uchar" )
        {
            if ( !kvs::kvsml::DataArray::ReadExternalData<kvs::UInt8>( data, nelements, filename, m_format ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return( false );
            }
        }
        else if ( m_type == "short" )
        {
            if ( !kvs::kvsml::DataArray::ReadExternalData<kvs::Int16>( data, nelements, filename, m_format ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return( false );
            }
        }
        else if ( m_type == "unsigned short" || m_type == "ushort" )
        {
            if ( !kvs::kvsml::DataArray::ReadExternalData<kvs::UInt16>( data, nelements, filename, m_format ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return( false );
            }
        }
        else if ( m_type == "int" )
        {
            if ( !kvs::kvsml::DataArray::ReadExternalData<kvs::Int32>( data, nelements, filename, m_format ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return( false );
            }
        }
        else if ( m_type == "unsigned int" || m_type == "uint" )
        {
            if ( !kvs::kvsml::DataArray::ReadExternalData<kvs::UInt32>( data, nelements, filename, m_format ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return( false );
            }
        }
        else if ( m_type == "float" )
        {
            if ( !kvs::kvsml::DataArray::ReadExternalData<kvs::Real32>( data, nelements, filename, m_format ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return( false );
            }
        }
        else if ( m_type == "double" )
        {
            if ( !kvs::kvsml::DataArray::ReadExternalData<kvs::Real64>( data, nelements, filename, m_format ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return( false );
            }
        }
        else
        {
            kvsMessageError( "'type' is not specified or unknown data type in <%s>.", tag_name.c_str() );
            return( false );
        }
    }

    return( true );
}

const bool DataArrayTag::read( const kvs::XMLNode::SuperClass* parent )
{
    kvs::IgnoreUnusedVariable( parent );
    return( true );
}

const bool DataArrayTag::write( kvs::XMLNode::SuperClass* parent )
{
    kvs::IgnoreUnusedVariable( parent );
    return( true );
}

} // end of namespace kvsml

} // end of namespace kvs
