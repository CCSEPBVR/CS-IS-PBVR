/****************************************************************************/
/**
 *  @file MessageBlock.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MessageBlock.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__MESSAGE_BLOCK_H_INCLUDE
#define KVS__MESSAGE_BLOCK_H_INCLUDE

#include <kvs/ValueArray>
#include <kvs/ClassName>


namespace kvs
{

/*==========================================================================*/
/**
 *  Message block class.
 */
/*==========================================================================*/
class MessageBlock
{
    kvsClassName( kvs::MessageBlock );

protected:

    /*     MessageBlock
     *
     *    --------------    ---
     *   | message_size |    |
     *    --------------     |
     *   |              |
     *   |              | m_block
     *   |   message    |
     *   |              |    |
     *   |              |    |
     *    --------------    ---
     */

    kvs::ValueArray<unsigned char> m_block; ///< message block

public:

    MessageBlock( void );

    MessageBlock( const void* data, const size_t data_size );

    MessageBlock( const std::string& data );

    template<typename T>
    MessageBlock( const std::vector<T>& data );

    virtual ~MessageBlock( void );

public:

    size_t size( void ) const;

    void* pointer( void );

    const void* pointer( void ) const;

    size_t blockSize( void ) const;

    void* blockPointer( void );

    const void* blockPointer( void ) const;

    std::string toString( void ) const;

public:

    void copy( const void* data, const size_t data_size );

    void copy( const std::string& data );

    template <typename T>
    void copy( const std::vector<T>& data );

    void* allocate( const size_t data_size );

    void deallocate( void );

};

} // end of namespace kvs

#endif // KVS__MESSAGE_BLOCK_H_INCLUDE
