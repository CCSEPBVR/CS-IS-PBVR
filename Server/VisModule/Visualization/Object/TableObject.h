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
#ifndef VIS_MODULE__TABLE_OBJECT_H_INCLUDE
#define VIS_MODULE__TABLE_OBJECT_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/ObjectBase>
#include <vismodule/Type>
#include <vismodule/AnyValueArray>
#include <vector>


namespace vismodule
{

/*===========================================================================*/
/**
 *  TableObject class.
 */
/*===========================================================================*/
class TableObject : public vismodule::ObjectBase
{
    // Class name.
    visModuleClassName( vismodule::TableObject );

    // Module information.
    visModuleCategory( Object );
    visModuleBaseClass( vismodule::ObjectBase );

public:

    typedef std::vector<std::string> LabelList;
    typedef std::vector<vismodule::AnyValueArray> ColumnList;
    typedef std::vector<vismodule::Real64> ValueList;
    typedef std::vector<vismodule::UInt8> RangeList;

protected:

    std::size_t m_nrows; ///< number of rows
    std::size_t m_ncolumns; ///< number of columns
    LabelList m_labels; ///< label list
    ColumnList m_columns; ///< column list
    ValueList m_min_values; ///< min. values for each column
    ValueList m_max_values; ///< max. values for each column
    ValueList m_min_ranges; ///< min. value range
    ValueList m_max_ranges; ///< max. value range
    RangeList m_inside_range_list; ///< check flags for value range

public:

    TableObject( void );

    TableObject( const vismodule::TableObject& table );

public:

    static vismodule::TableObject* DownCast( vismodule::ObjectBase* object );

    static const vismodule::TableObject* DownCast( const vismodule::ObjectBase& object );

public:

    void addColumn( const vismodule::AnyValueArray& array, const std::string& label = "" );

    template <typename T>
    void addColumn( const vismodule::ValueArray<T>& array, const std::string& label = "" );

    template <typename T>
    void addColumn( const std::vector<T>& array, const std::string& label = "" );

    const std::size_t ncolumns( void ) const;

    const std::size_t nrows( void ) const;

    const LabelList labelList( void ) const;

    const std::string label( const std::size_t index ) const;

    const ColumnList columnList( void ) const;

    const vismodule::AnyValueArray& column( const std::size_t index ) const;

    const ValueList minValueList( void ) const;

    const vismodule::Real64 minValue( const std::size_t index ) const;

    const ValueList maxValueList( void ) const;

    const vismodule::Real64 maxValue( const std::size_t index ) const;

    template <typename T>
    const T& at( const std::size_t row, const std::size_t column ) const;

    void setMinValue( const std::size_t column_index, const vismodule::Real64 value );

    void setMaxValue( const std::size_t column_index, const vismodule::Real64 value );

    void setMinRange( const std::size_t column_index, const vismodule::Real64 range );

    void setMaxRange( const std::size_t column_index, const vismodule::Real64 range );

    void setRange( const std::size_t column_index, const vismodule::Real64 min_range, const vismodule::Real64 max_range );

    void moveMinRange( const std::size_t column_index, const vismodule::Real64 drange );

    void moveMaxRange( const std::size_t column_index, const vismodule::Real64 drange );

    void moveRange( const std::size_t column_index, const vismodule::Real64 drange );

    void resetRange( const std::size_t column_index );

    void resetRange( void );

public:

    const ValueList& minRangeList( void ) const;

    const ValueList& maxRangeList( void ) const;

    const RangeList& insideRangeList( void ) const;

    const vismodule::Real64 minRange( const std::size_t column_index ) const;

    const vismodule::Real64 maxRange( const std::size_t column_index ) const;

    const bool insideRange( const std::size_t row_index ) const;

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
inline void TableObject::addColumn( const vismodule::ValueArray<T>& array, const std::string& label )
{
    this->addColumn( vismodule::AnyValueArray( array ), label ); // Shallow copy.
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
    this->addColumn( vismodule::AnyValueArray( array ), label ); // Deep copy.
}

/*===========================================================================*/
/**
 *  @brief  Returns value specified by indices.
 *  @param  row [in] row index
 *  @param  column [in] column index
 */
/*===========================================================================*/
template <typename T>
inline const T& TableObject::at( const std::size_t row, const std::size_t column ) const
{
    return( this->column( column ).template at<T>( row ) );
}

} // end of namespace vismodule

#endif // VIS_MODULE__TABLE_OBJECT_H_INCLUDE
