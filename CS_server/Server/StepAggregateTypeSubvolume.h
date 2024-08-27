/****************************************************************************/
/**
 *  @file StepAggregateTypeSubvolume.h
 */
/****************************************************************************/
#ifndef PBVR__KVS__STEP_AGGREGATE_TYPE_SUBVOLUME_H_INCLUDE
#define PBVR__KVS__STEP_AGGREGATE_TYPE_SUBVOLUME_H_INCLUDE

#include <kvs/FileFormatBase>
#include <kvs/AnyValueArray>
#include <kvs/ValueArray>
#include <kvs/Type>
#include <kvs/Vector3>
#include <string>
#include <Core/FileFormat/KVSML/ObjectTag.h>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  GTimestep unstructured volume object format.
 */
/*===========================================================================*/
class StepAggregateTypeSubvolume : public kvs::FileFormatBase
{
    kvsClassName( kvs::StepAggregateTypeSubvolume );

public:

    enum WritingDataType
    {
        Ascii = 0,
        ExternalAscii,
        ExternalBinary
    };

protected:

    std::string                  m_cell_type;     ///< cell type
    size_t                       m_vector_length; ///< vector length
    size_t                       m_nnodes;        ///< number of nodes
    size_t                       m_ncells;        ///< number of cells
    kvs::AnyValueArray           m_values;        ///< field value array
    kvs::ValueArray<kvs::Real32> m_coords;        ///< coordinate value array
    kvs::ValueArray<kvs::UInt32> m_connections;   ///< connection id array

public:

    StepAggregateTypeSubvolume();

    StepAggregateTypeSubvolume( const std::string& filename, const int st, const int vl );

    virtual ~StepAggregateTypeSubvolume();

public:

    const std::string cellType() const;

    const size_t veclen() const;

    const size_t nnodes() const;

    const size_t ncells() const;

    const kvs::AnyValueArray& values() const;

    const kvs::ValueArray<kvs::Real32>& coords() const;

    const kvs::ValueArray<kvs::UInt32>& connections() const;

public:

    void setCellType( const std::string& cellType );

    void setVeclen( const size_t veclen );

    void setNNodes( const size_t nnodes );

    void setNCells( const size_t ncells );

    void setValues( const kvs::AnyValueArray& values );

    void setCoords( const kvs::ValueArray<kvs::Real32>& coords );

    void setConnections( const kvs::ValueArray<kvs::UInt32>& connections );

public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );

};
} // end of namespace kvs
#endif // PBVR__KVS__STEP_AGGREGATE_TYPE_SUBVOLUME_H_INCLUDE
