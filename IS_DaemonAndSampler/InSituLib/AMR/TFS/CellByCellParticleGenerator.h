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
#ifndef PBVR__CELL_BY_CELL_PARTICLE_GENERATOR_H_INCLUDE
#define PBVR__CELL_BY_CELL_PARTICLE_GENERATOR_H_INCLUDE

#include <kvs/OpacityMap>
#include <kvs/Vector3>
#include <kvs/Math>
#include <kvs/Camera>
//#include <kvs/ObjectBase>
#include "ObjectBase.h"

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

void GetViewport( int ( *viewport )[4] );

void GetProjectionMatrix( double ( *projection )[16] );

void GetModelviewMatrix( const pbvr::ObjectBase* object, double ( *modelview )[16] );

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

} // end of namespace

namespace pbvr
{

namespace CellByCellParticleGenerator
{

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

inline void CalculateDensityParameters(
    const kvs::Camera*     camera,
    const kvs::ObjectBase* object,
    const float            subpixel_level,
    const float            sampling_step,
    const float            max_opacity,
    float* p_sampling_volume_inverse,
    float* p_max_density );

const float CalculateDensity(
    const float opacity,
    const float sampling_volume_inverse,
    const float max_opacity,
    const float max_density );

const kvs::ValueArray<float> CalculateDensityMap(
    const kvs::Camera*     camera,
    const kvs::ObjectBase* object,
    const float            subpixel_level,
    const float            sampling_step,
    const kvs::OpacityMap& opacity_map );

} // end of namespace CellByCellParticleGenerator

} // end of namespace pbvr


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
    KVS_ASSERT( !( kvs::Math::IsZero( back - front ) ) );

    const T cotA = cosA / sinA;
    const T elements[16] =
    {
        cotA / aspect,    0,                                    0,  0,
        0, cotA,                                    0,  0,
        0,    0,     -( back + front ) / ( back - front ), -1,
        0,    0, -( back* front * 2 ) / ( back - front ),  0
    };

    return ( kvs::Matrix44<T>( elements ) );
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

    return ( kvs::Matrix44<T>( elements ) );
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

    return ( kvs::Matrix44<T>( elements ) );
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

    return ( kvs::Matrix44<T>( elements ) );
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

    return ( kvs::Matrix44<T>( elements ) );
}

inline void GetViewport( const kvs::Camera* camera, int ( *viewport )[4] )
{
    ( *viewport )[0] = 0;
    ( *viewport )[1] = 0;
    ( *viewport )[2] = camera->windowWidth();
    ( *viewport )[3] = camera->windowHeight();
}

//inline void GetProjectionMatrix( const kvs::Camera* camera, double (*projection)[16] )
inline void GetProjectionMatrix( double ( *projection )[16] )
{
    const bool  perspective = true;//camera->isPerspective();
    const float fov = 45.0;//camera->fieldOfView();
    const float aspect = 1.0;//static_cast<float>( camera->windowWidth() ) / camera->windowHeight();
    const float front = 1.0;//camera->front();
    const float back = 2000.0;//camera->back();
    const float left = -5.0;//camera->left();
    const float right = 5.0;//camera->right();
    const float bottom = -5.0;//camera->bottom();
    const float top = 5.0;//camera->top();
    const kvs::Matrix44f P = perspective ?
                             ::PerspectiveMatrix<float>( fov, aspect, front, back ) :
                             ::OrthogonalMatrix<float>( left, right, bottom, top, front, back );

    ( *projection )[ 0] = P[0][0];
    ( *projection )[ 1] = P[0][1];
    ( *projection )[ 2] = P[0][2];
    ( *projection )[ 3] = P[0][3];
    ( *projection )[ 4] = P[1][0];
    ( *projection )[ 5] = P[1][1];
    ( *projection )[ 6] = P[1][2];
    ( *projection )[ 7] = P[1][3];
    ( *projection )[ 8] = P[2][0];
    ( *projection )[ 9] = P[2][1];
    ( *projection )[10] = P[2][2];
    ( *projection )[11] = P[2][3];
    ( *projection )[12] = P[3][0];
    ( *projection )[13] = P[3][1];
    ( *projection )[14] = P[3][2];
    ( *projection )[15] = P[3][3];
}

//inline void GetModelviewMatrix( const kvs::Camera* camera, const pbvr::ObjectBase* object, double (*modelview)[16] )
inline void GetModelviewMatrix( const kvs::ObjectBase* object, double ( *modelview )[16] )
{
    const kvs::Vector3f  min_external = object->minExternalCoord();
    const kvs::Vector3f  max_external = object->maxExternalCoord();
    const kvs::Vector3f  center       = ( max_external + min_external ) * 0.5f;
    const kvs::Vector3f  diff         = max_external - min_external;
    const float          normalize    = 6.0f / kvs::Math::Max( diff.x(), diff.y(), diff.z() );
    const kvs::Vector3f  eye( 0.0, 0.0, 12.0 );//= camera->position();
    const kvs::Vector3f  up( 0.0, 1.0, 0.0 );//= camera->upVector();
    const kvs::Vector3f  target( 0.0, 0.0, 0.0 );//= camera->lookAt();

    const kvs::Matrix44f T = ::TranslationMatrix<float>( -center.x(), -center.y(), -center.z() );
    const kvs::Matrix44f S = ::ScalingMatrix<float>( normalize, normalize, normalize );
    const kvs::Matrix44f L = ::LookAtMatrix<float>( eye, up, target );
    const kvs::Matrix44f M = L * S * T;

    ( *modelview )[ 0] = M[0][0];
    ( *modelview )[ 1] = M[1][0];
    ( *modelview )[ 2] = M[2][0];
    ( *modelview )[ 3] = M[3][0];
    ( *modelview )[ 4] = M[0][1];
    ( *modelview )[ 5] = M[1][1];
    ( *modelview )[ 6] = M[2][1];
    ( *modelview )[ 7] = M[3][1];
    ( *modelview )[ 8] = M[0][2];
    ( *modelview )[ 9] = M[1][2];
    ( *modelview )[10] = M[2][2];
    ( *modelview )[11] = M[3][2];
    ( *modelview )[12] = M[0][3];
    ( *modelview )[13] = M[1][3];
    ( *modelview )[14] = M[2][3];
    ( *modelview )[15] = M[3][3];
}

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

namespace pbvr
{

namespace CellByCellParticleGenerator
{

inline const float CalculateObjectDepth(
    const kvs::ObjectBase* object,
    const double         modelview[16],
    const double         projection[16],
    const int            viewport[4] )
{
    // calculate suitable depth.
    double x, y, z;

    ::Project(
        object->objectCenter().x(),
        object->objectCenter().y(),
        object->objectCenter().z(),
        modelview, projection, viewport,
        &x, &y, &z );

    const float object_depth = static_cast<float>( z );

    return ( object_depth );
}

inline const float CalculateSubpixelLength(
    const float    subpixel_level,
    const float    object_depth,
    const double modelview[16],
    const double projection[16],
    const int    viewport[4] )
{
    double wx_min, wy_min, wz_min;
    double wx_max, wy_max, wz_max;

    ::UnProject(
        0.0, 0.0, double( object_depth ),
        modelview, projection, viewport,
        &wx_min, &wy_min, &wz_min );

    ::UnProject(
        1.0, 1.0 , double( object_depth ),
        modelview, projection, viewport,
        &wx_max, &wy_max, &wz_max );

    const float subpixel_length = static_cast<float>( ( wx_max - wx_min ) / subpixel_level );

    return ( subpixel_length );
}

inline void CalculateDensityParameters(
    const kvs::Camera*     camera,
    const kvs::ObjectBase* object,
    const float            subpixel_level,
    const float            sampling_step,
    const float            max_opacity,
    float* p_sampling_volume_inverse,
    float* p_max_density )
{
    // Calculate a transform matrix.
    double modelview[16];
    ::GetModelviewMatrix( object, &modelview );
    double projection[16];
    ::GetProjectionMatrix( &projection );
    int    viewport[4];
    ::GetViewport( camera, &viewport );

    // Calculate a depth of the center of gravity of the object.
    const float object_depth = CalculateObjectDepth( object, modelview, projection, viewport );

    // Calculate suitable subpixel length.
    const float subpixel_length = CalculateSubpixelLength( subpixel_level, object_depth, modelview, projection, viewport );

    // Calculate density map from the subpixel length and the opacity map.
    //const float max_opacity = 1.0f - std::exp( -sampling_step / subpixel_length );

    const float sampling_volume_inverse = 1.0f / ( subpixel_length * subpixel_length * sampling_step );
    const float max_density = -std::log( 1.0f - max_opacity ) * sampling_volume_inverse;

    *p_sampling_volume_inverse = sampling_volume_inverse;
    *p_max_density = max_density;
}

inline const float CalculateDensity(
    const float opacity,
    const float sampling_volume_inverse,
    const float max_opacity,
    const float max_density )
{
    const float density = opacity < max_opacity ? -std::log( 1.0f - opacity ) * sampling_volume_inverse : max_density;
    return density;
}

inline const float CalculateGreatDensity(
    const kvs::Camera*           camera,
    const kvs::ObjectBase*       object,
    const float                  subpixel_level,
    const float                  sampling_step )
{
    // Calculate a transform matrix.
    double modelview[16];
    ::GetModelviewMatrix( object, &modelview );
    double projection[16];
    ::GetProjectionMatrix( &projection );
    int    viewport[4];
    ::GetViewport( camera, &viewport );

    // Calculate a depth of the center of gravity of the object.
    const float object_depth = CalculateObjectDepth( object, modelview, projection, viewport );

    // Calculate suitable subpixel length.
    const float subpixel_length = CalculateSubpixelLength( subpixel_level, object_depth, modelview, projection, viewport );

    const float sampling_volume_inverse = 1.0f / ( subpixel_length * subpixel_length * sampling_step );

    // Calculate default density factor.
    const float semi_max_density  = -std::log( 1.0f - 0.98f ) * sampling_volume_inverse;//kawamura2
    //const float semi_max_density  = -std::log( 1.0f - 0.5f ) * sampling_volume_inverse;

    return semi_max_density;
}

} // end of namespace CellByCellParticleGenerator

} // end of namespace pbvr

#endif // KVS__CELL_BY_CELL_PARTICLE_GENERATOR_H_INCLUDE
