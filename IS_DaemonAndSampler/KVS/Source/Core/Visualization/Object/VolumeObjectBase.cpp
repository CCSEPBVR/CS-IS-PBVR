/****************************************************************************/
/**
 *  @file VolumeObjectBase.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: VolumeObjectBase.cpp 848 2011-06-29 11:35:52Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "VolumeObjectBase.h"


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructs a new empty VolumeObjectBase.
 */
/*==========================================================================*/
VolumeObjectBase::VolumeObjectBase( void )
    : kvs::ObjectBase()
    , m_label( "" )
    , m_veclen( 0 )
    , m_coords()
    , m_values()
    , m_has_min_max_values( false )
    , m_min_value( 0.0 )
    , m_max_value( 0.0 )
{
}

/*==========================================================================*/
/**
 *  Constructs a new VolumeObjectBase.
 *
 *  @param veclen     [in] Vector length.
 *  @param coords     [in] Coordinate array.
 *  @param values     [in] Value array.
 */
/*==========================================================================*/
VolumeObjectBase::VolumeObjectBase(
    const size_t     veclen,
    const Coords&    coords,
    const Values&    values )
    : kvs::ObjectBase()
    , m_label( "" )
    , m_veclen( veclen )
    , m_coords( coords )
    , m_values( values )
    , m_has_min_max_values( false )
    , m_min_value( 0.0 )
    , m_max_value( 0.0 )
{
}

/*==========================================================================*/
/**
 *  Constructs a copy of other.
 *
 *  @param other [in] Structured volume.
 */
/*==========================================================================*/
VolumeObjectBase::VolumeObjectBase( const VolumeObjectBase& other )
    : kvs::ObjectBase( other )
    , m_label( other.label() )
    , m_veclen( other.veclen() )
    , m_coords( other.coords() )
    , m_values( other.values() )
    , m_has_min_max_values( other.hasMinMaxValues() )
    , m_min_value( other.minValue() )
    , m_max_value( other.maxValue() )
{
    // this->shallowCopy( other );
}

/*==========================================================================*/
/**
 *  Destroys the VolumeObjectBase.
 */
/*==========================================================================*/
VolumeObjectBase::~VolumeObjectBase( void )
{
}

kvs::VolumeObjectBase* VolumeObjectBase::DownCast( kvs::ObjectBase* object )
{
    const kvs::ObjectBase::ObjectType type = object->objectType();
    if ( type != kvs::ObjectBase::Volume )
    {
        kvsMessageError("Input object is not a volume object.");
        return( NULL );
    }

    kvs::VolumeObjectBase* volume = static_cast<kvs::VolumeObjectBase*>( object );

    return( volume );
}

const kvs::VolumeObjectBase* VolumeObjectBase::DownCast( const kvs::ObjectBase* object )
{
    return( VolumeObjectBase::DownCast( const_cast<kvs::ObjectBase*>( object ) ) );
}

std::ostream& operator << ( std::ostream& os, const kvs::VolumeObjectBase& object )
{
#ifdef KVS_COMPILER_VC
#if KVS_COMPILER_VERSION_LESS_OR_EQUAL( 8, 0 )
    // @TODO Cannot instance the object that is a abstract class here (error:C2259).
#endif
#else
#ifndef NO_CLIENT
    os << static_cast<const kvs::ObjectBase&>( object ) << std::endl;
#endif
#endif

    const std::ios_base::fmtflags flags( os.flags() );
    os << "Veclen:  " << object.veclen() << std::endl;
    os.setf( std::ios::boolalpha );
    os << "Set of min/max value:  " << object.hasMinMaxValues() << std::endl;
    os.unsetf( std::ios::boolalpha );
    os << "Min value:  " << object.minValue() << std::endl;
    os << "Max value:  " << object.maxValue();
    os.flags( flags );

    return( os );
}

/*==========================================================================*/
/**
 *  Sets data label.
 */
/*==========================================================================*/
void VolumeObjectBase::setLabel( const std::string& label )
{
    m_label = label;
}

/*==========================================================================*/
/**
 *  Sets the vector length.
 */
/*==========================================================================*/
void VolumeObjectBase::setVeclen( const size_t veclen )
{
    m_veclen = veclen;
}

/*==========================================================================*/
/**
 *  Sets the coordinate array.
 *
 *  @param coords [in] Coordinate array.
 */
/*==========================================================================*/
void VolumeObjectBase::setCoords( const Coords& coords )
{
    m_coords = coords;
}

/*==========================================================================*/
/**
 *  Sets the value array.
 *
 *  @param values [in] Value array.
 */
/*==========================================================================*/
void VolumeObjectBase::setValues( const Values& values )
{
    m_values = values;
}

/*==========================================================================*/
/**
 *  Sets the min/max values.
 *  @param min_value [in] Minimum value.
 *  @param max_value [in] Maximum value.
 */
/*==========================================================================*/
void VolumeObjectBase::setMinMaxValues(
    const kvs::Real64 min_value,
    const kvs::Real64 max_value ) const
{
    m_min_value          = min_value;
    m_max_value          = max_value;
    m_has_min_max_values = true;
}


const kvs::ObjectBase::ObjectType VolumeObjectBase::objectType( void ) const
{
    return( kvs::ObjectBase::Volume );
}

const std::string& VolumeObjectBase::label( void ) const
{
    return( m_label );
}

/*==========================================================================*/
/**
 *  Returns the vector length.
 */
/*==========================================================================*/
const size_t VolumeObjectBase::veclen( void ) const
{
    return( m_veclen );
}

/*==========================================================================*/
/**
 *  Returns the coord array.
 */
/*==========================================================================*/
const VolumeObjectBase::Coords& VolumeObjectBase::coords( void ) const
{
    return( m_coords );
}

/*==========================================================================*/
/**
 *  Returns the value array.
 */
/*==========================================================================*/
const VolumeObjectBase::Values& VolumeObjectBase::values( void ) const
{
    return( m_values );
}

/*==========================================================================*/
/**
 *  Returns true if this class has the min/max values; otherwise returns false.
 *
 *  @return Whether this class has the min/max values or not.
 */
/*==========================================================================*/
const bool VolumeObjectBase::hasMinMaxValues( void ) const
{
    return( m_has_min_max_values );
}

/*==========================================================================*/
/**
 *  Returns the minimum value.
 *
 *  @return Minimum value.
 */
/*==========================================================================*/
const kvs::Real64 VolumeObjectBase::minValue( void ) const
{
    return( m_min_value );
}

/*==========================================================================*/
/**
 *  Returns the maximum value.
 *
 *  @return Maximum value.
 */
/*==========================================================================*/
const kvs::Real64 VolumeObjectBase::maxValue( void ) const
{
    return( m_max_value );
}

/*==========================================================================*/
/**
 *  Update the min/max node value.
 */
/*==========================================================================*/
void VolumeObjectBase::updateMinMaxValues( void ) const
{
    const std::type_info& type = m_values.typeInfo()->type();
    if (      type == typeid( kvs::Int8   ) ) { this->calculate_min_max_values<kvs::Int8  >(); }
    else if ( type == typeid( kvs::Int16  ) ) { this->calculate_min_max_values<kvs::Int16 >(); }
    else if ( type == typeid( kvs::Int32  ) ) { this->calculate_min_max_values<kvs::Int32 >(); }
    else if ( type == typeid( kvs::Int64  ) ) { this->calculate_min_max_values<kvs::Int64 >(); }
    else if ( type == typeid( kvs::UInt8  ) ) { this->calculate_min_max_values<kvs::UInt8 >(); }
    else if ( type == typeid( kvs::UInt16 ) ) { this->calculate_min_max_values<kvs::UInt16>(); }
    else if ( type == typeid( kvs::UInt32 ) ) { this->calculate_min_max_values<kvs::UInt32>(); }
    else if ( type == typeid( kvs::UInt64 ) ) { this->calculate_min_max_values<kvs::UInt64>(); }
    else if ( type == typeid( kvs::Real32 ) ) { this->calculate_min_max_values<kvs::Real32>(); }
    else if ( type == typeid( kvs::Real64 ) ) { this->calculate_min_max_values<kvs::Real64>(); }
}

void VolumeObjectBase::shallowCopy( const VolumeObjectBase& object )
{
    BaseClass::operator=( object );
    this->m_has_min_max_values = object.hasMinMaxValues();
    this->m_min_value = object.minValue();
    this->m_max_value = object.maxValue();
    this->m_label = object.label();
    this->m_veclen = object.veclen();
    this->m_coords.shallowCopy( object.coords() );
    this->m_values.shallowCopy( object.values() );
}

void VolumeObjectBase::deepCopy( const VolumeObjectBase& object )
{
    BaseClass::operator=( object );
    this->m_has_min_max_values = object.hasMinMaxValues();
    this->m_min_value = object.minValue();
    this->m_max_value = object.maxValue();
    this->m_label = object.label();
    this->m_veclen = object.veclen();
    this->m_coords.deepCopy( object.coords() );

    const size_t size = object.values().size();
    const std::type_info& type = object.values().typeInfo()->type();
    if (      type == typeid( kvs::Int8 ) )   { this->m_values.deepCopy( object.values().pointer<kvs::Int8>(), size ); }
    else if ( type == typeid( kvs::UInt8 ) )  { this->m_values.deepCopy( object.values().pointer<kvs::UInt8>(), size ); }
    else if ( type == typeid( kvs::Int16 ) )  { this->m_values.deepCopy( object.values().pointer<kvs::Int16>(), size ); }
    else if ( type == typeid( kvs::UInt16 ) ) { this->m_values.deepCopy( object.values().pointer<kvs::UInt16>(), size ); }
    else if ( type == typeid( kvs::Int32 ) )  { this->m_values.deepCopy( object.values().pointer<kvs::Int32>(), size ); }
    else if ( type == typeid( kvs::UInt32 ) ) { this->m_values.deepCopy( object.values().pointer<kvs::UInt32>(), size ); }
    else if ( type == typeid( kvs::Int64 ) )  { this->m_values.deepCopy( object.values().pointer<kvs::Int64>(), size ); }
    else if ( type == typeid( kvs::UInt64 ) ) { this->m_values.deepCopy( object.values().pointer<kvs::UInt64>(), size ); }
    else if ( type == typeid( kvs::Real32 ) ) { this->m_values.deepCopy( object.values().pointer<kvs::Real32>(), size ); }
    else if ( type == typeid( kvs::Real64 ) ) { this->m_values.deepCopy( object.values().pointer<kvs::Real64>(), size ); }
}

} // end of namespace kvs
