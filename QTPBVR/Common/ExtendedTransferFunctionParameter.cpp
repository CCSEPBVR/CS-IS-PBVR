#include "ExtendedTransferFunctionParameter.h"

bool ExtendedTransferFunctionParameter::operator==( const ExtendedTransferFunctionParameter& s ) const
{
    bool v0 = true;

    v0 &= ( this->m_resolution == s.m_resolution );
    v0 &= ( this->m_color_variable_min == s.m_color_variable_min );
    v0 &= ( this->m_color_variable_max == s.m_color_variable_max );
    v0 &= ( this->m_opacity_variable_min == s.m_opacity_variable_min );
    v0 &= ( this->m_opacity_variable_max == s.m_opacity_variable_max );

    const size_t n_color   = this->colorMap().table().size();
    const size_t n_opacity = this->opacityMap().table().size();

    v0 &= ( n_color == s.colorMap().table().size() );
    v0 &= ( n_opacity == s.opacityMap().table().size() );

    if ( v0 )
    {
        for ( size_t n = 0; n < n_color; n++ )
        {
            const kvs::UInt8 r0 = this->colorMap().table().at( n );
            const kvs::UInt8 r1 = s.colorMap().table().at( n );
            v0 &= ( r0 == r1 );
        }
        for ( size_t n = 0; n < n_opacity; n++ )
        {
            const float r0 = this->opacityMap().table().at( n );
            const float r1 = s.opacityMap().table().at( n );
            v0 &= ( r0 == r1 );
        }
    }

    return v0;
}

bool NamedTransferFunctionParameter::operator==( const NamedTransferFunctionParameter& s ) const
{
    bool v0 = true;

    v0 &= ( this->m_name == s.m_name );
    v0 &= ( this->m_color_variable == s.m_color_variable );
    v0 &= ( this->m_opacity_variable == s.m_opacity_variable );

    if ( v0 )
    {
        v0 &= ExtendedTransferFunctionParameter::operator==( s );
    }

    return v0;
}

/**
 * １次伝達関数名(C1,C2,C3...)を比較する。
 * @param left		比較１
 * @param right		比較２
 * @return			比較結果：比較１-比較２
 */
bool NamedTransferFunctionParameter::compareName(const NamedTransferFunctionParameter& left, const NamedTransferFunctionParameter& right)
{
    std::string left_name = left.m_name;
    std::string right_name = right.m_name;
    if (left_name.length() <= 1 || right_name.length() <= 1) {
        return left_name.compare(right_name);
    }

    std::string left_prefix = left_name.substr(0, 1);
    std::string right_prefix = right_name.substr(0, 1);
    int left_num = std::atoi(left_name.substr(1).c_str());
    int right_num = std::atoi(right_name.substr(1).c_str());

    int cmp = left_prefix.compare(right_prefix);
    if (cmp != 0) return true;
    return (left_num < right_num);
}

/**
 * １次伝達関数名(C1,C2,C3...)名からCNNの番号（NN）を取得する。
 * @param trans		１次伝達関数
 * @return		CNN, ONNの番号（NN）
 */
int NamedTransferFunctionParameter::getNameNumber() const
{
    if (this->m_name.length() <= 1) {
        return 0;
    }

    std::string prefix = this->m_name.substr(0, 1);
    int name_num = std::atoi(this->m_name.substr(1).c_str());

    return name_num;
}

