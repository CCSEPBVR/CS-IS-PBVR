#include <array>

#include <vismodule/GenerateParticle>
#include <vismodule/GeneratePOL>
#include <vismodule/ParticleProperty>
#include <vismodule/PlotOverLineProperty>
#include <vismodule/MultiVolumeProperty>
#include <vismodule/KVSMLObjectPlotOverLine>
#include <vismodule/PlotOverLineGenerator>
#include <vismodule/JobDispatcher>
#include <vismodule/ParticleMonitor>
#include <vismodule/ParameterFileReader>

#ifndef CPU_VER
#include <vismodule/JobCollector>
#endif

void SetDefaultPOLParameterCS( PlotOverLineProperty& pol_property )
{
    pol_property.m_plot_flag      = false;
    pol_property.m_plot_variable  = "q1";
    pol_property.m_start_point[0] = 0;
    pol_property.m_start_point[1] = 0;
    pol_property.m_start_point[2] = 0;
    pol_property.m_end_point[0]   = 1;
    pol_property.m_end_point[1]   = 1;
    pol_property.m_end_point[2]   = 1;
    pol_property.m_sampling_size  = 256;
}

std::unique_ptr<vismodule::KVSMLObjectPlotOverLine> GeneratePOLCS(
    std::string& file_path,
    const int time_step,
    const PlotOverLineProperty& pol_property,
    MultiVolumePropertyList& mvpl
)
{
    int rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    rank = 0;
	mpi_size = 1;
#endif

    int st, vl, wid = 0;
    JobDispatcher jd;
    const int tf_number  = mvpl.m_list[0].m_number_ingredients;
    const int resolution = pol_property.m_sampling_size;
    bool nan_error = false;

#ifndef CPU_VER
    JobCollector jc( &jd );
#endif

    vismodule::ValueArray<float> values_on_line;
    vismodule::ValueArray<float> x_axis;
    vismodule::ValueArray<bool>  mask;

    values_on_line.allocate( resolution );
    x_axis.allocate( resolution );
    mask.allocate( resolution );

    values_on_line.fill( 0x00 );
    x_axis.fill( 0x00 );
    mask.fill( 0x00 );

    jd.initialize( 
        time_step,
        time_step,
        mvpl.m_total_number_subvolumes,
        mvpl.m_total_min_subvolume_coord,
        mvpl.m_total_max_subvolume_coord,
        -1.0,
        1
    );

    while ( jd.dispatchNext( wid, &st, &vl ) )
    {
        vismodule::KVSMLObjectPlotOverLine* tmp_obj = new vismodule::KVSMLObjectPlotOverLine;
        std::vector<float> tmp_values( resolution, 0 );
        std::vector<int>   tmp_mask( resolution, 0 );
        std::vector<float> tmp_axis( resolution, 0 );
    
        // make plot over line
        if ( ( rank > 0 ) || ( mpi_size == 1 ) )
        {
            int xvl, fidx;
            fidx = mvpl.getFileIndex( vl, &xvl );
            MultiVolumeProperty& mvp = mvpl.m_list[fidx];
            mvp.setFilePath( file_path, st, xvl );
        
            // generate plot over line start
            try
            {
                vismodule::VolumeObjectBase* volume = nullptr;
                PlotOverLineGenerator pol_generator;

                if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                {
                }
#ifdef EXTEND_FILE_FORMAT
                else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                {
                    generate_volume( file_path, mvp, st, xvl, volume );
                }
#endif
                else // filetype: kvsml
                {
                    generate_volume( file_path, mvp, volume );
                }

                std::unique_ptr<std::unique_ptr<Type[]>[]> values;
                int nvariables = 0;
                int ncoords = 0;
                vismodule::VolumeObjectBase::VolumeType voltype = volume->volumeType();                    

                if( voltype == vismodule::VolumeObjectBase::VolumeType::Unstructured )
                {
                    domain_parameters_unstruct dom;
                    std::unique_ptr<float[]> coordinates;
                    std::unique_ptr<unsigned int[]> connections;
                    int ncells = 0;
                    vismodule::VolumeObjectBase::CellType celltype;

                    store_volume_in_variables_array_unstruct( volume, dom, values, nvariables, coordinates, ncoords, connections, ncells, celltype );

                    std::vector<Type*> raw_pointers_vector( nvariables );
                    for ( size_t i = 0; i < nvariables; ++i )
                    {
                        raw_pointers_vector[i] = values.get()[i].get();
                    }

                    // generate plot over line
                    pol_generator.GeneratePOLUnstruct(
                        pol_property,
                        raw_pointers_vector.data(),
                        nvariables,
                        coordinates.get(),
                        ncoords,
                        connections.get(),
                        ncells,
                        celltype,
                        tmp_obj
                    );
                }
                else // ( voltype == vismodule::VolumeObjectBase::VolumeType::Structured )
                {
                    domain_parameters_struct dom; 
                    std::array<int, 3> resolution;

                    store_volume_in_variables_array_struct( volume, dom, resolution, values, nvariables, ncoords );

                    std::vector<Type*> raw_pointers_vector( nvariables );
                    for ( size_t i = 0; i < nvariables; ++i )
                    {
                        raw_pointers_vector[i] = values.get()[i].get();
                    }                        

                    // generate plot over line
                    pol_generator.GeneratePOLStruct(
                        pol_property,
                        dom,
                        raw_pointers_vector.data(),
                        nvariables,
                        tmp_obj
                    );
                }

                delete volume;
            }
            catch ( const std::runtime_error& e )
            {
#ifdef _DEBUG // debug by @hira
                printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
                std::cerr << e.what();
                nan_error = true;
            }
            // generate plot over line end

            // store the value of tmp_obj in a variable array for jobCollect.
            for( size_t i = 0; i < resolution; i++ )
            { 
                tmp_axis[i] = tmp_obj->x_axis()[i];
                if ( tmp_obj->mask()[i] )
                {
                    tmp_mask[i]   = 1;
                    tmp_values[i] = tmp_obj->values_on_line()[i];
                }
            }
        } // make plot over line

#ifndef CPU_VER
        if ( mpi_size > 1 ) {
            jc.jobCollect_pol( tmp_axis, tmp_mask, tmp_values, &nan_error, &wid );
        }
#endif

        // aggregate the variable array.
        for( size_t i = 0; i < resolution; i++ )
        { 
            x_axis[i] = tmp_axis[i];
            if ( tmp_mask[i] )
            {
                mask[i]           = 1;
                values_on_line[i] = tmp_values[i];
            }
        }

        delete tmp_obj;
    } // end of while(DispatchNext)

    nan_error = false;

    return std::make_unique<vismodule::KVSMLObjectPlotOverLine>( values_on_line, x_axis, mask );
}

void SetDefaultPOLParameterIS( PlotOverLineProperty& pol_property )
{
    const char *envBuf = NULL;
    std::string visParamDir;
    std::string plotOverLineParameterPath;
    std::string plotOverLineParameterPath_old;
    ParameterFileReader ppr;

    envBuf = std::getenv( "VIS_PARAM_DIR" );

    if (envBuf == NULL) {
        visParamDir = "./";
    }
    else {
        visParamDir = envBuf;
        if (visParamDir[visParamDir.size() - 1] != '/') {
            visParamDir += "/";
        }
    }

    plotOverLineParameterPath     =  visParamDir;
    plotOverLineParameterPath_old =  visParamDir;
    plotOverLineParameterPath     += "parameter.pol";
    plotOverLineParameterPath_old += "parameter_old.pol";

    ppr.readPlotOverLineParameterFile( plotOverLineParameterPath_old.c_str() );
    ppr.setPlotOverLineParameter( pol_property );
}

std::unique_ptr<vismodule::KVSMLObjectPlotOverLine> GeneratePOLIS(
    const int time_step,
    const PlotOverLineProperty& pol_property
)
{
    const int resolution = pol_property.m_sampling_size;

    vismodule::KVSMLObjectPlotOverLine* tmp_obj = new vismodule::KVSMLObjectPlotOverLine;
    vismodule::ValueArray<float> values_on_line;
    vismodule::ValueArray<float> x_axis;
    vismodule::ValueArray<bool>  mask;

    values_on_line.allocate( resolution );
    x_axis.allocate( resolution );
    mask.allocate( resolution );

    values_on_line.fill( 0x00 );
    x_axis.fill( 0x00 );
    mask.fill( 0x00 );

    ParticleMonitor pm;
    pm.check();

    if( pm.stepExisted() )
    {
        pm.setTimeStep_pol( time_step );
    }
    else
    {
        pm.setTimeStep_pol(0);
    }
    
    // get plot over line
    pm.readPlotOverLineFile();
    pm.getPlotOverLine( tmp_obj );

    // make parameter
    for( size_t i = 0; i < resolution; i++ )
    { 
        x_axis[i] = tmp_obj->x_axis()[i];
        if ( tmp_obj->mask()[i] )
        {
            mask[i]           = 1;
            values_on_line[i] = tmp_obj->values_on_line()[i];
        }
    }

    delete tmp_obj;

    return std::make_unique<vismodule::KVSMLObjectPlotOverLine>( values_on_line, x_axis, mask );
}
