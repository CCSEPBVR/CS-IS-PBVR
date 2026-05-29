/****************************************************************************/
/**
 *  @file AggregateTypeSubvolume.h
 */
/****************************************************************************/
#ifndef VIS_MODULE__AGGREGATE_TYPE_SUBVOLUME_H_INCLUDE
#define VIS_MODULE__AGGREGATE_TYPE_SUBVOLUME_H_INCLUDE

#include <vismodule/FileFormatBase>
#include <vismodule/AnyValueArray>
#include <vismodule/ValueArray>
#include <vismodule/Type>
#include <vismodule/Vector3>
#include <string>
#include <FileFormat/KVSML/ObjectTag.h>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  GSubvolume unstructured volume object format.
 */
/*===========================================================================*/
class AggregateTypeSubvolume : public vismodule::FileFormatBase
{
    visModuleClassName( vismodule::AggregateTypeSubvolume );

public:

    enum WritingDataType
    {
        Ascii = 0,
        ExternalAscii,
        ExternalBinary
    };

protected:

    std::string                  m_cell_type;     ///< cell type
    std::size_t                       m_vector_length; ///< vector length
    std::size_t                       m_nnodes;        ///< number of nodes
    std::size_t                       m_ncells;        ///< number of cells
    vismodule::AnyValueArray           m_values;        ///< field value array
    vismodule::ValueArray<vismodule::Real32> m_coords;        ///< coordinate value array
    vismodule::ValueArray<vismodule::UInt32> m_connections;   ///< connection id array

public:

    AggregateTypeSubvolume();

    AggregateTypeSubvolume( const std::string& filename, const int st, const int vl );

    virtual ~AggregateTypeSubvolume();

public:

    const std::string cellType() const;

    const std::size_t veclen() const;

    const std::size_t nnodes() const;

    const std::size_t ncells() const;

    const vismodule::AnyValueArray& values() const;

    const vismodule::ValueArray<vismodule::Real32>& coords() const;

    const vismodule::ValueArray<vismodule::UInt32>& connections() const;

public:

    void setCellType( const std::string& cellType );

    void setVeclen( const std::size_t veclen );

    void setNNodes( const std::size_t nnodes );

    void setNCells( const std::size_t ncells );

    void setValues( const vismodule::AnyValueArray& values );

    void setCoords( const vismodule::ValueArray<vismodule::Real32>& coords );

    void setConnections( const vismodule::ValueArray<vismodule::UInt32>& connections );

public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );

};
} // end of namespace vismodule
#endif // VIS_MODULE__AGGREGATE_TYPE_SUBVOLUME_H_INCLUDE
