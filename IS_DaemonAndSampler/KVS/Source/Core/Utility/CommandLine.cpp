/****************************************************************************/
/**
 *  @file CommandLine.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: CommandLine.cpp 878 2011-08-01 05:44:17Z naohisa.sakamoto $
 */
/****************************************************************************/
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <kvs/DebugNew>
#include <kvs/CommandLine>
#include <kvs/Message>
#include <kvs/Math>


namespace kvs
{

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
CommandLine::Argument::Argument( void )
    : m_length( 0 )
    , m_data( 0 )
{
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param data [in] pointer to argument value
 */
/*==========================================================================*/
CommandLine::Argument::Argument( const char* data )
    : m_length( strlen( data ) )
    , m_data( 0 )
{
    m_data = new char[ m_length + 1 ];
    if ( !m_data )
    {
        kvsMessageError( "Cannot allocate memory." );
        return;
    }

    memset( m_data, 0, m_length + 1 );
    strncpy( m_data, data, m_length );
    m_data[ m_length ] = '\0';
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param arg [in] argument
 */
/*==========================================================================*/
CommandLine::Argument::Argument( const CommandLine::Argument& other )
    : m_length( other.m_length )
    , m_data( 0 )
{
    m_data = new char[ m_length + 1 ];
    if ( !m_data )
    {
        kvsMessageError( "Cannot allocate memory." );
        return;
    }

    memset( m_data, 0, m_length + 1 );
    strncpy( m_data, other.m_data, m_length );
    m_data[ m_length ] = '\0';
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
CommandLine::Argument::~Argument( void )
{
    if ( m_data ) { delete[] m_data; }
}

/*==========================================================================*/
/**
 *  Get length of argument value.
 *  @return length of argument value
 */
/*==========================================================================*/
const size_t CommandLine::Argument::length( void ) const
{
    return( m_length );
}

/*==========================================================================*/
/**
 *  Get argument value.
 *  @return argument value
 */
/*==========================================================================*/
const char* CommandLine::Argument::data( void ) const
{
    return( m_data );
}

/*==========================================================================*/
/**
 *  Substitution operator.
 *  @param data [in] pointer to argument value
 */
/*==========================================================================*/
CommandLine::Argument& CommandLine::Argument::operator =( const char* rhs )
{
    if ( m_data ) { delete[] m_data; }

    m_length = strlen( rhs );
    m_data = new char[ m_length + 1 ];
    if ( !m_data )
    {
        kvsMessageError( "Cannot allocate memory." );
        return( *this );
    }

    memset( m_data, 0, m_length + 1 );
    strncpy( m_data, rhs, m_length );
    m_data[ m_length ] = '\0';

    return( *this );
}

/*==========================================================================*/
/**
 *  Substitution operator.
 *  @param arg [in] argument value
 */
/*==========================================================================*/
CommandLine::Argument& CommandLine::Argument::operator =( const CommandLine::Argument& rhs )
{
    if ( m_data ) { delete[] m_data; }

    m_length = rhs.m_length;
    m_data = new char[ m_length + 1 ];
    if ( !m_data )
    {
        kvsMessageError( "Cannot allocate memory." );
        return( *this );
    }

    memset( m_data, 0, m_length + 1 );
    strncpy( m_data, rhs.m_data, m_length );
    m_data[ m_length ] = '\0';

    return( *this );
}

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
CommandLine::Option::Option( void )
    : m_name( "" )
    , m_description( "" )
    , m_nvalues( 0 )
    , m_is_required( false )
    , m_is_given( false )
    , m_values( 0 )
{
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param name [in] option name
 *  @param description [in] description for option
 *  @param nvalues [in] number of option values
 *  @param is_required [in] whether option is required ?
 */
/*==========================================================================*/
CommandLine::Option::Option(
    const std::string& name,
    const std::string& description,
    size_t             nvalues,
    bool               is_required )
    : m_name( name )
    , m_description( description )
    , m_nvalues( nvalues )
    , m_is_required( is_required )
    , m_is_given( false )
    , m_values( 0 )
{
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param option [in] option
 */
/*==========================================================================*/
CommandLine::Option::Option( const CommandLine::Option& other )
    : m_name( other.m_name )
    , m_description( other.m_description )
    , m_nvalues( other.m_nvalues )
    , m_is_required( other.m_is_required )
    , m_is_given( other.m_is_given )
    , m_values( 0 )
{
    std::copy( other.m_values.begin(),
               other.m_values.end(),
               m_values.begin() );
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
CommandLine::Option::~Option( void )
{
    m_values.clear();
}

/*==========================================================================*/
/**
 *  Set option value.
 */
/*==========================================================================*/
void CommandLine::Option::setValue( const CommandLine::Argument& value )
{
    m_values.push_back( value );
    m_is_given = true;
}

/*==========================================================================*/
/**
 *  Change option status to 'given'.
 */
/*==========================================================================*/
void CommandLine::Option::given( void )
{
    m_is_given = true;
}

/*==========================================================================*/
/**
 *  Get option name.
 */
/*==========================================================================*/
const std::string& CommandLine::Option::name( void ) const
{
    return( m_name );
}

/*==========================================================================*/
/**
 *  Get option description.
 */
/*==========================================================================*/
const std::string& CommandLine::Option::description( void ) const
{
    return( m_description );
}

/*==========================================================================*/
/**
 *  Get number of option values.
 *  @return number of option values
 */
/*==========================================================================*/
const size_t CommandLine::Option::nvalues( void ) const
{
    return( m_nvalues );
}

/*==========================================================================*/
/**
 *  Test whether option is required.
 *  @return true, if option is required
 */
/*==========================================================================*/
const bool CommandLine::Option::isRequired( void ) const
{
    return( m_is_required );
}

/*==========================================================================*/
/**
 *  Test whether option is already given.
 *  @return true, if option is given
 */
/*==========================================================================*/
const bool CommandLine::Option::isGiven( void ) const
{
    return( m_is_given );
}

/*==========================================================================*/
/**
 *  Get option value list.
 *  @return option value list
 */
/*==========================================================================*/
const std::vector<CommandLine::Argument>& CommandLine::Option::values( void ) const
{
    return( m_values );
}

/*==========================================================================*/
/**
 *  Substitution operator.
 *  @param option [in] option value
 */
/*==========================================================================*/
CommandLine::Option& CommandLine::Option::operator =( const CommandLine::Option& rhs )
{
    m_name        = rhs.m_name;
    m_description = rhs.m_description;
    m_nvalues     = rhs.m_nvalues;
    m_is_required = rhs.m_is_required;
    m_is_given    = rhs.m_is_given;

    std::copy( rhs.m_values.begin(),
               rhs.m_values.end(),
               m_values.begin() );

    return( *this );
}

/*==========================================================================*/
/**
 *  Comparison operator '<'.
 *  @param lhs [in] option (left hand side)
 *  @param rhs [in] option (right hand side)
 */
/*==========================================================================*/
const bool operator <( const CommandLine::Option& lhs, const CommandLine::Option& rhs )
{
    return( lhs.m_name < rhs.m_name );
}

/*==========================================================================*/
/**
 *  Comparison operator '=='.
 *  @param lhs [in] option (left hand side)
 *  @param rhs [in] option (right hand side)
 */
/*==========================================================================*/
const bool operator ==( const CommandLine::Option& lhs, const CommandLine::Option& rhs )
{
    return( lhs.m_name == rhs.m_name );
}

/*==========================================================================*/
/**
 *  Constructor.
 */
/*==========================================================================*/
CommandLine::Value::Value( void )
    : m_description( "" )
    , m_is_required( true )
    , m_is_given( false )
    , m_value()
{
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param description [in] description of value
 *  @param is_required [in] whether option is required ?
 */
/*==========================================================================*/
CommandLine::Value::Value( const std::string& description, bool is_required )
    : m_description( description )
    , m_is_required( is_required )
    , m_is_given( false )
    , m_value()
{
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param value [in] value
 */
/*==========================================================================*/
CommandLine::Value::Value( const CommandLine::Value& other )
    : m_description( other.m_description )
    , m_is_required( other.m_is_required )
    , m_is_given( other.m_is_given )
    , m_value( other.m_value )
{
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
CommandLine::Value::~Value( void )
{
}

/*==========================================================================*/
/**
 *  Set value.
 *  @param value [in] pinter to value
 */
/*==========================================================================*/
void CommandLine::Value::setValue( const char* value )
{
    m_value    = value;
    m_is_given = true;
}

/*==========================================================================*/
/**
 *  Get description of value
 *  @return description of value
 */
/*==========================================================================*/
const std::string& CommandLine::Value::description( void ) const
{
    return( m_description );
}

/*==========================================================================*/
/**
 *  Test whether value is required.
 *  @return true, if value is required
 */
/*==========================================================================*/
const bool CommandLine::Value::isRequired( void ) const
{
    return( m_is_required );
}

/*==========================================================================*/
/**
 *  Test whether value is already given.
 *  @return true, if value is given
 */
/*==========================================================================*/
const bool CommandLine::Value::isGiven( void ) const
{
    return( m_is_given );
}

/*==========================================================================*/
/**
 *  Substitution operator.
 *  @param value [in] value.
 */
/*==========================================================================*/
CommandLine::Value& CommandLine::Value::operator =( const CommandLine::Value& rhs )
{
    m_description = rhs.m_description;
    m_value       = rhs.m_value;

    return( *this );
}

/*==========================================================================*/
/**
 *  Comparison operator '<'.
 *  @param lhs [in] value (left hand side)
 *  @param rhs [in] value (right hand side)
 */
/*==========================================================================*/
const bool operator <( const CommandLine::Value& lhs, const CommandLine::Value& rhs )
{
    return( lhs.m_description < rhs.m_description );
}

/*==========================================================================*/
/**
 *  Comparison operator '=='.
 *  @param lhs [in] value (left hand side)
 *  @param rhs [in] value (right hand side)
 */
/*==========================================================================*/
const bool operator ==( const CommandLine::Value& lhs, const CommandLine::Value& rhs )
{
    return( lhs.m_description == rhs.m_description );
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param argc [in] argument count
 *  @param argv [in] argument value
 */
/*==========================================================================*/
CommandLine::CommandLine( int argc, char** argv )
    : m_argc( argc )
    , m_argv( argv )
    , m_command_name( std::string( argv[0] ) )
    , m_max_length( 0 )
    , m_no_help( true )
    , m_help_option( "h" )
    , m_arguments( 0 )
    , m_options( 0 )
    , m_values( 0 )
{
}

/*==========================================================================*/
/**
 *  Constructor.
 *  @param argc [in] argument count
 *  @param argv [in] argument value
 *  @param command_name [in] command name
 */
/*==========================================================================*/
CommandLine::CommandLine( int argc, char** argv, const std::string& command_name )
    : m_argc( argc )
    , m_argv( argv )
    , m_command_name( command_name )
    , m_max_length( 0 )
    , m_no_help( true )
    , m_help_option( "h" )
    , m_arguments( 0 )
    , m_options( 0 )
    , m_values( 0 )
{
}

/*==========================================================================*/
/**
 *  Destructor.
 */
/*==========================================================================*/
CommandLine::~CommandLine( void )
{
    this->clear();
}

/*==========================================================================*/
/**
 *  Get arument count.
 *  @return argument count
 */
/*==========================================================================*/
const int CommandLine::argc( void ) const
{
    return( m_argc );
}

/*==========================================================================*/
/**
 *  Get argument values.
 *  @return argument values
 */
/*==========================================================================*/
char** const CommandLine::argv( void ) const
{
    return( m_argv );
}

/*==========================================================================*/
/**
 *  Get command name.
 *  @return command name
 */
/*==========================================================================*/
const std::string& CommandLine::commandName( void ) const
{
    return( m_command_name );
}

/*==========================================================================*/
/**
 *  Parse command line arguments.
 *  @return true, if parse process is done successfully
 */
/*==========================================================================*/
const bool CommandLine::parse( void )
{
    bool allow_no_value = ( m_values.size() == 0 );
    if( !allow_no_value )
    {
        // Check the 'is_required' property of the input values, when the some
        // input values are added in m_values.
        Values::const_iterator v = m_values.begin();
        while ( v != m_values.end() )
        {
            if ( !v->isRequired() ) { allow_no_value = true; }
            ++v;
        }
    }

    // Output help message.
    if ( m_argc == 1 && !allow_no_value )
    {
        this->print_help_message( UsageOnly );
        this->clear();
        return( false );
    }

    // Oputput help message in detail when '-h' option is given.
    for ( int i = 1; i < m_argc; ++i )
    {
        const char* const p_value = m_argv[i];

        if ( !m_no_help && is_help_option( p_value ) )
        {
            this->print_help_message( UsageAndOption );
            this->clear();
            return( false );
        }
        else
        {
            CommandLine::Argument arg( p_value );
            m_arguments.push_back( arg );
        }
    }

    // Main loops in this method.
    Arguments::iterator argument = m_arguments.begin();
    Options::iterator   option   = m_options.begin();
    Values::iterator    value    = m_values.begin();
    while ( argument != m_arguments.end() )
    {
        if ( this->is_option( argument ) )
        {
            // Search the given option from the option set.
            option = this->find_option( argument );
            if ( option == m_options.end() )
            {
                kvsMessageError( "Unknown option '%s'", argument->data() );
                this->clear();
                return( false );
            }

            // Read the option values.
            if ( !this->read_option_values( argument, option ) )
            {
                kvsMessageError( "%d values is required for the option '-%s'",
                                 option->nvalues(),
                                 option->name().c_str() );
                this->clear();
                return( false );
            }
        }
        else
        {
            if ( value != m_values.end() )
            {
                value->setValue( argument->data() );

                ++value;
            }

            ++argument;
        }
    }

    // Check the required argument.
    option = m_options.begin();
    for ( size_t i = 0; i < m_options.size(); ++i )
    {
        if ( option->isRequired() )
        {
            if ( !option->isGiven() )
            {
                kvsMessageError( "Option '-%s' is required.", option->name().c_str() );
                this->clear();
                return( false );
            }
        }
        ++option;
    }

    value = m_values.begin();
    for ( size_t i = 0; i < m_values.size(); ++i )
    {
        if ( value->isRequired() )
        {
            if ( !value->isGiven() )
            {
                kvsMessageError("Input value is required.");
                this->clear();
                return( false );
            }
        }
        ++value;
    }

    return( true );
}

/*==========================================================================*/
/**
 *  Read command line arguments.
 *  @return true, if the reading process is done successfully
 */
/*==========================================================================*/
const bool CommandLine::read( void )
{
    bool allow_no_value = ( m_values.size() == 0 );
    if( !allow_no_value )
    {
        // Check the 'is_required' property of the input values, when the some
        // input values are added in m_values.
        Values::const_iterator v = m_values.begin();
        while ( v != m_values.end() )
        {
            if ( !v->isRequired() ) { allow_no_value = true; }
            ++v;
        }
    }

    // Output help message.
    if ( m_argc == 1 && !allow_no_value )
    {
        this->print_help_message( UsageOnly );
        this->clear();
        return( false );
    }

    // Oputput help message in detail when '-h' option is given.
    for ( int i = 1; i < m_argc; ++i )
    {
        const char* const p_value = m_argv[i];

        if ( !m_no_help && is_help_option( p_value ) )
        {
            this->print_help_message( UsageAndOption );
            this->clear();
            return( false );
        }
        else
        {
            CommandLine::Argument arg( p_value );
            m_arguments.push_back( arg );
        }
    }

    // Main loops in this method.
    Arguments::iterator argument = m_arguments.begin();
    Options::iterator   option   = m_options.begin();
    Values::iterator    value    = m_values.begin();
    while ( argument != m_arguments.end() )
    {
        if ( this->is_option( argument ) )
        {
            // Serch the given option from the option set.
            option = this->find_option( argument );
            if ( option == m_options.end() ) { return( true ); }

            // Read the option values.
            if ( !this->read_option_values( argument, option ) ) { return( true ); }
        }
        else
        {
            if ( value != m_values.end() )
            {
                if ( value->isRequired() )
                {
                    if ( !argument->data() ) { return( true ); }

                    value->setValue( argument->data() );
                }

                ++value;
            }

            ++argument;
        }
    }

    return( true );
}

/*===========================================================================*/
/**
 *  @brief  Clear argument lists.
 */
/*===========================================================================*/
void CommandLine::clear( void )
{
    m_arguments.clear();
    m_options.clear();
    m_values.clear();
}

/*==========================================================================*/
/**
 *  Test whether command line has input value.
 *  @return true, if command line has input value
 */
/*==========================================================================*/
const bool CommandLine::hasValues( void ) const
{
    Values::const_iterator value      = m_values.begin();
    Values::const_iterator last_value = m_values.end();

    if ( value == last_value ) { return( false ); }

    while ( value != last_value )
    {
        if ( !value->isGiven() ) { return( false ); }
        ++value;
    }

    return( true );
}

/*==========================================================================*/
/**
 *  Get number of input values.
 *  @return number of input values
 */
/*==========================================================================*/
const size_t CommandLine::nvalues( void ) const
{
    return( m_values.size() );
}

/*==========================================================================*/
/**
 *  Test whether command line has given option.
 *  @param option_name [in] option name
 *  @return true, if command line has given option
 */
/*==========================================================================*/
const bool CommandLine::hasOption( const std::string& option_name ) const
{
    Option            key( option_name );
    Options::const_iterator p = std::find( m_options.begin(), m_options.end(), key );

    return( ( p != m_options.end() ) && ( p->isGiven() ) );
}

/*==========================================================================*/
/**
 *  Test whether command line has option value for given option.
 *  @param option_name [in] option name
 *  @return true, if command line has option value for given option
 */
/*==========================================================================*/
const bool CommandLine::hasOptionValue( const std::string& option_name ) const
{
    Option            key( option_name );
    Options::const_iterator p = std::find( m_options.begin(), m_options.end(), key );
    if ( p == m_options.end() ) { return( false ); }

    return( p->nvalues() > 0 );
}

/*==========================================================================*/
/**
 *  Get number of options.
 *  @return number of options
 */
/*==========================================================================*/
const size_t CommandLine::noptions( void ) const
{
    return( m_options.size() );
}

/*==========================================================================*/
/**
 *  Add help option.
 *  @param help_option [in] character for help option
 */
/*==========================================================================*/
void CommandLine::addHelpOption( const std::string& help_option )
{
    this->add_help_option( help_option );
}

/*==========================================================================*/
/**
 *  Add option.
 *  @param name [in] option name
 *  @param description [in] option description
 *  @param nvalues [in] number of option values
 *  @param is_required [in] whether option is required ?
 */
/*==========================================================================*/
void CommandLine::addOption(
    const std::string& name,
    const std::string& description,
    size_t             nvalues,
    bool               is_required )
{
    this->add_option( name, description, nvalues, is_required );
}

/*==========================================================================*/
/**
 *  Add input value.
 *  @param description [in] description of input value
 *  @param is_required [in] whether input value is required ?
 */
/*==========================================================================*/
void CommandLine::addValue( const std::string& description, bool is_required )
{
    this->add_value( description, is_required );
}

/*==========================================================================*/
/**
 *  Show help message.
 *  @param mode [in] help message mode
 */
/*==========================================================================*/
void CommandLine::showHelpMessage( HelpMessageMode mode ) const
{
    this->print_help_message( mode );
}

/*==========================================================================*/
/**
 *  Add help option.
 *  @param help_option [in] character for help option
 */
/*==========================================================================*/
void CommandLine::add_help_option( const std::string& help_option )
{
    m_help_option = help_option;
    m_no_help     = false;

    this->add_option( help_option, "Output help message.", 0, false );
}

/*==========================================================================*/
/**
 *  Add option.
 *  @param name [in] option name
 *  @param description [in] option description
 *  @param nvalues [in] number of option values
 *  @param is_required [in] whether option is required ?
 */
/*==========================================================================*/
void CommandLine::add_option(
    const std::string& name,
    const std::string& description,
    size_t             nvalues,
    bool               is_required )
{
    m_max_length = Math::Max( name.length() + 1, m_max_length );

    Option option( name, description, nvalues, is_required );
    m_options.push_back( option );
}

/*==========================================================================*/
/**
 *  Add input value.
 *  @param description [in] description of input value
 *  @param is_required [in] whether input value is required ?
 */
/*==========================================================================*/
void CommandLine::add_value( const std::string& description, bool is_required )
{
    CommandLine::Value value( description, is_required );
    m_values.push_back( value );
}

/*==========================================================================*/
/**
 *  Test whether given argument is option.
 *  @param argument [in] argument
 *  @return true, if given argument is option
 */
/*==========================================================================*/
const bool CommandLine::is_option( Arguments::iterator& argument ) const
{
    return( argument->data()[0] == '-' );
}

/*==========================================================================*/
/**
 *  Get option name.
 *  @param argument [in] iterator of argument
 *  @return option name
 */
/*==========================================================================*/
const std::string CommandLine::get_option_name( Arguments::iterator& argument ) const
{
    return( std::string( argument->data() ).substr( 1 ) );
}


/*==========================================================================*/
/**
 *  Test whether given value is help option.
 *  @param value [in] pointer to value
 *  @return true, if given value is help option
 */
/*==========================================================================*/
const bool CommandLine::is_help_option( const char* value ) const
{
    return( std::string( value ) == "-" + m_help_option );
}

/*==========================================================================*/
/**
 *  Find option.
 *  @param argument [in] argument
 *  @return iterator of option for given argument
 */
/*==========================================================================*/
CommandLine::Options::iterator CommandLine::find_option(
    Arguments::iterator& argument )
{
    std::string         option_name( this->get_option_name( argument ) );
    CommandLine::Option key( option_name );

    return( std::find( m_options.begin(), m_options.end(), key ) );
}

/*==========================================================================*/
/**
 *  Read option values.
 *  @param argument [in] iterator of argument
 *  @param option [in] iterator of option
 *  @return true, if read process is done successfully
 */
/*==========================================================================*/
const bool CommandLine::read_option_values(
    Arguments::iterator& argument,
    Options::iterator&   option )
{
    ++argument; // skip option name.

    // No option value.
    if ( option->nvalues() == 0 )
    {
        option->given();
        return( true );
    }

    for ( size_t i = 0; i < option->nvalues(); ++i )
    {
        if ( argument == m_arguments.end() ) { return( false ); }
        if ( this->is_option( argument ) ) { return( false ); }

        option->setValue( *argument );

        ++argument;
    }

    return( true );
}

/*==========================================================================*/
/**
 *  Print help message.
 *  @param mode [in] help message mode
 */
/*==========================================================================*/
void CommandLine::print_help_message( HelpMessageMode mode ) const
{
    if ( mode >= UsageOnly )
    {
        std::cerr << "Usage: " << m_command_name << " ";
        std::cerr << "[options] ";
        const size_t ninputs = m_values.size();

        if ( ninputs > 0 )
        {
            for ( size_t i = 0; i < ninputs; ++i )
            {
                std::cerr << "<" << m_values[i].description() << "> ";
            }
        }
        std::cerr << std::endl;
    }

    if ( mode >= UsageAndOption )
    {
        std::cerr << std::endl << "Options:" << std::endl;
        Options::const_iterator option = m_options.begin();
        while ( option != m_options.end() )
        {
            std::string option_name( "-" + option->name() );
            std::cerr << std::left << std::setw( m_max_length + 1 ) << option_name;
            std::cerr << ": " << option->description();
            if ( option->isRequired() ) { std::cerr << " [required]"; }
            std::cerr << std::endl;
            ++option;
        }
    }
}

} // end of namespace kvs
