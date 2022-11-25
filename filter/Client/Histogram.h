#ifndef PBVR__KVS__VISCLIENT__HISTOGRAM_H_INCLUDE
#define PBVR__KVS__VISCLIENT__HISTOGRAM_H_INCLUDE

#include <kvs/glut/Histogram>

namespace kvs
{
namespace visclient
{

class Histogram : public kvs::glut::Histogram
{
    kvsClassName( kvs::visclient::Histogram );

public:

    typedef kvs::glut::Histogram BaseClass;

public:
    Histogram( kvs::ScreenBase* screen ):
        kvs::glut::Histogram( screen )
    {
    }

    void setTable( const kvs::FrequencyTable& table )
    {
        m_table = table;
    }

    void Redraw()
    {
        if ( !BaseClass::isShown() ) return;

        this->update_texture();
        BaseClass::screen()->redraw();
    }
};

}
}

#endif  // PBVR__KVS__VISCLIENT__HISTOGRAM_H_INCLUDE

