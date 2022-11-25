/****************************************************************************/
/**
 *  @file Tree.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Tree.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__TREE_H_INCLUDE
#define KVS__TREE_H_INCLUDE

#include <cassert>
#include <memory>
#include <stdexcept>
#include <iterator>
#include <set>
#include <queue>
#include <kvs/ClassName>
#include <kvs/Assert>
#include <kvs/Message>
#include <kvs/Compiler>


namespace kvs
{

/*==========================================================================*/
/**
 *  Tree class.
 */
/*==========================================================================*/
template <typename T>
class Tree
{
    kvsClassName( kvs::Tree );

public:

    struct Node;

    class iterator_base;
    class pre_order_iterator;
    class post_order_iterator;
    class sibling_iterator;
    class breadth_first_iterator;
    class leaf_iterator;

public:

    // Type definitions.
    typedef Tree<T>                   tree_type;
    typedef Node                      node_type;
    typedef T                         value_type;
    typedef std::allocator<node_type> allocator_type;
    typedef pre_order_iterator        iterator;

protected:

    node_type*     m_head;      ///< head node
    allocator_type m_allocator; ///< allocator

public:

    Tree( void )
    {
        this->initialize_head();
    }

    Tree( const T& t )
    {
        this->initialize_head();
        this->set_head( t );
    }

    virtual ~Tree( void )
    {
        this->clear();
        m_allocator.deallocate( m_head, 1 );
    }

public:

    pre_order_iterator begin( void ) const
    {
        return( pre_order_iterator( m_head->next_sibling ) );
    }

    pre_order_iterator end( void ) const
    {
        return( pre_order_iterator( m_head ) );
    }

    post_order_iterator beginPost( void ) const
    {
        node_type* tmp = m_head->next_sibling;

        if( tmp != m_head )
        {
            while( tmp->first_child ) tmp = tmp->first_child;
        }

        return( post_order_iterator( tmp ) );
    }

    post_order_iterator endPost( void ) const
    {
        return( post_order_iterator( m_head ) );
    }

    sibling_iterator begin( const iterator_base& pos ) const
    {
        if( pos.node()->first_child == 0 )
        {
            return( end(pos) );
        }

        return( pos.node()->first_child );
    }

    sibling_iterator end( const iterator_base& pos ) const
    {
        sibling_iterator ret(0);
        ret.m_parent = pos.node();

        return( ret );
    }

    breadth_first_iterator beginBreadth( void ) const
    {
        return( breadth_first_iterator( m_head->next_sibling ) );
    }

    breadth_first_iterator endBreadth( void ) const
    {
#if defined ( KVS_COMPILER_GCC )
#if KVS_COMPILER_VERSION_LESS_OR_EQUAL( 3, 3 )
        breadth_first_iterator e;
        return( e );
#else
        return( breadth_first_iterator() );
#endif
#else
        return( breadth_first_iterator() );
#endif
    }

    leaf_iterator beginLeaf( void ) const
    {
        node_type* tmp = m_head->next_sibling;

        if( tmp != m_head )
        {
            while( tmp->first_child ) tmp = tmp->first_child;
        }

        return( leaf_iterator( tmp ) );
    }

    leaf_iterator endLeaf( void ) const
    {
        return( leaf_iterator( m_head ) );
    }

public:

    void clear( void )
    {
        if( m_head )
        {
            while( m_head->next_sibling != m_head )
            {
                this->erase( pre_order_iterator( m_head->next_sibling ) );
            }
        }
    }

    template <typename Iter>
    Iter erase( Iter it )
    {
        node_type* cur = it.node();

        KVS_ASSERT( cur != m_head );

        Iter ret = it;
        ret.skipChildren();
        ++ret;

        this->eraseChildren( it );
        if( cur->prev_sibling == 0 )
        {
            cur->parent->first_child = cur->next_sibling;
        }
        else
        {
            cur->prev_sibling->next_sibling = cur->next_sibling;
        }

        if( cur->next_sibling == 0 )
        {
            cur->parent->last_child = cur->prev_sibling;
        }
        else
        {
            cur->next_sibling->prev_sibling = cur->prev_sibling;
        }

        this->destructor( &cur->data );
        m_allocator.deallocate( cur, 1 );

        return( ret );
    }

    void eraseChildren( const iterator_base& it )
    {
        node_type* cur  = it.node()->first_child;
        node_type* prev = 0;

        while( cur != 0 )
        {
            prev = cur;
            cur  = cur->next_sibling;

            this->eraseChildren( pre_order_iterator(prev) );
            this->destructor( &prev->data );
            m_allocator.deallocate( prev, 1 );
        }

        it.node()->first_child = 0;
        it.node()->last_child  = 0;
    }

public:

    // insert node as last child of node pointed to by position (first one inserts empty node)
    template <typename Iter>
    Iter appendChild( Iter position )
    {
        KVS_ASSERT( position.node() != m_head );

        node_type* tmp = m_allocator.allocate( 1, 0 );
        if ( !tmp )
        {
            kvsMessageError( "Cannot allocate memory for the appending node." );
            return( 0 );
        }

#if !defined( KVS_ENABLE_MEM_DEBUG )
        this->constructor( &tmp->data );
#endif
        tmp->first_child = 0;
        tmp->last_child  = 0;

        tmp->parent() = position.node;
        if( position.node()->last_child != 0 )
        {
            position.node()->last_child->next_sibling = tmp;
        }
        else
        {
            position.node()->first_child = tmp;
        }
        tmp->prev_sibling = position.node->last_child;
        position.node->last_child = tmp;
        tmp->next_sibling = 0;

        return( tmp );
    }

    template <typename Iter>
    Iter appendChild( Iter position, const T& x )
    {
        KVS_ASSERT( position.node() != m_head );

        node_type* tmp = m_allocator.allocate( 1, 0 );
        if ( !tmp )
        {
            kvsMessageError( "Cannot allocate memory for the appending node." );
            return ( 0 );
        }

#if defined( KVS_ENABLE_MEM_DEBUG )
        tmp->data = x;
#else
        this->constructor( &tmp->data, x );
#endif

        tmp->first_child = 0;
        tmp->last_child  = 0;

        tmp->parent = position.node();
        if( position.node()->last_child != 0 )
        {
            position.node()->last_child->next_sibling = tmp;
        }
        else
        {
            position.node()->first_child = tmp;
        }

        tmp->prev_sibling = position.node()->last_child;
        position.node()->last_child = tmp;
        tmp->next_sibling = 0;

        return( tmp );
    }

public:

    // insert node as previous sibling of node pointed to by position
    template <typename Iter>
    Iter insert( Iter position, const T& x )
    {
        node_type* tmp = m_allocator.allocate( 1, 0 );
        if ( !tmp )
        {
            kvsMessageError( "Cannot allocate memory for the appending node." );
            return ( 0 );
        }

#if defined( KVS_ENABLE_MEM_DEBUG )
        tmp->data = x;
#else
        this->constructor( &tmp->data, x );
#endif
        tmp->first_child = 0;
        tmp->last_child  = 0;

        tmp->parent       = position.node()->parent;
        tmp->next_sibling = position.node();
        tmp->prev_sibling = position.node()->prev_sibling;
        position.node()->prev_sibling = tmp;

        if( tmp->prev_sibling == 0 )
        {
            tmp->parent->first_child = tmp;
        }
        else
        {
            tmp->prev_sibling->next_sibling = tmp;
        }

        return( tmp );
    }

    // modify bt @hira at 2016012/01
#if __clang__ != 1 || __clang_major__ < 7
    // insert node as previous sibling of node pointed to by position
    sibling_iterator insert( sibling_iterator position, const T& x )
    {
        node_type* tmp = m_allocator.allocate( 1, 0 );
        if ( !tmp )
        {
            kvsMessageError( "Cannot allocate memory for the appending node." );
            return ( 0 );
        }

#if defined( KVS_ENABLE_MEM_DEBUG )
        tmp->data = x;
#else
        this->constructor( &tmp->data, x );
#endif
        tmp->first_child = 0;
        tmp->last_child  = 0;

        tmp->next_sibling = position.node();
        if( position.node() == 0 )
        { // iterator points to end of a subtree
            tmp->parent       = position.parent;
            tmp->prev_sibling = position.range_last();
        }
        else
        {
            tmp->parent       = position.node()->parent;
            tmp->prev_sibling = position.node()->prev_sibling;
            position.node()->prev_sibling = tmp;
        }

        if( tmp->prev_sibling == 0 )
        {
            tmp->parent->first_child = tmp;
        }
        else
        {
            tmp->prev_sibling->next_sibling = tmp;
        }

        return( tmp );
    }
#endif

public:

    // count the total number of nodes
    int size( void ) const
    {
        pre_order_iterator it  = begin();
        pre_order_iterator eit = end();

        int i = 0;
        while( it != eit ){ ++i; ++it; }

        return( i );
    }

    // compute the depth to the root
    int depth( const iterator_base& it ) const
    {
        node_type* pos = it.node();

        KVS_ASSERT( pos != 0 );

        int ret = 0;
        while( pos->parent != 0 ){ pos = pos->parent; ++ret; }

        return( ret );
    }

    // count the number of children of node at position
    unsigned int numberOfChildren( const iterator_base& it ) const
    {
        node_type* pos = it.node()->first_child;
        if( pos == 0 ) return( 0 );

        unsigned int ret = 1;
        while( ( pos = pos->next_sibling ) ) ++ret;

        return( ret );
    }

    // count the number of 'next' siblings of node at iterator
    unsigned int numberOfSiblings( const iterator_base& it ) const
    {
        node_type* pos = it.node();

        unsigned int ret = 1;
        while( pos->next_sibling && pos->next_sibling != m_head )
        {
            ++ret;
            pos = pos->next_sibling;
        }

        return( ret );
    }

protected:

    void initialize_head( void )
    {
        m_head = m_allocator.allocate( 1, 0 );
        if ( !m_head )
        {
            kvsMessageError( "Cannot allocate memory for the head node." );
        }

        m_head->parent       = 0;
        m_head->first_child  = 0;
        m_head->last_child   = 0;
        m_head->prev_sibling = m_head;
        m_head->next_sibling = m_head;
    }

    pre_order_iterator set_head( const T& x )
    {
        KVS_ASSERT( begin() == end() );

        return( insert( begin(), x ) );
    }

private:

#if !defined( KVS_ENABLE_MEM_DEBUG )
    template <typename T1, typename T2>
    inline void constructor( T1* p, T2& val )
    {
        new( (void*)p ) T1(val);
    }

    template <typename T1>
    inline void constructor( T1* p )
    {
        new( (void*)p ) T1;
    }
#endif

    template <typename T1>
    inline void destructor( T1* p )
    {
        p->~T1();
    }
};

/*==========================================================================*/
/**
 *  @struct TreeNode
 *  Tree node structure.
 */
/*==========================================================================*/
template <typename T>
struct Tree<T>::Node
{
    Node* parent;       ///< pointer to the parent node
    Node* first_child;  ///< pointer to the first child node
    Node* last_child;   ///< pointer to the last child node
    Node* prev_sibling; ///< pointer to the previous sibling node
    Node* next_sibling; ///< pointer to the next sibling node
    T     data;         ///< node data
};

/*==========================================================================*/
/**
 *  Base iterator class for the tree class.
 */
/*==========================================================================*/
template <typename T>
class Tree<T>::iterator_base
{
    // Friend classes.
    friend class pre_order_iterator;
    friend class post_order_iterator;
    friend class sibling_iterator;
    friend class breadth_first_iterator;
    friend class leaf_iterator;

public:

    // Type definitions.
    typedef T                               value_type;
    typedef T*                              pointer;
    typedef T&                              reference;
    typedef std::size_t                     size_type;
    typedef std::ptrdiff_t                  difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;

protected:

    node_type* m_node;                  ///< pointer to the node
    bool       m_skip_current_children; ///< check flag

public:

    iterator_base( void ):
        m_node( 0 ),
        m_skip_current_children( false )
    {
    }

    iterator_base( node_type* node ):
        m_node( node ),
        m_skip_current_children( false )
    {
    }

    virtual ~iterator_base( void )
    {
    }

public:

    virtual iterator_base&  operator ++ ( void ) = 0;
    virtual iterator_base&  operator -- ( void ) = 0;
    virtual iterator_base&  operator += ( unsigned int ) = 0;
    virtual iterator_base&  operator -= ( unsigned int ) = 0;

public:

    reference operator * ( void ) const
    {
        return( m_node->data );
    }

    pointer operator -> ( void ) const
    {
        return( &(m_node->data) );
    }

public:

    typename tree_type::sibling_iterator begin( void ) const
    {
        sibling_iterator ret( m_node->first_child );
        ret.m_parent = m_node;

        return( ret );
    }

    typename tree_type::sibling_iterator end( void ) const
    {
        sibling_iterator ret(0);
        ret.m_parent = m_node;

        return( ret );
    }

    node_type* node( void )
    {
        return( m_node );
    }

    node_type* node( void ) const
    {
        return( m_node );
    }

    void skipChildren( void )
    {
        m_skip_current_children = true;
    }

    unsigned int numberOfChildren( void ) const
    {
        node_type* p = m_node->first_child;
        if( p == 0 ) return( 0 );

        unsigned int counter = 1;
        while( p != m_node->last_child )
        {
            ++counter;
            p = p->next_sibling;
        }

        return( counter );
    }
};

/*==========================================================================*/
/**
 *  Pre-order iterator.
 */
/*==========================================================================*/
template <typename T>
class Tree<T>::pre_order_iterator : public Tree<T>::iterator_base
{
    using iterator_base::m_node;
    using iterator_base::m_skip_current_children;

public:

    pre_order_iterator( void ):
        iterator_base( 0 )
    {
    }

    pre_order_iterator( node_type* node ):
        iterator_base( node )
    {
    }

    pre_order_iterator( const iterator_base& other ):
        iterator_base( other.node() )
    {
    }

    pre_order_iterator( const sibling_iterator& other ):
        iterator_base( other.node() )
    {
        if( m_node == 0 )
        {
            if( other.rangeLast() != 0 )
            {
                m_node = other.rangeLast();
            }
            else
            {
                m_node = other.m_parent;
            }

            iterator_base::skipChildren();
            ++(*this);
        }
    }

public:

    bool operator == ( const pre_order_iterator& other ) const
    {
        if( other.node() == m_node ) return( true );
        else                         return( false );
    }

    bool operator != ( const pre_order_iterator& other) const
    {
        if( other.node() != m_node ) return( true );
        else                         return( false );
    }

public:

    virtual iterator_base& operator ++ ( void )
    {
        KVS_ASSERT( m_node != 0 );

        if( !m_skip_current_children &&
            m_node->first_child != 0 )
        {
            m_node = m_node->first_child;
        }
        else
        {
            m_skip_current_children = false;

            while( m_node->next_sibling == 0 )
            {
                m_node = m_node->parent;
                if( m_node == 0 ) return( *this );
            }
            m_node = m_node->next_sibling;
        }

        return( *this );
    }

    virtual iterator_base& operator -- ( void )
    {
        KVS_ASSERT( m_node != 0 );

        if( m_node->prev_sibling )
        {
            m_node = m_node->prev_sibling;
            while( m_node->last_child )
            {
                m_node = m_node->last_child;
            }
        }
        else
        {
            m_node = m_node->parent;
            if( m_node == 0 ) return( *this );
        }

        return( *this );
    }

    virtual iterator_base& operator += ( unsigned int num )
    {
        while( num > 0 )
        {
            ++(*this);
            --num;
        }

        return( *this );
    }

    virtual iterator_base& operator -= ( unsigned int num )
    {
        while( num > 0 )
        {
            --(*this);
            --num;
        }

        return( *this );
    }
};

/*==========================================================================*/
/**
 *  Post-order iterator.
 */
/*==========================================================================*/
template <typename T>
class Tree<T>::post_order_iterator : public Tree<T>::iterator_base
{
    using iterator_base::m_node;
    using iterator_base::m_skip_current_children;

public:

    post_order_iterator( void ):
        iterator_base(0)
    {
    }

    post_order_iterator( node_type* t ):
        iterator_base(t)
    {
    }

    post_order_iterator( const iterator_base& other ):
        iterator_base(other.node())
    {
    }

    post_order_iterator( const sibling_iterator& other ):
        iterator_base(other.node())
    {
        if( m_node == 0 )
        {
            if( other.range_last() != 0 ) m_node = other.range_last();
            else                          m_node = other.parent;
            iterator_base::skip_children();
            ++(*this);
        }
    }

public:

    bool operator == ( const post_order_iterator& other ) const
    {
        if( other.node() == m_node ) return( true );
        else                         return( false );
    }

    bool operator != ( const post_order_iterator& other ) const
    {
        if( other.node() != m_node ) return( true );
        else                         return( false );
    }

public:

    virtual iterator_base& operator ++ ( void )
    {
        KVS_ASSERT( m_node != 0 );

        if( m_node->next_sibling == 0 )
        {
            m_node = m_node->parent;
        }
        else
        {
            m_node = m_node->next_sibling;
            if( m_skip_current_children )
            {
                m_skip_current_children = false;
            }
            else
            {
                while( m_node->first_child ) m_node = m_node->first_child;
            }
        }

        return( *this );
    }

    virtual iterator_base&  operator--()
    {
        KVS_ASSERT( m_node != 0 );

        if( m_skip_current_children || m_node->last_child == 0 )
        {
            m_skip_current_children = false;

            while( m_node->prev_sibling == 0 ) m_node = m_node->parent;
            m_node = m_node->prev_sibling;
        }
        else
        {
            m_node = m_node->last_child;
        }

        return( *this );
    }

    virtual iterator_base& operator += ( unsigned int num )
    {
        while( num > 0 )
        {
            ++(*this);
            --num;
        }

        return( *this );
    }

    virtual iterator_base& operator -= ( unsigned int num )
    {
        while( num > 0 )
        {
            --(*this);
            --num;
        }

        return( *this );
    }

    void descend_all( void )
    {
        KVS_ASSERT( m_node != 0 );

        while( m_node->first_child ) m_node = m_node->first_child;
    }
};

/*==========================================================================*/
/**
 *  Sibling iterator.
 */
/*==========================================================================*/
template <typename T>
class Tree<T>::sibling_iterator : public Tree<T>::iterator_base
{
    using iterator_base::m_node;
    using iterator_base::m_skip_current_children;

public:

    node_type* m_parent;

public:

    sibling_iterator( void ):
        iterator_base()
    {
    }

    sibling_iterator( node_type* t ):
        iterator_base(t)
    {
        this->set_parent();
    }

    sibling_iterator( const sibling_iterator& other ):
        iterator_base(other),
        m_parent( other.m_parent )
    {
    }

    sibling_iterator( const iterator_base& other ):
        iterator_base(other.node())
    {
        this->set_parent();
    }

    bool operator == ( const sibling_iterator& other ) const
    {
        if( other.node() == m_node ) return( true );
        else                         return( false );
    }

    bool operator != ( const sibling_iterator& other ) const
    {
        if( other.node() != m_node ) return( true );
        else                         return( false );
    }

    virtual iterator_base& operator ++ ( void )
    {
        if( m_node ) m_node = m_node->next_sibling;

        return( *this );
    }

    virtual iterator_base& operator -- ( void )
    {
        if( m_node )
        {
            m_node = m_node->prev_sibling;
        }
        else
        {
            KVS_ASSERT( m_parent != 0 );

            m_node = m_parent->last_child;
        }

        return( *this );
    }

    virtual iterator_base& operator += ( unsigned int num )
    {
        while( num > 0 )
        {
            ++(*this);
            --num;
        }
        return( *this );
    }

    virtual iterator_base& operator -= ( unsigned int num )
    {
        while( num > 0 )
        {
            --(*this);
            --num;
        }
        return( *this );
    }

    node_type* rangeFirst( void ) const
    {
        node_type* tmp = m_parent->first_child;
        return( tmp );
    }

    node_type* rangeLast( void ) const
    {
        return( m_parent->last_child );
    }

private:

    void set_parent( void )
    {
        m_parent = 0;

        if( m_node == 0 )         return;
        if( m_node->parent != 0 ) m_parent = m_node->parent;
    }
};

/*==========================================================================*/
/**
 *  Breadth first iterator.
 */
/*==========================================================================*/
template <typename T>
class Tree<T>::breadth_first_iterator : public Tree<T>::iterator_base
{
    using iterator_base::m_node;

private:

    std::queue<node_type*> m_node_queue;

public:

    breadth_first_iterator( void ):
        iterator_base()
    {
    }

    breadth_first_iterator( node_type* t ):
        iterator_base( t )
    {
        m_node_queue.push( t );
    }

    breadth_first_iterator( const iterator_base& other ):
        iterator_base( other.node() )
    {
        m_node_queue.push( other.node() );
    }

    bool operator == ( const breadth_first_iterator& other ) const
    {
        if( other.node() == m_node ) return( true );
        else                         return( false );
    }

    bool operator != ( const breadth_first_iterator& other ) const
    {
        if( other.node() != m_node ) return( true );
        else                         return( false );
    }

    virtual iterator_base& operator ++ ( void )
    {
        sibling_iterator sibling = this->begin();
        while( sibling != this->end() )
        {
            m_node_queue.push( sibling.node() );
            ++sibling;
        }

        m_node_queue.pop();
        if( m_node_queue.size() > 0 )
        {
            m_node = m_node_queue.front();
        }
        else
        {
            m_node = 0;
        }

        return( *this );
    }

    virtual iterator_base& operator += ( unsigned int num )
    {
        while( num > 0 )
        {
            ++(*this);
            --num;
        }

        return( *this );
    }

private:

    virtual iterator_base&  operator -- ( void )
    {
        return( *this );
    }

    virtual iterator_base&  operator -= ( unsigned int )
    {
        return( *this );
    }
};

/*==========================================================================*/
/**
 *  Leaf iterator.
 */
/*==========================================================================*/
template <typename T>
class Tree<T>::leaf_iterator : public Tree<T>::iterator_base
{
    using iterator_base::m_node;

public:

    leaf_iterator( void ):
        iterator_base( 0 )
    {
    }

    leaf_iterator( node_type* t ):
        iterator_base( t )
    {
    }

    leaf_iterator( const sibling_iterator& other ):
        iterator_base( other.node() )
    {
        if( m_node == 0 )
        {
            if( other.range_last() != 0 )
            {
                m_node = other.range_last();
            }
            else
            {
                m_node = other.m_parent;
            }

            ++(*this);
        }
    }

    leaf_iterator( const iterator_base& other ):
        iterator_base( other.node() )
    {
    }

    bool operator == ( const leaf_iterator& other ) const
    {
        if( other.node() == m_node ) return( true );
        else                         return( false );
    }

    bool operator != ( const leaf_iterator& other ) const
    {
        if( other.node() != m_node ) return( true );
        else                         return( false );
    }


    virtual iterator_base& operator ++ ( void )
    {
        while( m_node->next_sibling == 0 )
        {
            if( m_node->parent == 0 ) return( *this );
            m_node = m_node->parent;
        }

        m_node = m_node->next_sibling;

        while( m_node->last_child )
        {
            m_node = m_node->first_child;
        }

        return( *this );
    }

    virtual iterator_base& operator -- ( void )
    {
        while( m_node->prev_sibling == 0 )
        {
            if( m_node->parent == 0 ) return( *this );
            m_node = m_node->parent;
        }

        m_node = m_node->prev_sibling;

        while( m_node->last_child )
        {
            m_node = m_node->last_child;
        }

        return( *this );
    }

    virtual iterator_base& operator += ( unsigned int num )
    {
        while( num > 0 )
        {
            ++(*this);
            --num;
        }
        return( *this );
    }

    virtual iterator_base& operator -= ( unsigned int num )
    {
        while( num > 0 )
        {
            --(*this);
            --num;
        }
        return( *this );
    }
};

} // end of namespace kvs

#endif // KVS__TREE_H_INCLUDE
