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
#ifndef VIS_MODULE__VOLUME_OBJECT_BASE_H_INCLUDE
#define VIS_MODULE__VOLUME_OBJECT_BASE_H_INCLUDE

#include <string>
#include <vismodule/ClassName>
#include <vismodule/ObjectBase>
#include <vismodule/Value>
#include <vismodule/ValueArray>
#include <vismodule/AnyValueArray>
#include <vismodule/Math>


namespace vismodule
{

/*==========================================================================*/
/**
 *  VolumeObjectBase.
 */
/*==========================================================================*/
class VolumeObjectBase
    : public vismodule::ObjectBase
{
    visModuleClassName( vismodule::VolumeObjectBase );

public:

    typedef vismodule::ObjectBase BaseClass;

    typedef vismodule::ValueArray<float> Coords;
    typedef vismodule::AnyValueArray     Values;

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
        Pyramid             = 5,  ///< Pyramidal cell.
        Point               = 1   ///< Point.
    };

private:

    std::string m_label; ///< data label
    size_t m_veclen; ///< Vector length.

    Coords m_coords; ///< Coordinate array.
    Values m_values; ///< Value array.

    mutable bool        m_has_min_max_values; ///< Whether includes min/max values or not.
    mutable vismodule::Real64 m_min_value;          ///< Minimum field value.
    mutable vismodule::Real64 m_max_value;          ///< Maximum field value.

public:

    VolumeObjectBase( void );

    VolumeObjectBase(
        const size_t     veclen,
        const Coords&    coords,
        const Values&    values );

    VolumeObjectBase( const VolumeObjectBase& other );

    virtual ~VolumeObjectBase( void );

public:

    static vismodule::VolumeObjectBase* DownCast( vismodule::ObjectBase* object );

    static const vismodule::VolumeObjectBase* DownCast( const vismodule::ObjectBase* object );

public:

    friend std::ostream& operator << ( std::ostream& os, const VolumeObjectBase& object );

public:

    void setLabel( const std::string& label );

    void setVeclen( const size_t veclen );

    void setCoords( const Coords& values );

    void setValues( const Values& values );

    void setMinMaxValues(
        const vismodule::Real64 min_value,
        const vismodule::Real64 max_value ) const;

public:

    const std::string& label( void ) const;

    const size_t veclen( void ) const;

    const Coords& coords( void ) const;

    const Values& values( void ) const;

    const bool hasMinMaxValues( void ) const;

    const vismodule::Real64 minValue( void ) const;

    const vismodule::Real64 maxValue( void ) const;

public:

    const ObjectType objectType( void ) const;

    virtual const VolumeType volumeType( void ) const = 0;

    virtual const GridType gridType( void ) const = 0;

    virtual const CellType cellType( void ) const = 0;

    virtual const size_t nnodes( void ) const = 0;

    void updateMinMaxValues( void ) const;

public:

    void shallowCopy( const VolumeObjectBase& object );

    void deepCopy( const VolumeObjectBase& object );

private:

    template<typename T>
    void calculate_min_max_values( void ) const;
};

template<typename T>
void VolumeObjectBase::calculate_min_max_values( void ) const
{
    const T*       value = reinterpret_cast<const T*>( m_values.pointer() );
    const T* const end   = value + this->nnodes() * m_veclen;

    if ( m_veclen == 1 )
    {
        T min_value = *value;
        T max_value = *value;

        while ( value < end )
        {
            min_value = vismodule::Math::Min( *value, min_value );
            max_value = vismodule::Math::Max( *value, max_value );
            ++value;
        }

        this->setMinMaxValues(
            static_cast<vismodule::Real64>( min_value ),
            static_cast<vismodule::Real64>( max_value ) );
    }
    else
    {
        vismodule::Real64 min_value = vismodule::Value<vismodule::Real64>::Max();
        vismodule::Real64 max_value = vismodule::Value<vismodule::Real64>::Min();

        const size_t veclen = m_veclen;

        while ( value < end )
        {
            vismodule::Real64 magnitude = 0.0;
            for ( size_t i = 0; i < veclen; ++i )
            {
                magnitude += static_cast<vismodule::Real64>( ( *value ) * ( *value ) );
                ++value;
            }

            min_value = vismodule::Math::Min( magnitude, min_value );
            max_value = vismodule::Math::Max( magnitude, max_value );
        }

        this->setMinMaxValues( std::sqrt( min_value ), std::sqrt( max_value ) );
    }
}

} // end of namespace vismodule

#endif // VIS_MODULE__VOLUME_OBJECT_BASE_H_INCLUDE
