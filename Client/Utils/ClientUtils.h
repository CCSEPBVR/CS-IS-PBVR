#ifndef CLIENT_UTILS_H
#define CLIENT_UTILS_H

#include <vector>

#include <QVector>
#include <QColor>

#include <kvs/RGBColor>
#include <kvs/PolygonObject>

// FIXME:toHogeHoge作るぐらいなら最初からColorMap,OpacityMapクラス側を変えた方がいい気がします
// FIXME:このインラインは本来不要なのでなくても動作するようにしてください
//============================================================
// Color conversion
//============================================================
inline QVector<QColor> toQVectorColors(
    const std::vector<kvs::RGBColor>& colors )
{
    QVector<QColor> qcolors;
    qcolors.reserve( static_cast<int>( colors.size() ) );

    for( const auto& c : colors )
    {
        qcolors.append( QColor( c.r(), c.g(), c.b() ) );
    }
    return qcolors;
}

inline std::vector<kvs::RGBColor> toStdVectorColors(
    const QVector<QColor>& colors )
{
    std::vector<kvs::RGBColor> vec;
    vec.reserve( static_cast<size_t>( colors.size() ) );

    for ( const auto& c : colors )
    {
        vec.emplace_back( c.red(), c.green(), c.blue() );
    }
    return vec;
}

//============================================================
// Color default
//============================================================

inline QVector<QColor> defaultColor()
{
    QVector<QColor> defaultColor { QColor(5, 48, 97), QColor(6, 50, 100), QColor(7, 52, 102), QColor(8, 54, 105), QColor(9, 56, 108), QColor(10, 58, 111), QColor(11, 60, 114), QColor(12, 62, 116), QColor(14, 64, 119), QColor(15, 66, 122), QColor(16, 68, 125), QColor(17, 70, 128), QColor(18, 72, 131), QColor(19, 74, 134), QColor(20, 76, 136), QColor(21, 78, 139), QColor(22, 80, 142), QColor(23, 83, 145), QColor(24, 85, 148), QColor(25, 87, 151), QColor(27, 89, 154), QColor(28, 91, 157), QColor(29, 93, 160), QColor(30, 95, 163), QColor(31, 98, 166), QColor(32, 100, 169), QColor(33, 102, 172), QColor(35, 104, 173), QColor(37, 105, 174), QColor(38, 107, 175), QColor(40, 109, 176), QColor(41, 111, 177), QColor(43, 113, 178), QColor(45, 114, 178), QColor(46, 116, 179), QColor(47, 118, 180), QColor(49, 120, 181), QColor(50, 121, 182), QColor(51, 123, 183), QColor(53, 125, 184), QColor(54, 127, 185), QColor(55, 129, 186), QColor(57, 130, 187), QColor(58, 132, 188), QColor(59, 134, 189), QColor(60, 136, 189), QColor(61, 138, 190), QColor(63, 140, 191), QColor(64, 141, 192), QColor(65, 143, 193), QColor(66, 145, 194), QColor(67, 147, 195), QColor(71, 149, 196), QColor(74, 151, 197), QColor(78, 153, 198), QColor(81, 154, 199), QColor(85, 156, 200), QColor(88, 158, 201), QColor(91, 160, 202), QColor(95, 162, 203), QColor(98, 164, 204), QColor(101, 166, 205), QColor(104, 168, 206), QColor(107, 170, 207), QColor(110, 172, 209), QColor(113, 174, 210), QColor(116, 175, 211), QColor(118, 177, 212), QColor(121, 179, 213), QColor(124, 181, 214), QColor(127, 183, 215), QColor(130, 185, 216), QColor(132, 187, 217), QColor(135, 189, 218), QColor(138, 191, 219), QColor(141, 193, 220), QColor(143, 195, 221), QColor(146, 197, 222), QColor(149, 198, 223), QColor(151, 200, 223), QColor(154, 201, 224), QColor(157, 202, 225), QColor(159, 203, 226), QColor(162, 205, 226), QColor(164, 206, 227), QColor(167, 207, 228), QColor(169, 208, 228), QColor(172, 210, 229), QColor(174, 211, 230), QColor(177, 212, 231), QColor(179, 214, 231), QColor(182, 215, 232), QColor(184, 216, 233), QColor(187, 217, 234), QColor(189, 219, 234), QColor(192, 220, 235), QColor(194, 221, 236), QColor(197, 223, 236), QColor(199, 224, 237), QColor(202, 225, 238), QColor(204, 226, 239), QColor(207, 228, 239), QColor(209, 229, 240), QColor(210, 230, 240), QColor(212, 230, 241), QColor(213, 231, 241), QColor(215, 232, 241), QColor(216, 232, 241), QColor(218, 233, 242), QColor(219, 234, 242), QColor(221, 235, 242), QColor(222, 235, 242), QColor(224, 236, 243), QColor(225, 237, 243), QColor(227, 237, 243), QColor(228, 238, 244), QColor(230, 239, 244), QColor(231, 239, 244), QColor(233, 240, 244), QColor(234, 241, 245), QColor(235, 241, 245), QColor(237, 242, 245), QColor(238, 243, 245), QColor(240, 244, 246), QColor(241, 244, 246), QColor(243, 245, 246), QColor(244, 246, 246), QColor(246, 246, 247), QColor(247, 247, 247), QColor(247, 246, 245), QColor(248, 245, 243), QColor(248, 244, 241), QColor(248, 243, 240), QColor(249, 242, 238), QColor(249, 241, 236), QColor(249, 239, 234), QColor(250, 238, 232), QColor(250, 237, 230), QColor(250, 236, 228), QColor(250, 235, 227), QColor(251, 234, 225), QColor(251, 233, 223), QColor(251, 232, 221), QColor(251, 231, 219), QColor(251, 230, 217), QColor(252, 229, 215), QColor(252, 228, 214), QColor(252, 227, 212), QColor(252, 225, 210), QColor(252, 224, 208), QColor(252, 223, 206), QColor(253, 222, 204), QColor(253, 221, 203), QColor(253, 220, 201), QColor(253, 219, 199), QColor(253, 217, 196), QColor(253, 215, 193), QColor(252, 212, 191), QColor(252, 210, 188), QColor(252, 208, 185), QColor(252, 206, 182), QColor(252, 204, 179), QColor(251, 202, 177), QColor(251, 200, 174), QColor(251, 197, 171), QColor(250, 195, 168), QColor(250, 193, 165), QColor(250, 191, 163), QColor(249, 189, 160), QColor(249, 187, 157), QColor(248, 184, 154), QColor(248, 182, 152), QColor(248, 180, 149), QColor(247, 178, 146), QColor(247, 176, 143), QColor(246, 174, 141), QColor(246, 171, 138), QColor(245, 169, 135), QColor(245, 167, 133), QColor(244, 165, 130), QColor(243, 162, 128), QColor(242, 160, 126), QColor(241, 157, 124), QColor(240, 155, 122), QColor(239, 152, 119), QColor(238, 149, 117), QColor(237, 147, 115), QColor(235, 144, 113), QColor(234, 142, 111), QColor(233, 139, 109), QColor(232, 136, 107), QColor(231, 134, 105), QColor(230, 131, 103), QColor(229, 128, 101), QColor(227, 126, 99), QColor(226, 123, 97), QColor(225, 120, 95), QColor(224, 118, 93), QColor(223, 115, 91), QColor(221, 112, 89), QColor(220, 110, 87), QColor(219, 107, 85), QColor(218, 104, 83), QColor(217, 102, 81), QColor(215, 99, 79), QColor(214, 96, 77), QColor(213, 94, 76), QColor(211, 91, 74), QColor(210, 89, 73), QColor(208, 86, 71), QColor(207, 84, 70), QColor(206, 82, 68), QColor(204, 79, 67), QColor(203, 77, 66), QColor(201, 74, 64), QColor(200, 72, 63), QColor(198, 69, 62), QColor(197, 66, 60), QColor(196, 64, 59), QColor(194, 61, 57), QColor(193, 58, 56), QColor(191, 55, 55), QColor(190, 53, 53), QColor(188, 50, 52), QColor(187, 46, 51), QColor(185, 43, 49), QColor(184, 40, 48), QColor(182, 36, 47), QColor(181, 33, 46), QColor(179, 29, 44), QColor(178, 24, 43), QColor(175, 23, 43), QColor(172, 22, 42), QColor(169, 21, 42), QColor(166, 20, 41), QColor(162, 19, 41), QColor(159, 18, 40), QColor(156, 17, 40), QColor(153, 15, 39), QColor(150, 14, 39), QColor(147, 13, 38), QColor(144, 12, 38), QColor(141, 11, 37), QColor(138, 10, 37), QColor(135, 9, 36), QColor(132, 8, 36), QColor(129, 7, 35), QColor(126, 6, 35), QColor(123, 5, 34), QColor(120, 4, 34), QColor(117, 3, 33), QColor(115, 2, 33), QColor(112, 2, 33), QColor(109, 1, 32), QColor(106, 1, 32), QColor(103, 0, 31) };
    return defaultColor;
}

//============================================================
// Opacity conversion
//============================================================

inline QVector<float> toQVectorOpacities( const std::vector<float>& values )
{
    QVector<float> qvalues;
    qvalues.reserve( static_cast<int>( values.size() ) );

    for( const auto& v : values )
    {
        qvalues.append( v );
    }
    return qvalues;
}

inline std::vector<float> toStdVectorOpacities( const QVector<float>& values )
{
    std::vector<float> vec;
    vec.reserve( static_cast<size_t>( values.size() ) );

    for( const auto& v : values )
    {
        vec.push_back( v );
    }
    return vec;
}

inline kvs::PolygonObject* createArrowGlyph(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::Real32>& directions,
        const kvs::ValueArray<kvs::Real32>& sizes,
        const kvs::ValueArray<kvs::UInt8>& colors )
{
    const size_t npoint = coords.size() / 3;
    const int slices = 20;

    std::vector<kvs::Vec3> all_vertices;
    std::vector<kvs::Vec3> all_normals;
    std::vector<kvs::UInt32> all_indices;
    std::vector<kvs::UInt8> all_colors;

    for( size_t i = 0, index = 0; i < npoint; i++, index += 3 )
    {
        kvs::Vec3 tip_position( coords.data() + index );   // 先端位置
        kvs::Vec3 direction( directions.data() + index );
        kvs::Real32 size = sizes[i];
        kvs::RGBColor color( colors.data() + index );

        if( direction.length() < 1e-6 )
        {
            std::cerr << "Error: Invalid direction vector." << std::endl;
            continue;
        }
        direction = direction.normalized();

        // --- 矢印寸法 ---
        const float cylinder_height = 0.7f * size;
        const float cylinder_radius = 0.07f * size;
        const float cone_height = 0.3f * size;
        const float cone_radius = 0.15f * size;
        const float arrow_height = cylinder_height + cone_height;

        std::vector<kvs::Vec3> vertices;
        std::vector<kvs::Vec3> normals;
        std::vector<kvs::UInt32> indices;

        // --- 円柱の構築 ---
        for( int s = 0; s < slices; ++s )
        {
            float angle = 2.0f * M_PI * s / slices;
            float x = cylinder_radius * std::cos( angle );
            float y = cylinder_radius * std::sin( angle );

            vertices.emplace_back( x, y, 0.0f );              // 底面
            normals.emplace_back( x, y, 0.0f );

            vertices.emplace_back( x, y, cylinder_height );   // 上面
            normals.emplace_back( x, y, 0.0f );
        }

        for( int s = 0; s < slices; ++s )
        {
            int next = ( s + 1 ) % slices;
            indices.push_back( s * 2 ); indices.push_back( next * 2 ); indices.push_back( s * 2 +1 );
            indices.push_back( s * 2 +1 ); indices.push_back( next * 2 ); indices.push_back( next * 2 +1 );
        }

        // --- 円錐の構築 ---
        std::vector<kvs::Vec3> base_vertices;
        for( int s = 0; s < slices; ++s )
        {
            float angle = 2.0f * M_PI * s / slices;
            base_vertices.emplace_back( cone_radius*std::cos( angle ), cone_radius*std::sin( angle ), cylinder_height );
        }

        for( int s = 0; s < slices; ++s )
        {
            int next = ( s + 1 ) % slices;
            kvs::Vec3 apex( 0, 0, cylinder_height + cone_height );
            kvs::Vec3 v1 = base_vertices[s];
            kvs::Vec3 v2 = base_vertices[next];

            vertices.push_back( v1 ); vertices.push_back( v2 ); vertices.push_back( apex );
            kvs::Vec3 normal = ( v2 - v1 ).cross( apex - v1 ).normalized();
            normals.push_back( normal ); normals.push_back( normal ); normals.push_back( normal );

            int base_index = vertices.size() - 3;
            indices.push_back( base_index ); indices.push_back( base_index + 1 ); indices.push_back( base_index + 2 );
        }

        // --- 先端が tip_position になるように Zを下方向にシフト ---
        for( auto& v : vertices ) v.z() -= arrow_height;

        // --- 回転・位置調整 ---
        kvs::Vec3 default_direction( 0, 0, 1 );
        kvs::Vec3 axis = default_direction.cross( direction );
        float angle = std::acos( default_direction.dot( direction ) );
        kvs::Mat3 rotation;
        if( axis.length() > 1e-6 )
        {

            rotation = kvs::Mat3::Rotation( axis.normalized(), angle*180.0/M_PI );
        }
        else
        {
            rotation = kvs::Mat3::Identity();
        }

        for( auto& v : vertices ) v = rotation * v + tip_position;
        for( auto& n : normals ) n = rotation * n;

        size_t offset = all_vertices.size();
        all_vertices.insert( all_vertices.end(), vertices.begin(), vertices.end() );
        all_normals.insert( all_normals.end(), normals.begin(), normals.end() );
        for( auto idx : indices ) all_indices.push_back( idx + offset );

        for( size_t c =0; c< vertices.size(); ++c )
        {
            all_colors.push_back( color.r() );
            all_colors.push_back( color.g() );
            all_colors.push_back( color.b() );
        }
    }

    kvs::PolygonObject* polygon = new kvs::PolygonObject();
    polygon->setCoords( kvs::ValueArray<kvs::Real32>( ( kvs::Real32* )all_vertices.data(), all_vertices.size()*3 ) );
    polygon->setConnections( kvs::ValueArray<kvs::UInt32>( all_indices.data(), all_indices.size() ) );
    polygon->setColors( kvs::ValueArray<kvs::UInt8>( all_colors.data(), all_colors.size() ) );
    polygon->setNormals( kvs::ValueArray<kvs::Real32>( ( kvs::Real32* )all_normals.data(), all_normals.size()*3 ) );
    polygon->setOpacity( 255 );
    polygon->setPolygonType(kvs::PolygonObject::Triangle);
    polygon->setColorType(kvs::PolygonObject::PolygonColor);
    polygon->setNormalType(kvs::PolygonObject::VertexNormal);

    return polygon;
}

#endif // CLIENT_UTILS_H
