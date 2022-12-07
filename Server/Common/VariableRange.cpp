
#include "VariableRange.h"
#include "Serializer.h"

VariableRange::VariableRange()
{
}

VariableRange::~VariableRange()
{
}

void VariableRange::setValue( const std::string& tfname, const float val )
{
    range_map_t::iterator imn = m_min.find( tfname );
    range_map_t::iterator imx = m_max.find( tfname );
    if ( imn == m_min.end() || val < imn->second )
    {
        m_min[tfname] = val;
    }
    if ( imx == m_max.end() || val > imx->second )
    {
        m_max[tfname] = val;
    }
}

float VariableRange::min( const std::string& tfname ) const
{
    float ret = 0.0;
    range_map_t::const_iterator imn = m_min.find( tfname );
    if ( imn != m_min.end() )
    {
        ret = imn->second;
    }
    return ret;
}

float VariableRange::max( const std::string& tfname ) const
{
    float ret = 1.0;
    range_map_t::const_iterator imx = m_max.find( tfname );
    if ( imx != m_max.end() )
    {
        ret = imx->second;
    }
    return ret;
}

void VariableRange::clear()
{
    m_min.clear();
    m_max.clear();
}

void VariableRange::merge( const VariableRange& vr )
{
    if ( m_max.empty() && m_min.empty() )
    {
        m_min = vr.m_min;
        m_max = vr.m_max;
    }
    else
    {
        for ( range_map_t::const_iterator imn = vr.m_min.begin(); imn != vr.m_min.end(); imn++ )
        {
            const std::string& nm = imn->first;
            range_map_t::const_iterator imx = vr.m_max.find( nm );

            setValue( nm, imn->second );
            setValue( nm, imx->second );
        }
    }
}

size_t VariableRange::byteSize() const
{
    size_t s = 0;

    s += jpv::Serializer::byteSize( m_min.size() );
    for ( range_map_t::const_iterator i = m_min.begin(); i != m_min.end(); i++ )
    {
        s += jpv::Serializer::byteSize( i->first );
        s += jpv::Serializer::byteSize( i->second );
    }
    s += jpv::Serializer::byteSize( m_max.size() );
    for ( range_map_t::const_iterator i = m_max.begin(); i != m_max.end(); i++ )
    {
        s += jpv::Serializer::byteSize( i->first );
        s += jpv::Serializer::byteSize( i->second );
    }

    return s;
}

size_t VariableRange::pack( char* buf ) const
{
    size_t index = 0;

    index += jpv::Serializer::write( buf + index, m_min.size() );
    for ( range_map_t::const_iterator i = m_min.begin(); i != m_min.end(); i++ )
    {
        index += jpv::Serializer::write( buf + index, i->first );
        index += jpv::Serializer::write( buf + index, i->second );
    }
    index += jpv::Serializer::write( buf + index, m_max.size() );
    for ( range_map_t::const_iterator i = m_max.begin(); i != m_max.end(); i++ )
    {
        index += jpv::Serializer::write( buf + index, i->first );
        index += jpv::Serializer::write( buf + index, i->second );
    }

    return index;
}

size_t VariableRange::unpack( const char* buf )
{
    std::string nm;
    float val;
    size_t s;
    size_t index = 0;

    index += jpv::Serializer::read( buf + index, &s );
    for ( size_t i = 0; i != s; i++ )
    {
        index += jpv::Serializer::read( buf + index, &nm );
        index += jpv::Serializer::read( buf + index, &val );
        m_min[nm] = val;
    }
    index += jpv::Serializer::read( buf + index, &s );
    for ( size_t i = 0; i != s; i++ )
    {
        index += jpv::Serializer::read( buf + index, &nm );
        index += jpv::Serializer::read( buf + index, &val );
        m_max[nm] = val;
    }

    return index;
}

