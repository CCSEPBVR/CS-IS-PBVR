#ifndef KVS_WRAPPER_COMMON_H_INCLUDED
#define KVS_WRAPPER_COMMON_H_INCLUDED

#include <vismodule/Argument>
#include <vismodule/NameListFile>
#include <vismodule/CellByCellParticleGenerator>
#include <vismodule/PointObject>
#include <vismodule/KVSMLObjectGlyph>
#include <vismodule/MultiVolumeProperty>
#include <vismodule/ParticleProperty>
#include <vismodule/GlyphProperty>
#include <vismodule/PlotOverLineProperty>
#include <vismodule/PlotOverTimeProperty>

// add FJ start
#ifndef SIMD_BLK_SIZE
#define SIMD_BLK_SIZE 128
#endif
// add FJ  end

typedef unsigned char Byte;

// Asynchronous io, using worker thread pwt.
#include "particle_write_thread.h"
extern bool async_io_enabled;
extern pbvr::ParticleWriteThread pwt;

void begin_wrapper_async_io();

void end_wrapper_async_io();

std::string FileNameOnly( const std::string& file_path );

std::string EnvValueOrUnset( const char* name );

void SetDefaultParticleParameter(
    ParticleProperty& particle_property,
    MultiVolumePropertyList& mvpl,
    const int nvariables
);

size_t CalculateSubpixelLevel(
    const int particle_limit,
    const vismodule::Camera& camera,
    const float sampling_step,
    const double total_volume,
    const vismodule::ObjectBase* volume
);

void SetParameterFilePath(
    const int time_step,
    std::string& historyFilePath,
    std::string& stateFilePath,
    std::string& coordMinMaxFilePath,
    std::string& particleFilePrefix,
    std::string& glyphFilePrefix,
    std::string& plotOverLineFilePrefix,
    std::string& plotOverTimeFilePrefix,
    std::string& tfFilePath,
    std::string& tfFilePath_old,
    std::string& tfFilePath_step,
    std::string& glyphParameterPath,
    std::string& glyphParameterPath_old,
    std::string& plotOverLineParameterPath,
    std::string& plotOverLineParameterPath_old,
    std::string& plotOverTimeParameterPath,
    std::string& plotOverTimeParameterPath_old
);

bool SetGlyphParameter(
    const std::string& glyphParameterPath,
    const std::string& glyphParameterPath_old,
    GlyphProperty& glyph_property,
    NameListFile& nameListFile
);

bool SetPlotOverLineParameter(
    const std::string& plotOverLineParameterPath,
    const std::string& plotOverLineParameterPath_old,
    PlotOverLineProperty& pol_property,
    NameListFile& nameListFile
);

bool SetPlotOverTimeParameter(
    const std::string& plotOverTimeParameterPath,
    const std::string& plotOverTimeParameterPath_old,
    PlotOverTimeProperty& pot_property,
    NameListFile& nameListFile
);

void MakeParticle(
    const vismodule::PointObject* point_object,
    std::vector<float>& coords,
    std::vector<Byte>&  colors,
    std::vector<float>& normals
);

void MakeGlyph(
    const vismodule::KVSMLObjectGlyph* glyph_object,
    std::vector<float>& coords,
    std::vector<float>& vectors,
    std::vector<float>& sizes,
    std::vector<unsigned char>& colors
);

void OutputParticleFiles(
    ParticleProperty& particle_property,
    const MultiVolumePropertyList& mvpl,
    const int time_step,
    const std::string& particleFilePrefix,
    const std::vector<float>& coords,
    const std::vector<Byte>& colors,
    const std::vector<float>& normals
);

void OutputParticleHistory(
    ParticleProperty& particle_property,
    const int tf_number,
    const int nvariables,
    const std::string& histryFilePath,
    const bool update_parameter_file,
    const vismodule::UInt64* c_bins,
    const vismodule::UInt64* o_bins,
    const float* max_array,
    const float* min_array
);

void OutputParticles(
    ParticleProperty& particle_property,
    const MultiVolumePropertyList& mvpl,
    const int start_time_step,
    const int time_step,
    const int tf_number,
    const int nvariables,
    const std::string& particleFilePrefix,
    const std::string& stateFilePath,
    const std::string& histryFilePath,
    const std::vector<float>& coords,
    const std::vector<Byte>& colors,
    const std::vector<float>& normals,
    const vismodule::UInt64* c_bins,
    const vismodule::UInt64* o_bins,
    const float* max_array,
    const float* min_array
);

void OutputGlyphs(
    const int time_step,
    const std::string& glyphFilePrefix,
    const std::vector<float>& coords,
    const std::vector<float>& vectors,
    const std::vector<float>& sizes,
    const std::vector<unsigned char>& colors
);

void OutputLine(
    const int time_step,
    const std::string& plotOverLineFilePrefix,
    const std::vector<float>& values_on_line,
    const std::vector<int>& mask,
    const std::vector<float>& x_axis
);

void OutputPOT(
    const int time_step,
    const std::string& plotOverTimeFilePrefix,
    const bool mask,
    const std::vector<float>& value_on_time
);

#endif // KVS_WRAPPER_COMMON_H_INCLUDED
