#include <iostream>
#include <fstream>
#include <algorithm>

#include <vismodule/MultiVolumeProperty>
#include <vismodule/File>
#include <vismodule/endian2>
#include <vismodule/NameListFile>

#ifdef EXTEND_FILE_FORMAT 
#include <filesystem>
#include <kvs/extendedfileformat/VtkXmlMultiBlock>
#include <kvs/extendedfileformat/NumeralSequenceFiles>
#include <kvs/extendedfileformat/VtkXmlUnstructuredGrid>
#include <kvs/extendedfileformat/VtkImporter>
#include <kvs/extendedfileformat/VtkXmlImageData>
#include <kvs/extendedfileformat/AvsUcd>
#include <kvs/extendedfileformat/VtkXmlPUnstructuredGrid>
#include <kvs/extendedfileformat/EnSightGoldBinary>
#endif

//--------------------------------------------------------------------------

int MultiVolumeProperty::loadPFI( const std::string& filename )
{

    std::ifstream fin( filename, std::ios::in | std::ios::binary );
    if (!fin.is_open()) {
        std::cerr << "ファイルを開けませんでした: " << filename << std::endl;
        return false;
    }
    char ch;
    is_binary = false;
    while (fin.get(ch))
    {
        // 非ASCII文字（制御文字）が含まれていればバイナリファイルと判別
        if (ch < 0x09 || (ch > 0x0D && ch < 0x20)) {
            is_binary = true; // バイナリ
        }
    }
    fin.close();

    if(!is_binary) // アスキー
    {
        NameListFile m_name_list_file(filename); 
        m_name_list_file.setName( "NODE_NUM" );
        m_name_list_file.setName( "CELL_NUM" );
        m_name_list_file.setName( "CELLTYPE" ); 
        m_name_list_file.setName( "FILETYPE" ); 
        m_name_list_file.setName( "FILES_NUM" ); 
        m_name_list_file.setName( "COMPONENT_NUM" ); 
        m_name_list_file.setName( "BEGINNING_STEP" ); 
        m_name_list_file.setName( "LAST_STEP" ); 
        m_name_list_file.setName( "SUBVOLUME_NUM" ); 
        m_name_list_file.setName( "MIN_COORD_0" ); 
        m_name_list_file.setName( "MIN_COORD_1" ); 
        m_name_list_file.setName( "MIN_COORD_2" ); 
        m_name_list_file.setName( "MAX_COORD_0" ); 
        m_name_list_file.setName( "MAX_COORD_1" ); 
        m_name_list_file.setName( "MAX_COORD_2" );

        m_name_list_file.read();

        m_number_nodes        = m_name_list_file.getValue<int>("NODE_NUM");
        m_number_elements     = m_name_list_file.getValue<int>("CELL_NUM");
        m_elem_type           = m_name_list_file.getValue<int>("CELLTYPE");
        m_file_type           = m_name_list_file.getValue<int>("FILETYPE");
        m_number_files        = m_name_list_file.getValue<int>("FILES_NUM");
        m_number_ingredients  = m_name_list_file.getValue<int>("COMPONENT_NUM");
        m_start_step          = m_name_list_file.getValue<int>("BEGINNING_STEP");
        m_end_steps           = m_name_list_file.getValue<int>("LAST_STEP");
        m_number_subvolumes   = m_name_list_file.getValue<int>("SUBVOLUME_NUM");
        float x_min, y_min, z_min;
        float x_max, y_max, z_max;
        x_min = m_name_list_file.getValue<float>("MIN_COORD_0"); 
        y_min = m_name_list_file.getValue<float>("MIN_COORD_1"); 
        z_min = m_name_list_file.getValue<float>("MIN_COORD_2"); 
        x_max = m_name_list_file.getValue<float>("MAX_COORD_0"); 
        y_max = m_name_list_file.getValue<float>("MAX_COORD_1"); 
        z_max = m_name_list_file.getValue<float>("MAX_COORD_2"); 
        m_min_object_coord.set( x_min, y_min, z_min );
        m_max_object_coord.set( x_max, y_max, z_max );
        std::cout << "m_end_steps = " << m_end_steps <<std::endl; 
        std::cout << "m_start_step = " << m_start_step <<std::endl; 
        m_number_steps = m_end_steps - m_start_step + 1;

        for(int i =0; i< m_number_subvolumes; i++)
        {
            std::stringstream ss_min, ss_max;
            ss_min << "SUB_VOLUME_MIN_COORD_" << i << "_";
            ss_max << "SUB_VOLUME_MAX_COORD_" << i << "_";
            const std::string tag_min_coord_base = ss_min.str();
            const std::string tag_max_coord_base = ss_max.str();
            m_name_list_file.setName( tag_min_coord_base + "0" );
            m_name_list_file.setName( tag_min_coord_base + "1" );
            m_name_list_file.setName( tag_min_coord_base + "2" );
            m_name_list_file.setName( tag_max_coord_base + "0" );
            m_name_list_file.setName( tag_max_coord_base + "1" );
            m_name_list_file.setName( tag_max_coord_base + "2" );
        }

        m_min_subvolume_coord.resize( m_number_subvolumes );
        m_max_subvolume_coord.resize( m_number_subvolumes );
        m_name_list_file.read();

        for(int vl =0; vl< m_number_subvolumes; vl++)
        {
            float sub_x_min, sub_y_min, sub_z_min;
            float sub_x_max, sub_y_max, sub_z_max;
            std::stringstream ss_min, ss_max;
            ss_min << "SUB_VOLUME_MIN_COORD_" << vl << "_";
            ss_max << "SUB_VOLUME_MAX_COORD_" << vl << "_";
            //変数読み取り処理
            const std::string tag_min_coord_base = ss_min.str();
            const std::string tag_max_coord_base = ss_max.str();
            sub_x_min = m_name_list_file.getValue<float>( tag_min_coord_base + "0"); 
            sub_y_min = m_name_list_file.getValue<float>( tag_min_coord_base + "1"); 
            sub_z_min = m_name_list_file.getValue<float>( tag_min_coord_base + "2"); 
            sub_x_max = m_name_list_file.getValue<float>( tag_max_coord_base + "0"); 
            sub_y_max = m_name_list_file.getValue<float>( tag_max_coord_base + "1"); 
            sub_z_max = m_name_list_file.getValue<float>( tag_max_coord_base + "2"); 
            m_min_subvolume_coord[vl].set( sub_x_min, sub_y_min, sub_z_min );
            m_max_subvolume_coord[vl].set( sub_x_max, sub_y_max, sub_z_max );
        }
    }
    else
    { 

#if 1
        std::ifstream fin( filename.c_str(), std::ios::in | std::ios::binary );
        if ( ! fin ) return -1;
//        fin.seekg( 0, std::ios_base::beg );
        fin.read( ( char* )&m_number_nodes, sizeof( int32_t ) );
        fin.read( ( char* )&m_number_elements, sizeof( int32_t ) );
        fin.read( ( char* )&m_elem_type, sizeof( int32_t ) );
        fin.read( ( char* )&m_file_type, sizeof( int32_t ) );
        fin.read( ( char* )&m_number_files, sizeof( int32_t ) );
        fin.read( ( char* )&m_number_ingredients, sizeof( int32_t ) );
        fin.read( ( char* )&m_start_step, sizeof( int32_t ) );
        fin.read( ( char* )&m_end_steps, sizeof( int32_t ) );
        fin.read( ( char* )&m_number_subvolumes, sizeof( int32_t ) );

        std::cout << "m_start_step = " << m_start_step << std::endl;
        std::cout << "m_end_step = " << m_end_steps << std::endl;
        endian2::LittleToHost( &m_number_nodes );
        endian2::LittleToHost( &m_number_elements );
        endian2::LittleToHost( &m_elem_type );
        endian2::LittleToHost( &m_file_type );
        endian2::LittleToHost( &m_number_files );
        endian2::LittleToHost( &m_number_ingredients );
        endian2::LittleToHost( &m_start_step );
        endian2::LittleToHost( &m_end_steps );
        endian2::LittleToHost( &m_number_subvolumes );
        m_number_steps = m_end_steps - m_start_step + 1;

        float x_min, y_min, z_min;
        float x_max, y_max, z_max;
        fin.read( ( char* )&x_min, sizeof( float ) );
        fin.read( ( char* )&y_min, sizeof( float ) );
        fin.read( ( char* )&z_min, sizeof( float ) );
        fin.read( ( char* )&x_max, sizeof( float ) );
        fin.read( ( char* )&y_max, sizeof( float ) );
        fin.read( ( char* )&z_max, sizeof( float ) );
        endian2::LittleToHost( &x_min );
        endian2::LittleToHost( &y_min );
        endian2::LittleToHost( &z_min );
        endian2::LittleToHost( &x_max );
        endian2::LittleToHost( &y_max );
        endian2::LittleToHost( &z_max );
        m_min_object_coord.set( x_min, y_min, z_min );
        m_max_object_coord.set( x_max, y_max, z_max );

        int skipsize = 0;
        skipsize += ( m_number_subvolumes + m_number_subvolumes ) * sizeof( int );
        fin.seekg( skipsize, std::ios::cur );

        m_min_subvolume_coord.resize( m_number_subvolumes );
        m_max_subvolume_coord.resize( m_number_subvolumes );
        for ( int32_t vl = 0; vl < m_number_subvolumes; vl++ )
        {
            float sub_x_min, sub_y_min, sub_z_min;
            float sub_x_max, sub_y_max, sub_z_max;
            fin.read( ( char* )&sub_x_min, sizeof( float ) );
            fin.read( ( char* )&sub_y_min, sizeof( float ) );
            fin.read( ( char* )&sub_z_min, sizeof( float ) );
            fin.read( ( char* )&sub_x_max, sizeof( float ) );
            fin.read( ( char* )&sub_y_max, sizeof( float ) );
            fin.read( ( char* )&sub_z_max, sizeof( float ) );
            endian2::LittleToHost( &sub_x_min );
            endian2::LittleToHost( &sub_y_min );
            endian2::LittleToHost( &sub_z_min );
            endian2::LittleToHost( &sub_x_max );
            endian2::LittleToHost( &sub_y_max );
            endian2::LittleToHost( &sub_z_max );
            m_min_subvolume_coord[vl].set( sub_x_min, sub_y_min, sub_z_min );
            m_max_subvolume_coord[vl].set( sub_x_max, sub_y_max, sub_z_max );
        }

//        m_ingredient_step.clear();
//        for ( int32_t s = 0; s < m_number_steps; s++ )
//        {
//            m_ingredient_step.push_back( IngredientsStep() );
//            m_ingredient_step[s].m_ingredient.clear();
//            for ( int32_t i = 0; i < m_number_ingredients; i++ )
//            {
//                float min, max;
//                fin.read( ( char* )&min, sizeof( float ) );
//                fin.read( ( char* )&max, sizeof( float ) );
//                endian2::LittleToHost( &min );
//                endian2::LittleToHost( &max );
//
//                m_ingredient_step[s].m_ingredient.push_back( IngredientsMinMax() );
//                m_ingredient_step[s].m_ingredient[i].m_min = min;
//                m_ingredient_step[s].m_ingredient[i].m_max = max;
//            }
//        }
//
//        m_min_value = m_ingredient_step[0].m_ingredient[0].m_min;
//        m_max_value = m_ingredient_step[0].m_ingredient[0].m_max;
//        for ( int32_t s = 0; s < m_number_steps; s++ )
//        {
//            for ( int32_t i = 0; i < m_number_ingredients; i++ )
//            {
//                float min = m_ingredient_step[s].m_ingredient[i].m_min;
//                float max = m_ingredient_step[s].m_ingredient[i].m_max;
//                if ( min < m_min_value ) m_min_value = min;
//                if ( max > m_max_value ) m_max_value = max;
//            }
//        }
#endif
        fin.close();
    }
    /*
      std::cout << "numNodes:      " << numNodes       << std::endl;
      std::cout << "m_number_elements:   " << m_number_elements    << std::endl;
      std::cout << "m_elem_type:      " << m_elem_type       << std::endl;
      std::cout << "fileType:      " << fileType       << std::endl;
      std::cout << "numFiles:      " << numFiles       << std::endl;
      std::cout << "m_number_ingredients:" << m_number_ingredients << std::endl;
      std::cout << "numSteps:      " << numSteps       << std::endl;
      std::cout << "m_number_subvolumes: " << m_number_subvolumes  << std::endl;
    */

    m_file_path = filename;
    return 0;
}

//--------------------------------------------------------------------------

MultiVolumePropertyList::MultiVolumePropertyList():
    m_total_number_nodes( 0 ), m_total_number_elements( 0 ), m_total_number_files( 0 ),
    m_total_start_steps( 0 ), m_total_last_step( 0 ), m_total_number_steps( 0 ),
    m_total_number_subvolumes( 0 ), m_total_min_value( 0.f ), m_total_max_value( 0.f )
{
}

MultiVolumePropertyList::~MultiVolumePropertyList()
{
}

int MultiVolumePropertyList::loadPFL( const std::string& filename )
{
    m_list.clear();
    m_total_min_subvolume_coord.clear();
    m_total_max_subvolume_coord.clear();

    vismodule::File pfl( filename );
    if ( filename.size() < 4 || ! pfl.isExisted() ) return -1;
    if ( filename.substr( filename.size() - 3 ) == "pfi" )
    {
        MultiVolumeProperty mvp;
        if ( mvp.loadPFI( filename ) < 0 ) return -1;
        m_list.push_back( mvp );

        m_total_number_nodes = mvp.m_number_nodes;
        m_total_number_elements = mvp.m_number_elements;
        m_total_number_files = mvp.m_number_files;
        m_total_start_steps = mvp.m_start_step;
        m_total_last_step = mvp.m_end_steps;
        m_total_number_steps = mvp.m_number_steps;
        m_total_number_subvolumes = mvp.m_number_subvolumes;
        m_total_min_object_coord = mvp.m_min_object_coord;
        m_total_max_object_coord = mvp.m_max_object_coord;
        m_total_min_subvolume_coord = mvp.m_min_subvolume_coord;
        m_total_max_subvolume_coord = mvp.m_max_subvolume_coord;
        m_total_min_value = mvp.m_min_value;
        m_total_max_value = mvp.m_max_value;
        m_total_number_ingredients = mvp.m_number_ingredients;
        m_total_ingredient.resize( m_total_number_ingredients );
        for (int32_t i = 0; i < m_total_number_ingredients; i++)
        {
            m_total_ingredient[i].m_min = FLT_MAX;
            m_total_ingredient[i].m_max = -FLT_MAX;
        }
        return 1;
    }

    std::ifstream fin( filename.c_str(), std::ios::in );
    if ( ! fin ) return -1;
    std::string fbuff;
    if ( ! getline( fin, fbuff ) ) return -1;
    if ( fbuff != std::string( "#PBVR PFI FILES" ) ) return -1;
    while ( getline( fin, fbuff ) )
    {
        vismodule::File pfi( fbuff );
        if ( fbuff.size() < 4 ) continue;
        if ( fbuff.substr( fbuff.size() - 3 ) != "pfi" ) continue;
        if ( ! pfi.isExisted() )
        {
            std::string xpath = pfl.pathName( true ) + pfl.Separator() + pfi.filePath( false );
            vismodule::File xpfi( xpath );
            if ( ! xpfi.isExisted() ) continue;
            fbuff = xpath;
        }
        MultiVolumeProperty mvp;
        mvp.loadPFI( fbuff );

        if ( m_list.empty() )
        {
            m_total_number_nodes = mvp.m_number_nodes;
            m_total_number_elements = mvp.m_number_elements;
            m_total_number_files = mvp.m_number_files;
            m_total_start_steps = mvp.m_start_step;
            m_total_last_step = mvp.m_end_steps;
            m_total_number_steps = mvp.m_number_steps;
            m_total_number_subvolumes = mvp.m_number_subvolumes;
            m_total_min_object_coord = mvp.m_min_object_coord;
            m_total_max_object_coord = mvp.m_max_object_coord;
            m_total_min_subvolume_coord = mvp.m_min_subvolume_coord;
            m_total_max_subvolume_coord = mvp.m_max_subvolume_coord;
            m_total_min_value = mvp.m_min_value;
            m_total_max_value = mvp.m_max_value;
            m_total_number_ingredients = mvp.m_number_ingredients;
        }
        else
        {
            m_total_number_nodes += mvp.m_number_nodes;
            m_total_number_elements += mvp.m_number_elements;
            m_total_number_files += mvp.m_number_files;
            m_total_start_steps = std::min( m_total_start_steps, mvp.m_start_step );
            m_total_last_step = std::max( m_total_last_step, mvp.m_end_steps );
            m_total_number_steps = m_total_last_step - m_total_start_steps + 1;
            m_total_number_subvolumes += mvp.m_number_subvolumes;
            m_total_min_object_coord[0]
                = std::min( m_total_min_object_coord[0], mvp.m_min_object_coord[0] );
            m_total_min_object_coord[1]
                = std::min( m_total_min_object_coord[1], mvp.m_min_object_coord[1] );
            m_total_min_object_coord[2]
                = std::min( m_total_min_object_coord[2], mvp.m_min_object_coord[2] );
            m_total_max_object_coord[0]
                = std::max( m_total_max_object_coord[0], mvp.m_max_object_coord[0] );
            m_total_max_object_coord[1]
                = std::max( m_total_max_object_coord[1], mvp.m_max_object_coord[1] );
            m_total_max_object_coord[2]
                = std::max( m_total_max_object_coord[2], mvp.m_max_object_coord[2] );
            std::copy( mvp.m_min_subvolume_coord.begin(), mvp.m_min_subvolume_coord.end(),
                       std::back_inserter( m_total_min_subvolume_coord ) );
            std::copy( mvp.m_max_subvolume_coord.begin(), mvp.m_max_subvolume_coord.end(),
                       std::back_inserter( m_total_max_subvolume_coord ) );
            m_total_min_value = std::min( m_total_min_value, mvp.m_min_value );
            m_total_max_value = std::max( m_total_max_value, mvp.m_max_value );
            m_total_number_ingredients = std::max( m_total_number_ingredients, mvp.m_number_ingredients );
        }

        m_list.push_back( mvp );
    } // end of while()
    fin.close();
 
    m_total_ingredient.resize( m_total_number_ingredients );

    return m_list.size();
}

#ifdef EXTEND_FILE_FORMAT
int MultiVolumePropertyList::loadVtm( const std::string& filename )
{
    int last_time_step = 0;
    int time_step = 0;
    std::unordered_map<int, int> sub_volume_ids;
    std::unordered_map<int, int> sub_volume_counts;
    std::unordered_map<int, int> number_of_nodes;
    std::unordered_map<int, int> number_of_elements;
    std::unordered_map<int, int> number_of_ingredients;
    std::unordered_map<int, kvs::Vec3> min_external_coords;
    std::unordered_map<int, kvs::Vec3> max_external_coords;
    std::unordered_map<int, std::unordered_map<int, kvs::Vec3>> min_object_coords;
    std::unordered_map<int, std::unordered_map<int, kvs::Vec3>> max_object_coords;
    std::unordered_map<int, float> min_values;
    std::unordered_map<int, float> max_values;
    bool is_unstructured = false;
    bool is_structured = false;

    m_list.clear();
    m_total_min_subvolume_coord.clear();
    m_total_max_subvolume_coord.clear();

    kvs::ExtendedFileFormat::VtkXmlMultiBlock input_vtm( filename );

    // サブボリューム数を確認, ボリュームタイプを確認(Structured, Unstructured)
    for ( auto format : input_vtm.eachBlock() )
    {
        if ( auto input_vtu = dynamic_cast<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid*>( format.get() ) )
        {
            is_unstructured = true;
            for ( auto vtu : input_vtu->eachCellType() )
            {
                kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> importer( input_vtu );
                kvs::UnstructuredVolumeObject* object = &importer;
                auto cell_type = object->cellType();
                if ( sub_volume_counts.count( cell_type ) == 0 )
                {
                    sub_volume_counts[cell_type] = 1;
                    number_of_nodes[cell_type] = object->nnodes();
                    number_of_elements[cell_type] = object->ncells();
                    number_of_ingredients[cell_type] = object->veclen();
                }
                else
                {
                    sub_volume_counts[cell_type] = sub_volume_counts[cell_type] + 1;
                    number_of_nodes[cell_type] += object->nnodes();
                    number_of_elements[cell_type] += object->ncells();
                }
            }
        }
        else if ( auto input_vti = dynamic_cast<kvs::ExtendedFileFormat::VtkXmlImageData*>( format.get() ) )
        {
            is_structured = true;
            kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlImageData> importer( input_vti );
            kvs::StructuredVolumeObject* object = &importer;
            auto cell_type = 7;
            kvs::Vec3ui resolution = object->resolution();

            if ( sub_volume_counts.count( cell_type ) == 0 )
            {
                sub_volume_counts[cell_type] = 1;
                number_of_nodes[cell_type] = static_cast<int>( resolution[0] * resolution[1] * resolution[2] );
                number_of_elements[cell_type] = static_cast<int>( ( resolution[0] - 1 ) * ( resolution[1] - 1 ) * ( resolution[2] - 1 ) );
                number_of_ingredients[cell_type] = object->veclen();
            }
            else
            {
                sub_volume_counts[cell_type] = sub_volume_counts[cell_type] + 1;
                number_of_nodes[cell_type] += static_cast<int>( resolution[0] * resolution[1] * resolution[2] );
                number_of_elements[cell_type] += static_cast<int>( ( resolution[0] - 1 ) * ( resolution[1] - 1 ) * ( resolution[2] - 1 ) );
            }     
        }
    }

    // ボリュームタイプが混ざっている場合はエラー出力
    if ( is_structured && is_unstructured )
    {
        visModuleMessageError("Mixed StructuredGrid and UnstructuredGrid files are not supported.");
    }

    // サブボリュームIDを初期化
    for ( auto& e : sub_volume_counts )
    {
        sub_volume_ids[e.first] = 0;
    }

    for ( auto format : input_vtm.eachBlock() )
    {
        if ( auto input_vtu = dynamic_cast<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid*>( format.get() ) )
        {
            for ( auto vtu : input_vtu->eachCellType() )
            {
                int sub_volume_id;
                kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> importer( input_vtu );
                kvs::UnstructuredVolumeObject* object = &importer;
                
                auto cell_type = object->cellType();

                if ( min_external_coords.count(cell_type) == 0 )
                {
                    min_external_coords[cell_type][0] = object->minExternalCoord()[0];
                    min_external_coords[cell_type][1] = object->minExternalCoord()[1];
                    min_external_coords[cell_type][2] = object->minExternalCoord()[2];
                    max_external_coords[cell_type][0] = object->maxExternalCoord()[0];
                    max_external_coords[cell_type][1] = object->maxExternalCoord()[1];
                    max_external_coords[cell_type][2] = object->maxExternalCoord()[2];
                    min_values[cell_type] = object->minValue();
                    max_values[cell_type] = object->maxValue();
                }
                else
                {
                    min_external_coords[cell_type][0] = std::min(min_external_coords[cell_type][0], object->minExternalCoord()[0]);
                    min_external_coords[cell_type][1] = std::min(min_external_coords[cell_type][1], object->minExternalCoord()[1]);
                    min_external_coords[cell_type][2] = std::min(min_external_coords[cell_type][2], object->minExternalCoord()[2]);
                    max_external_coords[cell_type][0] = std::max(max_external_coords[cell_type][0], object->maxExternalCoord()[0]);
                    max_external_coords[cell_type][1] = std::max(max_external_coords[cell_type][1], object->maxExternalCoord()[1]);
                    max_external_coords[cell_type][2] = std::max(max_external_coords[cell_type][2], object->maxExternalCoord()[2]);
                    min_values[cell_type] = std::min(min_values[cell_type], float(object->minValue()));
                    max_values[cell_type] = std::max(max_values[cell_type], float(object->maxValue()));
                }

                sub_volume_id = sub_volume_ids[cell_type];

                if ( min_object_coords[cell_type].count(sub_volume_ids[cell_type]) == 0 )
                {
                    min_object_coords[cell_type][sub_volume_id][0] = object->minObjectCoord()[0];
                    min_object_coords[cell_type][sub_volume_id][1] = object->minObjectCoord()[1];
                    min_object_coords[cell_type][sub_volume_id][2] = object->minObjectCoord()[2];
                    max_object_coords[cell_type][sub_volume_id][0] = object->maxObjectCoord()[0];
                    max_object_coords[cell_type][sub_volume_id][1] = object->maxObjectCoord()[1];
                    max_object_coords[cell_type][sub_volume_id][2] = object->maxObjectCoord()[2];
                }
                else
                {
                    min_object_coords[cell_type][sub_volume_id][0] = std::min(min_object_coords[cell_type][sub_volume_id][0], object->minObjectCoord()[0]);
                    min_object_coords[cell_type][sub_volume_id][1] = std::min(min_object_coords[cell_type][sub_volume_id][1], object->minObjectCoord()[1]);
                    min_object_coords[cell_type][sub_volume_id][2] = std::min(min_object_coords[cell_type][sub_volume_id][2], object->minObjectCoord()[2]);
                    max_object_coords[cell_type][sub_volume_id][0] = std::max(max_object_coords[cell_type][sub_volume_id][0], object->maxObjectCoord()[0]);
                    max_object_coords[cell_type][sub_volume_id][1] = std::max(max_object_coords[cell_type][sub_volume_id][1], object->maxObjectCoord()[1]);
                    max_object_coords[cell_type][sub_volume_id][2] = std::max(max_object_coords[cell_type][sub_volume_id][2], object->maxObjectCoord()[2]);
                }

                ++sub_volume_ids[cell_type];
            }
        }
        else if ( auto input_vti = dynamic_cast<kvs::ExtendedFileFormat::VtkXmlImageData*>( format.get() ) )
        {
            int sub_volume_id;
            kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlImageData> importer( input_vti );
            kvs::StructuredVolumeObject* object = &importer;
            object->setMinMaxExternalCoords( object->minObjectCoord(), object->maxObjectCoord() );

            auto cell_type = 7;

            if ( min_external_coords.count(cell_type) == 0 )
            {
                min_external_coords[cell_type][0] = object->minExternalCoord()[0];
                min_external_coords[cell_type][1] = object->minExternalCoord()[1];
                min_external_coords[cell_type][2] = object->minExternalCoord()[2];
                max_external_coords[cell_type][0] = object->maxExternalCoord()[0];
                max_external_coords[cell_type][1] = object->maxExternalCoord()[1];
                max_external_coords[cell_type][2] = object->maxExternalCoord()[2];
                min_values[cell_type] = object->minValue();
                max_values[cell_type] = object->maxValue();
            }
            else
            {
                min_external_coords[cell_type][0] = std::min(min_external_coords[cell_type][0], object->minExternalCoord()[0]);
                min_external_coords[cell_type][1] = std::min(min_external_coords[cell_type][1], object->minExternalCoord()[1]);
                min_external_coords[cell_type][2] = std::min(min_external_coords[cell_type][2], object->minExternalCoord()[2]);
                max_external_coords[cell_type][0] = std::max(max_external_coords[cell_type][0], object->maxExternalCoord()[0]);
                max_external_coords[cell_type][1] = std::max(max_external_coords[cell_type][1], object->maxExternalCoord()[1]);
                max_external_coords[cell_type][2] = std::max(max_external_coords[cell_type][2], object->maxExternalCoord()[2]);
                min_values[cell_type] = std::min(min_values[cell_type], float(object->minValue()));
                max_values[cell_type] = std::max(max_values[cell_type], float(object->maxValue()));
            }

            sub_volume_id = sub_volume_ids[cell_type];

            if ( min_object_coords[cell_type].count(sub_volume_ids[cell_type]) == 0 )
            {
                min_object_coords[cell_type][sub_volume_id][0] = object->minObjectCoord()[0];
                min_object_coords[cell_type][sub_volume_id][1] = object->minObjectCoord()[1];
                min_object_coords[cell_type][sub_volume_id][2] = object->minObjectCoord()[2];
                max_object_coords[cell_type][sub_volume_id][0] = object->maxObjectCoord()[0];
                max_object_coords[cell_type][sub_volume_id][1] = object->maxObjectCoord()[1];
                max_object_coords[cell_type][sub_volume_id][2] = object->maxObjectCoord()[2];
            }
            else
            {
                min_object_coords[cell_type][sub_volume_id][0] = std::min(min_object_coords[cell_type][sub_volume_id][0], object->minObjectCoord()[0]);
                min_object_coords[cell_type][sub_volume_id][1] = std::min(min_object_coords[cell_type][sub_volume_id][1], object->minObjectCoord()[1]);
                min_object_coords[cell_type][sub_volume_id][2] = std::min(min_object_coords[cell_type][sub_volume_id][2], object->minObjectCoord()[2]);
                max_object_coords[cell_type][sub_volume_id][0] = std::max(max_object_coords[cell_type][sub_volume_id][0], object->maxObjectCoord()[0]);
                max_object_coords[cell_type][sub_volume_id][1] = std::max(max_object_coords[cell_type][sub_volume_id][1], object->maxObjectCoord()[1]);
                max_object_coords[cell_type][sub_volume_id][2] = std::max(max_object_coords[cell_type][sub_volume_id][2], object->maxObjectCoord()[2]);
            }
            ++sub_volume_ids[cell_type];
        }
    }

    for (auto& e: sub_volume_ids)
    {
        MultiVolumeProperty mvp;
        auto cell_type = e.first;
        int file_type = 0;

        if ( is_structured ) file_type = 3; 
        if ( is_unstructured ) file_type = 4;

        mvp.m_number_nodes = number_of_nodes[cell_type];
        mvp.m_number_elements = number_of_elements[cell_type];
        mvp.m_elem_type = cell_type;
        mvp.m_file_type = file_type;
        mvp.m_number_files = sub_volume_counts[cell_type] * (last_time_step + 1);
        mvp.m_number_ingredients = number_of_ingredients[cell_type];
        mvp.m_start_step = 0;
        mvp.m_end_steps = last_time_step;
        mvp.m_number_subvolumes = sub_volume_counts[cell_type];
        float x_min, y_min, z_min;
        float x_max, y_max, z_max;
        x_min = min_external_coords[cell_type][0];
        y_min = min_external_coords[cell_type][1];
        z_min = min_external_coords[cell_type][2];
        x_max = max_external_coords[cell_type][0];
        y_max = max_external_coords[cell_type][1];
        z_max = max_external_coords[cell_type][2];
        mvp.m_min_object_coord.set(x_min, y_min, z_min);
        mvp.m_max_object_coord.set(x_max, y_max, z_max);
        mvp.m_number_steps = mvp.m_end_steps - mvp.m_start_step + 1;
        mvp.m_min_subvolume_coord.resize(mvp.m_number_subvolumes);
        mvp.m_max_subvolume_coord.resize(mvp.m_number_subvolumes);
        mvp.m_file_path = filename;
        mvp.m_min_value = min_values[cell_type];
        mvp.m_max_value = max_values[cell_type];

        for (int sub_volume_id = 0; sub_volume_id < sub_volume_counts[cell_type]; sub_volume_id++)
        {
            float sub_x_min, sub_y_min, sub_z_min;
            float sub_x_max, sub_y_max, sub_z_max;
            sub_x_min = min_object_coords[cell_type][sub_volume_id][0];
            sub_y_min = min_object_coords[cell_type][sub_volume_id][1];
            sub_z_min = min_object_coords[cell_type][sub_volume_id][2];
            sub_x_max = max_object_coords[cell_type][sub_volume_id][0];
            sub_y_max = max_object_coords[cell_type][sub_volume_id][1];
            sub_z_max = max_object_coords[cell_type][sub_volume_id][2];
            mvp.m_min_subvolume_coord[sub_volume_id].set(sub_x_min, sub_y_min, sub_z_min);
            mvp.m_max_subvolume_coord[sub_volume_id].set(sub_x_max, sub_y_max, sub_z_max);
        }

// for debug
#if 0
        std::cout << "==================== cell type:" << cell_type << " start =========================" << std::endl;
        std::cout << "m_number_nodes:" << mvp.m_number_nodes << std::endl;
        std::cout << "m_number_elements:" << mvp.m_number_elements << std::endl;
        std::cout << "m_number_files:" << mvp.m_number_files << std::endl;
        std::cout << "m_start_step:" << mvp.m_start_step << std::endl;
        std::cout << "m_end_steps:" << mvp.m_end_steps << std::endl;
        std::cout << "m_number_steps:" << mvp.m_number_steps << std::endl;
        std::cout << "m_number_subvolumes:" << mvp.m_number_subvolumes << std::endl;
        std::cout << "m_min_object_coord[0]:" << mvp.m_min_object_coord[0] << std::endl;
        std::cout << "m_min_object_coord[1]:" << mvp.m_min_object_coord[1] << std::endl;
        std::cout << "m_min_object_coord[2]:" << mvp.m_min_object_coord[2] << std::endl;
        std::cout << "m_max_object_coord[0]:" << mvp.m_max_object_coord[0] << std::endl;
        std::cout << "m_max_object_coord[1]:" << mvp.m_max_object_coord[1] << std::endl;
        std::cout << "m_max_object_coord[2]:" << mvp.m_max_object_coord[2] << std::endl;
        std::cout << "m_min_subvolume_coord[0]:" << mvp.m_min_subvolume_coord[0][0] << std::endl;
        std::cout << "m_min_subvolume_coord[1]:" << mvp.m_min_subvolume_coord[0][1] << std::endl;
        std::cout << "m_min_subvolume_coord[2]:" << mvp.m_min_subvolume_coord[0][2] << std::endl;
        std::cout << "m_max_subvolume_coord[0]:" << mvp.m_max_subvolume_coord[0][0] << std::endl;
        std::cout << "m_max_subvolume_coord[1]:" << mvp.m_max_subvolume_coord[0][1] << std::endl;
        std::cout << "m_max_subvolume_coord[2]:" << mvp.m_max_subvolume_coord[0][2] << std::endl;
        std::cout << "m_min_value:" << mvp.m_min_value << std::endl;
        std::cout << "m_max_value:" << mvp.m_max_value << std::endl;
        std::cout << "m_number_ingredients:" << mvp.m_number_ingredients << std::endl;
        std::cout << "==================== cell type:" << cell_type << " end =========================" << std::endl;
#endif

        if ( this->m_list.empty() )
        {
            this->m_total_number_nodes = mvp.m_number_nodes;
            this->m_total_number_elements = mvp.m_number_elements;
            this->m_total_number_files = mvp.m_number_files;
            this->m_total_start_steps = mvp.m_start_step;
            this->m_total_last_step = mvp.m_end_steps;
            this->m_total_number_steps = mvp.m_number_steps;
            this->m_total_number_subvolumes = mvp.m_number_subvolumes;
            this->m_total_min_object_coord = mvp.m_min_object_coord;
            this->m_total_max_object_coord = mvp.m_max_object_coord;
            this->m_total_min_subvolume_coord = mvp.m_min_subvolume_coord;
            this->m_total_max_subvolume_coord = mvp.m_max_subvolume_coord;
            this->m_total_min_value = mvp.m_min_value;
            this->m_total_max_value = mvp.m_max_value;
            this->m_total_number_ingredients = mvp.m_number_ingredients;
        }
        else
        {
            this->m_total_number_nodes += mvp.m_number_nodes;
            this->m_total_number_elements += mvp.m_number_elements;
            this->m_total_number_files += mvp.m_number_files;
            this->m_total_start_steps = std::min(this->m_total_start_steps, mvp.m_start_step);
            this->m_total_last_step = std::max(this->m_total_last_step, mvp.m_end_steps);
            this->m_total_number_steps = this->m_total_last_step - this->m_total_start_steps + 1;
            this->m_total_number_subvolumes += mvp.m_number_subvolumes;
            this->m_total_min_object_coord[0] = std::min(this->m_total_min_object_coord[0], mvp.m_min_object_coord[0]);
            this->m_total_min_object_coord[1] = std::min(this->m_total_min_object_coord[1], mvp.m_min_object_coord[1]);
            this->m_total_min_object_coord[2] = std::min(this->m_total_min_object_coord[2], mvp.m_min_object_coord[2]);
            this->m_total_max_object_coord[0] = std::max(this->m_total_max_object_coord[0], mvp.m_max_object_coord[0]);
            this->m_total_max_object_coord[1] = std::max(this->m_total_max_object_coord[1], mvp.m_max_object_coord[1]);
            this->m_total_max_object_coord[2] = std::max(this->m_total_max_object_coord[2], mvp.m_max_object_coord[2]);
            std::copy(mvp.m_min_subvolume_coord.begin(), mvp.m_min_subvolume_coord.end(), std::back_inserter(this->m_total_min_subvolume_coord));
            std::copy(mvp.m_max_subvolume_coord.begin(), mvp.m_max_subvolume_coord.end(), std::back_inserter(this->m_total_max_subvolume_coord));
            this->m_total_min_value = std::min(this->m_total_min_value, mvp.m_min_value);
            this->m_total_max_value = std::max(this->m_total_max_value, mvp.m_max_value);
            this->m_total_number_ingredients = std::max(this->m_total_number_ingredients, mvp.m_number_ingredients);
        }
        this->m_list.push_back(mvp);
    }

    m_total_ingredient.resize( m_total_number_ingredients );

    return m_list.size();    
}

int MultiVolumePropertyList::loadSeriesVtm( const std::string& filename )
{
    namespace fs = std::filesystem;
    fs::path filepath = filename;
    kvs::ExtendedFileFormat::NumeralSequenceFiles<kvs::ExtendedFileFormat::VtkXmlMultiBlock> time_series( filepath.generic_string() );
    int last_time_step = time_series.numberOfFiles() - 1;
    int time_step = 0;
    std::unordered_map<int, int> sub_volume_ids;
    std::unordered_map<int, int> sub_volume_counts;
    std::unordered_map<int, int> number_of_nodes;
    std::unordered_map<int, int> number_of_elements;
    std::unordered_map<int, int> number_of_ingredients;
    std::unordered_map<int, kvs::Vec3> min_external_coords;
    std::unordered_map<int, kvs::Vec3> max_external_coords;
    std::unordered_map<int, std::unordered_map<int, kvs::Vec3>> min_object_coords;
    std::unordered_map<int, std::unordered_map<int, kvs::Vec3>> max_object_coords;
    std::unordered_map<int, float> min_values;
    std::unordered_map<int, float> max_values;
    bool is_unstructured = false;
    bool is_structured = false;
    
    m_list.clear();
    m_total_min_subvolume_coord.clear();
    m_total_max_subvolume_coord.clear();
    
    for ( auto input_vtm : time_series.eachTimeStep() )
    {
        if ( time_step == 0 )
        {
            for ( auto format : input_vtm.eachBlock() )
            {
                if ( auto input_vtu = dynamic_cast<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid*>( format.get() ) )
                {
                    is_unstructured = true;
                    for ( auto vtu : input_vtu->eachCellType() )
                    {
                        kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> importer( input_vtu );
                        kvs::UnstructuredVolumeObject* object = &importer;
                        auto cell_type = object->cellType();

                        if ( sub_volume_counts.count( cell_type ) == 0 )
                        {
                            sub_volume_counts[cell_type] = 1;
                            number_of_nodes[cell_type] = object->nnodes();
                            number_of_elements[cell_type] = object->ncells();
                            number_of_ingredients[cell_type] = object->veclen();
                        }
                        else
                        {
                            sub_volume_counts[cell_type] = sub_volume_counts[cell_type] + 1;
                            number_of_nodes[cell_type] += object->nnodes();
                            number_of_elements[cell_type] += object->ncells();
                        }
                    }
                }
                else if ( auto input_vti = dynamic_cast<kvs::ExtendedFileFormat::VtkXmlImageData*>( format.get() ) )
                {
                    is_structured = true;
                    kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlImageData> importer( input_vti );
                    kvs::StructuredVolumeObject* object = &importer;
                    auto cell_type = 7;
                    kvs::Vec3ui resolution = object->resolution();

                    if ( sub_volume_counts.count( cell_type ) == 0 )
                    {
                        sub_volume_counts[cell_type] = 1;
                        number_of_nodes[cell_type] = static_cast<int>( resolution[0] * resolution[1] * resolution[2] );
                        number_of_elements[cell_type] = static_cast<int>( ( resolution[0] - 1 ) * ( resolution[1] - 1 ) * ( resolution[2] - 1 ) );
                        number_of_ingredients[cell_type] = object->veclen();
                    }
                    else
                    {
                        sub_volume_counts[cell_type] = sub_volume_counts[cell_type] + 1;
                        number_of_nodes[cell_type] += static_cast<int>( resolution[0] * resolution[1] * resolution[2] );
                        number_of_elements[cell_type] += static_cast<int>( ( resolution[0] - 1 ) * ( resolution[1] - 1 ) * ( resolution[2] - 1 ) );
                    }
                }
            }
        }

        if ( is_structured && is_unstructured )
        {
            visModuleMessageError("Mixed StructuredGrid and UnstructuredGrid files are not supported.");
        }
        
        // Two-pass
        for ( auto& e : sub_volume_counts )
        {
            sub_volume_ids[e.first] = 0;
        }	
        
        for ( auto format : input_vtm.eachBlock() )
        {
            if ( auto input_vtu = dynamic_cast<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid*>( format.get() ) )
            {
                for ( auto vtu : input_vtu->eachCellType() )
                {
                    int sub_volume_id;
                    kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> importer( input_vtu );
                    kvs::UnstructuredVolumeObject* object = &importer;
                    
                    auto cell_type = object->cellType();

                    if ( min_external_coords.count(cell_type) == 0 )
                    {
                        min_external_coords[cell_type][0] = object->minExternalCoord()[0];
                        min_external_coords[cell_type][1] = object->minExternalCoord()[1];
                        min_external_coords[cell_type][2] = object->minExternalCoord()[2];
                        max_external_coords[cell_type][0] = object->maxExternalCoord()[0];
                        max_external_coords[cell_type][1] = object->maxExternalCoord()[1];
                        max_external_coords[cell_type][2] = object->maxExternalCoord()[2];
                        min_values[cell_type] = object->minValue();
                        max_values[cell_type] = object->maxValue();
                    }
                    else
                    {
                        min_external_coords[cell_type][0] = std::min(min_external_coords[cell_type][0], object->minExternalCoord()[0]);
                        min_external_coords[cell_type][1] = std::min(min_external_coords[cell_type][1], object->minExternalCoord()[1]);
                        min_external_coords[cell_type][2] = std::min(min_external_coords[cell_type][2], object->minExternalCoord()[2]);
                        max_external_coords[cell_type][0] = std::max(max_external_coords[cell_type][0], object->maxExternalCoord()[0]);
                        max_external_coords[cell_type][1] = std::max(max_external_coords[cell_type][1], object->maxExternalCoord()[1]);
                        max_external_coords[cell_type][2] = std::max(max_external_coords[cell_type][2], object->maxExternalCoord()[2]);
                        min_values[cell_type] = std::min(min_values[cell_type], float(object->minValue()));
                        max_values[cell_type] = std::max(max_values[cell_type], float(object->maxValue()));
                    }

		            sub_volume_id = sub_volume_ids[cell_type];

                    if ( min_object_coords[cell_type].count(sub_volume_ids[cell_type]) == 0 )
                    {
                        min_object_coords[cell_type][sub_volume_id][0] = object->minObjectCoord()[0];
                        min_object_coords[cell_type][sub_volume_id][1] = object->minObjectCoord()[1];
                        min_object_coords[cell_type][sub_volume_id][2] = object->minObjectCoord()[2];
                        max_object_coords[cell_type][sub_volume_id][0] = object->maxObjectCoord()[0];
                        max_object_coords[cell_type][sub_volume_id][1] = object->maxObjectCoord()[1];
                        max_object_coords[cell_type][sub_volume_id][2] = object->maxObjectCoord()[2];
                    }
                    else
                    {
                        min_object_coords[cell_type][sub_volume_id][0] = std::min(min_object_coords[cell_type][sub_volume_id][0], object->minObjectCoord()[0]);
                        min_object_coords[cell_type][sub_volume_id][1] = std::min(min_object_coords[cell_type][sub_volume_id][1], object->minObjectCoord()[1]);
                        min_object_coords[cell_type][sub_volume_id][2] = std::min(min_object_coords[cell_type][sub_volume_id][2], object->minObjectCoord()[2]);
                        max_object_coords[cell_type][sub_volume_id][0] = std::max(max_object_coords[cell_type][sub_volume_id][0], object->maxObjectCoord()[0]);
                        max_object_coords[cell_type][sub_volume_id][1] = std::max(max_object_coords[cell_type][sub_volume_id][1], object->maxObjectCoord()[1]);
                        max_object_coords[cell_type][sub_volume_id][2] = std::max(max_object_coords[cell_type][sub_volume_id][2], object->maxObjectCoord()[2]);
                    }

		            ++sub_volume_ids[cell_type];
                }
            }
            else if ( auto input_vti = dynamic_cast<kvs::ExtendedFileFormat::VtkXmlImageData*>( format.get() ) )
            {
                int sub_volume_id;
                kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlImageData> importer( input_vti );
                kvs::StructuredVolumeObject* object = &importer;
                object->setMinMaxExternalCoords( object->minObjectCoord(), object->maxObjectCoord() );

                auto cell_type = 7;

                if ( min_external_coords.count(cell_type) == 0 )
                {
                    min_external_coords[cell_type][0] = object->minExternalCoord()[0];
                    min_external_coords[cell_type][1] = object->minExternalCoord()[1];
                    min_external_coords[cell_type][2] = object->minExternalCoord()[2];
                    max_external_coords[cell_type][0] = object->maxExternalCoord()[0];
                    max_external_coords[cell_type][1] = object->maxExternalCoord()[1];
                    max_external_coords[cell_type][2] = object->maxExternalCoord()[2];
                    min_values[cell_type] = object->minValue();
                    max_values[cell_type] = object->maxValue();
                }
                else
                {
                    min_external_coords[cell_type][0] = std::min(min_external_coords[cell_type][0], object->minExternalCoord()[0]);
                    min_external_coords[cell_type][1] = std::min(min_external_coords[cell_type][1], object->minExternalCoord()[1]);
                    min_external_coords[cell_type][2] = std::min(min_external_coords[cell_type][2], object->minExternalCoord()[2]);
                    max_external_coords[cell_type][0] = std::max(max_external_coords[cell_type][0], object->maxExternalCoord()[0]);
                    max_external_coords[cell_type][1] = std::max(max_external_coords[cell_type][1], object->maxExternalCoord()[1]);
                    max_external_coords[cell_type][2] = std::max(max_external_coords[cell_type][2], object->maxExternalCoord()[2]);
                    min_values[cell_type] = std::min(min_values[cell_type], float(object->minValue()));
                    max_values[cell_type] = std::max(max_values[cell_type], float(object->maxValue()));
                }

                sub_volume_id = sub_volume_ids[cell_type];

                if ( min_object_coords[cell_type].count(sub_volume_ids[cell_type]) == 0 )
                {
                    min_object_coords[cell_type][sub_volume_id][0] = object->minObjectCoord()[0];
                    min_object_coords[cell_type][sub_volume_id][1] = object->minObjectCoord()[1];
                    min_object_coords[cell_type][sub_volume_id][2] = object->minObjectCoord()[2];
                    max_object_coords[cell_type][sub_volume_id][0] = object->maxObjectCoord()[0];
                    max_object_coords[cell_type][sub_volume_id][1] = object->maxObjectCoord()[1];
                    max_object_coords[cell_type][sub_volume_id][2] = object->maxObjectCoord()[2];
                }
                else
                {
                    min_object_coords[cell_type][sub_volume_id][0] = std::min(min_object_coords[cell_type][sub_volume_id][0], object->minObjectCoord()[0]);
                    min_object_coords[cell_type][sub_volume_id][1] = std::min(min_object_coords[cell_type][sub_volume_id][1], object->minObjectCoord()[1]);
                    min_object_coords[cell_type][sub_volume_id][2] = std::min(min_object_coords[cell_type][sub_volume_id][2], object->minObjectCoord()[2]);
                    max_object_coords[cell_type][sub_volume_id][0] = std::max(max_object_coords[cell_type][sub_volume_id][0], object->maxObjectCoord()[0]);
                    max_object_coords[cell_type][sub_volume_id][1] = std::max(max_object_coords[cell_type][sub_volume_id][1], object->maxObjectCoord()[1]);
                    max_object_coords[cell_type][sub_volume_id][2] = std::max(max_object_coords[cell_type][sub_volume_id][2], object->maxObjectCoord()[2]);
                }
                ++sub_volume_ids[cell_type];
            }
        }
        ++time_step;
    }

    for (auto& e: sub_volume_ids)
    {
        MultiVolumeProperty mvp;
        auto cell_type = e.first;
        int file_type = 0;

        if ( is_structured ) file_type = 3; 
        if ( is_unstructured ) file_type = 4;

        mvp.m_number_nodes = number_of_nodes[cell_type];
        mvp.m_number_elements = number_of_elements[cell_type];
        mvp.m_elem_type = cell_type;
        mvp.m_file_type = file_type;
        mvp.m_number_files = sub_volume_counts[cell_type] * (last_time_step + 1);
        mvp.m_number_ingredients = number_of_ingredients[cell_type];
        mvp.m_start_step = 0;
        mvp.m_end_steps = last_time_step;
        mvp.m_number_subvolumes = sub_volume_counts[cell_type];
        float x_min, y_min, z_min;
        float x_max, y_max, z_max;
        x_min = min_external_coords[cell_type][0];
        y_min = min_external_coords[cell_type][1];
        z_min = min_external_coords[cell_type][2];
        x_max = max_external_coords[cell_type][0];
        y_max = max_external_coords[cell_type][1];
        z_max = max_external_coords[cell_type][2];
        mvp.m_min_object_coord.set(x_min, y_min, z_min);
        mvp.m_max_object_coord.set(x_max, y_max, z_max);
        mvp.m_number_steps = mvp.m_end_steps - mvp.m_start_step + 1;
        mvp.m_min_subvolume_coord.resize(mvp.m_number_subvolumes);
        mvp.m_max_subvolume_coord.resize(mvp.m_number_subvolumes);
        mvp.m_file_path = filename;
        mvp.m_min_value = min_values[cell_type];
        mvp.m_max_value = max_values[cell_type];

        for (int sub_volume_id = 0; sub_volume_id < sub_volume_counts[cell_type]; sub_volume_id++)
        {
            float sub_x_min, sub_y_min, sub_z_min;
            float sub_x_max, sub_y_max, sub_z_max;
            sub_x_min = min_object_coords[cell_type][sub_volume_id][0];
            sub_y_min = min_object_coords[cell_type][sub_volume_id][1];
            sub_z_min = min_object_coords[cell_type][sub_volume_id][2];
            sub_x_max = max_object_coords[cell_type][sub_volume_id][0];
            sub_y_max = max_object_coords[cell_type][sub_volume_id][1];
            sub_z_max = max_object_coords[cell_type][sub_volume_id][2];
            mvp.m_min_subvolume_coord[sub_volume_id].set(sub_x_min, sub_y_min, sub_z_min);
            mvp.m_max_subvolume_coord[sub_volume_id].set(sub_x_max, sub_y_max, sub_z_max);
        }

// for debug
#if 0
        std::cout << "==================== cell type:" << cell_type << " start =========================" << std::endl;
        std::cout << "m_number_nodes:" << mvp.m_number_nodes << std::endl;
        std::cout << "m_number_elements:" << mvp.m_number_elements << std::endl;
        std::cout << "m_number_files:" << mvp.m_number_files << std::endl;
        std::cout << "m_start_step:" << mvp.m_start_step << std::endl;
        std::cout << "m_end_steps:" << mvp.m_end_steps << std::endl;
        std::cout << "m_number_steps:" << mvp.m_number_steps << std::endl;
        std::cout << "m_number_subvolumes:" << mvp.m_number_subvolumes << std::endl;
        std::cout << "m_min_object_coord[0]:" << mvp.m_min_object_coord[0] << std::endl;
        std::cout << "m_min_object_coord[1]:" << mvp.m_min_object_coord[1] << std::endl;
        std::cout << "m_min_object_coord[2]:" << mvp.m_min_object_coord[2] << std::endl;
        std::cout << "m_max_object_coord[0]:" << mvp.m_max_object_coord[0] << std::endl;
        std::cout << "m_max_object_coord[1]:" << mvp.m_max_object_coord[1] << std::endl;
        std::cout << "m_max_object_coord[2]:" << mvp.m_max_object_coord[2] << std::endl;
        std::cout << "m_min_subvolume_coord[0]:" << mvp.m_min_subvolume_coord[0][0] << std::endl;
        std::cout << "m_min_subvolume_coord[1]:" << mvp.m_min_subvolume_coord[0][1] << std::endl;
        std::cout << "m_min_subvolume_coord[2]:" << mvp.m_min_subvolume_coord[0][2] << std::endl;
        std::cout << "m_max_subvolume_coord[0]:" << mvp.m_max_subvolume_coord[0][0] << std::endl;
        std::cout << "m_max_subvolume_coord[1]:" << mvp.m_max_subvolume_coord[0][1] << std::endl;
        std::cout << "m_max_subvolume_coord[2]:" << mvp.m_max_subvolume_coord[0][2] << std::endl;
        std::cout << "m_min_value:" << mvp.m_min_value << std::endl;
        std::cout << "m_max_value:" << mvp.m_max_value << std::endl;
        std::cout << "m_number_ingredients:" << mvp.m_number_ingredients << std::endl;
        std::cout << "==================== cell type:" << cell_type << " end =========================" << std::endl;
#endif

        if ( this->m_list.empty() )
        {
            this->m_total_number_nodes = mvp.m_number_nodes;
            this->m_total_number_elements = mvp.m_number_elements;
            this->m_total_number_files = mvp.m_number_files;
            this->m_total_start_steps = mvp.m_start_step;
            this->m_total_last_step = mvp.m_end_steps;
            this->m_total_number_steps = mvp.m_number_steps;
            this->m_total_number_subvolumes = mvp.m_number_subvolumes;
            this->m_total_min_object_coord = mvp.m_min_object_coord;
            this->m_total_max_object_coord = mvp.m_max_object_coord;
            this->m_total_min_subvolume_coord = mvp.m_min_subvolume_coord;
            this->m_total_max_subvolume_coord = mvp.m_max_subvolume_coord;
            this->m_total_min_value = mvp.m_min_value;
            this->m_total_max_value = mvp.m_max_value;
            this->m_total_number_ingredients = mvp.m_number_ingredients;
        }
        else
        {
            this->m_total_number_nodes += mvp.m_number_nodes;
            this->m_total_number_elements += mvp.m_number_elements;
            this->m_total_number_files += mvp.m_number_files;
            this->m_total_start_steps = std::min(this->m_total_start_steps, mvp.m_start_step);
            this->m_total_last_step = std::max(this->m_total_last_step, mvp.m_end_steps);
            this->m_total_number_steps = this->m_total_last_step - this->m_total_start_steps + 1;
            this->m_total_number_subvolumes += mvp.m_number_subvolumes;
            this->m_total_min_object_coord[0] = std::min(this->m_total_min_object_coord[0], mvp.m_min_object_coord[0]);
            this->m_total_min_object_coord[1] = std::min(this->m_total_min_object_coord[1], mvp.m_min_object_coord[1]);
            this->m_total_min_object_coord[2] = std::min(this->m_total_min_object_coord[2], mvp.m_min_object_coord[2]);
            this->m_total_max_object_coord[0] = std::max(this->m_total_max_object_coord[0], mvp.m_max_object_coord[0]);
            this->m_total_max_object_coord[1] = std::max(this->m_total_max_object_coord[1], mvp.m_max_object_coord[1]);
            this->m_total_max_object_coord[2] = std::max(this->m_total_max_object_coord[2], mvp.m_max_object_coord[2]);
            std::copy(mvp.m_min_subvolume_coord.begin(), mvp.m_min_subvolume_coord.end(), std::back_inserter(this->m_total_min_subvolume_coord));
            std::copy(mvp.m_max_subvolume_coord.begin(), mvp.m_max_subvolume_coord.end(), std::back_inserter(this->m_total_max_subvolume_coord));
            this->m_total_min_value = std::min(this->m_total_min_value, mvp.m_min_value);
            this->m_total_max_value = std::max(this->m_total_max_value, mvp.m_max_value);
            this->m_total_number_ingredients = std::max(this->m_total_number_ingredients, mvp.m_number_ingredients);
        }
        this->m_list.push_back(mvp);
    }

    m_total_ingredient.resize( m_total_number_ingredients );

    return m_list.size();
}

int MultiVolumePropertyList::loadVtu( const std::string& filename )
{
    kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid input_vtu( filename );
    int last_time_step = 0;
    int time_step = 0;
    std::unordered_map<int, int> sub_volume_ids;
    std::unordered_map<int, int> sub_volume_counts;
    std::unordered_map<int, int> number_of_nodes;
    std::unordered_map<int, int> number_of_elements;
    std::unordered_map<int, int> number_of_ingredients;
    std::unordered_map<int, kvs::Vec3> min_external_coords;
    std::unordered_map<int, kvs::Vec3> max_external_coords;
    std::unordered_map<int, kvs::Vec3> min_object_coords;
    std::unordered_map<int, kvs::Vec3> max_object_coords;
    std::unordered_map<int, float> min_values;
    std::unordered_map<int, float> max_values;
    
    m_list.clear();
    m_total_min_subvolume_coord.clear();
    m_total_max_subvolume_coord.clear();

    for ( auto vtu : input_vtu.eachCellType() )
    {
        kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> importer( &vtu );
        kvs::UnstructuredVolumeObject* object = &importer;
        
        auto cell_type = object->cellType();

        number_of_nodes[cell_type] = object->nnodes();
        number_of_elements[cell_type] = object->ncells();
        number_of_ingredients[cell_type] = object->veclen();
        min_external_coords[cell_type][0] = object->minExternalCoord()[0];
        min_external_coords[cell_type][1] = object->minExternalCoord()[1];
        min_external_coords[cell_type][2] = object->minExternalCoord()[2];
        max_external_coords[cell_type][0] = object->maxExternalCoord()[0];
        max_external_coords[cell_type][1] = object->maxExternalCoord()[1];
        max_external_coords[cell_type][2] = object->maxExternalCoord()[2];
        min_object_coords[cell_type][0] = object->minObjectCoord()[0];
        min_object_coords[cell_type][1] = object->minObjectCoord()[1];
        min_object_coords[cell_type][2] = object->minObjectCoord()[2];
        max_object_coords[cell_type][0] = object->maxObjectCoord()[0];
        max_object_coords[cell_type][1] = object->maxObjectCoord()[1];
        max_object_coords[cell_type][2] = object->maxObjectCoord()[2];
        min_values[cell_type] = object->minValue();
        max_values[cell_type] = object->maxValue();
    }

    for (auto& e: number_of_nodes)
    {
        MultiVolumeProperty mvp;
        auto cell_type = e.first;

        mvp.m_number_nodes = number_of_nodes[cell_type];
        mvp.m_number_elements = number_of_elements[cell_type];
        mvp.m_elem_type = cell_type;
        mvp.m_file_type = 4; // unstructured
        mvp.m_number_files = last_time_step + 1;
        mvp.m_number_ingredients = number_of_ingredients[cell_type];
        mvp.m_start_step = 0;
        mvp.m_end_steps = last_time_step;
        mvp.m_number_subvolumes = 1;
        float x_min, y_min, z_min;
        float x_max, y_max, z_max;
        x_min = min_external_coords[cell_type][0];
        y_min = min_external_coords[cell_type][1];
        z_min = min_external_coords[cell_type][2];
        x_max = max_external_coords[cell_type][0];
        y_max = max_external_coords[cell_type][1];
        z_max = max_external_coords[cell_type][2];
        mvp.m_min_object_coord.set(x_min, y_min, z_min);
        mvp.m_max_object_coord.set(x_max, y_max, z_max);
        mvp.m_number_steps = mvp.m_end_steps - mvp.m_start_step + 1;
        mvp.m_min_subvolume_coord.resize(mvp.m_number_subvolumes);
        mvp.m_max_subvolume_coord.resize(mvp.m_number_subvolumes);
        mvp.m_file_path = filename;
        mvp.m_min_value = min_values[cell_type];
        mvp.m_max_value = max_values[cell_type];
        float sub_x_min, sub_y_min, sub_z_min;
        float sub_x_max, sub_y_max, sub_z_max;
        sub_x_min = min_object_coords[cell_type][0];
        sub_y_min = min_object_coords[cell_type][1];
        sub_z_min = min_object_coords[cell_type][2];
        sub_x_max = max_object_coords[cell_type][0];
        sub_y_max = max_object_coords[cell_type][1];
        sub_z_max = max_object_coords[cell_type][2];
        mvp.m_min_subvolume_coord[0].set(sub_x_min, sub_y_min, sub_z_min);
        mvp.m_max_subvolume_coord[0].set(sub_x_max, sub_y_max, sub_z_max);

// for debug
#if 0
        std::cout << "==================== cell type:" << cell_type << " start =========================" << std::endl;
        std::cout << "m_number_nodes:" << mvp.m_number_nodes << std::endl;
        std::cout << "m_number_elements:" << mvp.m_number_elements << std::endl;
        std::cout << "m_number_files:" << mvp.m_number_files << std::endl;
        std::cout << "m_start_step:" << mvp.m_start_step << std::endl;
        std::cout << "m_end_steps:" << mvp.m_end_steps << std::endl;
        std::cout << "m_number_steps:" << mvp.m_number_steps << std::endl;
        std::cout << "m_number_subvolumes:" << mvp.m_number_subvolumes << std::endl;
        std::cout << "m_min_object_coord[0]:" << mvp.m_min_object_coord[0] << std::endl;
        std::cout << "m_min_object_coord[1]:" << mvp.m_min_object_coord[1] << std::endl;
        std::cout << "m_min_object_coord[2]:" << mvp.m_min_object_coord[2] << std::endl;
        std::cout << "m_max_object_coord[0]:" << mvp.m_max_object_coord[0] << std::endl;
        std::cout << "m_max_object_coord[1]:" << mvp.m_max_object_coord[1] << std::endl;
        std::cout << "m_max_object_coord[2]:" << mvp.m_max_object_coord[2] << std::endl;
        std::cout << "m_min_subvolume_coord[0]:" << mvp.m_min_subvolume_coord[0][0] << std::endl;
        std::cout << "m_min_subvolume_coord[1]:" << mvp.m_min_subvolume_coord[0][1] << std::endl;
        std::cout << "m_min_subvolume_coord[2]:" << mvp.m_min_subvolume_coord[0][2] << std::endl;
        std::cout << "m_max_subvolume_coord[0]:" << mvp.m_max_subvolume_coord[0][0] << std::endl;
        std::cout << "m_max_subvolume_coord[1]:" << mvp.m_max_subvolume_coord[0][1] << std::endl;
        std::cout << "m_max_subvolume_coord[2]:" << mvp.m_max_subvolume_coord[0][2] << std::endl;
        std::cout << "m_min_value:" << mvp.m_min_value << std::endl;
        std::cout << "m_max_value:" << mvp.m_max_value << std::endl;
        std::cout << "m_number_ingredients:" << mvp.m_number_ingredients << std::endl;
        std::cout << "==================== cell type:" << cell_type << " end =========================" << std::endl;
#endif

        if ( this->m_list.empty() )
        {
            this->m_total_number_nodes = mvp.m_number_nodes;
            this->m_total_number_elements = mvp.m_number_elements;
            this->m_total_number_files = mvp.m_number_files;
            this->m_total_start_steps = mvp.m_start_step;
            this->m_total_last_step = mvp.m_end_steps;
            this->m_total_number_steps = mvp.m_number_steps;
            this->m_total_number_subvolumes = mvp.m_number_subvolumes;
            this->m_total_min_object_coord = mvp.m_min_object_coord;
            this->m_total_max_object_coord = mvp.m_max_object_coord;
            this->m_total_min_subvolume_coord = mvp.m_min_subvolume_coord;
            this->m_total_max_subvolume_coord = mvp.m_max_subvolume_coord;
            this->m_total_min_value = mvp.m_min_value;
            this->m_total_max_value = mvp.m_max_value;
            this->m_total_number_ingredients = mvp.m_number_ingredients;
        }
        else
        {
            this->m_total_number_nodes += mvp.m_number_nodes;
            this->m_total_number_elements += mvp.m_number_elements;
            this->m_total_number_files += mvp.m_number_files;
            this->m_total_start_steps = std::min(this->m_total_start_steps, mvp.m_start_step);
            this->m_total_last_step = std::max(this->m_total_last_step, mvp.m_end_steps);
            this->m_total_number_steps = this->m_total_last_step - this->m_total_start_steps + 1;
            this->m_total_number_subvolumes += mvp.m_number_subvolumes;
            this->m_total_min_object_coord[0] = std::min(this->m_total_min_object_coord[0], mvp.m_min_object_coord[0]);
            this->m_total_min_object_coord[1] = std::min(this->m_total_min_object_coord[1], mvp.m_min_object_coord[1]);
            this->m_total_min_object_coord[2] = std::min(this->m_total_min_object_coord[2], mvp.m_min_object_coord[2]);
            this->m_total_max_object_coord[0] = std::max(this->m_total_max_object_coord[0], mvp.m_max_object_coord[0]);
            this->m_total_max_object_coord[1] = std::max(this->m_total_max_object_coord[1], mvp.m_max_object_coord[1]);
            this->m_total_max_object_coord[2] = std::max(this->m_total_max_object_coord[2], mvp.m_max_object_coord[2]);
            std::copy(mvp.m_min_subvolume_coord.begin(), mvp.m_min_subvolume_coord.end(), std::back_inserter(this->m_total_min_subvolume_coord));
            std::copy(mvp.m_max_subvolume_coord.begin(), mvp.m_max_subvolume_coord.end(), std::back_inserter(this->m_total_max_subvolume_coord));
            this->m_total_min_value = std::min(this->m_total_min_value, mvp.m_min_value);
            this->m_total_max_value = std::max(this->m_total_max_value, mvp.m_max_value);
            this->m_total_number_ingredients = std::max(this->m_total_number_ingredients, mvp.m_number_ingredients);
        }
        this->m_list.push_back(mvp);
    }

    m_total_ingredient.resize( m_total_number_ingredients );

    return m_list.size();
}

int MultiVolumePropertyList::loadSeriesVtu( const std::string& filename )
{
    namespace fs = std::filesystem;
    fs::path filepath = filename;
    kvs::ExtendedFileFormat::NumeralSequenceFiles<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> time_series( filepath.generic_string() );
    int last_time_step = time_series.numberOfFiles() - 1;
    int time_step = 0;
    int sub_volume_id = 0;
    int sub_volume_count = 1;
    std::unordered_map<int, int> number_of_nodes;
    std::unordered_map<int, int> number_of_elements;
    std::unordered_map<int, int> number_of_ingredients;
    std::unordered_map<int, kvs::Vec3> min_external_coords;
    std::unordered_map<int, kvs::Vec3> max_external_coords;
    std::unordered_map<int, kvs::Vec3> min_object_coords;
    std::unordered_map<int, kvs::Vec3> max_object_coords;
    std::unordered_map<int, float> min_values;
    std::unordered_map<int, float> max_values;
    
    m_list.clear();
    m_total_min_subvolume_coord.clear();
    m_total_max_subvolume_coord.clear();

    for ( auto whole_vtu : time_series.eachTimeStep() )
    {        
        for ( auto vtu : whole_vtu.eachCellType() )
        {
            kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> importer( &vtu );
            kvs::UnstructuredVolumeObject* object = &importer;
            
            auto cell_type = object->cellType();

            if ( number_of_nodes.count(cell_type) == 0 )
            {
                number_of_nodes[cell_type] = object->nnodes();
                number_of_elements[cell_type] = object->ncells();
                number_of_ingredients[cell_type] = object->veclen();
                min_external_coords[cell_type][0] = object->minExternalCoord()[0];
                min_external_coords[cell_type][1] = object->minExternalCoord()[1];
                min_external_coords[cell_type][2] = object->minExternalCoord()[2];
                max_external_coords[cell_type][0] = object->maxExternalCoord()[0];
                max_external_coords[cell_type][1] = object->maxExternalCoord()[1];
                max_external_coords[cell_type][2] = object->maxExternalCoord()[2];
                min_object_coords[cell_type][0] = object->minObjectCoord()[0];
                min_object_coords[cell_type][1] = object->minObjectCoord()[1];
                min_object_coords[cell_type][2] = object->minObjectCoord()[2];
                max_object_coords[cell_type][0] = object->maxObjectCoord()[0];
                max_object_coords[cell_type][1] = object->maxObjectCoord()[1];
                max_object_coords[cell_type][2] = object->maxObjectCoord()[2];
                min_values[cell_type] = object->minValue();
                max_values[cell_type] = object->maxValue();
            }
            else
            {
                min_external_coords[cell_type][0] = std::min(min_external_coords[cell_type][0], object->minExternalCoord()[0]);
                min_external_coords[cell_type][1] = std::min(min_external_coords[cell_type][1], object->minExternalCoord()[1]);
                min_external_coords[cell_type][2] = std::min(min_external_coords[cell_type][2], object->minExternalCoord()[2]);
                max_external_coords[cell_type][0] = std::max(max_external_coords[cell_type][0], object->maxExternalCoord()[0]);
                max_external_coords[cell_type][1] = std::max(max_external_coords[cell_type][1], object->maxExternalCoord()[1]);
                max_external_coords[cell_type][2] = std::max(max_external_coords[cell_type][2], object->maxExternalCoord()[2]);
                min_object_coords[cell_type][0] = std::min(min_object_coords[cell_type][0], object->minObjectCoord()[0]);
                min_object_coords[cell_type][1] = std::min(min_object_coords[cell_type][1], object->minObjectCoord()[1]);
                min_object_coords[cell_type][2] = std::min(min_object_coords[cell_type][2], object->minObjectCoord()[2]);
                max_object_coords[cell_type][0] = std::max(max_object_coords[cell_type][0], object->maxObjectCoord()[0]);
                max_object_coords[cell_type][1] = std::max(max_object_coords[cell_type][1], object->maxObjectCoord()[1]);
                max_object_coords[cell_type][2] = std::max(max_object_coords[cell_type][2], object->maxObjectCoord()[2]);
                min_values[cell_type] = std::min(min_values[cell_type], float(object->minValue()));
                max_values[cell_type] = std::max(max_values[cell_type], float(object->maxValue()));
            }
        }
        ++time_step;
    }

    for (auto& e: number_of_nodes)
    {
        MultiVolumeProperty mvp;
        auto cell_type = e.first;

        mvp.m_number_nodes = number_of_nodes[cell_type];
        mvp.m_number_elements = number_of_elements[cell_type];
        mvp.m_elem_type = cell_type;
        mvp.m_file_type = 4; // unstructured
        mvp.m_number_files = last_time_step + 1;
        mvp.m_number_ingredients = number_of_ingredients[cell_type];
        mvp.m_start_step = 0;
        mvp.m_end_steps = last_time_step;
        mvp.m_number_subvolumes = 1;
        float x_min, y_min, z_min;
        float x_max, y_max, z_max;
        x_min = min_external_coords[cell_type][0];
        y_min = min_external_coords[cell_type][1];
        z_min = min_external_coords[cell_type][2];
        x_max = max_external_coords[cell_type][0];
        y_max = max_external_coords[cell_type][1];
        z_max = max_external_coords[cell_type][2];
        mvp.m_min_object_coord.set(x_min, y_min, z_min);
        mvp.m_max_object_coord.set(x_max, y_max, z_max);
        mvp.m_number_steps = mvp.m_end_steps - mvp.m_start_step + 1;
        mvp.m_min_subvolume_coord.resize(mvp.m_number_subvolumes);
        mvp.m_max_subvolume_coord.resize(mvp.m_number_subvolumes);
        mvp.m_file_path = filename;
        mvp.m_min_value = min_values[cell_type];
        mvp.m_max_value = max_values[cell_type];
        float sub_x_min, sub_y_min, sub_z_min;
        float sub_x_max, sub_y_max, sub_z_max;
        sub_x_min = min_object_coords[cell_type][0];
        sub_y_min = min_object_coords[cell_type][1];
        sub_z_min = min_object_coords[cell_type][2];
        sub_x_max = max_object_coords[cell_type][0];
        sub_y_max = max_object_coords[cell_type][1];
        sub_z_max = max_object_coords[cell_type][2];
        mvp.m_min_subvolume_coord[0].set(sub_x_min, sub_y_min, sub_z_min);
        mvp.m_max_subvolume_coord[0].set(sub_x_max, sub_y_max, sub_z_max);

// for debug
#if 0
        std::cout << "==================== cell type:" << cell_type << " start =========================" << std::endl;
        std::cout << "m_number_nodes:" << mvp.m_number_nodes << std::endl;
        std::cout << "m_number_elements:" << mvp.m_number_elements << std::endl;
        std::cout << "m_number_files:" << mvp.m_number_files << std::endl;
        std::cout << "m_start_step:" << mvp.m_start_step << std::endl;
        std::cout << "m_end_steps:" << mvp.m_end_steps << std::endl;
        std::cout << "m_number_steps:" << mvp.m_number_steps << std::endl;
        std::cout << "m_number_subvolumes:" << mvp.m_number_subvolumes << std::endl;
        std::cout << "m_min_object_coord[0]:" << mvp.m_min_object_coord[0] << std::endl;
        std::cout << "m_min_object_coord[1]:" << mvp.m_min_object_coord[1] << std::endl;
        std::cout << "m_min_object_coord[2]:" << mvp.m_min_object_coord[2] << std::endl;
        std::cout << "m_max_object_coord[0]:" << mvp.m_max_object_coord[0] << std::endl;
        std::cout << "m_max_object_coord[1]:" << mvp.m_max_object_coord[1] << std::endl;
        std::cout << "m_max_object_coord[2]:" << mvp.m_max_object_coord[2] << std::endl;
        std::cout << "m_min_subvolume_coord[0]:" << mvp.m_min_subvolume_coord[0][0] << std::endl;
        std::cout << "m_min_subvolume_coord[1]:" << mvp.m_min_subvolume_coord[0][1] << std::endl;
        std::cout << "m_min_subvolume_coord[2]:" << mvp.m_min_subvolume_coord[0][2] << std::endl;
        std::cout << "m_max_subvolume_coord[0]:" << mvp.m_max_subvolume_coord[0][0] << std::endl;
        std::cout << "m_max_subvolume_coord[1]:" << mvp.m_max_subvolume_coord[0][1] << std::endl;
        std::cout << "m_max_subvolume_coord[2]:" << mvp.m_max_subvolume_coord[0][2] << std::endl;
        std::cout << "m_min_value:" << mvp.m_min_value << std::endl;
        std::cout << "m_max_value:" << mvp.m_max_value << std::endl;
        std::cout << "m_number_ingredients:" << mvp.m_number_ingredients << std::endl;
        std::cout << "==================== cell type:" << cell_type << " end =========================" << std::endl;
#endif

        if ( this->m_list.empty() )
        {
            this->m_total_number_nodes = mvp.m_number_nodes;
            this->m_total_number_elements = mvp.m_number_elements;
            this->m_total_number_files = mvp.m_number_files;
            this->m_total_start_steps = mvp.m_start_step;
            this->m_total_last_step = mvp.m_end_steps;
            this->m_total_number_steps = mvp.m_number_steps;
            this->m_total_number_subvolumes = mvp.m_number_subvolumes;
            this->m_total_min_object_coord = mvp.m_min_object_coord;
            this->m_total_max_object_coord = mvp.m_max_object_coord;
            this->m_total_min_subvolume_coord = mvp.m_min_subvolume_coord;
            this->m_total_max_subvolume_coord = mvp.m_max_subvolume_coord;
            this->m_total_min_value = mvp.m_min_value;
            this->m_total_max_value = mvp.m_max_value;
            this->m_total_number_ingredients = mvp.m_number_ingredients;
        }
        else
        {
            this->m_total_number_nodes += mvp.m_number_nodes;
            this->m_total_number_elements += mvp.m_number_elements;
            this->m_total_number_files += mvp.m_number_files;
            this->m_total_start_steps = std::min(this->m_total_start_steps, mvp.m_start_step);
            this->m_total_last_step = std::max(this->m_total_last_step, mvp.m_end_steps);
            this->m_total_number_steps = this->m_total_last_step - this->m_total_start_steps + 1;
            this->m_total_number_subvolumes += mvp.m_number_subvolumes;
            this->m_total_min_object_coord[0] = std::min(this->m_total_min_object_coord[0], mvp.m_min_object_coord[0]);
            this->m_total_min_object_coord[1] = std::min(this->m_total_min_object_coord[1], mvp.m_min_object_coord[1]);
            this->m_total_min_object_coord[2] = std::min(this->m_total_min_object_coord[2], mvp.m_min_object_coord[2]);
            this->m_total_max_object_coord[0] = std::max(this->m_total_max_object_coord[0], mvp.m_max_object_coord[0]);
            this->m_total_max_object_coord[1] = std::max(this->m_total_max_object_coord[1], mvp.m_max_object_coord[1]);
            this->m_total_max_object_coord[2] = std::max(this->m_total_max_object_coord[2], mvp.m_max_object_coord[2]);
            std::copy(mvp.m_min_subvolume_coord.begin(), mvp.m_min_subvolume_coord.end(), std::back_inserter(this->m_total_min_subvolume_coord));
            std::copy(mvp.m_max_subvolume_coord.begin(), mvp.m_max_subvolume_coord.end(), std::back_inserter(this->m_total_max_subvolume_coord));
            this->m_total_min_value = std::min(this->m_total_min_value, mvp.m_min_value);
            this->m_total_max_value = std::max(this->m_total_max_value, mvp.m_max_value);
            this->m_total_number_ingredients = std::max(this->m_total_number_ingredients, mvp.m_number_ingredients);
        }
        this->m_list.push_back(mvp);
    }

    m_total_ingredient.resize( m_total_number_ingredients );

    return m_list.size();
}

int MultiVolumePropertyList::loadVti( const std::string& filename )
{
    int cell_type = 7;
    int sub_volume_id = 0;
    float x_min, y_min, z_min;
    float x_max, y_max, z_max;
    float sub_x_min, sub_y_min, sub_z_min;
    float sub_x_max, sub_y_max, sub_z_max;
    MultiVolumeProperty mvp;

    m_list.clear();
    m_total_min_subvolume_coord.clear();
    m_total_max_subvolume_coord.clear();

    kvs::ExtendedFileFormat::VtkXmlImageData input_vti( filename );
    kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlImageData> importer( &input_vti );
    kvs::StructuredVolumeObject* object = &importer;
    kvs::Vec3ui resolution = object->resolution();
    object->setMinMaxExternalCoords( object->minObjectCoord(), object->maxObjectCoord() );

    mvp.m_number_nodes = static_cast<int>( resolution[0] * resolution[1] * resolution[2] );
    mvp.m_number_elements = static_cast<int>( ( resolution[0] - 1 ) * ( resolution[1] - 1 ) * ( resolution[2] - 1 ) );
    mvp.m_elem_type = cell_type;
    mvp.m_file_type = 3; // Structured
    mvp.m_number_files = 1;
    mvp.m_number_ingredients = object->veclen();
    mvp.m_start_step = 0;
    mvp.m_end_steps = 0;
    mvp.m_number_subvolumes = 1;
    x_min = object->minExternalCoord()[0];
    y_min = object->minExternalCoord()[1];
    z_min = object->minExternalCoord()[2];
    x_max = object->maxExternalCoord()[0];
    y_max = object->maxExternalCoord()[1];
    z_max = object->maxExternalCoord()[2];
    mvp.m_min_object_coord.set(x_min, y_min, z_min);
    mvp.m_max_object_coord.set(x_max, y_max, z_max);
    mvp.m_number_steps = 1;
    mvp.m_min_subvolume_coord.resize(mvp.m_number_subvolumes);
    mvp.m_max_subvolume_coord.resize(mvp.m_number_subvolumes);   
    mvp.m_file_path = filename;
    mvp.m_min_value = object->minValue();
    mvp.m_max_value = object->maxValue();
    sub_x_min = object->minObjectCoord()[0];
    sub_y_min = object->minObjectCoord()[1];
    sub_z_min = object->minObjectCoord()[2];
    sub_x_max = object->maxObjectCoord()[0];
    sub_y_max = object->maxObjectCoord()[1];
    sub_z_max = object->maxObjectCoord()[2];
    mvp.m_min_subvolume_coord[sub_volume_id].set(sub_x_min, sub_y_min, sub_z_min);
    mvp.m_max_subvolume_coord[sub_volume_id].set(sub_x_max, sub_y_max, sub_z_max);

    this->m_total_number_nodes = mvp.m_number_nodes;
    this->m_total_number_elements = mvp.m_number_elements;
    this->m_total_number_files = mvp.m_number_files;
    this->m_total_start_steps = mvp.m_start_step;
    this->m_total_last_step = mvp.m_end_steps;
    this->m_total_number_steps = mvp.m_number_steps;
    this->m_total_number_subvolumes = mvp.m_number_subvolumes;
    this->m_total_min_object_coord = mvp.m_min_object_coord;
    this->m_total_max_object_coord = mvp.m_max_object_coord;
    this->m_total_min_subvolume_coord = mvp.m_min_subvolume_coord;
    this->m_total_max_subvolume_coord = mvp.m_max_subvolume_coord;
    this->m_total_min_value = mvp.m_min_value;
    this->m_total_max_value = mvp.m_max_value;
    this->m_total_number_ingredients = mvp.m_number_ingredients;

    this->m_list.push_back(mvp);
    m_total_ingredient.resize( m_total_number_ingredients );
    return m_list.size();
}

int MultiVolumePropertyList::loadSeriesVti( const std::string& filename )
{
    namespace fs = std::filesystem;
    fs::path filepath = filename;
    kvs::ExtendedFileFormat::NumeralSequenceFiles<kvs::ExtendedFileFormat::VtkXmlImageData> time_series( filepath.generic_string() );
    int last_time_step = time_series.numberOfFiles() - 1;
    int time_step = 0;
    int sub_volume_id = 0;
    int sub_volume_count = 1;
    int cell_type = 7;
    int number_of_nodes;
    int number_of_elements;
    int number_of_ingredients;
    kvs::Vec3 min_external_coords;
    kvs::Vec3 max_external_coords;
    kvs::Vec3 min_object_coords;
    kvs::Vec3 max_object_coords;
    float min_values;
    float max_values;
    MultiVolumeProperty mvp;
    float x_min, y_min, z_min;
    float x_max, y_max, z_max;
    float sub_x_min, sub_y_min, sub_z_min;
    float sub_x_max, sub_y_max, sub_z_max;

    m_list.clear();
    m_total_min_subvolume_coord.clear();
    m_total_max_subvolume_coord.clear();

    for ( auto vti : time_series.eachTimeStep() )
    {
        kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlImageData> importer( &vti );
        kvs::StructuredVolumeObject* object = &importer;
        kvs::Vec3ui resolution = object->resolution();
        object->setMinMaxExternalCoords( object->minObjectCoord(), object->maxObjectCoord() );

        if ( time_step == 0 )
        {
            number_of_nodes = static_cast<int>( resolution[0] * resolution[1] * resolution[2] );
            number_of_elements = static_cast<int>( ( resolution[0] - 1 ) * ( resolution[1] - 1 ) * ( resolution[2] - 1 ) );
            number_of_ingredients = object->veclen();
            min_external_coords[0] = object->minExternalCoord()[0];
            min_external_coords[1] = object->minExternalCoord()[1];
            min_external_coords[2] = object->minExternalCoord()[2];
            max_external_coords[0] = object->maxExternalCoord()[0];
            max_external_coords[1] = object->maxExternalCoord()[1];
            max_external_coords[2] = object->maxExternalCoord()[2];
            min_object_coords[0] = object->minObjectCoord()[0];
            min_object_coords[1] = object->minObjectCoord()[1];
            min_object_coords[2] = object->minObjectCoord()[2];
            max_object_coords[0] = object->maxObjectCoord()[0];
            max_object_coords[1] = object->maxObjectCoord()[1];
            max_object_coords[2] = object->maxObjectCoord()[2];
            min_values = object->minValue();
            max_values = object->maxValue();
        }
        else
        {
            min_external_coords[0] = std::min(min_external_coords[0], object->minExternalCoord()[0]);
            min_external_coords[1] = std::min(min_external_coords[1], object->minExternalCoord()[1]);
            min_external_coords[2] = std::min(min_external_coords[2], object->minExternalCoord()[2]);
            max_external_coords[0] = std::max(max_external_coords[0], object->maxExternalCoord()[0]);
            max_external_coords[1] = std::max(max_external_coords[1], object->maxExternalCoord()[1]);
            max_external_coords[2] = std::max(max_external_coords[2], object->maxExternalCoord()[2]);
            min_object_coords[0] = std::min(min_object_coords[0], object->minObjectCoord()[0]);
            min_object_coords[1] = std::min(min_object_coords[1], object->minObjectCoord()[1]);
            min_object_coords[2] = std::min(min_object_coords[2], object->minObjectCoord()[2]);
            max_object_coords[0] = std::max(max_object_coords[0], object->maxObjectCoord()[0]);
            max_object_coords[1] = std::max(max_object_coords[1], object->maxObjectCoord()[1]);
            max_object_coords[2] = std::max(max_object_coords[2], object->maxObjectCoord()[2]);
            min_values = std::min(min_values, float(object->minValue()));
            max_values = std::max(max_values, float(object->maxValue()));
        }
    }

    mvp.m_number_nodes = number_of_nodes;
    mvp.m_number_elements = number_of_elements;
    mvp.m_elem_type = cell_type;
    mvp.m_file_type = 3; // Structured
    mvp.m_number_files = last_time_step + 1;
    mvp.m_number_ingredients = number_of_ingredients;
    mvp.m_start_step = 0;
    mvp.m_end_steps = last_time_step;
    mvp.m_number_subvolumes = 1;
    x_min = min_external_coords[0];
    y_min = min_external_coords[1];
    z_min = min_external_coords[2];
    x_max = max_external_coords[0];
    y_max = max_external_coords[1];
    z_max = max_external_coords[2];
    mvp.m_min_object_coord.set(x_min, y_min, z_min);
    mvp.m_max_object_coord.set(x_max, y_max, z_max);
    mvp.m_number_steps = mvp.m_end_steps - mvp.m_start_step + 1;
    mvp.m_min_subvolume_coord.resize(mvp.m_number_subvolumes);
    mvp.m_max_subvolume_coord.resize(mvp.m_number_subvolumes);
    mvp.m_file_path = filename;
    mvp.m_min_value = min_values;
    mvp.m_max_value = max_values;
    sub_x_min = min_object_coords[0];
    sub_y_min = min_object_coords[1];
    sub_z_min = min_object_coords[2];
    sub_x_max = max_object_coords[0];
    sub_y_max = max_object_coords[1];
    sub_z_max = max_object_coords[2];
    mvp.m_min_subvolume_coord[sub_volume_id].set(sub_x_min, sub_y_min, sub_z_min);
    mvp.m_max_subvolume_coord[sub_volume_id].set(sub_x_max, sub_y_max, sub_z_max);

// for debug
#if 0
        std::cout << "==================== cell type:" << cell_type << " start =========================" << std::endl;
        std::cout << "m_number_nodes:" << mvp.m_number_nodes << std::endl;
        std::cout << "m_number_elements:" << mvp.m_number_elements << std::endl;
        std::cout << "m_number_files:" << mvp.m_number_files << std::endl;
        std::cout << "m_start_step:" << mvp.m_start_step << std::endl;
        std::cout << "m_end_steps:" << mvp.m_end_steps << std::endl;
        std::cout << "m_number_steps:" << mvp.m_number_steps << std::endl;
        std::cout << "m_number_subvolumes:" << mvp.m_number_subvolumes << std::endl;
        std::cout << "m_min_object_coord[0]:" << mvp.m_min_object_coord[0] << std::endl;
        std::cout << "m_min_object_coord[1]:" << mvp.m_min_object_coord[1] << std::endl;
        std::cout << "m_min_object_coord[2]:" << mvp.m_min_object_coord[2] << std::endl;
        std::cout << "m_max_object_coord[0]:" << mvp.m_max_object_coord[0] << std::endl;
        std::cout << "m_max_object_coord[1]:" << mvp.m_max_object_coord[1] << std::endl;
        std::cout << "m_max_object_coord[2]:" << mvp.m_max_object_coord[2] << std::endl;
        std::cout << "m_min_subvolume_coord[0]:" << mvp.m_min_subvolume_coord[0][0] << std::endl;
        std::cout << "m_min_subvolume_coord[1]:" << mvp.m_min_subvolume_coord[0][1] << std::endl;
        std::cout << "m_min_subvolume_coord[2]:" << mvp.m_min_subvolume_coord[0][2] << std::endl;
        std::cout << "m_max_subvolume_coord[0]:" << mvp.m_max_subvolume_coord[0][0] << std::endl;
        std::cout << "m_max_subvolume_coord[1]:" << mvp.m_max_subvolume_coord[0][1] << std::endl;
        std::cout << "m_max_subvolume_coord[2]:" << mvp.m_max_subvolume_coord[0][2] << std::endl;
        std::cout << "m_min_value:" << mvp.m_min_value << std::endl;
        std::cout << "m_max_value:" << mvp.m_max_value << std::endl;
        std::cout << "m_number_ingredients:" << mvp.m_number_ingredients << std::endl;
        std::cout << "==================== cell type:" << cell_type << " end =========================" << std::endl;
#endif

    this->m_total_number_nodes = mvp.m_number_nodes;
    this->m_total_number_elements = mvp.m_number_elements;
    this->m_total_number_files = mvp.m_number_files;
    this->m_total_start_steps = mvp.m_start_step;
    this->m_total_last_step = mvp.m_end_steps;
    this->m_total_number_steps = mvp.m_number_steps;
    this->m_total_number_subvolumes = mvp.m_number_subvolumes;
    this->m_total_min_object_coord = mvp.m_min_object_coord;
    this->m_total_max_object_coord = mvp.m_max_object_coord;
    this->m_total_min_subvolume_coord = mvp.m_min_subvolume_coord;
    this->m_total_max_subvolume_coord = mvp.m_max_subvolume_coord;
    this->m_total_min_value = mvp.m_min_value;
    this->m_total_max_value = mvp.m_max_value;
    this->m_total_number_ingredients = mvp.m_number_ingredients;

    this->m_list.push_back(mvp);
    m_total_ingredient.resize( m_total_number_ingredients );
    return m_list.size();
}

int MultiVolumePropertyList::loadInp( const std::string& filename )
{
    kvs::ExtendedFileFormat::AvsUcd input_inp( filename );
    int last_time_step = 0;
    int time_step = 0;
    std::unordered_map<int, int> sub_volume_ids;
    std::unordered_map<int, int> sub_volume_counts;
    std::unordered_map<int, int> number_of_nodes;
    std::unordered_map<int, int> number_of_elements;
    std::unordered_map<int, int> number_of_ingredients;
    std::unordered_map<int, kvs::Vec3> min_external_coords;
    std::unordered_map<int, kvs::Vec3> max_external_coords;
    std::unordered_map<int, kvs::Vec3> min_object_coords;
    std::unordered_map<int, kvs::Vec3> max_object_coords;
    std::unordered_map<int, float> min_values;
    std::unordered_map<int, float> max_values;
    
    m_list.clear();
    m_total_min_subvolume_coord.clear();
    m_total_max_subvolume_coord.clear();

    for ( auto inp : input_inp.eachCellType() )
    {
        kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::AvsUcd> importer( &inp );
        kvs::UnstructuredVolumeObject* object = &importer;
        
        auto cell_type = object->cellType();

        number_of_nodes[cell_type] = object->nnodes();
        number_of_elements[cell_type] = object->ncells();
        number_of_ingredients[cell_type] = object->veclen();
        min_external_coords[cell_type] = object->minExternalCoord();
        max_external_coords[cell_type] = object->maxExternalCoord();
        min_object_coords[cell_type][0] = object->minObjectCoord()[0];
        min_object_coords[cell_type][1] = object->minObjectCoord()[1];
        min_object_coords[cell_type][2] = object->minObjectCoord()[2];
        max_object_coords[cell_type][0] = object->maxObjectCoord()[0];
        max_object_coords[cell_type][1] = object->maxObjectCoord()[1];
        max_object_coords[cell_type][2] = object->maxObjectCoord()[2];
        min_values[cell_type] = object->minValue();
        max_values[cell_type] = object->maxValue();
    }

    for (auto& e: number_of_nodes)
    {
        MultiVolumeProperty mvp;
        auto cell_type = e.first;

        mvp.m_number_nodes = number_of_nodes[cell_type];
        mvp.m_number_elements = number_of_elements[cell_type];
        mvp.m_elem_type = cell_type;
        mvp.m_file_type = 4; // unstructured
        mvp.m_number_files = last_time_step + 1;
        mvp.m_number_ingredients = number_of_ingredients[cell_type];
        mvp.m_start_step = 0;
        mvp.m_end_steps = last_time_step;
        mvp.m_number_subvolumes = 1;
        float x_min, y_min, z_min;
        float x_max, y_max, z_max;
        x_min = min_external_coords[cell_type][0];
        y_min = min_external_coords[cell_type][1];
        z_min = min_external_coords[cell_type][2];
        x_max = max_external_coords[cell_type][0];
        y_max = max_external_coords[cell_type][1];
        z_max = max_external_coords[cell_type][2];
        mvp.m_min_object_coord.set(x_min, y_min, z_min);
        mvp.m_max_object_coord.set(x_max, y_max, z_max);
        mvp.m_number_steps = mvp.m_end_steps - mvp.m_start_step + 1;
        mvp.m_min_subvolume_coord.resize(mvp.m_number_subvolumes);
        mvp.m_max_subvolume_coord.resize(mvp.m_number_subvolumes);
        mvp.m_file_path = filename;
        mvp.m_min_value = min_values[cell_type];
        mvp.m_max_value = max_values[cell_type];
        float sub_x_min, sub_y_min, sub_z_min;
        float sub_x_max, sub_y_max, sub_z_max;
        sub_x_min = min_object_coords[cell_type][0];
        sub_y_min = min_object_coords[cell_type][1];
        sub_z_min = min_object_coords[cell_type][2];
        sub_x_max = max_object_coords[cell_type][0];
        sub_y_max = max_object_coords[cell_type][1];
        sub_z_max = max_object_coords[cell_type][2];
        mvp.m_min_subvolume_coord[0].set(sub_x_min, sub_y_min, sub_z_min);
        mvp.m_max_subvolume_coord[0].set(sub_x_max, sub_y_max, sub_z_max);

// for debug
#if 0
        std::cout << "==================== cell type:" << cell_type << " start =========================" << std::endl;
        std::cout << "m_number_nodes:" << mvp.m_number_nodes << std::endl;
        std::cout << "m_number_elements:" << mvp.m_number_elements << std::endl;
        std::cout << "m_number_files:" << mvp.m_number_files << std::endl;
        std::cout << "m_start_step:" << mvp.m_start_step << std::endl;
        std::cout << "m_end_steps:" << mvp.m_end_steps << std::endl;
        std::cout << "m_number_steps:" << mvp.m_number_steps << std::endl;
        std::cout << "m_number_subvolumes:" << mvp.m_number_subvolumes << std::endl;
        std::cout << "m_min_object_coord[0]:" << mvp.m_min_object_coord[0] << std::endl;
        std::cout << "m_min_object_coord[1]:" << mvp.m_min_object_coord[1] << std::endl;
        std::cout << "m_min_object_coord[2]:" << mvp.m_min_object_coord[2] << std::endl;
        std::cout << "m_max_object_coord[0]:" << mvp.m_max_object_coord[0] << std::endl;
        std::cout << "m_max_object_coord[1]:" << mvp.m_max_object_coord[1] << std::endl;
        std::cout << "m_max_object_coord[2]:" << mvp.m_max_object_coord[2] << std::endl;
        std::cout << "m_min_subvolume_coord[0]:" << mvp.m_min_subvolume_coord[0][0] << std::endl;
        std::cout << "m_min_subvolume_coord[1]:" << mvp.m_min_subvolume_coord[0][1] << std::endl;
        std::cout << "m_min_subvolume_coord[2]:" << mvp.m_min_subvolume_coord[0][2] << std::endl;
        std::cout << "m_max_subvolume_coord[0]:" << mvp.m_max_subvolume_coord[0][0] << std::endl;
        std::cout << "m_max_subvolume_coord[1]:" << mvp.m_max_subvolume_coord[0][1] << std::endl;
        std::cout << "m_max_subvolume_coord[2]:" << mvp.m_max_subvolume_coord[0][2] << std::endl;
        std::cout << "m_min_value:" << mvp.m_min_value << std::endl;
        std::cout << "m_max_value:" << mvp.m_max_value << std::endl;
        std::cout << "m_number_ingredients:" << mvp.m_number_ingredients << std::endl;
        std::cout << "==================== cell type:" << cell_type << " end =========================" << std::endl;
#endif

        if ( this->m_list.empty() )
        {
            this->m_total_number_nodes = mvp.m_number_nodes;
            this->m_total_number_elements = mvp.m_number_elements;
            this->m_total_number_files = mvp.m_number_files;
            this->m_total_start_steps = mvp.m_start_step;
            this->m_total_last_step = mvp.m_end_steps;
            this->m_total_number_steps = mvp.m_number_steps;
            this->m_total_number_subvolumes = mvp.m_number_subvolumes;
            this->m_total_min_object_coord = mvp.m_min_object_coord;
            this->m_total_max_object_coord = mvp.m_max_object_coord;
            this->m_total_min_subvolume_coord = mvp.m_min_subvolume_coord;
            this->m_total_max_subvolume_coord = mvp.m_max_subvolume_coord;
            this->m_total_min_value = mvp.m_min_value;
            this->m_total_max_value = mvp.m_max_value;
            this->m_total_number_ingredients = mvp.m_number_ingredients;
        }
        else
        {
            this->m_total_number_nodes += mvp.m_number_nodes;
            this->m_total_number_elements += mvp.m_number_elements;
            this->m_total_number_files += mvp.m_number_files;
            this->m_total_start_steps = std::min(this->m_total_start_steps, mvp.m_start_step);
            this->m_total_last_step = std::max(this->m_total_last_step, mvp.m_end_steps);
            this->m_total_number_steps = this->m_total_last_step - this->m_total_start_steps + 1;
            this->m_total_number_subvolumes += mvp.m_number_subvolumes;
            this->m_total_min_object_coord[0] = std::min(this->m_total_min_object_coord[0], mvp.m_min_object_coord[0]);
            this->m_total_min_object_coord[1] = std::min(this->m_total_min_object_coord[1], mvp.m_min_object_coord[1]);
            this->m_total_min_object_coord[2] = std::min(this->m_total_min_object_coord[2], mvp.m_min_object_coord[2]);
            this->m_total_max_object_coord[0] = std::max(this->m_total_max_object_coord[0], mvp.m_max_object_coord[0]);
            this->m_total_max_object_coord[1] = std::max(this->m_total_max_object_coord[1], mvp.m_max_object_coord[1]);
            this->m_total_max_object_coord[2] = std::max(this->m_total_max_object_coord[2], mvp.m_max_object_coord[2]);
            std::copy(mvp.m_min_subvolume_coord.begin(), mvp.m_min_subvolume_coord.end(), std::back_inserter(this->m_total_min_subvolume_coord));
            std::copy(mvp.m_max_subvolume_coord.begin(), mvp.m_max_subvolume_coord.end(), std::back_inserter(this->m_total_max_subvolume_coord));
            this->m_total_min_value = std::min(this->m_total_min_value, mvp.m_min_value);
            this->m_total_max_value = std::max(this->m_total_max_value, mvp.m_max_value);
            this->m_total_number_ingredients = std::max(this->m_total_number_ingredients, mvp.m_number_ingredients);
        }
        this->m_list.push_back(mvp);
    }

    m_total_ingredient.resize( m_total_number_ingredients );

    return m_list.size();
}
int MultiVolumePropertyList::loadPvtu( const std::string& filename )
{
    int last_time_step = 0;
    int time_step = 0;
    std::unordered_map<int, int> sub_volume_ids;
    std::unordered_map<int, int> sub_volume_counts;
    std::unordered_map<int, int> number_of_nodes;
    std::unordered_map<int, int> number_of_elements;
    std::unordered_map<int, int> number_of_ingredients;
    std::unordered_map<int, kvs::Vec3> min_external_coords;
    std::unordered_map<int, kvs::Vec3> max_external_coords;
    std::unordered_map<int, std::unordered_map<int, kvs::Vec3>> min_object_coords;
    std::unordered_map<int, std::unordered_map<int, kvs::Vec3>> max_object_coords;
    std::unordered_map<int, float> min_values;
    std::unordered_map<int, float> max_values;

    m_list.clear();
    m_total_min_subvolume_coord.clear();
    m_total_max_subvolume_coord.clear();

    kvs::ExtendedFileFormat::VtkXmlPUnstructuredGrid input_pvtu( filename );

    for ( auto vtu : input_pvtu.eachPiece() )
    {
        for ( auto target_cell_type : vtu.eachCellType() )
        {
            kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> importer( &target_cell_type );
            kvs::UnstructuredVolumeObject* object = &importer;
            auto cell_type = object->cellType();
            if ( sub_volume_counts.count( cell_type ) == 0 )
            {
                sub_volume_counts[cell_type] = 1;
                number_of_nodes[cell_type] = object->nnodes();
                number_of_elements[cell_type] = object->ncells();
                number_of_ingredients[cell_type] = object->veclen();
            }
            else
            {
                sub_volume_counts[cell_type] = sub_volume_counts[cell_type] + 1;
                number_of_nodes[cell_type] += object->nnodes();
                number_of_elements[cell_type] += object->ncells();
            }
        }
    }

    // サブボリュームIDを初期化
    for ( auto& e : sub_volume_counts )
    {
        sub_volume_ids[e.first] = 0;
    }

    for ( auto vtu : input_pvtu.eachPiece() )
    {
        for ( auto target_cell_type : vtu.eachCellType() )
        {
            int sub_volume_id;
            kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> importer( &target_cell_type );
            kvs::UnstructuredVolumeObject* object = &importer;
                
            auto cell_type = object->cellType();

            if ( min_external_coords.count(cell_type) == 0 )
            {
                min_external_coords[cell_type][0] = object->minExternalCoord()[0];
                min_external_coords[cell_type][1] = object->minExternalCoord()[1];
                min_external_coords[cell_type][2] = object->minExternalCoord()[2];
                max_external_coords[cell_type][0] = object->maxExternalCoord()[0];
                max_external_coords[cell_type][1] = object->maxExternalCoord()[1];
                max_external_coords[cell_type][2] = object->maxExternalCoord()[2];
                min_values[cell_type] = object->minValue();
                max_values[cell_type] = object->maxValue();
            }
            else
            {
                min_external_coords[cell_type][0] = std::min(min_external_coords[cell_type][0], object->minExternalCoord()[0]);
                min_external_coords[cell_type][1] = std::min(min_external_coords[cell_type][1], object->minExternalCoord()[1]);
                min_external_coords[cell_type][2] = std::min(min_external_coords[cell_type][2], object->minExternalCoord()[2]);
                max_external_coords[cell_type][0] = std::max(max_external_coords[cell_type][0], object->maxExternalCoord()[0]);
                max_external_coords[cell_type][1] = std::max(max_external_coords[cell_type][1], object->maxExternalCoord()[1]);
                max_external_coords[cell_type][2] = std::max(max_external_coords[cell_type][2], object->maxExternalCoord()[2]);
                min_values[cell_type] = std::min(min_values[cell_type], float(object->minValue()));
                max_values[cell_type] = std::max(max_values[cell_type], float(object->maxValue()));
            }

            sub_volume_id = sub_volume_ids[cell_type];

            if ( min_object_coords[cell_type].count(sub_volume_ids[cell_type]) == 0 )
            {
                min_object_coords[cell_type][sub_volume_id][0] = object->minObjectCoord()[0];
                min_object_coords[cell_type][sub_volume_id][1] = object->minObjectCoord()[1];
                min_object_coords[cell_type][sub_volume_id][2] = object->minObjectCoord()[2];
                max_object_coords[cell_type][sub_volume_id][0] = object->maxObjectCoord()[0];
                max_object_coords[cell_type][sub_volume_id][1] = object->maxObjectCoord()[1];
                max_object_coords[cell_type][sub_volume_id][2] = object->maxObjectCoord()[2];
            }
            else
            {
                min_object_coords[cell_type][sub_volume_id][0] = std::min(min_object_coords[cell_type][sub_volume_id][0], object->minObjectCoord()[0]);
                min_object_coords[cell_type][sub_volume_id][1] = std::min(min_object_coords[cell_type][sub_volume_id][1], object->minObjectCoord()[1]);
                min_object_coords[cell_type][sub_volume_id][2] = std::min(min_object_coords[cell_type][sub_volume_id][2], object->minObjectCoord()[2]);
                max_object_coords[cell_type][sub_volume_id][0] = std::max(max_object_coords[cell_type][sub_volume_id][0], object->maxObjectCoord()[0]);
                max_object_coords[cell_type][sub_volume_id][1] = std::max(max_object_coords[cell_type][sub_volume_id][1], object->maxObjectCoord()[1]);
                max_object_coords[cell_type][sub_volume_id][2] = std::max(max_object_coords[cell_type][sub_volume_id][2], object->maxObjectCoord()[2]);
            }

            ++sub_volume_ids[cell_type];
        }
    }

    for (auto& e: sub_volume_ids)
    {
        MultiVolumeProperty mvp;
        auto cell_type = e.first;

        mvp.m_number_nodes = number_of_nodes[cell_type];
        mvp.m_number_elements = number_of_elements[cell_type];
        mvp.m_elem_type = cell_type;
        mvp.m_file_type = 4; // Unstructured
        mvp.m_number_files = sub_volume_counts[cell_type] * (last_time_step + 1);
        mvp.m_number_ingredients = number_of_ingredients[cell_type];
        mvp.m_start_step = 0;
        mvp.m_end_steps = last_time_step;
        mvp.m_number_subvolumes = sub_volume_counts[cell_type];
        float x_min, y_min, z_min;
        float x_max, y_max, z_max;
        x_min = min_external_coords[cell_type][0];
        y_min = min_external_coords[cell_type][1];
        z_min = min_external_coords[cell_type][2];
        x_max = max_external_coords[cell_type][0];
        y_max = max_external_coords[cell_type][1];
        z_max = max_external_coords[cell_type][2];
        mvp.m_min_object_coord.set(x_min, y_min, z_min);
        mvp.m_max_object_coord.set(x_max, y_max, z_max);
        mvp.m_number_steps = mvp.m_end_steps - mvp.m_start_step + 1;
        mvp.m_min_subvolume_coord.resize(mvp.m_number_subvolumes);
        mvp.m_max_subvolume_coord.resize(mvp.m_number_subvolumes);
        mvp.m_file_path = filename;
        mvp.m_min_value = min_values[cell_type];
        mvp.m_max_value = max_values[cell_type];

        for (int sub_volume_id = 0; sub_volume_id < sub_volume_counts[cell_type]; sub_volume_id++)
        {
            float sub_x_min, sub_y_min, sub_z_min;
            float sub_x_max, sub_y_max, sub_z_max;
            sub_x_min = min_object_coords[cell_type][sub_volume_id][0];
            sub_y_min = min_object_coords[cell_type][sub_volume_id][1];
            sub_z_min = min_object_coords[cell_type][sub_volume_id][2];
            sub_x_max = max_object_coords[cell_type][sub_volume_id][0];
            sub_y_max = max_object_coords[cell_type][sub_volume_id][1];
            sub_z_max = max_object_coords[cell_type][sub_volume_id][2];
            mvp.m_min_subvolume_coord[sub_volume_id].set(sub_x_min, sub_y_min, sub_z_min);
            mvp.m_max_subvolume_coord[sub_volume_id].set(sub_x_max, sub_y_max, sub_z_max);
        }

// for debug
#if 0
        std::cout << "==================== cell type:" << cell_type << " start =========================" << std::endl;
        std::cout << "m_number_nodes:" << mvp.m_number_nodes << std::endl;
        std::cout << "m_number_elements:" << mvp.m_number_elements << std::endl;
        std::cout << "m_number_files:" << mvp.m_number_files << std::endl;
        std::cout << "m_start_step:" << mvp.m_start_step << std::endl;
        std::cout << "m_end_steps:" << mvp.m_end_steps << std::endl;
        std::cout << "m_number_steps:" << mvp.m_number_steps << std::endl;
        std::cout << "m_number_subvolumes:" << mvp.m_number_subvolumes << std::endl;
        std::cout << "m_min_object_coord[0]:" << mvp.m_min_object_coord[0] << std::endl;
        std::cout << "m_min_object_coord[1]:" << mvp.m_min_object_coord[1] << std::endl;
        std::cout << "m_min_object_coord[2]:" << mvp.m_min_object_coord[2] << std::endl;
        std::cout << "m_max_object_coord[0]:" << mvp.m_max_object_coord[0] << std::endl;
        std::cout << "m_max_object_coord[1]:" << mvp.m_max_object_coord[1] << std::endl;
        std::cout << "m_max_object_coord[2]:" << mvp.m_max_object_coord[2] << std::endl;
        for (int sub_volume_id = 0; sub_volume_id < sub_volume_counts[cell_type]; sub_volume_id++)
        {
            std::cout << "m_min_subvolume_coord[" << sub_volume_id << "][0]:" << mvp.m_min_subvolume_coord[sub_volume_id][0] << std::endl;
            std::cout << "m_min_subvolume_coord[" << sub_volume_id << "][1]:" << mvp.m_min_subvolume_coord[sub_volume_id][1] << std::endl;
            std::cout << "m_min_subvolume_coord[" << sub_volume_id << "][2]:" << mvp.m_min_subvolume_coord[sub_volume_id][2] << std::endl;
            std::cout << "m_max_subvolume_coord[" << sub_volume_id << "][0]:" << mvp.m_max_subvolume_coord[sub_volume_id][0] << std::endl;
            std::cout << "m_max_subvolume_coord[" << sub_volume_id << "][1]:" << mvp.m_max_subvolume_coord[sub_volume_id][1] << std::endl;
            std::cout << "m_max_subvolume_coord[" << sub_volume_id << "][2]:" << mvp.m_max_subvolume_coord[sub_volume_id][2] << std::endl;
        }
        std::cout << "m_min_value:" << mvp.m_min_value << std::endl;
        std::cout << "m_max_value:" << mvp.m_max_value << std::endl;
        std::cout << "m_number_ingredients:" << mvp.m_number_ingredients << std::endl;
        std::cout << "==================== cell type:" << cell_type << " end =========================" << std::endl;
#endif

        if ( this->m_list.empty() )
        {
            this->m_total_number_nodes = mvp.m_number_nodes;
            this->m_total_number_elements = mvp.m_number_elements;
            this->m_total_number_files = mvp.m_number_files;
            this->m_total_start_steps = mvp.m_start_step;
            this->m_total_last_step = mvp.m_end_steps;
            this->m_total_number_steps = mvp.m_number_steps;
            this->m_total_number_subvolumes = mvp.m_number_subvolumes;
            this->m_total_min_object_coord = mvp.m_min_object_coord;
            this->m_total_max_object_coord = mvp.m_max_object_coord;
            this->m_total_min_subvolume_coord = mvp.m_min_subvolume_coord;
            this->m_total_max_subvolume_coord = mvp.m_max_subvolume_coord;
            this->m_total_min_value = mvp.m_min_value;
            this->m_total_max_value = mvp.m_max_value;
            this->m_total_number_ingredients = mvp.m_number_ingredients;
        }
        else
        {
            this->m_total_number_nodes += mvp.m_number_nodes;
            this->m_total_number_elements += mvp.m_number_elements;
            this->m_total_number_files += mvp.m_number_files;
            this->m_total_start_steps = std::min(this->m_total_start_steps, mvp.m_start_step);
            this->m_total_last_step = std::max(this->m_total_last_step, mvp.m_end_steps);
            this->m_total_number_steps = this->m_total_last_step - this->m_total_start_steps + 1;
            this->m_total_number_subvolumes += mvp.m_number_subvolumes;
            this->m_total_min_object_coord[0] = std::min(this->m_total_min_object_coord[0], mvp.m_min_object_coord[0]);
            this->m_total_min_object_coord[1] = std::min(this->m_total_min_object_coord[1], mvp.m_min_object_coord[1]);
            this->m_total_min_object_coord[2] = std::min(this->m_total_min_object_coord[2], mvp.m_min_object_coord[2]);
            this->m_total_max_object_coord[0] = std::max(this->m_total_max_object_coord[0], mvp.m_max_object_coord[0]);
            this->m_total_max_object_coord[1] = std::max(this->m_total_max_object_coord[1], mvp.m_max_object_coord[1]);
            this->m_total_max_object_coord[2] = std::max(this->m_total_max_object_coord[2], mvp.m_max_object_coord[2]);
            std::copy(mvp.m_min_subvolume_coord.begin(), mvp.m_min_subvolume_coord.end(), std::back_inserter(this->m_total_min_subvolume_coord));
            std::copy(mvp.m_max_subvolume_coord.begin(), mvp.m_max_subvolume_coord.end(), std::back_inserter(this->m_total_max_subvolume_coord));
            this->m_total_min_value = std::min(this->m_total_min_value, mvp.m_min_value);
            this->m_total_max_value = std::max(this->m_total_max_value, mvp.m_max_value);
            this->m_total_number_ingredients = std::max(this->m_total_number_ingredients, mvp.m_number_ingredients);
        }
        this->m_list.push_back(mvp);
    }

    m_total_ingredient.resize( m_total_number_ingredients );

    return m_list.size();   
}
int MultiVolumePropertyList::loadSeriesPvtu( const std::string& filename )
{
    namespace fs = std::filesystem;
    fs::path filepath = filename;
    kvs::ExtendedFileFormat::NumeralSequenceFiles<kvs::ExtendedFileFormat::VtkXmlPUnstructuredGrid> time_series( filepath.generic_string() );
    int last_time_step = time_series.numberOfFiles() - 1;
    int time_step = 0;
    std::unordered_map<int, int> sub_volume_ids;
    std::unordered_map<int, int> sub_volume_counts;
    std::unordered_map<int, int> number_of_nodes;
    std::unordered_map<int, int> number_of_elements;
    std::unordered_map<int, int> number_of_ingredients;
    std::unordered_map<int, kvs::Vec3> min_external_coords;
    std::unordered_map<int, kvs::Vec3> max_external_coords;
    std::unordered_map<int, std::unordered_map<int, kvs::Vec3>> min_object_coords;
    std::unordered_map<int, std::unordered_map<int, kvs::Vec3>> max_object_coords;
    std::unordered_map<int, float> min_values;
    std::unordered_map<int, float> max_values;

    m_list.clear();
    m_total_min_subvolume_coord.clear();
    m_total_max_subvolume_coord.clear();

    for ( auto pvtu : time_series.eachTimeStep() )
    {
        if ( time_step == 0 )
        {
            for ( auto vtu : pvtu.eachPiece() )
            {
                for ( auto target_cell_type : vtu.eachCellType() )
                {
                    kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> importer( &target_cell_type );
                    kvs::UnstructuredVolumeObject* object = &importer;
                    auto cell_type = object->cellType();

                    if ( sub_volume_counts.count( cell_type ) == 0 )
                    {
                        sub_volume_counts[cell_type] = 1;
                        number_of_nodes[cell_type] = object->nnodes();
                        number_of_elements[cell_type] = object->ncells();
                        number_of_ingredients[cell_type] = object->veclen();
                    }
                    else
                    {
                        sub_volume_counts[cell_type] = sub_volume_counts[cell_type] + 1;
                        number_of_nodes[cell_type] += object->nnodes();
                        number_of_elements[cell_type] += object->ncells();
                    }
                }
            }
        }
        
        // Two-pass
        for ( auto& e : sub_volume_counts )
        {
            sub_volume_ids[e.first] = 0;
        }	
        
        for ( auto vtu : pvtu.eachPiece() )
        {
            for ( auto target_cell_type : vtu.eachCellType() )
            {
                int sub_volume_id;
                kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> importer( &target_cell_type );
                kvs::UnstructuredVolumeObject* object = &importer;
                    
                auto cell_type = object->cellType();

                if ( min_external_coords.count(cell_type) == 0 )
                {
                    min_external_coords[cell_type][0] = object->minExternalCoord()[0];
                    min_external_coords[cell_type][1] = object->minExternalCoord()[1];
                    min_external_coords[cell_type][2] = object->minExternalCoord()[2];
                    max_external_coords[cell_type][0] = object->maxExternalCoord()[0];
                    max_external_coords[cell_type][1] = object->maxExternalCoord()[1];
                    max_external_coords[cell_type][2] = object->maxExternalCoord()[2];
                    min_values[cell_type] = object->minValue();
                    max_values[cell_type] = object->maxValue();
                }
                else
                {
                    min_external_coords[cell_type][0] = std::min(min_external_coords[cell_type][0], object->minExternalCoord()[0]);
                    min_external_coords[cell_type][1] = std::min(min_external_coords[cell_type][1], object->minExternalCoord()[1]);
                    min_external_coords[cell_type][2] = std::min(min_external_coords[cell_type][2], object->minExternalCoord()[2]);
                    max_external_coords[cell_type][0] = std::max(max_external_coords[cell_type][0], object->maxExternalCoord()[0]);
                    max_external_coords[cell_type][1] = std::max(max_external_coords[cell_type][1], object->maxExternalCoord()[1]);
                    max_external_coords[cell_type][2] = std::max(max_external_coords[cell_type][2], object->maxExternalCoord()[2]);
                    min_values[cell_type] = std::min(min_values[cell_type], float(object->minValue()));
                    max_values[cell_type] = std::max(max_values[cell_type], float(object->maxValue()));
                }

		        sub_volume_id = sub_volume_ids[cell_type];

                if ( min_object_coords[cell_type].count(sub_volume_ids[cell_type]) == 0 )
                {
                    min_object_coords[cell_type][sub_volume_id][0] = object->minObjectCoord()[0];
                    min_object_coords[cell_type][sub_volume_id][1] = object->minObjectCoord()[1];
                    min_object_coords[cell_type][sub_volume_id][2] = object->minObjectCoord()[2];
                    max_object_coords[cell_type][sub_volume_id][0] = object->maxObjectCoord()[0];
                    max_object_coords[cell_type][sub_volume_id][1] = object->maxObjectCoord()[1];
                    max_object_coords[cell_type][sub_volume_id][2] = object->maxObjectCoord()[2];
                }
                else
                {
                    min_object_coords[cell_type][sub_volume_id][0] = std::min(min_object_coords[cell_type][sub_volume_id][0], object->minObjectCoord()[0]);
                    min_object_coords[cell_type][sub_volume_id][1] = std::min(min_object_coords[cell_type][sub_volume_id][1], object->minObjectCoord()[1]);
                    min_object_coords[cell_type][sub_volume_id][2] = std::min(min_object_coords[cell_type][sub_volume_id][2], object->minObjectCoord()[2]);
                    max_object_coords[cell_type][sub_volume_id][0] = std::max(max_object_coords[cell_type][sub_volume_id][0], object->maxObjectCoord()[0]);
                    max_object_coords[cell_type][sub_volume_id][1] = std::max(max_object_coords[cell_type][sub_volume_id][1], object->maxObjectCoord()[1]);
                    max_object_coords[cell_type][sub_volume_id][2] = std::max(max_object_coords[cell_type][sub_volume_id][2], object->maxObjectCoord()[2]);
                }

		        ++sub_volume_ids[cell_type];
            }
        }
        ++time_step;
    }

    for (auto& e: sub_volume_ids)
    {
        MultiVolumeProperty mvp;
        auto cell_type = e.first;

        mvp.m_number_nodes = number_of_nodes[cell_type];
        mvp.m_number_elements = number_of_elements[cell_type];
        mvp.m_elem_type = cell_type;
        mvp.m_file_type = 4; // Unstructured
        mvp.m_number_files = sub_volume_counts[cell_type] * (last_time_step + 1);
        mvp.m_number_ingredients = number_of_ingredients[cell_type];
        mvp.m_start_step = 0;
        mvp.m_end_steps = last_time_step;
        mvp.m_number_subvolumes = sub_volume_counts[cell_type];
        float x_min, y_min, z_min;
        float x_max, y_max, z_max;
        x_min = min_external_coords[cell_type][0];
        y_min = min_external_coords[cell_type][1];
        z_min = min_external_coords[cell_type][2];
        x_max = max_external_coords[cell_type][0];
        y_max = max_external_coords[cell_type][1];
        z_max = max_external_coords[cell_type][2];
        mvp.m_min_object_coord.set(x_min, y_min, z_min);
        mvp.m_max_object_coord.set(x_max, y_max, z_max);
        mvp.m_number_steps = mvp.m_end_steps - mvp.m_start_step + 1;
        mvp.m_min_subvolume_coord.resize(mvp.m_number_subvolumes);
        mvp.m_max_subvolume_coord.resize(mvp.m_number_subvolumes);
        mvp.m_file_path = filename;
        mvp.m_min_value = min_values[cell_type];
        mvp.m_max_value = max_values[cell_type];

        for (int sub_volume_id = 0; sub_volume_id < sub_volume_counts[cell_type]; sub_volume_id++)
        {
            float sub_x_min, sub_y_min, sub_z_min;
            float sub_x_max, sub_y_max, sub_z_max;
            sub_x_min = min_object_coords[cell_type][sub_volume_id][0];
            sub_y_min = min_object_coords[cell_type][sub_volume_id][1];
            sub_z_min = min_object_coords[cell_type][sub_volume_id][2];
            sub_x_max = max_object_coords[cell_type][sub_volume_id][0];
            sub_y_max = max_object_coords[cell_type][sub_volume_id][1];
            sub_z_max = max_object_coords[cell_type][sub_volume_id][2];
            mvp.m_min_subvolume_coord[sub_volume_id].set(sub_x_min, sub_y_min, sub_z_min);
            mvp.m_max_subvolume_coord[sub_volume_id].set(sub_x_max, sub_y_max, sub_z_max);
        }

// for debug
#if 0
        std::cout << "==================== cell type:" << cell_type << " start =========================" << std::endl;
        std::cout << "m_number_nodes:" << mvp.m_number_nodes << std::endl;
        std::cout << "m_number_elements:" << mvp.m_number_elements << std::endl;
        std::cout << "m_number_files:" << mvp.m_number_files << std::endl;
        std::cout << "m_start_step:" << mvp.m_start_step << std::endl;
        std::cout << "m_end_steps:" << mvp.m_end_steps << std::endl;
        std::cout << "m_number_steps:" << mvp.m_number_steps << std::endl;
        std::cout << "m_number_subvolumes:" << mvp.m_number_subvolumes << std::endl;
        std::cout << "m_min_object_coord[0]:" << mvp.m_min_object_coord[0] << std::endl;
        std::cout << "m_min_object_coord[1]:" << mvp.m_min_object_coord[1] << std::endl;
        std::cout << "m_min_object_coord[2]:" << mvp.m_min_object_coord[2] << std::endl;
        std::cout << "m_max_object_coord[0]:" << mvp.m_max_object_coord[0] << std::endl;
        std::cout << "m_max_object_coord[1]:" << mvp.m_max_object_coord[1] << std::endl;
        std::cout << "m_max_object_coord[2]:" << mvp.m_max_object_coord[2] << std::endl;
        for (int sub_volume_id = 0; sub_volume_id < sub_volume_counts[cell_type]; sub_volume_id++)
        {
            std::cout << "m_min_subvolume_coord[" << sub_volume_id << "][0]:" << mvp.m_min_subvolume_coord[sub_volume_id][0] << std::endl;
            std::cout << "m_min_subvolume_coord[" << sub_volume_id << "][1]:" << mvp.m_min_subvolume_coord[sub_volume_id][1] << std::endl;
            std::cout << "m_min_subvolume_coord[" << sub_volume_id << "][2]:" << mvp.m_min_subvolume_coord[sub_volume_id][2] << std::endl;
            std::cout << "m_max_subvolume_coord[" << sub_volume_id << "][0]:" << mvp.m_max_subvolume_coord[sub_volume_id][0] << std::endl;
            std::cout << "m_max_subvolume_coord[" << sub_volume_id << "][1]:" << mvp.m_max_subvolume_coord[sub_volume_id][1] << std::endl;
            std::cout << "m_max_subvolume_coord[" << sub_volume_id << "][2]:" << mvp.m_max_subvolume_coord[sub_volume_id][2] << std::endl;
        }
        std::cout << "m_min_value:" << mvp.m_min_value << std::endl;
        std::cout << "m_max_value:" << mvp.m_max_value << std::endl;
        std::cout << "m_number_ingredients:" << mvp.m_number_ingredients << std::endl;
        std::cout << "==================== cell type:" << cell_type << " end =========================" << std::endl;
#endif

        if ( this->m_list.empty() )
        {
            this->m_total_number_nodes = mvp.m_number_nodes;
            this->m_total_number_elements = mvp.m_number_elements;
            this->m_total_number_files = mvp.m_number_files;
            this->m_total_start_steps = mvp.m_start_step;
            this->m_total_last_step = mvp.m_end_steps;
            this->m_total_number_steps = mvp.m_number_steps;
            this->m_total_number_subvolumes = mvp.m_number_subvolumes;
            this->m_total_min_object_coord = mvp.m_min_object_coord;
            this->m_total_max_object_coord = mvp.m_max_object_coord;
            this->m_total_min_subvolume_coord = mvp.m_min_subvolume_coord;
            this->m_total_max_subvolume_coord = mvp.m_max_subvolume_coord;
            this->m_total_min_value = mvp.m_min_value;
            this->m_total_max_value = mvp.m_max_value;
            this->m_total_number_ingredients = mvp.m_number_ingredients;
        }
        else
        {
            this->m_total_number_nodes += mvp.m_number_nodes;
            this->m_total_number_elements += mvp.m_number_elements;
            this->m_total_number_files += mvp.m_number_files;
            this->m_total_start_steps = std::min(this->m_total_start_steps, mvp.m_start_step);
            this->m_total_last_step = std::max(this->m_total_last_step, mvp.m_end_steps);
            this->m_total_number_steps = this->m_total_last_step - this->m_total_start_steps + 1;
            this->m_total_number_subvolumes += mvp.m_number_subvolumes;
            this->m_total_min_object_coord[0] = std::min(this->m_total_min_object_coord[0], mvp.m_min_object_coord[0]);
            this->m_total_min_object_coord[1] = std::min(this->m_total_min_object_coord[1], mvp.m_min_object_coord[1]);
            this->m_total_min_object_coord[2] = std::min(this->m_total_min_object_coord[2], mvp.m_min_object_coord[2]);
            this->m_total_max_object_coord[0] = std::max(this->m_total_max_object_coord[0], mvp.m_max_object_coord[0]);
            this->m_total_max_object_coord[1] = std::max(this->m_total_max_object_coord[1], mvp.m_max_object_coord[1]);
            this->m_total_max_object_coord[2] = std::max(this->m_total_max_object_coord[2], mvp.m_max_object_coord[2]);
            std::copy(mvp.m_min_subvolume_coord.begin(), mvp.m_min_subvolume_coord.end(), std::back_inserter(this->m_total_min_subvolume_coord));
            std::copy(mvp.m_max_subvolume_coord.begin(), mvp.m_max_subvolume_coord.end(), std::back_inserter(this->m_total_max_subvolume_coord));
            this->m_total_min_value = std::min(this->m_total_min_value, mvp.m_min_value);
            this->m_total_max_value = std::max(this->m_total_max_value, mvp.m_max_value);
            this->m_total_number_ingredients = std::max(this->m_total_number_ingredients, mvp.m_number_ingredients);
        }
        this->m_list.push_back(mvp);
    }

    m_total_ingredient.resize( m_total_number_ingredients );

    return m_list.size();
}

int MultiVolumePropertyList::loadEnsightGold( const std::string& filename )
{
    kvs::ExtendedFileFormat::EnSightGoldBinary input_case( filename );
    auto time_steps_container = input_case.eachTimeStep();
    int last_time_step = time_steps_container.lastTimeStep();
    int time_step = 0;
    int sub_volume_count = 0;
    int cell_type = 0;
    int number_of_nodes;
    int number_of_elements;
    int number_of_ingredients;
    kvs::Vec3 min_external_coords;
    kvs::Vec3 max_external_coords;
    std::unordered_map<int, kvs::Vec3> min_object_coords;
    std::unordered_map<int, kvs::Vec3> max_object_coords;
    float min_value;
    float max_value;
    MultiVolumeProperty mvp;
    float x_min, y_min, z_min;
    float x_max, y_max, z_max;
    float sub_x_min, sub_y_min, sub_z_min;
    float sub_x_max, sub_y_max, sub_z_max;

    m_list.clear();
    m_total_min_subvolume_coord.clear();
    m_total_max_subvolume_coord.clear();

    for ( auto time_and_format : time_steps_container )
    {
        auto time = time_and_format.first;
        auto& multi_block_format = time_and_format.second;
        int sub_volume_id = 0;

        if ( time_step == 0 )
        {
            for ( auto format : multi_block_format.eachBlock() )
            {
                if ( format )
                {
                    if ( auto unstructured_volume_format = dynamic_cast<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid*>( format.get() ) )
                    {
                        kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> importer( unstructured_volume_format );
                        kvs::UnstructuredVolumeObject* object = &importer;
                        cell_type = object->cellType();

                        if ( cell_type == kvs::UnstructuredVolumeObject::CellType::UnknownCellType )
                        {
                            continue;
                        }

                        if ( sub_volume_count == 0 )
                        {
                            sub_volume_count = 1;
                            number_of_nodes = object->nnodes();
                            number_of_elements = object->ncells();
                            number_of_ingredients = object->veclen();
                        }
                        else
                        {
                            ++sub_volume_count;
                            number_of_nodes += object->nnodes();
                            number_of_elements += object->ncells();
                        }
                    }
                }
            }
        }


        for ( auto format : multi_block_format.eachBlock() )
        {
            if ( !format )
            {
                std::cout << "Unsupported VTK data type" << std::endl;
            }
            else if ( auto unstructured_volume_format = dynamic_cast<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid*>( format.get() ) )
            {
                try
                {
                    kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> importer( unstructured_volume_format );
                    kvs::UnstructuredVolumeObject* object = &importer;

                    if ( object->cellType() == kvs::UnstructuredVolumeObject::CellType::UnknownCellType )
                    {
                        continue;
                    }

                    if ( time_step == 0 && sub_volume_id == 0 )
                    {
                        min_external_coords[0] = object->minExternalCoord()[0];
                        min_external_coords[1] = object->minExternalCoord()[1];
                        min_external_coords[2] = object->minExternalCoord()[2];
                        max_external_coords[0] = object->maxExternalCoord()[0];
                        max_external_coords[1] = object->maxExternalCoord()[1];
                        max_external_coords[2] = object->maxExternalCoord()[2];
                        min_value = object->minValue();
                        max_value = object->maxValue();
                    }
                    else
                    {
                        min_external_coords[0] = std::min(min_external_coords[0], object->minExternalCoord()[0]);
                        min_external_coords[1] = std::min(min_external_coords[1], object->minExternalCoord()[1]);
                        min_external_coords[2] = std::min(min_external_coords[2], object->minExternalCoord()[2]);
                        max_external_coords[0] = std::max(max_external_coords[0], object->maxExternalCoord()[0]);
                        max_external_coords[1] = std::max(max_external_coords[1], object->maxExternalCoord()[1]);
                        max_external_coords[2] = std::max(max_external_coords[2], object->maxExternalCoord()[2]);
                        min_value = std::min(min_value, float(object->minValue()));
                        max_value = std::max(max_value, float(object->maxValue()));
                    }

                    if ( time_step == 0 )
                    {
                        min_object_coords[sub_volume_id][0] = object->minObjectCoord()[0];
                        min_object_coords[sub_volume_id][1] = object->minObjectCoord()[1];
                        min_object_coords[sub_volume_id][2] = object->minObjectCoord()[2];
                        max_object_coords[sub_volume_id][0] = object->maxObjectCoord()[0];
                        max_object_coords[sub_volume_id][1] = object->maxObjectCoord()[1];
                        max_object_coords[sub_volume_id][2] = object->maxObjectCoord()[2];
                    }
                    else
                    {
                        min_object_coords[sub_volume_id][0] = std::min(min_object_coords[sub_volume_id][0], object->minObjectCoord()[0]);
                        min_object_coords[sub_volume_id][1] = std::min(min_object_coords[sub_volume_id][1], object->minObjectCoord()[1]);
                        min_object_coords[sub_volume_id][2] = std::min(min_object_coords[sub_volume_id][2], object->minObjectCoord()[2]);
                        max_object_coords[sub_volume_id][0] = std::max(max_object_coords[sub_volume_id][0], object->maxObjectCoord()[0]);
                        max_object_coords[sub_volume_id][1] = std::max(max_object_coords[sub_volume_id][1], object->maxObjectCoord()[1]);
                        max_object_coords[sub_volume_id][2] = std::max(max_object_coords[sub_volume_id][2], object->maxObjectCoord()[2]);
                    }
                }
                catch ( std::runtime_error& e )
                {
                    std::cout << e.what() << std::endl;
                }
            }
            else
            {
                std::cout << "Supported, but non-unstructured grid" << std::endl;
            }
            ++sub_volume_id;
        }
        ++time_step;
    }

    mvp.m_number_nodes = number_of_nodes;
    mvp.m_number_elements = number_of_elements;
    mvp.m_elem_type = cell_type;
    mvp.m_file_type = 4; // Unstructured
    mvp.m_number_files = sub_volume_count * ( last_time_step + 1 );
    mvp.m_number_ingredients = number_of_ingredients;
    mvp.m_start_step = 0;
    mvp.m_end_steps = last_time_step;
    mvp.m_number_subvolumes = sub_volume_count;
    x_min = min_external_coords[0];
    y_min = min_external_coords[1];
    z_min = min_external_coords[2];
    x_max = max_external_coords[0];
    y_max = max_external_coords[1];
    z_max = max_external_coords[2];
    mvp.m_min_object_coord.set(x_min, y_min, z_min);
    mvp.m_max_object_coord.set(x_max, y_max, z_max);
    mvp.m_number_steps = mvp.m_end_steps - mvp.m_start_step + 1;
    mvp.m_min_subvolume_coord.resize(mvp.m_number_subvolumes);
    mvp.m_max_subvolume_coord.resize(mvp.m_number_subvolumes);
    mvp.m_file_path = filename;
    mvp.m_min_value = min_value;
    mvp.m_max_value = max_value;

    for ( int sub_volume_id = 0; sub_volume_id < sub_volume_count; sub_volume_id++ )
    {
        float sub_x_min, sub_y_min, sub_z_min;
        float sub_x_max, sub_y_max, sub_z_max;
        sub_x_min = min_object_coords[sub_volume_id][0];
        sub_y_min = min_object_coords[sub_volume_id][1];
        sub_z_min = min_object_coords[sub_volume_id][2];
        sub_x_max = max_object_coords[sub_volume_id][0];
        sub_y_max = max_object_coords[sub_volume_id][1];
        sub_z_max = max_object_coords[sub_volume_id][2];
        mvp.m_min_subvolume_coord[sub_volume_id].set(sub_x_min, sub_y_min, sub_z_min);
        mvp.m_max_subvolume_coord[sub_volume_id].set(sub_x_max, sub_y_max, sub_z_max);
    }

// for debug
#if 0
    std::cout << "==================== cell type:" << cell_type << " start =========================" << std::endl;
    std::cout << "m_number_nodes:" << mvp.m_number_nodes << std::endl;
    std::cout << "m_number_elements:" << mvp.m_number_elements << std::endl;
    std::cout << "m_number_files:" << mvp.m_number_files << std::endl;
    std::cout << "m_start_step:" << mvp.m_start_step << std::endl;
    std::cout << "m_end_steps:" << mvp.m_end_steps << std::endl;
    std::cout << "m_number_steps:" << mvp.m_number_steps << std::endl;
    std::cout << "m_number_subvolumes:" << mvp.m_number_subvolumes << std::endl;
    std::cout << "m_min_object_coord[0]:" << mvp.m_min_object_coord[0] << std::endl;
    std::cout << "m_min_object_coord[1]:" << mvp.m_min_object_coord[1] << std::endl;
    std::cout << "m_min_object_coord[2]:" << mvp.m_min_object_coord[2] << std::endl;
    std::cout << "m_max_object_coord[0]:" << mvp.m_max_object_coord[0] << std::endl;
    std::cout << "m_max_object_coord[1]:" << mvp.m_max_object_coord[1] << std::endl;
    std::cout << "m_max_object_coord[2]:" << mvp.m_max_object_coord[2] << std::endl;
    for (int sub_volume_id = 0; sub_volume_id < sub_volume_count; sub_volume_id++)
    {
        std::cout << "m_min_subvolume_coord[" << sub_volume_id << "][0]:" << mvp.m_min_subvolume_coord[sub_volume_id][0] << std::endl;
        std::cout << "m_min_subvolume_coord[" << sub_volume_id << "][1]:" << mvp.m_min_subvolume_coord[sub_volume_id][1] << std::endl;
        std::cout << "m_min_subvolume_coord[" << sub_volume_id << "][2]:" << mvp.m_min_subvolume_coord[sub_volume_id][2] << std::endl;
        std::cout << "m_max_subvolume_coord[" << sub_volume_id << "][0]:" << mvp.m_max_subvolume_coord[sub_volume_id][0] << std::endl;
        std::cout << "m_max_subvolume_coord[" << sub_volume_id << "][1]:" << mvp.m_max_subvolume_coord[sub_volume_id][1] << std::endl;
    }
    std::cout << "m_min_value:" << mvp.m_min_value << std::endl;
    std::cout << "m_max_value:" << mvp.m_max_value << std::endl;
    std::cout << "m_number_ingredients:" << mvp.m_number_ingredients << std::endl;
    std::cout << "==================== cell type:" << cell_type << " end =========================" << std::endl;
#endif

    this->m_total_number_nodes = mvp.m_number_nodes;
    this->m_total_number_elements = mvp.m_number_elements;
    this->m_total_number_files = mvp.m_number_files;
    this->m_total_start_steps = mvp.m_start_step;
    this->m_total_last_step = mvp.m_end_steps;
    this->m_total_number_steps = mvp.m_number_steps;
    this->m_total_number_subvolumes = mvp.m_number_subvolumes;
    this->m_total_min_object_coord = mvp.m_min_object_coord;
    this->m_total_max_object_coord = mvp.m_max_object_coord;
    this->m_total_min_subvolume_coord = mvp.m_min_subvolume_coord;
    this->m_total_max_subvolume_coord = mvp.m_max_subvolume_coord;
    this->m_total_min_value = mvp.m_min_value;
    this->m_total_max_value = mvp.m_max_value;
    this->m_total_number_ingredients = mvp.m_number_ingredients;
    this->m_list.push_back(mvp);

    m_total_ingredient.resize( m_total_number_ingredients );

    return m_list.size();
}
#endif

void MultiVolumePropertyList::calculate_ingredient_min_max( const MultiVolumeProperty &mvp, 
                                                         std::vector<MultiVolumeProperty::IngredientsMinMax> *total_ingredient )
{
    for ( int32_t i = 0; i < mvp.m_number_ingredients; i++ )
    {
        float min = mvp.m_ingredient_step[0].m_ingredient[i].m_min;
        float max = mvp.m_ingredient_step[0].m_ingredient[i].m_max;
        (*total_ingredient)[i].m_min = std::min(min, (*total_ingredient)[i].m_min);
        (*total_ingredient)[i].m_max = std::max(max, (*total_ingredient)[i].m_max);
    }
}

int MultiVolumePropertyList::getFileIndex( const int vl, int* xvl ) const
{
    if ( m_list.empty() ) return -1;
    if ( vl >= m_total_number_subvolumes ) return -1;

    int idx, cvl = vl;
    for ( idx = 0; idx < m_list.size(); idx++ )
    {
        if ( cvl < m_list[idx].m_number_subvolumes ) break;
        cvl -= m_list[idx].m_number_subvolumes;
    }
    if ( idx >= m_list.size() ) return -1;
    *xvl = cvl;
    return idx;
}

void MultiVolumePropertyList::cropTimeStep( const int s, const int e )
{
    if ( m_list.empty() ) return;
    if ( s > e ) return;

    if ( s != INT_MIN )
    {
        if ( s > m_total_start_steps )
            m_total_start_steps = s;
    }

    if ( e != INT_MAX )
    {
        if ( e < m_total_last_step )
            m_total_last_step = e;
    }

    m_total_number_steps = m_total_last_step - m_total_start_steps + 1;
}
