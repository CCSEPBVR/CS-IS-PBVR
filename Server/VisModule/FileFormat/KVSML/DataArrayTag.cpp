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
#include <vismodule/DataArrayTag>
#include "DataArray.h"
#include <vismodule/XMLNode>
#include <vismodule/XMLElement>
#include <vismodule/XMLDocument>
#include <vismodule/File>
#include <vismodule/ValueArray>
#include <vismodule/AnyValueArray>
#include <vismodule/IgnoreUnusedVariable>
#include <iostream>
#include <fstream>
#include <sstream>


namespace vismodule
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new data array tag class.
 */
/*===========================================================================*/
DataArrayTag::DataArrayTag():
    vismodule::kvsml::TagBase( "DataArray" ),
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
DataArrayTag::~DataArrayTag()
{
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the tag has 'type' or not.
 *  @return true, if the tag has 'type'
 */
/*===========================================================================*/
const bool DataArrayTag::hasType() const
{
    return m_has_type;
}

/*===========================================================================*/
/**
 *  @brief  Returns a type that is specfied by 'type'.
 *  @return type
 */
/*===========================================================================*/
const std::string& DataArrayTag::type() const
{
    return m_type;
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the tag has 'file' or not.
 *  @return true, if the tag has 'file'
 */
/*===========================================================================*/
const bool DataArrayTag::hasFile() const
{
    return m_has_file;
}

/*===========================================================================*/
/**
 *  @brief  Returns a external file name that is specfied by 'file'.
 *  @return external file name
 */
/*===========================================================================*/
const std::string& DataArrayTag::file() const
{
    return m_file;
}

/*===========================================================================*/
/**
 *  @brief  Tests whether the tag has 'format' or not.
 *  @return true, if the tag has 'format'
 */
/*===========================================================================*/
const bool DataArrayTag::hasFormat() const
{
    return m_has_format;
}

/*===========================================================================*/
/**
 *  @brief  Returns a external file format that is specfied by 'format'.
 *  @return external file format
 */
/*===========================================================================*/
const std::string& DataArrayTag::format() const
{
    return m_format;
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
    const vismodule::XMLNode::SuperClass* parent,
    const std::size_t nelements,
    vismodule::AnyValueArray* data )
{
    const std::string tag_name = BaseClass::name();

    BaseClass::m_node = vismodule::XMLNode::FindChildNode( parent, tag_name );
    if ( !BaseClass::m_node )
    {
        visModuleMessageError( "Cannot find <%s>.", tag_name.c_str() );
        return false;
    }

    this->read_attribute();

    return this->read_data( nelements, data );
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
    vismodule::XMLNode::SuperClass* parent,
    const vismodule::AnyValueArray& data,
    const std::string& pathname )
{
    if ( data.size() == 0 ) return true;

    const std::string tag_name = BaseClass::name();
    vismodule::XMLElement element( tag_name );
    element.setAttribute( "type", vismodule::kvsml::DataArray::GetDataType( data ) );

    // Internal data: <DataArray type="xxx">xxx</DataArray>
    if ( !m_has_file )
    {
        // Write the data array to string-stream.
        std::ostringstream oss( std::ostringstream::out );
        const std::type_info& data_type = data.typeInfo()->type();
        const std::size_t data_size = data.size();
        if ( data_type == typeid( vismodule::Int8 ) )
        {
            const vismodule::Int8* values = data.pointer<vismodule::Int8>();
            for ( std::size_t i = 0; i < data_size; i++ ) oss << vismodule::Int16( values[i] ) << " ";
        }
        else if ( data_type == typeid( vismodule::UInt8 ) )
        {
            const vismodule::UInt8* values = data.pointer<vismodule::UInt8>();
            for ( std::size_t i = 0; i < data_size; i++ ) oss << vismodule::UInt16( values[i] ) << " ";
        }
        else if ( data_type == typeid( vismodule::Int16 ) )
        {
            const vismodule::Int16* values = data.pointer<vismodule::Int16>();
            for ( std::size_t i = 0; i < data_size; i++ ) oss << values[i] << " ";
        }
        else if ( data_type == typeid( vismodule::UInt16 ) )
        {
            const vismodule::UInt16* values = data.pointer<vismodule::UInt16>();
            for ( std::size_t i = 0; i < data_size; i++ ) oss << values[i] << " ";
        }
        else if ( data_type == typeid( vismodule::Int32 ) )
        {
            const vismodule::Int32* values = data.pointer<vismodule::Int32>();
            for ( std::size_t i = 0; i < data_size; i++ ) oss << values[i] << " ";
        }
        else if ( data_type == typeid( vismodule::UInt32 ) )
        {
            const vismodule::UInt32* values = data.pointer<vismodule::UInt32>();
            for ( std::size_t i = 0; i < data_size; i++ ) oss << values[i] << " ";
        }
        else if ( data_type == typeid( vismodule::Real32 ) )
        {
            const vismodule::Real32* values = data.pointer<vismodule::Real32>();
            for ( std::size_t i = 0; i < data_size; i++ ) oss << values[i] << " ";
        }
        else if ( data_type == typeid( vismodule::Real64 ) )
        {
            const vismodule::Real64* values = data.pointer<vismodule::Real64>();
            for ( std::size_t i = 0; i < data_size; i++ ) oss << values[i] << " ";
        }
        else if ( data_type == typeid( vismodule::Real128 ) )
        {
            const vismodule::Real128* values = data.pointer<vismodule::Real128>();
            for ( std::size_t i = 0; i < data_size; i++ ) oss << values[i] << " ";
        }

        // Insert the data array as string-stream to the parent node.
        vismodule::TiXmlText text;
        text.SetValue( oss.str() );

        vismodule::XMLNode::SuperClass* node = parent->InsertEndChild( element );
        if ( !node )
        {
            visModuleMessageError( "Cannot insert <%s>.", tag_name.c_str() );
            return false;
        }

        return node->InsertEndChild( text ) != NULL;
    }

    // External data: <DataArray type="xxx" format="xxx" file="xxx"/>
    else
    {
        if ( !m_has_format )
        {
            visModuleMessageError( "'format' is not spcified in <%s>.", tag_name.c_str() );
            return false;
        }

        element.setAttribute( "file", m_file );
        element.setAttribute( "format", m_format );
        parent->InsertEndChild( element );

        // Write the data to the external data file.
        const std::string filename = pathname + vismodule::File::Separator() + m_file;
        return vismodule::kvsml::DataArray::WriteExternalData( data, m_file, m_format );
    }
}

/*===========================================================================*/
/**
 *  @brief  Reads attributes.
 */
/*===========================================================================*/
const void DataArrayTag::read_attribute()
{
    // Element
    const vismodule::XMLElement::SuperClass* element = vismodule::XMLNode::ToElement( m_node );

    // type="xxx"
    const std::string type = vismodule::XMLElement::AttributeValue( element, "type" );
    if ( type != "" )
    {
        m_has_type = true;
        m_type = type;
    }

    // file="xxx"
    const std::string file = vismodule::XMLElement::AttributeValue( element, "file" );
    if ( file != "" )
    {
        m_has_file = true;
        m_file = file;
    }

    // format="xxx"
    const std::string format = vismodule::XMLElement::AttributeValue( element, "format" );
    if ( format != "" )
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
const bool DataArrayTag::read_data( const std::size_t nelements, vismodule::AnyValueArray* data )
{
    const std::string tag_name = BaseClass::name();

    // Internal data.
    if ( m_file == "" )
    {
        const vismodule::TiXmlText* array_text = vismodule::XMLNode::ToText( m_node );
        if ( !array_text )
        {
            visModuleMessageError( "No value in <%s>.", tag_name.c_str() );
            return false;
        }

        // <DataArray type="xxx">xxx</DataArray>
        const std::string delim( " ,\t\n" );
        vismodule::Tokenizer t( array_text->Value(), delim );

        if ( m_type == "char" )
        {
            if ( !vismodule::kvsml::DataArray::ReadInternalData<vismodule::Int8>( data, nelements, t ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "unsigned char" || m_type == "uchar" )
        {
            if ( !vismodule::kvsml::DataArray::ReadInternalData<vismodule::UInt8>( data, nelements, t ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "short" )
        {
            if ( !vismodule::kvsml::DataArray::ReadInternalData<vismodule::Int16>( data, nelements, t ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "unsigned short" || m_type == "ushort" )
        {
            if ( !vismodule::kvsml::DataArray::ReadInternalData<vismodule::UInt16>( data, nelements, t ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "int" )
        {
            if ( !vismodule::kvsml::DataArray::ReadInternalData<vismodule::Int32>( data, nelements, t ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "unsigned int" || m_type == "uint" )
        {
            if ( !vismodule::kvsml::DataArray::ReadInternalData<vismodule::UInt32>( data, nelements, t ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "float" )
        {
            if ( !vismodule::kvsml::DataArray::ReadInternalData<vismodule::Real32>( data, nelements, t ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "double" )
        {
            if ( !vismodule::kvsml::DataArray::ReadInternalData<vismodule::Real64>( data, nelements, t ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else
        {
            visModuleMessageError( "'type' is not specified or unknown data type in <%s>.", tag_name.c_str() );
            return false;
        }
    }

    // External data.
    else
    {
        // <DataArray file="xxx" type="xxx" format="xxx"/>
        if ( m_format == "" )
        {
            visModuleMessageError( "'format' is not specified in <%s>.", tag_name.c_str() );
            return false;
        }

        // Filename as an absolute path.
        const vismodule::XMLDocument* document
            = reinterpret_cast<vismodule::XMLDocument*>( m_node->GetDocument() );
        const std::string path = vismodule::File( document->filename() ).pathName( true );
        const std::string filename = path + vismodule::File::Separator() + m_file;

        if ( m_type == "char" )
        {
            if ( !vismodule::kvsml::DataArray::ReadExternalData<vismodule::Int8>( data, nelements, filename, m_format ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "unsigned char" || m_type == "uchar" )
        {
            if ( !vismodule::kvsml::DataArray::ReadExternalData<vismodule::UInt8>( data, nelements, filename, m_format ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "short" )
        {
            if ( !vismodule::kvsml::DataArray::ReadExternalData<vismodule::Int16>( data, nelements, filename, m_format ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "unsigned short" || m_type == "ushort" )
        {
            if ( !vismodule::kvsml::DataArray::ReadExternalData<vismodule::UInt16>( data, nelements, filename, m_format ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "int" )
        {
            if ( !vismodule::kvsml::DataArray::ReadExternalData<vismodule::Int32>( data, nelements, filename, m_format ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "unsigned int" || m_type == "uint" )
        {
            if ( !vismodule::kvsml::DataArray::ReadExternalData<vismodule::UInt32>( data, nelements, filename, m_format ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "float" )
        {
            if ( !vismodule::kvsml::DataArray::ReadExternalData<vismodule::Real32>( data, nelements, filename, m_format ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "double" )
        {
            if ( !vismodule::kvsml::DataArray::ReadExternalData<vismodule::Real64>( data, nelements, filename, m_format ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else
        {
            visModuleMessageError( "'type' is not specified or unknown data type in <%s>.", tag_name.c_str() );
            return false;
        }
    }

    return true;
}

const bool DataArrayTag::read( const vismodule::XMLNode::SuperClass* parent )
{
    vismodule::IgnoreUnusedVariable( parent );
    return true;
}

const bool DataArrayTag::write( vismodule::XMLNode::SuperClass* parent )
{
    vismodule::IgnoreUnusedVariable( parent );
    return true;
}

} // end of namespace kvsml

} // end of namespace vismodule
