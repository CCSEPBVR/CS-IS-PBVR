#include "GlyphPolygon.h"
#include <kvs/Quaternion>
GlyphPolygon::GlyphPolygon() {}

kvs::PolygonObject* GlyphPolygon::createArrowGlyphPolygon( const DataInitializer& data )
{
    auto npoints = data.coords.size() / 3;

    std::vector<kvs::Vec3> all_vertices;
    std::vector<kvs::Vec3> all_normals;
    std::vector<kvs::UInt32> all_indices;
    std::vector<kvs::UInt8> all_colors;

    for ( size_t i = 0, index = 0; i < npoints; i++, index += 3 )
    {
        kvs::Vec3 position( data.coords.data() + index );
        kvs::Vec3 direction( data.directions.data() + index );
        kvs::Real32 size = data.sizes[i];
        kvs::RGBColor color( data.colors.data() + index );

        if ( direction.length() < 1e-6 )
        {
            std::cerr << "Error: Invalid direction vector. Length is near zero." << std::endl;
            return nullptr;
        }

        direction = direction.normalized();
        const int slices = 20;

        for( int i = 0; i < slices * 3; i++ )
        {
            // if( c_change )
            // {
            //     all_colors.push_back( color.r() * -1 );
            //     all_colors.push_back( color.g() * -1 );
            //     all_colors.push_back( color.b() * -1 );
            // }
            // else
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

    auto* combined_arrow = new kvs::PolygonObject();
    combined_arrow->setCoords( kvs::ValueArray<kvs::Real32>( ( kvs::Real32* )all_vertices.data(), all_vertices.size() * 3 ) );
    combined_arrow->setNormals( kvs::ValueArray<kvs::Real32>( ( kvs::Real32* )all_normals.data(), all_normals.size() * 3 ) );
    combined_arrow->setConnections( kvs::ValueArray<kvs::UInt32>( all_indices.data(), all_indices.size() ) );
    combined_arrow->setColors(  kvs::ValueArray<kvs::UInt8>( all_colors.data(), all_colors.size() ) );
    combined_arrow->setColorType( kvs::PolygonObject::PolygonColor );
    combined_arrow->setPolygonType( kvs::PolygonObject::Triangle );
    combined_arrow->setNormalType( kvs::PolygonObject::VertexNormal );

    return combined_arrow;
}
kvs::PolygonObject* GlyphPolygon::createDiamondGlyphPolygon( const DataInitializer& data )
{
    auto npoints = data.coords.size() / 3;

    std::vector<kvs::Vec3> all_vertices;
    std::vector<kvs::Vec3> all_normals;
    std::vector<kvs::UInt32> all_indices;
    std::vector<kvs::UInt8> all_colors;

    for ( size_t i = 0, index = 0; i < npoints; i++, index += 3 )
    {
        kvs::Vec3 position( data.coords.data() + index );
        kvs::Vec3 direction( data.directions.data() + index );
        kvs::Real32 size = data.sizes[i];
        kvs::RGBColor color( data.colors.data() + index );

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
            // if( c_change )
            // {
            //     all_colors.push_back( color.r() * -1 );
            //     all_colors.push_back( color.g() * -1 );
            //     all_colors.push_back( color.b() * -1 );
            // }
            // else
            {
                all_colors.push_back( color.r() );
                all_colors.push_back( color.g() );
                all_colors.push_back( color.b() );
            }
        }
    }

    // ダイアモンドオブジェクトを作成して登録
    kvs::PolygonObject* diamond_object = new kvs::PolygonObject();
    diamond_object->setCoords(kvs::ValueArray<kvs::Real32>((kvs::Real32*)all_vertices.data(), all_vertices.size() * 3));
    diamond_object->setNormals(kvs::ValueArray<kvs::Real32>((kvs::Real32*)all_normals.data(), all_normals.size() * 3));
    diamond_object->setConnections(kvs::ValueArray<kvs::UInt32>(all_indices.data(), all_indices.size()));
    diamond_object->setColors(kvs::ValueArray<kvs::UInt8>(all_colors.data(), all_colors.size()));
    diamond_object->setPolygonType(kvs::PolygonObject::Triangle);
    diamond_object->setColorType(kvs::PolygonObject::VertexColor);
    diamond_object->setNormalType(kvs::PolygonObject::PolygonNormal);
    // diamond_object->setOpacity( 128 );

    return diamond_object;
}
kvs::PolygonObject* GlyphPolygon::createSphereGlyphPolygon( const DataInitializer& data )
{
    auto npoints = data.coords.size() / 3;

    const float radius = 0.5f; // スフィアの半径
    const int slices = 10;    // スフィアの横分割数
    const int stacks = 10;    // スフィアの縦分割数

    std::vector<kvs::Vec3> all_vertices;
    std::vector<kvs::Vec3> all_normals;
    std::vector<kvs::UInt8> all_colors;
    std::vector<kvs::UInt32> all_indices;

    for (size_t i = 0, index = 0; i < npoints; i++, index += 3)
    {
        kvs::Vec3 position(data.coords.data() + index);
        kvs::Real32 size = data.sizes[i];
        kvs::RGBColor color(data.colors.data() + index);

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
                // if( c_change )
                // {
                //     all_colors.push_back( color.r() * -1 );
                //     all_colors.push_back( color.g() * -1 );
                //     all_colors.push_back( color.b() * -1 );
                // }
                // else
                {
                    all_colors.push_back( color.r() );
                    all_colors.push_back( color.g() );
                    all_colors.push_back( color.b() );
                }
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

    // スフィアオブジェクトを作成して登録
    kvs::PolygonObject* sphere_object = new kvs::PolygonObject();
    sphere_object->setCoords(kvs::ValueArray<kvs::Real32>((kvs::Real32*)all_vertices.data(), all_vertices.size() * 3));
    sphere_object->setNormals(kvs::ValueArray<kvs::Real32>((kvs::Real32*)all_normals.data(), all_normals.size() * 3));
    sphere_object->setColors(kvs::ValueArray<kvs::UInt8>(all_colors.data(), all_colors.size()));
    sphere_object->setConnections(kvs::ValueArray<kvs::UInt32>(all_indices.data(), all_indices.size()));
    sphere_object->setPolygonType(kvs::PolygonObject::Triangle);
    sphere_object->setColorType(kvs::PolygonObject::VertexColor);
    sphere_object->setNormalType(kvs::PolygonObject::VertexNormal);

    return sphere_object;
}
