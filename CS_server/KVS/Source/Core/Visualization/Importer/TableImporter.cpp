/*****************************************************************************/
/**
 *  @file   TableImporter.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TableImporter.cpp 876 2011-07-30 15:43:18Z naohisa.sakamoto@gmail.com $
 */
/*****************************************************************************/
#include "TableImporter.h"
#include <kvs/DebugNew>
#include <kvs/KVSMLObjectTable>
#include <string>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new TableImporter class.
 */
/*===========================================================================*/
TableImporter::TableImporter( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new TableImporter class.
 *  @param  filename [in] filename
 */
/*===========================================================================*/
TableImporter::TableImporter( const std::string& filename )
{
    if ( kvs::KVSMLObjectTable::CheckFileExtension( filename ) )
    {
        kvs::KVSMLObjectTable* file_format = new kvs::KVSMLObjectTable( filename );
        if( !file_format )
        {
            BaseClass::m_is_success = false;
            kvsMessageError("Cannot read '%s'.",filename.c_str());
            return;
        }

        if( file_format->isFailure() )
        {
            BaseClass::m_is_success = false;
            kvsMessageError("Cannot read '%s'.",filename.c_str());
            delete file_format;
            return;
        }

        this->import( file_format );
        delete file_format;
    }
    else
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Cannot import '%s'.",filename.c_str());
        return;
    }
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new TableImporter class.
 *  @param  file_format [in] pointer to the file format
 */
/*===========================================================================*/
TableImporter::TableImporter( const kvs::FileFormatBase* file_format )
{
    this->exec( file_format );
}

/*===========================================================================*/
/**
 *  @brief  Returns pointer to the imported table object.
 *  @param  file_format [in] pointer to the file format
 *  @return pointer to the imported table object
 */
/*===========================================================================*/
TableImporter::SuperClass* TableImporter::exec( const kvs::FileFormatBase* file_format )
{
    if ( !file_format )
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Input file format is NULL.");
        return( NULL );
    }

    const std::string class_name = file_format->className();
    if ( class_name == "kvs::KVSMLObjectTable" )
    {
        this->import( static_cast<const kvs::KVSMLObjectTable*>( file_format ) );
    }
    else
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Input file format is not supported.");
        return( NULL );
    }

    return( this );
}

/*===========================================================================*/
/**
 *  @brief  Imports table data from KVSML format file.
 *  @param  kvsml [in] pointer to the KVSMLObjectTable
 */
/*===========================================================================*/
void TableImporter::import( const kvs::KVSMLObjectTable* kvsml )
{
    const size_t ncolumns = kvsml->ncolumns();
    for ( size_t i = 0; i < ncolumns; i++ )
    {
        const std::string label = kvsml->labelList().at(i);
        const kvs::AnyValueArray& column = kvsml->columnList().at(i);
        SuperClass::addColumn( column, label );

        if ( kvsml->hasMinValueList().at(i) ) SuperClass::setMinValue( i, kvsml->minValueList().at(i) );
        if ( kvsml->hasMaxValueList().at(i) ) SuperClass::setMaxValue( i, kvsml->maxValueList().at(i) );
        if ( kvsml->hasMinRangeList().at(i) ) SuperClass::setMinRange( i, kvsml->minRangeList().at(i) );
        if ( kvsml->hasMaxRangeList().at(i) ) SuperClass::setMaxRange( i, kvsml->maxRangeList().at(i) );
    }
}

} // end of namespace kvs
