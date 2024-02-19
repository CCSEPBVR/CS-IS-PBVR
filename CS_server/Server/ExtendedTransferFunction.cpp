#include "ExtendedTransferFunction.h"

ExtendedTransferFunction::ExtendedTransferFunction()
{
}

ExtendedTransferFunction::~ExtendedTransferFunction()
{
}

ExtendedTransferFunction::ExtendedTransferFunction( const TransferFunction& tf ):
    TransferFunction( tf )
{
}

ExtendedTransferFunction::ExtendedTransferFunction( const std::string& red_function_string,
                                                    const std::string& green_function_string,
                                                    const std::string& blue_function_string,
                                                    const std::string& opacity_function_string,
                                                    const size_t      resolution,
                                                    const float       min_value,
                                                    const float       max_value ):
    TransferFunction( resolution )
{
    std::string rfstr = red_function_string;
    std::string gfstr = green_function_string;
    std::string bfstr = blue_function_string;
    std::string ofstr = opacity_function_string;
    kvs::ColorMap*   c = new kvs::ColorMap( resolution, min_value, max_value );
    kvs::OpacityMap* a = new kvs::OpacityMap( resolution, min_value, max_value );

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
    for ( size_t i = 0; i < resolution; ++i, x += stride )
    {
        kvs::UInt8 r, g, b;
        float opacity;

        var_x = x;
        r = rf.eval();
        g = gf.eval();
        b = bf.eval();
        opacity = of.eval();

        kvs::RGBColor color( r, g, b );
        c->addPoint( x, color );
        a->addPoint( x, opacity );
    }
    c->create();
    a->create();

    this->setColorMap( *c );
    this->setOpacityMap( *a );
    this->setRange( min_value, max_value );

    delete c, a;
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


