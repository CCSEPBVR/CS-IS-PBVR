/****************************************************************************/
/**
 *  @file CommandLine.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: CommandLine.h 878 2011-08-01 05:44:17Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__COMMAND_LINE_H_INCLUDE
#define KVS__COMMAND_LINE_H_INCLUDE

#include <string>
#include <sstream>
#include <vector>
#include <typeinfo>
#include <algorithm>
#include <kvs/ClassName>
#include <kvs/Message>


namespace kvs
{

/*==========================================================================*/
/**
 *  Commandline class
 */
/*==========================================================================*/
class CommandLine
{
    kvsClassName( kvs::CommandLine );

public:

    enum HelpMessageMode
    {
        UsageOnly,     ///< usage only mode
        UsageAndOption ///< usage and option mode
    };

public:

    class Argument;
    class Option;
    class Value;

public:

    typedef std::vector<Argument> Arguments;
    typedef std::vector<Option>   Options;
    typedef std::vector<Value>    Values;

protected:

    int          m_argc;         ///< argument count
    char**       m_argv;         ///< argument values
    std::string  m_command_name; ///< command name
    size_t       m_max_length;   ///< max length of the option name
    bool         m_no_help;      ///< no help option
    std::string  m_help_option;  ///< help option character (ex: 'h')
    Arguments    m_arguments;    ///< argument values
    Options      m_options;      ///< options
    Values       m_values;       ///< values

public:

    CommandLine( int argc, char** argv );

    CommandLine( int argc, char** argv, const std::string& command_name );

    virtual ~CommandLine( void );

public:

    const int argc( void ) const;

    char** const argv( void ) const;

    const std::string& commandName( void ) const;

    const bool parse( void );

    const bool read( void );

    void clear( void );

public:

    template <class T>
    const T value( size_t index = 0 ) const;

    const bool hasValues( void ) const;

    const size_t nvalues( void ) const;

public:

    template <class T>
    const T optionValue( const std::string& option_name, size_t index = 0 ) const;

    const bool hasOption( const std::string& option_name ) const;

    const bool hasOptionValue( const std::string& option_name ) const;

    const size_t noptions( void ) const;

public:

    void addHelpOption( const std::string& help_option = "h" );

    void addOption(
        const std::string& name,
        const std::string& description,
        size_t             nvalues     = 0,
        bool               is_required = false );

    void addValue( const std::string& description, bool is_required = true );

    void showHelpMessage( HelpMessageMode mode = UsageOnly ) const;

protected:

    void add_help_option( const std::string& help_option = "h" );

    void add_option(
        const std::string& name,
        const std::string& description,
        size_t             nvalues     = 0,
        bool               is_required = false );

    void add_value( const std::string& description, bool is_required = true );

private:

    const bool is_option( Arguments::iterator& argument ) const;

    const std::string get_option_name( Arguments::iterator& argument ) const;

    const bool is_help_option( const char* value ) const;

    Options::iterator find_option( Arguments::iterator& argument );

    const bool read_option_values(
        Arguments::iterator& argument,
        Options::iterator&   option );

    void print_help_message( HelpMessageMode mode ) const;
};

/*==========================================================================*/
/**
 *  Argument value class.
 */
/*==========================================================================*/
class CommandLine::Argument
{
private:

    size_t m_length; ///< length of the argument value
    char*  m_data;   ///< argument value

public:

    Argument( void );

    explicit Argument( const char* data );

    Argument( const Argument& other );

    virtual ~Argument( void );

public:

    const size_t length( void ) const;

    const char* data( void ) const;

public:

    Argument& operator =( const char* data );

    Argument& operator =( const Argument& arg );
};

/*==========================================================================*/
/**
 *  Option class.
 */
/*==========================================================================*/
class CommandLine::Option
{
private:

    std::string           m_name;        ///< option name
    std::string           m_description; ///< option description
    size_t                m_nvalues;     ///< number of required values
    bool                  m_is_required; ///< true, if the option is required
    bool                  m_is_given;    ///< true, if the option is given
    std::vector<Argument> m_values;      ///< option values

public:

    Option( void );

    explicit Option(
        const std::string& name,
        const std::string& description = "",
        size_t             nvalues     = 0,
        bool               is_required = false );

    Option( const Option& other );

    virtual ~Option( void );

public:

    void setValue( const Argument& value );

    void given( void );

public:

    const std::string& name( void ) const;

    const std::string& description( void ) const;

    const size_t nvalues( void ) const;

    const bool isRequired( void ) const;

    const bool isGiven( void ) const;

    const std::vector<Argument>& values( void ) const;

    template <typename T>
    const T value( size_t index ) const;

public:

    Option& operator =( const Option& rhs );

    friend const bool operator <( const Option& lhs, const Option& rhs );

    friend const bool operator ==( const Option& lhs, const Option& rhs );
};

/*==========================================================================*/
/**
 *  Value class.
 */
/*==========================================================================*/
class CommandLine::Value
{
private:

    std::string m_description; ///< value description
    bool        m_is_required; ///< true, if the value is required
    bool        m_is_given;    ///< true, if the value is given
    Argument    m_value;       ///< value (not allocated)

public:

    Value( void );

    explicit Value(
        const std::string& description,
        bool               is_required = true );

    Value( const Value& other );

    virtual ~Value( void );

public:

    void setValue( const char* value );

    const std::string& description( void ) const;

    const bool isRequired( void ) const;

    const bool isGiven( void ) const;

    template <typename T>
    const T value( void ) const;

public:

    Value& operator =( const Value& rhs );

    friend const bool operator <( const Value& lhs, const Value& rhs );

    friend const bool operator ==( const Value& lhs, const Value& rhs );
};

/*==========================================================================*/
/**
 *  Get input value.
 *  @param index [in] index of input value list
 *  @return input value
 */
/*==========================================================================*/
template <class T>
inline const T CommandLine::value( size_t index ) const
{
    return( m_values[index].value<T>() );
}

/*==========================================================================*/
/**
 *  Get option value for given option.
 *  @param option_name [in] option name
 *  @param index [in] index of option value list
 *  @return option value
 */
/*==========================================================================*/
template <class T>
inline const T CommandLine::optionValue( const std::string& option_name, size_t index ) const
{
    Option                  key( option_name );
    Options::const_iterator option =
        std::find( m_options.begin(), m_options.end(), key );

    if ( ( option == m_options.end() ) || ( !option->isGiven() ) )
    {
        kvsMessageError( "Cannot find '-%s' option.", option->name().c_str() );
        return( T( 0 ) );
    }

    if ( (size_t)( option->nvalues() ) < index )
    {
        kvsMessageError( "Cannot get option value by given index." );
        return( T( 0 ) );
    }

    T ret = option->value<T>( index );
    return( ret );
}

/*==========================================================================*/
/**
 *  Get option value.
 *  @param index [in] index of option value list
 *  @return option value
 */
/*==========================================================================*/
template <typename T>
inline const T CommandLine::Option::value( size_t index ) const
{
    if ( m_nvalues < index )
    {
        kvsMessageError( "Option '-%s' has only %d values.",
                         m_name.c_str(), m_nvalues );
        return( T( 0 ) );
    }

    T ret;
    std::stringstream s( m_values[index].data() );
    s >> ret;

    return( ret );
}

/*==========================================================================*/
/**
 *  Get the option value (Specialization for std::string class).
 *  @param index [in] index of option value list
 *  @return option value
 */
/*==========================================================================*/
template<>
inline const std::string CommandLine::Option::value<std::string>( size_t index ) const
{
    if ( m_nvalues < index )
    {
        kvsMessageError( "Option '-%s' has only %d values.",
                         m_name.c_str(), m_nvalues );
        return( "" );
    }

    return( m_values[index].data() );
}

/*==========================================================================*/
/**
 *  Get value.
 *  @return value
 */
/*==========================================================================*/
template <typename T>
inline const T CommandLine::Value::value( void ) const
{
    T ret;
    std::stringstream s( m_value.data() );
    s >> ret;

    return( ret );
}

/*==========================================================================*/
/**
 *  Get the value (Specialization for std::string class).
 *  @return value
 */
/*==========================================================================*/
template <>
inline const std::string CommandLine::Value::value<std::string>( void ) const
{
    return( m_value.data() );
}

} // end of namespace kvs

#endif // KVS__COMMAND_LINE_H_INCLUDE
