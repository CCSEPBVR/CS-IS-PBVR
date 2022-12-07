/*****************************************************************************/
/**
 *  @file   ValueTag.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ValueTag.h 848 2011-06-29 11:35:52Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__KVSML__VALUE_TAG_H_INCLUDE
#define KVS__KVSML__VALUE_TAG_H_INCLUDE

#include <string>
#include <kvs/XMLNode>
#include "TagBase.h"


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Tag class for <Value>
 */
/*===========================================================================*/
class ValueTag : public kvs::kvsml::TagBase
{
public:

    typedef kvs::kvsml::TagBase BaseClass;

protected:

    bool m_has_label; ///< flag to check whether 'label' is specified or not
    bool m_has_veclen; ///< flag to check whether 'veclen' is specified or not
    bool m_has_min_value; ///< flag to check whether 'min_value' is specified or not
    bool m_has_max_value; ///< flag to check whether 'max_value' is specified or not
    std::string m_label; ///< data label
    size_t m_veclen; ///< vector length
    double m_min_value; ///< minimum value
    double m_max_value; ///< maximum value

public:

    ValueTag( void );

    virtual ~ValueTag( void );

public:

    const bool hasLabel( void ) const;

    const bool hasVeclen( void ) const;

    const bool hasMinValue( void ) const;

    const bool hasMaxValue( void ) const;

    const std::string& label( void ) const;

    const size_t veclen( void ) const;

    const double minValue( void ) const;

    const double maxValue( void ) const;

public:

    void setLabel( const std::string& label );

    void setVeclen( const size_t veclen );

    void setMinValue( const double min_value );

    void setMaxValue( const double max_value );

public:

    const bool read( const kvs::XMLNode::SuperClass* parent );

    const bool write( kvs::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace kvs

#endif // KVS__KVSML__VALUE_TAG_H_INCLUDE
