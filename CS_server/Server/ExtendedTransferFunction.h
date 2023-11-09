#ifndef PBVR__EXTENDED_TRANSFER_FUNCTION_H_INCLUDE
#define PBVR__EXTENDED_TRANSFER_FUNCTION_H_INCLUDE

#include <kvs/ColorMap>
#include <kvs/OpacityMap>
#include "TransferFunction.h"
#include "function.h"
#include "function_op.h"
#include "function_parser.h"
#include "Types.h"

class ExtendedTransferFunction : public pbvr::TransferFunction
{
public:
    int32_t m_resolution;
    std::string m_equation_red;
    std::string m_equation_green;
    std::string m_equation_blue;
    std::string m_equation_opacity;
    float m_color_variable_min;
    float m_color_variable_max;
    float m_opacity_variable_min;
    float m_opacity_variable_max;

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

};

class NamedTransferFunction : public ExtendedTransferFunction
{
public:
    enum Selection
    {
        SelectExtendTransferFunction = 0,
        SelectTransferFunction = 1,
    };

public:
    Selection m_selection;

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

#endif // PBVR__EXTENDED_TRANSFER_FUNCTION_H_INCLUDE
