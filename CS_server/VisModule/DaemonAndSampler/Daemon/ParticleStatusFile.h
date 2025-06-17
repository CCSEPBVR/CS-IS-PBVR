#ifndef __JUPITER_PARTICLE_STATUS_FILE_H__
#define __JUPITER_PARTICLE_STATUS_FILE_H__

#include <iostream>
#include <string>
#include <vector>
#include <vismodule/Type>
#include <vismodule/NameListFile>

class ParticleStatusFile
{
public:
    enum Status
    {
        NO_STATUS_FILE,
        NO_STEP,
        STEP_CHANGED,
        SAME_STEP
    };

private:
    typedef std::vector<std::string> NameArray;
    Status            m_status;
    std::string       m_file_name;
    vismodule::UInt32       m_start_time_step;
    vismodule::UInt32       m_latest_time_step;
    NameArray         m_name;

private:
    template<typename T>
    inline std::template vector<T> split( const std::string& line, const char delim  );

    template<typename T>
    inline std::template vector<T> split_csv( const std::string& line );

    void set_name();
    void assign_name_list( const NameListFile& name_list_file );
    void read_name_list_file();

public:
    ParticleStatusFile();
    ParticleStatusFile( const std::string& file );
    void setFileName( const std::string& file );
    void read();
    vismodule::UInt32 getStartTimeStep();
    vismodule::UInt32 getLatestTimeStep();
    Status getStatus();
};

#endif
