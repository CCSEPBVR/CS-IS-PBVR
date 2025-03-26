/*****************************************************************************/
/**
 *  @file   TableImporter.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TableImporter.h 846 2011-06-21 07:04:44Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef VIS_MODULE__TABLE_IMPORTER_H_INCLUDE
#define VIS_MODULE__TABLE_IMPORTER_H_INCLUDE

#include <vismodule/ImporterBase>
#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/TableObject>
#include <vismodule/KVSMLObjectTable>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Importer class for TableObject.
 */
/*===========================================================================*/
class TableImporter : public vismodule::ImporterBase, public vismodule::TableObject
{
    // Class name.
    visModuleClassName( vismodule::TableImporter );

    // Module information.
    visModuleCategory( Importer );
    visModuleBaseClass( vismodule::ImporterBase );
    visModuleSuperClass( vismodule::TableObject );

public:

    TableImporter( void );

    TableImporter( const std::string& filename );

    TableImporter( const vismodule::FileFormatBase& file_format );

public:

    SuperClass* exec( const vismodule::FileFormatBase& file_format );

private:

    void import( const vismodule::KVSMLObjectTable* kvsml );
};

} // end of namespace vismodule

#endif // VIS_MODULE__TABLE_IMPORTER_H_INCLUDE
