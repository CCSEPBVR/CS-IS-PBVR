/*****************************************************************************/
/**
 *  @file   SystemInformation.h
 *  @author Naohisa Sakamoto
 */
/*****************************************************************************/
#pragma once
#include <cstdio>
#include <stddef.h>


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  System information class.
 */
/*===========================================================================*/
class SystemInformation
{
public:
    static std::size_t NumberOfProcessors();
    static std::size_t TotalMemorySize();
    static std::size_t FreeMemorySize();

private:
    SystemInformation();
};

} // end of namespace kvs
