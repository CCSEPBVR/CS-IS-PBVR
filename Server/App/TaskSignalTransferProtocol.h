#include "TaskSignal.h"

#include <vismodule/ParticleProperty>
#include <vismodule/GlyphProperty>
#include <vismodule/PlotOverLineProperty>

void SendInitialStepSignal( const std::string& volume_data_file_path, const std::string& transfer_function_file_path );
void SendGenerateParticleSignal( const std::string& volume_data_file_path, const int time_step );
void SendGenerateGlyphSignal( const std::string& volume_data_file_path, const int time_step );
void SendGeneratePlorOverLineSignal( const std::string& volume_data_file_path, const int time_step );
void SendGenerateObjectSignal( const TaskSignal task_signal, const std::string& volume_data_file_path, const int time_step );
void SendParticlePropertySignal( const ParticleProperty& particle_property );
void SendGlyphPropertySignal( const GlyphProperty& glyph_property );
void SendPlotOverLinePropertySignal( const PlotOverLineProperty& pol_property );
void ReceiveInitialStepSignal( const char* buf, std::string& volume_data_file_path, std::string& transfer_function_file_path );
void ReceiveGenerateObjectSignal( const char* buf, std::string& volume_data_file_path, int& time_step );
