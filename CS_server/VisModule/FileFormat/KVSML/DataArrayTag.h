/*****************************************************************************/
/**
 *  @file   DataArrayTag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DataArrayTag.h 667 2011-02-22 16:07:54Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__KVSML__DATA_ARRAY_TAG_H_INCLUDE
#define VIS_MODULE__KVSML__DATA_ARRAY_TAG_H_INCLUDE

#include <string>
#include <vismodule/ValueArray>
#include <vismodule/AnyValueArray>
#include <vismodule/File>
#include <vismodule/XMLNode>
#include <vismodule/XMLElement>
#include <vismodule/XMLDocument>
#include "DataArray.h"
#include <FileFormat/KVSML/TagBase.h>


namespace vismodule
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <DataArray>
 */
/*===========================================================================*/
class DataArrayTag : public vismodule::kvsml::TagBase
{
public:

    typedef vismodule::kvsml::TagBase BaseClass;

protected:

    bool m_has_type; ///< flag to check whether 'type' is specified or not
    std::string m_type; ///< data type string
    bool m_has_file; ///< flag to check whether 'file' is specified or not
    std::string m_file; ///< external file name
    bool m_has_format; ///< flag to check whether 'format' is specified or not
    std::string m_format; ///< external file format

public:

    DataArrayTag();

    virtual ~DataArrayTag();

public:

    const bool hasType() const;

    const std::string& type() const;

    const bool hasFile() const;

    const std::string& file() const;

    const bool hasFormat() const;

    const std::string& format() const;

public:

    void setFile( const std::string& file );

    void setFormat( const std::string& format );

public:

    const bool read( const vismodule::XMLNode::SuperClass* parent, const size_t nelements, vismodule::AnyValueArray* data );

    template <typename T>
    const bool read( const vismodule::XMLNode::SuperClass* parent, const size_t nelements, vismodule::ValueArray<T>* data );

    const bool write( vismodule::XMLNode::SuperClass* parent, const vismodule::AnyValueArray& data, const std::string& pathname );

    template <typename T>
    const bool write( vismodule::XMLNode::SuperClass* parent, const vismodule::ValueArray<T>& data, const std::string& pathname );

private:

    const void read_attribute();

    const bool read_data( const size_t nelements, vismodule::AnyValueArray* data );

    template <typename T>
    const bool read_data( const size_t nelements, vismodule::ValueArray<T>* data );

private:

    const bool read( const vismodule::XMLNode::SuperClass* parent );

    const bool write( vismodule::XMLNode::SuperClass* parent );
};

/*===========================================================================*/
/**
 *  @brief  Reads a data array tag.
 *  @param  parent [in] pointer to the parent node
 *  @param  nelements [in] number of elements of the data array
 *  @param  data [out] data array
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
template <typename T>
inline const bool DataArrayTag::read(
    const vismodule::XMLNode::SuperClass* parent,
    const size_t nelements,
    vismodule::ValueArray<T>* data )
{
    const std::string tag_name = BaseClass::name();

    BaseClass::m_node = vismodule::XMLNode::FindChildNode( parent, tag_name );
    if ( !BaseClass::m_node )
    {
        visModuleMessageError( "Cannot find <%s>.", tag_name.c_str() );
        return false;
    }

    this->read_attribute();

    return this->read_data<T>( nelements, data );
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
template <typename T>
inline const bool DataArrayTag::write(
    vismodule::XMLNode::SuperClass* parent,
    const vismodule::ValueArray<T>& data,
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
        const size_t data_size = data.size();
        if ( typeid( T ) == typeid( vismodule::Int8 ) || typeid( T ) == typeid( vismodule::UInt8 ) )
        {
            for ( size_t i = 0; i < data_size; i++ ) oss << int( data.at( i ) ) << " ";
        }
        else
        {
            for ( size_t i = 0; i < data_size; i++ ) oss << data.at( i ) << " ";
        }

        // Insert the data array as string-stream to the parent node.
        vismodule::TiXmlText text;
        text.SetValue( oss.str() );

        vismodule::XMLNode::SuperClass* node = parent->InsertEndChild( element );
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

        // Set text.
        const std::string filename = pathname + vismodule::File::Separator() + m_file;
        return vismodule::kvsml::DataArray::WriteExternalData( data, filename, m_format );
    }

    return true;
}

/*===========================================================================*/
/**
 *  @brief  Reads a data array.
 *  @param  nelements [in] number of elements
 *  @param  data [out] pointer to the data array
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
template <typename T>
const bool DataArrayTag::read_data( const size_t nelements, vismodule::ValueArray<T>* data )
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

        // <DataArray>xxx</DataArray>
        const std::string delim( " \n" );
        vismodule::Tokenizer tokenizer( array_text->Value(), delim );

        if ( !vismodule::kvsml::DataArray::ReadInternalData<T>( data, nelements, tokenizer ) )
        {
            visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
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
            if ( !vismodule::kvsml::DataArray::ReadExternalData<T, vismodule::Int8>( data, nelements, filename, m_format ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "unsigned char" || m_type == "uchar" )
        {
            if ( !vismodule::kvsml::DataArray::ReadExternalData<T, vismodule::UInt8>( data, nelements, filename, m_format ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "short" )
        {
            if ( !vismodule::kvsml::DataArray::ReadExternalData<T, vismodule::Int16>( data, nelements, filename, m_format ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "unsigned short" || m_type == "ushort" )
        {
            if ( !vismodule::kvsml::DataArray::ReadExternalData<T, vismodule::UInt16>( data, nelements, filename, m_format ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "int" )
        {
            if ( !vismodule::kvsml::DataArray::ReadExternalData<T, vismodule::Int32>( data, nelements, filename, m_format ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "unsigned int" || m_type == "uint" )
        {
            if ( !vismodule::kvsml::DataArray::ReadExternalData<T, vismodule::UInt32>( data, nelements, filename, m_format ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "float" )
        {
            if ( !vismodule::kvsml::DataArray::ReadExternalData<T, vismodule::Real32>( data, nelements, filename, m_format ) )
            {
                visModuleMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "double" )
        {
            if ( !vismodule::kvsml::DataArray::ReadExternalData<T, vismodule::Real64>( data, nelements, filename, m_format ) )
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

} // end of namespace kvsml

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML__DATA_ARRAY_TAG_H_INCLUDE
