#ifndef VIS_MODULE__FILTER_INFOMATION_H_INCLDE
#define VIS_MODULE__FILTER_INFOMATION_H_INCLDE

#include <string>
#include <vector>
#include "Types.h"
#include <vismodule/Vector3>
#include <float.h>

class MultiVolumeProperty
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
    vismodule::Vector3f m_min_object_coord;
    vismodule::Vector3f m_max_object_coord;
    std::vector<vismodule::Vector3f> m_min_subvolume_coord;
    std::vector<vismodule::Vector3f> m_max_subvolume_coord;
    float   m_min_value;
    float   m_max_value;
    std::string m_file_path;
    bool is_binary;
    std::vector<IngredientsStep> m_ingredient_step;
    
    std::vector<std::string> m_name_list; //add by shimomura 20240726

public:
    int loadPFI( const std::string& filename );
};


class MultiVolumePropertyList
{
public:
    MultiVolumePropertyList();
    ~MultiVolumePropertyList();

    int loadPFL( const std::string& filename );
#ifdef EXTEND_FILE_FORMAT 
    int loadVtm( const std::string& filename );
    int loadSeriesVtm( const std::string& filename );
    int loadVtu( const std::string& filename );
    int loadSeriesVtu( const std::string& filename );
    int loadVti( const std::string& filename );
    int loadSeriesVti( const std::string& filename );
#endif
    int getFileIndex( const int vl, int* xvl ) const;
    void cropTimeStep( const int s, const int e );

private:

    std::vector<MultiVolumeProperty::IngredientsStep> m_total_ingredient_step;

    void calculate_ingredient_min_max( const MultiVolumeProperty &mvp,
                                       std::vector<MultiVolumeProperty::IngredientsMinMax> *total_ingredient );
public:
    std::vector<MultiVolumeProperty> m_list;

    int32_t m_total_number_nodes;
    int32_t m_total_number_elements;
    int32_t m_total_number_files;
    int32_t m_total_start_steps;
    int32_t m_total_last_step;
    int32_t m_total_number_steps;
    int32_t m_total_number_subvolumes;
    vismodule::Vector3f m_total_min_object_coord;
    vismodule::Vector3f m_total_max_object_coord;
    std::vector<vismodule::Vector3f> m_total_min_subvolume_coord;
    std::vector<vismodule::Vector3f> m_total_max_subvolume_coord;
    float   m_total_min_value;
    float   m_total_max_value;
    int32_t m_total_number_ingredients;
    std::vector<MultiVolumeProperty::IngredientsMinMax> m_total_ingredient;
};

#endif // VIS_MODULE__FILTER_INFO_H_INCLDE

