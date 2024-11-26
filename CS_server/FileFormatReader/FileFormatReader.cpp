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

FilterInformationList FileFormatReader::ConvertFilterInformationList( std::string  file_path )
{
    FilterInformationList fil;
    fil.m_list.clear();
    fil.m_total_min_subvolume_coord.clear();
    fil.m_total_max_subvolume_coord.clear();

    std::unordered_map<int, cvt::UnstructuredPfi> pfi_map;
    pfi_map = ConvertUnstructuredPfiMap(file_path);

    // 連想配列に格納しているcvt::UnstructuredPfiクラス毎に処理を実施
    for (auto& e : pfi_map)
    {
        // キーにはセルタイプ, 値にcvt::UnstructuredPfiクラスのインスタンス
        cvt::UnstructuredPfi pfi = e.second;
        FilterInformationFile fi;

        // cvt::UnstructuredPfiクラスの情報をFilterInformationFileクラスに格納する
        int number_of_nodes = std::accumulate(pfi.node_counts[0].begin(), pfi.node_counts[0].end(), 0);
        fi.m_number_nodes = number_of_nodes;

        int number_of_elements = std::accumulate(pfi.cell_counts[0].begin(), pfi.cell_counts[0].end(), 0);
        fi.m_number_elements = number_of_elements;

        fi.m_elem_type = pfi.type_of_cells;

        int type_of_file = 0;
        fi.m_file_type = type_of_file;

        int number_of_file = pfi.max_sub_volume_id * (pfi.last_time_step + 1);
        fi.m_number_files = number_of_file;
    
        fi.m_number_ingredients = pfi.number_of_component;

        int step_of_beginning = 0;
        fi.m_start_step = step_of_beginning;

        int step_of_end = pfi.last_time_step;
        fi.m_end_steps = step_of_end;
        
        int number_of_sub_volumes = pfi.max_sub_volume_id;
        fi.m_number_subvolumes = number_of_sub_volumes;

        float x_min, y_min, z_min;
        float x_max, y_max, z_max;
        x_min = pfi.min_external_coords[0];
        y_min = pfi.min_external_coords[1];
        z_min = pfi.min_external_coords[2];
        x_max = pfi.max_external_coords[0];
        y_max = pfi.max_external_coords[1];
        z_max = pfi.max_external_coords[2];
        fi.m_min_object_coord.set(x_min, y_min, z_min);
        fi.m_max_object_coord.set(x_max, y_max, z_max);

        std::cout << "m_end_steps = " << fi.m_end_steps << std::endl;
        std::cout << "m_start_step = " << fi.m_start_step << std::endl;
        fi.m_number_steps = fi.m_end_steps - fi.m_start_step + 1;

        fi.m_min_subvolume_coord.resize(fi.m_number_subvolumes);
        fi.m_max_subvolume_coord.resize(fi.m_number_subvolumes);

        fi.m_file_path = file_path;

        for (int vl = 0; vl < fi.m_number_subvolumes; vl++)
        {
            float sub_x_min, sub_y_min, sub_z_min;
            float sub_x_max, sub_y_max, sub_z_max;
            sub_x_min = pfi.min_object_coords[0][vl][0];
            sub_y_min = pfi.min_object_coords[0][vl][1];
            sub_z_min = pfi.min_object_coords[0][vl][2];
            sub_x_max = pfi.max_object_coords[0][vl][0];
            sub_y_max = pfi.max_object_coords[0][vl][1];
            sub_z_max = pfi.max_object_coords[0][vl][2];
            fi.m_min_subvolume_coord[vl].set(sub_x_min, sub_y_min, sub_z_min);
            fi.m_max_subvolume_coord[vl].set(sub_x_max, sub_y_max, sub_z_max);
        }

        // FilterInformationListクラスの情報を更新する        
        if (fil.m_list.empty())
        {
            fil.m_total_number_nodes = fi.m_number_nodes;
            fil.m_total_number_elements = fi.m_number_elements;
            fil.m_total_number_files = fi.m_number_files;
            fil.m_total_start_steps = fi.m_start_step;
            fil.m_total_last_step = fi.m_end_steps;
            fil.m_total_number_steps = fi.m_number_steps;
            fil.m_total_number_subvolumes = fi.m_number_subvolumes;
            fil.m_total_min_object_coord = fi.m_min_object_coord;
            fil.m_total_max_object_coord = fi.m_max_object_coord;
            fil.m_total_min_subvolume_coord = fi.m_min_subvolume_coord;
            fil.m_total_max_subvolume_coord = fi.m_max_subvolume_coord;
            fil.m_total_min_value = fi.m_min_value;
            fil.m_total_max_value = fi.m_max_value;
            fil.m_total_number_ingredients = fi.m_number_ingredients;
        }
        else
        {
            fil.m_total_number_nodes += fi.m_number_nodes;
            fil.m_total_number_elements += fi.m_number_elements;
            fil.m_total_number_files += fi.m_number_files;
            fil.m_total_start_steps = std::min(fil.m_total_start_steps, fi.m_start_step);
            fil.m_total_last_step = std::max(fil.m_total_last_step, fi.m_end_steps);
            fil.m_total_number_steps = fil.m_total_last_step - fil.m_total_start_steps + 1;
            fil.m_total_number_subvolumes += fi.m_number_subvolumes;
            fil.m_total_min_object_coord[0] = std::min(fil.m_total_min_object_coord[0], fi.m_min_object_coord[0]);
            fil.m_total_min_object_coord[1] = std::min(fil.m_total_min_object_coord[1], fi.m_min_object_coord[1]);
            fil.m_total_min_object_coord[2] = std::min(fil.m_total_min_object_coord[2], fi.m_min_object_coord[2]);
            fil.m_total_max_object_coord[0] = std::max(fil.m_total_max_object_coord[0], fi.m_max_object_coord[0]);
            fil.m_total_max_object_coord[1] = std::max(fil.m_total_max_object_coord[1], fi.m_max_object_coord[1]);
            fil.m_total_max_object_coord[2] = std::max(fil.m_total_max_object_coord[2], fi.m_max_object_coord[2]);
            std::copy(fi.m_min_subvolume_coord.begin(), fi.m_min_subvolume_coord.end(), std::back_inserter(fil.m_total_min_subvolume_coord));
            std::copy(fi.m_max_subvolume_coord.begin(), fi.m_max_subvolume_coord.end(), std::back_inserter(fil.m_total_max_subvolume_coord));
            fil.m_total_min_value = std::min(fil.m_total_min_value, fi.m_min_value);
            fil.m_total_max_value = std::max(fil.m_total_max_value, fi.m_max_value);
            fil.m_total_number_ingredients = std::max(fil.m_total_number_ingredients, fi.m_number_ingredients);
        }

        // FilterInformationListクラスにFilterInformationFileクラスを格納する
        fil.m_list.push_back(fi);
    }

    fil.m_total_ingredient.resize(fil.m_total_number_ingredients);

    return fil;
}