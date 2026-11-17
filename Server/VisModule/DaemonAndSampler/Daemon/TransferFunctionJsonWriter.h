#ifndef DEFAULT_TF_JSON_H
#define DEFAULT_TF_JSON_H

#include <string>
#include <vismodule/ParticleProperty>
#include "../../../../Shared/json.hpp"

namespace TransferFunctionJsonWriter
{

nlohmann::json ToJson( const ParticleProperty& particle_property );
void WriteTfJson( const ParticleProperty& particle_property, const std::string& json_file_path );
nlohmann::json ParseTfFile( const std::string& tf_file_path );
void WriteTfJson( const std::string& tf_file_path, const std::string& json_file_path );
nlohmann::json LoadTfJson( const std::string& json_file_path );
nlohmann::json LoadParticlePropertyJson( const std::string& json_file_path );

} // namespace default_tf_json

#endif // DEFAULT_TF_JSON_H
