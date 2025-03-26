/*****************************************************************************/
/**
 *  @file   KVSMLObjectTable.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: KVSMLObjectTable.h 846 2011-06-21 07:04:44Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__KVSML_OBJECT_TABLE_H_INCLUDE
#define VIS_MODULE__KVSML_OBJECT_TABLE_H_INCLUDE

#include <vismodule/FileFormatBase>
#include <vismodule/AnyValueArray>
#include <vismodule/Type>
#include <vismodule/Vector3>
#include <string>
#include <vector>
#include <FileFormat/KVSML/KVSMLTag.h>
#include <FileFormat/KVSML/ObjectTag.h>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  KVSML table object format.
 */
/*===========================================================================*/
class KVSMLObjectTable : public vismodule::FileFormatBase
{
    visModuleClassName( vismodule::KVSMLObjectTable );

public:

    enum WritingDataType
    {
        Ascii = 0,
        ExternalAscii,
        ExternalBinary
    };

protected:

    vismodule::kvsml::KVSMLTag m_kvsml_tag;   ///< KVSML tag information
    vismodule::kvsml::ObjectTag m_object_tag; ///< Object tag information
    WritingDataType m_writing_type; ///< writing data type
    size_t m_nrows; ///< number of rows
    size_t m_ncolumns; ///< number of columns
    std::vector<std::string> m_labels; ///< column label list
    std::vector<vismodule::AnyValueArray> m_columns; ///< column list
    std::vector<bool> m_has_min_values; ///< min. value flag list
    std::vector<bool> m_has_max_values; ///< max. value flag list
    std::vector<bool> m_has_min_ranges; ///< min. range flag list
    std::vector<bool> m_has_max_ranges; ///< max. range flag list
    std::vector<double> m_min_values; ///< min. value list
    std::vector<double> m_max_values; ///< max. value list
    std::vector<double> m_min_ranges; ///< min. range list
    std::vector<double> m_max_ranges; ///< max. range list

public:

    KVSMLObjectTable( void );

    KVSMLObjectTable( const std::string& filename );

    virtual ~KVSMLObjectTable( void );

public:

    const vismodule::kvsml::KVSMLTag& KVSMLTag( void ) const;

    const vismodule::kvsml::ObjectTag& objectTag( void ) const;

    const size_t nrows( void ) const;

    const size_t ncolumns( void ) const;

    const std::vector<std::string>& labelList( void ) const;

    const std::vector<vismodule::AnyValueArray>& columnList( void ) const;

    const std::vector<bool>& hasMinValueList( void ) const;

    const std::vector<bool>& hasMaxValueList( void ) const;

    const std::vector<bool>& hasMinRangeList( void ) const;

    const std::vector<bool>& hasMaxRangeList( void ) const;

    const std::vector<double>& minValueList( void ) const;

    const std::vector<double>& maxValueList( void ) const;

    const std::vector<double>& minRangeList( void ) const;

    const std::vector<double>& maxRangeList( void ) const;

public:

    void setFilename( const std::string& filename );

    void setWritingDataType( const WritingDataType writing_type );

    void setMinValue( const size_t column_index, const double value );

    void setMaxValue( const size_t column_index, const double value );

    void setMinRange( const size_t column_index, const double range );

    void setMaxRange( const size_t column_index, const double range );

    void addColumn( const vismodule::AnyValueArray& column, const std::string& label );

public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );

public:

    static const bool CheckFileExtension( const std::string& filename );

    static const bool CheckFileFormat( const std::string& filename );

    friend std::ostream& operator <<( std::ostream& os, const KVSMLObjectTable& rhs );
};

} // end of namespace vismodule

#endif // VIS_MODULE__KVSML_OBJECT_TABLE_H_INCLUDE
