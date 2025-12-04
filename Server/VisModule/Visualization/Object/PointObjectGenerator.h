#ifndef VIS_MODULE__POINT_OBJECT_GENERATOR_H_INCLUDE
#define VIS_MODULE__POINT_OBJECT_GENERATOR_H_INCLUDE

#include <memory>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <vismodule/UnstructuredVolumeObject>
#include <vismodule/MultiVolumeProperty>
#include <vismodule/ExtendedTransferFunction>
#include <vismodule/CellByCellParticleGenerator>

#include <kvs/PointObject>

typedef struct
{
    //common
    double importing;

    //master
    double master_initialize;
    double master_set_parameters;
    double master_sampling;
//    double master_importing;
//    double master_writting;
    double master_jobdispatch; 
    double master_mpi_reduce;
    double master_recieving;
    double master_sending;

    //worker
    double worker_initialize;
    double worker_set_parameters;
    double worker_sampling;
//    double worker_writting;
    double worker_mpibcast;
    double worker_mpi_reduce;
    double worker_recieving;
    double worker_sending;
    int nparticles;
} time_parameters;                             

class Argument;
namespace vismodule
{
class Camera;
}

namespace vismodule
{
class PointObject;
class VolumeObjectBase;
class UnstructuredVolumeObject;

class PointObjectGenerator
{
private:
    vismodule::PointObject* m_object;
    const MultiVolumeProperty*   m_mvp;

    vismodule::UnstructuredVolumeObject* m_volume;
    
    vismodule::CoordSynthesizerStrings m_coord_synthesizer_strings;
    vismodule::CoordSynthesizerTokens  m_coord_synthesizer_tokens;

public:

    PointObjectGenerator() : m_object( NULL ), m_mvp(NULL) {};
    ~PointObjectGenerator()
    {
        //delete m_object;
    }

    void createFromFile(
        const Argument& param, const vismodule::Camera& camera, const size_t subpixel_level, const float sampling_step );

    void createFromFile(
        const Argument& param, const vismodule::Camera& camera, const size_t subpixel_level, const float sampling_step, const int st, const int vl );

    vismodule::PointObject* getPointObject()
    {
        return m_object;
    }

    std::string getErrorMessage( const size_t maxMemory ) const;

    void setFilterInfo( const MultiVolumeProperty *mvp )
    {
        m_mvp = mvp;
    }

    void setCoordSynthStr( const std::string& xss, const std::string& yss, const std::string& zss )
    {
        vismodule::CoordSynthesizerStrings css( 0, xss, yss, zss );
        this->setCoordSynthStrs( css );
    }

    void setCoordSynthStrs( const vismodule::CoordSynthesizerStrings& css )
    {
        m_coord_synthesizer_strings = css;
    }

    void setCoordSynthTkns( const vismodule::CoordSynthesizerTokens& cst )
    {
        m_coord_synthesizer_tokens.m_x_coord_synthesizer_token = cst.m_x_coord_synthesizer_token;
        m_coord_synthesizer_tokens.m_y_coord_synthesizer_token = cst.m_y_coord_synthesizer_token;
        m_coord_synthesizer_tokens.m_z_coord_synthesizer_token = cst.m_z_coord_synthesizer_token;
    }

    void setCoordSynthTS( const int ts )
    {
        m_coord_synthesizer_strings.m_time_step = ts;
    }

    vismodule::CoordSynthesizerStrings getCoordSynthStrs() const
    {
        return m_coord_synthesizer_strings;
    }

    vismodule::CoordSynthesizerTokens  getCoordSynthTkns() const
    {
        return m_coord_synthesizer_tokens;
    }

    vismodule::PointObject* run( const Argument& param, const vismodule::Camera& camera, const int st );
    vismodule::PointObject* run( const Argument& param, const vismodule::Camera& camera, const int st, const int vl);

    vismodule::PointObject* GenerateParticleStruct(
        const Argument& param,
        const domain_parameters_struct& dom,
        Type** values,
        const int nvariables
        // const jpv::ServerMode server_mode
    );

    void GenerateParticleUnstruct(
        const Argument& param,
        const domain_parameters_unstruct& dom,
        Type** values,
        const int nvariables,
        float* coordinates,
        const int ncoords,
        unsigned int* connections,
        const int ncells,
        const vismodule::VolumeObjectBase::CellType& celltype,
        std::unique_ptr<kvs::PointObject> point_object
        // const jpv::ServerMode server_mode
    );

private:
    vismodule::PointObject* sampling( const Argument& param, const vismodule::Camera& camera, vismodule::VolumeObjectBase& volume, const size_t subpixel_level, const float sampling_step );

    template <typename T>
    void copy_values(
        vismodule::AnyValueArray& valueArray, std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
        int nvariables,
        int nnodes
    );
};

}

#endif    // VIS_MODULE__POINT_OBJECT_GENERATOR_H_INCLUDE
