/*****************************************************************************/
/**
 *  @file   ExternalFaces.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ExternalFaces.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "ExternalFaces.h"
#include <kvs/VolumeObjectBase>
#include <kvs/StructuredVolumeObject>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/TransferFunction>
#include <kvs/IgnoreUnusedVariable>
#include <kvs/Timer>
#include <map>


namespace
{

/*===========================================================================*/
/**
 *  @brief  Calculates color indices for the given nodes.
 *  @param  value [in] pointer to the node value
 *  @param  min_value [in] minimum value of the node value
 *  @param  max_value [in] maximum value of the node value
 *  @param  veclen [in] vector length of the node data
 *  @param  colormap_resolution [in] resolution of the color map
 *  @param  node_index [in] node indices
 *  @param  color_level [out] pointer to the color indices
 */
/*===========================================================================*/
template <const size_t N, typename T>
inline void GetColorIndices(
    const T* value,
    const kvs::Real64 min_value,
    const kvs::Real64 max_value,
    const size_t veclen,
    const size_t colormap_resolution,
    const kvs::UInt32 node_index[N],
    kvs::UInt32 (*color_index)[N] )
{
    const kvs::Real64 normalize =
        static_cast<kvs::Real64>( colormap_resolution - 1 ) / ( max_value - min_value );

    // Scalar data.
    if ( veclen == 1 )
    {
        for ( size_t i = 0; i < N; i++ )
        {
            (*color_index)[i] = kvs::UInt32( normalize * ( kvs::Real64( value[ node_index[i] ] ) - min_value ) );
        }
    }
    // Vector data.
    else
    {
        // In case of the vector component, the magnitude value is calculated.
        kvs::Real64 magnitude[N]; memset( magnitude, 0, sizeof( kvs::Real64 ) * N );
        for ( size_t i = 0; i < veclen; ++i )
        {
            for ( size_t j = 0; j < N; j++ )
            {
                magnitude[j] += kvs::Math::Square( kvs::Real64( value[ veclen * node_index[j] + i ] ) );
            }
        }

        for ( size_t i = 0; i < N; i++ )
        {
            magnitude[i] = kvs::Math::SquareRoot( magnitude[i] );
            (*color_index)[i] = kvs::UInt32( normalize * ( magnitude[i] - min_value ) );
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Triangle face class.
 */
/*===========================================================================*/
class Face
{
private:

    kvs::UInt32 m_id[3]; ///< vertex IDs

public:

    Face( const kvs::UInt32 id0, const kvs::UInt32 id1, const kvs::UInt32 id2 );

public:

    const kvs::UInt32 id( const size_t index ) const;

    void set( const kvs::UInt32 id0, const kvs::UInt32 id1, const kvs::UInt32 id2 );

    friend const bool operator == ( const Face& f0, const Face& f1 );
};

/*===========================================================================*/
/**
 *  @brief  Constructs a new Face class.
 *  @param  id0 [in] ID 0
 *  @param  id1 [in] ID 1
 *  @param  id2 [in] ID 2
 *  @return <ReturnValue>
 */
/*===========================================================================*/
inline Face::Face( const kvs::UInt32 id0, const kvs::UInt32 id1, const kvs::UInt32 id2 )
{
    this->set( id0, id1, id2 );
}

/*===========================================================================*/
/**
 *  @brief  Returns a vertex ID.
 *  @param  index [in] index (0-2)
 *  @return vertex ID
 */
/*===========================================================================*/
inline const kvs::UInt32 Face::id( const size_t index ) const
{
    return( m_id[ index ] );
}

/*===========================================================================*/
/**
 *  @brief  Sets vertex IDs.
 *  @param  id0 [in] ID 0
 *  @param  id1 [in] ID 1
 *  @param  id2 [in] ID 2
 */
/*===========================================================================*/
inline void Face::set( const kvs::UInt32 id0, const kvs::UInt32 id1, const kvs::UInt32 id2 )
{
    m_id[0] = id0;
    m_id[1] = id1;
    m_id[2] = id2;
}

/*===========================================================================*/
/**
 *  @brief  '==' operator for the Face class.
 *  @param  f0 [in] face 0
 *  @param  f1 [in] face 1
 *  @return true, if the face 0 is equal to the face 1
 */
/*===========================================================================*/
inline const bool operator == ( const Face& f0, const Face& f1 )
{
    for ( size_t i = 0; i < 3; i++ )
    {
        bool flag = false;
        for ( size_t j = 0; j < 3; j++ )
        {
            if ( f0.id(i) == f1.id(j) ) { flag = true; continue; }
        }
        if ( !flag ) return( false );
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Face map class.
 */
/*===========================================================================*/
class FaceMap
{
public:

    typedef kvs::UInt32 Key;
    typedef Face Value;
    typedef std::multimap<Key,Value> Bucket;

private:

    size_t m_nvertices; ///< number of vertices of the original data
    Bucket m_bucket; ///< bucket for the edge data

public:

    FaceMap( const size_t nvertices );

public:

    const Bucket& bucket( void ) const;

    void insert( const kvs::UInt32 id0, const kvs::UInt32 id1, const kvs::UInt32 id2 );

private:

    const Key get_key( const kvs::UInt32 id0, const kvs::UInt32 id1, const kvs::UInt32 id2 );
};

/*===========================================================================*/
/**
 *  @brief  Constructs a new FaceMap class.
 *  @param  nvertices [in] number of vertices
 */
/*===========================================================================*/
inline FaceMap::FaceMap( const size_t nvertices ):
    m_nvertices( nvertices )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns a bucket for the face.
 *  @return bucket
 */
/*===========================================================================*/
inline const FaceMap::Bucket& FaceMap::bucket( void ) const
{
    return( m_bucket );
}

/*===========================================================================*/
/**
 *  @brief  Inserts a face indicated by the given IDs.
 *  @param  id0 [in] ID 0
 *  @param  id1 [in] ID 1
 *  @param  id2 [in] ID 2
 */
/*===========================================================================*/
inline void FaceMap::insert( const kvs::UInt32 id0, const kvs::UInt32 id1, const kvs::UInt32 id2 )
{
    const Key key( this->get_key( id0, id1, id2 ) );
    const Value value( id0, id1, id2 );

    Bucket::iterator f = m_bucket.find( key );
    Bucket::const_iterator last = m_bucket.end();
    if ( f != last )
    {
        Bucket::const_iterator upper = m_bucket.upper_bound( key );
        do
        {
            if ( f->second == value )
            {
                // The edge has been already inserted in the bucket.
                m_bucket.erase( f );
                return;
            }
            f++;
        } while ( f != upper );
    }

    m_bucket.insert( std::make_pair( key, value ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns a key for searching the face.
 *  @param  id0 [in] ID 0
 *  @param  id1 [in] ID 1
 *  @param  id2 [in] ID 2
 *  @return key
 */
/*===========================================================================*/
inline const FaceMap::Key FaceMap::get_key( const kvs::UInt32 id0, const kvs::UInt32 id1, const kvs::UInt32 id2 )
{
    const kvs::UInt32 sum = id0 + id1 + id2;
    return( sum % kvs::UInt32( m_nvertices ) );
}

/*===========================================================================*/
/**
 *  @brief  Creates a face map for the tetrahedral cells.
 *  @param  volume [in] pointer to the unstructured volume object
 *  @param  face_map [out] pointer to the face map
 */
/*===========================================================================*/
inline void CreateTetrahedraFaceMap(
    const kvs::UnstructuredVolumeObject* volume,
    FaceMap* face_map )
{
    const kvs::UInt32* connections = volume->connections().pointer();
    const size_t ncells = volume->ncells();
    for ( size_t cell_index = 0, connection_index = 0; cell_index < ncells; cell_index++ )
    {
        // Local vertices of the tetrahedral cell.
        const kvs::UInt32 v0 = connections[ connection_index     ];
        const kvs::UInt32 v1 = connections[ connection_index + 1 ];
        const kvs::UInt32 v2 = connections[ connection_index + 2 ];
        const kvs::UInt32 v3 = connections[ connection_index + 3 ];
        connection_index += 4;

        // Local faces of the cell (4 triangle meshes).
        face_map->insert( v0, v1, v2 );
        face_map->insert( v0, v2, v3 );
        face_map->insert( v0, v3, v1 );
        face_map->insert( v1, v3, v2 );
    }
}

/*===========================================================================*/
/**
 *  @brief  Creates a face map for the quadratic tetrahedral cells.
 *  @param  volume [in] pointer to the unstructured volume object
 *  @param  face_map [out] pointer to the face map
 */
/*===========================================================================*/
inline void CreateQuadraticTetrahedraFaceMap(
    const kvs::UnstructuredVolumeObject* volume,
    FaceMap* face_map )
{
    const kvs::UInt32* connections = volume->connections().pointer();
    const size_t ncells = volume->ncells();
    for ( size_t cell_index = 0, connection_index = 0; cell_index < ncells; cell_index++ )
    {
        // Local vertices of the quadratic tetrahedral cell.
        const kvs::UInt32 v0 = connections[ connection_index     ];
        const kvs::UInt32 v1 = connections[ connection_index + 1 ];
        const kvs::UInt32 v2 = connections[ connection_index + 2 ];
        const kvs::UInt32 v3 = connections[ connection_index + 3 ];
        const kvs::UInt32 v4 = connections[ connection_index + 4 ];
        const kvs::UInt32 v5 = connections[ connection_index + 5 ];
        const kvs::UInt32 v6 = connections[ connection_index + 6 ];
        const kvs::UInt32 v7 = connections[ connection_index + 7 ];
        const kvs::UInt32 v8 = connections[ connection_index + 8 ];
        const kvs::UInt32 v9 = connections[ connection_index + 9 ];
        connection_index += 10;

        // Local faces of the cell (16 triangle meshes).
        face_map->insert( v0, v4, v5 );
        face_map->insert( v4, v1, v7 );
        face_map->insert( v5, v7, v2 );
        face_map->insert( v7, v5, v4 );

        face_map->insert( v0, v5, v6 );
        face_map->insert( v5, v2, v8 );
        face_map->insert( v6, v8, v3 );
        face_map->insert( v8, v6, v5 );

        face_map->insert( v0, v6, v4 );
        face_map->insert( v6, v3, v9 );
        face_map->insert( v4, v9, v1 );
        face_map->insert( v9, v4, v6 );

        face_map->insert( v1, v9, v7 );
        face_map->insert( v9, v3, v8 );
        face_map->insert( v7, v8, v2 );
        face_map->insert( v8, v7, v9 );
    }
}

/*===========================================================================*/
/**
 *  @brief  Creates a face map for the hexahedral cells.
 *  @param  volume [in] pointer to the unstructured volume object
 *  @param  face_map [out] pointer to the face map
 */
/*===========================================================================*/
inline void CreateHexahedraFaceMap(
    const kvs::UnstructuredVolumeObject* volume,
    FaceMap* face_map )
{
    const kvs::UInt32* connections = volume->connections().pointer();
    const size_t ncells = volume->ncells();
    for ( size_t cell_index = 0, connection_index = 0; cell_index < ncells; cell_index++ )
    {
        // Local vertices of the quadratic tetrahedral cell.
        const kvs::UInt32 v0 = connections[ connection_index     ];
        const kvs::UInt32 v1 = connections[ connection_index + 1 ];
        const kvs::UInt32 v2 = connections[ connection_index + 2 ];
        const kvs::UInt32 v3 = connections[ connection_index + 3 ];
        const kvs::UInt32 v4 = connections[ connection_index + 4 ];
        const kvs::UInt32 v5 = connections[ connection_index + 5 ];
        const kvs::UInt32 v6 = connections[ connection_index + 6 ];
        const kvs::UInt32 v7 = connections[ connection_index + 7 ];
        connection_index += 8;

        // Local faces of the cell (12 triangle meshes).
        face_map->insert( v0, v4, v1 );
        face_map->insert( v1, v4, v5 );

        face_map->insert( v1, v5, v2 );
        face_map->insert( v2, v5, v6 );

        face_map->insert( v2, v6, v3 );
        face_map->insert( v3, v6, v7 );

        face_map->insert( v3, v7, v0 );
        face_map->insert( v0, v7, v4 );

        face_map->insert( v0, v1, v3 );
        face_map->insert( v3, v1, v2 );

        face_map->insert( v5, v4, v6 );
        face_map->insert( v6, v4, v7 );
    }
}

/*===========================================================================*/
/**
 *  @brief  Creates a face map for the quadratic hexahedral cells.
 *  @param  volume [in] pointer to the unstructured volume object
 *  @param  face_map [out] pointer to the face map
 */
/*===========================================================================*/
inline void CreateQuadraticHexahedraFaceMap(
    const kvs::UnstructuredVolumeObject* volume,
    FaceMap* face_map )
{
    const kvs::UInt32* connections = volume->connections().pointer();
    const size_t ncells = volume->ncells();
    for ( size_t cell_index = 0, connection_index = 0; cell_index < ncells; cell_index++ )
    {
        // Local vertices of the quadratic tetrahedral cell.
        const kvs::UInt32 v0  = connections[ connection_index      ];
        const kvs::UInt32 v1  = connections[ connection_index +  1 ];
        const kvs::UInt32 v2  = connections[ connection_index +  2 ];
        const kvs::UInt32 v3  = connections[ connection_index +  3 ];
        const kvs::UInt32 v4  = connections[ connection_index +  4 ];
        const kvs::UInt32 v5  = connections[ connection_index +  5 ];
        const kvs::UInt32 v6  = connections[ connection_index +  6 ];
        const kvs::UInt32 v7  = connections[ connection_index +  7 ];
        const kvs::UInt32 v8  = connections[ connection_index +  8 ];
        const kvs::UInt32 v9  = connections[ connection_index +  9 ];
        const kvs::UInt32 v10 = connections[ connection_index + 10 ];
        const kvs::UInt32 v11 = connections[ connection_index + 11 ];
        const kvs::UInt32 v12 = connections[ connection_index + 12 ];
        const kvs::UInt32 v13 = connections[ connection_index + 13 ];
        const kvs::UInt32 v14 = connections[ connection_index + 14 ];
        const kvs::UInt32 v15 = connections[ connection_index + 15 ];
        const kvs::UInt32 v16 = connections[ connection_index + 16 ];
        const kvs::UInt32 v17 = connections[ connection_index + 17 ];
        const kvs::UInt32 v18 = connections[ connection_index + 18 ];
        const kvs::UInt32 v19 = connections[ connection_index + 19 ];
        connection_index += 20;

        // Local faces of the cell (36 triangle meshes).
        face_map->insert( v0,  v16, v8  );
        face_map->insert( v4,  v12, v16 );
        face_map->insert( v5,  v17, v12 );
        face_map->insert( v1,  v8,  v17 );
        face_map->insert( v8,  v16, v17 );
        face_map->insert( v12, v17, v16 );

        face_map->insert( v1,  v17, v9  );
        face_map->insert( v5,  v13, v17 );
        face_map->insert( v6,  v18, v13 );
        face_map->insert( v2,  v9,  v18 );
        face_map->insert( v9,  v17, v18 );
        face_map->insert( v13, v18, v17 );

        face_map->insert( v2,  v18, v10 );
        face_map->insert( v6,  v14, v18 );
        face_map->insert( v7,  v19, v14 );
        face_map->insert( v3,  v10, v19 );
        face_map->insert( v10, v18, v19 );
        face_map->insert( v14, v19, v18 );

        face_map->insert( v3,  v19, v11 );
        face_map->insert( v7,  v15, v19 );
        face_map->insert( v4,  v16, v15 );
        face_map->insert( v0,  v11, v16 );
        face_map->insert( v11, v19, v16 );
        face_map->insert( v15, v16, v19 );

        face_map->insert( v0,  v8,  v11 );
        face_map->insert( v1,  v9,  v8  );
        face_map->insert( v2,  v10, v9  );
        face_map->insert( v3,  v11, v10 );
        face_map->insert( v11, v8,  v10 );
        face_map->insert( v9,  v10, v8  );

        face_map->insert( v5,  v12, v13 );
        face_map->insert( v4,  v15, v12 );
        face_map->insert( v7,  v14, v15 );
        face_map->insert( v6,  v13, v14 );
        face_map->insert( v13, v12, v14 );
        face_map->insert( v15, v14, v12 );
    }
}

/*===========================================================================*/
/**
 *  @brief  Calculates external faces using the face map.
 *  @param  volume [in] pointer to the unstructured volume object
 *  @param  cmap [in] color map
 *  @param  face_map [in] face map
 *  @param  coords [out] pointer to the coordinate value array
 *  @param  colors [out] pointer to the color value array
 *  @param  normals [out] pointer to the normal vector array
 */
/*===========================================================================*/
template <typename T>
void CalculateFaces(
    const kvs::UnstructuredVolumeObject* volume,
    const kvs::ColorMap cmap,
    const FaceMap& face_map,
    kvs::ValueArray<kvs::Real32>* coords,
    kvs::ValueArray<kvs::UInt8>* colors,
    kvs::ValueArray<kvs::Real32>* normals )
{
    // Parameters of the volume data.
    if ( !volume->hasMinMaxValues() ) { volume->updateMinMaxValues(); }
    const kvs::Real64 min_value = volume->minValue();
    const kvs::Real64 max_value = volume->maxValue();
    const size_t veclen = volume->veclen();
    const T* value = reinterpret_cast<const T*>( volume->values().pointer() );

    const size_t nfaces = face_map.bucket().size();
    const size_t nvertices = nfaces * 3;
    const kvs::Real32* volume_coord = volume->coords().pointer();

    kvs::Real32* coord = coords->allocate( nvertices * 3 );
    kvs::UInt8* color = colors->allocate( nvertices * 3 );
    kvs::Real32* normal = normals->allocate( nfaces * 3 );

    kvs::UInt32 node_index[3] = { 0, 0, 0 };
    kvs::UInt32 color_level[3] = { 0, 0, 0 };

    FaceMap::Bucket::const_iterator f = face_map.bucket().begin();
    FaceMap::Bucket::const_iterator last = face_map.bucket().end();
    while ( f != last )
    {
        node_index[0] = f->second.id(0);
        node_index[1] = f->second.id(1);
        node_index[2] = f->second.id(2);

        const kvs::Vector3f v0( volume_coord + 3 * node_index[0] );
        const kvs::Vector3f v1( volume_coord + 3 * node_index[1] );
        const kvs::Vector3f v2( volume_coord + 3 * node_index[2] );
        // v0
        *( coord++ ) = v0.x();
        *( coord++ ) = v0.y();
        *( coord++ ) = v0.z();
        // v1
        *( coord++ ) = v1.x();
        *( coord++ ) = v1.y();
        *( coord++ ) = v1.z();
        // v2
        *( coord++ ) = v2.x();
        *( coord++ ) = v2.y();
        *( coord++ ) = v2.z();

        GetColorIndices<3>( value, min_value, max_value, veclen, cmap.resolution(), node_index, &color_level );
        // c0
        *( color++ ) = cmap[ color_level[0] ].red();
        *( color++ ) = cmap[ color_level[0] ].green();
        *( color++ ) = cmap[ color_level[0] ].blue();
        // c1
        *( color++ ) = cmap[ color_level[1] ].red();
        *( color++ ) = cmap[ color_level[1] ].green();
        *( color++ ) = cmap[ color_level[1] ].blue();
        // c2
        *( color++ ) = cmap[ color_level[2] ].red();
        *( color++ ) = cmap[ color_level[2] ].green();
        *( color++ ) = cmap[ color_level[2] ].blue();

        const kvs::Vector3f n( ( v1 - v0 ).cross( v2 - v0 ) );
        // n0
        *( normal++ ) = n.x();
        *( normal++ ) = n.y();
        *( normal++ ) = n.z();

        f++;
    }
}

} // end of namespace


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new ExternalFaces class.
 */
/*===========================================================================*/
ExternalFaces::ExternalFaces( void ):
    kvs::MapperBase(),
    kvs::PolygonObject()
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new ExternalFaces class.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
ExternalFaces::ExternalFaces( const kvs::VolumeObjectBase* volume ):
    kvs::MapperBase(),
    kvs::PolygonObject()
{
    this->exec( volume );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new ExternalFaces class.
 *  @param  volume [in] pointer to the structured volume object
 *  @param  transfer_function [in] transfer function
 */
/*===========================================================================*/
ExternalFaces::ExternalFaces(
    const kvs::VolumeObjectBase* volume,
    const kvs::TransferFunction& transfer_function ):
    kvs::MapperBase( transfer_function ),
    kvs::PolygonObject()
{
    this->exec( volume );
}

/*===========================================================================*/
/**
 *  @brief  Destructs the ExternalFaces class.
 */
/*===========================================================================*/
ExternalFaces::~ExternalFaces( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Executes the edge extraction.
 *  @param  object [in] pointer of the volume object
 *  @return pointer of the line object
 */
/*===========================================================================*/
ExternalFaces::SuperClass* ExternalFaces::exec( const kvs::ObjectBase* object )
{
    if ( !object )
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Input object is NULL.");
        return( NULL );
    }

    const kvs::VolumeObjectBase* volume = kvs::VolumeObjectBase::DownCast( object );
    if ( !volume )
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Input object is not volume dat.");
        return( NULL );
    }

    const kvs::VolumeObjectBase::VolumeType type = volume->volumeType();
    if ( type == kvs::VolumeObjectBase::Structured )
    {
        this->mapping( kvs::StructuredVolumeObject::DownCast( volume ) );
    }
    else // type == kvs::VolumeObjectBase::Unstructured
    {
        this->mapping( kvs::UnstructuredVolumeObject::DownCast( volume ) );
    }

    return( this );
}

/*===========================================================================*/
/**
 *  @brief  Mapping for a structured volume object.
 *  @param  volume [in] pointer to the strctured volume object
 */
/*===========================================================================*/
void ExternalFaces::mapping( const kvs::StructuredVolumeObject* volume )
{
    BaseClass::attach_volume( volume );
    BaseClass::set_range( volume );
    BaseClass::set_min_max_coords( volume, this );

    this->calculate_coords( volume );

    const std::type_info& type = volume->values().typeInfo()->type();
    if (      type == typeid( kvs::Int8   ) ) { this->calculate_colors<kvs::Int8  >( volume ); }
    else if ( type == typeid( kvs::Int16  ) ) { this->calculate_colors<kvs::Int16 >( volume ); }
    else if ( type == typeid( kvs::Int32  ) ) { this->calculate_colors<kvs::Int32 >( volume ); }
    else if ( type == typeid( kvs::Int64  ) ) { this->calculate_colors<kvs::Int64 >( volume ); }
    else if ( type == typeid( kvs::UInt8  ) ) { this->calculate_colors<kvs::UInt8 >( volume ); }
    else if ( type == typeid( kvs::UInt16 ) ) { this->calculate_colors<kvs::UInt16>( volume ); }
    else if ( type == typeid( kvs::UInt32 ) ) { this->calculate_colors<kvs::UInt32>( volume ); }
    else if ( type == typeid( kvs::UInt64 ) ) { this->calculate_colors<kvs::UInt64>( volume ); }
    else if ( type == typeid( kvs::Real32 ) ) { this->calculate_colors<kvs::Real32>( volume ); }
    else if ( type == typeid( kvs::Real64 ) ) { this->calculate_colors<kvs::Real64>( volume ); }

    SuperClass::setPolygonType( kvs::PolygonObject::Quadrangle );
    SuperClass::setColorType( kvs::PolygonObject::VertexColor );
    SuperClass::setNormalType( kvs::PolygonObject::PolygonNormal );
    if ( SuperClass::nopacities() == 0 ) SuperClass::setOpacity( 255 );
}

/*===========================================================================*/
/**
 *  @brief  Calculates vertex values (coordinate, color and normal).
 *  @param  volume [in] pointer to the structured volume object
 */
/*===========================================================================*/
void ExternalFaces::calculate_coords( const kvs::StructuredVolumeObject* volume )
{
    const kvs::VolumeObjectBase::GridType type = volume->gridType();
    if ( type == kvs::VolumeObjectBase::Uniform )
    {
        this->calculate_uniform_coords( volume );
    }
    else if ( type == kvs::VolumeObjectBase::Rectilinear )
    {
        this->calculate_rectilinear_coords( volume );
    }
    else if ( type == kvs::VolumeObjectBase::Curvilinear )
    {
        this->calculate_curvilinear_coords( volume );
    }
    else if ( type == kvs::VolumeObjectBase::Irregular )
    {
        this->calculate_irregular_coords( volume );
    }
    else
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Not supported grid type.");
    }
}

/*===========================================================================*/
/**
 *  @brief  Calculates vertex values of the uniform volume object.
 *  @param  volume [in] pointer to the structured volume object
 */
/*===========================================================================*/
void ExternalFaces::calculate_uniform_coords( const kvs::StructuredVolumeObject* volume )
{
    const kvs::Vector3ui resolution( volume->resolution() );
    const kvs::Vector3f  volume_size( volume->maxObjectCoord() - volume->minObjectCoord() );
    const kvs::Vector3ui ngrids( resolution - kvs::Vector3ui( 1, 1, 1 ) );
    const kvs::Vector3f  grid_size(
        volume_size.x() / static_cast<float>( ngrids.x() ),
        volume_size.y() / static_cast<float>( ngrids.y() ),
        volume_size.z() / static_cast<float>( ngrids.z() ) );

    const size_t nexternal_faces =
        2 * ngrids.x() * ngrids.y() +
        2 * ngrids.y() * ngrids.z() +
        2 * ngrids.z() * ngrids.x();
    const size_t nexternal_vertices = nexternal_faces * 4;

    kvs::ValueArray<kvs::Real32> coords( 3 * nexternal_vertices );
    kvs::Real32* coord = coords.pointer();

    kvs::ValueArray<kvs::Real32> normals( 3 * nexternal_faces );
    kvs::Real32* normal = normals.pointer();

    // XY (Z=0) plane.
    {
        const float z = 0.0f;
        const kvs::Vector3f n( 0.0f, 0.0f, -1.0f );
        for ( size_t j = 0; j < ngrids.y(); j++ )
        {
            const float y = grid_size.y() * static_cast<float>( j );
            for ( size_t i = 0; i < ngrids.x(); i++ )
            {
                const float x = grid_size.x() * static_cast<float>( i );
                // v3
                *( coord++ ) = x;
                *( coord++ ) = y + grid_size.y();
                *( coord++ ) = z;
                // v2
                *( coord++ ) = x + grid_size.x();
                *( coord++ ) = y + grid_size.y();
                *( coord++ ) = z;
                // v1
                *( coord++ ) = x + grid_size.x();
                *( coord++ ) = y;
                *( coord++ ) = z;
                // v0
                *( coord++ ) = x;
                *( coord++ ) = y;
                *( coord++ ) = z;
                // n0
                *( normal++ ) = n.x();
                *( normal++ ) = n.y();
                *( normal++ ) = n.z();
            }
        }
    }

    // XY (Z=ngrids.z()) plane.
    {
        const float z = grid_size.z() * static_cast<float>( ngrids.z() );
        const kvs::Vector3f n( 0.0f, 0.0f, 1.0f );
        for ( size_t j = 0; j < ngrids.y(); j++ )
        {
            const float y = grid_size.y() * static_cast<float>( j );
            for ( size_t i = 0; i < ngrids.x(); i++ )
            {
                const float x = grid_size.x() * static_cast<float>( i );
                // v0
                *( coord++ ) = x;
                *( coord++ ) = y;
                *( coord++ ) = z;
                // v1
                *( coord++ ) = x + grid_size.x();
                *( coord++ ) = y;
                *( coord++ ) = z;
                // v2
                *( coord++ ) = x + grid_size.x();
                *( coord++ ) = y + grid_size.y();
                *( coord++ ) = z;
                // v3
                *( coord++ ) = x;
                *( coord++ ) = y + grid_size.y();
                *( coord++ ) = z;
                // n0
                *( normal++ ) = n.x();
                *( normal++ ) = n.y();
                *( normal++ ) = n.z();
            }
        }
    }

    // YZ (X=0) plane.
    {
        const float x = 0.0f;
        const kvs::Vector3f n( -1.0f, 0.0f, 0.0f );
        for ( size_t j = 0; j < ngrids.y(); j++ )
        {
            const float y = grid_size.y() * static_cast<float>( j );
            for ( size_t k = 0; k < ngrids.z(); k++ )
            {
                const float z = grid_size.z() * static_cast<float>( k );
                // v0
                *( coord++ ) = x;
                *( coord++ ) = y;
                *( coord++ ) = z;
                // v1
                *( coord++ ) = x;
                *( coord++ ) = y;
                *( coord++ ) = z + grid_size.z();
                // v2
                *( coord++ ) = x;
                *( coord++ ) = y + grid_size.y();
                *( coord++ ) = z + grid_size.z();
                // v3
                *( coord++ ) = x;
                *( coord++ ) = y + grid_size.y();
                *( coord++ ) = z;
                // n0
                *( normal++ ) = n.x();
                *( normal++ ) = n.y();
                *( normal++ ) = n.z();
            }
        }
    }

    // YZ (X=ngrids.x()) plane.
    {
        const float x = grid_size.y() * static_cast<float>( ngrids.x() );
        const kvs::Vector3f n( 1.0f, 0.0f, 0.0f );
        for ( size_t j = 0; j < ngrids.y(); j++ )
        {
            const float y = grid_size.y() * static_cast<float>( j );
            for ( size_t k = 0; k < ngrids.z(); k++ )
            {
                const float z = grid_size.z() * static_cast<float>( k );
                // v3
                *( coord++ ) = x;
                *( coord++ ) = y + grid_size.y();
                *( coord++ ) = z;
                // v2
                *( coord++ ) = x;
                *( coord++ ) = y + grid_size.y();
                *( coord++ ) = z + grid_size.z();
                // v1
                *( coord++ ) = x;
                *( coord++ ) = y;
                *( coord++ ) = z + grid_size.z();
                // v0
                *( coord++ ) = x;
                *( coord++ ) = y;
                *( coord++ ) = z;
                // n0
                *( normal++ ) = n.x();
                *( normal++ ) = n.y();
                *( normal++ ) = n.z();
            }
        }
    }

    // XZ (Y=0) plane.
    {
        const float y = 0.0f;
        const kvs::Vector3f n( 0.0f, -1.0f, 0.0f );
        for ( size_t k = 0; k < ngrids.z(); k++ )
        {
            const float z = grid_size.z() * static_cast<float>( k );
            for ( size_t i = 0; i < ngrids.x(); i++ )
            {
                const float x = grid_size.x() * static_cast<float>( i );
                // v0
                *( coord++ ) = x;
                *( coord++ ) = y;
                *( coord++ ) = z;
                // v1
                *( coord++ ) = x + grid_size.z();
                *( coord++ ) = y;
                *( coord++ ) = z;
                // v2
                *( coord++ ) = x + grid_size.x();
                *( coord++ ) = y;
                *( coord++ ) = z + grid_size.z();
                // v3
                *( coord++ ) = x;
                *( coord++ ) = y;
                *( coord++ ) = z + grid_size.z();
                // n0
                *( normal++ ) = n.x();
                *( normal++ ) = n.y();
                *( normal++ ) = n.z();
            }
        }
    }

    // XZ (Y=ngrids.y()) plane.
    {
        const float y = grid_size.y() * static_cast<float>( ngrids.y() );
        const kvs::Vector3f n( 0.0f, 1.0f, 0.0f );
        for ( size_t k = 0; k < ngrids.z(); k++ )
        {
            const float z = grid_size.z() * static_cast<float>( k );
            for ( size_t i = 0; i < ngrids.x(); i++ )
            {
                const float x = grid_size.x() * static_cast<float>( i );
                // v3
                *( coord++ ) = x;
                *( coord++ ) = y;
                *( coord++ ) = z + grid_size.z();
                // v2
                *( coord++ ) = x + grid_size.x();
                *( coord++ ) = y;
                *( coord++ ) = z + grid_size.z();
                // v1
                *( coord++ ) = x + grid_size.x();
                *( coord++ ) = y;
                *( coord++ ) = z;
                // v0
                *( coord++ ) = x;
                *( coord++ ) = y;
                *( coord++ ) = z;
                // n0
                *( normal++ ) = n.x();
                *( normal++ ) = n.y();
                *( normal++ ) = n.z();
            }
        }
    }

    SuperClass::setCoords( coords );
    SuperClass::setNormals( normals );
}

/*===========================================================================*/
/**
 *  @brief  Calculates vertex values of the rectiliear volume object.
 *  @param  volume [in] pointer to the structured volume object
 */
/*===========================================================================*/
void ExternalFaces::calculate_rectilinear_coords( const kvs::StructuredVolumeObject* volume )
{
    kvs::IgnoreUnusedVariable( volume );

    BaseClass::m_is_success = false;
    kvsMessageError("Rectilinear volume has not yet support.");
}

/*===========================================================================*/
/**
 *  @brief  Calculates vertex values of the curvilinear volume object.
 *  @param  volume [in] pointer to the structured volume object
 */
/*===========================================================================*/
void ExternalFaces::calculate_curvilinear_coords( const kvs::StructuredVolumeObject* volume )
{
    kvs::IgnoreUnusedVariable( volume );

    BaseClass::m_is_success = false;
    kvsMessageError("Curvilinear volume has not yet support.");
}

/*===========================================================================*/
/**
 *  @brief  Calculates vertex values of the irregular volume object.
 *  @param  volume [in] pointer to the structured volume object
 */
/*===========================================================================*/
void ExternalFaces::calculate_irregular_coords( const kvs::StructuredVolumeObject* volume )
{
    kvs::IgnoreUnusedVariable( volume );

    BaseClass::m_is_success = false;
    kvsMessageError("Irregular volume has not yet support.");
}

/*===========================================================================*/
/**
 *  @brief  Calculates color values.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
template <typename T>
void ExternalFaces::calculate_colors( const kvs::StructuredVolumeObject* volume )
{
    // Parameters of the volume data.
    if ( !volume->hasMinMaxValues() ) { volume->updateMinMaxValues(); }
    const kvs::Real64 min_value = volume->minValue();
    const kvs::Real64 max_value = volume->maxValue();
    const size_t veclen = volume->veclen();
    const size_t nnodes_per_line = volume->nnodesPerLine();
    const size_t nnodes_per_slice = volume->nnodesPerSlice();
    const kvs::Vector3ui resolution( volume->resolution() );
    const kvs::Vector3ui ngrids( resolution - kvs::Vector3ui( 1, 1, 1 ) );
    const T* value = reinterpret_cast<const T*>( volume->values().pointer() );

    const size_t nexternal_faces =
        2 * ngrids.x() * ngrids.y() +
        2 * ngrids.y() * ngrids.z() +
        2 * ngrids.z() * ngrids.x();
    const size_t nexternal_vertices = nexternal_faces * 4;

    const kvs::ColorMap cmap( BaseClass::colorMap() );

    kvs::ValueArray<kvs::UInt8> colors( 3 * nexternal_vertices );
    kvs::UInt8* color = colors.pointer();

    kvs::UInt32 node_index[4];
    kvs::UInt32 color_level[4];

    // XY (Z=0) plane.
    {
        const size_t k = 0;
        const size_t offset0 = k * nnodes_per_line;
        for ( size_t j = 0, offset = offset0; j < ngrids.y(); j++, offset = offset0 + j * nnodes_per_line )
        {
            for ( size_t i = 0; i < ngrids.x(); i++, offset += 1 )
            {
                node_index[0] = offset;
                node_index[1] = node_index[0] + 1;
                node_index[2] = node_index[1] + nnodes_per_line;
                node_index[3] = node_index[0] + nnodes_per_line;
                ::GetColorIndices<4>( value, min_value, max_value, veclen, cmap.resolution(), node_index, &color_level );
                // v3
                *( color++ ) = cmap[ color_level[3] ].red();
                *( color++ ) = cmap[ color_level[3] ].green();
                *( color++ ) = cmap[ color_level[3] ].blue();
                // v2
                *( color++ ) = cmap[ color_level[2] ].red();
                *( color++ ) = cmap[ color_level[2] ].green();
                *( color++ ) = cmap[ color_level[2] ].blue();
                // v1
                *( color++ ) = cmap[ color_level[1] ].red();
                *( color++ ) = cmap[ color_level[1] ].green();
                *( color++ ) = cmap[ color_level[1] ].blue();
                // v0
                *( color++ ) = cmap[ color_level[0] ].red();
                *( color++ ) = cmap[ color_level[0] ].green();
                *( color++ ) = cmap[ color_level[0] ].blue();
            }
        }
    }

    // XY (Z=ngrids.z()) plane.
    {
        const size_t k = ngrids.z();
        const size_t offset0 = k * nnodes_per_slice;
        for ( size_t j = 0, offset = offset0; j < ngrids.y(); j++, offset = offset0 + j * nnodes_per_line )
        {
            for ( size_t i = 0; i < ngrids.x(); i++, offset += 1 )
            {
                node_index[0] = offset;
                node_index[1] = node_index[0] + 1;
                node_index[2] = node_index[1] + nnodes_per_line;
                node_index[3] = node_index[0] + nnodes_per_line;
                ::GetColorIndices<4>( value, min_value, max_value, veclen, cmap.resolution(), node_index, &color_level );
                // v0
                *( color++ ) = cmap[ color_level[0] ].red();
                *( color++ ) = cmap[ color_level[0] ].green();
                *( color++ ) = cmap[ color_level[0] ].blue();
                // v1
                *( color++ ) = cmap[ color_level[1] ].red();
                *( color++ ) = cmap[ color_level[1] ].green();
                *( color++ ) = cmap[ color_level[1] ].blue();
                // v2
                *( color++ ) = cmap[ color_level[2] ].red();
                *( color++ ) = cmap[ color_level[2] ].green();
                *( color++ ) = cmap[ color_level[2] ].blue();
                // v3
                *( color++ ) = cmap[ color_level[3] ].red();
                *( color++ ) = cmap[ color_level[3] ].green();
                *( color++ ) = cmap[ color_level[3] ].blue();
            }
        }
    }

    // YZ (X=0) plane.
    {
        const size_t i = 0;
        const size_t offset0 = i;
        for ( size_t j = 0, offset = offset0; j < ngrids.y(); j++, offset = offset0 + j * nnodes_per_line )
        {
            for ( size_t k = 0; k < ngrids.z(); k++, offset += nnodes_per_slice )
            {
                node_index[0] = offset;
                node_index[1] = node_index[0] + nnodes_per_slice;
                node_index[2] = node_index[1] + nnodes_per_line;
                node_index[3] = node_index[0] + nnodes_per_line;
                ::GetColorIndices<4>( value, min_value, max_value, veclen, cmap.resolution(), node_index, &color_level );
                // v0
                *( color++ ) = cmap[ color_level[0] ].red();
                *( color++ ) = cmap[ color_level[0] ].green();
                *( color++ ) = cmap[ color_level[0] ].blue();
                // v1
                *( color++ ) = cmap[ color_level[1] ].red();
                *( color++ ) = cmap[ color_level[1] ].green();
                *( color++ ) = cmap[ color_level[1] ].blue();
                // v2
                *( color++ ) = cmap[ color_level[2] ].red();
                *( color++ ) = cmap[ color_level[2] ].green();
                *( color++ ) = cmap[ color_level[2] ].blue();
                // v3
                *( color++ ) = cmap[ color_level[3] ].red();
                *( color++ ) = cmap[ color_level[3] ].green();
                *( color++ ) = cmap[ color_level[3] ].blue();
            }
        }
    }

    // YZ (X=ngrids.x()) plane.
    {
        const size_t i = ngrids.x();
        const size_t offset0 = i;
        for ( size_t j = 0, offset = offset0; j < ngrids.y(); j++, offset = offset0 + j * nnodes_per_line )
        {
            for ( size_t k = 0; k < ngrids.z(); k++, offset += nnodes_per_slice )
            {
                node_index[0] = offset;
                node_index[1] = node_index[0] + nnodes_per_slice;
                node_index[2] = node_index[1] + nnodes_per_line;
                node_index[3] = node_index[0] + nnodes_per_line;
                ::GetColorIndices<4>( value, min_value, max_value, veclen, cmap.resolution(), node_index, &color_level );
                // v3
                *( color++ ) = cmap[ color_level[3] ].red();
                *( color++ ) = cmap[ color_level[3] ].green();
                *( color++ ) = cmap[ color_level[3] ].blue();
                // v2
                *( color++ ) = cmap[ color_level[2] ].red();
                *( color++ ) = cmap[ color_level[2] ].green();
                *( color++ ) = cmap[ color_level[2] ].blue();
                // v1
                *( color++ ) = cmap[ color_level[1] ].red();
                *( color++ ) = cmap[ color_level[1] ].green();
                *( color++ ) = cmap[ color_level[1] ].blue();
                // v0
                *( color++ ) = cmap[ color_level[0] ].red();
                *( color++ ) = cmap[ color_level[0] ].green();
                *( color++ ) = cmap[ color_level[0] ].blue();
            }
        }
    }

    // XZ (Y=0) plane.
    {
        const size_t j = 0;
        const size_t offset0 = j * nnodes_per_line;
        for ( size_t k = 0, offset = offset0; k < ngrids.z(); k++, offset =  offset0 + k * nnodes_per_slice )
        {
            for ( size_t i = 0; i < ngrids.x(); i++, offset += 1 )
            {
                node_index[0] = offset;
                node_index[1] = node_index[0] + 1;
                node_index[2] = node_index[1] + nnodes_per_slice;
                node_index[3] = node_index[0] + nnodes_per_slice;
                ::GetColorIndices<4>( value, min_value, max_value, veclen, cmap.resolution(), node_index, &color_level );
                // v0
                *( color++ ) = cmap[ color_level[0] ].red();
                *( color++ ) = cmap[ color_level[0] ].green();
                *( color++ ) = cmap[ color_level[0] ].blue();
                // v1
                *( color++ ) = cmap[ color_level[1] ].red();
                *( color++ ) = cmap[ color_level[1] ].green();
                *( color++ ) = cmap[ color_level[1] ].blue();
                // v2
                *( color++ ) = cmap[ color_level[2] ].red();
                *( color++ ) = cmap[ color_level[2] ].green();
                *( color++ ) = cmap[ color_level[2] ].blue();
                // v3
                *( color++ ) = cmap[ color_level[3] ].red();
                *( color++ ) = cmap[ color_level[3] ].green();
                *( color++ ) = cmap[ color_level[3] ].blue();
            }
        }
    }

    // XZ (Y=ngrids.y()) plane.
    {
        const size_t j = ngrids.y();
        const size_t offset0 = j * nnodes_per_line;
        for ( size_t k = 0, offset = offset0; k < ngrids.z(); k++, offset = offset0 + k * nnodes_per_slice )
        {
            for ( size_t i = 0; i < ngrids.x(); i++, offset += 1 )
            {
                node_index[0] = offset;
                node_index[1] = node_index[0] + 1;
                node_index[2] = node_index[1] + nnodes_per_slice;
                node_index[3] = node_index[0] + nnodes_per_slice;
                ::GetColorIndices<4>( value, min_value, max_value, veclen, cmap.resolution(), node_index, &color_level );
                // v3
                *( color++ ) = cmap[ color_level[3] ].red();
                *( color++ ) = cmap[ color_level[3] ].green();
                *( color++ ) = cmap[ color_level[3] ].blue();
                // v2
                *( color++ ) = cmap[ color_level[2] ].red();
                *( color++ ) = cmap[ color_level[2] ].green();
                *( color++ ) = cmap[ color_level[2] ].blue();
                // v1
                *( color++ ) = cmap[ color_level[1] ].red();
                *( color++ ) = cmap[ color_level[1] ].green();
                *( color++ ) = cmap[ color_level[1] ].blue();
                // v0
                *( color++ ) = cmap[ color_level[0] ].red();
                *( color++ ) = cmap[ color_level[0] ].green();
                *( color++ ) = cmap[ color_level[0] ].blue();
            }
        }
    }

    SuperClass::setColors( colors );
}

/*===========================================================================*/
/**
 *  @brief  Mapping for a unstructured volume object.
 *  @param  volume [in] pointer to the unstrctured volume object
 */
/*===========================================================================*/
void ExternalFaces::mapping( const kvs::UnstructuredVolumeObject* volume )
{
    BaseClass::attach_volume( volume );
    BaseClass::set_range( volume );
    BaseClass::set_min_max_coords( volume, this );

    switch ( volume->cellType() )
    {
    case kvs::UnstructuredVolumeObject::Tetrahedra:
    {
        const std::type_info& type = volume->values().typeInfo()->type();
        if (      type == typeid( kvs::Int8   ) ) { this->calculate_tetrahedral_faces<kvs::Int8  >( volume ); }
        else if ( type == typeid( kvs::Int16  ) ) { this->calculate_tetrahedral_faces<kvs::Int16 >( volume ); }
        else if ( type == typeid( kvs::Int32  ) ) { this->calculate_tetrahedral_faces<kvs::Int32 >( volume ); }
        else if ( type == typeid( kvs::Int64  ) ) { this->calculate_tetrahedral_faces<kvs::Int64 >( volume ); }
        else if ( type == typeid( kvs::UInt8  ) ) { this->calculate_tetrahedral_faces<kvs::UInt8 >( volume ); }
        else if ( type == typeid( kvs::UInt16 ) ) { this->calculate_tetrahedral_faces<kvs::UInt16>( volume ); }
        else if ( type == typeid( kvs::UInt32 ) ) { this->calculate_tetrahedral_faces<kvs::UInt32>( volume ); }
        else if ( type == typeid( kvs::UInt64 ) ) { this->calculate_tetrahedral_faces<kvs::UInt64>( volume ); }
        else if ( type == typeid( kvs::Real32 ) ) { this->calculate_tetrahedral_faces<kvs::Real32>( volume ); }
        else if ( type == typeid( kvs::Real64 ) ) { this->calculate_tetrahedral_faces<kvs::Real64>( volume ); }
        break;
    }
    case kvs::UnstructuredVolumeObject::QuadraticTetrahedra:
    {
        const std::type_info& type = volume->values().typeInfo()->type();
        if (      type == typeid( kvs::Int8   ) ) { this->calculate_quadratic_tetrahedral_faces<kvs::Int8  >( volume ); }
        else if ( type == typeid( kvs::Int16  ) ) { this->calculate_quadratic_tetrahedral_faces<kvs::Int16 >( volume ); }
        else if ( type == typeid( kvs::Int32  ) ) { this->calculate_quadratic_tetrahedral_faces<kvs::Int32 >( volume ); }
        else if ( type == typeid( kvs::Int64  ) ) { this->calculate_quadratic_tetrahedral_faces<kvs::Int64 >( volume ); }
        else if ( type == typeid( kvs::UInt8  ) ) { this->calculate_quadratic_tetrahedral_faces<kvs::UInt8 >( volume ); }
        else if ( type == typeid( kvs::UInt16 ) ) { this->calculate_quadratic_tetrahedral_faces<kvs::UInt16>( volume ); }
        else if ( type == typeid( kvs::UInt32 ) ) { this->calculate_quadratic_tetrahedral_faces<kvs::UInt32>( volume ); }
        else if ( type == typeid( kvs::UInt64 ) ) { this->calculate_quadratic_tetrahedral_faces<kvs::UInt64>( volume ); }
        else if ( type == typeid( kvs::Real32 ) ) { this->calculate_quadratic_tetrahedral_faces<kvs::Real32>( volume ); }
        else if ( type == typeid( kvs::Real64 ) ) { this->calculate_quadratic_tetrahedral_faces<kvs::Real64>( volume ); }
        break;
    }
    case kvs::UnstructuredVolumeObject::Hexahedra:
    {
        const std::type_info& type = volume->values().typeInfo()->type();
        if (      type == typeid( kvs::Int8   ) ) { this->calculate_hexahedral_faces<kvs::Int8  >( volume ); }
        else if ( type == typeid( kvs::Int16  ) ) { this->calculate_hexahedral_faces<kvs::Int16 >( volume ); }
        else if ( type == typeid( kvs::Int32  ) ) { this->calculate_hexahedral_faces<kvs::Int32 >( volume ); }
        else if ( type == typeid( kvs::Int64  ) ) { this->calculate_hexahedral_faces<kvs::Int64 >( volume ); }
        else if ( type == typeid( kvs::UInt8  ) ) { this->calculate_hexahedral_faces<kvs::UInt8 >( volume ); }
        else if ( type == typeid( kvs::UInt16 ) ) { this->calculate_hexahedral_faces<kvs::UInt16>( volume ); }
        else if ( type == typeid( kvs::UInt32 ) ) { this->calculate_hexahedral_faces<kvs::UInt32>( volume ); }
        else if ( type == typeid( kvs::UInt64 ) ) { this->calculate_hexahedral_faces<kvs::UInt64>( volume ); }
        else if ( type == typeid( kvs::Real32 ) ) { this->calculate_hexahedral_faces<kvs::Real32>( volume ); }
        else if ( type == typeid( kvs::Real64 ) ) { this->calculate_hexahedral_faces<kvs::Real64>( volume ); }
        break;
    }
    case kvs::UnstructuredVolumeObject::QuadraticHexahedra:
    {
        const std::type_info& type = volume->values().typeInfo()->type();
        if (      type == typeid( kvs::Int8   ) ) { this->calculate_quadratic_hexahedral_faces<kvs::Int8  >( volume ); }
        else if ( type == typeid( kvs::Int16  ) ) { this->calculate_quadratic_hexahedral_faces<kvs::Int16 >( volume ); }
        else if ( type == typeid( kvs::Int32  ) ) { this->calculate_quadratic_hexahedral_faces<kvs::Int32 >( volume ); }
        else if ( type == typeid( kvs::Int64  ) ) { this->calculate_quadratic_hexahedral_faces<kvs::Int64 >( volume ); }
        else if ( type == typeid( kvs::UInt8  ) ) { this->calculate_quadratic_hexahedral_faces<kvs::UInt8 >( volume ); }
        else if ( type == typeid( kvs::UInt16 ) ) { this->calculate_quadratic_hexahedral_faces<kvs::UInt16>( volume ); }
        else if ( type == typeid( kvs::UInt32 ) ) { this->calculate_quadratic_hexahedral_faces<kvs::UInt32>( volume ); }
        else if ( type == typeid( kvs::UInt64 ) ) { this->calculate_quadratic_hexahedral_faces<kvs::UInt64>( volume ); }
        else if ( type == typeid( kvs::Real32 ) ) { this->calculate_quadratic_hexahedral_faces<kvs::Real32>( volume ); }
        else if ( type == typeid( kvs::Real64 ) ) { this->calculate_quadratic_hexahedral_faces<kvs::Real64>( volume ); }
        break;
    }
    default:
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Not supported cell type.");
        break;
    }
    }

    SuperClass::setPolygonType( kvs::PolygonObject::Triangle );
    SuperClass::setColorType( kvs::PolygonObject::VertexColor );
    SuperClass::setNormalType( kvs::PolygonObject::PolygonNormal );
    if ( SuperClass::nopacities() == 0 ) SuperClass::setOpacity( 255 );
}

/*===========================================================================*/
/**
 *  @brief  Calculates external faces for the tetrahedral cells.
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename T>
void ExternalFaces::calculate_tetrahedral_faces( const kvs::UnstructuredVolumeObject* volume )
{
    ::FaceMap face_map( volume->nnodes() );
    CreateTetrahedraFaceMap( volume, &face_map );

    kvs::ValueArray<kvs::Real32> coords;
    kvs::ValueArray<kvs::UInt8> colors;
    kvs::ValueArray<kvs::Real32> normals;
    ::CalculateFaces<T>( volume, BaseClass::colorMap(), face_map, &coords, &colors, &normals );

    SuperClass::setCoords( coords );
    SuperClass::setColors( colors );
    SuperClass::setNormals( normals );
}

/*===========================================================================*/
/**
 *  @brief  Calculates external faces for the quadratic tetrahedral cells.
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename T>
void ExternalFaces::calculate_quadratic_tetrahedral_faces( const kvs::UnstructuredVolumeObject* volume )
{
    ::FaceMap face_map( volume->nnodes() );
    CreateQuadraticTetrahedraFaceMap( volume, &face_map );

    kvs::ValueArray<kvs::Real32> coords;
    kvs::ValueArray<kvs::UInt8> colors;
    kvs::ValueArray<kvs::Real32> normals;
    ::CalculateFaces<T>( volume, BaseClass::colorMap(), face_map, &coords, &colors, &normals );

    SuperClass::setCoords( coords );
    SuperClass::setColors( colors );
    SuperClass::setNormals( normals );
}

/*===========================================================================*/
/**
 *  @brief  Calculates external faces for the hexahedral cells.
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename T>
void ExternalFaces::calculate_hexahedral_faces( const kvs::UnstructuredVolumeObject* volume )
{
    ::FaceMap face_map( volume->nnodes() );
    CreateHexahedraFaceMap( volume, &face_map );

    kvs::ValueArray<kvs::Real32> coords;
    kvs::ValueArray<kvs::UInt8> colors;
    kvs::ValueArray<kvs::Real32> normals;
    ::CalculateFaces<T>( volume, BaseClass::colorMap(), face_map, &coords, &colors, &normals );

    SuperClass::setCoords( coords );
    SuperClass::setColors( colors );
    SuperClass::setNormals( normals );
}

/*===========================================================================*/
/**
 *  @brief  Calculates external faces for the quadratic hexahedral cells.
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename T>
void ExternalFaces::calculate_quadratic_hexahedral_faces( const kvs::UnstructuredVolumeObject* volume )
{
    ::FaceMap face_map( volume->nnodes() );
    CreateQuadraticHexahedraFaceMap( volume, &face_map );

    kvs::ValueArray<kvs::Real32> coords;
    kvs::ValueArray<kvs::UInt8> colors;
    kvs::ValueArray<kvs::Real32> normals;
    ::CalculateFaces<T>( volume, BaseClass::colorMap(), face_map, &coords, &colors, &normals );

    SuperClass::setCoords( coords );
    SuperClass::setColors( colors );
    SuperClass::setNormals( normals );
}

} // end of namespace kvs
