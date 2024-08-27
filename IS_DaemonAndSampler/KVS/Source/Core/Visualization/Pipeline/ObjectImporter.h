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
#ifndef KVS__OBJECT_IMPORTER_H_INCLUDE
#define KVS__OBJECT_IMPORTER_H_INCLUDE

#include <string>
#include <kvs/ClassName>
#include <kvs/FileFormatBase>
#include <kvs/ImporterBase>
#include <kvs/ObjectBase>


namespace kvs
{

/*==========================================================================*/
/**
 *  Object importer class.
 */
/*==========================================================================*/
class ObjectImporter
{
    kvsClassName_without_virtual( kvs::ObjectImporter );

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
    kvs::FileFormatBase* m_file_format;   ///< pointer to the estimated file format class
    kvs::ImporterBase*   m_importer;      ///< pointer to the estimated importer class

public:

    explicit ObjectImporter( const std::string& filename );

    ~ObjectImporter( void );

public:

    kvs::ObjectBase* import( void );

private:

    bool estimate_file_format( void );

    bool estimate_importer( void );
};

} // end of namespace kvs

#endif // KVS__OBJECT_IMPORTER_H_INCLUDE
