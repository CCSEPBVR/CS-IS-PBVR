#ifndef VIS_MODULE__POINT_OBJECT_GENERATOR_H_INCLUDE
#define VIS_MODULE__POINT_OBJECT_GENERATOR_H_INCLUDE

#include <memory>
#include <vector>
#include <string>
#include <sys/stat.h>

#include "ServerMode.h"

#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/MultiVolumeProperty>
#include <vismodule/ExtendedTransferFunction>
#include <vismodule/CellByCellParticleGenerator>
#include <vismodule/ParticleProperty>                         

namespace vismodule
{
class Camera;
class PointObject;
class VolumeObjectBase;

class PointObjectGenerator
{
public:

    PointObjectGenerator() {};
    ~PointObjectGenerator(){};

    vismodule::PointObject* GenerateParticleStruct(
        const ParticleProperty& param,
        const domain_parameters_struct& dom,
        Type** values,
        const int nvariables,
        ServerMode server_mode,
        /*  粒子数の倍率。既定 1.0f で従来と同一の粒子数になる。
         *  アンサンブルメンバーの重ね描きで 1/N を渡す。 */
        const float repetition = 1.0f
    );

    vismodule::PointObject* GenerateParticleUnstruct(
        const ParticleProperty& param,
        const domain_parameters_unstruct& dom,
        Type** values,
        const int nvariables,
        float* coordinates,
        const int ncoords,
        unsigned int* connections,
        const int ncells,
        const vismodule::VolumeObjectBase::CellType& celltype,
        ServerMode server_mode,
        /*  粒子数の倍率。既定 1.0f で従来と同一の粒子数になる。
         *  アンサンブルメンバーの重ね描きで 1/N を渡す。 */
        const float repetition = 1.0f
    );
}; // class ParticleProperty

} // namespace vismodule

#endif    // VIS_MODULE__POINT_OBJECT_GENERATOR_H_INCLUDE
