#ifndef PBVR__FILTER_INFOMATION_H_INCLDE
#define PBVR__FILTER_INFOMATION_H_INCLDE

#include <string>
#include <vector>
#include "Types.h"
#include <kvs/Vector3>
#include <float.h>

class FilterInformationFile
{
public:
    class IngredientsMinMax
    {
    public:
        float m_min;
        float m_max;
    };
    class IngredientsStep
    {
    public:
        std::vector<IngredientsMinMax> m_ingredient;
    };

public:
    int32_t m_number_nodes;
    int32_t m_number_elements;
    int32_t m_elem_type;
    int32_t m_file_type;
    int32_t m_number_files;
    int32_t m_number_ingredients;
    int32_t m_start_step;
    int32_t m_end_steps;
    int32_t m_number_steps;
    int32_t m_number_subvolumes;
    kvs::Vector3f m_min_object_coord;
    kvs::Vector3f m_max_object_coord;
    std::vector<kvs::Vector3f> m_min_subvolume_coord;
    std::vector<kvs::Vector3f> m_max_subvolume_coord;
    float   m_min_value;
    float   m_max_value;
    std::string m_file_path;
    std::vector<IngredientsStep> m_ingredient_step;

public:
    int loadPFI( const std::string& filename );
};


class FilterInformationList
{
public:
    FilterInformationList();
    ~FilterInformationList();

    int loadPFL( const std::string& filename );
    int getFileIndex( const int vl, int* xvl ) const;
    void cropTimeStep( const int s, const int e );

private:

    std::vector<FilterInformationFile::IngredientsStep> m_total_ingredient_step;

    void calculate_ingredient_min_max( const FilterInformationFile &fi,
                                       std::vector<FilterInformationFile::IngredientsMinMax> *total_ingredient );
public:
    std::vector<FilterInformationFile> m_list;

    int32_t m_total_number_nodes;
    int32_t m_total_number_elements;
    int32_t m_total_number_files;
    int32_t m_total_start_steps;
    int32_t m_total_end_step;
    int32_t m_total_number_steps;
    int32_t m_total_number_subvolumes;
    kvs::Vector3f m_total_min_object_coord;
    kvs::Vector3f m_total_max_object_coord;
    std::vector<kvs::Vector3f> m_total_min_subvolume_coord;
    std::vector<kvs::Vector3f> m_total_max_subvolume_coord;
    float   m_total_min_value;
    float   m_total_max_value;
    int32_t m_total_number_ingredients;
    std::vector<FilterInformationFile::IngredientsMinMax> m_total_ingredient;
};

#endif // PBVR__FILTER_INFO_H_INCLDE

