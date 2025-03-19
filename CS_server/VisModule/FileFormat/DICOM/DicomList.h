/****************************************************************************/
/**
 *  @file DicomList.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: DicomList.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__DICOM_LIST_H_INCLUDE
#define VIS_MODULE__DICOM_LIST_H_INCLUDE

#include <algorithm>
#include <vector>
#include <string>
#include <vismodule/Vector2>
#include <vismodule/FileFormatBase>
#include <vismodule/ClassName>
#include <vismodule/Dicom>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  DICOM list class.
 */
/*===========================================================================*/
class DicomList : public vismodule::FileFormatBase
{
    visModuleClassName( vismodule::DicomList );

public:

    struct SortingByImageNumber
    {
        bool operator () ( const vismodule::Dicom* dicom1, const vismodule::Dicom* dicom2 );
    };

    struct SortingBySeriesNumber
    {
        bool operator () ( const vismodule::Dicom* dicom1, const vismodule::Dicom* dicom2 );
    };

    struct SortingBySliceLocation
    {
        bool operator () ( const vismodule::Dicom* dicom1, const vismodule::Dicom* dicom2 );
    };

protected:

    std::vector<vismodule::Dicom*> m_list;            ///< DICOM list
    size_t                   m_row;             ///< row
    size_t                   m_column;          ///< column
    double                   m_slice_thickness; ///< slice thickness
    double                   m_slice_spacing;   ///< slice spacing
    vismodule::Vector2f            m_pixel_spacing;   ///< pixel spacing
    int                      m_min_raw_value;   ///< min. value of the raw data
    int                      m_max_raw_value;   ///< max. value of the raw data
    bool                     m_extension_check; ///< check the file extension

public:

    DicomList( void );

    DicomList( const std::string& dirname, const bool extension_check = true );

    virtual ~DicomList( void );

public:

    const vismodule::Dicom* operator [] ( const size_t index ) const;

    vismodule::Dicom* operator [] ( const size_t index );

public:

    void push_back( const vismodule::Dicom& dicom );

    void push_back( vismodule::Dicom* dicom );

    const size_t size( void ) const;

    void clear( void );

public:

    const size_t row( void ) const;

    const size_t column( void ) const;

    const size_t nslices( void ) const;

    const size_t width( void ) const;

    const size_t height( void ) const;

    const double sliceThickness( void ) const;

    const double sliceSpacing( void ) const;

    const vismodule::Vector2f& pixelSpacing( void ) const;

    const int minRawValue( void ) const;

    const int maxRawValue( void ) const;

    void enableExtensionCheck( void );

    void disableExtensionCheck( void );

public:

    const bool read( const std::string& dirname );

    const bool write( const std::string& dirname );

    void sort( void )
    {
        std::sort( m_list.begin(), m_list.end(), SortingBySliceLocation() );
    }

    template <typename CompareMethod>
    void sort( void )
    {
        std::sort( m_list.begin(), m_list.end(), CompareMethod() );
    }

public:

    static const bool CheckDirectory( const std::string& dirname, const bool extension_check = true );
};

} // end of namespace vismodule

#endif // VIS_MODULE__DICOM_LIST_H_INCLUDE
