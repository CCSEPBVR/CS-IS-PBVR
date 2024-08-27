#ifndef PARTICLE_WRITE_THREAD_H
#define PARTICLE_WRITE_THREAD_H

#include <iostream>
#include <thread>
#include <kvs/PointObject>
#include <kvs/PointExporter>
#include <kvs/KVSMLObjectPoint>

#include <kvs/Timer>

namespace pbvr
{

class  ParticleWriteThread
{
private:

    kvs::PointObject* m_point_object;
//    char* m_filename;    // Underlying pointer changes in main thread.
    std::string m_filename;
    std::string m_state_filename;
    size_t m_timestep;

    std::thread* m_thread;
    bool  m_is_allocated = false;
    std::string  m_wname = "none";

public:

    ParticleWriteThread ()
        {
            m_point_object = new kvs::PointObject();
        }

    ~ParticleWriteThread ()
        {
            delete m_point_object;
        }

public:

    void setPointObject( kvs::PointObject* point_object )
        {
            m_point_object->deepCopy( *point_object );
        }

    void setFilename(  const char* filename )
        {
            m_filename = filename;
        }

    void setTimestep( int step,const char* filename)
    {
        m_timestep=step;
        m_state_filename=filename;
    }

    void writeParticleFile( )
        {
            kvs::Timer timer;
            timer.start();

            // Output kvsml data
            kvs::KVSMLObjectPoint* kvsml_object = new kvs::PointExporter<kvs::KVSMLObjectPoint>( m_point_object );
            kvsml_object->setWritingDataType( kvs::KVSMLObjectPoint::ExternalBinary );
            kvsml_object->write( m_filename );
            delete kvsml_object;

            // Output state.txt
            std::ofstream ofs( m_state_filename, std::ios::out);
            if( !ofs.is_open() ) std::cout<<"Cannot open state.txt"<<std::endl;
            ofs<<"START_STEP="<< 0 <<std::endl;
            ofs<<"LATEST_STEP="<<m_timestep<<std::endl;
            ofs.close();

            timer.stop();
            std::cout<<"thread time ="<<timer.msec()<<"[msec]"<<std::endl;
        }

    void check_working_func()
        {
            std::cout << "working func : " << m_wname << std::endl;
        }

    void work(const bool is_work )
        {
            if ( is_work ) {
                if ( m_is_allocated ) {
                    std::cout << __PRETTY_FUNCTION__ << " : "
                              << "error worker thread is already allocated!!" << std::endl;
                    exit(-1);
                }

                m_wname = __PRETTY_FUNCTION__;

                m_is_allocated = true;
                m_thread = new std::thread( &ParticleWriteThread::writeParticleFile, this );
            }
        }

    void join(const bool  is_join)
        {
            if ( is_join ) {
                if ( m_is_allocated ) {
                    m_thread->join();

                    delete m_thread;
                    m_is_allocated = false;
                    m_wname = "none";
                }
            }
        }

};

} // end of namespace pbvr

#endif
