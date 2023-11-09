/*****************************************************************************/
/**
 *  @file   ApplicationInterface.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ApplicationInterface.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__SAGE__APPLICATION_INTERFACE_H_INCLUDE
#define KVS__SAGE__APPLICATION_TNTERFACE_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/OpenGL>
#include <kvs/sage/SAGE>
#include <string>


namespace kvs
{

namespace sage
{

/*===========================================================================*/
/**
 *  @brief  SAGE application interface class.
 */
/*===========================================================================*/
class ApplicationInterface : public sail
{
    kvsClassName_without_virtual( kvs::sage::ApplicationInterface );

private:

    typedef sail SuperClass;

private:

    int    m_argc;                    ///< argument count
    char** m_argv;                    ///< argument values
    std::string m_configuration_file; ///< configuration filename
    bool   m_initialized;             ///< check flag whether the SAIL is initialized or not
    int    m_application_id;          ///< application ID
    int    m_application_width;       ///< application window width
    int    m_application_height;      ///< application window height

public:

    ApplicationInterface( int argc, char** argv );

    ~ApplicationInterface( void );

public:

    bool initialize( void );

public:

    const bool isInitialized( void ) const;

    const int applicationID( void ) const;

    const int applicationWidth( void ) const;

    const int applicationHeight( void ) const;

public:

    void setConfigurationFile( const std::string filename );

    void setApplicationID( const int id );

    void setApplicationWidth( const int width );

    void setApplicationHeight( const int height );
};

} // end of namespace sage

} // end of namespace kvs

#endif // KVS__SAGE__APPLICATION_INTERFACE_H_INCLUDE
