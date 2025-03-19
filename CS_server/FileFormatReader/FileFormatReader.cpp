#include "FileFormatReader.h"

#include "FileFormat/VTK/VtkXmlUnstructuredGrid.h"
#include "PBVRFileInformation/Pfl.h"
#include "Importer/VtkImporter.h"
#include "Exporter/UnstructuredVolumeObjectExporter.h"

std::unordered_map<int, cvt::UnstructuredPfi> FileFormatReader::ConvertUnstructuredPfiMap( std::string file_path )
{
    std::unordered_map<int, cvt::UnstructuredPfi> pfi_map;
    std::cout << "Reading " << file_path << " ..." << std::endl;
    // 変換前ファイルパスからファイルフォーマットクラスを作成
    cvt::VtkXmlUnstructuredGrid input_vtu(file_path);

    // セルタイプ毎に処理を実施
    for (auto vtu : input_vtu.eachCellType())
    {
        int time_step = 0;
        int last_time_step = 0;
        int sub_volume_id = 1;
        int sub_volume_count = 1;

        // ファイルフォーマットからimporterに変換
        cvt::VtkImporter<cvt::VtkXmlUnstructuredGrid> importer(&vtu);
        std::cout << "  cell type: " << importer.cellType() << std::endl;

        // importerからObjectに変換
        kvs::UnstructuredVolumeObject* object = &importer;
        // Objectからexporterに変換
        cvt::UnstructuredVolumeObjectExporter exporter(&importer);

        // 最初のタイムステップの場合cvt::UnstructuredPfiクラスのインスタンスを作成
        if (time_step == 0)
        {
            pfi_map.emplace(
                static_cast<int>(object->cellType()),
                cvt::UnstructuredPfi(object->veclen(), last_time_step, sub_volume_count)
            );
        }
        // 生成したクラスにオブジェクトを登録する
        pfi_map.at(static_cast<int>(object->cellType()))
            .registerObject(&exporter, time_step, sub_volume_id);
    }

    return pfi_map;
}

MultiVolumePropertyList FileFormatReader::ConvertFilterInformationList( std::string  file_path )
{
    MultiVolumePropertyList mvpl;
    mvpl.m_list.clear();
    mvpl.m_total_min_subvolume_coord.clear();
    mvpl.m_total_max_subvolume_coord.clear();

    std::unordered_map<int, cvt::UnstructuredPfi> pfi_map;
    pfi_map = ConvertUnstructuredPfiMap(file_path);

    // 連想配列に格納しているcvt::UnstructuredPfiクラス毎に処理を実施
    for (auto& e : pfi_map)
    {
        // キーにはセルタイプ, 値にcvt::UnstructuredPfiクラスのインスタンス
        cvt::UnstructuredPfi pfi = e.second;
        MultiVolumeProperty mvp;

        // cvt::UnstructuredPfiクラスの情報をMultiVolumePropertyクラスに格納する
        int number_of_nodes = std::accumulate(pfi.node_counts[0].begin(), pfi.node_counts[0].end(), 0);
        mvp.m_number_nodes = number_of_nodes;

        int number_of_elements = std::accumulate(pfi.cell_counts[0].begin(), pfi.cell_counts[0].end(), 0);
        mvp.m_number_elements = number_of_elements;

        mvp.m_elem_type = pfi.type_of_cells;

        int type_of_file = 0;
        mvp.m_file_type = type_of_file;

        int number_of_file = pfi.max_sub_volume_id * (pfi.last_time_step + 1);
        mvp.m_number_files = number_of_file;
    
        mvp.m_number_ingredients = pfi.number_of_component;

        int step_of_beginning = 0;
        mvp.m_start_step = step_of_beginning;

        int step_of_end = pfi.last_time_step;
        mvp.m_end_steps = step_of_end;
        
        int number_of_sub_volumes = pfi.max_sub_volume_id;
        mvp.m_number_subvolumes = number_of_sub_volumes;

        float x_min, y_min, z_min;
        float x_max, y_max, z_max;
        x_min = pfi.min_external_coords[0];
        y_min = pfi.min_external_coords[1];
        z_min = pfi.min_external_coords[2];
        x_max = pfi.max_external_coords[0];
        y_max = pfi.max_external_coords[1];
        z_max = pfi.max_external_coords[2];
        mvp.m_min_object_coord.set(x_min, y_min, z_min);
        mvp.m_max_object_coord.set(x_max, y_max, z_max);

        std::cout << "m_end_steps = " << mvp.m_end_steps << std::endl;
        std::cout << "m_start_step = " << mvp.m_start_step << std::endl;
        mvp.m_number_steps = mvp.m_end_steps - mvp.m_start_step + 1;

        mvp.m_min_subvolume_coord.resize(mvp.m_number_subvolumes);
        mvp.m_max_subvolume_coord.resize(mvp.m_number_subvolumes);

        mvp.m_file_path = file_path;

        for (int vl = 0; vl < mvp.m_number_subvolumes; vl++)
        {
            float sub_x_min, sub_y_min, sub_z_min;
            float sub_x_max, sub_y_max, sub_z_max;
            sub_x_min = pfi.min_object_coords[0][vl][0];
            sub_y_min = pfi.min_object_coords[0][vl][1];
            sub_z_min = pfi.min_object_coords[0][vl][2];
            sub_x_max = pfi.max_object_coords[0][vl][0];
            sub_y_max = pfi.max_object_coords[0][vl][1];
            sub_z_max = pfi.max_object_coords[0][vl][2];
            mvp.m_min_subvolume_coord[vl].set(sub_x_min, sub_y_min, sub_z_min);
            mvp.m_max_subvolume_coord[vl].set(sub_x_max, sub_y_max, sub_z_max);
        }

        // MultiVolumePropertyListクラスの情報を更新する        
        if (mvpl.m_list.empty())
        {
            mvpl.m_total_number_nodes = mvp.m_number_nodes;
            mvpl.m_total_number_elements = mvp.m_number_elements;
            mvpl.m_total_number_files = mvp.m_number_files;
            mvpl.m_total_start_steps = mvp.m_start_step;
            mvpl.m_total_last_step = mvp.m_end_steps;
            mvpl.m_total_number_steps = mvp.m_number_steps;
            mvpl.m_total_number_subvolumes = mvp.m_number_subvolumes;
            mvpl.m_total_min_object_coord = mvp.m_min_object_coord;
            mvpl.m_total_max_object_coord = mvp.m_max_object_coord;
            mvpl.m_total_min_subvolume_coord = mvp.m_min_subvolume_coord;
            mvpl.m_total_max_subvolume_coord = mvp.m_max_subvolume_coord;
            mvpl.m_total_min_value = mvp.m_min_value;
            mvpl.m_total_max_value = mvp.m_max_value;
            mvpl.m_total_number_ingredients = mvp.m_number_ingredients;
        }
        else
        {
            mvpl.m_total_number_nodes += mvp.m_number_nodes;
            mvpl.m_total_number_elements += mvp.m_number_elements;
            mvpl.m_total_number_files += mvp.m_number_files;
            mvpl.m_total_start_steps = std::min(mvpl.m_total_start_steps, mvp.m_start_step);
            mvpl.m_total_last_step = std::max(mvpl.m_total_last_step, mvp.m_end_steps);
            mvpl.m_total_number_steps = mvpl.m_total_last_step - mvpl.m_total_start_steps + 1;
            mvpl.m_total_number_subvolumes += mvp.m_number_subvolumes;
            mvpl.m_total_min_object_coord[0] = std::min(mvpl.m_total_min_object_coord[0], mvp.m_min_object_coord[0]);
            mvpl.m_total_min_object_coord[1] = std::min(mvpl.m_total_min_object_coord[1], mvp.m_min_object_coord[1]);
            mvpl.m_total_min_object_coord[2] = std::min(mvpl.m_total_min_object_coord[2], mvp.m_min_object_coord[2]);
            mvpl.m_total_max_object_coord[0] = std::max(mvpl.m_total_max_object_coord[0], mvp.m_max_object_coord[0]);
            mvpl.m_total_max_object_coord[1] = std::max(mvpl.m_total_max_object_coord[1], mvp.m_max_object_coord[1]);
            mvpl.m_total_max_object_coord[2] = std::max(mvpl.m_total_max_object_coord[2], mvp.m_max_object_coord[2]);
            std::copy(mvp.m_min_subvolume_coord.begin(), mvp.m_min_subvolume_coord.end(), std::back_inserter(mvpl.m_total_min_subvolume_coord));
            std::copy(mvp.m_max_subvolume_coord.begin(), mvp.m_max_subvolume_coord.end(), std::back_inserter(mvpl.m_total_max_subvolume_coord));
            mvpl.m_total_min_value = std::min(mvpl.m_total_min_value, mvp.m_min_value);
            mvpl.m_total_max_value = std::max(mvpl.m_total_max_value, mvp.m_max_value);
            mvpl.m_total_number_ingredients = std::max(mvpl.m_total_number_ingredients, mvp.m_number_ingredients);
        }

        // MultiVolumePropertyListクラスにMultiVolumePropertyクラスを格納する
        mvpl.m_list.push_back(fi);
    }

    mvpl.m_total_ingredient.resize(mvpl.m_total_number_ingredients);

    return fil;
}