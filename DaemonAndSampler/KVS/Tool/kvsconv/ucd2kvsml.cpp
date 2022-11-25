/*****************************************************************************/
/**
 *  @file   ucd2kvsml.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ucd2kvsml.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "ucd2kvsml.h"
#include <memory>
#include <string>
#include <kvs/File>
#include <kvs/AVSUcd>
#include <kvs/KVSMLObjectUnstructuredVolume>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/UnstructuredVolumeImporter>
#include <kvs/UnstructuredVolumeExporter>


namespace kvsconv
{

namespace ucd2kvsml
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new Argument class for a ucd2kvsml.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
Argument::Argument( int argc, char** argv ):
    kvsconv::Argument::Common( argc, argv, kvsconv::ucd2kvsml::CommandName )
{
    add_option( kvsconv::ucd2kvsml::CommandName, kvsconv::ucd2kvsml::Description, 0 );
    add_option( "e", "External data file. (optional)", 0, false );
    add_option( "b", "Data file as binary. (optional)", 0, false );
}

/*===========================================================================*/
/**
 *  @brief  Returns a input filename.
 *  @return input filename
 */
/*===========================================================================*/
const std::string Argument::inputFilename( void )
{
    return( this->value<std::string>() );
}

/*===========================================================================*/
/**
 *  @brief  Returns a output filename.
 *  @param  filename [in] input filename
 *  @return output filename.
 */
/*===========================================================================*/
const std::string Argument::outputFilename( const std::string& filename )
{
    if ( this->hasOption("output") )
    {
        return( this->optionValue<std::string>("output") );
    }
    else
    {
        // Replace the extension as follows: xxxx.inp -> xxx.kvsml.
        const std::string basename = kvs::File( filename ).baseName();
        const std::string extension = "kvsml";
        return( basename + "." + extension );
    }
}

/*===========================================================================*/
/**
 *  @brief  Returns a writing data type (ascii/external ascii/external binary).
 *  @return writing data type
 */
/*===========================================================================*/
const kvs::KVSMLObjectUnstructuredVolume::WritingDataType Argument::writingDataType( void )
{
    if ( this->hasOption("b") )
    {
        return( kvs::KVSMLObjectUnstructuredVolume::ExternalBinary );
    }
    else
    {
        if ( this->hasOption("e") )
        {
            return( kvs::KVSMLObjectUnstructuredVolume::ExternalAscii );
        }
    }

    return( kvs::KVSMLObjectUnstructuredVolume::Ascii );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new Main class for a ucd2kvsml.
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
Main::Main( int argc, char** argv )
{
    m_argc = argc;
    m_argv = argv;
}

/*===========================================================================*/
/**
 *  @brief  Executes main process.
 */
/*===========================================================================*/
const bool Main::exec( void )
{
    // Parse specified arguments.
    ucd2kvsml::Argument arg( m_argc, m_argv );
    if( !arg.parse() ) return( false );

    // Set a input filename and a output filename.
    m_input_name = arg.inputFilename();
    m_output_name = arg.outputFilename( m_input_name );

    kvs::File file( m_input_name );
    if ( !file.isExisted() )
    {
        kvsMessageError("Input data file '%s' is not existed.",m_input_name.c_str());
        return( false );
    }

    // Read AVS UCD data file.
    kvs::AVSUcd* input = new kvs::AVSUcd( m_input_name );
    if ( !input )
    {
        kvsMessageError("Cannot allocate for the AVS UCD class.");
        return( false );
    }

    if ( input->isFailure() )
    {
        kvsMessageError("Cannot read a file %s.", m_input_name.c_str() );
        delete input;
        return( false );
    }

    // Import AVS UCD data as unstructured volume object.
    kvs::UnstructuredVolumeObject* object = new kvs::UnstructuredVolumeImporter( input );
    if ( !object )
    {
        kvsMessageError("Cannot import AVS UCD data.");
        delete input;
        return( false );
    }

    delete input;

    // Export the unstructured volume object to KVSML data (unstructured volume).
    kvs::KVSMLObjectUnstructuredVolume* output =
        new kvs::UnstructuredVolumeExporter<kvs::KVSMLObjectUnstructuredVolume>( object );
    if ( !output )
    {
        kvsMessageError("Cannot export unstructured volume object.");
        delete object;
        return( false );
    }

    delete object;

    // Set the writing data type.
    output->setWritingDataType( arg.writingDataType() );

    // Write to KVSML data file.
    if ( !output->write( m_output_name ) )
    {
        kvsMessageError("Cannot write to KVSML data file %s.", m_output_name.c_str() );
        delete output;
        return( false );
    }

    delete output;

    return( true );
}

} // end of namespace ucd2kvsml

} // end of namespace kvsconv
