/*****************************************************************************/
/**
 *  @file   ExporterBase.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ExporterBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__EXPORTER_BASE_H_INCLUDE
#define VIS_MODULE__EXPORTER_BASE_H_INCLUDE

#include <vismodule/ClassName>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Exporter base class.
 */
/*===========================================================================*/
template <typename FileFormatType>
class ExporterBase : public FileFormatType
{
    visModuleClassName( vismodule::ExporterBase );

protected:

    bool m_is_success; ///< check flag for exporting

public:

    ExporterBase( void );

    virtual ~ExporterBase( void );

public:

    virtual FileFormatType* exec( const vismodule::ObjectBase& object ) = 0;

public:

    const bool isSuccess( void ) const;

    const bool isFailure( void ) const;
};

template <typename FileFormatType>
ExporterBase<FileFormatType>::ExporterBase( void )
{
}

template <typename FileFormatType>
ExporterBase<FileFormatType>::~ExporterBase( void )
{
}

template <typename FileFormatType>
const bool ExporterBase<FileFormatType>::isSuccess( void ) const
{
    return( m_is_success );
}

template <typename FileFormatType>
const bool ExporterBase<FileFormatType>::isFailure( void ) const
{
    return( !m_is_success );
}

} // end of namespace vismodule

#endif // VIS_MODULE__EXPORTER_BASE_H_INCLUDE
