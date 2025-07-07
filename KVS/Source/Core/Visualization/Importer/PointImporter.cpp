/****************************************************************************/
/**
 *  @file   PointImporter.cpp
 *  @author Naohisa Sakamoto
 */
/****************************************************************************/
#include "PointImporter.h"
#include <kvs/DebugNew>
#include <kvs/KVSMLPointObject>
#include <kvs/Math>
#include <kvs/Vector3>
#include <string>


namespace kvs
{

/*==========================================================================*/
/**
 *  @brief  Constructs a new PointImporter class.
 */
/*==========================================================================*/
PointImporter::PointImporter()
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new PointImporter class.
 *  @param  filename [in] input filename
 */
/*===========================================================================*/
PointImporter::PointImporter( const std::string& filename )
{
    if ( kvs::KVSMLPointObject::CheckExtension( filename ) )
    {
        BaseClass::setSuccess( SuperClass::read( filename ) );
    }
    else if ( kvs::LAS::CheckExtension( filename ) )
    {
        kvs::LAS* file_format = new kvs::LAS( filename );
        if ( !file_format )
        {
            BaseClass::setSuccess( false );
            kvsMessageError("Cannot read '%s'.",filename.c_str());
            return;
        }

        if ( file_format->isFailure() )
        {
            BaseClass::setSuccess( false );
            kvsMessageError("Cannot read '%s'.",filename.c_str());
            delete file_format;
            return;
        }

        this->import( file_format );
        delete file_format;
    }
    else if ( kvs::PTS::CheckExtension( filename ) )
    {
        kvs::PTS* file_format = new kvs::PTS( filename );
        if ( !file_format )
        {
            BaseClass::setSuccess( false );
            kvsMessageError("Cannot read '%s'.",filename.c_str());
            return;
        }

        if ( file_format->isFailure() )
        {
            BaseClass::setSuccess( false );
            kvsMessageError("Cannot read '%s'.",filename.c_str());
            delete file_format;
            return;
        }

        this->import( file_format );
        delete file_format;
    }
    else
    {
        BaseClass::setSuccess( false );
        kvsMessageError("Cannot import '%'.",filename.c_str());
        return;
    }
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new PointImporter class.
 *  @param file_format [in] pointer to the file format
 */
/*==========================================================================*/
PointImporter::PointImporter( const kvs::FileFormatBase* file_format )
{
    this->exec( file_format );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the PointImporter class.
 */
/*===========================================================================*/
PointImporter::~PointImporter()
{
}

/*===========================================================================*/
/**
 *  @brief  Executes the import process.
 *  @param  file_format [in] pointer to the file format class
 *  @return pointer to the imported point object
 */
/*===========================================================================*/
PointImporter::SuperClass* PointImporter::exec( const kvs::FileFormatBase* file_format )
{
    if ( !file_format )
    {
        BaseClass::setSuccess( false );
        kvsMessageError("Input file format is NULL.");
        return NULL;
    }

    if ( dynamic_cast<const kvs::KVSMLPointObject*>( file_format ) )
    {
        BaseClass::setSuccess( SuperClass::read( file_format->filename() ) );
    }
    else if ( const kvs::LAS* las = dynamic_cast<const kvs::LAS*>( file_format ) )
    {
        this->import( las );
    }
    else if ( const kvs::PTS* pts = dynamic_cast<const kvs::PTS*>( file_format ) )
    {
        this->import( pts );
    }
    else
    {
        BaseClass::setSuccess( false );
        kvsMessageError("Input file format is not supported.");
        return NULL;
    }

    return this;
}

/*==========================================================================*/
/**
 *  @brief  Imports the LAS format data.
 *  @param  las [in] pointer to the LAS format file
 */
/*==========================================================================*/
void kvs::PointImporter::import( const kvs::LAS* las )
{
    SuperClass::setCoords( las->coords() );
    SuperClass::setColors( las->colors() );
    SuperClass::updateMinMaxCoords();
}

/*==========================================================================*/
/**
 *  @brief  Imports the PTS format data.
 *  @param  las [in] pointer to the PTS format file
 */
/*==========================================================================*/
void kvs::PointImporter::import( const kvs::PTS* pts )
{
    SuperClass::setCoords( pts->coords() );
    SuperClass::setColors( pts->colors() );
    SuperClass::updateMinMaxCoords();
}

} // end of namespace kvs
