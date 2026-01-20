#include <vismodule/Calculate>
#include <vismodule/JobDispatcher>
#include <vismodule/GenerateParticle>
#include <vismodule/GenerateGlyph>
#include <vismodule/ParticleProperty>
#include <vismodule/GlyphProperty>
#include <vismodule/KVSMLObjectGlyph>
#include <vismodule/GlyphSeedGenerator>
#include <vismodule/ParticleMonitor>
#include <vismodule/ParameterFileReader>

#ifndef CPU_VER
    #include <vismodule/JobCollector>
#endif

void SetDefaultGlyphParameterCS( GlyphProperty& glyph_property )
{
    std::vector<int32_t> glyph_color_map_table = {59,76,192,60,78,194,61,80,195,62,81,197,64,83,198,65,85,200,66,86,201,67,88,203,68,90,204,69,92,206,71,93,207,72,95,209,73,97,210,74,99,211,75,100,213,77,102,214,78,104,215,79,105,217,80,107,218,82,109,219,83,110,221,84,112,222,85,114,223,87,115,224,88,117,225,89,119,226,90,120,228,92,122,229,93,124,230,94,125,231,95,127,232,97,128,233,98,130,234,99,132,235,101,133,236,102,135,237,103,136,238,105,138,239,106,139,239,107,141,240,109,142,241,110,144,242,111,145,243,113,147,243,114,148,244,115,150,245,117,151,246,118,153,246,119,154,247,121,156,248,122,157,248,124,159,249,125,160,249,126,161,250,128,163,250,129,164,251,130,166,251,132,167,252,133,168,252,135,170,252,136,171,253,137,172,253,139,173,253,140,175,254,141,176,254,143,177,254,144,178,254,146,180,254,147,181,255,148,182,255,150,183,255,151,184,255,153,185,255,154,187,255,155,188,255,157,189,255,158,190,255,159,191,255,161,192,255,162,193,255,164,194,254,165,195,254,166,196,254,168,197,254,169,198,254,170,199,253,172,200,253,173,201,253,174,201,252,176,202,252,177,203,252,178,204,251,180,205,251,181,206,250,182,206,250,183,207,249,185,208,249,186,209,248,187,209,248,189,210,247,190,211,246,191,211,246,192,212,245,193,212,244,195,213,244,196,214,243,197,214,242,198,215,241,200,215,241,201,216,240,202,216,239,203,216,238,204,217,237,205,217,236,206,218,235,208,218,234,209,218,233,210,219,232,211,219,231,212,219,230,213,219,229,214,220,228,215,220,227,216,220,226,217,220,225,218,220,224,219,221,222,220,221,221,221,220,220,222,220,219,223,219,217,225,219,216,226,218,214,227,218,213,228,217,211,229,217,210,229,216,209,230,216,207,231,215,206,232,214,204,233,214,203,234,213,201,235,212,200,235,211,198,236,211,197,237,210,195,238,209,194,238,208,192,239,207,191,239,206,189,240,206,187,241,205,186,241,204,184,242,203,183,242,202,181,243,201,180,243,200,178,244,199,177,244,198,175,244,197,173,245,196,172,245,195,170,245,193,169,246,192,167,246,191,166,246,190,164,246,189,162,247,188,161,247,186,159,247,185,158,247,184,156,247,183,155,247,181,153,247,180,151,247,179,150,247,178,148,247,176,147,247,175,145,247,173,143,247,172,142,247,171,140,247,169,139,247,168,137,247,166,136,246,165,134,246,163,132,246,162,131,246,160,129,245,159,128,245,157,126,245,156,125,244,154,123,244,153,122,244,151,120,243,149,119,243,148,117,242,146,116,242,144,114,241,143,112,241,141,111,240,139,109,240,138,108,239,136,106,239,134,105,238,133,104,237,131,102,237,129,101,236,127,99,235,125,98,234,124,96,234,122,95,233,120,93,232,118,92,231,116,90,230,114,89,229,112,88,229,111,86,228,109,85,227,107,83,226,105,82,225,103,81,224,101,79,223,99,78,222,97,77,221,95,75,220,93,74,219,91,73,218,89,71,216,86,70,215,84,69,214,82,67,213,80,66,212,78,65,211,76,63,209,73,62,208,71,61,207,69,60,206,67,59,204,64,57,203,62,56,202,59,55,200,57,54,199,54,52,198,52,51,196,49,50,195,46,49,193,43,48,192,40,47,191,37,46,189,34,44,188,30,43,186,26,42,185,22,41,183,17,40,182,11,39,180,4,38};
    
    // std::vector<std::string> size_variables = { "q1", "q2", "q3" };
    // std::vector<std::string> color_data_variables = { "q1", "q2", "q3" };
    std::vector<std::string> size_variables = { "q1" };
    std::vector<std::string> color_data_variables = { "q1" };
    
    glyph_property.m_glyph_flag                 = true;
    glyph_property.m_glyph_type                 = GlyphType::Arrow;
    glyph_property.m_scale_factor               = 1;
    glyph_property.m_direction_variable[0]      = "q1";
    glyph_property.m_direction_variable[1]      = "q2";
    glyph_property.m_direction_variable[2]      = "q3";
    glyph_property.m_size_sampling_method       = DataDefines::Constant;
    glyph_property.m_size_variable              = size_variables;
    glyph_property.m_distribution_mode          = GlyphMode::UniformDistribution;
    // glyph_property.m_distribution_mode          = GlyphMode::EveryNthPoints;
    glyph_property.m_number_of_sampling_point   = 1000;
    glyph_property.m_seed                       = 1;
    glyph_property.m_stride                     = 3;
    glyph_property.m_color_data_sampling_method = DataDefines::Constant;
    glyph_property.m_color_data_variable        = color_data_variables;
    glyph_property.m_glyph_color_map_table      = glyph_color_map_table;
    glyph_property.m_glyph_color_min            = 0;
    glyph_property.m_glyph_color_max            = 1;
    glyph_property.m_glyph_size_min             = 0;
    glyph_property.m_glyph_size_max             = 1;
    
    return;
}

std::unique_ptr<kvs::PolygonGlyphObject> GenerateGlyphCS(
    std::string& file_path,
    const int time_step,
    const GlyphProperty& glyph_property,
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
    const int size_index  = 0; // for minmax array index
    const int color_index = 1; // for minmax array index
    JobDispatcher jd;
    std::unique_ptr<kvs::PolygonGlyphObject> glyph_object = std::make_unique<kvs::PolygonGlyphObject>();
    bool nan_error = false;

#ifndef CPU_VER
    JobCollector jc( &jd );
#endif

    /* 一旦保留
    int cnt = 2;
    float* tmp_max;
    float* tmp_min;
    tmp_max = new float[cnt]; 
    tmp_min = new float[cnt];
    
    for ( int tf = 0; tf < cnt; tf++ )
    {
        tmp_max[tf] = FLT_MIN;
        tmp_min[tf] = FLT_MAX;
    }
    */

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
        vismodule::KVSMLObjectGlyph* recv_obj = new vismodule::KVSMLObjectGlyph;
        vismodule::KVSMLObjectGlyph* send_obj = new vismodule::KVSMLObjectGlyph;

        // make glyph
        // make size and color minmax
        if ( ( rank > 0 ) || ( mpi_size == 1 ) )
        {
            int xvl, fidx;
            fidx = mvpl.getFileIndex( vl, &xvl );
            MultiVolumeProperty& mvp = mvpl.m_list[fidx];
            mvp.setFilePath( file_path, st, xvl );

            // generate glyph start
            try
            {
                vismodule::VolumeObjectBase* volume = nullptr;
                vismodule::GlyphSeedGenerator glyph_creator;

                if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                {
                    generate_volume( file_path, mvp, st, xvl, volume );
                }
#ifdef EXTEND_FILE_FORMAT
                else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                {
                    generate_volume( file_path, mvp, st, xvl, volume );
                }                                
#endif
                else     // filetype: kvsml
                {
                    generate_volume( file_path, mvp, volume );
                }

                if ( !volume )
                {
                    throw std::runtime_error("Failed to generate volume object.");
                }

                std::unique_ptr<std::unique_ptr<Type[]>[]> values;
                int nvariables = 0;
                int ncoords = 0;
                vismodule::VolumeObjectBase::VolumeType voltype = volume->volumeType();
                int number_of_divide = mvpl.m_total_number_subvolumes;

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

                    // generate particle
                    glyph_creator.GenerateGlyphUnstruct(
                        glyph_property,
                        number_of_divide,
                        raw_pointers_vector.data(),
                        nvariables,
                        coordinates.get(),
                        ncoords,
                        connections.get(),
                        ncells,
                        celltype,
                        ServerMode::CS,
                        send_obj
                    );
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
                    glyph_creator.GenerateGlyphStruct(
                        glyph_property,
                        number_of_divide,
                        dom,
                        raw_pointers_vector.data(),
                        nvariables,
                        ServerMode::CS,
                        send_obj
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
            // generate glyph end

            /* 一旦保留
            // get minmax
            tmp_max[size_index]  = vismodule::Math::Max(  tmp_max[size_index], send_obj->sizeMax()  );
            tmp_min[size_index]  = vismodule::Math::Min(  tmp_min[size_index], send_obj->sizeMin()  );
            tmp_max[color_index] = vismodule::Math::Max( tmp_max[color_index], send_obj->colorMax() );
            tmp_min[color_index] = vismodule::Math::Min( tmp_min[color_index], send_obj->colorMin() );
            */
        }
        // make glyph
        // make size and color minmax

#ifndef CPU_VER
        if ( mpi_size > 1 ) {
            if ( rank == 0 )
            {
                jc.jobCollect_glyph( recv_obj, &nan_error, &wid );
            }
            else
            {
                jc.jobCollect_glyph( send_obj, &nan_error, &wid );
            }
        }
#else
        recv_obj->clear();
        recv_obj->setCoords( send_obj->coords() );
        recv_obj->setColors( send_obj->colors() );
        recv_obj->setDirections( send_obj->directions() );
        recv_obj->setSizes( send_obj->sizes() );
        recv_obj->setColorMin( send_obj->colorMin() );
        recv_obj->setColorMax( send_obj->colorMax() );
        recv_obj->setSizeMin( send_obj->sizeMin() );
        recv_obj->setSizeMax( send_obj->sizeMax() );
        printf(" %zu glyphs generated\n", send_obj->coords().size() / 3);
#endif
        delete send_obj;

    /*
    std::cout << "recv_obj->directions():" << std::endl;
    int counter = 0;
    for ( size_t i = 0; i < recv_obj->directions().size(); i++ )
    {
        if ( i % 3 == 0 ) std::cout << counter << ":";
        
        std::cout << recv_obj->directions()[i] << ", ";
        
        if ( i % 3 == 2 )
        {
            counter++;
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
    */

        kvs::ValueArray<kvs::Real32> kvs_coords;
        kvs::ValueArray<kvs::UInt8>  kvs_colors;
        kvs::ValueArray<kvs::Real32> kvs_directions;
        kvs::ValueArray<kvs::Real32> kvs_sizes;

        if ( glyph_object->coords().size() > 0 )
        {
            const size_t ncoords     = glyph_object->coords().size() + recv_obj->coords().size();
            const size_t ncolors     = glyph_object->colors().size() + recv_obj->colors().size();
            const size_t ndirections = glyph_object->directions().size() + recv_obj->directions().size();
            const size_t nsizes      = glyph_object->sizes().size() + recv_obj->sizes().size();

            kvs_coords.allocate( ncoords );
            kvs_colors.allocate( ncolors );
            kvs_directions.allocate( ndirections );
            kvs_sizes.allocate( nsizes );

            kvs::Real32* pcoords     = kvs_coords.data();
            kvs::UInt8* pcolors      = kvs_colors.data();
            kvs::Real32* pdirections = kvs_directions.data();
            kvs::Real32* psizes      = kvs_sizes.data();

            memcpy( pcoords, glyph_object->coords().data(), glyph_object->coords().byteSize() );
            memcpy( pcoords + glyph_object->coords().size(), recv_obj->coords().pointer(), recv_obj->coords().byteSize() );
            memcpy( pcolors, glyph_object->colors().data(), glyph_object->colors().byteSize() );
            memcpy( pcolors + glyph_object->colors().size(), recv_obj->colors().pointer(), recv_obj->colors().byteSize() );
            memcpy( pdirections, glyph_object->directions().data(), glyph_object->directions().byteSize() );
            memcpy( pdirections + glyph_object->directions().size(), recv_obj->directions().pointer(), recv_obj->directions().byteSize() );     
            memcpy( psizes, glyph_object->sizes().data(), glyph_object->sizes().byteSize() );
            memcpy( psizes + glyph_object->sizes().size(), recv_obj->sizes().pointer(), recv_obj->sizes().byteSize() );     
            
            glyph_object->setCoords( kvs_coords );
            glyph_object->setColors( kvs_colors );
            glyph_object->setDirections( kvs_directions );
            glyph_object->setSizes( kvs_sizes );
        }
        else
        {
            const size_t ncoords     = recv_obj->coords().size();
            const size_t ncolors     = recv_obj->colors().size();
            const size_t ndirections = recv_obj->directions().size();
            const size_t nsizes      = recv_obj->sizes().size();

            kvs_coords.allocate( ncoords );
            kvs_colors.allocate( ncolors );
            kvs_directions.allocate( ndirections );
            kvs_sizes.allocate( nsizes );

            kvs::Real32* pcoords     = kvs_coords.data();
            kvs::UInt8* pcolors      = kvs_colors.data();
            kvs::Real32* pdirections = kvs_directions.data();
            kvs::Real32* psizes      = kvs_sizes.data();

            memcpy( pcoords, recv_obj->coords().pointer(), recv_obj->coords().byteSize() );
            memcpy( pcolors, recv_obj->colors().pointer(), recv_obj->colors().byteSize() );
            memcpy( pdirections, recv_obj->directions().pointer(), recv_obj->directions().byteSize() );     
            memcpy( psizes, recv_obj->sizes().pointer(), recv_obj->sizes().byteSize() );

            glyph_object->setCoords( kvs_coords );
            glyph_object->setColors( kvs_colors );
            glyph_object->setDirections( kvs_directions );
            glyph_object->setSizes( kvs_sizes );
        }

        delete recv_obj;
    } // end of while(DispatchNext)

#ifndef CPU_VER
    /* 一旦保留
    if ( mpi_size > 1 )
    {
        MPI_Allreduce( MPI_IN_PLACE, tmp_max, cnt, MPI_FLOAT, MPI_MAX , MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_min, cnt, MPI_FLOAT, MPI_MIN , MPI_COMM_WORLD );
    }
    */
#endif

    nan_error = false;
    /* 一旦保留
    delete[] tmp_min;
    delete[] tmp_max;
    */

    /*
    std::cout << "glyph_object->directions():" << std::endl;
    int counter = 0;
    for ( size_t i = 0; i < glyph_object->directions().size(); i++ )
    {
        if ( i % 3 == 0 ) std::cout << counter << ":";
        
        std::cout << glyph_object->directions()[i] << ", ";
        
        if ( i % 3 == 2 )
        {
            counter++;
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
    */

    kvs::ValueArray<kvs::Real32> kvs_scaled_sizes;
    kvs_scaled_sizes.allocate( glyph_object->sizes().size() );

    for ( size_t i = 0; i < glyph_object->sizes().size(); i++ )
    {
        kvs_scaled_sizes[i] = glyph_object->sizes()[i] * glyph_property.m_scale_factor;
    }

    return std::make_unique<kvs::PolygonGlyphObject>(
        glyph_object->coords(),
        glyph_object->directions(),
        kvs_scaled_sizes,
        glyph_object->colors(),
        static_cast<kvs::PolygonGlyphObject::GlyphType>( glyph_property.m_glyph_type )
    );
}

void SetDefaultGlyphParameterIS( GlyphProperty& glyph_property )
{
    const char *envBuf = NULL;
    std::string visParamDir;
    std::string glyphParameterPath;
    std::string glyphParameterPath_old;
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

    glyphParameterPath     = visParamDir;
    glyphParameterPath_old = visParamDir;
    glyphParameterPath     += "parameter.gly";
    glyphParameterPath_old += "parameter_old.gly";

    ppr.readGlyphParameterFile( glyphParameterPath_old.c_str() );

    ppr.setGlyphParameter( glyph_property );
}

std::unique_ptr<kvs::PolygonGlyphObject> GenerateGlyphIS(
    const int time_step,
    const GlyphProperty& glyph_property,
    const MultiVolumePropertyList& mvpl
)
{
    std::unique_ptr<vismodule::KVSMLObjectGlyph> recv_obj = std::make_unique<vismodule::KVSMLObjectGlyph>();
    std::unique_ptr<kvs::PolygonGlyphObject> glyph_object = std::make_unique<kvs::PolygonGlyphObject>();
    const int size_index  = 0; // for minmax array index
    const int color_index = 1; // for minmax array index

    /* 一旦保留
    int cnt = 2;
    float* tmp_max;
    float* tmp_min;
    tmp_max = new float[cnt]; 
    tmp_min = new float[cnt];
    
    for ( int tf = 0; tf < cnt; tf++ )
    {
        tmp_max[tf] = FLT_MIN;
        tmp_min[tf] = FLT_MAX;
    }
    */

    ParticleMonitor pm;
    pm.check();
    
    if( pm.stepExisted() )
    {
        pm.setTimeStep_glyph( time_step );
    }
    else
    {
        pm.setTimeStep_glyph(0);
    }
    
    // get glyph
    pm.readGlyphFile();
    pm.getGlyph( recv_obj.get() );
    
    /* 一旦保留
    // get minmax
    tmp_max[size_index]  = vismodule::Math::Max(  tmp_max[size_index], kvsml_object_glyph->sizeMax()  );
    tmp_min[size_index]  = vismodule::Math::Min(  tmp_min[size_index], kvsml_object_glyph->sizeMin()  );
    tmp_max[color_index] = vismodule::Math::Max( tmp_max[color_index], kvsml_object_glyph->colorMax() );
    tmp_min[color_index] = vismodule::Math::Min( tmp_min[color_index], kvsml_object_glyph->colorMin() );
    */

    // delete[] tmp_min;
    // delete[] tmp_max;

    kvs::ValueArray<kvs::Real32> kvs_coords;
    kvs::ValueArray<kvs::UInt8>  kvs_colors;
    kvs::ValueArray<kvs::Real32> kvs_directions;
    kvs::ValueArray<kvs::Real32> kvs_sizes;

    const size_t ncoords     = recv_obj->coords().size();
    const size_t ncolors     = recv_obj->colors().size();
    const size_t ndirections = recv_obj->directions().size();
    const size_t nsizes      = recv_obj->sizes().size();

    kvs_coords.allocate( ncoords );
    kvs_colors.allocate( ncolors );
    kvs_directions.allocate( ndirections );
    kvs_sizes.allocate( nsizes );

    kvs::Real32* pcoords     = kvs_coords.data();
    kvs::UInt8* pcolors      = kvs_colors.data();
    kvs::Real32* pdirections = kvs_directions.data();
    kvs::Real32* psizes      = kvs_sizes.data();

    memcpy( pcoords, recv_obj->coords().pointer(), recv_obj->coords().byteSize() );
    memcpy( pcolors, recv_obj->colors().pointer(), recv_obj->colors().byteSize() );
    memcpy( pdirections, recv_obj->directions().pointer(), recv_obj->directions().byteSize() );     
    memcpy( psizes, recv_obj->sizes().pointer(), recv_obj->sizes().byteSize() );

    glyph_object->setCoords( kvs_coords );
    glyph_object->setColors( kvs_colors );
    glyph_object->setDirections( kvs_directions );
    glyph_object->setSizes( kvs_sizes );

    kvs::ValueArray<kvs::Real32> kvs_scaled_sizes;
    kvs_scaled_sizes.allocate( glyph_object->sizes().size() );

    for ( size_t i = 0; i < glyph_object->sizes().size(); i++ )
    {
        kvs_scaled_sizes[i] = glyph_object->sizes()[i] * glyph_property.m_scale_factor;
    }

    return std::make_unique<kvs::PolygonGlyphObject>(
        glyph_object->coords(),
        glyph_object->directions(),
        kvs_scaled_sizes,
        glyph_object->colors(),
        static_cast<kvs::PolygonGlyphObject::GlyphType>( glyph_property.m_glyph_type )
    );
}

