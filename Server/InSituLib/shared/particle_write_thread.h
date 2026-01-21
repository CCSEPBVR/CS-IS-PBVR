#ifndef PARTICLE_WRITE_THREAD_H
#define PARTICLE_WRITE_THREAD_H

#include <iostream>
#include <fstream>
#include <thread>
#include <vismodule/PointObject>
#include <vismodule/PointExporter>
#include <vismodule/KVSMLObjectPoint>

#include <vismodule/Timer>

namespace pbvr
{

class  ParticleWriteThread
{
private:

    vismodule::PointObject* m_point_object;
//    char* m_filename;    // Underlying pointer changes in main thread.
    std::string m_filename;

    std::thread* m_thread;
    bool  m_is_allocated = false;
    std::string  m_wname = "none";

public:

    ParticleWriteThread ()
        {
            m_point_object = new vismodule::PointObject();
        }

    ~ParticleWriteThread ()
        {
            delete m_point_object;
        }

public:

    void setPointObject( vismodule::PointObject* point_object )
        {
            m_point_object->deepCopy( *point_object );
        }

    void setFilename(  const char* filename )
        {
            m_filename = filename;
        }

    void writeParticleFile()
        {
            vismodule::Timer timer;
            timer.start();

            // Output kvsml data
            //vismodule::KVSMLObjectPoint* kvsml_object = new vismodule::PointExporter<vismodule::KVSMLObjectPoint>( m_point_object );
            vismodule::KVSMLObjectPoint* kvsml_object = new vismodule::PointExporter<vismodule::KVSMLObjectPoint>( *m_point_object );
            kvsml_object->setWritingDataType( vismodule::KVSMLObjectPoint::ExternalBinary );
            kvsml_object->write( m_filename );
            delete kvsml_object;

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
