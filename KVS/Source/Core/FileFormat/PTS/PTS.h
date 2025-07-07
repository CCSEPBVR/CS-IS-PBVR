/****************************************************************************/
/**
 *  @file    PTS.h
 *  @authors
 */
/****************************************************************************/
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <kvs/ValueArray>
#include <kvs/FileFormatBase>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  PTS point object format.
 */
/*===========================================================================*/
class PTS : public kvs::FileFormatBase
{
public:

    typedef kvs::FileFormatBase BaseClass;

private:
    kvs::UInt32 m_npoints;
    kvs::UInt32 m_ncomponents;
    kvs::ValueArray<kvs::Real32> m_coords; ///< coordinate array
    kvs::ValueArray<kvs::UInt8> m_colors; ///< color(r,g,b) array

public:
    static bool CheckExtension( const std::string& filename );

public:
    PTS();
    PTS( const std::string& filename );

    const kvs::ValueArray<kvs::Real32>& coords() const { return m_coords; }
    const kvs::ValueArray<kvs::UInt8>& colors() const { return m_colors; }

    bool read( const std::string& filename );
    bool write( const std::string& filename );

private:
    std::vector<std::string> split(const std::string& input, char delimiter);
};

}
