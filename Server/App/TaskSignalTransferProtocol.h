#include "TaskSignal.h"

#include <string>
#include <vector>

#include <vismodule/ParticleProperty>
#include <vismodule/GlyphProperty>
#include <vismodule/PlotOverLineProperty>

void SendInitialStepSignal( const std::string& volume_data_file_path,
                            const std::string& transfer_function_file_path,
                            const std::string& cam_connectivity_file_path = std::string(),
                            const std::vector<std::string>& slac_mode_file_paths = {},
                            const std::vector<std::string>& primary_step_file_paths = {} );
void SendGenerateParticleSignal( const std::string& volume_data_file_path, const int time_step );
void SendGenerateGlyphSignal( const std::string& volume_data_file_path, const int time_step );
void SendGeneratePlorOverLineSignal( const std::string& volume_data_file_path, const int time_step );
void SendGenerateObjectSignal( const TaskSignal task_signal, const std::string& volume_data_file_path, const int time_step );
void SendParticlePropertySignal( const ParticleProperty& particle_property );
void SendGlyphPropertySignal( const GlyphProperty& glyph_property );
void SendPlotOverLinePropertySignal( const PlotOverLineProperty& pol_property );
void ReceiveInitialStepSignal( const char* buf, std::string& volume_data_file_path,
                               std::string& transfer_function_file_path,
                               std::string& cam_connectivity_file_path,
                               std::vector<std::string>& slac_mode_file_paths,
                               std::vector<std::string>& primary_step_file_paths );
void ReceiveGenerateObjectSignal( const char* buf, std::string& volume_data_file_path, int& time_step );
