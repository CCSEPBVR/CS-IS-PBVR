/****************************************************************************/
/**
 *  @file KVSMLObjectPlotOverTime.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KVSMLObjectPlotOverTime.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__KVSML_OBJECT_PLOT_OVER_TIME_H_INCLUDE
#define VIS_MODULE__KVSML_OBJECT_PLOT_OVER_TIME_H_INCLUDE

#include <string>
#include <sstream>
#include <vismodule/ValueArray>
#include <vismodule/FileFormatBase>

namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  KVSML point object format.
 */
/*===========================================================================*/
class KVSMLObjectPlotOverTime : public vismodule::FileFormatBase
{
    visModuleClassName( vismodule::KVSMLObjectPlotOverTime );

protected:
    vismodule::ValueArray<float> m_values_on_time;
    bool                         m_mask;

public:
    KVSMLObjectPlotOverTime();
    KVSMLObjectPlotOverTime( const std::string& filename );
    KVSMLObjectPlotOverTime( 
        const vismodule::ValueArray<float> values_on_time,
        const bool mask
    );   
    virtual ~KVSMLObjectPlotOverTime();

    vismodule::ValueArray<float>& values_on_time(){ return m_values_on_time; }
    const vismodule::ValueArray<float>& values_on_time() const { return m_values_on_time; };
    bool mask(){ return m_mask; }
    const bool mask() const { return m_mask; }

    void setValuesOnTime( const vismodule::ValueArray<float>& values_on_time );
    void setMask( bool mask );

    const bool read( const std::string& filename );
    const bool write( const std::string& filename );

    std::vector<std::string> split( const std::string& input, char delimiter )
    {
        std::istringstream stream( input );

        std::string field;
        std::vector<std::string> result;
        while ( std::getline( stream, field, delimiter ) ) {
            result.push_back( field );
        }
        return result;
    }

    friend std::ostream& operator <<( std::ostream& os, const KVSMLObjectPlotOverTime& rhs );
};

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML_OBJECT_PLOT_OVER_TIME_H_INCLUDE