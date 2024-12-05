/****************************************************************************/
/**
 *  @file LineImporter.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: LineImporter.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__LINE_IMPORTER_H_INCLUDE
#define VIS_MODULE__LINE_IMPORTER_H_INCLUDE

#include <vismodule/ImporterBase>
#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/LineObject>
#include <vismodule/KVSMLObjectLine>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Line object importer.
 */
/*==========================================================================*/
class LineImporter
    : public vismodule::ImporterBase
    , public vismodule::LineObject
{
    // Class name.
    visModuleClassName( vismodule::LineImporter );

    // Module information.
    visModuleCategory( Importer );
    visModuleBaseClass( vismodule::ImporterBase );
    visModuleSuperClass( vismodule::LineObject );

public:

    LineImporter( void );

    LineImporter( const std::string& filename );

    LineImporter( const vismodule::FileFormatBase& file_format );

    virtual ~LineImporter( void );

public:

    SuperClass* exec( const vismodule::FileFormatBase& file_format );

private:

    void import( const vismodule::KVSMLObjectLine* kvsml );

    void set_min_max_coord( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__LINE_IMPORTER_H_INCLUDE
