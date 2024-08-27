/*****************************************************************************/
/**
 *  @file   ColumnTag.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ColumnTag.h 846 2011-06-21 07:04:44Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__KVSML__COLUMN_TAG_H_INCLUDE
#define KVS__KVSML__COLUMN_TAG_H_INCLUDE

#include <string>
#include <kvs/XMLNode>
#include <kvs/XMLElement>
#include <Core/FileFormat/KVSML/TagBase.h>


namespace kvs
{

namespace kvsml
{

/*===========================================================================*/
/**
 *  @brief  Column tag class.
 */
/*===========================================================================*/
class ColumnTag : public kvs::kvsml::TagBase
{
public:

    typedef kvs::kvsml::TagBase BaseClass;

protected:

    bool m_has_label; ///< flag to check whether the label is spcified or not
    bool m_has_min_value; ///< flag to check whether the min_value is spcified or not
    bool m_has_max_value; ///< flag to check whether the max_value is spcified or not
    bool m_has_min_range; ///< flag to check whether the min_range is spcified or not
    bool m_has_max_range; ///< flag to check whether the max_range is spcified or not
    std::string m_label; ///< column label
    double m_min_value; ///< min. value
    double m_max_value; ///< max. value
    double m_min_range; ///< min. range
    double m_max_range; ///< max. range

public:

    ColumnTag( void );

public:

    const bool hasLabel( void ) const;

    const bool hasMinValue( void ) const;

    const bool hasMaxValue( void ) const;

    const bool hasMinRange( void ) const;

    const bool hasMaxRange( void ) const;

    const std::string& label( void ) const;

    const double minValue( void ) const;

    const double maxValue( void ) const;

    const double minRange( void ) const;

    const double maxRange( void ) const;

    void setLabel( const std::string& label );

    void setMinValue( const double value );

    void setMaxValue( const double value );

    void setMinRange( const double range );

    void setMaxRange( const double range );

public:

    const bool read( const kvs::XMLNode::SuperClass* parent );

    const bool read( const kvs::XMLElement::SuperClass* element );

    const bool write( kvs::XMLNode::SuperClass* parent );
};

} // end of namespace kvsml

} // end of namespace kvs

#endif // KVS__KVSML__COLUMN_TAG_H_INCLUDE
