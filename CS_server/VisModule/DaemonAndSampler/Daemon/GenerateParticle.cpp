#include <vismodule/GenerateParticle>

void generate_particle(
    Argument &param,
    MultiVolumePropertyList& mvpl,
    bool &nan_error, 
#ifndef CPU_VER
    JobCollector& jc, 
#endif
    JobDispatcher& jd,
    jpv::ParticleTransferServer pts,
    jpv::ServerMode server_mode,
    jpv::InitializeParameter init_param
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
    jpv::ParticleTransferServerMessage servMes;
    int tf_number;
    VariableRange vr; // jobcollectorで使用、削除予定
    ParticleMonitor pm;

    if ( server_mode == jpv::ServerMode::CS )
    {
        tf_number = param.m_transfunc_array.size();
    } // server_mode == jpv::ServerMode::CS
    else // server_mode == jpv::ServerMode::IS
    {
        pm.check();

        if( pm.stepExisted() )
        {
            pm.setTimeStep_particle( pm.particleStatusFile().getLatestTimeStep() );
        }
        else
        {
            pm.setTimeStep_particle(0);
        }
        pm.readParticleHistoryFile();

        tf_number = pm.particleHistoryFile().colorHistogramArray().size();
    } // server_mode == jpv::ServerMode::IS

    std::cout << "param.m_time_step:" << param.m_time_step << std::endl;

    jd.initialize(
        param.m_time_step,
        param.m_time_step,
        mvpl.m_total_number_subvolumes,
        mvpl.m_total_min_subvolume_coord,
        mvpl.m_total_max_subvolume_coord,
        param.m_latency_threshold,
        param.m_job_id_pack_size
    );

    if ( rank == 0 )
    {
        SetServerMessageParameter( param, mvpl, servMes );

        if ( init_param == jpv::InitializeParameter::generate_particle )
        {
            // send sub volume num server message
            std::cout << "INFO: send sub volume num server message" << std::endl;
            servMes.m_message_size = servMes.byteSize();
            servMes.show();
            pts.sendMessage( servMes );
        }  
    }

    vismodule::UInt64* tmp_c_bins;
    vismodule::UInt64* tmp_o_bins;
    float* tmp_max;
    float* tmp_min;

    tmp_c_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
    tmp_o_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
    tmp_max = new float[tf_number * 2]; // color, opacity
    tmp_min = new float[tf_number * 2]; // color, opacity

    for ( size_t i = 0; i < (DEFAULT_NBINS * tf_number); i++ )
    {
        tmp_c_bins[i] = 0;
        tmp_o_bins[i] = 0;
    }

    for ( int i = 0; i < (tf_number * 2); i++ )
    {
        tmp_max[i] = FLT_MIN;
        tmp_min[i] = FLT_MAX;
    }

    while ( jd.dispatchNext( wid, &st, &vl ) )
    {
        vismodule::PointObject* originalObject = new vismodule::PointObject;

        if ( server_mode == jpv::ServerMode::CS )
        {
            vismodule::PointObject* tmp_obj = nullptr;

            // make point object and histgram and range
            if ( ( rank > 0 ) || ( mpi_size == 1 ) )
            {
                int xvl, fidx;
                fidx = mvpl.getFileIndex( vl, &xvl );
                MultiVolumeProperty& mvp = mvpl.m_list[fidx];
                mvp.setFilePath( param.m_input_data, st, xvl );
                param.m_subvolume_id = xvl;
                // generate point object start
                try
                {
                    vismodule::VolumeObjectBase* volume = nullptr;
                    vismodule::CS_PointObjectGenerator point_object_generator;

                    point_object_generator.setCoordSynthStr(
                        param.m_x_synthesis,
                        param.m_y_synthesis,
                        param.m_z_synthesis
                    );

                    // generate volume object
                    if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                    {
                        generate_volume( param, mvp, st, xvl, volume );
                    }
                    else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                    {
                        generate_volume( param, mvp, st, xvl, volume );
                    }
                    else // filetype: kvsml
                    {
                        generate_volume( param, mvp, st, volume );
                    }

                    if ( !volume )
                    {
                        throw std::runtime_error("Failed to generate volume object.");
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

                        float max_opacity;
                        float max_density;
                        float sampling_volume_inverse;
                        
                        vismodule::CellByCellParticleGenerator::CalculateDensityConstaint(
                            *param.m_camera,
                            *volume,
                            static_cast<float>( param.m_subpixel_level ),
                            param.m_sampling_step,
                            &sampling_volume_inverse,
                            &max_opacity,
                            &max_density
                        );

                        param.m_transfunc_synthesizer->setMaxOpacity( max_opacity );
                        param.m_transfunc_synthesizer->setMaxDensity( max_density );
                        param.m_transfunc_synthesizer->setSamplingVolumeInverse( sampling_volume_inverse );

                        std::vector<Type*> raw_pointers_vector( nvariables );
                        for ( size_t i = 0; i < nvariables; ++i )
                        {
                            raw_pointers_vector[i] = values.get()[i].get();
                        }

                        // generate particle
                        tmp_obj = point_object_generator.GenerateParticleUnstruct( param, dom, raw_pointers_vector.data(), nvariables, coordinates.get(), ncoords, connections.get(), ncells, celltype, server_mode );
                    }
                    else // ( voltype == vismodule::VolumeObjectBase::VolumeType::Structured )
                    {
                        domain_parameters_struct dom; 

                        store_volume_in_variables_array_struct( volume, dom, values, nvariables, ncoords );

                        std::vector<Type*> raw_pointers_vector( nvariables );
                        for ( size_t i = 0; i < nvariables; ++i )
                        {
                            raw_pointers_vector[i] = values.get()[i].get();
                        }                        

                        // generate particle
                        tmp_obj = point_object_generator.GenerateParticleStruct( param, dom, raw_pointers_vector.data(), nvariables, server_mode );
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
                // generate point object end

                MakeHistgram( tmp_obj, tf_number, tmp_c_bins, tmp_o_bins );

                MakeParticleMinMax( param.m_transfunc_synthesizer, tf_number, tmp_max, tmp_min );
            } // make point object and histgram and range

#ifndef CPU_VER
            if ( mpi_size > 1 ) {
                if ( rank == 0 )
                {
                    jc.jobCollect( originalObject, &vr, &nan_error, &wid );
                }
                else
                {
                    jc.jobCollect( tmp_obj, &vr, &nan_error, &wid );
                }
            }
#else
            if ( init_param == jpv::InitializeParameter::generate_particle )
            {
                size_t nmemb = tmp_obj->nvertices() * 3;
                vismodule::ValueArray<vismodule::Real32> coords_array ( tmp_obj->coords().pointer() , nmemb );
                vismodule::ValueArray<vismodule::UInt8>  colors_array ( tmp_obj->colors().pointer() , nmemb );
                vismodule::ValueArray<vismodule::Real32> normals_array( tmp_obj->normals().pointer(), nmemb );
                
                originalObject->clear();
                originalObject->setCoords( coords_array );
                originalObject->setColors( colors_array );
                originalObject->setNormals( normals_array );
            }
#endif

            delete tmp_obj;
        } // server_mode == jpv::ServerMode::CS
        else // server_mode == jpv::ServerMode::IS
        {
            // get point object
            if ( init_param == jpv::InitializeParameter::generate_particle )
            {
                pm.readParticleFile();
                pm.getParticle( originalObject );
            }

            // get histgram start
            int c_count = 0;
            for ( int tf = 0; tf < tf_number; tf++ )
            {
                for ( int res = 0; res < DEFAULT_NBINS; res++ )
                {
                    tmp_c_bins[c_count] = pm.particleHistoryFile().colorHistogramArray()[tf][res];
                    c_count++;
                }
            }

            int o_count = 0;
            for ( int tf = 0; tf < tf_number; tf++ )
            {
                for ( int res = 0; res < DEFAULT_NBINS; res++ )
                {
                    tmp_o_bins[o_count] = pm.particleHistoryFile().opacityHistogramArray()[tf][res];
                    o_count++;
                }
            }
            // get histgram end
        } // server_mode == jpv::ServerMode::IS

        // send particle
        if ( rank == 0 && init_param == jpv::InitializeParameter::generate_particle )
        {
            // make particle server message start
            servMes.m_flag_send_bins = 0;
            // vismodule::PointObject* object = originalObject;
            // if ( originalObject != object ) delete originalObject;
            servMes.m_number_particle = originalObject->coords().size() / 3;
            printf( " %zu perticles generated\n", servMes.m_number_particle / 3 );

            if ( servMes.m_number_particle > 0 )
            {
                servMes.m_positions = std::make_unique<float[]>(3 * servMes.m_number_particle);
                servMes.m_normals   = std::make_unique<float[]>(3 * servMes.m_number_particle);
                servMes.m_colors    = std::make_unique<unsigned char[]>(3 * servMes.m_number_particle);
            }
            else
            {
                servMes.m_positions = NULL;
                servMes.m_normals   = NULL;
                servMes.m_colors    = NULL;
            }
            for ( int i = 0; i < servMes.m_number_particle; ++i )
            {
                servMes.m_positions[3 * i + 0] = originalObject->coords()[3 * i + 0];
                servMes.m_positions[3 * i + 1] = originalObject->coords()[3 * i + 1];
                servMes.m_positions[3 * i + 2] = originalObject->coords()[3 * i + 2];
                servMes.m_normals[3 * i + 0]   = originalObject->normals()[3 * i + 0];
                servMes.m_normals[3 * i + 1]   = originalObject->normals()[3 * i + 1];
                servMes.m_normals[3 * i + 2]   = originalObject->normals()[3 * i + 2];
                servMes.m_colors[3 * i + 0]    = originalObject->colors()[3 * i + 0];
                servMes.m_colors[3 * i + 1]    = originalObject->colors()[3 * i + 1];
                servMes.m_colors[3 * i + 2]    = originalObject->colors()[3 * i + 2];
            }
            // make particle server message end

            // send particle server message
            std::cout << "INFO: send particle server message" << std::endl;
            servMes.m_message_size = servMes.byteSize();
            servMes.show();
            pts.sendMessage( servMes );
        } // send particle

        delete originalObject;
    } // end of while(DispatchNext)

#ifndef CPU_VER
    if ( mpi_size > 1 ) {
        MPI_Allreduce( MPI_IN_PLACE, tmp_c_bins, c_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_o_bins, o_bins_size, MPI_UNSIGNED_LONG, MPI_SUM , MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_max, ( tf_number * 2 ), MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_min, ( tf_number * 2 ), MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
    }
#endif

    if ( server_mode == jpv::ServerMode::CS )
    {
        vr = setVariablerange2( tmp_max, tmp_min, ( tf_number * 2 ) );
    }
    else // server_mode == jpv::ServerMode::IS
    {
        vr = pm.particleHistoryFile().variableRange();
    }

    // send histgram and range
    if ( rank == 0 )
    {
        // make histgram server message start
        servMes.m_number_particle = 0;
        servMes.m_flag_send_bins = 1;
        servMes.m_transfer_function_count = tf_number;
        servMes.m_server_side_variable_range = vr;
        setParamTransferFunctionToServer( &servMes, &param );

        servMes.m_color_nbins   = new vismodule::UInt64[tf_number];
        servMes.m_opacity_nbins = new vismodule::UInt64[tf_number];
        servMes.m_color_bins.resize( tf_number );
        servMes.m_opacity_bins.resize( tf_number );

        for ( int tf = 0; tf < tf_number; tf++ )
        {
            servMes.m_color_nbins[tf] = DEFAULT_NBINS;
            servMes.m_opacity_nbins[tf] = DEFAULT_NBINS;
            servMes.m_color_bins[tf]   =  new vismodule::UInt64[ DEFAULT_NBINS ];
            servMes.m_opacity_bins[tf] =  new vismodule::UInt64[ DEFAULT_NBINS ];
            for ( int res = 0; res < DEFAULT_NBINS; res++ )
            {
                servMes.m_color_bins[tf][res] = 0;
            }
            for ( int res = 0; res < DEFAULT_NBINS; res++ )
            {
                servMes.m_opacity_bins[tf][res] = 0;
            }
        }

        servMes.setColorHistogramBins(
            param.m_transfunc_array.size(),
            DEFAULT_NBINS,
            tmp_c_bins
        );

        servMes.setOpacityHistogramBins(
            param.m_transfunc_array.size(),
            DEFAULT_NBINS,
            tmp_o_bins
        );
        // make histgram and range server message end

        // TEST START 2015.1.14
        if ( nan_error )
        {
            strncpy( servMes.m_header, "JPTP /1.0 899 OK\r\n", 18 );
            servMes.m_server_status = 1;
            servMes.m_number_particle = 0;
            servMes.m_number_glyph = 0 ;
            servMes.m_flag_send_bins = 1;
            std::cout << "!!!!!!!!!!!! Send serverStatus = 1 " << std::endl;
        }

        // send histgram server message
        std::cout << "INFO: send histgram and range server message" << std::endl;
        servMes.m_message_size = servMes.byteSize();
        servMes.show();
        pts.sendMessage( servMes );

        // TEST START 2015.1.14
        servMes.m_server_status = 0;
        // TEST END 2015.1.14

        for ( int tf = 0; tf < tf_number; tf++ )
        {
            delete[] servMes.m_color_bins[tf];
            delete[] servMes.m_opacity_bins[tf];
        }

        delete[] servMes.m_color_nbins;
        delete[] servMes.m_opacity_nbins;
    } // send histgram and range

    nan_error = false;

    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
}

void generate_volume(
    const Argument& param,
    const MultiVolumeProperty& mvp,
    const int time_step,
    vismodule::VolumeObjectBase*& volume
)
{
    struct stat s;
    if ( stat( param.m_input_data.c_str(), &s ) )
    {
        std::cout << "Error. read failed:" << param.m_input_data << std::endl;
        exit( 1 );
    }

    if ( vismoduleview::FileChecker::ImportableStructuredVolume( param.m_input_data ))
    {
        std::cout << "Structured !" <<std::endl;
        volume = new vismodule::StructuredVolumeImporter( param.m_input_data ); 
        int id = param.m_subvolume_id;
        volume->updateMinMaxValues();
        volume->setMinMaxObjectCoords( mvp.m_min_subvolume_coord[id], mvp.m_max_subvolume_coord[id] );
        volume->setMinMaxExternalCoords( mvp.m_min_subvolume_coord[id], mvp.m_max_subvolume_coord[id] );

    } 
    else if ( vismoduleview::FileChecker::ImportableUnstructuredVolume( param.m_input_data))
    {
        std::cout << "Unstructured !" <<std::endl;
        volume = new vismodule::UnstructuredVolumeImporter( param.m_input_data );  
        volume->updateMinMaxValues();
        volume->setMinMaxObjectCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
        volume->setMinMaxExternalCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
    }
    else 
    {
        visModuleMessageError("%s is not volume data.", param.m_input_data.c_str());
    }

    std::cout << *volume << std::endl;
    std::cout << "min:" << volume->minObjectCoord()   << ", max:" << volume->maxObjectCoord() << std::endl;
    std::cout << "min:" << volume->minExternalCoord() << ", max:" << volume->maxExternalCoord() << std::endl;

    return;
}

void generate_volume(
    const Argument &param,
    const MultiVolumeProperty& mvp,
    const int time_step,
    const int sub_volume_id,
    vismodule::VolumeObjectBase*& volume
)
{
    size_t found_kvsml = param.m_input_data_base.find(".kvsml");
    size_t found_vtm   = param.m_input_data_base.find(".vtm");
    size_t found_vtu   = param.m_input_data_base.find(".vtu");
    size_t found_vti   = param.m_input_data_base.find(".vti");
    size_t found_inp   = param.m_input_data_base.find(".inp");
    size_t found_pvtu  = param.m_input_data_base.find(".pvtu");
    size_t found_case  = param.m_input_data_base.find(".case");

    if ( found_kvsml != std::string::npos )
    {
        volume = new vismodule::UnstructuredVolumeImporter( param.m_input_data );
    
        vismodule::File ifpx( mvp.m_file_path );
        std::string path_base = ifpx.pathName() + ifpx.Separator() + ifpx.baseName();
    
        volume = new vismodule::UnstructuredVolumeImporter( path_base, mvp.m_file_type, time_step, sub_volume_id );
    }
#ifdef EXTEND_FILE_FORMAT
    else if ( found_vtm != std::string::npos )
    {
        // structured
        if( mvp.m_file_type == 3 )
        {
            volume = new vismodule::StructuredVolumeImporter( mvp.m_file_path, time_step, sub_volume_id );
        }
        // unstructured
        if( mvp.m_file_type == 4 )
        {
            volume = new vismodule::UnstructuredVolumeImporter( mvp.m_file_path, mvp.m_file_type, mvp.m_elem_type, time_step, sub_volume_id );
        }
    }
    else if ( found_vtu  != std::string::npos ||
              found_inp  != std::string::npos ||
              found_pvtu != std::string::npos ||
              found_case != std::string::npos
            )
    {
        volume = new vismodule::UnstructuredVolumeImporter( mvp.m_file_path, mvp.m_file_type, mvp.m_elem_type, time_step, sub_volume_id );
    }
    else if ( found_vti != std::string::npos )
    {
        volume = new vismodule::StructuredVolumeImporter( mvp.m_file_path, time_step, sub_volume_id );
    }
#endif

    // CS only
    if ( volume )
    {
        std::string xss = param.m_x_synthesis;
        std::string yss = param.m_y_synthesis;
        std::string zss = param.m_z_synthesis;
        vismodule::CoordSynthesizerStrings css( 0, xss, yss, zss );
        volume->setCoordSynthesizerStrings( css );
    }

    // .vtm .pvtu .case file format
    if ( ( found_vtm != std::string::npos ) || ( found_pvtu != std::string::npos ) || ( found_case != std::string::npos ) )
    {
        // Structured Volume Data
        if ( mvp.m_file_type == 3 )
        {
            volume->updateMinMaxValues();
            volume->setMinMaxObjectCoords( mvp.m_min_subvolume_coord[sub_volume_id], mvp.m_max_subvolume_coord[sub_volume_id] );
            volume->setMinMaxExternalCoords( mvp.m_min_subvolume_coord[sub_volume_id], mvp.m_max_subvolume_coord[sub_volume_id] );
        }

        // Unstructured Volume Data
        else if ( mvp.m_file_type == 4 )
        {
            volume->updateMinMaxValues();
            volume->setMinMaxObjectCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
            volume->setMinMaxExternalCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
        }
    }
    else
    {
        volume->setMinMaxValues( mvp.m_min_value, mvp.m_max_value );
        volume->setMinMaxObjectCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
        volume->setMinMaxExternalCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
    }

    std::cout << *volume << std::endl;
    std::cout << "min:" << volume->minObjectCoord()   << ", max:" << volume->maxObjectCoord() << std::endl;
    std::cout << "min:" << volume->minExternalCoord() << ", max:" << volume->maxExternalCoord() << std::endl;    
}

void store_volume_in_variables_array_common(
    const vismodule::VolumeObjectBase* volume,
    std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
    int& nvariables,
    int& ncoords
)
{
    //詰め替え処理
    vismodule::AnyValueArray valueArray;
    valueArray = volume->values(); 
    ncoords    = volume->nnodes();
    nvariables = volume->veclen();

    // ここで変数の値をfloatでまとめることで粒子生成のテンプレート化を回避
    // std::unique_ptr<std::unique_ptr<Type[]>[]> tmp_values(new std::unique_ptr<Type[]>[nvariables]);
    values = std::make_unique<std::unique_ptr<Type[]>[]>( nvariables );

    // 実行時型分岐で呼び出す
    const std::type_info& type = volume->values().typeInfo()->type();
    if (type == typeid( vismodule::Int8 ) )
    {
        copy_values<vismodule::Int8>( valueArray, values, nvariables, ncoords );
    }  
    else if ( type == typeid( vismodule::Int16 ) )
    {
        copy_values<vismodule::Int16>( valueArray, values, nvariables, ncoords );
    } 
    else if ( type == typeid( vismodule::Int32 ) )
    {
        copy_values<vismodule::Int32>( valueArray, values, nvariables, ncoords );
    }
    else if ( type == typeid( vismodule::Int64 ) )
    {
        copy_values<vismodule::Int64>( valueArray, values, nvariables, ncoords );
    }
    else if ( type == typeid( vismodule::UInt8 ) )
    {
        copy_values<vismodule::UInt8>( valueArray, values, nvariables, ncoords );
    }
    else if ( type == typeid( vismodule::UInt16 ) )
    {
        copy_values<vismodule::UInt16>( valueArray, values, nvariables, ncoords );
    }
    else if ( type == typeid( vismodule::UInt32 ) )
    {
        copy_values<vismodule::UInt32>( valueArray, values, nvariables, ncoords );
    }
    else if ( type == typeid( vismodule::UInt64 ) )
    {
        copy_values<vismodule::UInt64>( valueArray, values, nvariables, ncoords );
    }
    else if ( type == typeid( vismodule::Real32 ) )
    {
        copy_values<vismodule::Real32>( valueArray, values, nvariables, ncoords );
    }
    else if ( type == typeid( vismodule::Real64 ) )
    {
        copy_values<vismodule::Real64>( valueArray, values, nvariables, ncoords );
    }
    else 
    {
        throw std::runtime_error( "Unsupported type" );
    }
}

void store_volume_in_variables_array_struct(
    const vismodule::VolumeObjectBase* volume,
    domain_parameters_struct& dom,
    std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
    int& nvariables,
    int& ncoords
)
{
    store_volume_in_variables_array_common( volume, values, nvariables, ncoords );

    const vismodule::StructuredVolumeObject* svo_p = static_cast<const vismodule::StructuredVolumeObject*>( volume );

    int resol[3] = {
        static_cast<int>(svo_p->resolution().x()),
        static_cast<int>(svo_p->resolution().y()),
        static_cast<int>(svo_p->resolution().z())
    };

    dom = {
        volume->minObjectCoord().x(),
        volume->minObjectCoord().y(),
        volume->minObjectCoord().z(),
        volume->maxObjectCoord().x(),
        volume->maxObjectCoord().y(),
        volume->maxObjectCoord().z(),
        resol,
        1.f
    };
}

void store_volume_in_variables_array_unstruct(
    const vismodule::VolumeObjectBase* volume,
    domain_parameters_unstruct& dom,
    std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
    int& nvariables,
    std::unique_ptr<float[]>& coordinates,
    int& ncoords,
    std::unique_ptr<unsigned int[]>& connections,
    int& ncells,
    vismodule::VolumeObjectBase::CellType& celltype
)
{
    store_volume_in_variables_array_common( volume, values, nvariables, ncoords );

    const vismodule::UnstructuredVolumeObject* uvo_p = static_cast<const vismodule::UnstructuredVolumeObject*>( volume );

    std::vector<float> tmp_coordinates;
    tmp_coordinates.assign( (float*)uvo_p->coords().begin(), (float*)uvo_p->coords().end() );
    coordinates = std::make_unique<float[]>( tmp_coordinates.size() );
    std::copy( tmp_coordinates.begin(), tmp_coordinates.end(), coordinates.get() );

    std::vector<unsigned int> tmp_connections;
    tmp_connections.assign( (unsigned int*)uvo_p->connections().begin(), (unsigned int*)uvo_p->connections().end() );
    connections = std::make_unique<unsigned int[]>( tmp_connections.size() );
    std::copy( tmp_connections.begin(), tmp_connections.end(), connections.get() );

    ncells   = uvo_p->ncells();
    celltype = uvo_p->cellType();

    dom = {
        volume->minObjectCoord().x(),
        volume->minObjectCoord().y(),
        volume->minObjectCoord().z(),
        volume->maxObjectCoord().x(),
        volume->maxObjectCoord().y(),
        volume->maxObjectCoord().z()
    };
}

template <typename T>
void copy_values(
    vismodule::AnyValueArray& valueArray,
    std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
    int nvariables,
    int nnodes
)
{
    for (int j = 0; j < nvariables; j++) 
    {
        values[j] = std::make_unique<Type[]>(nnodes);
        for (int i = 0; i < nnodes; i++) 
        {
            int it = j * nnodes + i;
            values[j][i] = valueArray.at<T>(it);
        }
    }
}

void MakeHistgram(
    const vismodule::PointObject* point_object,
    const int tf_number,
    vismodule::UInt64* c_bins,
    vismodule::UInt64* o_bins
)
{
    int c_count = 0;
    for ( int tf = 0; tf < tf_number; tf++ )
    {
        int c_nbins = point_object->getNbins();
        for ( int res = 0; res < c_nbins; res++ )
        {
            c_bins[ c_count ] += point_object->getCHistogram()[ c_count ];
            c_count++;
        }
    }

    int o_count = 0;
    for ( int tf = 0; tf < tf_number; tf++ )
    {
        int o_nbins = point_object->getNbins();
        for ( int res = 0; res < o_nbins; res++ )
        {
            o_bins[ o_count ] += point_object->getOHistogram()[ o_count ];
            o_count++;
        }
    }
}

void MakeParticleMinMax(
    const TransferFunctionSynthesizer* transfer_function_synthesizer,
    const int tf_number,
    float* max_array,
    float* min_array
)
{
    for( int i = 0; i < tf_number; i++ )
    {
        max_array[2 * i + 1] = vismodule::Math::Max( max_array[2 * i + 1], transfer_function_synthesizer->m_c_max[i] );
        min_array[2 * i + 1] = vismodule::Math::Min( min_array[2 * i + 1], transfer_function_synthesizer->m_c_min[i] );
        max_array[2 * i    ] = vismodule::Math::Max( max_array[2 * i    ], transfer_function_synthesizer->m_o_max[i] );
        min_array[2 * i    ] = vismodule::Math::Min( min_array[2 * i    ], transfer_function_synthesizer->m_o_min[i] );
    }
}

#if 0
void generate_particle_worker(
    Argument &param,
    MultiVolumePropertyList& mvpl,
    bool &nan_error,
#ifndef CPU_VER
    JobCollector& jc, 
#endif
    JobDispatcher& jd
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
    std::vector<vismodule::CS_PointObjectGenerator> point_generator_lst;
    VariableRange vr;
    int tf_number;

    tf_number = mvpl.m_list[0].m_number_ingredients;

    point_generator_lst.clear();
    point_generator_lst.resize(mvpl.m_list.size());
                    
    jd.initialize(
        param.m_time_step,
        param.m_time_step,
        mvpl.m_total_number_subvolumes,
        mvpl.m_total_min_subvolume_coord,
        mvpl.m_total_max_subvolume_coord,
        param.m_latency_threshold,
        param.m_job_id_pack_size
    );

    param.m_sampling_step = CalculateSamplingStep( mvpl );
    param.m_subpixel_level = CalculateSubpixelLevel( param, mvpl, *param.m_camera );

    // make variable range
    for ( int tf = 0; tf < tf_number; tf++ )
    {
        std::stringstream ss;
        ss << (tf + 1);
        const std::string idxbuf = ss.str();
        vr.setValue("t" + idxbuf + "_var_o", param.m_transfunc_synthesizer->m_c_min[tf])
        vr.setValue("t" + idxbuf + "_var_o", param.m_transfunc_synthesizer->m_c_min[tf])
        vr.setValue("t" + idxbuf + "_var_c", param.m_transfunc_synthesizer->m_c_min[tf])
        vr.setValue("t" + idxbuf + "_var_c", param.m_transfunc_synthesizer->m_c_min[tf])
    }

    while ( jd.dispatchNext( wid, &st, &vl ) )
    {
        int xvl, fidx;
        vismodule::PointObject* tmp_obj = NULL;
        fidx = mvpl.getFileIndex( vl, &xvl );
        MultiVolumeProperty& mvp = mvpl.m_list[fidx];
        mvp.setFilePath( param.m_input_data, st, xvl );
        point_generator_lst[fidx].setFilterInfo( &mvp );
        param.m_subvolume_id = xvl;

        // generate point object start
        try
        {
            point_generator_lst[fidx].setCoordSynthStr( param.m_x_synthesis, param.m_y_synthesis, param.m_z_synthesis );
            if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
            {
                tmp_obj = point_generator_lst[fidx].run( param, *param.m_camera, st, xvl );

            }
#ifdef EXTEND_FILE_FORMAT
            else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
            {
                tmp_obj = point_generator_lst[fidx].run( param, *param.m_camera, st, xvl );
            }                            
#endif
            else // filetype: kvsml
            {
                tmp_obj = point_generator_lst[fidx].run( param, *param.m_camera, st );
            }
        }
        catch ( const std::runtime_error& e )
        {
#ifdef _DEBUG		// debug by @hira
            printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
            std::cerr << e.what();
            nan_error = true;
        }
        // generate point object end

        // make histgram start
        int c_bins_size = 0;
        int o_bins_size = 0;
        for ( int tf = 0; tf < tf_number; tf++ )
        {
            c_bins_size += DEFAULT_NBINS;
            o_bins_size += DEFAULT_NBINS;
        }

        vismodule::UInt64* tmp_c_bins;
        vismodule::UInt64* tmp_o_bins;
        tmp_c_bins = new vismodule::UInt64[c_bins_size];
        tmp_o_bins = new vismodule::UInt64[o_bins_size];
                            
        for ( int tf = 0; tf < c_bins_size; tf++ )
        {
            tmp_c_bins[tf] = 0;
        }

        for ( int tf = 0; tf < o_bins_size; tf++ )
        {
            tmp_o_bins[tf] = 0;
        }

        int c_count = 0;
        for ( int tf = 0; tf < tf_number; tf++ )
        {
            c_nbins = tmp_obj->getNbins();
            for ( int res = 0; res < c_nbins; res++ )
            {
                tmp_c_bins[c_count] += tmp_obj->getCHistogram()[c_count] ;
                c_count++;
            }
        }

        int o_count = 0;
        for ( int tf = 0; tf < tf_number; tf++ )
        {
            o_nbins = tmp_obj->getNbins();
            for ( int res = 0; res < o_nbins; res++ )
            {
                tmp_o_bins[o_count] += tmp_obj->getOHistogram()[o_count];
                o_count++;
            }
        }
        // make histgram end

#ifndef CPU_VER
        jc.jobCollect( tmp_obj, &vr, &nan_error, &wid );
#endif
        if ( nan_error )
        {
            nan_error = false;
        }

        delete tmp_obj;
    } // end of while(DispatchNext)

#ifndef CPU_VER
    MPI_Allreduce( MPI_IN_PLACE, tmp_c_bins, c_bins_size, MPI_UNSIGNED_LONG, MPI_SUM, MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, tmp_o_bins, o_bins_size, MPI_UNSIGNED_LONG, MPI_SUM, MPI_COMM_WORLD );
#endif

    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
}

void generate_particle_IS(
    jpv::ParticleTransferServerMessage& servMes,
    Argument &param,
    MultiVolumePropertyList& mvpl, 
    JobDispatcher& jd,
    jpv::ParticleTransferServer pts
)
{
    int st, vl, wid = 0;
    VariableRange vr;
    int tf_number;

    // Using environment variables, the constructor of the ParticleMonitor class
    // set particle file, glyph file, plot over line file, status file, history file,
    // and the min/max coordinates of the object.
    ParticleMonitor pm;
    pm.check();

    if( pm.stepExisted() )
    {
        pm.setTimeStep_particle( mvpl.m_total_last_step );
    }
    else
    {
        pm.setTimeStep_particle(0);
        std::cout << "WARN:particle status file does not exist" << std::endl;
    }
    pm.readParticleHistoryFile();

    tf_number = pm.particleHistoryFile().colorHistogramArray().size();
    vr = pm.particleHistoryFile().variableRange();

#if 0
    std::cout << "particle monitor variable range" << std::endl;
    std::cout << "tfname, MIN, MAX" << std::endl;
    for( int i = 0; i < tf_number; i++ )
    {
        std::stringstream tt;
        float c_min, c_max, o_min, o_max;

        tt << "t" << i + 1;
        c_min = vr.min( tt.str() + "_var_c" );
        c_max = vr.max( tt.str() + "_var_c" );
        o_min = vr.min( tt.str() + "_var_o" );
        o_max = vr.max( tt.str() + "_var_o" );

        std::cout << "C" << i << ":" << c_min << ", " << c_max << std::endl;
        std::cout << "C" << i << ":" << o_min << ", " << o_max << std::endl;
    }
#endif

    // get pointObject
    vismodule::PointObject* originalObject = new vismodule::PointObject;
    pm.readParticleFile();
    pm.getParticle( originalObject );

    // get histgram start
    int c_bins_size = 0;
    int o_bins_size = 0;
    for ( int tf = 0; tf < tf_number; tf++ )
    {
        c_bins_size += DEFAULT_NBINS;
        o_bins_size += DEFAULT_NBINS;
    }

    vismodule::UInt64* tmp_c_bins;
    vismodule::UInt64* tmp_o_bins;
    tmp_c_bins = new vismodule::UInt64[c_bins_size];
    tmp_o_bins = new vismodule::UInt64[o_bins_size];

    for ( int tf = 0; tf < c_bins_size; tf++ )
    {
        tmp_c_bins[tf] = 0;
    }

    for ( int tf = 0; tf < o_bins_size; tf++ )
    {
        tmp_o_bins[tf] = 0;
    }

    int c_count = 0;
    for ( int tf = 0; tf < tf_number; tf++ )
    {
        for ( int res = 0; res < DEFAULT_NBINS; res++ )
        {
            tmp_c_bins[c_count] = pm.particleHistoryFile().colorHistogramArray()[tf][res];
            c_count++;
        }
    }

    int o_count = 0;
    for ( int tf = 0; tf < tf_number; tf++ )
    {
        for ( int res = 0; res < DEFAULT_NBINS; res++ )
        {
            tmp_o_bins[o_count] = pm.particleHistoryFile().opacityHistogramArray()[tf][res];
            o_count++;
        }
    }
    // get histgram end

    // make sub volume num server message start
    strncpy( servMes.m_header, "JPTP /1.0 100 OK\r\n", 18 );
    servMes.m_camera = param.m_camera;
    servMes.m_server_status = 0;
    servMes.m_time_step = param.m_time_step;
    servMes.m_level_index = param.m_level_index;
    servMes.m_repeat_level = param.m_repeat_level;
    servMes.m_number_particle = 0;
    servMes.m_number_glyph = 0;
    servMes.m_flag_send_bins = 1;
    servMes.m_number_volume_divide = 1;
    servMes.m_transfer_function_count = 0;
    servMes.m_start_step = mvpl.m_total_start_steps;
    servMes.m_last_step = mvpl.m_total_last_step;
    servMes.m_number_step = mvpl.m_total_number_steps;
    servMes.m_min_object_coord[0] = mvpl.m_total_min_object_coord[0];
    servMes.m_min_object_coord[1] = mvpl.m_total_min_object_coord[1];
    servMes.m_min_object_coord[2] = mvpl.m_total_min_object_coord[2];
    servMes.m_max_object_coord[0] = mvpl.m_total_max_object_coord[0];
    servMes.m_max_object_coord[1] = mvpl.m_total_max_object_coord[1];
    servMes.m_max_object_coord[2] = mvpl.m_total_max_object_coord[2];
    // servMes.m_min_value = mvpl.m_total_min_value;
    // servMes.m_max_value = mvpl.m_total_max_value;
    servMes.m_number_nodes = mvpl.m_total_number_nodes;
    servMes.m_number_elements = mvpl.m_total_number_elements;
    servMes.m_element_type = mvpl.m_list[0].m_elem_type;
    servMes.m_file_type = mvpl.m_list[0].m_file_type;
    servMes.m_number_ingredients = mvpl.m_list[0].m_number_ingredients;
    servMes.m_color_transfer_function_synthesis = param.m_color_transfer_function_synthesis;
    servMes.m_opacity_transfer_function_synthesis = param.m_opacity_transfer_function_synthesis;
    servMes.m_particle_limit = param.m_particle_limit;
    servMes.m_particle_density = param.m_particle_density;
    servMes.m_subpixel_level = param.m_subpixel_level;
    servMes.m_server_side_variable_range = vr;
    // make sub volume num server message end

    // send sub volume num server message
    std::cout << "INFO: send sub volume num server message" << std::endl;
    servMes.m_message_size = servMes.byteSize();
    servMes.show();
    pts.sendMessage( servMes );

    // make particle server message start
    servMes.m_flag_send_bins = 0;
    servMes.m_number_particle = originalObject->coords().size() / 3;
    printf(" %zu perticles generated\n", servMes.m_number_particle);

    if ( servMes.m_number_particle > 0 )
    {
        servMes.m_positions = std::make_unique<float[]>(3 * servMes.m_number_particle);
        servMes.m_normals   = std::make_unique<float[]>(3 * servMes.m_number_particle);
        servMes.m_colors    = std::make_unique<unsigned char[]>(3 * servMes.m_number_particle);
    }
    else
    {
        servMes.m_positions = NULL;
        servMes.m_normals   = NULL;
        servMes.m_colors    = NULL;
    }
    for ( int i = 0; i < servMes.m_number_particle; ++i )
    {
        servMes.m_positions[3 * i + 0] = originalObject->coords()[3 * i + 0];
        servMes.m_positions[3 * i + 1] = originalObject->coords()[3 * i + 1];
        servMes.m_positions[3 * i + 2] = originalObject->coords()[3 * i + 2];
        servMes.m_normals[3 * i + 0]   = originalObject->normals()[3 * i + 0];
        servMes.m_normals[3 * i + 1]   = originalObject->normals()[3 * i + 1];
        servMes.m_normals[3 * i + 2]   = originalObject->normals()[3 * i + 2];
        servMes.m_colors[3 * i + 0]    = originalObject->colors()[3 * i + 0];
        servMes.m_colors[3 * i + 1]    = originalObject->colors()[3 * i + 1];
        servMes.m_colors[3 * i + 2]    = originalObject->colors()[3 * i + 2];
    }
    // make particle server message end

    // send particle server message
    std::cout << "INFO: send particle server message" << std::endl;
    servMes.m_message_size = servMes.byteSize();
    servMes.show();
    pts.sendMessage( servMes );

    delete originalObject;

    // make histgram server message start
    servMes.m_number_particle = 0;
    servMes.m_flag_send_bins = 1;
    servMes.m_transfer_function_count = tf_number;
    servMes.m_color_nbins   = new vismodule::UInt64[tf_number];
    servMes.m_opacity_nbins = new vismodule::UInt64[tf_number];
    servMes.m_color_bins.resize( tf_number );
    servMes.m_opacity_bins.resize( tf_number );

    for ( int tf = 0; tf < tf_number; tf++ )
    {
        servMes.m_color_nbins[tf] = DEFAULT_NBINS;
        servMes.m_opacity_nbins[tf] = DEFAULT_NBINS;
        servMes.m_color_bins[tf]   =  new vismodule::UInt64[ DEFAULT_NBINS ];
        servMes.m_opacity_bins[tf] =  new vismodule::UInt64[ DEFAULT_NBINS ];
        for ( int res = 0; res < DEFAULT_NBINS; res++ )
        {
            servMes.m_color_bins[tf][res] = 0;
        }
        for ( int res = 0; res < DEFAULT_NBINS; res++ )
        {
            servMes.m_opacity_bins[tf][res] = 0;
        }
    }

    servMes.setColorHistogramBins
    (
        param.m_transfunc_array.size(),
        DEFAULT_NBINS,
        tmp_c_bins
    );
    servMes.setOpacityHistogramBins
    (
        param.m_transfunc_array.size(),
        DEFAULT_NBINS,
        tmp_o_bins
    );
    // make histgram server message end

    // send histgram server message
    std::cout << "INFO: send histgram server message" << std::endl;
    servMes.m_message_size = servMes.byteSize();
    servMes.show();
    pts.sendMessage( servMes );

    for ( int tf = 0; tf < servMes.m_transfer_function_count; tf++ )
    {
        delete[] servMes.m_color_bins[tf];
        delete[] servMes.m_opacity_bins[tf];
    }
    delete[] servMes.m_color_nbins;
    delete[] servMes.m_opacity_nbins;

    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
}
#endif