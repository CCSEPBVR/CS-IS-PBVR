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

#include "VolumeObjectBase.h"
#include <kvs/OpacityMap>
#include <kvs/Vector3>
#include <kvs/Math>
#include <kvs/Camera>
#include "CellBase.h"
#include "TetrahedralCell.h"
#include "QuadraticTetrahedralCell.h"
#include "HexahedralCell.h"
#include "QuadraticHexahedralCell.h"
#include "PrismaticCell.h"
#include "PyramidalCell.h"
#include "TriangleCell.h"
#include "SquareCell.h"
#include "QuadraticTriangleCell.h"
#include "QuadraticSquareCell.h"
#include "StructuredVolumeObject.h"
#include "UnstructuredVolumeObject.h"

#ifdef KMATH
#include <kmath_random.h>
extern KMATH_Random km_random;
#endif

#ifdef _OPENMP
#  include <omp.h>
#endif // _OPENMP

#ifndef CPU_VER
#include "mpi.h"
#endif

namespace
{

template <typename T>
kvs::Matrix44<T> PerspectiveMatrix( const T& fov_y, const T& aspect, const T& front, const T& back );

template <typename T>
kvs::Matrix44<T> OrthogonalMatrix( const T& left, const T& right, const T& bottom, const T& top, const T& front, const T& back );

template <typename T>
kvs::Matrix44<T> LookAtMatrix(
    const kvs::Vector3<T>& eye,
    const kvs::Vector3<T>& up,
    const kvs::Vector3<T>& target );

template <typename T>
kvs::Matrix44<T> ScalingMatrix( const T& x, const T& y, const T& z, const T& w = T( 1 ) );

template <typename T>
kvs::Matrix44<T> TranslationMatrix( const T& x, const T& y, const T& z, const T& w = T( 1 ) );

void GetViewport( int ( *viewport )[4] );

void GetProjectionMatrix( double ( *projection )[16] );

void GetModelviewMatrix( const pbvr::ObjectBase& object, double ( *modelview )[16] );

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

const float GetRandomNumber();

const kvs::Vector3f RandomSamplingInCube( const kvs::Vector3f& v );

const float CalculateObjectDepth(
    const pbvr::ObjectBase& object,
    const double         modelview[16],
    const double         projection[16],
    const int            viewport[4] );

const float CalculateSubpixelLength(
    const float    subpixel_level,
    const float    object_depth,
    const double modelview[16],
    const double projection[16],
    const int    viewport[4] );

inline void CalculateDensityConstaint(
    const kvs::Camera&     camera,
    const pbvr::ObjectBase& object,
    const float            subpixel_level,
    const float            sampling_step,
    float* p_sampling_volume_inverse,
    float* p_max_opacity,
    float* p_max_density );

const float CalculateDensity(
    const float opacity,
    const float sampling_volume_inverse,
    const float max_opacity,
    const float max_density );

const kvs::ValueArray<float> CalculateDensityMap(
    const kvs::Camera&     camera,
    const pbvr::ObjectBase& object,
    const float            subpixel_level,
    const float            sampling_step,
    const kvs::OpacityMap& opacity_map );

} // end of namespace CellByCellParticleGenerator

} // end of namespace pbvr


namespace
{

template <typename T>
inline kvs::Matrix44<T> PerspectiveMatrix( const T& fov_y, const T& aspect, const T& front, const T& back )
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

    return kvs::Matrix44<T>( elements );
}

template <typename T>
inline kvs::Matrix44<T> OrthogonalMatrix( const T& left, const T& right, const T& bottom, const T& top, const T& front, const T& back )
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

    return kvs::Matrix44<T>( elements );
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

    return kvs::Matrix44<T>( elements );
}

template <typename T>
inline kvs::Matrix44<T> ScalingMatrix( const T& x, const T& y, const T& z, const T& w )
{
    const T elements[ 16 ] =
    {
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, w
    };

    return kvs::Matrix44<T>( elements );
}

template <typename T>
inline kvs::Matrix44<T> TranslationMatrix( const T& x, const T& y, const T& z, const T& w )
{
    const T elements[ 16 ] =
    {
        1, 0, 0, x,
        0, 1, 0, y,
        0, 0, 1, z,
        0, 0, 0, w
    };

    return kvs::Matrix44<T>( elements );
}

inline void GetViewport( const kvs::Camera& camera, int ( *viewport )[4] )
{
    ( *viewport )[0] = 0;
    ( *viewport )[1] = 0;
    ( *viewport )[2] = camera.windowWidth();
    ( *viewport )[3] = camera.windowHeight();
}

//inline void GetProjectionMatrix( const kvs::Camera* camera, double (*projection)[16] )
inline void GetProjectionMatrix( double ( *projection )[16] )
{
    const bool  perspective = true;//camera->isPerspective();
    const float fov = 45.0;//camera->fieldOfView();
    const float aspect = 1.0;//static_cast<float>( camera->m_window_width() ) / camera->m_window_height();
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
inline void GetModelviewMatrix( const pbvr::ObjectBase& object, double ( *modelview )[16] )
{
    const kvs::Vector3f  min_external = object.minExternalCoord();
    const kvs::Vector3f  max_external = object.maxExternalCoord();
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

inline const float GetRandomNumber()
{
    double rv;
#ifdef KMATH
    #pragma omp critical(random)
    {
        km_random.get( rv );
    }
    return static_cast<kvs::Real32>( rv - 1.0 );
#else
    // xorshift RGNs with period at least 2^128 - 1.
//    static float t24 = 1.0/16777216.0; /* 0.5**24 */
    static kvs::UInt32 x = 123456789, y = 362436069, z = 521288629, w = 88675123;
    kvs::UInt32 t;
    t = ( x ^ ( x << 11 ) );
    x = y;
    y = z;
    z = w;
    w = ( w ^ ( w >> 19 ) ) ^ ( t ^ ( t >> 8 ) );

    return w * ( 1.0f / 4294967296.0f ); // = w * ( 1.0f / kvs::Value<kvs::UInt32>::Max() + 1 )
//    return t24 * static_cast<float>( w >> 8 );
#endif
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
    return v + d;
}

inline const float CalculateObjectDepth(
    const pbvr::ObjectBase& object,
    const double         modelview[16],
    const double         projection[16],
    const int            viewport[4] )
{
    // calculate suitable depth.
    double x, y, z;

    ::Project(
        object.objectCenter().x(),
        object.objectCenter().y(),
        object.objectCenter().z(),
        modelview, projection, viewport,
        &x, &y, &z );

    const float object_depth = static_cast<float>( z );

    return object_depth;
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

    return subpixel_length;
}

inline void CalculateDensityConstaint(
    const kvs::Camera&     camera,
    const pbvr::ObjectBase& object,
    const float            subpixel_level,
    const float            sampling_step,
    float* p_sampling_volume_inverse,
    float* p_max_opacity,
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
    const float max_opacity = 1.0f - std::exp( -sampling_step / subpixel_length );

    const float sampling_volume_inverse = 1.0f / ( subpixel_length * subpixel_length * sampling_step );
    const float max_density = -std::log( 1.0f - 0.98f ) * sampling_volume_inverse;

    *p_sampling_volume_inverse = sampling_volume_inverse;
    *p_max_opacity = max_opacity;
    *p_max_density = max_density;
}

inline const float CalculateDensity(
    const float opacity,
    const float sampling_volume_inverse,
    const float max_opacity,
    const float max_density )
{
    float density;
    if ( opacity < max_opacity )
    {
        density = -std::log( 1.0f - opacity ) * sampling_volume_inverse;
    }
    else
    {
        density = max_density;
    }
    return density;
}

inline const float CalculateDensity2d(
    const float opacity,
    const float sampling_volume_inverse,
    const float max_opacity,
    const float max_density )
{
    float density;

    density = 100000;

    return density;
}

inline const kvs::ValueArray<float> CalculateDensityMap(
    const kvs::Camera&     camera,
    const pbvr::ObjectBase& object,
    const float            subpixel_level,
    const float            sampling_step,
    const kvs::OpacityMap& opacity_map )
{
    float sampling_volume_inverse;
    float max_opacity;
    float max_density;

    CalculateDensityConstaint( camera,
                               object,
                               subpixel_level,
                               sampling_step,
                               &sampling_volume_inverse,
                               &max_opacity,
                               &max_density );

    const size_t resolution = opacity_map.resolution();

    // Create the density map.
    kvs::ValueArray<float> density_map;
    if ( !density_map.allocate( resolution ) )
    {
        kvsMessageError( "Cannot allocate memory for a density map." );
        return density_map;
    }

    for ( size_t i = 0; i < resolution; ++i )
    {
        const float opacity = opacity_map[i];

        density_map[i] = CalculateDensity( opacity,
                                           sampling_volume_inverse,
                                           max_opacity,
                                           max_density );
    }

    return density_map;
}

inline const double CalculateTotalVolume( const pbvr::VolumeObjectBase* object )
{
    const pbvr::VolumeObjectBase::VolumeType volume_type = object->volumeType();
    if ( volume_type == pbvr::VolumeObjectBase::Structured )
    {
        const pbvr::StructuredVolumeObject* volume =
            reinterpret_cast<const pbvr::StructuredVolumeObject*>( object );

        kvs::Vector3ui length( volume->resolution() - kvs::Vector3ui( 1, 1, 1 ) );
        return length.x() * length.y() * length.z();
    }
    else
    {
        const pbvr::UnstructuredVolumeObject* volume =
            reinterpret_cast<const pbvr::UnstructuredVolumeObject*>( object );

        // Set a cell interpolator. Value type is assumed as float.
#if _OPENMP
        int max_threads = omp_get_max_threads();
#else
        int max_threads = 1;
#endif
        pbvr::CellBase<float>** cell = new pbvr::CellBase<float>* [max_threads];;
        // add by @hira at 2016/12/01
        for ( int n = 0; n < max_threads; n++ ) cell[n] = NULL;

        switch ( volume->cellType() )
        {
        case pbvr::VolumeObjectBase::Tetrahedra:
        {
            for ( int n = 0; n < max_threads; n++ )
            {
                cell[n] = new pbvr::TetrahedralCell<float>( *volume );
            }
            break;
        }
        case pbvr::VolumeObjectBase::QuadraticTetrahedra:
        {
            for ( int n = 0; n < max_threads; n++ )
            {
                cell[n] = new pbvr::QuadraticTetrahedralCell<float>( *volume );
            }
            break;
        }
        case pbvr::VolumeObjectBase::Hexahedra:
        {
            for ( int n = 0; n < max_threads; n++ )
            {
                cell[n] = new pbvr::HexahedralCell<float>( *volume );
            }
            break;
        }
        case pbvr::VolumeObjectBase::QuadraticHexahedra:
        {
            for ( int n = 0; n < max_threads; n++ )
            {
                cell[n] = new pbvr::QuadraticHexahedralCell<float>( *volume );
            }
            break;
        }
        case pbvr::VolumeObjectBase::Prism:
        {
            for ( int n = 0; n < max_threads; n++ )
            {
                cell[n] = new pbvr::PrismaticCell<float>( *volume );
            }
            break;
        }
        case pbvr::VolumeObjectBase::Pyramid:
        {
            for ( int n = 0; n < max_threads; n++ )
            {
                cell[n] = new pbvr::PyramidalCell<float>( *volume );
            }
            break;
        }
        case pbvr::VolumeObjectBase::Triangle:
        {
            for ( int n = 0; n < max_threads; n++ )
            {
                cell[n] = new pbvr::TriangleCell<float>( *volume );
            }
            break;
        }
        case pbvr::VolumeObjectBase::QuadraticTriangle:
        {
            for ( int n = 0; n < max_threads; n++ )
            {
                cell[n] = new pbvr::QuadraticTriangleCell<float>( *volume );
            }
            break;
        }
        case pbvr::VolumeObjectBase::Square:
        {
            for ( int n = 0; n < max_threads; n++ )
            {
                cell[n] = new pbvr::SquareCell<float>( *volume );
            }
            break;
        }
        case pbvr::VolumeObjectBase::QuadraticSquare:
        {
            for ( int n = 0; n < max_threads; n++ )
            {
                cell[n] = new pbvr::QuadraticSquareCell<float>( *volume );
            }
            break;
        }
        default:
        {
            kvsMessageError( "Unsupported cell type." );
            break;
        }
        }

        double total_volume = 0.0f;
        const size_t ncells = volume->ncells();

#pragma omp parallel reduction(+: total_volume)
        {
#if _OPENMP
            int thid     = omp_get_thread_num();
#else
            int thid     = 0;
#endif
#pragma omp for schedule(dynamic)
            for ( long index = 0; index < ncells; ++index )
            {
                // add by @hira by 2016/12/01
                if (cell[thid] == NULL) continue;

                // Bind the cell which is indicated by 'index'.
                cell[thid]->bindCell( index );

                // Calculate a number of particles in this cell.
                const double volume_of_cell = cell[thid]->volume();
                total_volume += volume_of_cell;
            } // end of 'cell' for-loop
        }
        // add by @hira at 2016/12/01
        for ( int n = 0; n < max_threads; n++ ) {
            if (cell[n] != NULL) delete cell[n];
        }
        // modify by @hira at 2016/12/01
        //delete cell;
        delete[] cell;

        return total_volume;
    }
}

inline const float CalculateGreatDensity(
    const kvs::Camera&           camera,
    const pbvr::ObjectBase&       object,
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
