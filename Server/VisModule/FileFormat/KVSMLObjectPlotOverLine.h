/****************************************************************************/
/**
 *  @file KVSMLObjectPlotOverLine.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KVSMLObjectPlotOverLine.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__KVSML_OBJECT_PLOT_OVER_LINE_H_INCLUDE
#define VIS_MODULE__KVSML_OBJECT_PLOT_OVER_LINE_H_INCLUDE

#include <fstream>
#include <iostream>
#include <string>

#include <vismodule/FileFormatBase>
#include <vismodule/ValueArray>
#include <vismodule/Type>
#include <sstream>
#include <vismodule/ValueArray>
#include <vismodule/File>
#include <vismodule/Type>
#include <vismodule/File>
#include <vismodule/IgnoreUnusedVariable>
#include <iostream>
#include <fstream>
#include <sstream>



namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  KVSML point object format.
 */
/*===========================================================================*/
class KVSMLObjectPlotOverLine : public vismodule::FileFormatBase
{
    visModuleClassName( vismodule::KVSMLObjectPlotOverLine );

public:

    enum WritingDataType
    {
        Ascii = 0,     ///< ascii data type
        ExternalAscii, ///< external ascii data type
        ExternalBinary ///< external binary data type
    };

protected:

     vismodule::ValueArray<float> m_values_on_line;
     vismodule::ValueArray<float> m_x_axis;
     vismodule::ValueArray<bool>  m_mask;
public:

    KVSMLObjectPlotOverLine();

    KVSMLObjectPlotOverLine(const int resolution) {setResolution(resolution);};
    KVSMLObjectPlotOverLine( const std::string& filename );

    KVSMLObjectPlotOverLine( const vismodule::ValueArray<float> values_on_line, const vismodule::ValueArray<float>  x_axis,
                      const vismodule::ValueArray<bool> mask );
    
    virtual ~KVSMLObjectPlotOverLine( void );

public:

    vismodule::ValueArray<float>& values_on_line(){return m_values_on_line;}
    vismodule::ValueArray<float>& x_axis(){return m_x_axis;}
    vismodule::ValueArray<bool>&  mask(){return m_mask;}
    const vismodule::ValueArray<float>& values_on_line() const;
    const vismodule::ValueArray<float>& x_axis() const;
    const vismodule::ValueArray<bool>&  mask() const;
public:

    void setValuesOnLine( const vismodule::ValueArray<float>& values_on_line );
    void setXAxis( const vismodule::ValueArray<float>& x_axis );
    void setMask( const vismodule::ValueArray<bool>& mask );

    void setResolution(const int resolution);
public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );

    std::vector<std::string> split(const std::string& input, char delimiter)
    {
        std::istringstream stream(input);

        std::string field;
        std::vector<std::string> result;
        while (std::getline(stream, field, delimiter)) {
            result.push_back(field);
        }
        return result;
    }


public:

    friend std::ostream& operator <<( std::ostream& os, const KVSMLObjectPlotOverLine& rhs );
};

} // end of namespace vismodule

#endif // KVS__KVSML_OBJECT_POINT_H_INCLUDE
