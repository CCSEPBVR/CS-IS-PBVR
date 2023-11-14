#ifndef PBVR__JPV__PARTICLE_TRANSFER_PROTOCOL_H_INCLUDE
#define PBVR__JPV__PARTICLE_TRANSFER_PROTOCOL_H_INCLUDE

#include <string>
#include "Types.h"
#include <kvs/Vector3>
#include "ExtendedTransferFunctionParameter.h"
#include "VariableRange.h"

#include <iostream>
#include <sstream>
#include <vector>

namespace kvs
{
class Camera;
class TransferFunction;
}

namespace jpv
{

class ParticleTransferUtils
{
public:
    static bool isLittleEndian();

private:
    static const int32_t m_magic_number;
};

class ParticleTransferClientMessage
{
public:
//    class VolumeEquation
//    {
//    public:
//        std::string m_name;
//        std::string m_equation;
//    };

    struct VolumeEquation
    {
        std::string m_name;
        std::string m_equation;
    };

    //2023/07/19 shimomura
    struct EquationToken
    {
        int exp_token[128];//数式のトークン配列
        int var_name[128];//数式の変数配列
        float value_array[128];//数式の値の配列
    };

public:
    char m_header[11];
    int32_t m_message_size;

    int32_t m_initialize_parameter;
    char m_sampling_method;
    int32_t m_subpixel_level;
    int32_t m_repeat_level;
    char m_shuffle_method;
    char m_node_type;
    float m_sampling_step;
    int32_t m_rendering_id;
    kvs::Camera* m_camera;

    int32_t m_time_parameter;
    int32_t m_begin_time;
    int32_t m_end_time;
    int32_t m_memory_size;
    int32_t m_step;

    int32_t m_trans_Parameter;
    int32_t m_level_index;

    int32_t m_enable_crop_region;
    float m_crop_region[6];
    int32_t m_particle_limit;
    float m_particle_density;
// #ifdef COMM_MODE_IS
     float particle_data_size_limit;
// #endif
    std::string m_input_directory;
    std::string m_filter_parameter_filename;         // add by @hira at 2016/12/01

    std::string m_x_synthesis;
    std::string m_y_synthesis;
    std::string m_z_synthesis;

    std::vector<NamedTransferFunctionParameter> m_transfer_function;
    std::vector<VolumeEquation> m_volume_equation;

    std::string m_transfer_function_synthesis;

    // add by @hira at 2016/12/01
    std::string m_color_transfer_function_synthesis;
    std::string m_opacity_transfer_function_synthesis;

    //2023/07/19 shimomura
    EquationToken opacity_func;//tfs_eq_token;
    EquationToken color_func;//tfs_eq_token;
    std::vector<EquationToken> opacity_var;//opacity_eq_token;
    std::vector<EquationToken> color_var;//color_eq_token;

    EquationToken x_synthesis_token;//x_synthesis;
    EquationToken y_synthesis_token;//y_synthesis;
    EquationToken z_synthesis_token;//z_synthesis;

public:
    // message のサイズを計算
    int32_t byteSize() const;
    // メッセージを byte 列に pack
    size_t pack( char* buf ) const;
    // byte 列からメッセージに unpack
    size_t unpack( const char* buf );

    ParticleTransferClientMessage();
    void show( void ) const;
};

class ParticleTransferServerMessage
{
public:

    struct VolumeEquation
    {
        std::string m_name;
        std::string m_equation;
    };

public:
#ifdef COMM_MODE_IS
    float maxObjectCoord[3];
    float minObjectCoord[3];
#endif

    char m_header[18];
    int32_t m_message_size;
    int32_t m_server_status;   // 2015.12.23 Add Param for NaN
    int32_t m_time_step;
    int32_t m_subpixel_level;
    int32_t m_repeat_level;
    int32_t m_level_index;
    int32_t m_number_particle;
    int32_t m_number_volume_divide;
    int32_t m_start_step;
    int32_t m_end_step;
    int32_t m_number_step;
    float* m_positions;
    float* m_normals;
    unsigned char* m_colors;
    float m_min_object_coord[3];
    float m_max_object_coord[3];
    float m_min_value;
    float m_max_value;
    int32_t m_number_nodes;
    int32_t m_number_elements;
    int32_t m_element_type;
    int32_t m_file_type;
    int32_t m_number_ingredients;
    int32_t m_flag_send_bins;
    int32_t m_transfer_function_count;
//#ifdef COMM_MODE_IS
    int32_t particle_limit;
    float particle_density;
    float particle_data_size_limit;
    kvs::Camera* camera;
    std::vector<NamedTransferFunctionParameter> transfunc;
    std::vector<VolumeEquation> voleqn;
//    std::vector<ParticleTransferClientMessage::VolumeEquation> voleqn;
//    std::string transferFunctionSynthesis;
//#endif
    std::string color_tf_synthesis;
    std::string opacity_tf_synthesis;
    kvs::UInt64* m_color_nbins;
    kvs::UInt64* m_opacity_bins_number;
    std::vector<kvs::UInt64*> m_color_bins;
    std::vector<kvs::UInt64*> m_opacity_bins;
    std::vector<std::string> m_color_bin_names;			// add by @hira at 2016/12/01
    std::vector<std::string> m_opacity_bin_names;		// add by @hira at 2016/12/01
    // message のサイズを計算
    int32_t byteSize() const;
    // メッセージを byte 列に pack
    size_t pack( char* buf ) const;
    // byte 列からメッセージに unpack
    size_t unpack_message( const char* buf );
    size_t unpack_particles( const char* buf );
    size_t unpack_bins( const size_t index, const char* buf );
private:
    float m_transfer_function_min_value;
    float m_transfer_function_max_value;
public:
    VariableRange m_variable_range;

    ParticleTransferServerMessage();

//    // add by @hira at 2016/12/01
//    void setColorHistogramBins(
//            int histogram_size,
//            int nbins,
//            const kvs::UInt64* c_bins,
//            const std::vector<std::string> &transfer_function_names,
//            const std::vector<std::string> &transfunc_synthesizer_names);
//    void setOpacityHistogramBins(
//            int histogram_size,
//            int nbins,
//            const kvs::UInt64* o_bins,
//            const std::vector<std::string> &transfer_function_names,
//            const std::vector<std::string> &transfunc_synthesizer_names);
   void initializeTransferFunction(const int32_t transfer_function_count, const int nbins);
    void show( void ) const;
};

}

#endif //PBVR__JPV__PARTICLE_TRANSFER_PROTOCOL_H_INCLUDE
