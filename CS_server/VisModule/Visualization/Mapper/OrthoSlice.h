/****************************************************************************/
/**
 *  @file OrthoSlice.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: OrthoSlice.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__ORTHO_SLICE_H_INCLUDE
#define VIS_MODULE__ORTHO_SLICE_H_INCLUDE

#include <vismodule/SlicePlane>
#include <vismodule/VolumeObjectBase>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Axis aligned slice plane class.
 */
/*==========================================================================*/
class OrthoSlice : public vismodule::SlicePlane
{
    // Class name.
    visModuleClassName( vismodule::OrthoSlice );

    // Module information.
    visModuleCategory( Mapper );
    visModuleSuperClass( vismodule::SlicePlane );

public:

    enum AlignedAxis
    {
        XAxis = 0,
        YAxis = 1,
        ZAxis = 2
    };

protected:

    AlignedAxis m_aligned_axis; ///< aligned axis

public:

    OrthoSlice( void );

    OrthoSlice(
        const vismodule::VolumeObjectBase& volume,
        const float                  position,
        const AlignedAxis            aligned_axis,
        const vismodule::TransferFunction& transfer_function );

public:

    void setPlane( const float position, const vismodule::OrthoSlice::AlignedAxis axis );
};

} // end of namespace vismodule

#endif // VIS_MODULE__ORTHO_SLICE_H_INCLUDE
