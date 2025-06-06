#ifndef VIS_MODULE_CALCULATE_H_INCLDE
#define VIS_MODULE_CALCULATE_H_INCLDE

//inline vismodule::UnstructuredVolumeObject* CreateVolumeData( const Argument& param,
inline vismodule::VolumeObjectBase* CreateVolumeData( const Argument& param,
                                                         const MultiVolumeProperty& mvp,
                                                         const int& steps, const int& subvols )
{
    if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 )
    {
        vismodule::File ifpx( mvp.m_file_path );
        std::string path_base = ifpx.pathName() + ifpx.Separator() + ifpx.baseName();
        //vismodule::UnstructuredVolumeObject* volume = new vismodule::UnstructuredVolumeImporter( path_base,
        vismodule::VolumeObjectBase* volume = new vismodule::UnstructuredVolumeImporter( path_base, mvp.m_file_type, steps, subvols );
        volume->setMinMaxValues( mvp.m_min_value, mvp.m_max_value );
        volume->setMinMaxObjectCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
        volume->setMinMaxExternalCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );

        return volume;
    }
#ifdef EXTEND_FILE_FORMAT
    // structured
    else if ( mvp.m_file_type == 3 )
    {
        vismodule::VolumeObjectBase* volume = new vismodule::StructuredVolumeImporter( mvp.m_file_path, steps, subvols );
        volume->setMinMaxValues( mvp.m_min_value, mvp.m_max_value );
        volume->setMinMaxObjectCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
        volume->setMinMaxExternalCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
        return volume;
    }
    // unstructured
    else if ( mvp.m_file_type == 4 )
    {
        vismodule::VolumeObjectBase* volume = new vismodule::UnstructuredVolumeImporter( mvp.m_file_path, mvp.m_file_type, mvp.m_elem_type, steps, subvols );
        volume->setMinMaxValues( mvp.m_min_value, mvp.m_max_value );
        volume->setMinMaxObjectCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
        volume->setMinMaxExternalCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
        return volume;
    }
#endif
    else
    {
        std::stringstream suffix;
        suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << ( steps )
               << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( subvols + 1 )
               << '_' << std::setw( 7 ) << std::setfill( '0' ) << mvp.m_number_subvolumes;

        //std::string m_input_data = param.m_input_data_base + suffix.str() + ".kvsml";
        vismodule::File ifpx( mvp.m_file_path );
        std::string m_input_data = ifpx.pathName() + ifpx.Separator()
                                   + ifpx.baseName() + suffix.str() + ".kvsml";
        //vismodule::UnstructuredVolumeObject* volume = new vismodule::UnstructuredVolumeImporter( m_input_data );

        vismodule::VolumeObjectBase* volume = nullptr;

        if      ( vismoduleview::FileChecker::ImportableStructuredVolume( m_input_data ))
        {
            std::cout << "Structured !" <<std::endl;
            volume = new vismodule::StructuredVolumeImporter( m_input_data ); 
        } 
        else if ( vismoduleview::FileChecker::ImportableUnstructuredVolume( m_input_data))
        {
            std::cout << "Unstructured !" <<std::endl;
            volume = new vismodule::UnstructuredVolumeImporter( m_input_data );  
        }
        else 
        {
            visModuleMessageError("%s is not volume data.", m_input_data.c_str());
            //return false;
        }

            volume->setMinMaxValues( mvp.m_min_value, mvp.m_max_value );
            volume->setMinMaxObjectCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
            volume->setMinMaxExternalCoords( mvp.m_min_object_coord, mvp.m_max_object_coord );
        return volume;
    }
}


inline float CalculateSamplingStep( const MultiVolumePropertyList& mvpl )
{
    float max_coord_length = vismodule::Math::Max<float>( mvpl.m_total_max_object_coord.x() - mvpl.m_total_min_object_coord.x(),
                                                    mvpl.m_total_max_object_coord.y() - mvpl.m_total_min_object_coord.y(),
                                                    mvpl.m_total_max_object_coord.z() - mvpl.m_total_min_object_coord.z() );
    return 0.1 * max_coord_length;
}

//kawamura2: This calculates optimized subpixel level.
inline size_t CalculateSubpixelLevel( const Argument& param,
                                      const MultiVolumePropertyList& mvpl,
                                      const vismodule::Camera& camera )
{
    namespace Generator = vismodule::CellByCellParticleGenerator;
    //vismodule::UnstructuredVolumeObject* volume;
    vismodule::VolumeObjectBase* volume;
    double total_volume = 0.0;
    double density_lev1 = 0.0;//kawamura2: particle density for subpixel_level=1
    int steps = mvpl.m_total_start_steps;
    int subvols = 0;

    //Total Volume Calculation
#ifndef CPU_VER
    int rank;
    int nprocs;
    MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
#else
    int rank = 0;
    int nprocs = 1;
#endif

    for ( subvols = 0; subvols < mvpl.m_total_number_subvolumes; subvols++ )
    {
        int xvl, fidx;
        fidx = mvpl.getFileIndex( subvols, &xvl );
        const MultiVolumeProperty& mvp = mvpl.m_list[fidx];

        if ( subvols % nprocs == rank )
        {
            VIS_MODULE_TIMER_STA( 16 );
            volume = CreateVolumeData( param, mvp, steps, xvl );
            VIS_MODULE_TIMER_END( 16 );

            VIS_MODULE_TIMER_STA( 17 );
            double local_volume = Generator::CalculateTotalVolume( *volume );
            VIS_MODULE_TIMER_END( 17 );

            VIS_MODULE_TIMER_STA( 18 );
            density_lev1 += Generator::CalculateGreatDensity( camera, *volume, 1,
                                                              param.m_sampling_step ) * local_volume;
            VIS_MODULE_TIMER_END( 18 );

            total_volume += local_volume;
            delete volume;
        }
    }

#ifndef CPU_VER
    VIS_MODULE_TIMER_STA( 19 );
    MPI_Allreduce( MPI_IN_PLACE, &density_lev1, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, &total_volume, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
    VIS_MODULE_TIMER_END( 19 );
#endif

    density_lev1 /= total_volume;

    //kawamura2 : Calculation of optimized subpixel level
    float particle_limit = param.m_particle_limit;
    float nparticles_lev1 = total_volume * density_lev1;
    float subpixel_level = sqrt( particle_limit / nparticles_lev1 );
    if ( subpixel_level < 1 ) subpixel_level = 1;
    else if ( subpixel_level > 12 ) subpixel_level = 12;

    return static_cast<size_t>( subpixel_level + 0.5f );
}

inline VariableRange Calculate_minmax( const Argument& param,
                                      const MultiVolumePropertyList& mvpl)
{
    namespace Generator = vismodule::CellByCellParticleGenerator;
    //vismodule::UnstructuredVolumeObject* volume;
    vismodule::VolumeObjectBase* volume;
    double total_volume = 0.0;
    double density_lev1 = 0.0;//kawamura2: particle density for subpixel_level=1
    int steps = mvpl.m_total_start_steps;
    int subvols = 0;

    vismodule::Real64 tmp_min, tmp_max;
    std::vector<vismodule::Real64> min_vec, max_vec;
    int nvariable = mvpl.m_total_number_ingredients;
    min_vec.resize(nvariable);
    max_vec.resize(nvariable);
    for(int i = 0 ;i < nvariable ; i++)
    {
        min_vec[i] = FLT_MAX; 
        max_vec[i] = FLT_MIN; 
    } 
# if 0
    //Total Volume Calculation
#ifndef CPU_VER
    int rank;
    int nprocs;
    MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
#else
    int rank = 0;
    int nprocs = 1;
#endif

    for ( steps = mvpl.m_total_start_steps; steps <= mvpl.m_total_last_step; steps++ )
    {
        for ( subvols = 0; subvols < mvpl.m_total_number_subvolumes; subvols++ )
        {
            int xvl, fidx;
            fidx = mvpl.getFileIndex( subvols, &xvl );
            const MultiVolumeProperty& mvp = mvpl.m_list[fidx];

            if ( subvols % nprocs == rank )
            {
                volume = CreateVolumeData( param, mvp, steps, xvl );
                //volume->updateMinMaxValues();
                int nnodes = volume->nnodes();
                for (int n =0; n< nvariable; n++) 
                {
                    tmp_min = volume->values().at<float>(0+n*nnodes); 
                    tmp_max = volume->values().at<float>(0+n*nnodes); 
                    for (int i = 1; i< nnodes; i++)
                    {
                        tmp_min = tmp_min < volume->values().at<float>(i+n*nnodes) ? tmp_min : volume->values().at<float>(i+n*nnodes) ; 
                        tmp_max = tmp_max > volume->values().at<float>(i+n*nnodes) ? tmp_max : volume->values().at<float>(i+n*nnodes) ; 
                    }
                    min_vec[n]=min_vec[n] < tmp_min ? min_vec[n] : tmp_min;
                    max_vec[n]=max_vec[n] > tmp_max ? max_vec[n] : tmp_max;
                }
                delete volume;
            }
        }
    }

#ifndef CPU_VER
    VIS_MODULE_TIMER_STA( 19 );
    MPI_Allreduce( MPI_IN_PLACE, min_vec.data(), nvariable, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, max_vec.data(), nvariable, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD );
    VIS_MODULE_TIMER_END( 19 );
#endif

   VariableRange vr;
   for (int n =0; n< nvariable; n++) 
   {
        std::stringstream ss; 
        ss << (n + 1); 
        const std::string idxbuf = ss.str();
        vr.setValue( "t" + idxbuf + "_var_o", max_vec[n]);
        vr.setValue( "t" + idxbuf + "_var_o", min_vec[n]);
        vr.setValue( "t" + idxbuf + "_var_c", max_vec[n]);
        vr.setValue( "t" + idxbuf + "_var_c", min_vec[n]);
   }
#endif

   VariableRange vr;
   for (int n =0; n< nvariable; n++) 
   {
        std::stringstream ss; 
        ss << (n + 1); 
        const std::string idxbuf = ss.str();
        vr.setValue( "t" + idxbuf + "_var_o", 1);
        vr.setValue( "t" + idxbuf + "_var_o", 0);
        vr.setValue( "t" + idxbuf + "_var_c", 1);
        vr.setValue( "t" + idxbuf + "_var_c", 0);
   }

//   std::cout << "vr_max = " << vr.max( "t1_var_c" ) << std::endl;     
//   std::cout << "vr_min = " << vr.min( "t1_var_c" ) << std::endl;     

   return vr;
}


void Calculate_minmax_glyph( const Argument& param,
                                      const MultiVolumePropertyList& mvpl,
                                      jpv::ParticleTransferClientMessage& clntMes)
{


    namespace Generator = vismodule::CellByCellParticleGenerator;
    vismodule::VolumeObjectBase* volume = nullptr;
    double total_volume = 0.0;
    double density_lev1 = 0.0;//kawamura2: particle density for subpixel_level=1
    //int steps = mvpl.m_total_start_steps;
    int steps = clntMes.m_step;
    int subvols = 0;

    vismodule::Real64 tmp_min, tmp_max;
    std::vector<float> min_vec, max_vec;
    int nvariable = mvpl.m_total_number_ingredients;
    int nvariablep2 = 2;
    min_vec.resize(nvariablep2);
    max_vec.resize(nvariablep2);
    for(int i = 0 ;i < nvariablep2 ; i++)
    {
        min_vec[i] = FLT_MAX; 
        max_vec[i] = FLT_MIN; 
    } 
# if 1
    //Total Volume Calculation
#ifndef CPU_VER
    int rank;
    int nprocs;
    MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
#else
    int rank = 0;
    int nprocs = 1;
#endif

    // color
    min_vec[0] = 0;
    max_vec[0] = 0;
    std::vector<int> color_data_variables;
    if( clntMes.m_color_data_sampling_method == jpv::DataDefines::VariableArray || clntMes.m_color_data_sampling_method == jpv::DataDefines::SingleVariable  )
    {
        for (int i =0 ; i< clntMes.m_color_data_variable.size(); i++)
        {
            color_data_variables.push_back( std::atoi(clntMes.m_color_data_variable[i].substr(1).c_str()) - 1); 
        }

    }
  // size
    min_vec[1] = 0;
    max_vec[1] = 0;
    std::vector<int> size_variables;
    if( clntMes.m_size_sampling_method == jpv::DataDefines::VariableArray || clntMes.m_size_sampling_method == jpv::DataDefines::SingleVariable  )
    {
        for (int i =0 ; i< clntMes.m_size_variable.size(); i++)
        {
            size_variables.push_back( std::atoi(clntMes.m_size_variable[i].substr(1).c_str()) - 1); 
        }

    }

      if( clntMes.m_color_data_sampling_method == jpv::DataDefines::VariableArray || clntMes.m_color_data_sampling_method == jpv::DataDefines::SingleVariable 
          || clntMes.m_color_data_sampling_method == jpv::DataDefines::VariableArray || clntMes.m_color_data_sampling_method == jpv::DataDefines::SingleVariable  )
      {
//    for ( steps = mvpl.m_total_start_steps; steps <= mvpl.m_total_start_step; steps++ ) //初回ステップのみ
//    {
        for ( subvols = 0; subvols < mvpl.m_total_number_subvolumes; subvols++ )
        {
            int xvl, fidx;
            fidx = mvpl.getFileIndex( subvols, &xvl );
            const MultiVolumeProperty& fi = mvpl.m_list[fidx];

            if ( subvols % nprocs == rank )
            {
                volume = CreateVolumeData( param, fi, steps, xvl );
                int nnodes = volume->nnodes();
                // color
                if( clntMes.m_color_data_sampling_method == jpv::DataDefines::VariableArray || clntMes.m_color_data_sampling_method == jpv::DataDefines::SingleVariable  )
                {
                    tmp_min = FLT_MAX;
                    tmp_max = FLT_MIN;
                    for (int i = 0; i< nnodes; i++)
                    {
                        float tmp = 0;
                        for(int k = 0 ; k< clntMes.m_color_data_variable.size() ; k++)
                        {
                            tmp += vismodule::Math::Square(volume->values().at<float>( i+ color_data_variables[k]*nnodes)) ;
                        }

                        tmp = std::sqrt(tmp);
                        tmp_min = tmp_min < tmp ? tmp_min : tmp ; 
                        tmp_max = tmp_max > tmp ? tmp_max : tmp ; 
                    }
                    min_vec[0]=min_vec[0] < tmp_min ? min_vec[0] : tmp_min;
                    max_vec[0]=max_vec[0] > tmp_max ? max_vec[0] : tmp_max;
            }
            // size
            if( clntMes.m_size_sampling_method == jpv::DataDefines::VariableArray || clntMes.m_size_sampling_method == jpv::DataDefines::SingleVariable  )
            {
                tmp_min = FLT_MAX;
                tmp_max = FLT_MIN;
                for (int i = 0; i< nnodes; i++)
                {
                    float tmp = 0;
                    for(int k = 0 ; k< clntMes.m_size_variable.size() ; k++)
                    {
                        tmp += vismodule::Math::Square(volume->values().at<float>( i+ size_variables[k]*nnodes)) ;
                    }
                    tmp = std::sqrt(tmp);
                    tmp_min = tmp_min < tmp ? tmp_min : tmp ; 
                    tmp_max = tmp_max > tmp ? tmp_max : tmp ; 
                }
                min_vec[1]=min_vec[1] < tmp_min ? min_vec[1] : tmp_min;
                max_vec[1]=max_vec[1] > tmp_max ? max_vec[1] : tmp_max;

            }
            delete volume;
            }
        }
//    }
#ifndef CPU_VER
    VIS_MODULE_TIMER_STA( 19 );
    MPI_Allreduce( MPI_IN_PLACE, min_vec.data(), nvariablep2, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, max_vec.data(), nvariablep2, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
    VIS_MODULE_TIMER_END( 19 );
#endif
      }
      if( clntMes.m_color_data_sampling_method == jpv::DataDefines::Constant )
      {
                min_vec[0]=0;
                max_vec[0]=1;
      }
      if( clntMes.m_size_sampling_method == jpv::DataDefines::Constant )
      {
                min_vec[1]=0;
                max_vec[1]=1;
      }

  clntMes.m_glyph_color_max = max_vec[0] ;
  clntMes.m_glyph_color_min = min_vec[0] ;
  clntMes.m_glyph_size_max  = max_vec[1] ;
  clntMes.m_glyph_size_min  = min_vec[1] ;
#endif
}


inline float CalculateDensityFactor( const Argument& param,
                                     const MultiVolumeProperty& mvp,
                                     const vismodule::Camera& camera )
{
    namespace Generator = vismodule::CellByCellParticleGenerator;
    //vismodule::UnstructuredVolumeObject* volume;
    vismodule::VolumeObjectBase* volume;
    double total_volume = 0.0;
    float great_density;
    int steps = mvp.m_start_step;
    int subvols = 0;
#ifndef CPU_VER
    int rank;
    int nprocs;
    MPI_Comm_size( MPI_COMM_WORLD, &nprocs );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
#else
    int rank = 0;
    int nprocs = 1;
#endif

    if ( rank == 0 )
    {
        VIS_MODULE_TIMER_STA( 16 );
        volume = CreateVolumeData( param, mvp, steps, subvols );
        VIS_MODULE_TIMER_END( 16 );
        VIS_MODULE_TIMER_STA( 17 );
        total_volume += Generator::CalculateTotalVolume( *volume );
        VIS_MODULE_TIMER_END( 17 );
        VIS_MODULE_TIMER_STA( 18 );
        great_density = Generator::CalculateGreatDensity( camera, *volume, param.m_subpixel_level,
                                                          param.m_sampling_step );
        VIS_MODULE_TIMER_END( 18 );

        delete volume;
    }
#ifndef CPU_VER
    VIS_MODULE_TIMER_STA( 19 );
    MPI_Bcast( &great_density, 1, MPI_FLOAT, 0, MPI_COMM_WORLD );
    VIS_MODULE_TIMER_END( 19 );
#endif


    for ( subvols = subvols + 1; subvols < mvp.m_number_subvolumes; subvols++ )
    {
        if ( subvols % nprocs == rank )
        {
            VIS_MODULE_TIMER_STA( 16 );
            volume = CreateVolumeData( param, mvp, steps, subvols );
            VIS_MODULE_TIMER_END( 16 );
            VIS_MODULE_TIMER_STA( 17 );
            total_volume += Generator::CalculateTotalVolume( *volume );
            VIS_MODULE_TIMER_END( 17 );

            delete volume;
        }
    }
#ifndef CPU_VER
    VIS_MODULE_TIMER_STA( 19 );
    MPI_Allreduce( MPI_IN_PLACE, &total_volume, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
    VIS_MODULE_TIMER_END( 19 );
#endif

    float total_nparticles = great_density * static_cast<float>( total_volume );
    float m_density_factor = static_cast<float>( param.m_particle_limit ) * 1000000 / total_nparticles;

    if ( m_density_factor > 1.0 ) m_density_factor = 1.0;

    return m_density_factor;
}

VariableRange  setVariablerange2(const float* tmp_max, const float* tmp_min, const int cnt )
{
    VariableRange vr;
    
    for ( int tf = 0; tf < cnt; tf++ )
    {
        std::stringstream ss; 
        ss << (tf + 1); 
        const std::string idxbuf = ss.str();
        vr.setValue( "t" + idxbuf + "_var_o", tmp_max[2*tf  ]);
        vr.setValue( "t" + idxbuf + "_var_o", tmp_min[2*tf  ]);
        vr.setValue( "t" + idxbuf + "_var_c", tmp_max[2*tf+1]);
        vr.setValue( "t" + idxbuf + "_var_c", tmp_min[2*tf+1]);
    }   
    return vr;
}

#endif
