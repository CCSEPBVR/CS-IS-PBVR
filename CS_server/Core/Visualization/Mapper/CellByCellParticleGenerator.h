/****************************************************************************/
/**
 *  @file CellByCellParticleGenerator.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: CellByCellParticleGenerator.h 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__CELL_BY_CELL_PARTICLE_GENERATOR_H_INCLUDE
#define KVS__CELL_BY_CELL_PARTICLE_GENERATOR_H_INCLUDE

#include <kvs/VolumeObjectBase>
#include <kvs/OpacityMap>
#include <kvs/Vector3>
#include <kvs/Math>
#include <kvs/Camera>


namespace
{

template <typename T>
kvs::Matrix44<T> PerspectiveMatrix( T fov_y, T aspect, T front, T back );

template <typename T>
kvs::Matrix44<T> OrthogonalMatrix( T left, T right, T bottom, T top, T front, T back );

template <typename T>
kvs::Matrix44<T> LookAtMatrix(
    const kvs::Vector3<T>& eye,
    const kvs::Vector3<T>& up,
    const kvs::Vector3<T>& target );

template <typename T>
kvs::Matrix44<T> ScalingMatrix( T x, T y, T z, T w = T( 1 ) );

template <typename T>
kvs::Matrix44<T> TranslationMatrix( T x, T y, T z, T w = T( 1 ) );

#ifndef NO_CLIENT
void GetViewport( const kvs::Camera* camera, GLint (*viewport)[4] );

void GetProjectionMatrix( const kvs::Camera* camera, GLdouble (*projection)[16] );

void GetModelviewMatrix( const kvs::Camera* camera, const kvs::ObjectBase* object, GLdouble (*modelview)[16] );

void Project(
    const GLdouble obj_x,
    const GLdouble obj_y,
    const GLdouble obj_z,
    const GLdouble modelview[16],
    const GLdouble projection[16],
    const GLint    viewport[4],
    GLdouble*      win_x,
    GLdouble*      win_y,
    GLdouble*      win_z );

void UnProject(
    const GLdouble win_x,
    const GLdouble win_y,
    const GLdouble win_z,
    const GLdouble modelview[16],
    const GLdouble projection[16],
    const GLint    viewport[4],
    GLdouble*      obj_x,
    GLdouble*      obj_y,
    GLdouble*      obj_z );
#else
void GetViewport( const kvs::Camera* camera, int (*viewport)[4] );

void GetProjectionMatrix( const kvs::Camera* camera, double (*projection)[16] );

void GetModelviewMatrix( const kvs::Camera* camera, const kvs::ObjectBase* object, double (*modelview)[16] );

void Project(
    const double obj_x,
    const double obj_y,
    const double obj_z,
    const double modelview[16],
    const double projection[16],
    const int    viewport[4],
    double*      win_x,
    double*      win_y,
    double*      win_z );

void UnProject(
    const double win_x,
    const double win_y,
    const double win_z,
    const double modelview[16],
    const double projection[16],
    const int    viewport[4],
    double*      obj_x,
    double*      obj_y,
    double*      obj_z );
#endif
} // end of namespace

namespace kvs
{

namespace CellByCellParticleGenerator
{

const float GetRandomNumber( void );

const kvs::Vector3f RandomSamplingInCube( const kvs::Vector3f& v );

#ifndef NO_CLIENT
const float CalculateObjectDepth(
    const kvs::ObjectBase* object,
    const GLdouble         modelview[16],
    const GLdouble         projection[16],
    const GLint            viewport[4] );

const float CalculateSubpixelLength(
    const float    subpixel_level,
    const float    object_depth,
    const GLdouble modelview[16],
    const GLdouble projection[16],
    const GLint    viewport[4] );
#else
const float CalculateObjectDepth(
    const kvs::ObjectBase* object,
    const double         modelview[16],
    const double         projection[16],
    const int            viewport[4] );

const float CalculateSubpixelLength(
    const float    subpixel_level,
    const float    object_depth,
    const double modelview[16],
    const double projection[16],
    const int    viewport[4] );
#endif
const kvs::ValueArray<float> CalculateDensityMap(
    const kvs::Camera*     camera,
    const kvs::ObjectBase* object,
    const float            subpixel_level,
    const float            sampling_step,
    const kvs::OpacityMap& opacity_map );

} // end of namespace CellByCellParticleGenerator

} // end of namespace kvs


namespace
{

template <typename T>
inline kvs::Matrix44<T> PerspectiveMatrix( T fov_y, T aspect, T front, T back )
{
    const T rad  = kvs::Math::Deg2Rad( fov_y / 2 );
    const T sinA = static_cast<T>( std::sin( rad ) );
    const T cosA = static_cast<T>( std::cos( rad ) );

    KVS_ASSERT( !( kvs::Math::IsZero( sinA ) ) );
    KVS_ASSERT( !( kvs::Math::IsZero( aspect ) ) );
    KVS_ASSERT( !( kvs::Math::IsZero( back -front ) ) );

    const T cotA = cosA / sinA;
    const T elements[16] =
    {
        cotA / aspect,    0,                                    0,  0,
                    0, cotA,                                    0,  0,
                    0,    0,     -( back + front ) / ( back - front ), -1,
                    0,    0, -( back * front * 2 ) / ( back - front ),  0
    };

    return( kvs::Matrix44<T>( elements ) );
}

template <typename T>
inline kvs::Matrix44<T> OrthogonalMatrix( T left, T right, T bottom, T top, T front, T back )
{
    const T width  = right - left;
    const T height = top - bottom;
    const T depth  = back - front;

    KVS_ASSERT( !( kvs::Math::IsZero( width  ) ) );
    KVS_ASSERT( !( kvs::Math::IsZero( height ) ) );
    KVS_ASSERT( !( kvs::Math::IsZero( depth  ) ) );

    const T elements[ 16 ] =
    {
                        2 / width,                          0,                       0, 0,
                                0,                 2 / height,                       0, 0,
                                0,                          0,              -2 / depth, 0,
        -( right + left ) / width, -( top + bottom ) / height, -( back + front ) / depth, 0
    };

    return( kvs::Matrix44<T>( elements ) );
}

template <typename T>
inline kvs::Matrix44<T> LookAtMatrix(
    const kvs::Vector3<T>& eye,
    const kvs::Vector3<T>& up,
    const kvs::Vector3<T>& target )
{
    kvs::Vector3<T> f( target - eye );
    kvs::Vector3<T> s( f.cross( up.normalize() ) );
    kvs::Vector3<T> u( s.cross( f ) );

    f.normalize();
    s.normalize();
    u.normalize();

    const T elements[ 16 ] =
    {
         s.x(),  s.y(),  s.z(), -eye.x(),
         u.x(),  u.y(),  u.z(), -eye.y(),
        -f.x(), -f.y(), -f.z(), -eye.z(),
             0,      0,      0,        1
    };

    return( kvs::Matrix44<T>( elements ) );
}

template <typename T>
inline kvs::Matrix44<T> ScalingMatrix( T x, T y, T z, T w )
{
    const T elements[ 16 ] =
    {
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, w
    };

    return( kvs::Matrix44<T>( elements ) );
}

template <typename T>
inline kvs::Matrix44<T> TranslationMatrix( T x, T y, T z, T w )
{
    const T elements[ 16 ] =
    {
        1, 0, 0, x,
        0, 1, 0, y,
        0, 0, 1, z,
        0, 0, 0, w
    };

    return( kvs::Matrix44<T>( elements ) );
}

#ifndef NO_CLIENT
inline void GetViewport( const kvs::Camera* camera, GLint (*viewport)[4] )
#else
inline void GetViewport( const kvs::Camera* camera, int (*viewport)[4] )
#endif
{
    (*viewport)[0] = 0;
    (*viewport)[1] = 0;
    (*viewport)[2] = camera->windowWidth();
    (*viewport)[3] = camera->windowHeight();
}

#ifndef NO_CLIENT
inline void GetProjectionMatrix( const kvs::Camera* camera, GLdouble (*projection)[16] )
#else
inline void GetProjectionMatrix( const kvs::Camera* camera, double (*projection)[16] )
#endif
{
    const bool  perspective = camera->isPerspective();
    const float fov = camera->fieldOfView();
    const float aspect = static_cast<float>( camera->windowWidth() ) / camera->windowHeight();
    const float front = camera->front();
    const float back = camera->back();
    const float left = camera->left();
    const float right = camera->right();
    const float bottom = camera->bottom();
    const float top = camera->top();
    const kvs::Matrix44f P = perspective ?
        ::PerspectiveMatrix<float>( fov, aspect, front, back ) :
        ::OrthogonalMatrix<float>( left, right, bottom, top, front, back );

    (*projection)[ 0] = P[0][0];
    (*projection)[ 1] = P[0][1];
    (*projection)[ 2] = P[0][2];
    (*projection)[ 3] = P[0][3];
    (*projection)[ 4] = P[1][0];
    (*projection)[ 5] = P[1][1];
    (*projection)[ 6] = P[1][2];
    (*projection)[ 7] = P[1][3];
    (*projection)[ 8] = P[2][0];
    (*projection)[ 9] = P[2][1];
    (*projection)[10] = P[2][2];
    (*projection)[11] = P[2][3];
    (*projection)[12] = P[3][0];
    (*projection)[13] = P[3][1];
    (*projection)[14] = P[3][2];
    (*projection)[15] = P[3][3];
}

#ifndef NO_CLIENT
inline void GetModelviewMatrix( const kvs::Camera* camera, const kvs::ObjectBase* object, GLdouble (*modelview)[16] )
#else
inline void GetModelviewMatrix( const kvs::Camera* camera, const kvs::ObjectBase* object, double (*modelview)[16] )
#endif
{
    const kvs::Vector3f  min_external = object->minExternalCoord();
    const kvs::Vector3f  max_external = object->maxExternalCoord();
    const kvs::Vector3f  center       = ( max_external + min_external ) * 0.5f;
    const kvs::Vector3f  diff         = max_external - min_external;
    const float          normalize    = 6.0f / kvs::Math::Max( diff.x(), diff.y(), diff.z() );
    const kvs::Vector3f  eye          = camera->position();
    const kvs::Vector3f  up           = camera->upVector();
    const kvs::Vector3f  target       = camera->lookAt();

    const kvs::Matrix44f T = ::TranslationMatrix<float>( -center.x(), -center.y(), -center.z() );
    const kvs::Matrix44f S = ::ScalingMatrix<float>( normalize, normalize, normalize );
    const kvs::Matrix44f L = ::LookAtMatrix<float>( eye, up, target );
    const kvs::Matrix44f M = L * S * T;

    (*modelview)[ 0] = M[0][0];
    (*modelview)[ 1] = M[1][0];
    (*modelview)[ 2] = M[2][0];
    (*modelview)[ 3] = M[3][0];
    (*modelview)[ 4] = M[0][1];
    (*modelview)[ 5] = M[1][1];
    (*modelview)[ 6] = M[2][1];
    (*modelview)[ 7] = M[3][1];
    (*modelview)[ 8] = M[0][2];
    (*modelview)[ 9] = M[1][2];
    (*modelview)[10] = M[2][2];
    (*modelview)[11] = M[3][2];
    (*modelview)[12] = M[0][3];
    (*modelview)[13] = M[1][3];
    (*modelview)[14] = M[2][3];
    (*modelview)[15] = M[3][3];
}

#ifndef NO_CLIENT
inline void Project(
    const GLdouble obj_x,
    const GLdouble obj_y,
    const GLdouble obj_z,
    const GLdouble modelview[16],
    const GLdouble projection[16],
    const GLint    viewport[4],
    GLdouble*      win_x,
    GLdouble*      win_y,
    GLdouble*      win_z )
#else
inline void Project(
    const double obj_x,
    const double obj_y,
    const double obj_z,
    const double modelview[16],
    const double projection[16],
    const int    viewport[4],
    double*      win_x,
    double*      win_y,
    double*      win_z )
#endif
{
    const kvs::Vector4d I( obj_x, obj_y, obj_z, 1.0 );

    const kvs::Matrix44d P(
        projection[0], projection[4], projection[8],  projection[12],
        projection[1], projection[5], projection[9],  projection[13],
        projection[2], projection[6], projection[10], projection[14],
        projection[3], projection[7], projection[11], projection[15] );

    const kvs::Matrix44d M(
        modelview[0], modelview[4], modelview[8],  modelview[12],
        modelview[1], modelview[5], modelview[9],  modelview[13],
        modelview[2], modelview[6], modelview[10], modelview[14],
        modelview[3], modelview[7], modelview[11], modelview[15] );

    const kvs::Vector4d O = P * M * I;

    const double w =  ( O.w() == 0.0f ) ? 1.0f : O.w();

    *win_x = viewport[0] + ( 1.0 + O.x() / w ) * viewport[2] / 2.0;
    *win_y = viewport[1] + ( 1.0 + O.y() / w ) * viewport[3] / 2.0;
    *win_z = ( 1.0 + O.z() / w ) / 2.0;
}

#ifndef NO_CLIENT
inline void UnProject(
    const GLdouble win_x,
    const GLdouble win_y,
    const GLdouble win_z,
    const GLdouble modelview[16],
    const GLdouble projection[16],
    const GLint    viewport[4],
    GLdouble*      obj_x,
    GLdouble*      obj_y,
    GLdouble*      obj_z )
#else
inline void UnProject(
    const double win_x,
    const double win_y,
    const double win_z,
    const double modelview[16],
    const double projection[16],
    const int    viewport[4],
    double*      obj_x,
    double*      obj_y,
    double*      obj_z )
#endif
{
    const kvs::Vector4d I(
        ( win_x - viewport[0] ) * 2.0 / viewport[2] - 1.0,
        ( win_y - viewport[1] ) * 2.0 / viewport[3] - 1.0,
        2.0 * win_z - 1.0,
        1.0 );

    const kvs::Matrix44d P(
        projection[0], projection[4], projection[8],  projection[12],
        projection[1], projection[5], projection[9],  projection[13],
        projection[2], projection[6], projection[10], projection[14],
        projection[3], projection[7], projection[11], projection[15] );

    const kvs::Matrix44d M(
        modelview[0], modelview[4], modelview[8],  modelview[12],
        modelview[1], modelview[5], modelview[9],  modelview[13],
        modelview[2], modelview[6], modelview[10], modelview[14],
        modelview[3], modelview[7], modelview[11], modelview[15] );

    const kvs::Vector4d O = ( P * M ).inverse() * I;

    const double w =  ( O.w() == 0.0 ) ? 1.0 : O.w();

    *obj_x = O.x() / w;
    *obj_y = O.y() / w;
    *obj_z = O.z() / w;
}

} // end of namespace

namespace kvs
{

namespace CellByCellParticleGenerator
{

inline const float GetRandomNumber( void )
{
    // xorshift RGNs with period at least 2^128 - 1.
//    static float t24 = 1.0/16777216.0; /* 0.5**24 */
    static kvs::UInt32 x=123456789,y=362436069,z=521288629,w=88675123;
    kvs::UInt32 t;
    t=(x^(x<<11));
    x=y;y=z;z=w;
    w=(w^(w>>19))^(t^(t>>8));

    return( w * ( 1.0f / 4294967296.0f ) ); // = w * ( 1.0f / kvs::Value<kvs::UInt32>::Max() + 1 )
//    return( t24 * static_cast<float>( w >> 8 ) );
}


inline const kvs::Vector3f RandomSamplingInCube( const kvs::Vector3f& v )
{
//    float x = GetRandomNumber(); while ( kvs::Math::Equal( x, 1.0f ) ) x = GetRandomNumber();
//    float y = GetRandomNumber(); while ( kvs::Math::Equal( y, 1.0f ) ) y = GetRandomNumber();
//    float z = GetRandomNumber(); while ( kvs::Math::Equal( z, 1.0f ) ) z = GetRandomNumber();
    const float x = GetRandomNumber();
    const float y = GetRandomNumber();
    const float z = GetRandomNumber();
    const kvs::Vector3f d( x, y, z );
    return( v + d );
}

#ifndef NO_CLIENT
inline const float CalculateObjectDepth(
    const kvs::ObjectBase* object,
    const GLdouble         modelview[16],
    const GLdouble         projection[16],
    const GLint            viewport[4] )
#else
inline const float CalculateObjectDepth(
    const kvs::ObjectBase* object,
    const double         modelview[16],
    const double         projection[16],
    const int            viewport[4] )
#endif
{
    // calculate suitable depth.
#ifndef NO_CLIENT
    GLdouble x, y, z;
#else
    double x, y, z;
#endif

    ::Project(
        object->objectCenter().x(),
        object->objectCenter().y(),
        object->objectCenter().z(),
        modelview, projection, viewport,
        &x, &y, &z );

    const float object_depth = static_cast<float>( z );

    return( object_depth );
}

#ifndef NO_CLIENT
inline const float CalculateSubpixelLength(
    const float    subpixel_level,
    const float    object_depth,
    const GLdouble modelview[16],
    const GLdouble projection[16],
    const GLint    viewport[4] )
#else
inline const float CalculateSubpixelLength(
    const float    subpixel_level,
    const float    object_depth,
    const double modelview[16],
    const double projection[16],
    const int    viewport[4] )
#endif
{
#ifndef NO_CLIENT
    GLdouble wx_min, wy_min, wz_min;
    GLdouble wx_max, wy_max, wz_max;

    ::UnProject(
        0.0, 0.0, GLdouble( object_depth ),
        modelview, projection, viewport,
        &wx_min, &wy_min, &wz_min);

    ::UnProject(
        1.0, 1.0 , GLdouble( object_depth ),
        modelview, projection, viewport,
        &wx_max, &wy_max, &wz_max);
#else
    double wx_min, wy_min, wz_min;
    double wx_max, wy_max, wz_max;

    ::UnProject(
        0.0, 0.0, double( object_depth ),
        modelview, projection, viewport,
        &wx_min, &wy_min, &wz_min);

    ::UnProject(
        1.0, 1.0 , double( object_depth ),
        modelview, projection, viewport,
        &wx_max, &wy_max, &wz_max);
#endif
    const float subpixel_length = static_cast<float>( ( wx_max - wx_min ) / subpixel_level );

    return( subpixel_length );
}

inline const kvs::ValueArray<float> CalculateDensityMap(
    const kvs::Camera*     camera,
    const kvs::ObjectBase* object,
    const float            subpixel_level,
    const float            sampling_step,
    const kvs::OpacityMap& opacity_map )
{
    // Calculate a transform matrix.
#ifndef NO_CLIENT
    GLdouble modelview[16];  ::GetModelviewMatrix( camera, object, &modelview );
    GLdouble projection[16]; ::GetProjectionMatrix( camera, &projection );
    GLint    viewport[4];    ::GetViewport( camera, &viewport );
#else
    double modelview[16];  ::GetModelviewMatrix( camera, object, &modelview );
    double projection[16]; ::GetProjectionMatrix( camera, &projection );
    int    viewport[4];    ::GetViewport( camera, &viewport );
#endif

    // Calculate a depth of the center of gravity of the object.
    const float object_depth = CalculateObjectDepth( object, modelview, projection, viewport );

    // Calculate suitable subpixel length.
    const float subpixel_length = CalculateSubpixelLength( subpixel_level, object_depth, modelview, projection, viewport );

    // Calculate density map from the subpixel length and the opacity map.
    const float max_opacity = 1.0f - std::exp( -sampling_step / subpixel_length );
    const float max_density = 1.0f / ( subpixel_length * subpixel_length * subpixel_length );

    const float sampling_volume_inverse = 1.0f / ( subpixel_length * subpixel_length * sampling_step );

    const size_t resolution = opacity_map.resolution();

    // Create the density map.
    kvs::ValueArray<float> density_map;
    if ( !density_map.allocate( resolution ) )
    {
        kvsMessageError("Cannot allocate memory for a density map.");
        return( density_map );
    }

    for ( size_t i = 0; i < resolution; ++i )
    {
        const float opacity = opacity_map[i];

        if ( opacity < max_opacity )
        {
            density_map[i] = -std::log( 1.0f - opacity ) * sampling_volume_inverse;
        }
        else
        {
            density_map[i] = max_density;
        }
    }

    return( density_map );
}

} // end of namespace CellByCellParticleGenerator

} // end of namespace kvs

#endif // KVS__CELL_BY_CELL_PARTICLE_GENERATOR_H_INCLUDE
