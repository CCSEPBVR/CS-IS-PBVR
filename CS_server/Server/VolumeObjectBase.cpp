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


namespace pbvr
{

/*==========================================================================*/
/**
 *  Constructs a new empty VolumeObjectBase.
 */
/*==========================================================================*/
VolumeObjectBase::VolumeObjectBase():
    pbvr::ObjectBase(),
    m_label( "" ),
    m_veclen( 0 ),
    m_coords(),
    m_values(),
    m_has_min_max_values( false ),
    m_min_value( 0.0 ),
    m_max_value( 0.0 ),
    m_pCoordSynthStrs( NULL )
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
    const Values&    values ):
    pbvr::ObjectBase(),
    m_label( "" ),
    m_veclen( veclen ),
    m_coords( coords ),
    m_values( values ),
    m_has_min_max_values( false ),
    m_min_value( 0.0 ),
    m_max_value( 0.0 ),
    m_pCoordSynthStrs( NULL )
{
}

/*==========================================================================*/
/**
 *  Constructs a copy of other.
 *
 *  @param other [in] Structured volume.
 */
/*==========================================================================*/
VolumeObjectBase::VolumeObjectBase( const VolumeObjectBase& other ):
    pbvr::ObjectBase( other ),
    m_label( other.label() ),
    m_veclen( other.veclen() ),
    m_coords( other.coords() ),
    m_values( other.values() ),
    m_has_min_max_values( other.hasMinMaxValues() ),
    m_min_value( other.minValue() ),
    m_max_value( other.maxValue() ),
    m_pCoordSynthStrs( NULL )
{
    // this->shallowCopy( other );
    setCoordSynthesizerStrings( *other.getCoordSynthesizerStrings() );
    //2023 shimomura 
    setCoordSynthesizerTokens(  *other.getCoordSynthesizerTokens() );
}

/*==========================================================================*/
/**
 *  Destroys the VolumeObjectBase.
 */
/*==========================================================================*/
VolumeObjectBase::~VolumeObjectBase()
{
    if ( m_pCoordSynthStrs ) delete m_pCoordSynthStrs;
}

pbvr::VolumeObjectBase* VolumeObjectBase::DownCast( pbvr::ObjectBase* object )
{
    const pbvr::ObjectBase::ObjectType type = object->objectType();
    if ( type != pbvr::ObjectBase::Volume )
    {
        kvsMessageError( "Input object is not a volume object." );
        return NULL;
    }

    pbvr::VolumeObjectBase* volume = static_cast<pbvr::VolumeObjectBase*>( object );

    return volume;
}

const pbvr::VolumeObjectBase* VolumeObjectBase::DownCast( const pbvr::ObjectBase& object )
{
    return VolumeObjectBase::DownCast( const_cast<pbvr::ObjectBase*>( &object ) );
}

std::ostream& operator << ( std::ostream& os, const pbvr::VolumeObjectBase& object )
{
#ifdef KVS_COMPILER_VC
//#if PBVR_COMPILER_VERSION_LESS_OR_EQUAL( 8, 0 )
//    // @TODO Cannot instance the object that is a abstract class here (error:C2259).
//#endif
#else
    //os << static_cast<const pbvr::ObjectBase&>( object ) << std::endl;
#endif

    const std::ios_base::fmtflags flags( os.flags() );
    os << "Veclen:  " << object.veclen() << std::endl;
    os.setf( std::ios::boolalpha );
    os << "Set of min/max value:  " << object.hasMinMaxValues() << std::endl;
    os.unsetf( std::ios::boolalpha );
    os << "Min value:  " << object.minValue() << std::endl;
    os << "Max value:  " << object.maxValue();
    os.flags( flags );

    return os;
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

void VolumeObjectBase::setCoordSynthesizerStrings( const CoordSynthesizerStrings& pcss )
{
    if ( m_pCoordSynthStrs )
    {
        delete m_pCoordSynthStrs;
        m_pCoordSynthStrs = NULL;
    }
    if ( &pcss == NULL ) return;
    m_pCoordSynthStrs = new CoordSynthesizerStrings();
    *m_pCoordSynthStrs = pcss;
}

//2023 shimomura
void VolumeObjectBase::setCoordSynthesizerTokens( const CoordSynthesizerTokens& pcst )
{
//    std::cout << " token   0" << std::endl;
//    if ( m_pCoordSynthTkns )
//    {
//        std::cout << " token  is  not NULL" << std::endl;
//        delete m_pCoordSynthTkns;
//        m_pCoordSynthTkns = NULL;
//    }
    if ( &pcst == NULL ) return;
    m_pCoordSynthTkns = new CoordSynthesizerTokens();
    *m_pCoordSynthTkns = pcst;
    //m_pCoordSynthTkns.m_x_coord_synthesizer_token = pcst.m_x_coord_synthesizer_token;
    //m_pCoordSynthTkns.m_y_coord_synthesizer_token = pcst.m_y_coord_synthesizer_token;
    //m_pCoordSynthTkns.m_z_coord_synthesizer_token = pcst.m_z_coord_synthesizer_token;
}

const pbvr::ObjectBase::ObjectType VolumeObjectBase::objectType() const
{
    return pbvr::ObjectBase::Volume;
}

const std::string& VolumeObjectBase::label() const
{
    return m_label;
}

/*==========================================================================*/
/**
 *  Returns the vector length.
 */
/*==========================================================================*/
const size_t VolumeObjectBase::veclen() const
{
    return m_veclen;
}

/*==========================================================================*/
/**
 *  Returns the coord array.
 */
/*==========================================================================*/
const VolumeObjectBase::Coords& VolumeObjectBase::coords() const
{
    return m_coords;
}

/*==========================================================================*/
/**
 *  Returns the value array.
 */
/*==========================================================================*/
const VolumeObjectBase::Values& VolumeObjectBase::values() const
{
    return m_values;
}

/*==========================================================================*/
/**
 *  Returns true if this class has the min/max values; otherwise returns false.
 *
 *  @return Whether this class has the min/max values or not.
 */
/*==========================================================================*/
const bool VolumeObjectBase::hasMinMaxValues() const
{
    return m_has_min_max_values;
}

/*==========================================================================*/
/**
 *  Returns the minimum value.
 *
 *  @return Minimum value.
 */
/*==========================================================================*/
const kvs::Real64 VolumeObjectBase::minValue() const
{
    return m_min_value;
}

/*==========================================================================*/
/**
 *  Returns the maximum value.
 *
 *  @return Maximum value.
 */
/*==========================================================================*/
const kvs::Real64 VolumeObjectBase::maxValue() const
{
    return m_max_value;
}

/*==========================================================================*/
/**
 *  Update the min/max node value.
 */
/*==========================================================================*/
void VolumeObjectBase::updateMinMaxValues() const
{
    const std::type_info& type = m_values.typeInfo()->type();
    if (      type == typeid( kvs::Int8   ) )
    {
        this->calculate_min_max_values<kvs::Int8  >();
    }
    else if ( type == typeid( kvs::Int16  ) )
    {
        this->calculate_min_max_values<kvs::Int16 >();
    }
    else if ( type == typeid( kvs::Int32  ) )
    {
        this->calculate_min_max_values<kvs::Int32 >();
    }
    else if ( type == typeid( kvs::Int64  ) )
    {
        this->calculate_min_max_values<kvs::Int64 >();
    }
    else if ( type == typeid( kvs::UInt8  ) )
    {
        this->calculate_min_max_values<kvs::UInt8 >();
    }
    else if ( type == typeid( kvs::UInt16 ) )
    {
        this->calculate_min_max_values<kvs::UInt16>();
    }
    else if ( type == typeid( kvs::UInt32 ) )
    {
        this->calculate_min_max_values<kvs::UInt32>();
    }
    else if ( type == typeid( kvs::UInt64 ) )
    {
        this->calculate_min_max_values<kvs::UInt64>();
    }
    else if ( type == typeid( kvs::Real32 ) )
    {
        this->calculate_min_max_values<kvs::Real32>();
    }
    else if ( type == typeid( kvs::Real64 ) )
    {
        this->calculate_min_max_values<kvs::Real64>();
    }
}

const CoordSynthesizerStrings* VolumeObjectBase::getCoordSynthesizerStrings() const
{
    return m_pCoordSynthStrs;
}

const CoordSynthesizerTokens*  VolumeObjectBase::getCoordSynthesizerTokens() const
{
    return m_pCoordSynthTkns;
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
    this->setCoordSynthesizerStrings( *object.getCoordSynthesizerStrings() );
    //2023 shimomura
    this->setCoordSynthesizerTokens( *object.getCoordSynthesizerTokens() );
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
    this->setCoordSynthesizerStrings( *object.getCoordSynthesizerStrings() );
    //2023 shimomura
    this->setCoordSynthesizerTokens( *object.getCoordSynthesizerTokens() );

    const size_t size = object.values().size();
    const std::type_info& type = object.values().typeInfo()->type();
    if (      type == typeid( kvs::Int8 ) )
    {
        this->m_values.deepCopy( object.values().pointer<kvs::Int8>(), size );
    }
    else if ( type == typeid( kvs::UInt8 ) )
    {
        this->m_values.deepCopy( object.values().pointer<kvs::UInt8>(), size );
    }
    else if ( type == typeid( kvs::Int16 ) )
    {
        this->m_values.deepCopy( object.values().pointer<kvs::Int16>(), size );
    }
    else if ( type == typeid( kvs::UInt16 ) )
    {
        this->m_values.deepCopy( object.values().pointer<kvs::UInt16>(), size );
    }
    else if ( type == typeid( kvs::Int32 ) )
    {
        this->m_values.deepCopy( object.values().pointer<kvs::Int32>(), size );
    }
    else if ( type == typeid( kvs::UInt32 ) )
    {
        this->m_values.deepCopy( object.values().pointer<kvs::UInt32>(), size );
    }
    else if ( type == typeid( kvs::Int64 ) )
    {
        this->m_values.deepCopy( object.values().pointer<kvs::Int64>(), size );
    }
    else if ( type == typeid( kvs::UInt64 ) )
    {
        this->m_values.deepCopy( object.values().pointer<kvs::UInt64>(), size );
    }
    else if ( type == typeid( kvs::Real32 ) )
    {
        this->m_values.deepCopy( object.values().pointer<kvs::Real32>(), size );
    }
    else if ( type == typeid( kvs::Real64 ) )
    {
        this->m_values.deepCopy( object.values().pointer<kvs::Real64>(), size );
    }
}

} // end of namespace pbvr
