/****************************************************************************/
/**
 *  @file Bounds.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Bounds.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__BOUNDS_H_INCLUDE
#define VIS_MODULE__BOUNDS_H_INCLUDE

#include <vector>
#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/ObjectBase>
#include <vismodule/LineObject>
#include <vismodule/Vector2>
#include <vismodule/Vector3>
#include <vismodule/Type>
#include "FilterBase.h"


namespace vismodule
{

/*==========================================================================*/
/**
 *  Create bounds object from volume data.
 */
/*==========================================================================*/
class Bounds : public vismodule::FilterBase, public vismodule::LineObject
{
    // Class name.
    visModuleClassName( vismodule::Bounds );

    // Module information.
    visModuleCategory( Filter );
    visModuleBaseClass( vismodule::FilterBase );
    visModuleSuperClass( vismodule::LineObject );

public:

    enum Type
    {
        Box    = 0, ///< box type bounds
        Corner = 1, ///< corner type bounds
        Circle = 2, ///< circle type bounds
    };

protected:

    Type  m_type;         ///< bounds type
    float m_corner_scale; ///< length of corner line
    float m_division;     ///< division of circle

public:

    Bounds( void );

    Bounds( const vismodule::ObjectBase* object, const Bounds::Type type = Bounds::Box );

    Bounds( const vismodule::Vector3f& min_coord, const vismodule::Vector3f& max_coord, const Bounds::Type type = Bounds::Box );

    virtual ~Bounds( void );

public:

    SuperClass* exec( const vismodule::ObjectBase* object );

public:

    void setType( const Bounds::Type type );

    void setCornerScale( const float corner_scale );

    void setCircleDivision( const float division );

private:

    void initialize( void );

    void create_box_bounds( void );

    void create_corner_bounds( void );

    void create_circle_bounds( void );

    void set_corner(
        const vismodule::Vector3f&      pos1,
        const vismodule::Vector3f&      pos2,
        std::vector<vismodule::Real32>* vertex,
        std::vector<vismodule::UInt32>* connect );
};

} // end of namespace vismodule

#endif // VIS_MODULE__BOUNDS_H_INCLUDE
