#ifndef PTS_H
#define PTS_H

#include <iostream>
#include <fstream>
#include <string>

#include <kvs/PointObject>
#include <kvs/Vector3>
class PTSImporter : public kvs::PointObject
{
public:
    PTSImporter( std::string filename );
    std::vector<std::string> split(const std::string& input, char delimiter);

private:
    kvs::UInt32 m_npoints;
    kvs::UInt32 m_ncomponents;
};

#endif // PTS_H
