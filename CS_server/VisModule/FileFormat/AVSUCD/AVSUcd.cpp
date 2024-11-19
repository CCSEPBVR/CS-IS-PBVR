/****************************************************************************/
/**
 *  @file AVSUcd.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: AVSUcd.cpp 656 2010-12-13 16:04:04Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "AVSUcd.h"
#include <kvs/File>
#include <kvs/Message>
#include <kvs/ValueArray>
#include <kvs/IgnoreUnusedVariable>
#include <cstdlib>
#include <cstring>


namespace
{

const size_t MaxLineLength = 512;

const char* const Delimiter = " \n\r";

const std::string CycleTypeToString[ kvs::AVSUcd::CycleTypeSize ] =
{
    "unknown",
    "data",
    "geometry",
    "data_geometry"
};

const std::string ElementTypeToString[ kvs::AVSUcd::ElementTypeSize ] =
{
    "unknown",
    "point",
    "line",
    "triangle",
    "quadrangle",
    "tetrahedra",
    "tetrahedra2",
    "pyramid",
    "prism",
    "hexahedra",
    "hexahedra2"
};

}

namespace
{

const kvs::AVSUcd::FormatType CheckFormatType( FILE* const ifs )
{
    char buffer[ ::MaxLineLength ];

    kvs::AVSUcd::FormatType format_type = kvs::AVSUcd::SingleStep;

    while ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
    {
        if ( buffer[0] == '#' )
        {
            // Skip comment line.
            continue;
        }
        else
        {
            // Skip first token.
            strtok( buffer, ::Delimiter );

            const char* const second_token = strtok( 0, ::Delimiter );

            if ( second_token != 0 )
            {
                format_type = kvs::AVSUcd::SingleStep;
            }
            else
            {
                format_type = kvs::AVSUcd::MultiStep;
            }

            break;
        }
    }

    // Go back file-pointer to head.
    fseek( ifs, 0, SEEK_SET );

    return( format_type );
}

}


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructs a new empty AVSUcd.
 */
/*==========================================================================*/
AVSUcd::AVSUcd( void )
    : FileFormatBase()
    , m_nsteps( 0 )
    , m_cycle_type( kvs::AVSUcd::CycleTypeUnknown )
    , m_element_type( kvs::AVSUcd::ElementTypeUnknown )
    , m_step_id( 0 )
    , m_step_comment( "" )
    , m_nnodes( 0 )
    , m_nelements( 0 )
    , m_nvalues_per_node( 0 )
    , m_ncomponents_per_node( 0 )
    , m_component_id( 0 )
{
}

/*==========================================================================*/
/**
 *  Constructs a new AVSUcd and reads the component of the step in the file.
 *
 *  @param filename     [in] Filename.
 *  @param step_id      [in] StepID.
 *  @param component_id [in] ComponentID.
 */
/*==========================================================================*/
AVSUcd::AVSUcd(
    const std::string& filename,
    const size_t       step_id,
    const size_t       component_id )
    : FileFormatBase()
    , m_nsteps( 0 )
    , m_cycle_type( kvs::AVSUcd::CycleTypeUnknown )
    , m_element_type( kvs::AVSUcd::ElementTypeUnknown )
    , m_step_id( step_id )
    , m_step_comment( "" )
    , m_nnodes( 0 )
    , m_nelements( 0 )
    , m_nvalues_per_node( 0 )
    , m_ncomponents_per_node( 0 )
    , m_component_id( component_id )
{
    if( this->read( filename ) ) { m_is_success = true; }
    else { m_is_success = false; }
}

/*==========================================================================*/
/**
 *  Destroys the AVSUcd.
 */
/*==========================================================================*/
AVSUcd::~AVSUcd( void )
{
}

const size_t AVSUcd::nsteps( void ) const
{
    return( m_nsteps );
}

const AVSUcd::CycleType AVSUcd::cycleType( void ) const
{
    return( m_cycle_type );
}

const AVSUcd::ElementType AVSUcd::elementType( void ) const
{
    return( m_element_type );
}

const size_t AVSUcd::stepID( void ) const
{
    return( m_step_id );
}

const std::string& AVSUcd::stepComment( void ) const
{
    return( m_step_comment );
}

const size_t AVSUcd::nnodes( void ) const
{
    return( m_nnodes );
}

const size_t AVSUcd::nelements( void ) const
{
    return( m_nelements );
}

const size_t AVSUcd::nvaluesPerNode( void ) const
{
    return( m_nvalues_per_node );
}

const size_t AVSUcd::ncomponentsPerNode( void ) const
{
    return( m_ncomponents_per_node );
}

const std::vector<size_t>& AVSUcd::veclens( void ) const
{
    return( m_veclens );
}

const std::vector<std::string>& AVSUcd::componentNames( void ) const
{
    return( m_component_names );
}

const std::vector<std::string>& AVSUcd::componentUnits( void ) const
{
    return( m_component_units );
}

const size_t AVSUcd::componentID( void ) const
{
    return( m_component_id );
}

const std::string& AVSUcd::componentName( void ) const
{
    return( m_component_names[ m_component_id ] );
}

const std::string& AVSUcd::componentUnit( void ) const
{
    return( m_component_units[ m_component_id ] );
}

const AVSUcd::Coords& AVSUcd::coords( void ) const
{
    return( m_coords );
}

const AVSUcd::Connections& AVSUcd::connections( void ) const
{
    return( m_connections );
}

const AVSUcd::Values& AVSUcd::values( void ) const
{
    return( m_values );
}

void AVSUcd::setNSteps( const size_t nsteps )
{
    m_nsteps = nsteps;
}

void AVSUcd::setCycleType( const CycleType cycle_type )
{
    m_cycle_type = cycle_type;
}

void AVSUcd::setElementType( const ElementType element_type )
{
    m_element_type = element_type;
}

void AVSUcd::setStepID( const size_t step_id )
{
    m_step_id = step_id;
}

void AVSUcd::setStepComment( const std::string& step_comment )
{
    m_step_comment = step_comment;
}

void AVSUcd::setNNodes( const size_t nnodes )
{
    m_nnodes = nnodes;
}

void AVSUcd::setNElements( const size_t nelements )
{
    m_nelements = nelements;
}

void AVSUcd::setNValuesPerNode( const size_t nvalues_per_node )
{
    m_nvalues_per_node = nvalues_per_node;
}

void AVSUcd::setNComponentsPerNode( const size_t ncomponents_per_node )
{
    m_ncomponents_per_node = ncomponents_per_node;
}

void AVSUcd::setVeclens( const std::vector<size_t>& veclens )
{
    m_veclens = veclens;
}

void AVSUcd::setComponentNames( const std::vector<std::string>& component_names )
{
    m_component_names.clear();

    const size_t size = component_names.size();
    for ( size_t i = 0; i < size; ++i )
    {
        m_component_names.push_back( component_names[ i ] );
    }
}

void AVSUcd::setComponentUnits( const std::vector<std::string>& component_units )
{
    m_component_units.clear();

    const size_t size = component_units.size();
    for ( size_t i = 0; i < size; ++i )
    {
        m_component_units.push_back( component_units[ i ] );
    }
}

void AVSUcd::setComponentID( const size_t component_id )
{
    m_component_id = component_id;
}

void AVSUcd::setCoords( const Coords& coords )
{
    m_coords = coords;
}

void AVSUcd::setConnections( const Connections& connections )
{
    m_connections = connections;
}

void AVSUcd::setValues( const Values& values )
{
    m_values = values;
}

/*==========================================================================*/
/**
 *  Reads AVS UCD data from the file.
 *
 *  @param filename [in] Filename.
 */
/*==========================================================================*/
const bool AVSUcd::read( const std::string& filename )
{
    m_filename = filename;

    FILE* const ifs = fopen( filename.c_str(), "rb" );

    if ( !ifs )
    {
        kvsMessageError( "Cannot open %s.", filename.c_str() );
        m_is_success = false;
        return( m_is_success );
    }

    try
    {
        const FormatType format_type = ::CheckFormatType( ifs );
        if ( format_type == SingleStep )
        {
            this->read_single_step_format( ifs );
        }
        else if ( format_type == MultiStep )
        {
            this->read_multi_step_format( ifs );
        }

        m_is_success = true;
    }
    catch ( const char* const error )
    {
        kvsMessageError( "%s: %s", filename.c_str(), error );

        m_is_success = false;
    }

    fclose( ifs );

    return( m_is_success );
}

/*==========================================================================*/
/**
 *  Writes AVS UCD data to the file.
 *
 *  @param filename [in] Filename.
 */
/*==========================================================================*/
const bool AVSUcd::write( const std::string& filename )
{
    m_filename = filename;

    FILE* const ofs = fopen( filename.c_str(), "wb" );

    if ( !ofs )
    {
        kvsMessageError( "Cannot open %s.", filename.c_str() );
        m_is_success = false;
        return( m_is_success );
    }

    try
    {
        this->write_single_step_format( ofs );

        m_is_success = true;
    }
    catch ( const char* const error )
    {
        kvsMessageError( "%s: %s", filename.c_str(), error );

        m_is_success = false;
    }

    fclose( ofs );

    return( m_is_success );
}

std::ostream& operator <<( std::ostream& os, const AVSUcd& rhs )
{
    os << "Filename                      : " << rhs.m_filename                            << std::endl;
    os << "Number of steps               : " << rhs.m_nsteps                              << std::endl;
    os << "Cycle type                    : " << ::CycleTypeToString[rhs.m_cycle_type]     << std::endl;
    os << "Element type                  : " << ::ElementTypeToString[rhs.m_element_type] << std::endl;

    os << "Step ID                       : " << rhs.m_step_id      << std::endl;
    os << "Step comment                  : " << rhs.m_step_comment << std::endl;

    os << "Number of nodes               : " << rhs.m_nnodes               << std::endl;
    os << "Number of elements            : " << rhs.m_nelements            << std::endl;
    os << "Number of values per node     : " << rhs.m_nvalues_per_node     << std::endl;
    os << "Number of components per node : " << rhs.m_ncomponents_per_node << std::endl;

    os << "Veclens of each component     : ";
    for ( size_t i = 0; i < rhs.m_ncomponents_per_node; ++i )
    {
        os << rhs.m_veclens[ i ] << " ";
    }
    os << std::endl;

    os << "Names of each component       : ";
    for ( size_t i = 0; i < rhs.m_ncomponents_per_node; ++i )
    {
        os << rhs.m_component_names[ i ] << " ";
    }
    os << std::endl;

    os << "Units of each component       : ";
    for ( size_t i = 0; i < rhs.m_ncomponents_per_node; ++i )
    {
        os << rhs.m_component_units[ i ] << " ";
    }
    os << std::endl;

    os << "Component ID                  : " << rhs.m_component_id << std::endl;

    return( os );
}

void AVSUcd::read_single_step_format( FILE* const ifs )
{
    char buffer[ ::MaxLineLength ];

    m_nsteps     = 1;
    m_cycle_type = DataGeom;

    m_step_id      = 0;
    m_step_comment = "";

    while ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
    {
        if ( buffer[0] == '#' )
        {
            // Skip header comment line.
            continue;
        }
        else
        {
            m_nnodes           = atoi( strtok( buffer, ::Delimiter ) );
            m_nelements        = atoi( strtok( 0,      ::Delimiter ) );
            m_nvalues_per_node = atoi( strtok( 0,      ::Delimiter ) );

            this->read_coords( ifs );
            this->read_connections( ifs );
            this->read_components( ifs );
            this->read_values( ifs );

            return;
        }
    }

    throw "Unexpected EOF in skipping header comments.";
}

void AVSUcd::read_multi_step_format( FILE* const ifs )
{
    char buffer[ ::MaxLineLength ];

    while ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
    {
        if ( buffer[0] == '#' )
        {
            // Skip comment line.
            continue;
        }
        else
        {
            m_nsteps = atoi( strtok( buffer, ::Delimiter ) );

            if ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
            {
                const char* const cycle_type = strtok( buffer, ::Delimiter );

                m_cycle_type =
                    !strcmp( cycle_type, "data" )      ? Data     :
                !strcmp( cycle_type, "geom" )      ? Geom     :
                !strcmp( cycle_type, "data_geom" ) ? DataGeom : CycleTypeUnknown;

                if ( m_nsteps == 1 )
                {
                    // In case of 'm_nsteps == 1', the data is read as 'data_geom'
                    // despite the specified cycle type.
                    this->read_multi_step_format_data_geom( ifs );
                }
                else
                {
                    if ( m_cycle_type == Data )
                    {
                        throw "Cycle type \"data\" is not supported.";
                        this->read_multi_step_format_data( ifs );
                    }
                    else if ( m_cycle_type == Geom )
                    {
                        throw "Cycle type \"geom\" is not supported.";
                        this->read_multi_step_format_geom( ifs );
                    }
                    else if ( m_cycle_type == DataGeom )
                    {
                        this->read_multi_step_format_data_geom( ifs );
                    }
                    else
                    {
                        throw "Unknown cycle type.";
                    }
                }
            }

            return;
        }
    }

    throw "Unexpected EOF in skipping header comments.";
}

void AVSUcd::read_multi_step_format_data( FILE* const ifs )
{
    kvs::IgnoreUnusedVariable( ifs );
}

void AVSUcd::read_multi_step_format_geom( FILE* const ifs )
{
    kvs::IgnoreUnusedVariable( ifs );
}

void AVSUcd::read_multi_step_format_data_geom( FILE* const ifs )
{
    char buffer[ ::MaxLineLength ];

    this->seek_target_step( ifs );

    if ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
    {
        m_nnodes    = atoi( strtok( buffer, ::Delimiter ) );
        m_nelements = atoi( strtok( 0,   ::Delimiter ) );
    }
    else
    {
        throw "Unexpected EOF in reading nnodes and nelements.";
    }

    this->read_coords( ifs );
    this->read_connections( ifs );

    if ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
    {
        m_nvalues_per_node = atoi( strtok( buffer, ::Delimiter ) );
    }
    else
    {
        throw "Unexpected EOF in reading nvalues per node.";
    }

    this->read_components( ifs );
    this->read_values( ifs );
}

void AVSUcd::seek_target_step( FILE* const ifs )
{
    char buffer[ ::MaxLineLength ];

    char target_step[ ::MaxLineLength ];
    sprintf( target_step, "step%u", static_cast<unsigned int>( m_step_id + 1 ) );

    while ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
    {
        const char* const first_token = strtok( buffer, ::Delimiter );
        if ( strcmp( first_token, target_step ) == 0 )
        {
            const char* const step_comment = strtok( 0, ::Delimiter );
            if ( step_comment != 0 )
            {
                m_step_comment = std::string( step_comment );
            }
            else
            {
                m_step_comment = std::string( "" );
            }

            return;
        }
    }

    throw "Unexpected EOF in seeking target step.";
}

void AVSUcd::read_coords( FILE* const ifs )
{
    char buffer[ ::MaxLineLength ];

    m_coords.allocate( 3 * m_nnodes );

    kvs::Real32* coord = m_coords.pointer();

    for ( size_t i = 0; i < m_nnodes; i++ )
    {
        if ( fgets( buffer, ::MaxLineLength, ifs ) )
        {
            // Node index.
            const int index = atoi( strtok( buffer, ::Delimiter ) ) - 1;

            coord[ index * 3 + 0 ] = static_cast<float>( atof( strtok( 0, ::Delimiter ) ) );
            coord[ index * 3 + 1 ] = static_cast<float>( atof( strtok( 0, ::Delimiter ) ) );
            coord[ index * 3 + 2 ] = static_cast<float>( atof( strtok( 0, ::Delimiter ) ) );
        }
    }
}

void AVSUcd::read_connections( FILE* const ifs )
{
    char buffer[ ::MaxLineLength ];

    // Read first line.
    if ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
    {
        strtok( buffer, ::Delimiter ); // Skip element index.
        strtok( 0, ::Delimiter ); // Skip material index.

        const char* const element_type = strtok( 0, ::Delimiter );

        m_element_type =
            !strcmp( element_type, "tet"  ) ? Tetrahedra  :
            !strcmp( element_type, "tet2" ) ? Tetrahedra2 :
            !strcmp( element_type, "hex2" ) ? Hexahedra2   :
            !strcmp( element_type, "hex" ) ? Hexahedra  : ElementTypeUnknown;

        if ( m_element_type == Tetrahedra )
        {
            m_connections.allocate( 4 * m_nelements );
        }
        else if ( m_element_type == Tetrahedra2 )
        {
            m_connections.allocate( 10 * m_nelements );
        }
        else if ( m_element_type == Hexahedra )
        {
            m_connections.allocate( 8 * m_nelements );
        }
        else if ( m_element_type == Hexahedra2 )
        {
            m_connections.allocate( 20 * m_nelements );
        }
        else
        {
            throw "Unknown element type.";
        }
    }
    else
    {
        throw "Unexpected EOF in reading first line of connections.";
    }

    kvs::UInt32*       connection = m_connections.pointer();
    kvs::UInt32* const end        = connection + m_connections.size();

    if ( m_element_type == Tetrahedra )
    {
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;

        while ( connection < end )
        {
            if ( fgets( buffer, ::MaxLineLength, ifs ) )
            {
                // skip element index.
                strtok( buffer, ::Delimiter );

                // skip material index.
                strtok( 0, ::Delimiter );

                const char* const element_type = strtok( 0, ::Delimiter );
                if ( strcmp( element_type, "tet" ) != 0 )
                {
                    throw "Multi-element type is not supported.";
                }

                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
            }
        }
    }
    else if ( m_element_type == Tetrahedra2 )
    {
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;

        while ( connection < end )
        {
            if ( fgets( buffer, ::MaxLineLength, ifs ) )
            {
                // skip element index.
                strtok( buffer, ::Delimiter );

                // skip material index.
                strtok( 0, ::Delimiter );

                const char* const element_type = strtok( 0, ::Delimiter );
                if ( strcmp( element_type, "tet2" ) != 0 )
                {
                    throw "Multi-element type is not supported.";
                }

                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
            }
        }
    }
    else if ( m_element_type == Hexahedra )
    {
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;

        while ( connection < end )
        {
            if ( fgets( buffer, ::MaxLineLength, ifs ) )
            {
                // Skip element index.
                strtok( buffer, ::Delimiter );

                // Skip material index.
                strtok( 0, ::Delimiter );

                const char* const element_type = strtok( 0, ::Delimiter );
                if ( strcmp( element_type, "hex" ) != 0 )
                {
                    throw "Multi-element type is not supported.";
                }

                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
            }
        }
    }
    else if ( m_element_type == Hexahedra2 )
    {
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
        *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;

        while ( connection < end )
        {
            if ( fgets( buffer, ::MaxLineLength, ifs ) )
            {
                // Skip element index.
                strtok( buffer, ::Delimiter );

                // Skip material index.
                strtok( 0, ::Delimiter );

                const char* const element_type = strtok( 0, ::Delimiter );
                if ( strcmp( element_type, "hex2" ) != 0 )
                {
                    throw "Multi-element type is not supported.";
                }

                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
                *( connection++ ) = atoi( strtok( 0, ::Delimiter ) ) - 1;
            }
        }
    }
    else
    {
        throw "Unknown element type.";
    }
}

void AVSUcd::read_components( FILE* const ifs )
{
    char buffer[ ::MaxLineLength ];

    if ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
    {
        m_ncomponents_per_node = atoi( strtok( buffer, ::Delimiter ) );

        for ( size_t i = 0; i < m_ncomponents_per_node; ++i )
        {
            m_veclens.push_back( atoi( strtok( 0, ::Delimiter ) ) );
        }
    }
    else
    {
        throw "Unexpected EOF in reading ncomponents per node";
    }

    for ( size_t i = 0; i < m_ncomponents_per_node; ++i )
    {
        if ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
        {
            const char* const component_name = strtok( buffer, ",\n\r" );
            if ( component_name != 0 )
            {
                m_component_names.push_back( std::string( component_name ) );
            }
            else
            {
                m_component_names.push_back( "" );
            }

            const char* const component_unit = strtok( 0, ",\n\r" );
            if ( component_unit != 0 )
            {
                m_component_units.push_back( std::string( component_unit ) );
            }
            else
            {
                m_component_units.push_back( "" );
            }
        }
        else
        {
            throw "Unexpected EOF in reading component name and unit";
        }
    }
}

void AVSUcd::read_values( FILE* const ifs )
{
    char buffer[ ::MaxLineLength ];

    const size_t veclen = m_veclens[ m_component_id ];
    m_values.allocate( veclen * m_nnodes );

    kvs::Real32* value = m_values.pointer();

    size_t nskips = 0;
    for ( size_t i = 0; i < m_component_id; ++i )
    {
        nskips += m_veclens[ i ];
    }

    for ( size_t i = 0; i < m_nnodes; i++ )
    {
        if ( fgets( buffer, ::MaxLineLength, ifs ) )
        {
            // Node index
            const int index = atoi( strtok( buffer, ::Delimiter ) ) - 1;

            // Skip other components
            for ( size_t j = 0; j < nskips; ++j )
            {
                strtok( 0, ::Delimiter );
            }

            for ( size_t j = 0; j < veclen; ++j )
            {
                value[ index * veclen + j ] = static_cast<kvs::Real32>( atof( strtok( 0, ::Delimiter ) ) );
            }
        }
    }
}

void AVSUcd::write_single_step_format( FILE* const ofs ) const
{
    fprintf( ofs, "%u %u %u 0 0\n",
             static_cast<unsigned int>( m_nnodes ),
             static_cast<unsigned int>( m_nelements ),
             static_cast<unsigned int>( m_nvalues_per_node ) );

    this->write_coords( ofs );
    this->write_connections( ofs );
    this->write_components( ofs );
    this->write_values( ofs );
}

void AVSUcd::write_coords( FILE* const ofs ) const
{
    size_t node_id = 1;

    const kvs::Real32*       coord = m_coords.pointer();
    const kvs::Real32* const end   = coord + m_coords.size();

    while ( coord < end )
    {
        const kvs::Real32 x = *( coord++ );
        const kvs::Real32 y = *( coord++ );
        const kvs::Real32 z = *( coord++ );

        fprintf( ofs, "%u %f %f %f\n",
                 static_cast<unsigned int>( node_id++ ),
                 x,
                 y,
                 z );
    }
}

void AVSUcd::write_connections( FILE* const ofs ) const
{
    size_t element_id = 1;

    const kvs::UInt32*       connection = m_connections.pointer();
    const kvs::UInt32* const end        = connection + m_connections.size();

    if ( m_element_type == Tetrahedra )
    {
        while ( connection < end )
        {
            const kvs::UInt32 node_id0 = *( connection++ );
            const kvs::UInt32 node_id1 = *( connection++ );
            const kvs::UInt32 node_id2 = *( connection++ );
            const kvs::UInt32 node_id3 = *( connection++ );

            fprintf( ofs, "%u 0 tet %u %u %u %u\n",
                     static_cast<unsigned int>( element_id++ ),
                     node_id0,
                     node_id1,
                     node_id2,
                     node_id3 );
        }
    }
    else if ( m_element_type == Tetrahedra2 )
    {
        while ( connection < end )
        {
            const kvs::UInt32 node_id0  = *( connection++ );
            const kvs::UInt32 node_id1  = *( connection++ );
            const kvs::UInt32 node_id2  = *( connection++ );
            const kvs::UInt32 node_id3  = *( connection++ );
            const kvs::UInt32 node_id4  = *( connection++ );
            const kvs::UInt32 node_id5  = *( connection++ );
            const kvs::UInt32 node_id6  = *( connection++ );
            const kvs::UInt32 node_id7  = *( connection++ );
            const kvs::UInt32 node_id8  = *( connection++ );
            const kvs::UInt32 node_id9  = *( connection++ );

            fprintf( ofs, "%u 0 tet %u %u %u %u %u %u %u %u %u %u\n",
                     static_cast<unsigned int>( element_id++ ),
                     node_id0,
                     node_id1,
                     node_id2,
                     node_id3,
                     node_id4,
                     node_id5,
                     node_id6,
                     node_id7,
                     node_id8,
                     node_id9 );
        }
    }
    else if ( m_element_type == Hexahedra )
    {
        while ( connection < end )
        {
            const kvs::UInt32 node_id0 = *( connection++ );
            const kvs::UInt32 node_id1 = *( connection++ );
            const kvs::UInt32 node_id2 = *( connection++ );
            const kvs::UInt32 node_id3 = *( connection++ );
            const kvs::UInt32 node_id4 = *( connection++ );
            const kvs::UInt32 node_id5 = *( connection++ );
            const kvs::UInt32 node_id6 = *( connection++ );
            const kvs::UInt32 node_id7 = *( connection++ );

            fprintf( ofs, "%u 0 hex %u %u %u %u %u %u %u %u\n",
                     static_cast<unsigned int>( element_id++ ),
                     node_id0,
                     node_id1,
                     node_id2,
                     node_id3,
                     node_id4,
                     node_id5,
                     node_id6,
                     node_id7 );
        }
    }
    else if ( m_element_type == Hexahedra2 )
    {
        while ( connection < end )
        {
            fprintf( ofs, "%u 0 hex2", static_cast<unsigned int>( element_id++ ) );

            const size_t nnodes = 20;
            for ( size_t i = 0; i < nnodes; i++ )
            {
                const kvs::UInt32 node_id = *( connection++ );
                fprintf( ofs, " %u", node_id );
            }

            fprintf( ofs, "\n" );
        }
    }
    else
    {
        throw "Unknown element type.";
    }
}

void AVSUcd::write_components( FILE* ofs ) const
{
    fprintf( ofs, "%u", static_cast<unsigned int>( m_ncomponents_per_node ) );
    for ( size_t i = 0; i < m_ncomponents_per_node; ++i )
    {
        fprintf( ofs, " %u", static_cast<unsigned int>( m_veclens[ i ] ) );
    }
    fprintf( ofs, "\n" );

    for ( size_t i = 0; i < m_ncomponents_per_node; ++i )
    {
        fprintf( ofs, "%s,%s\n", m_component_names[ i ].c_str(), m_component_units[ i ].c_str() );
    }
}

void AVSUcd::write_values( FILE* ofs ) const
{
    size_t node_id = 1;

    const kvs::Real32*       value = m_values.pointer();
    const kvs::Real32* const end   = value + m_values.size();

    while ( value < end )
    {
        fprintf( ofs, "%u %f\n", static_cast<unsigned int>( node_id++ ), *( value++ ) );
    }
}

const bool AVSUcd::CheckFileExtension( const std::string& filename )
{
    const kvs::File file( filename );
    if ( file.extension() == "ucd" || file.extension() == "UCD" ||
         file.extension() == "inp" || file.extension() == "INP" )
    {
        return( true );
    }

    return( false );
}

const bool AVSUcd::CheckFileFormat( const std::string& filename )
{
    FILE* ifs = fopen( filename.c_str(), "rb" );
    if( !ifs )
    {
        kvsMessageError( "Cannot open %s.", filename.c_str() );
        return( false );
    }

    const AVSUcd::FormatType format_type = ::CheckFormatType( ifs );
    if ( format_type == AVSUcd::SingleStep )
    {
        char buffer[ ::MaxLineLength ];
        while ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
        {
            // Skip comment line.
            if ( buffer[0] == '#' ) { continue; }
            else
            {
                const long nnodes = strtol( strtok( buffer, ::Delimiter ), NULL, 10 );
                if ( nnodes == 0 )
                {
                    fclose( ifs );
                    return( false );
                }

                const long nelements = strtol( strtok( 0, ::Delimiter ), NULL, 10 );
                if ( nelements == 0 )
                {
                    fclose( ifs );
                    return( false );
                }

                const long nvalues_per_node = strtol( strtok( 0, ::Delimiter ), NULL, 10 );
                if ( nvalues_per_node == 0 )
                {
                    fclose( ifs );
                    return( false );
                }

                fclose( ifs );
                return( true );
            }
        }
    }
    else if ( format_type == AVSUcd::MultiStep )
    {
        char buffer[ ::MaxLineLength ];
        while ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
        {
            // Skip comment line.
            if ( buffer[0] == '#' ) { continue; }
            else
            {
                const long nsteps = strtol( strtok( buffer, ::Delimiter ), NULL, 10 );
                if ( nsteps == 0 )
                {
                    fclose( ifs );
                    return( false );
                }

                if ( fgets( buffer, ::MaxLineLength, ifs ) != 0 )
                {
                    const char* const cycle_type = strtok( buffer, ::Delimiter );
                    if ( !strcmp( cycle_type, "data" ) ||
                         !strcmp( cycle_type, "geom" ) ||
                         !strcmp( cycle_type, "data_geom" ) )
                    {
                        fclose( ifs );
                        return( true );
                    }
                }

                fclose( ifs );
                return( false );
            }
        }
    }

    fclose( ifs );
    return( false );
}

} // end of namespace kvs
