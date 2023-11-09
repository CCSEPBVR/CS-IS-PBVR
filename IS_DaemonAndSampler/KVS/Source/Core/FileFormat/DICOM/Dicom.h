/****************************************************************************/
/**
 *  @file Dicom.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Dicom.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__DICOM_H_INCLUDE
#define KVS__DICOM_H_INCLUDE

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <algorithm>
#include <kvs/FileFormatBase>
#include <kvs/ValueArray>
#include <kvs/Type>
#include <kvs/Vector2>
#include <kvs/ClassName>
#include "Attribute.h"
#include "Window.h"
#include "Element.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  DICOM class.
 */
/*===========================================================================*/
class Dicom : public kvs::FileFormatBase
{
    kvsClassName( kvs::Dicom );

public:

    typedef dcm::Element            Element;
    typedef std::list<dcm::Element> ElementList;

protected:

    // DICOM attribute
    dcm::Attribute          m_attribute;            ///< attribute

    // DICOM header information
    std::list<dcm::Element> m_element_list;         ///< element list

    std::string             m_modality;             ///< (0008,0060) modality
    std::string             m_manufacturer;         ///< (0008,0070) manufacturer

    double                  m_slice_thickness;      ///< (0018,0050) slice thickness
    double                  m_slice_spacing;        ///< (0018,0088) spacing between slices

    int                     m_series_number;        ///< (0020,0011) series number
    int                     m_image_number;         ///< (0020,0013) image number
    double                  m_slice_location;       ///< (0020,1041) slice location

    unsigned short          m_row;                  ///< (0028,0010) rows (height)
    unsigned short          m_column;               ///< (0028,0011) columns (width)
    kvs::Vector2f           m_pixel_spacing;        ///< (0028,0030) pixel spacing
    unsigned short          m_bits_allocated;       ///< (0028,0100) bits allocated
    unsigned short          m_bits_stored;          ///< (0028,0101) bits stored
    unsigned short          m_high_bit;             ///< (0028,0102) high bit
    bool                    m_pixel_representation; ///< (0028,0103) pixel representation
    int                     m_window_level;         ///< (0028,1050) window center
    int                     m_window_width;         ///< (0028,1051) window width
    double                  m_rescale_intersept;    ///< (0028,1052) rescale intersept
    double                  m_rescale_slope;        ///< (0028,1053) rescale slope

    // Parameters in this class
    dcm::Window                 m_window;           ///< window
    int                         m_min_raw_value;    ///< min. value of the raw data
    int                         m_max_raw_value;    ///< max. value of the raw data
    std::ios::pos_type          m_position;         ///< raw data position
    kvs::ValueArray<char>       m_raw_data;         ///< raw data

public:

    Dicom( void );

    Dicom( const std::string& filename );

    Dicom( const Dicom& dicom );

    virtual ~Dicom( void );

public:

    void initialize( void );

    void clear( void );

public:

    Dicom& operator = ( const Dicom& d );

    friend std::ostream& operator << ( std::ostream& os, const Dicom& d );

public:

    const unsigned short width( void ) const;

    const unsigned short height( void ) const;

    const int seriesNumber( void ) const;

    const int imageNumber( void ) const;

    const double sliceLocation( void ) const;

    const unsigned short row( void ) const;

    const unsigned short column( void ) const;

    const unsigned int size( void ) const;

    const unsigned short bitsAllocated( void ) const;

    const unsigned short bytesAllocated( void ) const;

    const unsigned short bitsStored( void ) const;

    const unsigned short bytesStored( void ) const;

    const unsigned short highBit( void ) const;

    const bool pixelRepresentation( void ) const;

    const int windowLevel( void ) const;

    const int windowWidth( void ) const;

    const int minWindowValue( void ) const;

    const int maxWindowValue( void ) const;

    const int minRawValue( void ) const;

    const int maxRawValue( void ) const;

    const int minValue( void ) const;

    const int maxValue( void ) const;

    const double rescaleIntersept( void ) const;

    const double rescaleSlope( void ) const;

    const dcm::Attribute& attribute( void ) const;

    const std::list<dcm::Element>& elementList( void ) const;

    const std::string& modality( void ) const;

    const kvs::Vector2f& pixelSpacing( void ) const;

    const std::string& manufacturer( void ) const;

    const double sliceThickness( void ) const;

    const double sliceSpacing( void ) const;

    const dcm::Window& window( void ) const;

    const std::ios::pos_type& position( void ) const;

    const kvs::ValueArray<char>& rawData( void ) const;

    const kvs::ValueArray<kvs::UInt8> pixelData( void ) const;

    const int rawValue( const size_t index ) const;

    const int rawValue( const size_t i, const size_t j ) const;

    const int value( const size_t index ) const;

    const int value( const size_t x, const size_t y ) const;

public:

    void setRawData( const kvs::ValueArray<char>& raw_data );

    void changeWindow( const int level, const int width );

    void resetWindow( void );

    std::list<dcm::Element>::iterator findElement( const dcm::Tag tag );

public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );

private:

    const bool read_header( std::ifstream& ifs );

    const bool read_data( std::ifstream& ifs );

    const bool write_header( std::ofstream& ofs );

    const bool write_header_csv( std::ofstream& ofs );

    const bool write_raw_data( std::ofstream& ofs );

    void set_windowing_parameter( void );

    void set_min_max_window_value( void );

    template <typename T>
    void calculate_min_max_raw_value( void );

    void set_min_max_raw_value( void );

    template <typename T>
    const kvs::ValueArray<kvs::UInt8> rescale_pixel_data( const int level, const int width ) const;

    const kvs::ValueArray<kvs::UInt8> get_pixel_data( const int level, const int width ) const;

    void parse_element( dcm::Element& element );

public:

    static const bool CheckFileExtension( const std::string& filename );

    static const bool CheckFileFormat( const std::string& filename );
};

} // end of namespace kvs

#endif // KVS__DICOM_H_INCLUDE
