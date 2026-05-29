/****************************************************************************/
/**
 *  @file TransferFunction.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TransferFunction.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__TRANSFER_FUNCTION_H_INCLUDE
#define VIS_MODULE__TRANSFER_FUNCTION_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/ColorMap>
#include <vismodule/OpacityMap>
#include <vismodule/VolumeObjectBase>


namespace vismodule
{

/*==========================================================================*/
/**
 *  TransferFunction.
 */
/*==========================================================================*/
class TransferFunction
{
    visModuleClassName( vismodule::TransferFunction );

private:

    vismodule::ColorMap   m_color_map;   ///< Color map.
    vismodule::OpacityMap m_opacity_map; ///< Opacity map.

public:

    explicit TransferFunction( const std::size_t resolution = 256 );

    //TransferFunction( const std::string& filename );

    // 'explicit' is not specified by design.
    TransferFunction( const vismodule::ColorMap& color_map );

    // 'explicit' is not specified by design.
    TransferFunction( const vismodule::OpacityMap& opacity_map );

    TransferFunction(
        const vismodule::ColorMap&   color_map,
        const vismodule::OpacityMap& opacity_map );

    TransferFunction( const TransferFunction& other );

    virtual ~TransferFunction();

public:

    void setColorMap( const vismodule::ColorMap& color_map );

    void setOpacityMap( const vismodule::OpacityMap& opacity_map );

    void setRange( const float min_value, const float max_value );

    void setColorRange( const float min_value, const float max_value );
  
    void setOpacityRange( const float min_value, const float max_value );

    void setRange( const vismodule::VolumeObjectBase& volume );

    void adjustRange( const float min_value, const float max_value );

    void adjustRange( const vismodule::VolumeObjectBase& volume );

    const bool hasRange() const;

    const float minValue() const;

    const float maxValue() const;

// opacity, color用minmaxを追加 20250730
    const float opacityMinValue() const;
    const float opacityMaxValue() const;
    const float colorMinValue() const;
    const float colorMaxValue() const;


public:

    const vismodule::ColorMap& colorMap() const;

    const vismodule::OpacityMap& opacityMap() const;

public:

    const std::size_t resolution() const;

public:

    void create( const std::size_t resolution );

    const bool read( const std::string& filename );

    //const bool write( const std::string& filename );

public:

    TransferFunction& operator =( const TransferFunction& rhs );
};

} // end of namespace vismodule

#endif // VIS_MODULE__TRANSFER_FUNCTION_H_INCLUDE
