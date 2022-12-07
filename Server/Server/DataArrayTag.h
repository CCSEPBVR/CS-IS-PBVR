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
#ifndef PBVR__KVS__KVSML__DATA_ARRAY_TAG_H_INCLUDE
#define PBVR__KVS__KVSML__DATA_ARRAY_TAG_H_INCLUDE

#include <string>
#include <kvs/ValueArray>
#include <kvs/AnyValueArray>
#include <kvs/File>
#include <kvs/XMLNode>
#include <kvs/XMLElement>
#include <kvs/XMLDocument>
#include "DataArray.h"
#include <Core/FileFormat/KVSML/TagBase.h>


namespace pbvr
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <DataArray>
 */
/*===========================================================================*/
class DataArrayTag : public kvs::kvsml::TagBase
{
public:

    typedef kvs::kvsml::TagBase BaseClass;

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

    const bool read( const kvs::XMLNode::SuperClass* parent, const size_t nelements, kvs::AnyValueArray* data );

    template <typename T>
    const bool read( const kvs::XMLNode::SuperClass* parent, const size_t nelements, kvs::ValueArray<T>* data );

    const bool write( kvs::XMLNode::SuperClass* parent, const kvs::AnyValueArray& data, const std::string& pathname );

    template <typename T>
    const bool write( kvs::XMLNode::SuperClass* parent, const kvs::ValueArray<T>& data, const std::string& pathname );

private:

    const void read_attribute();

    const bool read_data( const size_t nelements, kvs::AnyValueArray* data );

    template <typename T>
    const bool read_data( const size_t nelements, kvs::ValueArray<T>* data );

private:

    const bool read( const kvs::XMLNode::SuperClass* parent );

    const bool write( kvs::XMLNode::SuperClass* parent );
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
    const kvs::XMLNode::SuperClass* parent,
    const size_t nelements,
    kvs::ValueArray<T>* data )
{
    const std::string tag_name = BaseClass::name();

    BaseClass::m_node = kvs::XMLNode::FindChildNode( parent, tag_name );
    if ( !BaseClass::m_node )
    {
        kvsMessageError( "Cannot find <%s>.", tag_name.c_str() );
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
    kvs::XMLNode::SuperClass* parent,
    const kvs::ValueArray<T>& data,
    const std::string& pathname )
{
    if ( data.size() == 0 ) return true;

    const std::string tag_name = BaseClass::name();
    kvs::XMLElement element( tag_name );
    element.setAttribute( "type", pbvr::kvsml::DataArray::GetDataType( data ) );

    // Internal data: <DataArray type="xxx">xxx</DataArray>
    if ( !m_has_file )
    {
        // Write the data array to string-stream.
        std::ostringstream oss( std::ostringstream::out );
        const size_t data_size = data.size();
        if ( typeid( T ) == typeid( kvs::Int8 ) || typeid( T ) == typeid( kvs::UInt8 ) )
        {
            for ( size_t i = 0; i < data_size; i++ ) oss << int( data.at( i ) ) << " ";
        }
        else
        {
            for ( size_t i = 0; i < data_size; i++ ) oss << data.at( i ) << " ";
        }

        // Insert the data array as string-stream to the parent node.
        TiXmlText text;
        text.SetValue( oss.str() );

        kvs::XMLNode::SuperClass* node = parent->InsertEndChild( element );
        return node->InsertEndChild( text ) != NULL;
    }

    // External data: <DataArray type="xxx" format="xxx" file="xxx"/>
    else
    {
        if ( !m_has_format )
        {
            kvsMessageError( "'format' is not spcified in <%s>.", tag_name.c_str() );
            return false;
        }

        element.setAttribute( "file", m_file );
        element.setAttribute( "format", m_format );
        parent->InsertEndChild( element );

        // Set text.
        const std::string filename = pathname + kvs::File::Separator() + m_file;
        return pbvr::kvsml::DataArray::WriteExternalData( data, filename, m_format );
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
const bool DataArrayTag::read_data( const size_t nelements, kvs::ValueArray<T>* data )
{
    const std::string tag_name = BaseClass::name();

    // Internal data.
    if ( m_file == "" )
    {
        const TiXmlText* array_text = kvs::XMLNode::ToText( m_node );
        if ( !array_text )
        {
            kvsMessageError( "No value in <%s>.", tag_name.c_str() );
            return false;
        }

        // <DataArray>xxx</DataArray>
        const std::string delim( " \n" );
        kvs::Tokenizer tokenizer( array_text->Value(), delim );

        if ( !pbvr::kvsml::DataArray::ReadInternalData<T>( data, nelements, tokenizer ) )
        {
            kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
            return false;
        }
    }
    // External data.
    else
    {
        // <DataArray file="xxx" type="xxx" format="xxx"/>
        if ( m_format == "" )
        {
            kvsMessageError( "'format' is not specified in <%s>.", tag_name.c_str() );
            return false;
        }

        // Filename as an absolute path.
        const kvs::XMLDocument* document
            = reinterpret_cast<kvs::XMLDocument*>( m_node->GetDocument() );
        const std::string path = kvs::File( document->filename() ).pathName( true );
        const std::string filename = path + kvs::File::Separator() + m_file;

        if ( m_type == "char" )
        {
            if ( !pbvr::kvsml::DataArray::ReadExternalData<T, kvs::Int8>( data, nelements, filename, m_format ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "unsigned char" || m_type == "uchar" )
        {
            if ( !pbvr::kvsml::DataArray::ReadExternalData<T, kvs::UInt8>( data, nelements, filename, m_format ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "short" )
        {
            if ( !pbvr::kvsml::DataArray::ReadExternalData<T, kvs::Int16>( data, nelements, filename, m_format ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "unsigned short" || m_type == "ushort" )
        {
            if ( !pbvr::kvsml::DataArray::ReadExternalData<T, kvs::UInt16>( data, nelements, filename, m_format ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "int" )
        {
            if ( !pbvr::kvsml::DataArray::ReadExternalData<T, kvs::Int32>( data, nelements, filename, m_format ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "unsigned int" || m_type == "uint" )
        {
            if ( !pbvr::kvsml::DataArray::ReadExternalData<T, kvs::UInt32>( data, nelements, filename, m_format ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "float" )
        {
            if ( !pbvr::kvsml::DataArray::ReadExternalData<T, kvs::Real32>( data, nelements, filename, m_format ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else if ( m_type == "double" )
        {
            if ( !pbvr::kvsml::DataArray::ReadExternalData<T, kvs::Real64>( data, nelements, filename, m_format ) )
            {
                kvsMessageError( "Cannot read the data array in <%s>.", tag_name.c_str() );
                return false;
            }
        }
        else
        {
            kvsMessageError( "'type' is not specified or unknown data type in <%s>.", tag_name.c_str() );
            return false;
        }
    }

    return true;
}

} // end of namespace kvsml

} // end of namespace kvs

#endif // PBVR__KVS__KVSML__DATA_ARRAY_TAG_H_INCLUDE
