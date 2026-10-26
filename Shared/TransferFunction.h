#ifndef TRANSFERFUNCTION_H
#define TRANSFERFUNCTION_H

#include <iostream>
#include <vector>

#include <kvs/RGBColor>

class TransferFunction
{
public:
    enum RangeMode
    {
        Unknown     = 0,
        UserRange   = 1,
        ServerSide  = 2,
    };

    struct ColorTransferFunction
    {
        std::string variable;
        RangeMode rangeMode;
        std::pair<double,double> userDefinedMinMax;
        std::pair<double,double> serverSideMinMax;
        std::vector<kvs::RGBColor> map;
        std::vector<int> histogram;
    };

    struct OpacityTransferFunction
    {
        std::string variable;
        RangeMode rangeMode;
        std::pair<double,double> userDefinedMinMax;
        std::pair<double,double> serverSideMinMax;
        std::vector<float> map;
        std::vector<int> histogram;
    };

    struct Item
    {
        ColorTransferFunction color;
        OpacityTransferFunction opacity;
    };

    void setColorSynthesizer( const std::string& synthesizer ) { m_color_synthesizer = synthesizer; }
    const std::string& colorSynthesizer() const { return m_color_synthesizer; }

    void setOpacitySynthesizer( const std::string& synthesizer ) { m_opacity_synthesizer = synthesizer; }
    const std::string& opacitySynthesizer() const { return m_opacity_synthesizer; }

    Item& at( std::size_t index )
    {
        return m_transfer_functions.at( index ); // 範囲外なら std::out_of_range
    }

    const Item& at( std::size_t index ) const
    {
        return m_transfer_functions.at( index ); // 範囲外アクセスは std::out_of_range を投げる
    }

    std::size_t count() const { return m_transfer_functions.size(); }
    void addTransferFunction( const Item& item )
    {
        m_transfer_functions.push_back( item );
    }

    void removeTransferFunction( std::size_t index )
    {
        if( index < m_transfer_functions.size() )
            m_transfer_functions.erase( m_transfer_functions.begin() + index );
    }

private:
    std::string m_color_synthesizer;
    std::string m_opacity_synthesizer;
    std::vector<Item> m_transfer_functions;
};

#endif // TRANSFERFUNCTION_H
