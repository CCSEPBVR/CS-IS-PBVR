/*****************************************************************************/
/**
 *  @file   KVSMLObjectTable.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KVSMLObjectTable.cpp 849 2011-07-02 06:28:53Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "KVSMLObjectTable.h"
#include "TableObjectTag.h"
#include "ColumnTag.h"
#include "DataArrayTag.h"
#include <kvs/File>
#include <kvs/XMLDocument>
#include <kvs/XMLDeclaration>
#include <kvs/XMLElement>
#include <kvs/XMLComment>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new KVSML table object class.
 */
/*===========================================================================*/
KVSMLObjectTable::KVSMLObjectTable( void ):
    m_writing_type( kvs::KVSMLObjectTable::Ascii ),
    m_nrows(0),
    m_ncolumns(0)
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new KVSML table object class by reading the given file.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
KVSMLObjectTable::KVSMLObjectTable( const std::string& filename ):
    m_writing_type( kvs::KVSMLObjectTable::Ascii ),
    m_nrows(0),
    m_ncolumns(0)
{
    if ( this->read( filename ) ) { m_is_success = true; }
    else { m_is_success = false; }
}

/*===========================================================================*/
/**
 *  @brief  Destructs the KVSML object unstructured volume object class.
 */
/*===========================================================================*/
KVSMLObjectTable::~KVSMLObjectTable( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns the KVSML tag.
 *  @return KVSML tag
 */
/*===========================================================================*/
const kvs::kvsml::KVSMLTag& KVSMLObjectTable::KVSMLTag( void ) const
{
    return( m_kvsml_tag );
}

/*===========================================================================*/
/**
 *  @brief  Returns the object tag.
 *  @return object tag
 */
/*===========================================================================*/
const kvs::kvsml::ObjectTag& KVSMLObjectTable::objectTag( void ) const
{
    return( m_object_tag );
}

/*===========================================================================*/
/**
 *  @brief  Returns number of rows.
 *  @return number of rows
 */
/*===========================================================================*/
const size_t KVSMLObjectTable::nrows( void ) const
{
    return( m_nrows );
}

/*===========================================================================*/
/**
 *  @brief  Returns number of columns.
 *  @return number of columns
 */
/*===========================================================================*/
const size_t KVSMLObjectTable::ncolumns( void ) const
{
    return( m_ncolumns );
}

/*===========================================================================*/
/**
 *  @brief  Returns label list.
 *  @return label list
 */
/*===========================================================================*/
const std::vector<std::string>& KVSMLObjectTable::labelList( void ) const
{
    return( m_labels );
}

/*===========================================================================*/
/**
 *  @brief  Returns column list.
 *  @return column list
 */
/*===========================================================================*/
const std::vector<kvs::AnyValueArray>& KVSMLObjectTable::columnList( void ) const
{
    return( m_columns );
}

const std::vector<bool>& KVSMLObjectTable::hasMinValueList( void ) const
{
    return( m_has_min_values );
}

const std::vector<bool>& KVSMLObjectTable::hasMaxValueList( void ) const
{
    return( m_has_max_values );
}

const std::vector<bool>& KVSMLObjectTable::hasMinRangeList( void ) const
{
    return( m_has_min_ranges );
}

const std::vector<bool>& KVSMLObjectTable::hasMaxRangeList( void ) const
{
    return( m_has_max_ranges );
}

const std::vector<double>& KVSMLObjectTable::minValueList( void ) const
{
    return( m_min_values );
}

const std::vector<double>& KVSMLObjectTable::maxValueList( void ) const
{
    return( m_max_values );
}

const std::vector<double>& KVSMLObjectTable::minRangeList( void ) const
{
    return( m_min_ranges );
}

const std::vector<double>& KVSMLObjectTable::maxRangeList( void ) const
{
    return( m_max_ranges );
}

/*===========================================================================*/
/**
 *  @brief  Sets a writing data type.
 *  @param  writing_type [in] writing data type
 */
/*===========================================================================*/
void KVSMLObjectTable::setWritingDataType( const WritingDataType writing_type )
{
    m_writing_type = writing_type;
}

/*===========================================================================*/
/**
 *  @brief  Sets a filename.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
void KVSMLObjectTable::setFilename( const std::string& filename )
{
    m_filename = filename;
}

/*===========================================================================*/
/**
 *  @brief  Sets a min. value.
 *  @param  value [in] min. value
 */
/*===========================================================================*/
void KVSMLObjectTable::setMinValue( const size_t column_index, const double value )
{
    m_has_min_values[column_index] = true;
    m_min_values[column_index] = value;
}

/*===========================================================================*/
/**
 *  @brief  Sets a max. value.
 *  @param  value [in] max. value
 */
/*===========================================================================*/
void KVSMLObjectTable::setMaxValue( const size_t column_index, const double value )
{
    m_has_max_values[column_index] = true;
    m_max_values[column_index] = value;
}

/*===========================================================================*/
/**
 *  @brief  Sets a min. range.
 *  @param  value [in] min. range
 */
/*===========================================================================*/
void KVSMLObjectTable::setMinRange( const size_t column_index, const double range )
{
    m_has_min_ranges[column_index] = true;
    m_min_ranges[column_index] = range;
}

/*===========================================================================*/
/**
 *  @brief  Sets a max. range.
 *  @param  value [in] max. range
 */
/*===========================================================================*/
void KVSMLObjectTable::setMaxRange( const size_t column_index, const double range )
{
    m_has_max_ranges[column_index] = true;
    m_max_ranges[column_index] = range;
}

/*===========================================================================*/
/**
 *  @brief  Adds a column.
 *  @param  column [in] column
 *  @param  label [in] column label
 */
/*===========================================================================*/
void KVSMLObjectTable::addColumn( const kvs::AnyValueArray& column, const std::string& label )
{
    m_columns.push_back( column );
    m_labels.push_back( label );

    m_ncolumns++;
    m_nrows = m_nrows > column.size() ? m_nrows : column.size();
}

/*===========================================================================*/
/**
 *  @brief  Read a KVSML table object from the given file.
 *  @param  filename [in] filename
 *  @return true, if the reading process is done successfully
 */
/*===========================================================================*/
const bool KVSMLObjectTable::read( const std::string& filename )
{
    m_filename = filename;

    // XML document
    kvs::XMLDocument document;
    if ( !document.read( filename ) )
    {
        kvsMessageError( "%s", document.ErrorDesc().c_str() );
        return( false );
    }

    // <KVSML>
    if ( !m_kvsml_tag.read( &document ) )
    {
        kvsMessageError( "Cannot read <%s>.", m_kvsml_tag.name().c_str() );
        return( false );
    }

    // <Object>
    if ( !m_object_tag.read( m_kvsml_tag.node() ) )
    {
        kvsMessageError( "Cannot read <%s>.", m_object_tag.name().c_str() );
        return( false );
    }

    // <TableObject>
    kvs::kvsml::TableObjectTag table_object_tag;
    if ( !table_object_tag.read( m_object_tag.node() ) )
    {
        kvsMessageError( "Cannot read <%s>.", table_object_tag.name().c_str() );
        return( false );
    }

    m_nrows = table_object_tag.nrows();
    m_ncolumns = table_object_tag.ncolumns();

    // <Column>
    kvs::kvsml::ColumnTag column_tag;
    kvs::XMLNode::SuperClass* node = kvs::XMLNode::FindChildNode( table_object_tag.node(), column_tag.name() );
    size_t counter = 0;
    while ( node )
    {
        column_tag.read( kvs::XMLNode::ToElement( node ) );

        if ( counter++ < m_ncolumns )
        {
            m_labels.push_back( column_tag.label() );
            m_has_min_values.push_back( column_tag.hasMinValue() );
            m_has_max_values.push_back( column_tag.hasMaxValue() );
            m_has_min_ranges.push_back( column_tag.hasMinRange() );
            m_has_max_ranges.push_back( column_tag.hasMaxRange() );
            m_min_values.push_back( column_tag.minValue() );
            m_max_values.push_back( column_tag.maxValue() );
            m_min_ranges.push_back( column_tag.minRange() );
            m_max_ranges.push_back( column_tag.maxRange() );

            kvs::AnyValueArray data_array;
            kvs::kvsml::DataArrayTag data_array_tag;
            if ( !data_array_tag.read( node, m_nrows, &data_array ) )
            {
                kvsMessageError( "Cannot read <%s>.", data_array_tag.name().c_str() );
                return( false );
            }

            m_columns.push_back( data_array );
        }

        node = table_object_tag.node()->IterateChildren( column_tag.name(), node );
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Writes the KVSML table object to the given file.
 *  @param  filename [in] filename
 *  @return true, if the writing process is done successfully
 */
/*===========================================================================*/
const bool KVSMLObjectTable::write( const std::string& filename )
{
    m_filename = filename;

    kvs::XMLDocument document;
    document.InsertEndChild( kvs::XMLDeclaration("1.0") );
    document.InsertEndChild( kvs::XMLComment(" Generated by kvs::KVSMLObjectTable::write() ") );

    // <KVSML>
    kvs::kvsml::KVSMLTag kvsml_tag;
    if ( !kvsml_tag.write( &document ) )
    {
        kvsMessageError( "Cannot write <%s>.", m_kvsml_tag.name().c_str() );
        return( false );
    }

    // <Object type="TableObject">
    kvs::kvsml::ObjectTag object_tag;
    object_tag.setType( "TableObject" );
    if ( !object_tag.write( kvsml_tag.node() ) )
    {
        kvsMessageError( "Cannot write <%s>.", object_tag.name().c_str() );
        return( false );
    }

    // <TableObject nrows="xxx", ncolumns="xxx">
    kvs::kvsml::TableObjectTag table_object_tag;
    table_object_tag.setNRows( m_nrows );
    table_object_tag.setNColumns( m_ncolumns );
    if ( !table_object_tag.write( object_tag.node() ) )
    {
        kvsMessageError( "Cannot write <%s>.", table_object_tag.name().c_str() );
        return( false );
    }

    for ( size_t i = 0; i < m_ncolumns; i++ )
    {
        // <Column label="xxx" min_value="xxx" max_value="xxx" min_range="xxx" max_range="xxx">
        kvs::kvsml::ColumnTag column_tag;
        const std::string label = m_labels.at(i);
        column_tag.setLabel( label );

        if ( m_has_min_values.at(i) ) { column_tag.setMinValue( m_min_values.at(i) ); }
        if ( m_has_max_values.at(i) ) { column_tag.setMaxValue( m_max_values.at(i) ); }
        if ( m_has_min_ranges.at(i) ) { column_tag.setMinRange( m_min_ranges.at(i) ); }
        if ( m_has_max_ranges.at(i) ) { column_tag.setMaxRange( m_max_ranges.at(i) ); }

        if ( !column_tag.write( table_object_tag.node() ) )
        {
            kvsMessageError( "Cannot write <%s>.", column_tag.name().c_str() );
            return( false );
        }

        // <DataArray>
        kvs::kvsml::DataArrayTag data_array;
        if ( m_writing_type == ExternalAscii )
        {
            data_array.setFile( kvs::kvsml::DataArray::GetDataFilename( m_filename, label ) );
            data_array.setFormat( "ascii" );
        }
        else if ( m_writing_type == ExternalBinary )
        {
            data_array.setFile( kvs::kvsml::DataArray::GetDataFilename( m_filename, label ) );
            data_array.setFormat( "binary" );
        }

        const std::string pathname = kvs::File( m_filename ).pathName();
        if ( !data_array.write( column_tag.node(), m_columns.at(i), pathname ) )
        {
            kvsMessageError( "Cannot write <%s>.", data_array.name().c_str() );
            return( false );
        }
    }

    return( document.write( m_filename ) );
}

/*===========================================================================*/
/**
 *  @brief  Checks the file extension.
 *  @param  filename [in] filename
 *  @return true, if the given filename has the supported extension
 */
/*===========================================================================*/
const bool KVSMLObjectTable::CheckFileExtension( const std::string& filename )
{
    const kvs::File file( filename );
    if ( file.extension() == "kvsml" ||
         file.extension() == "KVSML" ||
         file.extension() == "xml"   ||
         file.extension() == "XML" )
    {
        return( true );
    }

    return( false );
}

/*===========================================================================*/
/**
 *  @brief  Check the file format.
 *  @param  filename [in] filename
 *  @return true, if the KVSMLObjectTable class can read the given file
 */
/*===========================================================================*/
const bool KVSMLObjectTable::CheckFileFormat( const std::string& filename )
{
    kvs::XMLDocument document;
    if ( !document.read( filename ) ) return( false );

    // <KVSML>
    kvs::kvsml::KVSMLTag kvsml_tag;
    if ( !kvsml_tag.read( &document ) ) return( false );

    // <Object>
    kvs::kvsml::ObjectTag object_tag;
    if ( !object_tag.read( kvsml_tag.node() ) ) return( false );

    if ( object_tag.type() != "TableObject" ) return( false );

    // <TableObject>
    kvs::kvsml::TableObjectTag table_tag;
    if ( !table_tag.read( object_tag.node() ) ) return( false );

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Output operator.
 *  @param  os [out] output stream
 *  @param  rhs [in] KVSML table object
 */
/*===========================================================================*/
std::ostream& operator <<( std::ostream& os, const KVSMLObjectTable& rhs )
{
    os << "Num. of rows: " << rhs.m_nrows << std::endl;
    os << "Num. of columns: " << rhs.m_ncolumns << std::endl;

    for ( size_t i = 0; i < rhs.m_ncolumns; i++ )
    {
        os << "Column[" << i << "] label: " << rhs.m_labels.at(i);
        if ( i < rhs.m_ncolumns - 1 ) os << std::endl;
    }

    return( os );
}

} // end of namespace kvs
