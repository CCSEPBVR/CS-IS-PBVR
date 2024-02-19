#include "KVSMLObjectPointWriter.h"
#include "endian2.h"

KVSMLObjectPointWriter::KVSMLObjectPointWriter( pbvr::PointObject* object, std::string basename )
{
    m_object = object;
    m_basename = basename;

    write_main_tag();
    write_coords();
    write_colors();
    write_normals();
}

void KVSMLObjectPointWriter::write_main_tag( void )
{
#ifdef KVS_COMPILER_VC
    const std::string basename_wo_dir = m_basename.substr( m_basename.rfind( "\\" ) + 1 );
#else
    const std::string basename_wo_dir = m_basename.substr( m_basename.rfind( "/" ) + 1 );
#endif
    std::string kvsml_filename    = m_basename + ".kvsml";
    std::string coords_filename   = basename_wo_dir + "_coords.dat";
    std::string colors_filename   = basename_wo_dir + "_colors.dat";
    std::string normals_filename  = basename_wo_dir + "_normals.dat";

    FILE* kvsml_file = fopen( kvsml_filename.c_str(), "w" );

    fprintf( kvsml_file, "<KVSML>\n" );
    fprintf( kvsml_file, "<Object type=\"PointObject\">\n" );
    fprintf( kvsml_file, "<PointObject>\n" );
#ifdef KVS_COMPILER_VC
    fprintf( kvsml_file, "<Vertex nvertices=\"%Iu\">\n", m_object->nvertices() );
#else
    fprintf( kvsml_file, "<Vertex nvertices=\"%zu\">\n", m_object->nvertices() );
#endif
    fprintf( kvsml_file, "<Coord>\n" );
    fprintf( kvsml_file, "<DataArray type=\"float\" format=\"binary\" file=\"%s\"/>\n", coords_filename.c_str() );
    fprintf( kvsml_file, "</Coord>\n" );
    fprintf( kvsml_file, "<Color>\n" );
    fprintf( kvsml_file, "<DataArray type=\"uchar\" format=\"binary\" file=\"%s\"/>\n", colors_filename.c_str() );
    fprintf( kvsml_file, "</Color>\n" );
    fprintf( kvsml_file, "<Normal>\n" );
    fprintf( kvsml_file, "<DataArray type=\"float\" format=\"binary\" file=\"%s\"/>\n", normals_filename.c_str() );
    fprintf( kvsml_file, "</Normal>\n" );
    fprintf( kvsml_file, "<Size>\n" );
    fprintf( kvsml_file, "<DataValue>\n" );
    fprintf( kvsml_file, "%f\n", m_object->size( 0 ) );
    fprintf( kvsml_file, "</DataValue>\n" );
    fprintf( kvsml_file, "</Size>\n" );
    fprintf( kvsml_file, "</Vertex>\n" );
    fprintf( kvsml_file, "</PointObject>\n" );
    fprintf( kvsml_file, "</Object>\n" );
    fprintf( kvsml_file, "</KVSML>\n" );

    fclose( kvsml_file );
}

void KVSMLObjectPointWriter:: write_coords()
{
    std::string coords_filename   = m_basename + "_coords.dat";
    FILE* coords_file = fopen( coords_filename.c_str(), "wb" );

    size_t nvertices = m_object->nvertices();
    const float* coords_pointer = m_object->coords().pointer();

    size_t size = sizeof( float );
    size_t nmemb = nvertices * 3;
    float* ptr = new float[nmemb];
    FILE* stream = coords_file;

    std::memcpy( ptr, coords_pointer, size * nmemb );
    endian2::htol( ptr, nmemb );
    std::cout << fwrite( ptr, size, nmemb, stream ) << "write coord\n";
    delete[] ptr;

    fclose( coords_file );
}

void KVSMLObjectPointWriter::write_colors()
{
    std::string colors_filename   = m_basename + "_colors.dat";
    FILE* colors_file = fopen( colors_filename.c_str(), "wb" );

    size_t nvertices = m_object->nvertices();
    const unsigned char* colors_pointer = m_object->colors().pointer();

    size_t size = sizeof( unsigned char );
    size_t nmemb = nvertices * 3;
    unsigned char* ptr = new unsigned char[nmemb];
    FILE* stream = colors_file;

    std::memcpy( ptr, colors_pointer, size * nmemb );
    endian2::htol( ptr, nmemb );
    std::cout << fwrite( ptr, size, nmemb, stream ) << "write color\n";
    delete[] ptr;

    fclose( colors_file );
}

void KVSMLObjectPointWriter::write_normals()
{
    std::string normals_filename  = m_basename + "_normals.dat";
    FILE* normals_file = fopen( normals_filename.c_str(), "wb" );

    size_t nvertices = m_object->nvertices();
    const float* normals_pointer = m_object->normals().pointer();

    size_t size = sizeof( float );
    size_t nmemb = nvertices * 3;
    float* ptr = new float[nmemb];
    FILE* stream = normals_file;

    std::memcpy( ptr, normals_pointer, size * nmemb );
    endian2::htol( ptr, nmemb );
    std::cout << fwrite( ptr, size, nmemb, stream ) << "write normal\n";
    delete[] ptr;

    fclose( normals_file );
}
