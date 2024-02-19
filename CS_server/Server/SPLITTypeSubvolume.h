/****************************************************************************/
/**
 *  @file SPLITTypeSubvolume.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: SPLITTypeSubvolume.h 848 2011-06-29 11:35:52Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef PBVR__SPLIT_TYPE_SUBVOLUME_H_INCLUDE
#define PBVR__SPLIT_TYPE_SUBVOLUME_H_INCLUDE

#include <kvs/FileFormatBase>
#include <kvs/AnyValueArray>
#include <kvs/ValueArray>
#include <kvs/Type>
#include <kvs/Vector3>
#include <string>
#include <Core/FileFormat/KVSML/KVSMLTag.h>
#include <Core/FileFormat/KVSML/ObjectTag.h>


namespace pbvr
{

/*===========================================================================*/
/**
 *  @brief  KVSML unstructured volume object format.
 */
/*===========================================================================*/
class SPLITTypeSubvolume : public kvs::FileFormatBase
{
    kvsClassName( pbvr::SPLITTypeSubvolume );

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
    size_t                       m_vector_length; ///< vector length
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

    SPLITTypeSubvolume();

    SPLITTypeSubvolume( const std::string& filename );

    virtual ~SPLITTypeSubvolume();

public:

    const kvs::kvsml::KVSMLTag& KVSMLTag() const;

    const kvs::kvsml::ObjectTag& objectTag() const;

public:

    const std::string& cellType() const;

    const bool hasLabel() const;

    const std::string& label() const;

    const size_t veclen() const;

    const size_t nnodes() const;

    const size_t ncells() const;

    const bool hasMinValue() const;

    const bool hasMaxValue() const;

    const double minValue() const;

    const double maxValue() const;

    const kvs::AnyValueArray& values() const;

    const kvs::ValueArray<kvs::Real32>& coords() const;

    const kvs::ValueArray<kvs::UInt32>& connections() const;

public:

    void setWritingDataType( const WritingDataType& writing_type );

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

    friend std::ostream& operator <<( std::ostream& os, const SPLITTypeSubvolume& rhs );
};

} // end of namespace pbvr

#endif // KVS__KVSML_OBJECT_UNSTRUCTURED_VOLUME_H_INCLUDE
