/****************************************************************************/
/**
 *  @file   UnstructuredVolumeObject.h
 *  @author Naohisa Sakamoto
 */
/****************************************************************************/
#pragma once
#include <ostream>
#include <kvs/Module>
#include <kvs/VolumeObjectBase>
#include <kvs/Indent>
#include <kvs/Deprecated>


namespace kvs
{

/*==========================================================================*/
/**
 *  Unstructured volume object class.
 */
/*==========================================================================*/
class UnstructuredVolumeObject : public kvs::VolumeObjectBase
{
    kvsModule( kvs::UnstructuredVolumeObject, Object );
    kvsModuleBaseClass( kvs::VolumeObjectBase );

public:
    using Connections = kvs::ValueArray<kvs::UInt32>;

    enum CellType
    {
        UnknownCellType = 0,  ///< Unknown cell type.
        Tetrahedra, ///< Tetrahedral cell.
        Hexahedra, ///< Hexahedral cell.
        QuadraticTetrahedra, ///< Quadratic tetrahedral cell.
        QuadraticHexahedra, ///< Quadratic Hexahedral cell.
        Pyramid, ///< Pyramidal cell.
        Point, ///< Point cell.
        Prism ///< Prism cell.
    };

private:
    CellType m_cell_type = UnknownCellType; ///< Cell type.
    std::size_t m_nnodes = 0; ///< Number of nodes.
    std::size_t m_ncells = 0; ///< Number of cells.
    Connections m_connections{}; ///< Connection ( Node ID ) array.

public:
    UnstructuredVolumeObject(): BaseClass( Unstructured ) {}
    virtual ~UnstructuredVolumeObject() = default;

    void shallowCopy( const UnstructuredVolumeObject& object );
    void deepCopy( const UnstructuredVolumeObject& object );
    void print( std::ostream& os, const kvs::Indent& indent = kvs::Indent(0) ) const;
    bool read( const std::string& filename );
    bool write( const std::string& filename, const bool ascii = true, const bool external = false ) const;

    void setCellType( CellType cell_type ) { m_cell_type = cell_type; }
    void setCellTypeToTetrahedra() { this->setCellType( Tetrahedra ); }
    void setCellTypeToHexahedra() { this->setCellType( Hexahedra ); }
    void setCellTypeToQuadraticTetrahedra() { this->setCellType( QuadraticTetrahedra ); }
    void setCellTypeToQuadraticHexahedra() { this->setCellType( QuadraticHexahedra ); }
    void setCellTypeToPyramid() { this->setCellType( Pyramid ); }
    void setCellTypeToPoint() { this->setCellType( Point ); }
    void setCellTypeToPrism() { this->setCellType( Prism ); }
    void setNumberOfNodes( const std::size_t nnodes ) { m_nnodes = nnodes; }
    void setNumberOfCells( const std::size_t ncells ) { m_ncells = ncells; }
    void setConnections( const Connections& connections ) { m_connections = connections; }

    CellType cellType() const { return m_cell_type; }
    std::size_t numberOfNodes() const { return m_nnodes; }
    std::size_t numberOfCells() const { return m_ncells; }
    const Connections& connections() const { return m_connections; }
    std::size_t numberOfCellNodes() const;

    void updateMinMaxCoords();
    void updateMinMaxValues() const;

public:
    KVS_DEPRECATED( UnstructuredVolumeObject(
                        const CellType cell_type,
                        const std::size_t nnodes,
                        const std::size_t ncells,
                        const std::size_t veclen,
                        const Coords& coords,
                        const Connections& connections,
                        const Values& values ) )
    {
        BaseClass::setVolumeType( Unstructured );
        this->setVeclen( veclen );
        this->setCoords( coords );
        this->setValues( values );
        this->setCellType( cell_type );
        this->setNumberOfNodes( nnodes );
        this->setNumberOfCells( ncells );
        this->setConnections( connections );
    }

    KVS_DEPRECATED( void setNNodes( const std::size_t nnodes ) ) { this->setNumberOfNodes( nnodes ); }
    KVS_DEPRECATED( void setNCells( const std::size_t ncells ) ) { this->setNumberOfCells( ncells ); }
    KVS_DEPRECATED( std::size_t nnodes() const ) { return this->numberOfNodes(); }
    KVS_DEPRECATED( std::size_t ncells() const ) { return this->numberOfCells(); }
    KVS_DEPRECATED( friend std::ostream& operator << ( std::ostream& os, const UnstructuredVolumeObject& object ) );
};

} // end of namespace kvs
