#include <sstream>
#include <algorithm>
#include <kvs/Directory>
#include <kvs/File>
#include <kvs/PolygonImporter>
#include <kvs/PointImporter>
#include <kvs/PointExporter>
//#include <kvs/KVSMLObjectPoint>
#include "ParticleMerger.h"
#include "KVSMLObjectPointWriter.h"

namespace kvs
{
namespace visclient
{

ParticleMerger::ParticleMerger():
    m_initial_step( m_parameter.m_particles.size() ),
    m_final_step( m_parameter.m_particles.size() ),
    m_division( m_parameter.m_particles.size() ),
    m_file_prefix( m_parameter.m_particles.size() ),
    m_local_object_subpixel_level(1)
{
}

ParticleMerger::~ParticleMerger()
{
}

void ParticleMerger::setParam( const ParticleMergeParameter& param, const size_t server_init_step, const size_t server_fin_step )
{
    size_t num = param.m_particles.size();
    m_last_parameter = m_parameter;
    m_parameter = param;

    for ( size_t n = 0; n < num; n++ )
    {
        if(!m_parameter.m_particles[n].m_enable) continue;
        const std::string m_file_path = m_parameter.m_particles[n].m_file_path;
        const std::string lastm_file_path = m_last_parameter.m_particles[n].m_file_path;
        if ( m_file_path == "server" )
        {
            std::cout << "ParticleMerger::setParam server " << param.m_particles.size()<<":"<< m_parameter.m_particles.size()<< std::endl;
            m_initial_step[n] = server_init_step;
            m_final_step[n] = server_fin_step;
        }
        else if ( m_file_path != lastm_file_path )
        {
            std::cout << "FILE check " << m_file_path << std::endl;
            kvs::File file_prefix( m_file_path );
            // search final step
            kvs::Directory dir( file_prefix.pathName() );
            kvs::FileList pre_files = dir.fileList();
            kvs::FileList files( 0 );
            kvs::FileList::iterator fi;
            std::string prefix;
            size_t num_kvsml = 0;
            size_t num_stl = 0;
            size_t step;
            size_t div = 0;
            size_t init_step = 0, fin_step = 0;
            std::string sstep, sdiv;
            std::stringstream ss;
            size_t step_length    = 5;
            size_t div_length     = 7;
            size_t div_num_length = 7;
            size_t kvsml_suffix_length = std::string("kvsml").length();
            size_t stl_suffix_length = std::string("stl").length();
            std::string f_prefix = file_prefix.fileName();

            size_t kvsml_file_length_point_object = f_prefix.length() + 1
                               + step_length + 1
                               + div_length + 1
                               + div_num_length + 1
                               + kvsml_suffix_length;

            size_t kvsml_file_length_polygon_object = f_prefix.length() + 1
                               + step_length + 1
                               + kvsml_suffix_length;

            size_t stl_file_length = f_prefix.length() + 1
                               + step_length + 1
                               + stl_suffix_length;

            for ( fi = pre_files.begin(); fi != pre_files.end(); fi++ )
            {
                std::string bn = fi->baseName();
                std::string f_name = fi->fileName();
                size_t sep1 = bn.find(f_prefix, 0);
                if (sep1 == 0 && kvsml_file_length_point_object == f_name.length())
                    files.push_back(*fi);
                if (sep1 == 0 && kvsml_file_length_polygon_object == f_name.length())
                    files.push_back(*fi);
                if (sep1 == 0 && stl_file_length == f_name.length())
                    files.push_back(*fi);
            }

            for ( fi = files.begin(); fi != files.end(); fi++ )
            {
                std::string ext = fi->extension();
                if ( ext == "kvsml" )
                {
                    std::string bn = fi->baseName();
                    std::string f_prefix = file_prefix.fileName();

                    bool is_multi_underbar = false;

                    if( std::count(bn.cbegin(),bn.cend(),'_') < 2 )
                    {
                    }else{
                     is_multi_underbar = true;
                    }

                    size_t sep0 = f_prefix.length();
                    size_t sep1 = bn.find( "_", sep0 );
                    size_t sep2 = bn.find( "_", sep1 + 1 );
                    size_t sep3 = bn.find( "_", sep2 + 1 );
                    if(is_multi_underbar == true)
                    {
                    if ( sep1 == std::string::npos || sep2 == std::string::npos || sep3 == std::string::npos )
                        continue;
                    }else
                    {
                        if ( sep1 == std::string::npos )
                            continue;
                    }
                    prefix = bn.substr( 0, sep1 );
                    sstep = bn.substr( sep1 + 1, step_length );
                    if(is_multi_underbar == true)
                    {
                        sdiv = bn.substr( sep3 + 1, div_length );
                    }
//                    std::cout << bn << " " << prefix << " " << sdiv << std::endl;

                    ss.clear();
                    ss << sstep << ' ' << sdiv;
                    ss >> step >> div;

                    init_step = ( num_kvsml ) ? std::min( init_step, step ) : step;
                    fin_step  = ( num_kvsml ) ? std::max( fin_step, step ) : step;

                    num_kvsml++;
                }

                if ( ext == "stl" )
                {
                    std::string bn = fi->baseName();
                    std::string f_prefix = file_prefix.fileName();
                    size_t sep0 = f_prefix.length();
                    size_t sep1 = bn.find( "_", sep0 );
                    if ( sep1 == std::string::npos )
                        continue;
                    prefix = bn.substr( 0, sep1 );
                    sstep = bn.substr( sep1 + 1, step_length );
//                    std::cout << bn << " " << prefix << " " << sdiv << std::endl;

                    ss.clear();
                    ss << sstep << ' ' << sdiv;
                    ss >> step >> div;

                    init_step = ( num_stl ) ? std::min( init_step, step ) : step;
                    fin_step  = ( num_stl ) ? std::max( fin_step, step ) : step;

                    num_stl++;

                }
            }

            m_division[n] = div;
            m_initial_step[n] = init_step;
            m_final_step[n] = fin_step;
            m_file_prefix[n] = dir.directoryPath( true ) + dir.Separator() + prefix;

            if ( num_kvsml != 0 )
            {
                // 存在領域設定ファイルがあるか確認をする
                std::string filename( m_file_path );
//              filename.append( dir.Separator().c_str() );
//              filename.append( prefix.c_str() );
                filename.append( "_pfi_coords_minmax.txt" );
                kvs::File f( filename.c_str()  );
                if ( f.isExisted() )
                {
                    // ファイルがある
                    FILE* fp = NULL;
                    fp = fopen( filename.c_str(), "r" );
                    fscanf( fp, "%f %f %f %f %f %f",
                            &m_parameter.m_particles[n].m_x_min,
                            &m_parameter.m_particles[n].m_y_min,
                            &m_parameter.m_particles[n].m_z_min,
                            &m_parameter.m_particles[n].m_x_max,
                            &m_parameter.m_particles[n].m_y_max,
                            &m_parameter.m_particles[n].m_z_max );
                    if ( fp != NULL ) fclose( fp );
                }
                else
                {
                    // ファイルがない
                    // 最初のstepのファイルをopenして情報を得る
                    std::string prefix = m_file_prefix[n];
                    std::stringstream suffix;
                    suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << m_initial_step[n]
                           << '_' << std::setw( 7 ) << std::setfill( '0' ) << 1
                           << '_' << std::setw( 7 ) << std::setfill( '0' ) << m_division[n];
                    std::string filename = prefix + suffix.str() + ".kvsml";

                    const kvs::File file( filename );
                    kvs::PointObject* impobj = new kvs::PointImporter( filename );
                    m_local_object_subpixel_level = impobj->size();
                    //サーバ側のmix max coordsを書き記したテキストファイルを読み込む。
                    //ない場合は従来の方法でminmaxの値を決める。
                    std::string filename_minMax = prefix + ".minMax";

                    std::cout << prefix << std::endl;
                    FILE* serverMinMaxCoords_file = fopen(filename_minMax.c_str(),"r");
                    if(serverMinMaxCoords_file == NULL)
                    {
                        const kvs::Vector3f& min_t = impobj->minObjectCoord();
                        const kvs::Vector3f& max_t = impobj->maxObjectCoord();
                        m_parameter.m_particles[n].m_x_min = min_t[0];
                        m_parameter.m_particles[n].m_y_min = min_t[1];
                        m_parameter.m_particles[n].m_z_min = min_t[2];
                        m_parameter.m_particles[n].m_x_max = max_t[0];
                        m_parameter.m_particles[n].m_y_max = max_t[1];
                        m_parameter.m_particles[n].m_z_max = max_t[2];
                    }else{
                        kvs::Vector3f min_t;
                        kvs::Vector3f max_t;
                        fscanf(serverMinMaxCoords_file,"%f,%f,%f,%f,%f,%f",
                               &min_t[0],
                                &min_t[1],
                                &min_t[2],
                                &max_t[0],
                                &max_t[1],
                                &max_t[2]);
                        m_parameter.m_particles[n].m_x_min = min_t[0];
                        m_parameter.m_particles[n].m_y_min = min_t[1];
                        m_parameter.m_particles[n].m_z_min = min_t[2];
                        m_parameter.m_particles[n].m_x_max = max_t[0];
                        m_parameter.m_particles[n].m_y_max = max_t[1];
                        m_parameter.m_particles[n].m_z_max = max_t[2];
                    }
                    delete impobj;
                }
            }

            if ( num_stl != 0 )
            {

                // ファイルがない
                // 最初のstepのファイルをopenして情報を得る
                std::string prefix = m_file_prefix[n];
                std::stringstream suffix;
                suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << m_initial_step[n];
                std::string filename = prefix + suffix.str() + ".stl";
                const kvs::File file( filename );
                kvs::PolygonObject* impobj = new kvs::PolygonImporter( filename );

                    const kvs::Vector3f& min_t = impobj->minObjectCoord();
                    const kvs::Vector3f& max_t = impobj->maxObjectCoord();
                    m_parameter.m_particles[n].m_x_min = min_t[0];
                    m_parameter.m_particles[n].m_y_min = min_t[1];
                    m_parameter.m_particles[n].m_z_min = min_t[2];
                    m_parameter.m_particles[n].m_x_max = max_t[0];
                    m_parameter.m_particles[n].m_y_max = max_t[1];
                    m_parameter.m_particles[n].m_z_max = max_t[2];

                delete impobj;
            }
        }
    }
}

bool ParticleMerger::calculateExternalCoords( float* crd )
{
    bool cf = false;
    size_t num = m_parameter.m_particles.size();
    for ( size_t n = 1; n < num; n++ )
    {
        if ( !m_parameter.m_particles[n].m_enable ) continue;
        if ( cf )
        {
            m_x_min = std::min( m_x_min, m_parameter.m_particles[n].m_x_min );
            m_y_min = std::min( m_y_min, m_parameter.m_particles[n].m_y_min );
            m_z_min = std::min( m_z_min, m_parameter.m_particles[n].m_z_min );
            m_x_max = std::max( m_x_max, m_parameter.m_particles[n].m_x_max );
            m_y_max = std::max( m_y_max, m_parameter.m_particles[n].m_y_max );
            m_z_max = std::max( m_z_max, m_parameter.m_particles[n].m_z_max );
        }
        else
        {
            cf = true;
            m_x_min = m_parameter.m_particles[n].m_x_min;
            m_y_min = m_parameter.m_particles[n].m_y_min;
            m_z_min = m_parameter.m_particles[n].m_z_min;
            m_x_max = m_parameter.m_particles[n].m_x_max;
            m_y_max = m_parameter.m_particles[n].m_y_max;
            m_z_max = m_parameter.m_particles[n].m_z_max;
        }
    }
    if ( cf )
    {
        crd[0] = m_x_min;
        crd[1] = m_y_min;
        crd[2] = m_z_min;
        crd[3] = m_x_max;
        crd[4] = m_y_max;
        crd[5] = m_z_max;
    }
    return cf;
}

void ParticleMerger::setEnable( int i )
{
    m_parameter.m_particles[i].m_enable = true;
}

size_t ParticleMerger::getMergedInitialTimeStep()
{
    size_t init_step = getMergedFinalTimeStep();

    size_t num = m_parameter.m_particles.size();
    for ( size_t n = 0; n < num; n++ )
    {
        if ( !m_parameter.m_particles[n].m_enable ) continue;
        init_step = std::min( init_step, m_initial_step[n] );
    }

    return init_step;
}

size_t ParticleMerger::getMergedFinalTimeStep()
{
    size_t fin_step = 0;

    size_t num = m_parameter.m_particles.size();
//      std::cout << "ParticleMerger::getMergedFinalTimeStep num " <<num<< std::endl;
    for ( size_t n = 0; n < num; n++ )
    {
        if ( !m_parameter.m_particles[n].m_enable ) continue;
        fin_step = std::max( fin_step, m_final_step[n] );
    }

    return fin_step;
}

kvs::PointObject* ParticleMerger::doMerge(
    const kvs::PointObject* server_particle,
    const size_t step,
        bool is_client_server_mode)
{
    kvs::PointObject* obj = new kvs::PointObject();
    size_t num = m_parameter.m_particles.size();

    for ( size_t n = 0; n < num; n++ )
    {
        // 2018.12.25 ステップ数の不一致は関係ない
        if ( !m_parameter.m_particles[n].m_enable ) continue;
        //if ( ( step < m_initial_step[n] ) && ( !m_parameter.m_particles[n].m_keep_initial_step ) ) continue;
        //if ( ( step > m_final_step[n] )   && ( !m_parameter.m_particles[n].m_keep_final_step )  ) continue;
        if ( ( step >= m_initial_step[n] && step <= m_final_step[n] || m_parameter.m_particles[n].m_keep_initial_step || m_parameter.m_particles[n].m_keep_final_step )
         && ( m_parameter.m_particles[n].m_file_path == "server" ) )
        {
//std::cout << "========== obj : " << obj << std::endl;
//std::cout << "========== server : " << &server_particle << std::endl;
            if (server_particle != NULL){
                ( *obj ) +=  (*server_particle);
            }

//            ( *obj ) += server_particle;
            continue;
        }

        // m_keepinitial_step, m_keep_funal_step は常にどちらかを出力する
        // 両方のチェックがある場合は、m_keepinitial_step優先とする
        //== start
        //size_t filestep = std::max( std::min( step, m_final_step[n] ), m_initial_step[n] );
        size_t filestep  = step;
        if(m_parameter.m_particles[n].m_keep_final_step == false && m_parameter.m_particles[n].m_keep_initial_step == false){
            if(m_final_step[n] < filestep){
            }else if(m_initial_step[n] <= filestep){
            }else{
            }
        }

        if(m_parameter.m_particles[n].m_keep_final_step == true && m_parameter.m_particles[n].m_keep_initial_step == false){
            if(m_final_step[n] < filestep){
                filestep = m_final_step[n];
            }else if(m_initial_step[n] <= filestep){
            }else{
            }
        }

        if(m_parameter.m_particles[n].m_keep_final_step == false && m_parameter.m_particles[n].m_keep_initial_step == true){
            if(m_initial_step[n] > filestep){
                filestep = m_initial_step[n];
            }else if(m_final_step[n] >= filestep){
            }else{
            }
        }

        if(m_parameter.m_particles[n].m_keep_final_step == true && m_parameter.m_particles[n].m_keep_initial_step == true){
            if(m_final_step[n] < filestep){
                filestep = m_final_step[n];
            }else if(m_initial_step[n] <= filestep){
            }else if(m_initial_step[n] > filestep){
                filestep = m_initial_step[n];
            }else{
            }
        }
//        if (m_parameter.m_particles[n].m_keep_initial_step)
//            filestep = m_initial_step[n];
//        else if (m_parameter.m_particles[n].m_keep_final_step)
//            filestep = m_final_step[n];
//        if (filestep < m_initial_step[n] || filestep > m_final_step[n]) continue;
        //== end
        for ( size_t m = 0; m < m_division[n]; m++ )
        {
            std::string prefix = m_file_prefix[n];
            std::stringstream suffix;
            suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << filestep
                   << '_' << std::setw( 7 ) << std::setfill( '0' ) << m + 1
                   << '_' << std::setw( 7 ) << std::setfill( '0' ) << m_division[n];
            std::string filename = prefix + suffix.str() + ".kvsml";
//          std::cout << filename << std::endl;

            const kvs::File file( filename );
            kvs::PointObject* impobj = new kvs::PointImporter( filename );
            size_t nmemb = impobj->coords().size();

            if ( nmemb != 0 )
            {
                //エラー回避
                obj->setSize( 1 );
                impobj->setSize( 1 );
                ( *obj ) += *impobj;
            }
            delete impobj;
        }
    }

    if ( m_parameter.m_do_export )
    {
        // Export merged PointObject to kvsml file

        std::stringstream suffix;
        suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << step
               << '_' << std::setw( 7 ) << std::setfill( '0' ) << 1
               << '_' << std::setw( 7 ) << std::setfill( '0' ) << 1;

        std::string basename = m_parameter.m_export_file_path + suffix.str();

        std::cout << basename << std::endl;
        KVSMLObjectPointWriter( *obj, basename );
    }
    //サーバーのポイントオブジェクトが存在する場合は、統合したポイントオブジェクトのsubpixellevelの値をサーバ側の値にセットする。
    //この時にサーバのsubpixellevelの値を保持しておく。
    //存在しない場合は保持した値をセットする。
    if(is_client_server_mode == true){
        if(server_particle != nullptr){
            obj->setSize(server_particle->size());
            m_server_side_subpixel_level = server_particle->size();
        }else{
                obj->setSize(m_server_side_subpixel_level);
        }
//        obj->setSize(server_particle->size());
    }

    if(is_client_server_mode == false){
        if(server_particle != nullptr){
            obj->setSize(server_particle->size());
            m_server_side_subpixel_level = server_particle->size();
        }else{
            if(m_server_side_subpixel_level != m_local_object_subpixel_level)
            {
                obj->setSize(m_local_object_subpixel_level);
            }else{
                obj->setSize(m_server_side_subpixel_level);
            }

        }
    }
    return obj;
}

bool ParticleMerger::isPolygonEnable(int index)
{
    return m_parameter.m_particles[index].m_enable;
}

std::string ParticleMerger::getPolygonFilePath(int index)
{
    return m_parameter.m_particles[index].m_file_path;
}

kvs::RGBColor ParticleMerger::getPolygonColor(int index)
{
    return m_parameter.m_particles[index].m_polygon_color;
}

double ParticleMerger::getPolygon_opacity(int index)
{
    return m_parameter.m_particles[index].m_polygon_opacity;
}

size_t ParticleMerger::getLocalObjectInitialStep(int index)
{
    return m_initial_step[index];
}

size_t ParticleMerger::getLocalObjectFinalStep(int index)
{
    return m_final_step[index];
}

bool ParticleMerger::getLocalObjectIsEnableKeepInitial(int index)
{
    return m_parameter.m_particles[index].m_keep_initial_step;
}

bool ParticleMerger::getLocalObjectIsEnableKeepFinal(int index)
{
    return m_parameter.m_particles[index].m_keep_final_step;
}

} // namespace visclient
} // namespace kvs

