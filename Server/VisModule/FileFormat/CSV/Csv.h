/*****************************************************************************/
/**
 *  @file   Csv.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Csv.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__CSV_H_INCLUDE
#define VIS_MODULE__CSV_H_INCLUDE

#include <vector>
#include <string>
#include <iostream>
#include <vismodule/FileFormatBase>
#include <vismodule/ClassName>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  CSV (Comma Separated Value) class.
 */
/*===========================================================================*/
class Csv : public vismodule::FileFormatBase
{
    visModuleClassName( vismodule::Csv );

public:

    typedef std::vector<std::string> Row;

protected:

    std::vector<Row> m_values; ///< value array

public:

    Csv( void );

    Csv( const std::string& filename );

    virtual ~Csv( void );

public:

    friend std::ostream& operator << ( std::ostream& os, const Csv& csv );

public:

    const std::size_t nrows( void ) const;

    const Row& row( const std::size_t index ) const;

    const std::string& value( const std::size_t i, const std::size_t j ) const;

    void addRow( const Row& row );

    void setRow( const std::size_t index, const Row& row );

    void setValue( const std::size_t i, const std::size_t j, const std::string& value );

public:

    const bool read( const std::string& filename );

    const bool write( const std::string& filename );

public:

    static const bool CheckFileExtension( const std::string& filename );

    static const bool CheckFileFormat( const std::string& filename );
};

} // end of namespace vismodule

#endif // VIS_MODULE__CSV_H_INCLUDE
