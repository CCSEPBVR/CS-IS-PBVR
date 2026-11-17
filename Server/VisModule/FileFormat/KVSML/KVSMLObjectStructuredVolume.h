/****************************************************************************/
/**
 *  @file KVSMLObjectStructuredVolume.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KVSMLObjectStructuredVolume.h 867 2011-07-24 06:12:29Z naohisa.sakamoto@gmail.com $
 */
/****************************************************************************/
#ifndef VIS_MODULE__KVSML_OBJECT_STRUCTURED_VOLUME_H_INCLUDE
#define VIS_MODULE__KVSML_OBJECT_STRUCTURED_VOLUME_H_INCLUDE

#include <vismodule/FileFormatBase>
#include <vismodule/AnyValueArray>
#include <vismodule/ValueArray>
#include <vismodule/Type>
#include <vismodule/Vector3>
#include <string>
#include "KVSMLTag.h"
#include "ObjectTag.h"


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  KVSML structured volume object format.
 */
/*===========================================================================*/
class KVSMLObjectStructuredVolume : public vismodule::FileFormatBase
{
    visModuleClassName( vismodule::KVSMLObjectStructuredVolume );

public:

    enum WritingDataType
    {
        Ascii = 0,
        ExternalAscii,
        ExternalBinary
    };

protected:

    vismodule::kvsml::KVSMLTag m_kvsml_tag;   ///< KVSML tag information
    vismodule::kvsml::ObjectTag m_object_tag; ///< Object tag information

    WritingDataType    m_writing_type;  ///< writing data type
    std::string        m_grid_type;     ///< grid type
    bool               m_has_label;     ///< data label is specified or not
    std::string        m_label;         ///< data label
    std::size_t             m_veclen;        ///< vector length
    vismodule::Vector3ui     m_resolution;    ///< grid resolution
    bool               m_has_min_value; ///< min. value is specified or not
    bool               m_has_max_value; ///< max. value is specified or not
    double             m_min_value;     ///< min. value
    double             m_max_value;     ///< max. value
    vismodule::AnyValueArray m_values;        ///< field value array
    vismodule::ValueArray<float> m_coords;    ///< coordinate array

public:

    KVSMLObjectStructuredVolume( void );

    KVSMLObjectStructuredVolume( const std::string& filename );

    virtual ~KVSMLObjectStructuredVolume( void );

public:

    const vismodule::kvsml::KVSMLTag& KVSMLTag( void ) const;

    const vismodule::kvsml::ObjectTag& objectTag( void ) const;

public:

    const std::string& gridType( void ) const;

    const bool hasLabel( void ) const;

    const std::string& label( void ) const;

    const std::size_t veclen( void ) const;

    const vismodule::Vector3ui& resolution( void ) const;

    const bool hasMinValue( void ) const;

    const bool hasMaxValue( void ) const;

    const double minValue( void ) const;

    const double maxValue( void ) const;

    const vismodule::AnyValueArray& values( void ) const;

    const vismodule::ValueArray<float>& coords( void ) const;

public:

    void setWritingDataType( const WritingDataType writing_type );

    void setGridType( const std::string& grid_type );

    void setLabel( const std::string& label );

    void setVeclen( const std::size_t veclen );

    void setResolution( const vismodule::Vector3ui& resolution );

    void setMinValue( const double min_value );

    void setMaxValue( const double max_value );

    void setValues( const vismodule::AnyValueArray& values );

    void setCoords( const vismodule::ValueArray<float>& coords );

public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );

public:

    static const bool CheckFileExtension( const std::string& filename );

    static const bool CheckFileFormat( const std::string& filename );

    friend std::ostream& operator <<( std::ostream& os, const KVSMLObjectStructuredVolume& rhs );
};

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML_OBJECT_STRUCTURED_VOLUME_H_INCLUDE
