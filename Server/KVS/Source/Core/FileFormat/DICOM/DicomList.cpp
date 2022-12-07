/*****************************************************************************/
/**
 *  @file   dicom_list.cpp
 *  @brief  DICOM list class.
 *
 *  @author Naohisa Sakamoto
 *  @date   2010/10/15 12:49:34
 */
/*----------------------------------------------------------------------------
 *
 *  $Author: naohisa $
 *  $Date: 2008/07/29 07:03:56 $
 *  $Source: /home/Repository/viz-server2/cvsroot/KVS_RC1/Source/Core/FileFormat/DICOM/DicomList.cpp,v $
 *  $Revision: 1.1 $
 */
/*****************************************************************************/
#include "DicomList.h"
#include <kvs/DebugNew>
#include <kvs/Dicom>
#include <kvs/File>
#include <kvs/Directory>
#include <kvs/Message>
#include <kvs/Math>
#include <kvs/IgnoreUnusedVariable>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Compare method for sorting by image number.
 */
/*===========================================================================*/
bool DicomList::SortingByImageNumber::operator () (
    const kvs::Dicom* dicom1,
    const kvs::Dicom* dicom2 )
{
    return( dicom1->imageNumber() < dicom2->imageNumber() );
}

/*===========================================================================*/
/**
 *  @brief  Compare method for sorting by series number.
 */
/*===========================================================================*/
bool DicomList::SortingBySeriesNumber::operator () (
    const kvs::Dicom* dicom1,
    const kvs::Dicom* dicom2 )
{
    return( dicom1->seriesNumber() < dicom2->seriesNumber() );
}

/*===========================================================================*/
/**
 *  @brief  Compare method for sorting by slice location.
 */
/*===========================================================================*/
bool DicomList::SortingBySliceLocation::operator () (
    const kvs::Dicom* dicom1,
    const kvs::Dicom* dicom2 )
{
    return( dicom1->sliceLocation() < dicom2->sliceLocation() );
}

/*===========================================================================*/
/**
 *  @brief  Constructor.
 */
/*===========================================================================*/
DicomList::DicomList( void ):
    m_row( 0 ),
    m_column( 0 ),
    m_slice_thickness( 0.0 ),
    m_min_raw_value( 0 ),
    m_max_raw_value( 0 ),
    m_extension_check( true )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructor.
 *  @param  dirname         [in] directory name
 *  @param  extension_check [in] file extension check flag
 */
/*===========================================================================*/
DicomList::DicomList( const std::string& dirname, const bool extension_check ):
    m_row( 0 ),
    m_column( 0 ),
    m_slice_thickness( 0.0 ),
    m_min_raw_value( 0 ),
    m_max_raw_value( 0 ),
    m_extension_check( extension_check )
{
    this->read( dirname );
    this->sort(); // Sorting by slice location. (default sorting method)
}

/*===========================================================================*/
/**
 *  @brief  Destructor.
 */
/*===========================================================================*/
DicomList::~DicomList( void )
{
    this->clear();
}

/*===========================================================================*/
/**
 *  @brief  [] operator.
 *  @param  index [in] list index
 */
/*===========================================================================*/
const kvs::Dicom* DicomList::operator [] ( const size_t index ) const
{
    return( m_list[index] );
}

/*===========================================================================*/
/**
 *  @brief  [] operator.
 *  @param  index [in] list index
 */
/*===========================================================================*/
kvs::Dicom* DicomList::operator [] ( const size_t index )
{
    return( m_list[index] );
}

/*===========================================================================*/
/**
 *  @brief  Push back method.
 *  @param  dicom [in] DICOM data
 */
/*===========================================================================*/
void DicomList::push_back( const kvs::Dicom& dicom )
{
    this->push_back( new kvs::Dicom( dicom ) );
}

/*===========================================================================*/
/**
 *  @brief  Push back method.
 *  @param  dicom [in] pointer to DICOM data.
 */
/*===========================================================================*/
void DicomList::push_back( kvs::Dicom* dicom )
{
    if( m_list.size() == 0 )
    {
        m_row             = dicom->row();
        m_column          = dicom->column();
        m_slice_thickness = dicom->sliceThickness();
        m_slice_spacing   = dicom->sliceSpacing();
        m_pixel_spacing   = dicom->pixelSpacing();
        m_min_raw_value   = dicom->minRawValue();
        m_max_raw_value   = dicom->maxRawValue();
    }
    else
    {
        if( m_row != dicom->row() || m_column != dicom->column() )
        {
            kvsMessageError("Not correspond image size.");
            return;
        }

        m_min_raw_value = kvs::Math::Min( m_min_raw_value, dicom->minRawValue() );
        m_max_raw_value = kvs::Math::Max( m_max_raw_value, dicom->maxRawValue() );
    }

    m_list.push_back( dicom );
}

/*===========================================================================*/
/**
 *  @brief  Return size of list.
 *  @return size of list
 */
/*===========================================================================*/
const size_t DicomList::size( void ) const
{
    return( m_list.size() );
}

/*===========================================================================*/
/**
 *  @brief  Clear.
 */
/*===========================================================================*/
void DicomList::clear( void )
{
    const size_t nslices = m_list.size();
    for( size_t i = 0; i < nslices; i++ )
    {
        if( m_list[i] ) delete m_list[i];
    }

    m_list.clear();
}

/*===========================================================================*/
/**
 *  @brief  Return the row size of the DICOM image.
 */
/*===========================================================================*/
const size_t DicomList::row( void ) const
{
    return( m_row );
}

/*===========================================================================*/
/**
 *  @brief  Return the column size of the DICOM image.
 */
/*===========================================================================*/
const size_t DicomList::column( void ) const
{
    return( m_column );
}

/*===========================================================================*/
/**
 *  @brief  Get number of slices.
 */
/*===========================================================================*/
const size_t DicomList::nslices( void ) const
{
    return( m_list.size() );
}

/*===========================================================================*/
/**
 *  @brief  Return the DICOM image width.
 */
/*===========================================================================*/
const size_t DicomList::width( void ) const
{
    return( m_column );
}

/*===========================================================================*/
/**
 *  @brief  Return the DICOM image height.
 */
/*===========================================================================*/
const size_t DicomList::height( void ) const
{
    return( m_row );
}

/*===========================================================================*/
/**
 *  @brief  Return the slice thickness.
 */
/*===========================================================================*/
const double DicomList::sliceThickness( void ) const
{
    return( m_slice_thickness );
}

/*===========================================================================*/
/**
 *  @brief  Return the slice spacing.
 */
/*===========================================================================*/
const double DicomList::sliceSpacing( void ) const
{
    return( m_slice_spacing );
}

/*===========================================================================*/
/**
 *  @brief  Return the pixel spacing.
 */
/*===========================================================================*/
const kvs::Vector2f& DicomList::pixelSpacing( void ) const
{
    return( m_pixel_spacing );
}

/*===========================================================================*/
/**
 *  @brief  Return the minimum raw value.
 *  @return minimum raw value
 */
/*===========================================================================*/
const int DicomList::minRawValue( void ) const
{
    return( m_min_raw_value );
}

/*===========================================================================*/
/**
 *  @brief  Return the maximum raw value.
 *  @return maximum raw value
 */
/*===========================================================================*/
const int DicomList::maxRawValue( void ) const
{
    return( m_max_raw_value );
}

/*===========================================================================*/
/**
 *  @brief  Enable extension check.
 */
/*===========================================================================*/
void DicomList::enableExtensionCheck( void )
{
    m_extension_check = true;
}

/*===========================================================================*/
/**
 *  @brief  Disable extension check.
 */
/*===========================================================================*/
void DicomList::disableExtensionCheck( void )
{
    m_extension_check = false;
}

/*===========================================================================*/
/**
 *  @brief  Read DICOM set from directory.
 *  @param  dirname [in] directory name
 */
/*===========================================================================*/
const bool DicomList::read( const std::string& dirname )
{
    kvs::Directory dir( dirname );
    if( !dir.isExisted() )
    {
        kvsMessageError( "%s is not existed.", dir.directoryPath().c_str() );
        return( false );
    }

    if( !dir.isDirectory() )
    {
        kvsMessageError( "%s is not directory.", dir.directoryPath().c_str() );
        return( false );
    }

    if( dir.fileList().size() == 0 )
    {
        kvsMessageError( "File not found in %s.", dir.directoryPath().c_str() );
        return( false );
    }

    // Read DICOM data file. (".dcm" only, if extension_check is true)
    bool flag = false;

    kvs::FileList::const_iterator file = dir.fileList().begin();
    kvs::FileList::const_iterator last = dir.fileList().end();
    while ( file != last )
    {
        if( m_extension_check )
        {
            if( file->extension() != "dcm" ) continue;
        }

        kvs::Dicom* dicom = new kvs::Dicom( file->filePath( true ) );
        if( !flag )
        {
            m_row             = dicom->row();
            m_column          = dicom->column();
            m_slice_thickness = dicom->sliceThickness();
            m_slice_spacing   = dicom->sliceSpacing();
            m_pixel_spacing   = dicom->pixelSpacing();
            m_min_raw_value   = dicom->minRawValue();
            m_max_raw_value   = dicom->maxRawValue();
            flag = true;
        }
        else
        {
            if( m_row != dicom->row() || m_column != dicom->column() )
            {
                kvsMessageError( "Not correspond image size (%s).", file->filePath().c_str() );
                continue;
            }

            m_min_raw_value = kvs::Math::Min( m_min_raw_value, dicom->minRawValue() );
            m_max_raw_value = kvs::Math::Max( m_max_raw_value, dicom->maxRawValue() );
        }

        m_list.push_back( dicom );

        ++file;
    }

    m_is_success = true;

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Write DICOM files. (Not implemented)
 *  @param  dirname [in] directory name
 *  @return true, if the writting process was done successfully
 */
/*===========================================================================*/
const bool DicomList::write( const std::string& dirname )
{
    kvs::IgnoreUnusedVariable( dirname );

    kvsMessageError("Writing method has not been implemented.");
    return( false );
}

const bool DicomList::CheckDirectory( const std::string& dirname, const bool extension_check )
{
    kvs::Directory dir( dirname );
    if( !dir.isExisted() )
    {
        kvsMessageError( "%s is not existed.", dir.directoryPath().c_str() );
        return( false );
    }

    if( !dir.isDirectory() )
    {
        kvsMessageError( "%s is not directory.", dir.directoryPath().c_str() );
        return( false );
    }

    if( dir.fileList().size() == 0 )
    {
        kvsMessageError( "File not found in %s.", dir.directoryPath().c_str() );
        return( false );
    }

    size_t counter = 0;
    kvs::FileList::const_iterator file = dir.fileList().begin();
    kvs::FileList::const_iterator last = dir.fileList().end();
    while ( file != last )
    {
        if( extension_check )
        {
            if( file->extension() != "dcm" ) continue;
            else counter++;
        }

        ++file;
    }

    if ( extension_check )
    {
        if ( counter == 0 )
        {
            kvsMessageError( "File not found in %s.", dir.directoryPath().c_str() );
            return( false );
        }
    }

    return( true );
}

} // end of namespace kvs
