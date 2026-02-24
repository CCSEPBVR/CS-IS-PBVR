#ifndef VIS_MODULE__EXTENDED_TRANSFER_FUNCTION_H_INCLUDE
#define VIS_MODULE__EXTENDED_TRANSFER_FUNCTION_H_INCLUDE

#include <cstdint>
#include <vismodule/ColorMap>
#include <vismodule/OpacityMap>
#include <vismodule/TransferFunction>
//#include "../../../FunctionParser/function.h"
//#include "../../../FunctionParser/function_op.h"
//#include "../../../FunctionParser/function_parser.h"
//#include "Types.h"
#include <vismodule/Type>
#include <vismodule/FrequencyTable>

class ExtendedTransferFunction : public vismodule::TransferFunction
{
public:
    int32_t m_resolution;

    std::string m_equation_red;
    std::string m_equation_green;
    std::string m_equation_blue;
    std::string m_equation_opacity;

    float m_user_color_variable_min;
    float m_user_color_variable_max;
    float m_user_opacity_variable_min;
    float m_user_opacity_variable_max;
    float m_server_color_variable_min;
    float m_server_color_variable_max;
    float m_server_opacity_variable_min;
    float m_server_opacity_variable_max;

    vismodule::UInt64 m_color_histogram[DEFAULT_NBINS];
    vismodule::UInt64 m_opacity_histogram[DEFAULT_NBINS];

public:

    ExtendedTransferFunction();
    ExtendedTransferFunction( const TransferFunction& tf );
    ExtendedTransferFunction( const std::string& red_function_string,
                              const std::string& green_function_string,
                              const std::string& blue_function_string,
                              const std::string& opacity_function_string,
                              const size_t      resolution,
                              const float       min_value,
                              const float       max_value );
    ~ExtendedTransferFunction();

    const float userColorMinValue() const { return m_user_color_variable_min; }
    const float userColorMaxValue() const { return m_user_color_variable_max; }
    const float userOpacityMinValue() const { return m_user_opacity_variable_min; }
    const float userOpacityMaxValue() const { return m_user_opacity_variable_max; }
    const float serverColorMinValue() const { return m_server_color_variable_min; }
    const float serverColorMaxValue() const { return m_server_color_variable_max; }
    const float serverOpacityMinValue() const { return m_server_opacity_variable_min; }
    const float serverOpacityMaxValue() const { return m_server_opacity_variable_max; }

    const vismodule::UInt64* colorHistogram() const { return m_color_histogram; }
    const vismodule::UInt64* opacityHistogram() const { return m_opacity_histogram; }
};

class NamedTransferFunction : public ExtendedTransferFunction
{
public:
    enum Selection
    {
        SelectExtendTransferFunction = 0,
        SelectTransferFunction = 1,
    };

    enum class ServerRangeMode : std::uint8_t
    {
        Unknown    = 0,
        UserRange  = 1,
        ServerSide = 2,
    };

public:
    Selection m_selection;
    ServerRangeMode m_server_color_range_mode;
    ServerRangeMode m_server_opacity_range_mode;

    std::string m_name;
    std::string m_color_variable;
    std::string m_opacity_variable;

public:
    NamedTransferFunction() :
        m_selection( SelectExtendTransferFunction ),
        m_name( "" ), m_color_variable( "" ), m_opacity_variable( "" )
    {
    }

    NamedTransferFunction( const TransferFunction& tf ) :
        m_selection( SelectExtendTransferFunction ),
        m_name( "" ), m_color_variable( "" ), m_opacity_variable( "" ),
        ExtendedTransferFunction( tf )
    {
    }

//    NamedTransferFunction& operator =( const NamedTransferFunction& rhs );
};

#endif // VIS_MODULE__EXTENDED_TRANSFER_FUNCTION_H_INCLUDE
