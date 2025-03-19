/****************************************************************************/
/**
 *  @file ObjectImporter.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ObjectImporter.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__OBJECT_IMPORTER_H_INCLUDE
#define VIS_MODULE__OBJECT_IMPORTER_H_INCLUDE

#include <string>
#include <vismodule/ClassName>
#include <vismodule/FileFormatBase>
#include <vismodule/ImporterBase>
#include <vismodule/ObjectBase>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Object importer class.
 */
/*==========================================================================*/
class ObjectImporter
{
    visModuleClassName_without_virtual( vismodule::ObjectImporter );

public:

    enum ImporterType
    {
        Image = 0,           ///< image object importer
        Point,               ///< point object importer
        Line,                ///< line object importer
        Polygon,             ///< polygon object importer
        StructuredVolume,    ///< structured volume object importer
        UnstructuredVolume,  ///< unstructured volume object importer
        Unknown              ///< unknown importer
    };

private:

    std::string          m_filename;      ///< input filename
    ImporterType         m_importer_type; ///< importer type
    vismodule::FileFormatBase* m_file_format;   ///< pointer to the estimated file format class
    vismodule::ImporterBase*   m_importer;      ///< pointer to the estimated importer class

public:

    explicit ObjectImporter( const std::string& filename );

    ~ObjectImporter( void );

public:

    vismodule::ObjectBase* import( void );

private:

    bool estimate_file_format( void );

    bool estimate_importer( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__OBJECT_IMPORTER_H_INCLUDE
