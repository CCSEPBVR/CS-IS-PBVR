/****************************************************************************/
/**
 *  @file ImporterBase.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ImporterBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__IMPORTER_BASE_H_INCLUDE
#define VIS_MODULE__IMPORTER_BASE_H_INCLUDE

#include <vismodule/FileFormatBase>
#include <vismodule/ObjectBase>
#include <vismodule/ClassName>
#include <vismodule/Module>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Importer base class.
 */
/*==========================================================================*/
class ImporterBase
{
    visModuleClassName( vismodule::ImporterBase );

    visModuleBase;

protected:

    bool m_is_success; ///< check flag for importing

public:

    ImporterBase();

    virtual ~ImporterBase();

public:

    virtual vismodule::ObjectBase* exec( const vismodule::FileFormatBase& file_format ) = 0;

public:

    const bool isSuccess() const;

    const bool isFailure() const;
};

} // end of namespace vismodule

#endif // VIS_MODULE__IMPORTER_BASE_H_INCLUDE
