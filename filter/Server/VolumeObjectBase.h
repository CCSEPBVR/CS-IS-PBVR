/****************************************************************************/
/**
 *  @file VolumeObjectBase.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: VolumeObjectBase.h 848 2011-06-29 11:35:52Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef PBVR__VOLUME_OBJECT_BASE_H_INCLUDE
#define PBVR__VOLUME_OBJECT_BASE_H_INCLUDE

#include <string>
#include "ClassName.h"
#include "ObjectBase.h"
#include <kvs/Value>
#include <kvs/ValueArray>
#include <kvs/AnyValueArray>
#include <kvs/Math>


namespace pbvr
{

struct CoordSynthesizerStrings
{
    std::string m_x_coord_synthesizer_string;
    std::string m_y_coord_synthesizer_string;
    std::string m_z_coord_synthesizer_string;
    int         m_time_step;
    CoordSynthesizerStrings( const int ts = 0,
                             const std::string& xs = std::string(),
                             const std::string& ys = std::string(),
                             const std::string& zs = std::string() ) :
        m_time_step( ts ),
        m_x_coord_synthesizer_string( xs ),
        m_y_coord_synthesizer_string( ys ),
        m_z_coord_synthesizer_string( zs )
    {}
};


/*==========================================================================*/
/**
 *  VolumeObjectBase.
 */
/*==========================================================================*/
class VolumeObjectBase : public pbvr::ObjectBase
{
    kvsClassName( pbvr::VolumeObjectBase );

public:

    typedef pbvr::ObjectBase BaseClass;

    typedef kvs::ValueArray<float> Coords;
    typedef kvs::AnyValueArray     Values;

public:

    enum VolumeType
    {
        Structured = 0, ///< Structured volume.
        Unstructured,   ///< Unstructured volume.
    };

    enum GridType
    {
        UnknownGridType = 0,
        Uniform,             ///< Uniform grid.
        Rectilinear,         ///< Rectilinear grid.
        Curvilinear,         ///< Curvilinear grid.
        Irregular,           ///< Irregular grid.
    };

    enum CellType
    {
        UnknownCellType     = 0,
        Tetrahedra          = 4,  ///< Tetrahedral cell.
        Hexahedra           = 8,  ///< Hexahedral cell.
        QuadraticTetrahedra = 10, ///< Quadratic tetrahedral cell.
        QuadraticHexahedra  = 20, ///< Quadratic Hexahedral cell.
        Prism               = 6,  ///< Prismatic cell.
        Pyramid             = 5,  ///< Pyramidal cell.
        Point               = 1,  ///< Point.
        Triangle            = 3,  ///< Triangle cell.
        QuadraticTriangle   = 9,  ///< Quadratic Triangle cell.
        Square              = 11, ///< Square cell.
        QuadraticSquare     = 12  ///< Quadratic Square cell.
    };

private:

    std::string m_label; ///< data label
    size_t m_veclen; ///< Vector length.

    Coords m_coords; ///< Coordinate array.
    Values m_values; ///< Value array.

    mutable bool        m_has_min_max_values; ///< Whether includes min/max values or not.
    mutable kvs::Real64 m_min_value;          ///< Minimum field value.
    mutable kvs::Real64 m_max_value;          ///< Maximum field value.

    CoordSynthesizerStrings*     m_pCoordSynthStrs;    ///< String set for coord synthesizer

public:

    VolumeObjectBase();

    VolumeObjectBase(
        const size_t     veclen,
        const Coords&    coords,
        const Values&    values );

    VolumeObjectBase( const VolumeObjectBase& other );

    virtual ~VolumeObjectBase();

public:

    static pbvr::VolumeObjectBase* DownCast( pbvr::ObjectBase* object );

    static const pbvr::VolumeObjectBase* DownCast( const pbvr::ObjectBase& object );

public:

    friend std::ostream& operator << ( std::ostream& os, const VolumeObjectBase& object );

public:

    void setLabel( const std::string& label );

    void setVeclen( const size_t veclen );

    void setCoords( const Coords& values );

    void setValues( const Values& values );

    void setMinMaxValues(
        const kvs::Real64 min_value,
        const kvs::Real64 max_value ) const;

    void setCoordSynthesizerStrings( const CoordSynthesizerStrings& pcss );

public:

    const std::string& label() const;

    const size_t veclen() const;

    const Coords& coords() const;

    const Values& values() const;

    const bool hasMinMaxValues() const;

    const kvs::Real64 minValue() const;

    const kvs::Real64 maxValue() const;

    const CoordSynthesizerStrings* getCoordSynthesizerStrings() const;

public:

    const ObjectType objectType() const;

    virtual const VolumeType volumeType() const = 0;

    virtual const GridType gridType() const = 0;

    virtual const CellType cellType() const = 0;

    virtual const size_t nnodes() const = 0;

    void updateMinMaxValues() const;

public:

    void shallowCopy( const VolumeObjectBase& object );

    void deepCopy( const VolumeObjectBase& object );

private:

    template<typename T>
    void calculate_min_max_values() const;
};

template<typename T>
void VolumeObjectBase::calculate_min_max_values() const
{
    const T*       value = reinterpret_cast<const T*>( m_values.pointer() );
    const T* const end   = value + this->nnodes() * m_veclen;

    if ( m_veclen == 1 )
    {
        T min_value = *value;
        T max_value = *value;

        while ( value < end )
        {
            min_value = kvs::Math::Min( *value, min_value );
            max_value = kvs::Math::Max( *value, max_value );
            ++value;
        }

        this->setMinMaxValues(
            static_cast<kvs::Real64>( min_value ),
            static_cast<kvs::Real64>( max_value ) );
    }
    else
    {
        kvs::Real64 min_value = kvs::Value<kvs::Real64>::Max();
        kvs::Real64 max_value = kvs::Value<kvs::Real64>::Min();

        const size_t veclen = m_veclen;

        while ( value < end )
        {
            kvs::Real64 magnitude = 0.0;
            for ( size_t i = 0; i < veclen; ++i )
            {
                magnitude += static_cast<kvs::Real64>( ( *value ) * ( *value ) );
                ++value;
            }

            min_value = kvs::Math::Min( magnitude, min_value );
            max_value = kvs::Math::Max( magnitude, max_value );
        }

        this->setMinMaxValues( std::sqrt( min_value ), std::sqrt( max_value ) );
    }
}

} // end of namespace pbvr

#endif // KVS__VOLUME_OBJECT_BASE_H_INCLUDE
