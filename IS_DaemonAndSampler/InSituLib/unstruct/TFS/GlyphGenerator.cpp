#include "GlyphGenerator.h"

//GlyphGenerator::GlyphGenerator( GlyphMode mode, jpv::DataDefines size_DataDefines, jpv::DataDefines color_DataDefines,
//           int  stride, int  seed, int number_of_sampling_point,
//           Type** values, int nvariables,
//           float* coordinates, int ncoords,
//           unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltype,
//           pbvr::TransferFunction& tf, TransferFunctionSynthesizer* tfs)  :
//    m_values( values ), m_nvariable(nvariables), m_size_DataDefines(size_DataDefines),m_color_DataDefines(color_DataDefines), 
//    m_coords( coordinates  ), m_ncoords( ncoords ),
//    m_connections( connections ), m_ncells( ncells ), m_tfs(tfs),
//    m_tf(tf)
//{
//
//    if( mode == GlyphMode:: All_points || mode == GlyphMode:: Every_points )
//    {
//        this->PointSampling( stride);
//    }
//    else if(mode == GlyphMode:: Uniform_distribution)
//    {
//        std::vector< std::vector< pbvr::CellBase<Type>* > >  interp;
//        DistributionSampling( number_of_sampling_point, seed, celltype );
//    }
//}

GlyphGenerator::GlyphGenerator(glyph_parameters &glyphParameter ,Type** values, int nvariables,
//GlyphGenerator::GlyphGenerator(pbvr_parameters& particleBase, const int time_step ,Type** values, int nvariables,
           float* coordinates, int ncoords,
           unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltype,
           TransferFunctionSynthesizer* tfs ) :
    m_values( values ), m_nvariable(nvariables), m_size_DataDefines(glyphParameter.m_size_sampling_method),m_color_DataDefines(glyphParameter.m_color_sampling_method), 
    m_coords( coordinates  ), m_ncoords( ncoords ), m_stride(glyphParameter.m_stride),
    m_connections( connections ), m_ncells( ncells ), m_tfs(tfs)
{

    //this -> SetGlyphParameter( particleBase, time_step ); 

    this -> CalculateMinMax(); // 絶対値のmin,max
//    this -> NormalizeValues();

    if( glyphParameter.m_distribution_modes == jpv::GlyphMode:: AllPoints || glyphParameter.m_distribution_modes == jpv::GlyphMode:: EveryNthPoints )
    {
        //this->PointSampling( m_stride);
        this->PointSampling( glyphParameter);
    }
    else if(glyphParameter.m_distribution_modes == jpv::GlyphMode:: UniformDistribution)
    {
        this->DistributionSampling( glyphParameter, celltype );
    }
}

#if 0
bool GlyphGenerator::SetGlyphParameter( pbvr_parameters& particleBase, const int time_step )
{
    std::string visParamDir;
    std::string glyphParamPath;
    std::string glyphFilePath;

    const char *envBuf = NULL;
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
    envBuf = std::getenv( "PARTICLE_DIR" );
    if (envBuf == NULL) {
        glyphFilePath = "./g_";
    }
    else {
        glyphFilePath = envBuf;
        if (glyphFilePath[glyphFilePath.size() - 1] != '/') {
            glyphFilePath += "/g_";
        }
        else {
            glyphFilePath += "g_";
        }
    }

    glyphParamPath = visParamDir + "parameter.gly";
   
    m_glyphParamPath = glyphParamPath;
    m_glyphFilePath = glyphFilePath;

    GlyphProperty glyph_property;

    glyph_property.LoadIN(glyphParamPath) ;

    bool glyph_flag;
    std::string              g_flag                = glyph_property.getString( "GLYPH_FLAG" );
    std::vector<std::string> direction_variables   = glyph_property.getTableString( "DIRECTION_VARIABLES" );
    std::string              size_sampling_method  = glyph_property.getString("SIZE_SAMPLING_METHOD");
    std::vector<std::string> size_variables        = glyph_property.getTableString( "SIZE_VARIABLES" );
    std::string distribution_modes                 = glyph_property.getString("DISTRIBUTION_MODE");
    int stride                                     = glyph_property.getInt("STRIDE");
    int seed                                       = glyph_property.getInt("SEED");
    int number_of_sample_points                    = glyph_property.getInt("NUMBER_OF_SMAPLING_POINT");
    std::string color_sampling_method              = glyph_property.getString("COLOR_DATA_SAMPLING_METHOD");
    std::vector<std::string> color_data_variables  = glyph_property.getTableString( "COLOR_VARIABLES" );
    

    float glyph_min=0; 
    float glyph_max=0;
    glyph_min = glyph_property.getFloat("GLYPH_COLOR_MIN");
    glyph_max = glyph_property.getFloat("GLYPH_COLOR_MAX");
    std::vector<int> i_table;
    i_table = glyph_property.getTableInt( "GLYPH_COLOR_MAP_TABLE" );
    kvs::ValueArray<kvs::UInt8> u_table( i_table.size() );
    for( size_t j = 0; j<i_table.size(); j++ ) u_table[j] = (kvs::UInt8)i_table[j];
    kvs::ColorMap color_map( u_table, glyph_min, glyph_max);

    m_color_map = color_map;
    if(strcmp(g_flag.c_str(), "TRUE") ==0 ) glyph_flag = true;
    else glyph_flag = false;
   
    if(direction_variables.size() < 3)
    { 
        std::cout << "variables number is less 3 numbers !!! Skip glyph generate process !!!" << std::endl;
        return false;  
    }
    for (int i = 0; i< 3 ; i++)
    {
        m_direction_variables.push_back ( std::atoi(direction_variables[i].substr(1).c_str()) - 1);
    }

    if     (size_sampling_method == "Constant"       ) m_size_sampling_method    = jpv::DataDefines::Constant;
    else if(size_sampling_method == "SingleVariable" ) m_size_sampling_method    = jpv::DataDefines::SingleVariable;
    else if(size_sampling_method == "VariableArray" )  m_size_sampling_method    = jpv::DataDefines::VariableArray;
    else 
    {
       std::cout << "No selecting Sampling method !!! Skip glyph generate process !!!" << std::endl;
       return false;  
    }

#if 1
    for (int i =0 ; i< size_variables.size(); i++)
    {
        glyphParameter.m_size_variables.push_back( std::atoi(size_variables[i].substr(1).c_str()) -1); 
    }

    if     (distribution_modes == "AllPoints"           ) glyphParameter.m_distribution_modes  = jpv::GlyphMode::AllPoints;
    else if(distribution_modes == "EveryNthPoints"         ) glyphParameter.m_distribution_modes  = jpv::GlyphMode::EveryNthPoints;
    else if(distribution_modes == "UniformDistribution" ) glyphParameter.m_distribution_modes  = jpv::GlyphMode::UniformDistribution;
    else 
    {
       std::cout << "Not selecting Distribution mode !!! Skip glyph generate process !!!" << std::endl;
       return false;  
    }

    glyphParameter.m_stride                  = stride;
    glyphParameter.m_seed                    = seed;
    glyphParameter.m_number_of_sample_points = number_of_sample_points;
    if     (color_sampling_method == "Constant"       ) glyphParameter.m_color_sampling_method    = jpv::DataDefines::Constant;
    else if(color_sampling_method == "SingleVariable" ) glyphParameter.m_color_sampling_method    = jpv::DataDefines::SingleVariable;
    else if(color_sampling_method == "VariableArray" ) glyphParameter.m_color_sampling_method    = jpv::DataDefines::VariableArray;
    else 
    {
       std::cout << "No selecting Sampling method !!! Skip glyph generate process !!!" << std::endl;
       return false;  
    }
    
    for (int i =0 ; i< color_data_variables.size(); i++)
    {
        glyphParameter.m_color_data_variables.push_back( std::atoi(color_data_variables[i].substr(1).c_str()) - 1); 
    }

    std::cout << "glyphParameter.m_direction_variables        = " << glyphParameter.m_direction_variables[0] << ", " << glyphParameter.m_direction_variables[1]   << std::endl; 
    std::cout << "glyphParameter.m_size_sampling_method       = " << static_cast<int>(glyphParameter.m_size_sampling_method)      << std::endl; 
    std::cout << "glyphParameter.m_size_variables             = " << glyphParameter.m_size_variables[0]  << ", "  << glyphParameter.m_size_variables[1]       << std::endl; 
    std::cout << "glyphParameter.m_distribution_modes         = " << static_cast<int>(glyphParameter.m_distribution_modes )       << std::endl; 
    std::cout << "glyphParameter.m_stride                     = " << glyphParameter.m_stride                    << std::endl; 
    std::cout << "glyphParameter.m_seed                       = " << glyphParameter.m_seed                      << std::endl; 
    std::cout << "glyphParameter.m_number_of_sample_points    = " << glyphParameter.m_number_of_sample_points   << std::endl; 
    std::cout << "glyphParameter.m_color_sampling_method      = " << static_cast<int>(glyphParameter.m_color_sampling_method )    << std::endl; 
    std::cout << "glyphParameter.m_color_data_variables       = " << glyphParameter.m_color_data_variables[0] << ", " << glyphParameter.m_color_data_variables[1]       << std::endl; 
#endif 
      return glyph_flag; 

}
#endif

void GlyphGenerator::PointSampling( glyph_parameters &glyphParameter)
{
    const int stride = glyphParameter.m_stride;
    int nPoints = m_ncoords/stride ;
    m_glyph_coords.resize(nPoints * 3);
    m_glyph_vectors.resize(nPoints * 3);
    m_glyph_sizes.resize(nPoints);
    m_glyph_colors_data.resize( nPoints );
    m_glyph_colors.resize( nPoints * 3); 

    glyphParameter.m_glyph_coords.resize(nPoints * 3);
    glyphParameter.m_glyph_vectors.resize(nPoints * 3);
    glyphParameter.m_glyph_sizes.resize(nPoints);
    glyphParameter.m_glyph_colors_data.resize( nPoints );
    glyphParameter.m_glyph_colors.resize( nPoints * 3); 

   //vertex by vertexでglyph_coord、glyph_vector、glyph_size、glyph_color_data、glyph_color
   //stabデータとして vector = 1,2,3, size = 4 , color_data =5 とする。

    // size 
   std::vector<int> vector_var = glyphParameter.m_direction_variables;
   int glyph_count =0;
   for (int i=0;i < m_ncoords; i+= stride)
   {
      glyphParameter.m_glyph_coords[3*glyph_count    ] = m_coords[3*i    ];
      glyphParameter.m_glyph_coords[3*glyph_count +1 ] = m_coords[3*i +1 ];
      glyphParameter.m_glyph_coords[3*glyph_count +2 ] = m_coords[3*i +2 ];

      glyphParameter.m_glyph_vectors[3*glyph_count    ] = m_values[vector_var[0]][i ];
      glyphParameter.m_glyph_vectors[3*glyph_count +1 ] = m_values[vector_var[1]][i ];
      glyphParameter.m_glyph_vectors[3*glyph_count +2 ] = m_values[vector_var[2]][i ];
      glyph_count++;
   }

   //size
   if (m_size_DataDefines == jpv::DataDefines::Constant)
   {
//       fill(m_glyph_sizes.begin()      , m_glyph_sizes.begin()      , 1);
       std::fill(glyphParameter.m_glyph_sizes.begin(), glyphParameter.m_glyph_sizes.begin() ,1);
   }
//   else if (m_size_DataDefines == jpv::DataDefines::SingleVariable) 
//   {
//       int glyph_count =0;
//       //float size_min = m_size_min[glyphParameter.m_size_variables[0]] < 0 ? 0 : m_size_min[glyphParameter.m_size_variables[0]];
//       //float size_max = m_size_max[glyphParameter.m_size_variables[0]] < 0 ? 0 : m_size_max[glyphParameter.m_size_variables[0]];
//
//       for (int i = 0 ; i < m_ncoords; i+= stride)
//       {
//           float ss = ( kvs::Math::Abs(m_values[glyphParameter.m_size_variables[0]][ i ]) - m_size_min[glyphParameter.m_size_variables[0]])/( m_size_max[glyphParameter.m_size_variables[0]] - m_size_min[glyphParameter.m_size_variables[0]] );
//           //float ss = kvs::Math::Abs(m_values[glyphParameter.m_size_variables[0]][ i ]) /( m_size_max[glyphParameter.m_size_variables[0]] );
//           glyphParameter.m_glyph_sizes[ glyph_count]       = ss;
//           //glyphParameter.m_glyph_sizes[ glyph_count]       = kvs::Math::Abs(m_values[glyphParameter.m_size_variables[0]][ i ]);
//           glyph_count++;
//       }
//   }
   else if (  m_size_DataDefines == jpv::DataDefines::VariableArray || m_size_DataDefines == jpv::DataDefines::SingleVariable ) 
   {
       std::vector<float> tmp_size(nPoints);
       std::vector<int> size_var = glyphParameter.m_size_variables;
       int n_size_data=size_var.size();
       float max = 0;
       float min = 0;
       for(int k = 0 ; k< n_size_data ; k++)
       {
           max += kvs::Math::Square( m_size_max[glyphParameter.m_size_variables[k]] ) ;
           min += kvs::Math::Square( m_size_min[glyphParameter.m_size_variables[k]] ) ;
       }
       float factor =0;
       if (std::sqrt(max) - std::sqrt(min) > 1e-6 ) 
       {
        factor = 1/ (std::sqrt(max) - std::sqrt(min)) ;
       }
       else  
       {
        factor = 1;
       }

       int glyph_count =0;
       for (int i=0;i < m_ncoords; i+= stride)
       {
           for(int k = 0 ; k< n_size_data ; k++)
           {
               tmp_size[ glyph_count ] += kvs::Math::Square( m_values[ size_var[k] ][i] ); 
           }
//           glyphParameter.m_glyph_sizes[ glyph_count ] = std::sqrt(tmp_size[ glyph_count ] - m_size_min[glyphParameter.m_size_variables[0]] )/( m_size_max[glyphParameter.m_size_variables[0]] - m_size_min[glyphParameter.m_size_variables[0]] );
           glyphParameter.m_glyph_sizes[ glyph_count ] = (std::sqrt(tmp_size[ glyph_count ]) - std::sqrt(min) )*factor;
           glyph_count++;
       }
   }

   //color 
   float glyph_color_data_max = FLT_MIN;
   float glyph_color_data_min = FLT_MAX;
   if (m_color_DataDefines == jpv::DataDefines::Constant)
   {
       std::fill(glyphParameter.m_glyph_colors.begin(), glyphParameter.m_glyph_colors.begin(), 0);
   }
   else if (m_color_DataDefines == jpv::DataDefines::SingleVariable) 
   {

       std::vector<int> color_var = glyphParameter.m_color_data_variables;
       glyph_color_data_max =  m_color_max[color_var[0]] ;
       glyph_color_data_min =  m_color_min[color_var[0]] ;
       int glyph_count =0;
       for (int i = 0; i < m_ncoords; i+= stride)
       {
           glyphParameter.m_glyph_colors_data[ glyph_count ] = m_values[ color_var[0] ][ i ];
           glyph_count++;
            
       }
   }
   else if (m_color_DataDefines == jpv::DataDefines::VariableArray) 
   {
       std::vector<int> color_var = glyphParameter.m_color_data_variables;
       int n_color_data=color_var.size();
       for(int k = 0 ; k< n_color_data ; k++)
       {
           glyph_color_data_max +=  kvs::Math::Square(m_color_max[color_var[k]]) ;
           glyph_color_data_min +=  kvs::Math::Square(m_color_min[color_var[k]]) ;
       }
       glyph_color_data_max = std::sqrt(glyph_color_data_max)  ;
       glyph_color_data_min = std::sqrt(glyph_color_data_max)  ;

       int glyph_count =0;
       std::vector<float> tmp_size(nPoints);
       for (int i = 0; i < m_ncoords; i+= stride)
       {
           for(int k = 0 ; k< n_color_data ; k++)
           {
            tmp_size[ glyph_count ] += kvs::Math::Square(m_values[color_var[k]][ i ]) ;
           }
           glyphParameter.m_glyph_colors_data[ glyph_count ] = std::sqrt(tmp_size[ glyph_count ]);
           glyph_count++;
       }
   }

        glyphParameter.m_color_map.setRange(glyph_color_data_min,glyph_color_data_max);
    for (int ii=0;ii < nPoints; ii++)
    {
        kvs::RGBColor colors; 
        colors = glyphParameter.m_color_map.at(glyphParameter.m_glyph_colors_data[ ii ]);
        glyphParameter.m_glyph_colors[3*ii    ] = colors.r() ;
        glyphParameter.m_glyph_colors[3*ii +1 ] = colors.g() ;
        glyphParameter.m_glyph_colors[3*ii +2 ] = colors.b() ;
    }
    this -> show(glyphParameter);
}

//void GlyphGenerator::DistributionSampling(int TotalNumbers, int seed, const pbvr::VolumeObjectBase::CellType& celltype)
void GlyphGenerator::DistributionSampling(glyph_parameters &glyphParameter, const pbvr::VolumeObjectBase::CellType& celltype)
{
    //const int TotalNumbers  = glyphParameter.m_number_of_sample_points;
    const int seed          = glyphParameter.m_seed;
    int nPoints = glyphParameter.m_number_of_sample_points; 
    m_glyph_coords.resize(nPoints * 3);
    m_glyph_vectors.resize(nPoints * 3);
    m_glyph_sizes.resize(nPoints);
    m_glyph_colors_data.resize( nPoints );
    m_glyph_colors.resize( nPoints * 3); 

    m_glyph_colors_data.clear();
//    glyphParameter.m_glyph_coords.resize(nPoints * 3);
//    glyphParameter.m_glyph_vectors.resize(nPoints * 3);
//    glyphParameter.m_glyph_sizes.resize(nPoints);
//    glyphParameter.m_glyph_colors.resize( nPoints * 3); 

#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif

    int mpi_rank = 0;

    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );

    //if(mpi->rank==0)std::cout<<"start generate_particles\n";
    static bool start_flag = true;
    static bool parameter_file_opened=false;
    kvs::Timer timer( kvs::Timer::Start );

    std::vector< std::vector< pbvr::CellBase<Type>* > >  interp;
    interp.resize( max_threads );

    switch ( celltype )
    {
        case pbvr::VolumeObjectBase::Tetrahedra:
            {
                if (mpi_rank == 0) std::cout << "celltype: tetrahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( m_nvariable );
                    for ( int j = 0; j < m_nvariable; j++ )
                    {
                        interp[i][j]  = new pbvr::TetrahedralCell<Type>( m_values[j], m_coords, m_ncoords, m_connections, m_ncells );
                    }
                }
                break;
            }
        case pbvr::VolumeObjectBase::QuadraticTetrahedra:
            {
                std::cout << "Cell type : Quadratic tetrahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( m_nvariable );
                    for ( int j = 0; j < m_nvariable; j++ )
                    {
                        interp[i][j]  = new pbvr::QuadraticTetrahedralCell<Type>( m_values[j], m_coords, m_ncoords, m_connections, m_ncells );
                    }
                }
                break;
            }
        case pbvr::VolumeObjectBase::Hexahedra:
            {
                std::cout << "Cell type : Hexahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( m_nvariable  );
                    for ( int j = 0; j < m_nvariable; j++ )
                    {
                        interp[i][j]  = new pbvr::HexahedralCell<Type>( m_values[j], m_coords, m_ncoords, m_connections, m_ncells );
                    }
                }
                break;
            }
        case pbvr::VolumeObjectBase::QuadraticHexahedra:
            {
                std::cout << "Cell type : Quadratic hexahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( m_nvariable );
                    for ( int j = 0; j < m_nvariable; j++ )
                    {
                        interp[i][j]  = new pbvr::QuadraticHexahedralCell<Type>( m_values[j], m_coords, m_ncoords, m_connections, m_ncells );
                    }
                }
                break;
            }
        case pbvr::VolumeObjectBase::Prism:
            {
                if (mpi_rank == 0) std::cout << "celltype: Prism " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( m_nvariable );
                    for ( int j = 0; j < m_nvariable; j++ )
                    {
                        interp[i][j]  = new pbvr::PrismaticCell<Type>( m_values[j], m_coords, m_ncoords, m_connections, m_ncells );
                    }
                }
                break;
            }
        case pbvr::VolumeObjectBase::Pyramid:
            {
                if (mpi_rank == 0) std::cout << "celltype: Pyramid" << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( m_nvariable );
                    for ( int j = 0; j < m_nvariable; j++ )
                    {
                        interp[i][j]  = new pbvr::PyramidalCell<Type>( m_values[j], m_coords, m_ncoords, m_connections, m_ncells );
                    }
                }
                break;
            }
        default:
            {
                //BaseClass::m_is_success = false;
                //kvsMessageError( "Unsupported cell type." );
                std::cout << "Unsupported cell type." << std::endl; 
                return;
            }
       }

    #pragma omp parallel
    {
#if _OPENMP
        int nthreads = omp_get_num_threads();
        int thid     = omp_get_thread_num();
#else
        int nthreads = 1;
        int thid     = 0;
#endif

        timer.start();

        kvs::MersenneTwister MT( seed );

    float TotalVolume = 0;
    float density = 0;
        // 動的な粒子データ配列
        std::vector<float> th_glyph_coords;
        std::vector<float> th_glyph_sizes;
        std::vector<float> th_glyph_colors_data;
        std::vector<Byte>  th_glyph_colors;
//        std::vector<float> th_vertex_normals;
        std::vector<float> th_glyph_vectors;
        std::vector<float> th_glyph_size;

       
        TransferFunctionSynthesizer** th_tfs = new TransferFunctionSynthesizer*[max_threads];
        std::vector< std::vector<pbvr::TransferFunction> > th_tf;

        for ( int n = 0; n < max_threads; n++ )
        {
            th_tfs[n] = new TransferFunctionSynthesizer( *m_tfs );
        }

        th_tf.resize( max_threads );
        for ( int i = 0; i < max_threads; i++ )
        {
            th_tf[ i ].resize( 1 );
            th_tf[i][0]= m_tf;
        }

        // -----------------------------------
        //配列の追加
        kvs::Vector3f local_center_array[ SIMD_BLK_SIZE ];
        kvs::Vector3f global_center_array[ SIMD_BLK_SIZE ];
        kvs::UInt32 cell_index[ SIMD_BLK_SIZE ];

        float cell_opacity_array[ SIMD_BLK_SIZE ];
        int nglyphs_array[ SIMD_BLK_SIZE ];

        kvs::Vector3f local_coord_array[ SIMD_BLK_SIZE ];
        kvs::Vector3f global_coord_array[ SIMD_BLK_SIZE ];
        kvs::Vector3f glyph_vector_array[ SIMD_BLK_SIZE ];
        kvs::Vector3f glyph_size_array[ SIMD_BLK_SIZE ];
        float density_array[ SIMD_BLK_SIZE ];

        
        kvs::Vector3f l_plus_coord[ SIMD_BLK_SIZE ];
        kvs::Vector3f l_minus_coord[ SIMD_BLK_SIZE ];
        kvs::Vector3f g_plus_coord[ SIMD_BLK_SIZE ];
        kvs::Vector3f g_minus_coord[ SIMD_BLK_SIZE ];
        float S_plus_opacity[ SIMD_BLK_SIZE ];
        float S_minus_opacity[ SIMD_BLK_SIZE ];
        float dsdx_array[ SIMD_BLK_SIZE ];
        float dsdy_array[ SIMD_BLK_SIZE ];
        float dsdz_array[ SIMD_BLK_SIZE ];

        kvs::Vector3f grad_array[ SIMD_BLK_SIZE ];
        kvs::RGBColor colors_array[ SIMD_BLK_SIZE ];
        float sizes_array[ SIMD_BLK_SIZE ];
        float colors_data_array[ SIMD_BLK_SIZE ];
        // -----------------------------------

#pragma omp for schedule( dynamic ) nowait
        for( int cell_base = 0; cell_base < m_ncells; cell_base += SIMD_BLK_SIZE )
        {

            int remain = ( m_ncells - cell_base > SIMD_BLK_SIZE )? SIMD_BLK_SIZE: m_ncells - cell_base;
            //一括でセルをバインドするための配列と、座標の取得
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell_index[cell_BLK] = (kvs::UInt32)(cell_base + cell_BLK);
                local_center_array[cell_BLK] = interp[thid][0]->localGravityPoint();
            }


            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                interp[thid][0]->bindCell( cell_index[cell_BLK] );
                TotalVolume += interp[thid][0]->volume();
            }
        }

        density =nPoints/TotalVolume;

        //粒子生成ループ開始
#pragma omp for schedule( dynamic ) nowait
        for( int cell_base = 0; cell_base < m_ncells; cell_base += SIMD_BLK_SIZE )
        {
           //ブロック内でのループ回数を取得
            int remain = ( m_ncells - cell_base > SIMD_BLK_SIZE )? SIMD_BLK_SIZE: m_ncells - cell_base;

        /////////////////////////////// Synthesized~ (), CalculateOpacity() ///////////////////////////////////
            //一括でセルをバインドするための配列と、座標の取得
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell_index[cell_BLK] = (kvs::UInt32)(cell_base + cell_BLK);
                local_center_array[cell_BLK] = interp[thid][0]->localGravityPoint();
            }

            //補間器にセルを一括でバインド
            for(int i = 0; i < m_nvariable; i++)
            {
                interp[thid][i]->bindCellArray(remain, cell_index);
            }

            interp[thid][0]->setLocalPointArray( remain, local_center_array );
            interp[thid][0]->transformLocalToGlobalArray( remain,
                                                          local_center_array,
                                                          global_center_array );

             //生成粒子数を計算
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                    nglyphs_array[cell_BLK] 
                        = calculate_number_of_particles( density, interp[thid][0]->volume(), &MT ) ;

            }
        /////////////////////////////// Synthesized~ (), CalculateOpacity() ///////////////////////////////////
        /////////////////////////////// CalculateOpacity(), CalculateColor() ///////////////////////////////////
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                // ------------------------------------------------

                for( int i = 0; i < nglyphs_array[cell_BLK]; i+=SIMD_BLK_SIZE )
                {
                    //ブロック内でのループ回数を取得
                    int remain_BLK = ( nglyphs_array[cell_BLK] - i > SIMD_BLK_SIZE )
                                                        ? SIMD_BLK_SIZE: nglyphs_array[cell_BLK] - i;
                    int nparticles_count = 0;
                    //一括でセルをバインドするための配列と、座標の取得
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        cell_index[j] = cell_base + cell_BLK;
                        local_coord_array[j] = interp[thid][0] -> randomSampling_MT( &MT );
                        //local_coord_array[j] = kvs::Vector3f ( 0.5, 0.5, 0.5 );;

                        //補間器にセルを一括でバインド
                        for( int k = 0; k < m_nvariable; k++ )
                        {
                            interp[thid][k]->bindCell( cell_index[j] );
                        }

                        interp[thid][0]->setLocalPoint( local_coord_array[j] );
                        global_coord_array[j] = interp[thid][0]->transformLocalToGlobal( local_coord_array[j] );

                    // 座標情報の格納
                                cell_index[ nparticles_count ] = cell_index[j];
                                local_coord_array[ nparticles_count ] = local_coord_array[j];
                                global_coord_array[ nparticles_count ] = global_coord_array[j];
                                nparticles_count +=1;
                }

// ------------------------------------------------

                    for( int j = 0; j < m_nvariable; j++ )
                    {
                        interp[thid][j]->bindCellArray( nparticles_count, cell_index );
                    }

                    //サイズ計算
                    if( m_size_DataDefines == jpv::DataDefines::Constant )
                    {
                        for( int j = 0; j < nparticles_count; j++ )
                        {
                            sizes_array[j] = 1;
                        }
                    }
                    else if (m_size_DataDefines == jpv::DataDefines::SingleVariable || m_size_DataDefines == jpv::DataDefines::VariableArray )
                    { 
                        th_tfs[thid]->CalculateGlyphSizeArray( interp[thid],
                                nparticles_count,
                                local_coord_array,
                                global_coord_array,
                                th_tf[thid],
                                glyphParameter,
                                sizes_array );

                    }

                    //色の計算
                    if( m_color_DataDefines ==  jpv::DataDefines::Constant )
                    {
                        for( int j = 0; j < nparticles_count; j++ )
                        {
                              colors_array[j].set(0,0,0);
                        }
                    }
                    else if (m_color_DataDefines ==  jpv::DataDefines::SingleVariable || m_size_DataDefines == jpv::DataDefines::VariableArray )
                    { 
//                        std::vector<int> color_var = glyphParameter.m_color_data_variables;
//                        int n_color_data=color_var.size();
//                        float glyph_color_data_max = FLT_MIN;
//                        float glyph_color_data_min = FLT_MAX;
//                        for(int k = 0 ; k< n_color_data ; k++)
//                        {
//                            glyph_color_data_max +=  kvs::Math::Square(m_color_max[color_var[k]]) ;
//                            glyph_color_data_min +=  kvs::Math::Square(m_color_min[color_var[k]]) ;
//                        }
//                        glyph_color_data_max = std::sqrt(glyph_color_data_max)  ;
//                        glyph_color_data_min = std::sqrt(glyph_color_data_max)  ;
//                        glyphParameter.m_color_map.setRange(glyph_color_data_min,glyph_color_data_max);


                        th_tfs[thid]->CalculateGlyphColorArray( interp[thid],
                                nparticles_count,
                                local_coord_array,
                                global_coord_array,
                                th_tf[thid],
                                glyphParameter,
                                colors_data_array );
                    }
                    // glyph_vectorの計算
                    th_tfs[thid]->CalculateGlyphVector( interp[thid],
                                                       nparticles_count,
                                                       local_coord_array,
                                                       global_coord_array,
                                                       glyphParameter,
                                                       glyph_vector_array );
 

                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        th_glyph_coords.push_back( global_coord_array[j].x() );
                        th_glyph_coords.push_back( global_coord_array[j].y() );
                        th_glyph_coords.push_back( global_coord_array[j].z() );

                        th_glyph_colors.push_back((int) colors_array[j].r() );
                        th_glyph_colors.push_back((int) colors_array[j].g() );
                        th_glyph_colors.push_back((int) colors_array[j].b() );

                        th_glyph_colors_data.push_back( colors_data_array[j] );
                        th_glyph_sizes.push_back( sizes_array[j] );

                        th_glyph_vectors.push_back( glyph_vector_array[j].x() );
                        th_glyph_vectors.push_back( glyph_vector_array[j].y() );
                        th_glyph_vectors.push_back( glyph_vector_array[j].z() );
                    }
                // ------------------------------------------------
                
                }//end of for i
            }
        /////////////////////////////// CalculateOpacity(), CalculateColor() ///////////////////////////////////
        }// end of for cell
//            std::cout << __FUNCTION__  << ": " << __LINE__ << std::endl;
        #pragma omp barrier
        #pragma omp critical
        {
            glyphParameter.m_glyph_coords.insert  ( glyphParameter.m_glyph_coords.end(), th_glyph_coords.begin(), th_glyph_coords.end() );
            glyphParameter.m_glyph_colors.insert  ( glyphParameter.m_glyph_colors.end(), th_glyph_colors.begin(), th_glyph_colors.end() );
            m_glyph_colors_data.insert  ( m_glyph_colors_data.end(), th_glyph_colors_data.begin(), th_glyph_colors_data.end() );
            glyphParameter.m_glyph_sizes.insert   ( glyphParameter.m_glyph_sizes.end(),  th_glyph_sizes.begin(),  th_glyph_sizes.end() );
            glyphParameter.m_glyph_vectors.insert ( glyphParameter.m_glyph_vectors.end(), th_glyph_vectors.begin(), th_glyph_vectors.end() );
        }

       this->show(glyphParameter); 
     } //#pragma omp parallel

//    glyphParameter.m_glyph_sizes;
    if (m_size_DataDefines == jpv::DataDefines::SingleVariable || m_size_DataDefines == jpv::DataDefines::VariableArray )
    { 
        int n_size_data=glyphParameter.m_glyph_sizes.size();
        float max=FLT_MIN;
        float min=FLT_MAX;
        for(int k = 0; k< n_size_data; k++)
        {
            max = kvs::Math::Max(glyphParameter.m_glyph_sizes[k], max ); 
            min = kvs::Math::Min(glyphParameter.m_glyph_sizes[k], min ); 
        }

        MPI_Allreduce( MPI_IN_PLACE, &min, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, &max, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );

        float factor = 0;
        //if (std::sqrt(max) - std::sqrt(min) > 1e-6)
        if (max - min > 1e-6)
        {
            //factor = 1.f/ (std::sqrt(max) - std::sqrt(min) ) ;
            factor = 1.f/ (max - min)  ;
            for( int j = 0; j < n_size_data; j++ )
            {
                //glyphParameter.m_glyph_sizes[j] = (std::sqrt(glyphParameter.m_glyph_sizes[j]) - std::sqrt(min))*factor;
                glyphParameter.m_glyph_sizes[j] = (glyphParameter.m_glyph_sizes[j] - min)*factor;
            }
        }
        else
        {
            for( int j = 0; j < n_size_data; j++ )
            {
                glyphParameter.m_glyph_sizes[j] = 1.f;
            }
        }
    }

    if (m_color_DataDefines == jpv::DataDefines::SingleVariable || m_color_DataDefines == jpv::DataDefines::VariableArray )
    { 
        //    glyphParameter.m_glyph_color;
        int n_color_data=m_glyph_colors_data.size();
        float max=FLT_MIN;
        float min=FLT_MAX;
        for(int k = 0; k< n_color_data; k++)
        {
            max = kvs::Math::Max(m_glyph_colors_data[k], max ); 
            min = kvs::Math::Min(m_glyph_colors_data[k], min ); 
        }

        MPI_Allreduce( MPI_IN_PLACE, &min, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, &max, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );

        glyphParameter.m_color_map.setRange(min,max);
        for( int jx=0; jx<n_color_data; jx++)
        {
            kvs::RGBColor color;
            color = glyphParameter.m_color_map.at( m_glyph_colors_data[jx] );
            glyphParameter.m_glyph_colors[3*jx     ] = color.r();
            glyphParameter.m_glyph_colors[3*jx + 1 ] = color.g();
            glyphParameter.m_glyph_colors[3*jx + 2 ] = color.b();
        }

    }

}

const size_t GlyphGenerator::calculate_number_of_particles(
    const float density,
    const float volume_of_cell,
    kvs::MersenneTwister* MT )
{
    const float N = density * volume_of_cell;
    const float R = MT->rand();

    size_t n = static_cast<size_t>( N );
    if ( N - n > R )
    {
        ++n;
    }

    return ( n );
}

void GlyphGenerator::NormalizeValues()
{
    m_size_max.resize(m_nvariable);
    m_size_min.resize(m_nvariable);
    for (int k =0;k <m_nvariable; k++)
    {
        float max = FLT_MIN;
        float min = FLT_MAX;
        for(int i =0; i< m_ncoords; i+=3)
        {
            max = kvs::Math::Max(kvs::Math::Abs(m_values[k][i]), kvs::Math::Abs(m_values[k][i+1]), kvs::Math::Abs(m_values[k][i+2]), max ); 
        }
        m_size_max[k] = max;
    }

    for (int k =0;k <m_nvariable; k++)
    {
        for(int i =0; i< m_ncoords; i+=3)
        {
            m_values[k][i] /= m_size_max[k];
        }
    }

}


void GlyphGenerator::CalculateMinMax()
{
    m_color_max.resize(m_nvariable);
    m_color_min.resize(m_nvariable);
    m_size_max.resize(m_nvariable);
    m_size_min.resize(m_nvariable);
    // size
    for (int k =0;k <m_nvariable; k++)
    {
        float max = FLT_MIN;
        float min = FLT_MAX;
        for(int i =0; i< m_ncoords; i+=m_stride)
        {
            max = kvs::Math::Max(kvs::Math::Abs(m_values[k][i]), max ); 
            min = kvs::Math::Min(kvs::Math::Abs(m_values[k][i]), min ); 
        }
        m_size_min[k] = min;
        m_size_max[k] = max;
    }

    MPI_Allreduce( MPI_IN_PLACE, m_size_min.data(), m_nvariable, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, m_size_max.data(), m_nvariable, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );


    //color
    for (int k =0;k <m_nvariable; k++)
    {
        float max = FLT_MIN;
        float min = FLT_MAX;
        for(int i =0; i< m_ncoords; i+=m_stride)
        {
            max = kvs::Math::Max(m_values[k][i],max ); 
            min = kvs::Math::Min(m_values[k][i],min ); 
        }
        m_color_min[k] = min;
        m_color_max[k] = max;
    }

    MPI_Allreduce( MPI_IN_PLACE, m_color_min.data(), m_nvariable, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
    MPI_Allreduce( MPI_IN_PLACE, m_color_max.data(), m_nvariable, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );

}

void GlyphGenerator::OutputGlyph(glyph_parameters &glyphParameter, const  pbvr_parameters& particleBase, const int time_step)
{
    int mpi_rank;
    int mpi_size;
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
    int nbins = 256;

    ///-------------------------------------//
    ///--------粒子配列をファイル出力----------//
    //--------------------------------------//
    //kvs::ValueArray<float> coords( particleBase.m_sample_coords );
    //kvs::ValueArray<Byte>  colors( particleBase.m_sample_colors );
    //kvs::ValueArray<float> normals(particleBase.m_sample_normals );
    kvs::ValueArray<float> coords( glyphParameter.m_glyph_coords  );
    kvs::ValueArray<float> vectors(glyphParameter.m_glyph_vectors );
    kvs::ValueArray<Byte>  colors( glyphParameter.m_glyph_colors   );
    kvs::ValueArray<float> sizes(  glyphParameter.m_glyph_sizes  );

    static bool first_step = true;
    static MPI_Comm new_comm;
    static int count;
    static int num_nodes;

#if 0
    /* 各ノード毎に粒子データを出力する。 */
    if( first_step )
    {
        int numprocs, myrank;
        int resultlen;
        char procname[MPI_MAX_PROCESSOR_NAME];
        char* procname_bak;
        char* procname_g;
        char* procname_p;

        MPI_Comm_size( MPI_COMM_WORLD, &numprocs );
        MPI_Comm_rank( MPI_COMM_WORLD, &myrank );

        /* ノード名を取得し、各ランクで共有する. */
        MPI_Get_processor_name( procname, &resultlen );
        procname_g = new char[ MPI_MAX_PROCESSOR_NAME * numprocs ];
        MPI_Allgather( procname,   MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                       procname_g, MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                       MPI_COMM_WORLD );

        int color;
        count = 1;
        for( color = 0; color < numprocs; color++ )
        {
            procname_p = procname_g + MPI_MAX_PROCESSOR_NAME * color;

            /* 要素の隣同士を比較して差異があった場合にカウントし, *
             * ノード毎に連続した番号を割り当てる.                 */
            if( color > 0 )
            {
                procname_bak = procname_p - MPI_MAX_PROCESSOR_NAME;
                if( strcmp( procname_p, procname_bak ) != 0 )
                    count++;
            }

            /* 自分のノード名が一致した要素番号をコミュニケータ分割のcolorとする */
            if( strcmp( procname_p, procname ) == 0 )
                break;
        }

        delete[] procname_g;
        
        MPI_Comm_split( MPI_COMM_WORLD, color, myrank, &new_comm );
        
        int split_numprocs;
        MPI_Comm_size( new_comm, &split_numprocs );
        
        /*
         * 各ノードに均等にランクが割り当てられることを前提とし,
         * 分割前のプロセス数と分割後のプロセス数の非を粒子ファイル数とする.
         */
        num_nodes = numprocs / split_numprocs;
        if( numprocs % split_numprocs > 0 ) num_nodes++;
        first_step = false;
    }   
    /*
     * ファイル名の粒子データのファイル名を入力する.
     * countが各ファイルで連続でない場合,ファイルが不在と見なしてデーモンでスピンロックがかかる.
     */
#if 0
    char filename[256];
    sprintf(filename, "./jupiter_particle_out/t_%05d_",time_step);
    sprintf(filename,"%s%07d_%07d.kvsml", filename, count, num_nodes );
#else
    // 20181226 start  環境変数で指定したファイルパスを参照する
    std::stringstream ss;
    //add by shimomura 20240614
//    ss << std::setfill('0') << std::setw(2) << static_cast<int>(celltype);
//    ss << "_";
    ss << std::setfill('0') << std::setw(5) << time_step;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << count;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_rank;
    ss << ".kvsml";
    glyphParameter.m_glyphFilePath += ss.str();
    // 20181226 end
#endif

    int particle_size = coords.size();
    int *recvcounts;
    int *displs;
    int *recvcounts_size;
    int *displs_size;
    int  new_number_of_process;
    int new_rank;

//    MPI_Comm_rank( new_comm, &new_rank );
//    MPI_Comm_size( new_comm, &new_number_of_process );

    /*
     *  recvcounts: 各ランク毎の受信バッファサイズ.
     *  displs:     受信先バッファ上の各ランク毎の受信バッファの位置(オフセット)
     */

    std::cout << __FUNCTION__  << ": " << __LINE__ << std::endl;
    //displs = new int[ new_number_of_process ];
    //recvcounts = new int[ new_number_of_process ];
    //displs_size      = new int[ new_number_of_process ];
    //recvcounts_size  = new int[ new_number_of_process ];
    displs              = new int[ mpi_size ];
    recvcounts          = new int[ mpi_size ];
    displs_size         = new int[ mpi_size ];
    recvcounts_size     = new int[ mpi_size ];

    MPI_Allgather( &particle_size, 1, MPI_INT,
                   recvcounts,     1, MPI_INT,
                   MPI_COMM_WORLD );
    displs[0] = 0;
    recvcounts_size[0] = recvcounts[0]/3;
    //for( int i =1; i< new_number_of_process; i++ )
    for( int i =1; i< mpi_size; i++ )
    {
        displs[i]       = displs[i-1] + recvcounts[i-1];
        recvcounts_size[i] = recvcounts[i]/3;
        displs_size[i]  = displs_size[i-1] + recvcounts_size[i-1];

    }


    kvs::ValueArray<float> new_coords(  displs[new_number_of_process-1] + recvcounts[new_number_of_process-1] );
    kvs::ValueArray<float> new_vectors( displs[new_number_of_process-1] + recvcounts[new_number_of_process-1] );
    kvs::ValueArray<Byte>  new_colors(  displs[new_number_of_process-1] + recvcounts[new_number_of_process-1] );
    kvs::ValueArray<float> new_sizes( displs_size[new_number_of_process-1] + recvcounts_size[new_number_of_process-1] );

    std::cout << __FUNCTION__  << ": " << __LINE__ << std::endl;
    MPI_Gatherv( coords.pointer(),   particle_size, MPI_FLOAT,
                 new_coords.pointer(), recvcounts, displs, MPI_FLOAT,
                 0, MPI_COMM_WORLD );
    
    MPI_Gatherv( vectors.pointer(),   particle_size, MPI_FLOAT,
                 new_vectors.pointer(), recvcounts, displs, MPI_FLOAT,
                 0, MPI_COMM_WORLD );

    MPI_Gatherv( colors.pointer(),   particle_size, MPI_BYTE,
                 new_colors.pointer(), recvcounts, displs, MPI_BYTE,
                 0, MPI_COMM_WORLD );

    MPI_Gatherv( sizes.pointer(),   particle_size/3, MPI_FLOAT,
                 new_sizes.pointer(), recvcounts_size, displs_size, MPI_FLOAT,
                 0, MPI_COMM_WORLD );
#endif

    std::stringstream ss;
    //add by shimomura 20240614
//    ss << std::setfill('0') << std::setw(2) << static_cast<int>(celltype);
//    ss << "_";
    ss << std::setfill('0') << std::setw(5) << time_step;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_rank+1;
    ss << "_";
    ss << std::setfill('0') << std::setw(7) << mpi_size;
    ss << ".kvsml";
    glyphParameter.m_glyphFilePath += ss.str();
    // 20181226 end

    /*  分割後コミュニケータのランク0で出力する  */
//    if( new_rank == 0 )
//    {
        //kvs::PointObject* point_object = new kvs::PointObject( new_coords, new_colors, new_normals, particleBase.m_subpixel_level );
        kvs::PointObject* point_object = new kvs::PointObject( coords, colors, vectors, sizes );
        point_object->setMinMaxObjectCoords( particleBase.m_min_vec, particleBase.m_max_vec );
        // If async_io is enabled, use worker thread to write kvsml data and state.txt
//        if (async_io_enabled){
//            pbvr::ParticleWriteThread* particle_write_thread =  &pwt;
//            particle_write_thread->join(true);
//            particle_write_thread->setPointObject( point_object );
//            particle_write_thread->setFilename(particleBase.m_ptcFilePath.c_str());
//            particle_write_thread->setTimestep(time_step ,particleBase.m_stateFilePath.c_str());
//            particle_write_thread->setStartTimestep(st_time_step); //add by shimomura 20240808
//            particle_write_thread->work(true);
//        }// If async_io is disabled, use kvs::PointExporter here in main thread.
//        else{
            kvs::KVSMLObjectPoint* kvsml_object = new kvs::PointExporter<kvs::KVSMLObjectPoint>( point_object );
            //kvsml_object->setWritingDataType( kvs::KVSMLObjectPoint::ExternalBinary );
            kvsml_object->setWritingDataType( kvs::KVSMLObjectPoint::ExternalAscii );
            kvsml_object->write( glyphParameter.m_glyphFilePath.c_str() );
            delete kvsml_object;
//        }
        delete point_object;
//    }
    

#if 0
    //static bool parameter_file_opened= particleBase.m_parameter_file_opened;
    static bool parameter_file_opened= true;
    //最大最小値の集計
    if( parameter_file_opened )
    {
        O_min_recv.fill(0x00);
        O_max_recv.fill(0x00);
        C_min_recv.fill(0x00);
        C_max_recv.fill(0x00);

//        if(mpi_rank==0)std::cout<<"MPI_Reduce"<<std::endl;
        MPI_Reduce( particleBase.m_O_min.pointer(), O_min_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MIN, 0, MPI_COMM_WORLD );
        MPI_Reduce( particleBase.m_O_max.pointer(), O_max_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD );
        MPI_Reduce( particleBase.m_C_min.pointer(), C_min_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MIN, 0, MPI_COMM_WORLD );
        MPI_Reduce( particleBase.m_C_max.pointer(), C_max_recv.pointer(),
                    tf_number, MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD );

//        if(mpi_rank==0) std::cout<<"end MPI_Reduce"<<std::endl;

        //ヒストグラムの集計
        o_histogram_recv.fill(0x00);
        MPI_Reduce( particleBase.m_o_histogram.pointer(), o_histogram_recv.pointer(),
                    tf_number*nbins, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );

        c_histogram_recv.fill(0x00);
        MPI_Reduce( particleBase.m_c_histogram.pointer(), c_histogram_recv.pointer(),
                    tf_number*nbins, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
    }

//    timer.stop();
//    time.mpi_reduce = timer.sec();
//    timer.start();
    //状態ファイルの出力
    if( mpi_rank == 0 )
    {
        // 20181226 start 環境変数で指定したファイルパスを使用
        //std::ofstream ofs( "state.txt", std::ios::out);
        // If async_io is enabled, state.txt will be written from worker thread.
        // If async_io is disabled, state.txt will be written here.
        if (!async_io_enabled){
            std::ofstream ofs( particleBase.m_stateFilePath.c_str(), std::ios::out);
            // 20181226 end
            if( !ofs.is_open() ) std::cout<<"Cannot open state.txt"<<std::endl;

            ofs<<"START_STEP="<< st_time_step <<std::endl;
            ofs<<"LATEST_STEP="<<time_step<<std::endl;

            ofs.close();
        }
        std::stringstream step;
        step << '_' << std::setw( 5 ) << std::setfill( '0' ) << time_step;

        // 20181226 start 環境変数で指定したファイルパスを使用
        //std::string history_file_name = "history" + step.str() + ".txt";
        std::string history_file_name = particleBase.m_visParamDir + "history" + step.str() + ".txt";
        // 20181226 end
        std::ofstream ofs2( history_file_name.c_str(), std::ios::out);


//        std::cout << "tf_number = " << tf_number <<std::endl;
//        std::cout << "O_min_recv[0] = " << O_min_recv[0] <<std::endl;
        ofs2<<"TF_NUMBER="<<tf_number<<std::endl;
        for( int i = 0; i < tf_number; i++ )
        {
            ofs2<<"MIN_O"<<i+1<<"="<<O_min_recv[i]<<std::endl;
            ofs2<<"MAX_O"<<i+1<<"="<<O_max_recv[i]<<std::endl;
            ofs2<<"MIN_C"<<i+1<<"="<<C_min_recv[i]<<std::endl;
            ofs2<<"MAX_C"<<i+1<<"="<<C_max_recv[i]<<std::endl;
            ofs2<<"RESOLUTION_O"<<i+1<<"="<<nbins<<std::endl;
            ofs2<<"HISTOGRAM_O"<<i+1<<"=";
            for(int j=0; j<nbins; j++)
            {
                ofs2<<o_histogram_recv[j + i*nbins]<<",";
            }
            ofs2<<std::endl;
            ofs2<<"RESOLUTION_C"<<i+1<<"="<<nbins<<std::endl;
            ofs2<<"HISTOGRAM_C"<<i+1<<"=";
            for(int j=0; j<nbins; j++)
            {
                ofs2<<c_histogram_recv[j + i*nbins]<<",";
            }
            ofs2<<std::endl;
        }
        ofs2 << "END_HISTORY_FILE=SUCCESS" << std::endl;
        ofs2.close();

        if (skip_flag)
        {
            // 20181226 start 環境変数で指定したファイルパスを使用
            //std::string jupiter_file_name = "jupiter" + step.str() + ".tf";
            std::string jupiter_file_name = particleBase.m_visParamDir + particleBase.m_tfFilename + step.str() + ".tf";
            // 20181226 end
            param->write( jupiter_file_name );
        }
    }
//    timer.stop();
//    time.write_text = timer.sec();
//
//    show_timer( time );
    //if(mpi->rank==0)std::cout<<"end generate_particles\n";
#endif


}


void GlyphGenerator::show(glyph_parameters &glyphParameter)
{

    std::cout << "glyphParameter.m_glyph_sizes.size() = " << glyphParameter.m_glyph_sizes.size() << std::endl;
    std::cout << "glyphParameter.m_glyph_sizes.size() = " << glyphParameter.m_glyph_coords.size() << std::endl;
    std::cout << "glyphParameter.m_glyph_sizes.size() = " << glyphParameter.m_glyph_colors.size() << std::endl;
    std::cout << "glyphParameter.m_glyph_sizes.size() = " << glyphParameter.m_glyph_vectors.size() << std::endl;
    
    int nsample = glyphParameter.m_glyph_sizes.size() <30 ? glyphParameter.m_glyph_sizes.size() : 30;
      for(int i=0; i<nsample; i++)
      {
          std::cout << "glyphParameter.m_glyph_coords = " <<  glyphParameter.m_glyph_coords[i] << std::endl; 
          std::cout << "glyphParameter.m_glyph_colors = " <<  (int)glyphParameter.m_glyph_colors[i]  << std::endl; 
          std::cout << "glyphParameter.m_glyph_sizes = " <<  glyphParameter.m_glyph_sizes[i] << std::endl; 
          std::cout << "glyphParameter.m_glyph_vectors = " <<  glyphParameter.m_glyph_vectors[i] << std::endl; 
      }
    
}

