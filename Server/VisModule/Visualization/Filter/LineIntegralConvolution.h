
/*****************************************************************************/
/**
 *  @file   line_integral_convolution.h
 *  @brief  LIC (Line Integral Convolution) filtering class.
 *
 *  @author Naohisa Sakamoto
 *  @date   2010/10/12 21:07:36
 */
/*----------------------------------------------------------------------------
 *
 *  $Author: kawamura $
 *  $Date: 2009/01/15 12:52:12 $
 *  $Source: /home/Repository/viz-server2/cvsroot/VIS_MODULE_RC1/Source/Core/Visualization/Filter/LineIntegralConvolution.h,v $
 *  $Revision: 1.2 $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__LINE_INTEGRAL_CONVOLUTION_H_INCLUDE
#define VIS_MODULE__LINE_INTEGRAL_CONVOLUTION_H_INCLUDE

#include <vismodule/StructuredVolumeObject>
#include <vismodule/FilterBase>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  LIC class.
 */
/*===========================================================================*/
class LineIntegralConvolution : public vismodule::FilterBase, public vismodule::StructuredVolumeObject
{
    // Class name.
    visModuleClassName( vismodule::LineIntegralConvolution );

    // Module information.
    visModuleCategory( Filter );
    visModuleBaseClass( vismodule::FilterBase );
    visModuleSuperClass( vismodule::StructuredVolumeObject );

protected:

    double                       m_length; ///< stream length
    vismodule::StructuredVolumeObject* m_noise;  ///< white noise volume

public:

    LineIntegralConvolution( void );

    LineIntegralConvolution( const vismodule::StructuredVolumeObject& volume );

    LineIntegralConvolution( const vismodule::StructuredVolumeObject& volume, const double length );

    virtual ~LineIntegralConvolution( void );

public:

    void setLength( const double length );

public:

    SuperClass* exec( const vismodule::ObjectBase& object );

protected:

    void filtering( const vismodule::StructuredVolumeObject& volume );

    void create_noise_volume( const vismodule::StructuredVolumeObject& volume );

    template <typename T>
    void convolution( const vismodule::StructuredVolumeObject& volume );
};

} // end of namespace vismodule

#endif // VIS_MODULE__LINE_INTEGRAL_CONVOLUTION_H_INCLUDE
