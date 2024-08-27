/*****************************************************************************/
/**
 *  @file   function.h
 *  @brief  Function class.
 *
 *  @author Naohia Sakamoto
 *  @date   2007/11/16 13:54:44
 */
/*----------------------------------------------------------------------------
 *
 *  $Author: naohisa $
 *  $Date: 2007/11/16 11:38:51 $
 *  $Source: /home/Repository/viz-server2/cvsroot/KVS/Library/Numeric/function.h,v $
 *  $Revision: 1.7 $
 */
/*****************************************************************************/
#ifndef KVS__FUNCTION_H_INCLUDE
#define KVS__FUNCTION_H_INCLUDE

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cmath>
#include <vector>
#include <string.h>

//#include "Utility/macro.h"

#ifdef WIN32
#define strdup _strdup
#endif

namespace FuncParser
{

/*===========================================================================*/
/**
 *  @brief
 */
/*===========================================================================*/
class FunctionParam
{
private:

    int    m_ip; // int type parameter
    double m_dp; // double type parameter
    void*  m_vp; // void type parameter

public:

    FunctionParam( void ):
        m_ip( 0 ),
        m_dp( 0.0 ),
        m_vp( 0 ) {};

    virtual ~FunctionParam( void ) {};

public:

    inline FunctionParam& operator = ( const int ip )
    {
        m_ip = ip;
        return ( *this );
    };

    inline FunctionParam& operator = ( const double dp )
    {
        m_dp = dp;
        return ( *this );
    };

    inline FunctionParam& operator = ( const void* vp )
    {
        m_vp = ( void* )vp;
        return ( *this );
    };

    inline operator int () const
    {
        return ( m_ip );
    };

    inline operator double () const
    {
        return ( m_dp );
    };

    inline operator void* () const
    {
        return ( m_vp );
    };
};

/*===========================================================================*/
/**
 *  @brief
 */
/*===========================================================================*/
class Value
{
public:

    Value( void ) {};

    virtual ~Value( void ) {};

public:

    virtual double v( FunctionParam* p ) = 0;
};

class Values : public Value
{
private:

    int     m_n;
    double* m_v;

    Values( void );

public:

    Values( const int n ):
        m_n( n ),
        m_v( 0 )
    {
        m_v = new double [m_n];
    };

    ~Values( void )
    {
        delete [] m_v;
    };

public:

    double v( FunctionParam* p )
    {
        return ( *( m_v + ( int ) * p ) );
    };

    inline double& operator () ( const int index )
    {
        //KVS_MACRO_ASSERT( index < m_n );
        if ( index >= m_n ) std::cout << "Error: index exceed array size\n";
        return ( m_v[index] );
    };

    inline double& operator [] ( const int index )
    {
        //KVS_MACRO_ASSERT( index < m_n );
        if ( index >= m_n ) std::cout << "Error: index exceed array size\n";
        return ( m_v[index] );
    };
};


/*===========================================================================*/
/**
 *  @brief
 */
/*===========================================================================*/
class FunctionCounter
{
private:

    unsigned long int        m_LINKcnt;
    static unsigned long int m_DATAcnt;

protected:

    static unsigned long int m_IDcnt;

public:

    FunctionCounter( void ):
        m_LINKcnt( 0 )
    {
        m_IDcnt++;
        m_DATAcnt++;
    };

    virtual ~FunctionCounter( void )
    {
        m_DATAcnt--;
    };

public:

    unsigned long int link_cnt( void ) const
    {
        return ( m_LINKcnt );
    };

    unsigned long int link( void )
    {
        return ( ++m_LINKcnt );
    };

    unsigned long int unlink( void )
    {
        return ( ( m_LINKcnt > 0 ) ? --m_LINKcnt : m_LINKcnt );
    };

    static unsigned long int DATAcnt( void )
    {
        return ( m_DATAcnt );
    };

    static unsigned long int IDcnt( void )
    {
        return ( m_IDcnt );
    };
};

/*===========================================================================*/
/**
 *  @brief
 */
/*===========================================================================*/
class FunctionCore : public FunctionCounter
{
    friend class Function;

private:

    unsigned long int m_id;
    int               m_disable;
    const char*       m_tag;

    FunctionCore( void );

protected:

    double            m_v;
    Value*            m_uv;

    FunctionCore( const FunctionCore& org ):
        m_id( org.m_id ),
        m_disable( org.m_disable ),
        m_tag( org.m_tag ),
        m_v( org.m_v ),
        m_uv( org.m_uv ) {};

    FunctionCore( const double v ):
        m_id( m_IDcnt ),
        m_disable( 0 ),
        m_tag( 0 ),
        m_v( v ),
        m_uv( 0 ) {};

    class str_stream
    {
    private:

        int   m_len;
        char* m_p;

    public:

        str_stream( void ):
            m_len( 0 ),
            m_p( 0 ) {};

        virtual ~str_stream( void )
        {
            delete [] m_p;
        };

    public:

        inline char* str( void ) const
        {
            return ( m_p );
        };

        inline str_stream& operator << ( const char* a )
        {
            if ( !a ) return ( *this );

            char* p = new char [ ( m_len += strlen( a ) ) + 1 ];
            strcpy( p, m_p ? m_p : "" );
            strcat( p, a );

            delete [] m_p;
            m_p = p;
            return ( *this );
        };

        inline str_stream& operator << ( const double a )
        {
            char p[256];
            sprintf( p, "%g", a );
            return ( *this << p );
        };

        inline str_stream& operator << ( const int a )
        {
            char p[256];
            sprintf( p, "%d", a );
            return ( *this << p );
        };
    };

public:

    virtual ~FunctionCore () {};

    virtual double eval( FunctionParam* p ) const = 0;

    virtual double deval( const FunctionCore& x, FunctionParam* p ) const = 0;

    virtual FunctionCore* d( const FunctionCore& x ) const = 0;

    virtual char* str( void ) const = 0;

    virtual FunctionCore* copy( void ) const = 0;

    virtual int level( void ) const
    {
        return ( 0 );
    };

    inline unsigned long int id( void ) const
    {
        return ( m_id );
    };

    inline int disable( void ) const
    {
        return ( m_disable );
    };

    inline int enable( void ) const
    {
        return ( !m_disable );
    };

    inline const char* tag( void ) const
    {
        return ( m_tag );
    };

    virtual bool find( const char *name ) const
    {
        if (name == NULL) return false;
        if (m_tag == NULL) return false;
        return (strcmp(name, m_tag) == 0);
    }
};

/*===========================================================================*/
/**
 *  @brief
 */
/*===========================================================================*/
class FunctionCore_opConst: public FunctionCore
{
private:

    FunctionCore_opConst( const FunctionCore_opConst& org ):
        FunctionCore( org ) {};

public:

    FunctionCore_opConst( const double v = 0.0 ):
        FunctionCore( v ) {};

    ~FunctionCore_opConst( void ) {};

private:

    double eval( FunctionParam* p ) const
    {
        return ( ( p && m_uv ) ? m_uv->v( p ) : m_v );
    };

    double deval( const FunctionCore& x, FunctionParam* p ) const
    {
        return ( disable() ? 0.0 : ( ( x.id() == id() ) ? 1.0 : 0.0 ) );
    };

    FunctionCore* d( const FunctionCore& x ) const
    {
        return ( disable() ?
                 new FunctionCore_opConst( 0.0 ) :
                 new FunctionCore_opConst( ( x.id() == id() ) ? 1.0 : 0.0 ) );
    };

    char* str( void ) const
    {
        str_stream s;
        if ( tag() ) return ( strdup( tag() ) );
        else        return ( s << m_v, strdup( s.str() ) );
    }

    FunctionCore* copy( void ) const
    {
        return ( new FunctionCore_opConst( *this ) );
    };
};

/*===========================================================================*/
/**
 *  @brief
 */
/*===========================================================================*/
class Function
{
private:

    FunctionCore* m_p;

public:

    Function( const double v = 0.0 ):
        m_p( new FunctionCore_opConst( v ) )
    {
        m_p->link();
    };

    Function( FunctionCore* p ):
        m_p( p )
    {
        m_p->link();
    };

    Function( const Function& org ):
        m_p( org.m_p )
    {
        m_p->link();
    };

    ~Function( void )
    {
        if ( m_p->unlink() < 1 ) delete m_p;
    };

public:

    inline FunctionCore* core( void ) const
    {
        return ( m_p->copy() );
    };

    inline operator const FunctionCore& () const
    {
        return ( *m_p );
    };

    inline double operator = ( const double org )
    {
        m_p->m_v = org;
        return ( org );
    };

    inline double operator += ( const double org )
    {
        m_p->m_v += org;
        return ( org );
    };

    inline Value& operator = ( Value& org )
    {
        m_p->m_uv = &org;
        return ( org );
    };

    inline Function& operator = ( const Function& org )
    {
        if ( this == &org ) return ( *this );
        if ( m_p->unlink() < 1 ) delete m_p;
        m_p = org.m_p;
        m_p->link();
        return ( *this );
    }

    double eval( FunctionParam* p = 0 ) const
    {
        return ( m_p->eval( p ) );
    };

    double deval( const Function& x, FunctionParam* p = 0 ) const
    {
        return ( m_p->deval( *( x.m_p ), p ) );
    };

    Function d( const Function& x ) const
    {
        return ( Function( m_p->d( *( x.m_p ) ) ) );
    };

    char* str( void ) const
    {
        return ( m_p->str() );
    };

    inline void disable( void )
    {
        m_p->m_disable = 1;
    };

    inline void enable( void )
    {
        m_p->m_disable = 0;
    };

    inline void tag( const char* tag )
    {
        m_p->m_tag = tag;
    };

    inline const char* tag( void )
    {
        return ( m_p->m_tag );
    };

    bool find( const char *name ) const
    {
        return ( m_p->find(name) );
    };
};

typedef Function               Variable;
typedef std::vector<Variable>  Variables;
typedef Function               Parameter;
typedef std::vector<Parameter> Parameters;

} // end of namespace FuncParser

#endif // KVS__FUNCTION_H_INCLUDE
