/*****************************************************************************/
/**
 *  @file   TableObject.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TableObject.h 853 2011-07-12 07:47:36Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef KVS__TABLE_OBJECT_H_INCLUDE
#define KVS__TABLE_OBJECT_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/ObjectBase>
#include <kvs/Type>
#include <kvs/AnyValueArray>
#include <vector>


namespace kvs
{

/*===========================================================================*/
/**
 *  TableObject class.
 */
/*===========================================================================*/
class TableObject : public kvs::ObjectBase
{
    // Class name.
    kvsClassName( kvs::TableObject );

    // Module information.
    kvsModuleCategory( Object );
    kvsModuleBaseClass( kvs::ObjectBase );

public:

    typedef std::vector<std::string> LabelList;
    typedef std::vector<kvs::AnyValueArray> ColumnList;
    typedef std::vector<kvs::Real64> ValueList;
    typedef std::vector<kvs::UInt8> RangeList;

protected:

    size_t m_nrows; ///< number of rows
    size_t m_ncolumns; ///< number of columns
    LabelList m_labels; ///< label list
    ColumnList m_columns; ///< column list
    ValueList m_min_values; ///< min. values for each column
    ValueList m_max_values; ///< max. values for each column
    ValueList m_min_ranges; ///< min. value range
    ValueList m_max_ranges; ///< max. value range
    RangeList m_inside_range_list; ///< check flags for value range

public:

    TableObject( void );

    TableObject( const kvs::TableObject& table );

public:

    static kvs::TableObject* DownCast( kvs::ObjectBase* object );

    static const kvs::TableObject* DownCast( const kvs::ObjectBase* object );

public:

    void addColumn( const kvs::AnyValueArray& array, const std::string& label = "" );

    template <typename T>
    void addColumn( const kvs::ValueArray<T>& array, const std::string& label = "" );

    template <typename T>
    void addColumn( const std::vector<T>& array, const std::string& label = "" );

    const size_t ncolumns( void ) const;

    const size_t nrows( void ) const;

    const LabelList labelList( void ) const;

    const std::string label( const size_t index ) const;

    const ColumnList columnList( void ) const;

    const kvs::AnyValueArray& column( const size_t index ) const;

    const ValueList minValueList( void ) const;

    const kvs::Real64 minValue( const size_t index ) const;

    const ValueList maxValueList( void ) const;

    const kvs::Real64 maxValue( const size_t index ) const;

    template <typename T>
    const T& at( const size_t row, const size_t column ) const;

    void setMinValue( const size_t column_index, const kvs::Real64 value );

    void setMaxValue( const size_t column_index, const kvs::Real64 value );

    void setMinRange( const size_t column_index, const kvs::Real64 range );

    void setMaxRange( const size_t column_index, const kvs::Real64 range );

    void setRange( const size_t column_index, const kvs::Real64 min_range, const kvs::Real64 max_range );

    void moveMinRange( const size_t column_index, const kvs::Real64 drange );

    void moveMaxRange( const size_t column_index, const kvs::Real64 drange );

    void moveRange( const size_t column_index, const kvs::Real64 drange );

    void resetRange( const size_t column_index );

    void resetRange( void );

public:

    const ValueList& minRangeList( void ) const;

    const ValueList& maxRangeList( void ) const;

    const RangeList& insideRangeList( void ) const;

    const kvs::Real64 minRange( const size_t column_index ) const;

    const kvs::Real64 maxRange( const size_t column_index ) const;

    const bool insideRange( const size_t row_index ) const;

    const ObjectType objectType( void ) const;
};

/*===========================================================================*/
/**
 *  @brief  Adds a column.
 *  @param  array [in] column array
 *  @param  label [in] column label
 */
/*===========================================================================*/
template <typename T>
inline void TableObject::addColumn( const kvs::ValueArray<T>& array, const std::string& label )
{
    this->addColumn( kvs::AnyValueArray( array ), label ); // Shallow copy.
}

/*===========================================================================*/
/**
 *  @brief  Adds a column.
 *  @param  array [in] column array
 *  @param  label [in] column label
 */
/*===========================================================================*/
template <typename T>
inline void TableObject::addColumn( const std::vector<T>& array, const std::string& label )
{
    this->addColumn( kvs::AnyValueArray( array ), label ); // Deep copy.
}

/*===========================================================================*/
/**
 *  @brief  Returns value specified by indices.
 *  @param  row [in] row index
 *  @param  column [in] column index
 */
/*===========================================================================*/
template <typename T>
inline const T& TableObject::at( const size_t row, const size_t column ) const
{
    return( this->column( column ).template at<T>( row ) );
}

} // end of namespace kvs

#endif // KVS__TABLE_OBJECT_H_INCLUDE
