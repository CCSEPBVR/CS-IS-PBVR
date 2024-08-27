/*****************************************************************************/
/**
 *  @file   function_op.h
 *  @brief  Function's operation class.
 *
 *  @author Naohisa Sakamoto
 *  @date   2006/12/16 16:18:12
 */
/*----------------------------------------------------------------------------
 *
 *  $Author: akira $
 *  $Date: 2007/10/28 18:31:01 $
 *  $Source: /home/Repository/viz-server2/cvsroot/KVS/Library/Numeric/function_op.h,v $
 *  $Revision: 1.6 $
 */
/*****************************************************************************/
#ifndef KVS__FUNCTION_OP_H_INCLUDE
#define KVS__FUNCTION_OP_H_INCLUDE

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cmath>


#define METHOD_EVAL_START(p) \
    if( disable() ) return( ( p && m_uv ) ? m_uv->v(p) : m_v )

#define METHOD_DEVAL_START(x,p) \
    if( disable() )     return( 0.0 );\
    if( x.id() == id()) return( 1.0 )

#define METHOD_D_START(x) \
    if( disable() )      return( new FunctionCore_opConst( 0.0 ) ); \
    if( x.id() == id() ) return( new FunctionCore_opConst( 1.0 ) )

#define METHOD_STR_START(s) \
    if( disable() ) return( s << m_v, strdup( s.str() ) )

#define FUNCTION_OP_ADD_LEVEL 100
#define FUNCTION_OP_SUB_LEVEL 100
#define FUNCTION_OP_MUL_LEVEL 10
#define FUNCTION_OP_DIV_LEVEL 10

namespace FuncParser
{

/*===========================================================================*/
/**
 *  @brief
 */
/*===========================================================================*/
class FunctionCore_opAdd: public FunctionCore
{
private:

    const FunctionCore* m_p1;
    const FunctionCore* m_p2;

private:

    FunctionCore_opAdd( void );

    FunctionCore_opAdd( const FunctionCore_opAdd& org ):
        FunctionCore( org )
    {
        m_p1 = org.m_p1;
        ( ( FunctionCounter* )m_p1 )->link();
        m_p2 = org.m_p2;
        ( ( FunctionCounter* )m_p2 )->link();
    };

private:

    double eval( FunctionParam* p ) const
    {
        METHOD_EVAL_START( p );
        if ( m_p1 == m_p2 ) return ( 2.0 * m_p1->eval( p ) );
        return ( m_p1->eval( p ) + m_p2->eval( p ) );
    };

    double deval( const FunctionCore& x, FunctionParam* p ) const
    {
        METHOD_DEVAL_START( x, p );
        // d(F + G)/dx =  f + g
        if ( m_p1 == m_p2 ) return ( 2.0 * m_p1->deval( x, p ) );
        return ( m_p1->deval( x, p ) + m_p2->deval( x, p ) );
    };

    FunctionCore* d( const FunctionCore& x ) const
    {
        METHOD_D_START( x );
        if ( m_p1 == m_p2 )
        {
            FunctionCore* m_p = m_p1->d( x );
            return ( new FunctionCore_opAdd( *m_p, *m_p ) );
        }
        return ( new FunctionCore_opAdd( *( m_p1->d( x ) ), *( m_p2->d( x ) ) ) );
    };

    char* str( void ) const
    {
        str_stream s;
        METHOD_STR_START( s );
        char* p1 = m_p1->str();
        char* p2 = m_p2->str();
#if SMALL_STRING
        if ( ( strcmp( p1, "0" ) == 0 ) && ( strcmp( p2, "0" ) == 0 ) )
            s << 0;
        else if ( strcmp( p1, "0" ) == 0 )
            s << p2;
        else if ( strcmp( p2, "0" ) == 0 )
            s << p1;
        else
            s << p1 << " + " << p2;
#else
        s << "(" << p1 << " + " << p2 << ")";
#endif
        free( p1 );
        free( p2 );
        return ( strdup( s.str() ) );
    };

    inline void detach( void )
    {
        if ( m_p1 && ( ( FunctionCounter* )m_p1 )->unlink() < 1 ) delete m_p1;
        if ( m_p2 && ( ( FunctionCounter* )m_p2 )->unlink() < 1 ) delete m_p2;
        m_p1 = m_p2 = 0;
    };

    FunctionCore* copy( void ) const
    {
        return ( new FunctionCore_opAdd( *this ) );
    };

    int level( void ) const
    {
        if ( disable() ) return ( 0 );

        int lev = FUNCTION_OP_ADD_LEVEL;
        char* p1 = m_p1->str();
        char* p2 = m_p2->str();

        if ( ( strcmp( p1, "0" ) == 0 ) && ( strcmp( p2, "0" ) == 0 ) )
            lev = 0;
        else if ( strcmp( p1, "0" ) == 0 )
            lev = m_p2->level();
        else if ( strcmp( p2, "0" ) == 0 )
            lev = m_p1->level();

        free( p1 );
        free( p2 );

        return ( lev );
    };

    bool find( const char *name ) const
    {
        bool p1 = m_p1->find(name);
        bool p2 = m_p2->find(name);
        return (p1 || p2);
    };

public:

    FunctionCore_opAdd( const FunctionCore& a, const FunctionCore& b ):
        FunctionCore( 0 )
    {
        m_p1 = &a;
        ( ( FunctionCounter* )m_p1 )->link();
        m_p2 = &b;
        ( ( FunctionCounter* )m_p2 )->link();
    };

    ~FunctionCore_opAdd( void )
    {
        detach();
    };
};

inline Function operator + ( const Function& a, const Function& b )
{
    return ( new FunctionCore_opAdd( a, b ) );
};

/*===========================================================================*/
/**
 *  @brief
 */
/*===========================================================================*/
class FunctionCore_opSub: public FunctionCore
{
private:

    const FunctionCore* m_p1;
    const FunctionCore* m_p2;

private:

    FunctionCore_opSub( void );

    FunctionCore_opSub( const FunctionCore_opSub& org ):
        FunctionCore( org )
    {
        m_p1 = org.m_p1;
        ( ( FunctionCounter* )m_p1 )->link();
        m_p2 = org.m_p2;
        ( ( FunctionCounter* )m_p2 )->link();
    };

    double eval( FunctionParam* p ) const
    {
        METHOD_EVAL_START( p );
        if ( m_p1 == m_p2 ) return ( 0.0 );
        return ( m_p1->eval( p ) - m_p2->eval( p ) );
    };

    double deval( const FunctionCore& x, FunctionParam* p ) const
    {
        METHOD_DEVAL_START( x, p );
        // d(F - G)/dx =  f - g
        if ( m_p1 == m_p2 ) return ( 0.0 );
        return ( m_p1->deval( x, p ) - m_p2->deval( x, p ) );
    };

    FunctionCore* d( const FunctionCore& x ) const
    {
        METHOD_D_START( x );
        if ( m_p1 == m_p2 )
        {
            FunctionCore* p = m_p1->d( x );
            return ( new FunctionCore_opSub( *p, *p ) );
        }
        return ( new FunctionCore_opSub( *( m_p1->d( x ) ), *( m_p2->d( x ) ) ) );
    };

    char* str( void ) const
    {
        str_stream s;
        METHOD_STR_START( s );
        char* p1 = m_p1->str();
        char* p2 = m_p2->str();
#if SMALL_STRING
        int lev = m_p2->level();
        if ( ( strcmp( p1, "0" ) == 0 ) && ( strcmp( p2, "0" ) == 0 ) )
            s << 0;
        else if ( strcmp( p1, "0" ) == 0 )
        {
            if ( lev > 0 ) s << "-(" << p2 << ")";
            else          s << "-" << p2;
        }
        else if ( strcmp( p2, "0" ) == 0 )
            s << p1;
        else
            s << p1 << " - " << p2;
#else
        s << "(" << p1 << " - " << p2 << ")";
#endif
        free( p1 );
        free( p2 );
        return ( strdup( s.str() ) );
    };

    inline void detach( void )
    {
        if ( m_p1 && ( ( FunctionCounter* )m_p1 )->unlink() < 1 ) delete m_p1;
        if ( m_p2 && ( ( FunctionCounter* )m_p2 )->unlink() < 1 ) delete m_p2;
        m_p1 = m_p2 = 0;
    };

    FunctionCore* copy( void ) const
    {
        return ( new FunctionCore_opSub( *this ) );
    };

    int level( void ) const
    {
        if ( disable() ) return ( 0 );

        int lev = FUNCTION_OP_SUB_LEVEL;
        char* p1 = m_p1->str();
        char* p2 = m_p2->str();

        if ( ( strcmp( p1, "0" ) == 0 ) && ( strcmp( p2, "0" ) == 0 ) )
            lev = 0;
        else if ( strcmp( p1, "0" ) == 0 )
            lev = m_p2->level();
        else if ( strcmp( p2, "0" ) == 0 )
            lev = m_p1->level();

        free( p1 );
        free( p2 );

        return ( lev );
    };


    bool find( const char *name ) const
    {
        bool p1 = m_p1->find(name);
        bool p2 = m_p2->find(name);
        return (p1 || p2);
    };

public:

    FunctionCore_opSub( const FunctionCore& a, const FunctionCore& b ):
        FunctionCore( 0 )
    {
        m_p1 = &a;
        ( ( FunctionCounter* )m_p1 )->link();
        m_p2 = &b;
        ( ( FunctionCounter* )m_p2 )->link();
    };

    ~FunctionCore_opSub( void )
    {
        detach();
    };
};

inline Function operator - ( const Function& a, const Function& b )
{
    return ( new FunctionCore_opSub( a, b ) );
};

/*===========================================================================*/
/**
 *  @brief
 */
/*===========================================================================*/
class FunctionCore_opMul: public FunctionCore
{
private:

    const FunctionCore* m_p1;
    const FunctionCore* m_p2;

private:

    FunctionCore_opMul( void );

    FunctionCore_opMul( const FunctionCore_opMul& org ):
        FunctionCore( org )
    {
        m_p1 = org.m_p1;
        ( ( FunctionCounter* )m_p1 )->link();
        m_p2 = org.m_p2;
        ( ( FunctionCounter* )m_p2 )->link();
    };

private:

    double eval( FunctionParam* p ) const
    {
        METHOD_EVAL_START( p );
        if ( m_p1 == m_p2 )
        {
            register double v = m_p1->eval( p );
            return ( v * v );
        }
        return ( m_p1->eval( p ) * m_p2->eval( p ) );
    };

    double deval( const FunctionCore& x, FunctionParam* p ) const
    {
        METHOD_DEVAL_START( x, p );
        // d(F * G)/dx = F*g + G*f
        if ( m_p1 == m_p2 )
            return ( 2.0 * m_p1->eval( p ) * m_p2->deval( x, p ) );

        return ( m_p1->eval( p )     * m_p2->deval( x, p ) +
                 m_p1->deval( x, p ) * m_p2->eval( p ) );
    };

    FunctionCore* d( const FunctionCore& x ) const
    {
        METHOD_D_START( x );
        if ( m_p1 == m_p2 )
        {
            FunctionCore* p = new FunctionCore_opMul( *m_p1, *( m_p2->d( x ) ) );
            return ( new FunctionCore_opAdd( *p, *p ) );
        }
        return ( new FunctionCore_opAdd( *( new FunctionCore_opMul( *m_p1, *( m_p2->d( x ) ) ) ),
                                         *( new FunctionCore_opMul( *( m_p1->d( x ) ), *m_p2 ) ) ) );
    };

    char* str( void ) const
    {
        str_stream s;
        METHOD_STR_START( s );
        char* p1 = m_p1->str();
        char* p2 = m_p2->str();
#if SMALL_STRING
        int lev1 = m_p1->level();
        int lev2 = m_p2->level();
        if ( ( strcmp( p1, "0" ) == 0 ) || ( strcmp( p2, "0" ) == 0 ) )
            s << 0;
        else if ( strcmp( p1, "1" ) == 0 )
            s << p2;
        else if ( strcmp( p2, "1" ) == 0 )
            s << p1;
        else
        {
            if ( lev1 > FUNCTION_OP_MUL_LEVEL )
                s << "(" << p1 << ")";
            else
                s << p1;
            if ( lev2 > FUNCTION_OP_MUL_LEVEL )
                s << " * (" << p2 << ")";
            else
                s << " * " << p2;
        }
#else
        s << p1 << " * " << p2;
#endif
        free( p1 );
        free( p2 );

        return ( strdup( s.str() ) );
    };

    inline void detach( void )
    {
        if ( m_p1 && ( ( FunctionCounter* )m_p1 )->unlink() < 1 ) delete m_p1;
        if ( m_p2 && ( ( FunctionCounter* )m_p2 )->unlink() < 1 ) delete m_p2;
        m_p1 = m_p2 = 0;
    };

    FunctionCore* copy( void ) const
    {
        return ( new FunctionCore_opMul( *this ) );
    };

    int level( void ) const
    {
        if ( disable() ) return ( 0 );

        int lev = FUNCTION_OP_MUL_LEVEL;
        char* p1 = m_p1->str();
        char* p2 = m_p2->str();

        if ( ( strcmp( p1, "0" ) == 0 ) || ( strcmp( p2, "0" ) == 0 ) )
            lev = 0;
        else if ( strcmp( p1, "1" ) == 0 )
            lev = m_p2->level();
        else if ( strcmp( p2, "1" ) == 0 )
            lev = m_p1->level();

        free( p1 );
        free( p2 );

        return ( lev );
    };

    bool find( const char *name ) const
    {
        bool p1 = m_p1->find(name);
        bool p2 = m_p2->find(name);
        return (p1 || p2);
    };

public:

    FunctionCore_opMul( const FunctionCore& a, const FunctionCore& b ):
        FunctionCore( 0 )
    {
        m_p1 = &a;
        ( ( FunctionCounter* )m_p1 )->link();
        m_p2 = &b;
        ( ( FunctionCounter* )m_p2 )->link();
    };

    ~FunctionCore_opMul( void )
    {
        detach();
    };
};

inline Function operator * ( const Function& a, const Function& b )
{
    return ( new FunctionCore_opMul( a, b ) );
};


/*===========================================================================*/
/**
 *  @brief
 */
/*===========================================================================*/
class FunctionCore_opDiv: public FunctionCore
{
private:

    const FunctionCore* m_p1;
    const FunctionCore* m_p2;

private:

    FunctionCore_opDiv( void );

    FunctionCore_opDiv( const FunctionCore_opDiv& org ):
        FunctionCore( org )
    {
        m_p1 = org.m_p1;
        ( ( FunctionCounter* )m_p1 )->link();
        m_p2 = org.m_p2;
        ( ( FunctionCounter* )m_p2 )->link();
    };

private:

    double eval( FunctionParam* p ) const
    {
        METHOD_EVAL_START( p );
        if ( m_p1 == m_p2 ) return ( 1.0 );
        return ( m_p1->eval( p ) / m_p2->eval( p ) );
    }

    double deval( const FunctionCore& x, FunctionParam* p ) const
    {
        METHOD_DEVAL_START( x, p );
        // d(F/G)/dx = (fG - Fg)/(G*G) = (f - Tg)/G
        if ( m_p1 == m_p2 ) return ( 1.0 );
        return ( ( m_p1->deval( x, p ) - eval( p ) * m_p2->deval( x, p ) )
                 / m_p2->eval( p ) );
    };

    FunctionCore* d( const FunctionCore& x ) const
    {
        METHOD_D_START( x );
        if ( m_p1 == m_p2 ) return ( new FunctionCore_opConst( 1.0 ) );
        const FunctionCore* Tg = new FunctionCore_opMul( *this, *( m_p2->d( x ) ) );
        const FunctionCore* f_Tg = new FunctionCore_opSub( *( m_p1->d( x ) ), *Tg );
        return ( new FunctionCore_opDiv( *f_Tg, *m_p2 ) );
    };

    char* str( void ) const
    {
        str_stream s;
        METHOD_STR_START( s );
        char* p1 = m_p1->str();
        char* p2 = m_p2->str();
#if SMALL_STRING
        int lev1 = m_p1->level();
        int lev2 = m_p2->level();
        if ( strcmp( p1, "0" ) == 0 )
            s << 0;
        else
        {
            if ( lev1 > FUNCTION_OP_DIV_LEVEL )
                s << "(" << p1 << ")";
            else
                s << p1;
            if ( lev2 > 0 )
                s << " / (" << p2 << ")";
            else
                s << " / " << p2;
        }
#else
        s << p1 << " / (" << p2 << ")";
#endif
        free( p1 );
        free( p2 );
        return ( strdup( s.str() ) );
    };

    inline void detach( void )
    {
        if ( m_p1 && ( ( FunctionCounter* )m_p1 )->unlink() < 1 ) delete m_p1;
        if ( m_p2 && ( ( FunctionCounter* )m_p2 )->unlink() < 1 ) delete m_p2;
        m_p1 = m_p2 = 0;
    };

    FunctionCore* copy( void ) const
    {
        return ( new FunctionCore_opDiv( *this ) );
    };

    int level( void ) const
    {
        if ( disable() ) return ( 0 );

        int lev = FUNCTION_OP_DIV_LEVEL;
        char* p1 = m_p1->str();
        if ( strcmp( p1, "0" ) == 0 ) lev = 0;
        free( p1 );

        return ( lev );
    };

    bool find( const char *name ) const
    {
        bool p1 = m_p1->find(name);
        bool p2 = m_p2->find(name);
        return (p1 || p2);
    };

public:

    FunctionCore_opDiv( const FunctionCore& a, const FunctionCore& b ):
        FunctionCore( 0 )
    {
        m_p1 = &a;
        ( ( FunctionCounter* )m_p1 )->link();
        m_p2 = &b;
        ( ( FunctionCounter* )m_p2 )->link();
    };

    ~FunctionCore_opDiv( void )
    {
        detach();
    };
};

inline Function operator / ( const Function& a, const Function& b )
{
    return ( new FunctionCore_opDiv( a, b ) );
};

/*===========================================================================*/
/**
 *  @brief
 */
/*===========================================================================*/
class FunctionCore_opExp: public FunctionCore
{
private:

    const FunctionCore* m_p;

private:

    FunctionCore_opExp( void );

    FunctionCore_opExp( const FunctionCore_opExp& org ):
        FunctionCore( org )
    {
        m_p = org.m_p;
        ( ( FunctionCounter* )m_p )->link();
    };

    double eval( FunctionParam* p ) const
    {
        METHOD_EVAL_START( p );
        return ( exp( m_p->eval( p ) ) );
    };

    double deval( const FunctionCore& x, FunctionParam* p ) const
    {
        METHOD_DEVAL_START( x, p );
        // d(exp(F))/dx = exp(F)*f
        return ( exp( m_p->eval( p ) ) * m_p->deval( x, p ) );
    };

    FunctionCore* d( const FunctionCore& x ) const
    {
        METHOD_D_START( x );
        return ( new FunctionCore_opMul( *( new FunctionCore_opExp( *m_p ) ), *( m_p->d( x ) ) ) );
    };

    char* str( void ) const
    {
        str_stream s;
        METHOD_STR_START( s );
        char* p = m_p->str();
        s << "exp(" << p << ")";
        free( p );
        return ( strdup( s.str() ) );
    };

    inline void detach( void )
    {
        if ( m_p && ( ( FunctionCounter* )m_p )->unlink() < 1 ) delete m_p;
        m_p = 0;
    };

    FunctionCore* copy( void ) const
    {
        return ( new FunctionCore_opExp( *this ) );
    };

    bool find( const char *name ) const
    {
        bool p1 = m_p->find(name);
        return (p1);
    };

public:

    FunctionCore_opExp( const FunctionCore& a ):
        FunctionCore( 0 )
    {
        m_p = &a;
        ( ( FunctionCounter* )m_p )->link();
    };

    ~FunctionCore_opExp( void )
    {
        detach();
    };
};

inline Function Exp( const Function& a )
{
    return ( new FunctionCore_opExp( a ) );
};


/*===========================================================================*/
/**
 *  @brief
 */
/*===========================================================================*/
class FunctionCore_opLog: public FunctionCore
{
private:

    const FunctionCore* m_p;

private:

    FunctionCore_opLog( void );

    FunctionCore_opLog( const FunctionCore_opLog& org ):
        FunctionCore( org )
    {
        m_p = org.m_p;
        ( ( FunctionCounter* )m_p )->link();
    };

private:

    double eval( FunctionParam* p ) const
    {
        METHOD_EVAL_START( p );
        return ( std::log( m_p->eval( p ) ) );
    };

    double deval( const FunctionCore& x, FunctionParam* p ) const
    {
        METHOD_DEVAL_START( x, p );
        // d(log(F))/dx = f/F
        return ( m_p->deval( x, p ) / m_p->eval( p ) );
    };

    FunctionCore* d( const FunctionCore& x ) const
    {
        METHOD_D_START( x );
        return ( new FunctionCore_opDiv( *( m_p->d( x ) ), *m_p ) );
    };

    char* str( void ) const
    {
        str_stream s;
        METHOD_STR_START( s );
        char* p = m_p->str();
        s << "log(" << p << ")";
        free( p );
        return ( strdup( s.str() ) );
    };

    inline void detach( void )
    {
        if ( m_p && ( ( FunctionCounter* )m_p )->unlink() < 1 ) delete m_p;
        m_p = 0;
    };

    FunctionCore* copy( void ) const
    {
        return ( new FunctionCore_opLog( *this ) );
    };

    bool find( const char *name ) const
    {
        bool p1 = m_p->find(name);
        return (p1);
    };

public:

    FunctionCore_opLog( const FunctionCore& a ):
        FunctionCore( 0 )
    {
        m_p = &a;
        ( ( FunctionCounter* )m_p )->link();
    };

    ~FunctionCore_opLog( void )
    {
        detach();
    };
};

inline Function Log( const Function& a )
{
    return ( new FunctionCore_opLog( a ) );
};

/*===========================================================================*/
/**
 *  @brief
 */
/*===========================================================================*/
class FunctionCore_opLog10: public FunctionCore
{
private:

    const FunctionCore* m_p;

private:

    FunctionCore_opLog10( void );

    FunctionCore_opLog10( const FunctionCore_opLog10& org ):
        FunctionCore( org )
    {
        m_p = org.m_p;
        ( ( FunctionCounter* )m_p )->link();
    };

private:

    double eval( FunctionParam* p ) const
    {
        METHOD_EVAL_START( p );
        return ( std::log10( m_p->eval( p ) ) );
    };

    double deval( const FunctionCore& x, FunctionParam* p ) const
    {
        METHOD_DEVAL_START( x, p );
        // d(log10(F))/dx =  (f/F) / log(10.0)
        return ( m_p->deval( x, p ) / m_p->eval( p ) / std::log( 10.0 ) );
    };

    FunctionCore* d( const FunctionCore& x ) const
    {
        METHOD_D_START( x );
        return ( new FunctionCore_opDiv( *( new FunctionCore_opDiv( *( m_p->d( x ) ), *m_p ) ),
                                         *( new FunctionCore_opConst( std::log( 10.0 ) ) ) ) );
    };

    char* str( void ) const
    {
        str_stream s;
        METHOD_STR_START( s );
        char* p = m_p->str();
        s << "log10(" << p << ")";
        free( p );
        return ( strdup( s.str() ) );
    };

    inline void detach( void )
    {
        if ( m_p && ( ( FunctionCounter* )m_p )->unlink() < 1 ) delete m_p;
        m_p = 0;
    };

    FunctionCore* copy( void ) const
    {
        return ( new FunctionCore_opLog10( *this ) );
    };

    bool find( const char *name ) const
    {
        bool p1 = m_p->find(name);
        return (p1);
    };

public:

    FunctionCore_opLog10( const FunctionCore& a ):
        FunctionCore( 0 )
    {
        m_p = &a;
        ( ( FunctionCounter* )m_p )->link();
    };

    ~FunctionCore_opLog10( void )
    {
        detach();
    };
};

inline Function Log10( const Function& a )
{
    return ( new FunctionCore_opLog10( a ) );
};


/*===========================================================================*/
/**
 *  @brief
 */
/*===========================================================================*/
class FunctionCore_opSqrt : public FunctionCore
{
private:

    const FunctionCore* m_p;

private:

    FunctionCore_opSqrt( void );

    FunctionCore_opSqrt( const FunctionCore_opSqrt& org ):
        FunctionCore( org )
    {
        m_p = org.m_p;
        ( ( FunctionCounter* )m_p )->link();
    };

private:

    double eval( FunctionParam* p ) const
    {
        METHOD_EVAL_START( p );
        return ( sqrt( m_p->eval( p ) ) );
    };

    double deval( const FunctionCore& x, FunctionParam* p ) const
    {
        METHOD_DEVAL_START( x, p );
        // d(sqrt(F))/dx = T(f/2F)
        return ( eval( p ) * ( m_p->deval( x, p ) / ( 2.0 * m_p->eval( p ) ) ) );
    };

    FunctionCore* d( const FunctionCore& x ) const
    {
        METHOD_D_START( x );
        const FunctionCore* F2 = new FunctionCore_opMul( *( new FunctionCore_opConst( 2.0 ) ), *m_p );
        const FunctionCore* f_2F = new FunctionCore_opDiv( *( m_p->d( x ) ), *F2 );
        return ( new FunctionCore_opMul( *this, *f_2F ) );
    };

    char* str( void ) const
    {
        str_stream s;
        METHOD_STR_START( s );
        char* p = m_p->str();
        s << "sqrt(" << p << ")";
        free( p );
        return ( strdup( s.str() ) );
    };

    inline void detach( void )
    {
        if ( m_p && ( ( FunctionCounter* )m_p )->unlink() < 1 ) delete m_p;
        m_p = 0;
    };

    FunctionCore* copy( void ) const
    {
        return ( new FunctionCore_opSqrt( *this ) );
    };

    bool find( const char *name ) const
    {
        bool p1 = m_p->find(name);
        return (p1);
    };

public:

    FunctionCore_opSqrt( const FunctionCore& a ):
        FunctionCore( 0 )
    {
        m_p = &a;
        ( ( FunctionCounter* )m_p )->link();
    };

    ~FunctionCore_opSqrt( void )
    {
        detach();
    };
};

inline Function Sqrt( const Function& a )
{
    return ( new FunctionCore_opSqrt( a ) );
}

/*===========================================================================*/
/**
 *  @brief
 */
/*===========================================================================*/
class FunctionCore_opSin: public FunctionCore
{
private:

    const FunctionCore* m_p;

private:

    FunctionCore_opSin( void );

    FunctionCore_opSin( const FunctionCore_opSin& org ):
        FunctionCore( org )
    {
        m_p = org.m_p;
        ( ( FunctionCounter* )m_p )->link();
    };

private:

    double eval( FunctionParam* p ) const
    {
        METHOD_EVAL_START( p );
        return ( std::sin( m_p->eval( p ) ) );
    };

    double deval( const FunctionCore& x, FunctionParam* p ) const
    {
        METHOD_DEVAL_START( x, p );
        // d(sin(F))/dx = cos(F) * f
        return ( std::cos( m_p->eval( p ) ) * m_p->deval( x, p ) );
    };

    FunctionCore* d( const FunctionCore& x ) const;

    char* str( void ) const
    {
        str_stream s;
        METHOD_STR_START( s );
        char* p = m_p->str();
        s << "sin(" << p << ")";
        free( p );
        return ( strdup( s.str() ) );
    };

    inline void detach( void )
    {
        if ( m_p && ( ( FunctionCounter* )m_p )->unlink() < 1 ) delete m_p;
        m_p = 0;
    };

    FunctionCore* copy( void ) const
    {
        return ( new FunctionCore_opSin( *this ) );
    };

    bool find( const char *name ) const
    {
        bool p1 = m_p->find(name);
        return (p1);
    };

public:

    FunctionCore_opSin( const FunctionCore& a ):
        FunctionCore( 0 )
    {
        m_p = &a;
        ( ( FunctionCounter* )m_p )->link();
    };

    ~FunctionCore_opSin( void )
    {
        detach();
    };
};

/*===========================================================================*/
/**
 *  @brief
 */
/*===========================================================================*/
class FunctionCore_opCos: public FunctionCore
{
private:

    const FunctionCore* m_p;

private:

    FunctionCore_opCos( void );

    FunctionCore_opCos( const FunctionCore_opCos& org ):
        FunctionCore( org )
    {
        m_p = org.m_p;
        ( ( FunctionCounter* )m_p )->link();
    };

    double eval( FunctionParam* p ) const
    {
        METHOD_EVAL_START( p );
        return ( std::cos( m_p->eval( p ) ) );
    };

    double deval( const FunctionCore& x, FunctionParam* p ) const
    {
        METHOD_DEVAL_START( x, p );
        // d(cos(F))/dx = -sin(F) * f
        return ( -std::sin( m_p->eval( p ) ) * m_p->deval( x, p ) );
    };

    FunctionCore* d( const FunctionCore& x ) const;

    char* str( void ) const
    {
        str_stream s;
        METHOD_STR_START( s );
        char* p = m_p->str();
        s << "cos(" << p << ")";
        free( p );
        return ( strdup( s.str() ) );
    };

    inline void detach( void )
    {
        if ( m_p && ( ( FunctionCounter* )m_p )->unlink() < 1 ) delete m_p;
        m_p = 0;
    };

    FunctionCore* copy( void ) const
    {
        return ( new FunctionCore_opCos( *this ) );
    };

    bool find( const char *name ) const
    {
        bool p1 = m_p->find(name);
        return (p1);
    };

public:

    FunctionCore_opCos( const FunctionCore& a ):
        FunctionCore( 0 )
    {
        m_p = &a;
        ( ( FunctionCounter* )m_p )->link();
    };

    ~FunctionCore_opCos( void )
    {
        detach();
    };
};

inline FunctionCore* FunctionCore_opSin::d( const FunctionCore& x ) const
{
    METHOD_D_START( x );
    // d(sin(F))/dx = cos(F) * f
    return ( new FunctionCore_opMul( *( new FunctionCore_opCos( *m_p ) ), *( m_p->d( x ) ) ) ) ;
}

inline FunctionCore* FunctionCore_opCos::d( const FunctionCore& x ) const
{
    METHOD_D_START( x );
    // d(cos(F))/dx = -sin(F) * f
    const FunctionCore* _1 = new FunctionCore_opConst( -1.0 );
    const FunctionCore* _sinF = new FunctionCore_opMul( *_1, *( new FunctionCore_opSin( *m_p ) ) );

    return ( new FunctionCore_opMul( *_sinF, *( m_p->d( x ) ) ) );
}

inline Function Sin( const Function& a )
{
    return ( new FunctionCore_opSin( a ) );
}

inline Function Cos( const Function& a )
{
    return ( new FunctionCore_opCos( a ) );
}

/*===========================================================================*/
/**
 *  @brief
 */
/*===========================================================================*/
class FunctionCore_opTan: public FunctionCore
{
private:

    const FunctionCore* m_p;

private:

    FunctionCore_opTan( void );

    FunctionCore_opTan( const FunctionCore_opTan& org ):
        FunctionCore( org )
    {
        m_p = org.m_p;
        ( ( FunctionCounter* )m_p )->link();
    };

private:

    double eval( FunctionParam* p ) const
    {
        METHOD_EVAL_START( p );
        return ( std::tan( m_p->eval( p ) ) );
    };

    double deval( const FunctionCore& x, FunctionParam* p ) const
    {
        METHOD_DEVAL_START( x, p );
        // d(tan(F))/dx = sec(F) * sec(F) * f = f / (cos(F) * cos(F))
        register double v = std::cos( m_p->eval( p ) );
        return ( m_p->deval( x, p ) / ( v * v ) );
    };

    FunctionCore* d( const FunctionCore& x ) const
    {
        METHOD_D_START( x );
        const FunctionCore* c  = new FunctionCore_opCos( *m_p );    // cos(F)
        const FunctionCore* c2 = new FunctionCore_opMul( *c, *c ); // cos(F) * cos(F)
        return ( new FunctionCore_opDiv( *( m_p->d( x ) ), *c2 ) ); // f / (cos(F) * cos(F))
    };

    char* str( void ) const
    {
        str_stream s;
        METHOD_STR_START( s );
        char* p = m_p->str();
        s << "tan(" << p << ")";
        free( p );
        return ( strdup( s.str() ) );
    };

    inline void detach( void )
    {
        if ( m_p && ( ( FunctionCounter* )m_p )->unlink() < 1 ) delete m_p;
        m_p = 0;
    };

    FunctionCore* copy( void ) const
    {
        return ( new FunctionCore_opTan( *this ) );
    };

    bool find( const char *name ) const
    {
        bool p1 = m_p->find(name);
        return (p1);
    };

public:

    FunctionCore_opTan( const FunctionCore& a ):
        FunctionCore( 0 )
    {
        m_p = &a;
        ( ( FunctionCounter* )m_p )->link();
    };

    ~FunctionCore_opTan( void )
    {
        detach();
    };
};

inline Function Tan( const Function& a )
{
    return ( new FunctionCore_opTan( a ) );
}

/*===========================================================================*/
/**
 *  @brief
 */
/*===========================================================================*/
class FunctionCore_opPow: public FunctionCore
{
private:

    const FunctionCore* m_p1;
    const FunctionCore* m_p2;

private:

    FunctionCore_opPow( void );

    FunctionCore_opPow( const FunctionCore_opPow& org ):
        FunctionCore( org )
    {
        m_p1 = org.m_p1;
        ( ( FunctionCounter* )m_p1 )->link();
        m_p2 = org.m_p2;
        ( ( FunctionCounter* )m_p2 )->link();
    };

    double eval( FunctionParam* p ) const
    {
        METHOD_EVAL_START( p );
        return ( std::pow( m_p1->eval( p ), m_p2->eval( p ) ) );
    };

    double deval( const FunctionCore& x, FunctionParam* p ) const
    {
        METHOD_DEVAL_START( x, p );
        // d(pow(F,G))/dx = T(G*f/F + g*logF)
        register double v = m_p1->eval( p ); // = F
        return ( eval( p ) * ( m_p2->eval( p ) * m_p1->deval( x, p ) / v +
                               m_p2->deval( x, p ) * std::log( v ) ) );
    };

    FunctionCore* d( const FunctionCore& x ) const
    {
        METHOD_D_START( x );
        const FunctionCore* T = this;
        const FunctionCore* F = m_p1;
        const FunctionCore* f = m_p1->d( x );
        const FunctionCore* G = m_p2;
        const FunctionCore* g = m_p2->d( x );

        const FunctionCore* glogF = new FunctionCore_opMul( *g,    *( new FunctionCore_opLog( *F ) ) );
        const FunctionCore* Gf    = new FunctionCore_opMul( *G,    *f );
        const FunctionCore* Gf_F  = new FunctionCore_opDiv( *Gf,   *F );
        const FunctionCore* tmp   = new FunctionCore_opAdd( *Gf_F, *glogF );
        return ( new FunctionCore_opMul( *T, *tmp ) );
    };

    char* str( void ) const
    {
        str_stream s;
        METHOD_STR_START( s );
        char* p1 = m_p1->str();
        char* p2 = m_p2->str();
        s << "pow(" << p1 << ", " << p2 << ")";
        free( p1 );
        free( p2 );
        return ( strdup( s.str() ) );
    };

    inline void detach( void )
    {
        if ( m_p1 && ( ( FunctionCounter* )m_p1 )->unlink() < 1 ) delete m_p1;
        if ( m_p2 && ( ( FunctionCounter* )m_p2 )->unlink() < 1 ) delete m_p2;
        m_p1 = m_p2 = 0;
    };

    FunctionCore* copy( void ) const
    {
        return ( new FunctionCore_opPow( *this ) );
    };

    bool find( const char *name ) const
    {
        bool p1 = m_p1->find(name);
        bool p2 = m_p2->find(name);
        return (p1 || p2);
    };

public:

    FunctionCore_opPow( const FunctionCore& a, const FunctionCore& b ):
        FunctionCore( 0 )
    {
        m_p1 = &a;
        ( ( FunctionCounter* )m_p1 )->link();
        m_p2 = &b;
        ( ( FunctionCounter* )m_p2 )->link();
    };

    ~FunctionCore_opPow( void )
    {
        detach();
    };
};

inline Function Pow( const Function& a, const Function& b )
{
    return ( new FunctionCore_opPow( a, b ) );
}

} // end of namespace FuncParser

#endif // KVS__FUNCTION_OP_H_INCLUDE
