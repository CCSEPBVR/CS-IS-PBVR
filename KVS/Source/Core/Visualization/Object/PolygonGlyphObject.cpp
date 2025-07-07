/****************************************************************************/
/**
 *  @file   PolygonGlyphObject.cpp
 *  @author TO0603
 */
/****************************************************************************/
#include "PolygonGlyphObject.h"
#include <kvs/StructuredVolumeObject>
#include <kvs/Quaternion>
#define M_PI 3.14159265358979323846

namespace kvs
{

kvs::Real32 PolygonGlyphObject::m_default_size = 1.0f;
kvs::Vec3 PolygonGlyphObject::m_default_direction = kvs::Vec3{ 0.0, 1.0, 0.0 };
kvs::RGBColor PolygonGlyphObject::m_default_color = kvs::RGBColor{ 255, 255, 255 };
kvs::UInt8 PolygonGlyphObject::m_default_opacity = 255;

/*===========================================================================*/
/**
 *  @brief  Constructs a new PolygonGlyphObject class.
 */
/*===========================================================================*/
kvs::PolygonGlyphObject::PolygonGlyphObject()
{

}

/*===========================================================================*/
/**
 *  @brief Constructs a new PolygonGlyphObject class.
 *  @param volume [in] pointer to the Volume object
 *  @param tfunc [in] transfer function
 *  @param type The type of glyph to be constructed.
 */
/*===========================================================================*/
kvs::PolygonGlyphObject::PolygonGlyphObject(
    const kvs::VolumeObjectBase* volume,
    const kvs::TransferFunction& transfer_function,
    const GlyphType type ):
    m_type( type )
{
    setTransferFunction( transfer_function );
    this->attach_volume( volume );

    switch ( m_type ) {
    case Arrow:
        createArrowGlyphPolygon();
        break;
    case Diamond:
        createDiamondGlyphPolygon();
        break;
    case Sphere:
        createSphereGlyphPolygon();
        break;
    default:
        createArrowGlyphPolygon();
        break;
    }
}

/*===========================================================================*/
/**
 *  @brief Constructs a new PolygonGlyphObject class.
 *  @param coords      Array of coordinates representing glyph positions.
 *  @param directions  Array of direction vectors for glyph orientation.
 *  @param sizes       Array of sizes for each glyph.
 *  @param colors      Array of colors for each glyph, specified as RGBA values.
 *  @param type        The type of glyph to be constructed.
 */
/*===========================================================================*/
kvs::PolygonGlyphObject::PolygonGlyphObject( const kvs::ValueArray<kvs::Real32> coords,
                                            const kvs::ValueArray<kvs::Real32> directions,
                                            const kvs::ValueArray<kvs::Real32> sizes,
                                            const kvs::ValueArray<kvs::UInt8> colors,
                                            const GlyphType type ):
    m_coords( coords ),
    m_directions( directions ),
    m_sizes( sizes ),
    m_colors( colors ),
    m_type( type ),
    m_volume( nullptr )
{
    switch ( m_type ) {
    case Arrow:
        createArrowGlyphPolygon();
        break;
    case Diamond:
        createDiamondGlyphPolygon();
        break;
    case Sphere:
        createSphereGlyphPolygon();
        break;
    default:
        createArrowGlyphPolygon();
        break;
    }
}

/*===========================================================================*/
/**
 *  @brief  Attaches a volume object.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
void kvs::PolygonGlyphObject::attach_volume( const kvs::VolumeObjectBase* volume )
{
    m_volume = volume;

    this->calculateCoords( volume );

    const std::type_info& type = volume->values().typeInfo()->type();
    if ( type == typeid( kvs::Int8 ) )
    {
        this->calculateSizes<kvs::Int8>( volume );
        this->calculateDirections<kvs::Int8>( volume );
        this->calculateColors<kvs::Int8>( volume );
        this->calculateOpacities<kvs::Int8>( volume );
    }
    else if ( type == typeid( kvs::Int16 ) )
    {
        this->calculateSizes<kvs::Int16>( volume );
        this->calculateDirections<kvs::Int16>( volume );
        this->calculateColors<kvs::Int16>( volume );
        this->calculateOpacities<kvs::Int16>( volume );
    }
    else if ( type == typeid( kvs::Int32 ) )
    {
        this->calculateSizes<kvs::Int32>( volume );
        this->calculateDirections<kvs::Int32>( volume );
        this->calculateColors<kvs::Int32>( volume );
        this->calculateOpacities<kvs::Int32>( volume );
    }
    else if ( type == typeid( kvs::Int64 ) )
    {
        this->calculateSizes<kvs::Int64>( volume );
        this->calculateDirections<kvs::Int64>( volume );
        this->calculateColors<kvs::Int64>( volume );
        this->calculateOpacities<kvs::Int64>( volume );
    }
    else if ( type == typeid( kvs::UInt8  ) )
    {
        this->calculateSizes<kvs::UInt8>( volume );
        this->calculateDirections<kvs::UInt8>( volume );
        this->calculateColors<kvs::UInt8>( volume );
        this->calculateOpacities<kvs::UInt8>( volume );
    }
    else if ( type == typeid( kvs::UInt16 ) )
    {
        this->calculateSizes<kvs::UInt16>( volume );
        this->calculateDirections<kvs::UInt16>( volume );
        this->calculateColors<kvs::UInt16>( volume );
        this->calculateOpacities<kvs::UInt16>( volume );
    }
    else if ( type == typeid( kvs::UInt32 ) )
    {
        this->calculateSizes<kvs::UInt32>( volume );
        this->calculateDirections<kvs::UInt32>( volume );
        this->calculateColors<kvs::UInt32>( volume );
        this->calculateOpacities<kvs::UInt32>( volume );
    }
    else if ( type == typeid( kvs::UInt64 ) )
    {
        this->calculateSizes<kvs::UInt64>( volume );
        this->calculateDirections<kvs::UInt64>( volume );
        this->calculateColors<kvs::UInt64>( volume );
        this->calculateOpacities<kvs::UInt64>( volume );
    }
    else if ( type == typeid( kvs::Real32 ) )
    {
        this->calculateSizes<kvs::Real32>( volume );
        this->calculateDirections<kvs::Real32>( volume );
        this->calculateColors<kvs::Real32>( volume );
        this->calculateOpacities<kvs::Real32>( volume );
    }
    else if ( type == typeid( kvs::Real64 ) )
    {
        this->calculateSizes<kvs::Real64>( volume );
        this->calculateDirections<kvs::Real64>( volume );
        this->calculateColors<kvs::Real64>( volume );
        this->calculateOpacities<kvs::Real64>( volume );
    }
}

/*===========================================================================*/
/**
 *  @brief  Creates a polygon for rendering arrow glyphs.
 *  @details This function generates the necessary polygons to represent
 *           arrow-shaped glyphs based on the object's internal data such
 *           as coordinates, directions, sizes, and colors.
 *  @note The min and max coordinates are obtained from the volume only
 *        when generating a polygon object from a volume object.
 */
/*===========================================================================*/
void kvs::PolygonGlyphObject::createArrowGlyphPolygon()
{
    const size_t npoint = this->coords().size() / 3;
    const kvs::ValueArray<kvs::Real32> coords = this->coords();
    const kvs::ValueArray<kvs::UInt8> colors = this->colors();
    const kvs::ValueArray<kvs::Real32> sizes = this->sizes();
    const kvs::ValueArray<kvs::UInt8> opacities = this->opacities();

    std::vector<kvs::Vec3> all_vertices;
    std::vector<kvs::Vec3> all_normals;
    std::vector<kvs::UInt32> all_indices;
    std::vector<kvs::UInt8> all_colors;

    for ( size_t i = 0, index = 0; i < npoint; i++, index += 3 )
    {
        kvs::Vec3 position( coords.data() + index );
        kvs::Vec3 direction( this->directions().data() + index );
        kvs::Real32 size = sizes[i];
        kvs::RGBColor color( colors.data() + index );

        if ( direction.length() < 1e-6 )
        {
            std::cerr << "Error: Invalid direction vector. Length is near zero." << std::endl;
            // return nullptr;
        }

        direction = direction.normalized();
        const int slices = 20;

        for( int i = 0; i < slices * 3; i++ )
        {
            {
                all_colors.push_back( color.r() );
                all_colors.push_back( color.g() );
                all_colors.push_back( color.b() );
            }
        }

        const float cylinder_radius = 0.07f * size;
        const float cylinder_height = 0.7f * size;

        std::vector<kvs::Vec3> vertices;
        std::vector<kvs::Vec3> normals;
        std::vector<kvs::UInt32> indices;

        // 円柱の構築
        for ( int i = 0; i < slices; ++i )
        {
            float angle = 2.0f * M_PI * i / slices;
            float x = cylinder_radius * std::cos( angle );
            float y = cylinder_radius * std::sin( angle );

            vertices.emplace_back( kvs::Vec3( x, y, 0.0f ) );
            normals.emplace_back( kvs::Vec3( x, y, 0.0f ).normalized() );

            vertices.emplace_back( kvs::Vec3( x, y, cylinder_height ) );
            normals.emplace_back( kvs::Vec3( x, y, 0.0f ).normalized() );
        }

        for ( int i = 0; i < slices; ++i )
        {
            int next = ( i + 1 ) % slices;

            indices.push_back( i * 2 );
            indices.push_back( next * 2 );
            indices.push_back( i * 2 + 1 );

            indices.push_back( i * 2 + 1 );
            indices.push_back( next * 2 );
            indices.push_back( next * 2 + 1 );
        }

        // 円錐の構築
        const float cone_radius = 0.15f * size;
        const float cone_height = 0.3f * size;
        const float angle_step = 2.0f * M_PI / slices;

        std::vector<kvs::Vec3> base_vertices;
        for (int i = 0; i < slices; ++i)
        {
            float angle = angle_step * i;
            float x = cone_radius * std::cos(angle);
            float y = cone_radius * std::sin(angle);
            base_vertices.emplace_back(x, y, cylinder_height);
        }

        for (int i = 0; i < slices; ++i)
        {
            int next = (i + 1) % slices;
            kvs::Vec3 v1 = base_vertices[i];
            kvs::Vec3 v2 = base_vertices[next];
            kvs::Vec3 apex(0.0f, 0.0f, cylinder_height + cone_height);

            vertices.push_back(v1);
            vertices.push_back(v2);
            vertices.push_back(apex);

            kvs::Vec3 edge1 = v2 - v1;
            kvs::Vec3 edge2 = apex - v1;
            kvs::Vec3 normal = edge1.cross(edge2).normalized();

            normals.push_back(normal);
            normals.push_back(normal);
            normals.push_back(normal);

            int base_index = vertices.size() - 3;
            indices.push_back(base_index);
            indices.push_back(base_index + 1);
            indices.push_back(base_index + 2);
        }

        kvs::Vec3 default_direction( 0.0f, 0.0f, 1.0f );
        kvs::Vec3 axis = default_direction.cross( direction );
        float angle = std::acos( default_direction.dot( direction ) );
        float degrees = angle * 180.0f / M_PI;

        kvs::Mat3 rotation;
        if ( axis.length() > 1e-6 )
        {
            rotation = kvs::Mat3::Rotation( axis.normalized(), degrees );
        }
        else
        {
            rotation = kvs::Mat3::Identity();
        }

        for ( auto& vertex : vertices )
        {
            vertex = rotation * vertex + position;
        }
        for ( auto& normal : normals )
        {
            normal = rotation * normal;
        }

        size_t index_offset = all_vertices.size();
        all_vertices.insert( all_vertices.end(), vertices.begin(), vertices.end() );
        all_normals.insert( all_normals.end(), normals.begin(), normals.end() );

        for ( auto index : indices )
        {
            all_indices.push_back( index + index_offset );
        }
    }

    BaseClass::setCoords( kvs::ValueArray<kvs::Real32>( ( kvs::Real32* )all_vertices.data(), all_vertices.size() * 3 ) );
    BaseClass::setConnections( kvs::ValueArray<kvs::UInt32>( all_indices.data(), all_indices.size() ) );
    BaseClass::setColors( kvs::ValueArray<kvs::UInt8>( all_colors.data(), all_colors.size() ) );
    BaseClass::setNormals( kvs::ValueArray<kvs::Real32>( ( kvs::Real32* )all_normals.data(), all_normals.size() * 3 ) );
    BaseClass::setOpacity( 255 );
    // BaseClass::setOpacity( int(m_opacity) );
    BaseClass::setPolygonType( kvs::PolygonObject::Triangle );
    BaseClass::setColorType( kvs::PolygonObject::PolygonColor );
    BaseClass::setNormalType( kvs::PolygonObject::VertexNormal );
    if( m_volume != nullptr )
    {
        BaseClass::setMinMaxObjectCoords( m_volume->minObjectCoord(), m_volume->maxObjectCoord() );
    }
}

/*===========================================================================*/
/**
 *  @brief  Creates a polygon for rendering diamond glyphs.
 *  @details This function generates the necessary polygons to represent
 *           diamond-shaped glyphs based on the object's internal data such
 *           as coordinates, directions, sizes, and colors.
 *  @note The min and max coordinates are obtained from the volume only
 *        when generating a polygon object from a volume object.
 */
/*===========================================================================*/
void kvs::PolygonGlyphObject::createDiamondGlyphPolygon()
{
    const size_t npoint = this->coords().size() / 3;
    const kvs::ValueArray<kvs::Real32> coords = this->coords();
    const kvs::ValueArray<kvs::UInt8> colors = this->colors();
    const kvs::ValueArray<kvs::Real32> sizes = this->sizes();
    const kvs::ValueArray<kvs::UInt8> opacities = this->opacities();

    std::vector<kvs::Vec3> all_vertices;
    std::vector<kvs::Vec3> all_normals;
    std::vector<kvs::UInt32> all_indices;
    std::vector<kvs::UInt8> all_colors;

    for ( size_t i = 0, index = 0; i < npoint; i++, index += 3 )
    {
        kvs::Vec3 position( coords.data() + index );
        kvs::Vec3 direction( this->directions().data() + index );
        kvs::Real32 size = sizes[i];
        kvs::RGBColor color( colors.data() + index );

        const kvs::Real32 Vertices[18] =
            {
                0.5f * size,  0.0f,          0.0f,
                0.0f,         0.0f,         -0.5f * size,
                -0.5f * size,  0.0f,          0.0f,
                0.0f,         0.0f,          0.5f * size,
                0.0f,         0.5f * size,   0.0f,
                0.0f,        -0.5f * size,   0.0f
            };

        const kvs::UInt32 Connections[24] =
            {
                4, 0, 1,
                4, 1, 2,
                4, 2, 3,
                4, 3, 0,
                5, 1, 0,
                5, 2, 1,
                5, 3, 2,
                5, 0, 3
            };

        std::vector<kvs::Vec3> vertices;
        for (int j = 0; j < 6; ++j)
        {
            vertices.emplace_back(Vertices[j * 3], Vertices[j * 3 + 1], Vertices[j * 3 + 2]);
        }

        std::vector<kvs::Vec3> normals;
        for (size_t j = 0; j < 24; j += 3)
        {
            kvs::Vec3 v0(vertices[Connections[j]]);
            kvs::Vec3 v1(vertices[Connections[j + 1]]);
            kvs::Vec3 v2(vertices[Connections[j + 2]]);
            kvs::Vec3 edge1 = v1 - v0;
            kvs::Vec3 edge2 = v2 - v0;
            normals.push_back(edge1.cross(edge2).normalized());
        }

        // 回転および位置の調整
        kvs::Vec3 default_direction(0.0f, 1.0f, 0.0f); // デフォルトの方向
        const auto v = direction.normalized();         // 正規化された方向ベクトル

        kvs::Mat3 rotation; // 回転行列

        // `direction` が `-default_direction` の場合
        if (v == -default_direction)
        {
            rotation = kvs::Mat3::Identity(); // 単位行列（反転不要）
        }
        else
        {
            // 回転軸（クロス積）
            const auto c = default_direction.cross(v);

            // 回転角度のコサイン値（ドット積）
            const auto d = default_direction.dot(v);

            // 四元数を計算
            const auto s = static_cast<float>(std::sqrt((1.0 + d) * 2.0));
            const auto q = kvs::Quaternion(c.x() / s, c.y() / s, c.z() / s, s / 2.0f);

            // 四元数から回転行列を生成
            rotation = q.toMatrix();
        }

        for (auto& vertex : vertices)
        {
            // 回転を適用
            vertex = rotation * vertex;

            // 位置を調整
            vertex += position;
        }

        for (auto& normal : normals)
        {
            normal = rotation * normal;
        }

        size_t index_offset = all_vertices.size();
        all_vertices.insert(all_vertices.end(), vertices.begin(), vertices.end());
        all_normals.insert(all_normals.end(), normals.begin(), normals.end());

        for (size_t j = 0; j < 24; ++j)
        {
            all_indices.push_back(Connections[j] + index_offset);
        }

        // 色設定
        for (int j = 0; j < 6; ++j)
        {
            all_colors.push_back( color.r() );
            all_colors.push_back( color.g() );
            all_colors.push_back( color.b() );
        }
    }

    BaseClass::setCoords( kvs::ValueArray<kvs::Real32>( ( kvs::Real32* )all_vertices.data(), all_vertices.size() * 3 ) );
    BaseClass::setConnections( kvs::ValueArray<kvs::UInt32>( all_indices.data(), all_indices.size() ) );
    BaseClass::setColors( kvs::ValueArray<kvs::UInt8>( all_colors.data(), all_colors.size() ) );
    BaseClass::setNormals( kvs::ValueArray<kvs::Real32>( ( kvs::Real32* )all_normals.data(), all_normals.size() * 3 ) );
    BaseClass::setOpacity( 255 );
    BaseClass::setPolygonType( kvs::PolygonObject::Triangle );
    BaseClass::setColorType( kvs::PolygonObject::VertexColor );
    BaseClass::setNormalType( kvs::PolygonObject::PolygonNormal );
    if( m_volume != nullptr )
    {
        BaseClass::setMinMaxObjectCoords( m_volume->minObjectCoord(), m_volume->maxObjectCoord() );
    }
    else
    {
        BaseClass::setMinMaxObjectCoords( BaseClass::minObjectCoord(), BaseClass::maxObjectCoord() );
    }
}

/*===========================================================================*/
/**
 *  @brief  Creates a polygon for rendering sphere glyphs.
 *  @details This function generates the necessary polygons to represent
 *           sphere-shaped glyphs based on the object's internal data such
 *           as coordinates, directions, sizes, and colors.
 *  @note The min and max coordinates are obtained from the volume only
 *        when generating a polygon object from a volume object.
 */
/*===========================================================================*/
void kvs::PolygonGlyphObject::createSphereGlyphPolygon()
{
    const size_t npoint = this->coords().size() / 3;
    const kvs::ValueArray<kvs::Real32> coords = this->coords();
    const kvs::ValueArray<kvs::UInt8> colors = this->colors();
    const kvs::ValueArray<kvs::Real32> sizes = this->sizes();
    const kvs::ValueArray<kvs::UInt8> opacities = this->opacities();

    const float radius = 0.5f; // スフィアの半径
    const int slices = 10;    // スフィアの横分割数
    const int stacks = 10;    // スフィアの縦分割数
    std::vector<kvs::Vec3> all_vertices;
    std::vector<kvs::Vec3> all_normals;
    std::vector<kvs::UInt32> all_indices;
    std::vector<kvs::UInt8> all_colors;

    for (size_t i = 0, index = 0; i < npoint; i++, index += 3)
    {
        kvs::Vec3 position(coords.data() + index);
        kvs::Real32 size = sizes[i];
        kvs::RGBColor color(colors.data() + index);

        // 頂点と法線の生成
        std::vector<std::vector<size_t>> vertex_indices(stacks + 1);
        for (int stack = 0; stack <= stacks; ++stack)
        {
            float phi = M_PI * stack / stacks; // 緯度
            for (int slice = 0; slice <= slices; ++slice)
            {
                float theta = 2.0f * M_PI * slice / slices; // 経度

                // 球の座標計算
                float x = radius * size * std::sin(phi) * std::cos(theta);
                float y = radius * size * std::cos(phi);
                float z = radius * size * std::sin(phi) * std::sin(theta);

                kvs::Vec3 vertex = position + kvs::Vec3(x, y, z);
                kvs::Vec3 normal = kvs::Vec3(x, y, z).normalized();

                all_vertices.push_back(vertex);
                all_normals.push_back(normal);

                // 頂点インデックスを保存
                vertex_indices[stack].push_back(all_vertices.size() - 1);

                // 色情報
                all_colors.push_back( color.r() );
                all_colors.push_back( color.g() );
                all_colors.push_back( color.b() );
            }
        }

        // インデックスの生成
        for (int stack = 0; stack < stacks; ++stack)
        {
            for (int slice = 0; slice < slices; ++slice)
            {
                // 上部三角形
                all_indices.push_back(vertex_indices[stack][slice]);
                all_indices.push_back(vertex_indices[stack + 1][slice]);
                all_indices.push_back(vertex_indices[stack + 1][slice + 1]);

                // 下部三角形
                all_indices.push_back(vertex_indices[stack][slice]);
                all_indices.push_back(vertex_indices[stack + 1][slice + 1]);
                all_indices.push_back(vertex_indices[stack][slice + 1]);
            }
        }
    }

    BaseClass::setCoords( kvs::ValueArray<kvs::Real32>( ( kvs::Real32* )all_vertices.data(), all_vertices.size() * 3 ) );
    BaseClass::setConnections( kvs::ValueArray<kvs::UInt32>( all_indices.data(), all_indices.size() ) );
    BaseClass::setColors( kvs::ValueArray<kvs::UInt8>( all_colors.data(), all_colors.size() ) );
    BaseClass::setNormals( kvs::ValueArray<kvs::Real32>( ( kvs::Real32* )all_normals.data(), all_normals.size() * 3 ) );
    BaseClass::setOpacity( 255 );
    BaseClass::setPolygonType( kvs::PolygonObject::Triangle );
    BaseClass::setColorType( kvs::PolygonObject::VertexColor );
    BaseClass::setNormalType( kvs::PolygonObject::VertexNormal );
    if( m_volume != nullptr )
    {
        BaseClass::setMinMaxObjectCoords( m_volume->minObjectCoord(), m_volume->maxObjectCoord() );
    }
}

// GlyphBase
/*===========================================================================*/
/**
 *  @brief  Calculates the coordinate value array.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
void PolygonGlyphObject::calculateCoords( const kvs::VolumeObjectBase* volume )
{
    const auto type = volume->volumeType();
    switch ( type )
    {
    case kvs::VolumeObjectBase::Structured:
    {
        this->calculateCoords( kvs::StructuredVolumeObject::DownCast( volume ) );
        break;
    }
    default: // kvs::VolumeObjectBase::Unstructured
    {
        this->setCoords( volume->coords() );
        break;
    }
    }
}

/*===========================================================================*/
/**
 *  @brief  Calculates the coordinate value array of the structured volume object.
 *  @param  volume [in] pointer to the structured volume object
 */
/*===========================================================================*/
void PolygonGlyphObject::calculateCoords( const kvs::StructuredVolumeObject* volume )
{
    const auto type = volume->gridType();
    switch ( type )
    {
    case kvs::StructuredVolumeObject::Uniform:
    {
        this->calculateUniformCoords( volume );
        break;
    }
    case kvs::StructuredVolumeObject::Rectilinear:
    {
        this->calculateRectilinearCoords( volume );
        break;
    }
    default: // kvs::StructuredVolumeObject::Curvilinear
    {
        this->setCoords( volume->coords() );
        break;
    }
    }
}

/*===========================================================================*/
/**
 *  @brief  Calculates the coordinate value array of the uniform type volume object.
 *  @param  volume [in] pointer to the structrued volume object
 */
/*===========================================================================*/
void PolygonGlyphObject::calculateUniformCoords( const kvs::StructuredVolumeObject* volume )
{
    kvs::ValueArray<kvs::Real32> coords( 3 * volume->numberOfNodes() );
    kvs::Real32* coord = coords.data();

    const auto resolution( volume->resolution() );
    const auto volume_size( volume->maxExternalCoord() - volume->minExternalCoord() );
    const auto ngrids( resolution - kvs::Vec3ui( 1, 1, 1 ) );
    const auto grid_size =
        kvs::Vec3
        {
            ( ngrids.x() == 0 ) ? 0.0f : volume_size.x() / static_cast<float>( ngrids.x() ),
            ( ngrids.y() == 0 ) ? 0.0f : volume_size.y() / static_cast<float>( ngrids.y() ),
            ( ngrids.z() == 0 ) ? 0.0f : volume_size.z() / static_cast<float>( ngrids.z() )
        };

    for ( size_t k = 0; k < resolution.z(); ++k )
    {
        const float z = grid_size.z() * static_cast<float>( k );
        for ( size_t j = 0; j < resolution.y(); ++j )
        {
            const float y = grid_size.y() * static_cast<float>( j );
            for ( size_t i = 0; i < resolution.x(); ++i )
            {
                const float x = grid_size.x() * static_cast<float>( i );
                *( coord++ ) = x;
                *( coord++ ) = y;
                *( coord++ ) = z;
            }
        }
    }

    this->setCoords( coords );
}

/*===========================================================================*/
/**
 *  @brief  Calculates the coordinate value array of the rectilinear type volume object.
 *  @param  volume [in] pointer to the structrued volume object
 */
/*===========================================================================*/
void PolygonGlyphObject::calculateRectilinearCoords( const kvs::StructuredVolumeObject* volume )
{
    kvs::ValueArray<kvs::Real32> coords( 3 * volume->numberOfNodes() );
    auto* coord = coords.data();

    const auto dimx = volume->resolution().x();
    const auto dimy = volume->resolution().y();
    const auto dimz = volume->resolution().z();

    const auto* xcoord = volume->coords().data();
    const auto* ycoord = xcoord + dimx;
    const auto* zcoord = ycoord + dimy;

    for ( size_t k = 0; k < dimz; ++k )
    {
        const auto z = zcoord[k];
        for ( size_t j = 0; j < dimy; ++j )
        {
            const auto y = ycoord[j];
            for ( size_t i = 0; i < dimx; ++i )
            {
                const auto x = xcoord[i];
                *( coord++ ) = x;
                *( coord++ ) = y;
                *( coord++ ) = z;
            }
        }
    }

    this->setCoords( coords );
}

/*===========================================================================*/
/**
 *  @brief  Calculates the size value array.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
template <typename T>
void PolygonGlyphObject::calculateSizes( const kvs::VolumeObjectBase* volume )
{
    const auto values = volume->values().asValueArray<T>();
    const size_t veclen = volume->veclen();
    const size_t nnodes = volume->numberOfNodes();

    if ( !volume->hasMinMaxValues() ) { volume->updateMinMaxValues(); }
    const kvs::Real32 min_value = static_cast<kvs::Real32>( volume->minValue() );
    const kvs::Real32 max_value = static_cast<kvs::Real32>( volume->maxValue() );
    const kvs::Real32 normalize = 1.0f / ( max_value - min_value );

    kvs::ValueArray<kvs::Real32> sizes( nnodes );
    auto* size = sizes.data();

    switch ( m_size_mode )
    {
    case PolygonGlyphObject::SizeByDefault:
    {
        sizes.fill( DefaultSize() );
        break;
    }
    case PolygonGlyphObject::SizeByMagnitude:
    {
        if ( veclen == 1 )
        {
            for ( size_t i = 0; i < nnodes; i++ )
            {
                size[i] = DefaultSize() * normalize * ( static_cast<float>(values[i]) - min_value );
            }
        }
        else if ( veclen == 3 )
        {
            for( size_t i = 0, index = 0; i < nnodes; i++, index += 3 )
            {
                const kvs::Vec3 v
                    {
                        static_cast<float>(values[index]),
                        static_cast<float>(values[index+1]),
                        static_cast<float>(values[index+2])
                    };
                size[i] = DefaultSize() * normalize * ( static_cast<float>(v.length()) - min_value );
            }
        }
        break;
    }
    default: break;
    }

    this->setSizes( sizes );
}

template void PolygonGlyphObject::calculateSizes<kvs::Int8>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateSizes<kvs::Int16>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateSizes<kvs::Int32>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateSizes<kvs::Int64>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateSizes<kvs::UInt8>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateSizes<kvs::UInt16>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateSizes<kvs::UInt32>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateSizes<kvs::UInt64>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateSizes<kvs::Real32>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateSizes<kvs::Real64>( const kvs::VolumeObjectBase* volume );

/*===========================================================================*/
/**
 *  @brief  Calculates the direction vector array.
 *  @param  volume [in] pointer to the structured volume object
 */
/*===========================================================================*/
template <typename T>
void PolygonGlyphObject::calculateDirections( const kvs::VolumeObjectBase* volume )
{
    const auto values = volume->values().asValueArray<T>();
    const size_t veclen = volume->veclen();
    const size_t nnodes = volume->numberOfNodes();
    if ( veclen == 3 )
    {
        kvs::ValueArray<kvs::Real32> directions( nnodes * veclen );
        kvs::Real32* direction = directions.data();

        for ( size_t i = 0; i < directions.size(); i++ )
        {
            direction[i] = static_cast<kvs::Real32>( values[i] );
        }

        this->setDirections( directions );
    }
}

template void PolygonGlyphObject::calculateDirections<kvs::Int8>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateDirections<kvs::Int16>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateDirections<kvs::Int32>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateDirections<kvs::Int64>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateDirections<kvs::UInt8>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateDirections<kvs::UInt16>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateDirections<kvs::UInt32>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateDirections<kvs::UInt64>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateDirections<kvs::Real32>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateDirections<kvs::Real64>( const kvs::VolumeObjectBase* volume );

/*===========================================================================*/
/**
 *  @brief  Calculates the color value array.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
template <typename T>
void PolygonGlyphObject::calculateColors( const kvs::VolumeObjectBase* volume )
{
    const auto values = volume->values().asValueArray<T>();
    const size_t veclen = volume->veclen();
    const size_t nnodes = volume->numberOfNodes();

    if ( !volume->hasMinMaxValues() ) { volume->updateMinMaxValues(); }
    const kvs::Real32 min_value = static_cast<kvs::Real32>(volume->minValue());
    const kvs::Real32 max_value = static_cast<kvs::Real32>(volume->maxValue());
    const kvs::Real32 normalize = 1.0f / ( max_value - min_value );

    kvs::ValueArray<kvs::UInt8> colors( 3 * nnodes );
    kvs::UInt8* color = colors.data();

    switch( m_color_mode )
    {
    case PolygonGlyphObject::ColorByDefault:
        for ( size_t i = 0; i < nnodes; i++ )
        {
            *( color++ ) = DefaultColor().r();
            *( color++ ) = DefaultColor().r();
            *( color++ ) = DefaultColor().r();
        }
        break;
    case PolygonGlyphObject::ColorByMagnitude:
    {
        const kvs::ColorMap color_map( m_tfunc.colorMap() );
        if ( veclen == 1 )
        {
            for ( size_t i = 0; i < nnodes; i++ )
            {
                const float d = normalize * ( static_cast<float>(values[i]) - min_value );
                const size_t level = static_cast<size_t>( 255.0f * d );
                const kvs::RGBColor c = color_map[ level ];
                *( color++ ) = c.r();
                *( color++ ) = c.g();
                *( color++ ) = c.b();
            }
        }
        else if ( veclen == 3 )
        {
            for ( size_t i = 0, index = 0; i < nnodes; i++, index += 3 )
            {
                const kvs::Vec3 v(
                    static_cast<float>(values[index]),
                    static_cast<float>(values[index+1]),
                    static_cast<float>(values[index+2]) );
                const float d = normalize * ( static_cast<float>(v.length()) - min_value );
                const size_t level = static_cast<size_t>( 255.0f * d );
                const kvs::RGBColor c = color_map[ level ];
                *( color++ ) = c.r();
                *( color++ ) = c.g();
                *( color++ ) = c.b();
            }
        }
        break;
    }
    default:
        break;
    }

    this->setColors( colors );
}

template void PolygonGlyphObject::calculateColors<kvs::Int8>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateColors<kvs::Int16>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateColors<kvs::Int32>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateColors<kvs::Int64>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateColors<kvs::UInt8>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateColors<kvs::UInt16>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateColors<kvs::UInt32>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateColors<kvs::UInt64>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateColors<kvs::Real32>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateColors<kvs::Real64>( const kvs::VolumeObjectBase* volume );

/*===========================================================================*/
/**
 *  @brief  Calculates the opacity value array.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
template <typename T>
void PolygonGlyphObject::calculateOpacities( const kvs::VolumeObjectBase* volume )
{
    const auto values = volume->values().asValueArray<T>();
    const size_t veclen = volume->veclen();
    const size_t nnodes = volume->numberOfNodes();

    if ( !volume->hasMinMaxValues() ) { volume->updateMinMaxValues(); }
    const kvs::Real32 min_value = static_cast<kvs::Real32>(volume->minValue());
    const kvs::Real32 max_value = static_cast<kvs::Real32>(volume->maxValue());
    const kvs::Real32 normalize = 255.0f / ( max_value - min_value );

    kvs::ValueArray<kvs::UInt8> opacities( nnodes );
    kvs::UInt8* opacity = opacities.data();

    switch( m_opacity_mode )
    {
    case PolygonGlyphObject::OpacityByDefault:
        for( size_t i = 0; i < nnodes; i++ ) opacity[i] = DefaultOpacity();
        break;
    case PolygonGlyphObject::OpacityByMagnitude:
        if ( veclen == 1 )
        {
            for( size_t i = 0; i < nnodes; i++ )
            {
                opacity[i] = static_cast<kvs::UInt8>( normalize * ( static_cast<float>(values[i]) - min_value ) );
            }
        }
        else if ( veclen == 3 )
        {
            for( size_t i = 0, index = 0; i < nnodes; i++, index += 3 )
            {
                const kvs::Vector3f v(
                    static_cast<float>(values[index]),
                    static_cast<float>(values[index+1]),
                    static_cast<float>(values[index+2]));
                opacity[i] = static_cast<kvs::UInt8>( normalize * ( v.length() - min_value ) );
            }
        }
        break;
    default:
        break;
    }

    this->setOpacities( opacities );
}

template void PolygonGlyphObject::calculateOpacities<kvs::Int8>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateOpacities<kvs::Int16>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateOpacities<kvs::Int32>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateOpacities<kvs::Int64>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateOpacities<kvs::UInt8>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateOpacities<kvs::UInt16>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateOpacities<kvs::UInt32>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateOpacities<kvs::UInt64>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateOpacities<kvs::Real32>( const kvs::VolumeObjectBase* volume );
template void PolygonGlyphObject::calculateOpacities<kvs::Real64>( const kvs::VolumeObjectBase* volume );

} //namespace kvs
