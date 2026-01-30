enum class TaskSignal : int
{
    EXIT = -1,
    WAITING = 0,
    INITIAL_STEP = 1,
    GENERATE_PARTICLE,
    GENERATE_GLYPH,
    GENERATE_PLOT_OVER_LINE,
    UPDATE_PARTICLE_PROPERTY,
    UPDATE_GLYPH_PROPERTY,
    UPDATE_PLOT_OVER_LINE_PROPERTY,
}

class ServerWorker
{
    void ServerWorker();
    void Run();
}

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