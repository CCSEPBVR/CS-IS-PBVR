#include "ExtendedTransferFunction.h"

#include <algorithm>

ExtendedTransferFunction::ExtendedTransferFunction()
{
//    m_resolution = DEFAULT_NBINS;
//    m_equation_red.clear();
//    m_equation_green.clear();
//    m_equation_blue.clear();
//    m_equation_opacity.clear();
//    m_user_color_variable_min = 0.0f;
//    m_user_color_variable_max = 0.0f;
//    m_user_opacity_variable_min = 0.0f;
//    m_user_opacity_variable_max = 0.0f;
//    m_server_color_variable_min = 0.0f;
//    m_server_color_variable_max = 0.0f;
//    m_server_opacity_variable_min = 0.0f;
//    m_server_opacity_variable_max = 0.0f;
//    std::fill_n( m_color_histogram, DEFAULT_NBINS, 0 );
//    std::fill_n( m_opacity_histogram, DEFAULT_NBINS, 0 );
//    m_has_color_histogram = false;
//    m_has_opacity_histogram = false;
}

ExtendedTransferFunction::~ExtendedTransferFunction()
{
}

ExtendedTransferFunction::ExtendedTransferFunction( const TransferFunction& tf ):
    TransferFunction( tf )
{
//    m_resolution = DEFAULT_NBINS;
//    m_equation_red.clear();
//    m_equation_green.clear();
//    m_equation_blue.clear();
//    m_equation_opacity.clear();
//    m_user_color_variable_min = 0.0f;
//    m_user_color_variable_max = 0.0f;
//    m_user_opacity_variable_min = 0.0f;
//    m_user_opacity_variable_max = 0.0f;
//    m_server_color_variable_min = 0.0f;
//    m_server_color_variable_max = 0.0f;
//    m_server_opacity_variable_min = 0.0f;
//    m_server_opacity_variable_max = 0.0f;
//    std::fill_n( m_color_histogram, DEFAULT_NBINS, 0 );
//    std::fill_n( m_opacity_histogram, DEFAULT_NBINS, 0 );
//    m_has_color_histogram = false;
//    m_has_opacity_histogram = false;
}

ExtendedTransferFunction::ExtendedTransferFunction( const std::string& red_function_string,
                                                    const std::string& green_function_string,
                                                    const std::string& blue_function_string,
                                                    const std::string& opacity_function_string,
                                                    const std::size_t      resolution,
                                                    const float       min_value,
                                                    const float       max_value ):
    TransferFunction( resolution )
{
    m_resolution = static_cast<int32_t>( resolution );
    m_equation_red = red_function_string;
    m_equation_green = green_function_string;
    m_equation_blue = blue_function_string;
    m_equation_opacity = opacity_function_string;
    m_user_color_variable_min = min_value;
    m_user_color_variable_max = max_value;
    m_user_opacity_variable_min = min_value;
    m_user_opacity_variable_max = max_value;
    m_server_color_variable_min = min_value;
    m_server_color_variable_max = max_value;
    m_server_opacity_variable_min = min_value;
    m_server_opacity_variable_max = max_value;
    std::fill_n( m_color_histogram, DEFAULT_NBINS, 0 );
    std::fill_n( m_opacity_histogram, DEFAULT_NBINS, 0 );
    m_has_color_histogram = false;
    m_has_opacity_histogram = false;
/*
    std::string rfstr = red_function_string;
    std::string gfstr = green_function_string;
    std::string bfstr = blue_function_string;
    std::string ofstr = opacity_function_string;
    vismodule::ColorMap*   c = new vismodule::ColorMap( resolution, min_value, max_value );
    vismodule::OpacityMap* a = new vismodule::OpacityMap( resolution, min_value, max_value );

    FuncParser::Variables vars;
    FuncParser::Variable var_x;
    FuncParser::Function rf, gf, bf, of;

    var_x.tag( "x" );
    vars.push_back( var_x );

    FuncParser::FunctionParser rf_parse( rfstr, rfstr.size() + 1 );
    FuncParser::FunctionParser gf_parse( gfstr, gfstr.size() + 1 );
    FuncParser::FunctionParser bf_parse( bfstr, bfstr.size() + 1 );
    FuncParser::FunctionParser of_parse( ofstr, ofstr.size() + 1 );
    FuncParser::FunctionParser::Error err_r = rf_parse.express( rf, vars );
    FuncParser::FunctionParser::Error err_g = gf_parse.express( gf, vars );
    FuncParser::FunctionParser::Error err_b = bf_parse.express( bf, vars );
    FuncParser::FunctionParser::Error err_o = of_parse.express( of, vars );

    const float stride = ( max_value - min_value ) / ( resolution - 1 );
    float x = min_value;
    for ( std::size_t i = 0; i < resolution; ++i, x += stride )
    {
        vismodule::UInt8 r, g, b;
        float opacity;

        var_x = x;
        r = rf.eval();
        g = gf.eval();
        b = bf.eval();
        opacity = of.eval();

        vismodule::RGBColor color( r, g, b );
        c->addPoint( x, color );
        a->addPoint( x, opacity );
    }
    c->create();
    a->create();

    this->setColorMap( *c );
    this->setOpacityMap( *a );
    this->setRange( min_value, max_value );

    delete c, a;
*/
}

/*==========================================================================*/
/**
 *  @brief  Substitution operator =.
 *  @param  trunsfer_function [in] transfer function
 *  @retval transfer function
 */
/*==========================================================================*/
//NamedTransferFunction& NamedTransferFunction::operator =( const NamedTransferFunction& rhs )
//{
//    m_color_map   = rhs.m_color_map;
//    m_opacity_map = rhs.m_opacity_map;
//
//    return *this;
//}
