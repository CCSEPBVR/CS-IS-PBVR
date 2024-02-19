/****************************************************************************/
/**
 *  @file KVSMLObjectUnstructuredVolume.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KVSMLObjectUnstructuredVolume.h 848 2011-06-29 11:35:52Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__KVSML_OBJECT_UNSTRUCTURED_VOLUME_H_INCLUDE
#define KVS__KVSML_OBJECT_UNSTRUCTURED_VOLUME_H_INCLUDE

#include <kvs/FileFormatBase>
#include <kvs/AnyValueArray>
#include <kvs/ValueArray>
#include <kvs/Type>
#include <kvs/Vector3>
#include <string>
#include "KVSMLTag.h"
#include "ObjectTag.h"


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  KVSML unstructured volume object format.
 */
/*===========================================================================*/
class KVSMLObjectUnstructuredVolume : public kvs::FileFormatBase
{
    kvsClassName( kvs::KVSMLObjectUnstructuredVolume );

public:

    enum WritingDataType
    {
        Ascii = 0,
        ExternalAscii,
        ExternalBinary
    };

protected:

    kvs::kvsml::KVSMLTag m_kvsml_tag;   ///< KVSML tag information
    kvs::kvsml::ObjectTag m_object_tag; ///< Object tag information

    WritingDataType              m_writing_type;  ///< writing data type
    std::string                  m_cell_type;     ///< cell type
    bool                         m_has_label;     ///< data label is specified or not
    std::string                  m_label;         ///< data label
    size_t                       m_veclen;        ///< vector length
    size_t                       m_nnodes;        ///< number of nodes
    size_t                       m_ncells;        ///< number of cells
    bool                         m_has_min_value; ///< min. value is specified or not
    bool                         m_has_max_value; ///< max. value is specified or not
    double                       m_min_value;     ///< min. value
    double                       m_max_value;     ///< max. value
    kvs::AnyValueArray           m_values;        ///< field value array
    kvs::ValueArray<kvs::Real32> m_coords;        ///< coordinate value array
    kvs::ValueArray<kvs::UInt32> m_connections;   ///< connection id array

public:

    KVSMLObjectUnstructuredVolume( void );

    KVSMLObjectUnstructuredVolume( const std::string& filename );

    virtual ~KVSMLObjectUnstructuredVolume( void );

public:

    const kvs::kvsml::KVSMLTag& KVSMLTag( void ) const;

    const kvs::kvsml::ObjectTag& objectTag( void ) const;

public:

    const std::string& cellType( void ) const;

    const bool hasLabel( void ) const;

    const std::string& label( void ) const;

    const size_t veclen( void ) const;

    const size_t nnodes( void ) const;

    const size_t ncells( void ) const;

    const bool hasMinValue( void ) const;

    const bool hasMaxValue( void ) const;

    const double minValue( void ) const;

    const double maxValue( void ) const;

    const kvs::AnyValueArray& values( void ) const;

    const kvs::ValueArray<kvs::Real32>& coords( void ) const;

    const kvs::ValueArray<kvs::UInt32>& connections( void ) const;

public:

    void setWritingDataType( const WritingDataType writing_type );

    void setCellType( const std::string& cell_type );

    void setLabel( const std::string& label );

    void setVeclen( const size_t veclen );

    void setNNodes( const size_t nnodes );

    void setNCells( const size_t ncells );

    void setMinValue( const double min_value );

    void setMaxValue( const double max_value );

    void setValues( const kvs::AnyValueArray& values );

    void setCoords( const kvs::ValueArray<kvs::Real32>& coords );

    void setConnections( const kvs::ValueArray<kvs::UInt32>& connections );

public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );

public:

    static const bool CheckFileExtension( const std::string& filename );

    static const bool CheckFileFormat( const std::string& filename );

    friend std::ostream& operator <<( std::ostream& os, const KVSMLObjectUnstructuredVolume& rhs );
};

} // end of namespace kvs

#endif // KVS__KVSML_OBJECT_UNSTRUCTURED_VOLUME_H_INCLUDE
