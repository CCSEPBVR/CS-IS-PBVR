#ifndef __JUPITER_PARTICLE_FILE_H__
#define __JUPITER_PARTICLE_FILE_H__

#include <string>
#include <sstream>
#include <algorithm>
#include <kvs/Directory>
#include <kvs/File>
#include <kvs/Type>
#include "PointObject.h"
#include "PointImporter.h"


class ParticleFile
{
private:
    kvs::UInt32 m_subvolume_number ;
    kvs::UInt32 m_initial_step;
    kvs::UInt32 m_final_step;
    kvs::UInt32 m_kvsml_file_number;
    std::string m_file_prefix;

public:
    void setFilePrefix( const std::string& prefix );
    void setParameterFromFile();
    void generatePointObject( const int time_step, pbvr::PointObject* object );
};

#endif
