#include "GlyphGenerator.h"
#include <filesystem>

// IS用 constructor
GlyphGenerator::GlyphGenerator(Type** values,
        int nvariables, float* coordinates, int ncoords,
        unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltype) :
    m_values( values ), m_nvariable(nvariables),  
    m_coords( coordinates  ), m_ncoords( ncoords ), 
    m_connections( connections ), m_ncells( ncells ) 
{
    m_g_flag = false; 
    m_g_flag = this -> SetGlyphParameter();
    if (m_g_flag)
    { 
        if( m_distribution_modes == jpv::GlyphMode:: AllPoints || m_distribution_modes == jpv::GlyphMode:: EveryNthPoints )
        {
            this->PointSampling();
        }
        else if(m_distribution_modes == jpv::GlyphMode:: UniformDistribution)
        {
            this->DistributionSampling( celltype );
        }
    }
}
// CS用 constructor
GlyphGenerator::GlyphGenerator(const jpv::ParticleTransferClientMessage& clntMes, const int number_of_divide, Type** values,
        int nvariables, float* coordinates, int ncoords,
        unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltype) :
    m_values( values ), m_nvariable(nvariables),  
    m_coords( coordinates  ), m_ncoords( ncoords ), 
    m_connections( connections ), m_ncells( ncells ) 
{
   
    m_g_flag = false; 
    m_g_flag = this -> InputParameter(clntMes, number_of_divide);
    if (m_g_flag)
    { 
        if( m_distribution_modes == jpv::GlyphMode:: AllPoints || m_distribution_modes == jpv::GlyphMode:: EveryNthPoints )
        {
            this->PointSampling();
        }
        else if(m_distribution_modes == jpv::GlyphMode:: UniformDistribution)
        {
            this->DistributionSampling( celltype );
        }
    }

}

bool GlyphGenerator::InputParameter(const jpv::ParticleTransferClientMessage& clntMes, const int number_of_divide )
{


    bool glyph_flag;
    int stride                                     = clntMes.m_stride;
    int seed                                       = clntMes.m_seed; 
    int number_of_sample_points                    = clntMes.m_number_of_sampling_point ;
    

    int mpi_size = 1;
    int mpi_rank = 0;
#ifndef CPU_VER
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#endif

#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif

    //number_of_sample_points /= (mpi_size-1);  
    number_of_sample_points /= max_threads;
    number_of_sample_points /= number_of_divide; // ファイル分割数 

    m_number_of_sample_points = number_of_sample_points;
    float glyph_min=0; 
    float glyph_max=0;
    m_color_min = clntMes.m_glyph_color_min;
    m_color_max = clntMes.m_glyph_color_max;
    m_size_min = clntMes.m_glyph_size_min;
    m_size_max = clntMes.m_glyph_size_max;

    int table_size = clntMes.m_glyph_color_map_table.size();    
    kvs::ValueArray<kvs::UInt8> u_table( table_size );
    for( size_t j = 0; j< table_size ; j++ ) u_table[j] = (kvs::UInt8)clntMes.m_glyph_color_map_table[j];
    kvs::ColorMap color_map( u_table, glyph_min, glyph_max);
    m_color_map = color_map;

    glyph_flag = clntMes.m_glyph_flag;
   
    for (int i = 0; i< 3 ; i++)
    {
        m_direction_variables.push_back ( std::atoi(clntMes.m_direction_variable[i].substr(1).c_str()) - 1);
    }

    m_size_sampling_method    =clntMes.m_size_sampling_method;

    for (int i =0 ; i< clntMes.m_size_variable.size(); i++)
    {
        m_size_variables.push_back( std::atoi(clntMes.m_size_variable[i].substr(1).c_str()) -1); 
    }

    m_distribution_modes = clntMes.m_distribution_mode; 

    m_color_sampling_method    = clntMes.m_color_data_sampling_method;
    for (int i =0 ; i< clntMes.m_color_data_variable.size(); i++)
    {
        m_color_data_variables.push_back( std::atoi(clntMes.m_color_data_variable[i].substr(1).c_str()) - 1); 
    }

#if 0
    std::cout << "m_direction_variables        = " << m_direction_variables[0] << ", " << m_direction_variables[1]   << std::endl; 
    std::cout << "m_size_sampling_method       = " << static_cast<int>(m_size_sampling_method)      << std::endl; 
    if(m_size_variables.size() > 0) std::cout << "m_size_variables             = " << m_size_variables[0]    << std::endl; 
    std::cout << "m_distribution_modes         = " << static_cast<int>(m_distribution_modes )       << std::endl; 
    std::cout << "m_stride                     = " << m_stride                    << std::endl; 
    std::cout << "m_seed                       = " << m_seed                      << std::endl; 
    std::cout << "m_number_of_sample_points    = " << m_number_of_sample_points   << std::endl; 
    std::cout << "m_color_sampling_method      = " << static_cast<int>(m_color_sampling_method )    << std::endl; 
    if(m_color_data_variables.size() > 0) std::cout << "m_color_data_variables       = " << m_color_data_variables[0] <<  std::endl; 
#endif 
      return glyph_flag; 

}
#if 1
//bool GlyphGenerator::SetGlyphParameter( pbvr_parameters& particleBase, const int time_step )
bool GlyphGenerator::SetGlyphParameter( )
{
    std::string visParamDir;
    std::string glyphParamPath;
    std::string glyphParamPath_old;
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
    glyphParamPath_old = visParamDir + "parameter_old.gly";
   
    m_glyphParamPath = glyphParamPath;
    m_glyphFilePath = glyphFilePath;

    GlyphProperty glyph_property;

    bool read_flag = glyph_property.LoadIN(glyphParamPath) ;

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
    

    int mpi_size = 1;
    int mpi_rank = 0;
#ifndef CPU_VER
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#endif
//    if(read_flag)
//    {
////        if(mpi_rank ==0) std::rename( glyphParamPath.c_str(), glyphParamPath_old.c_str() );
//        if(mpi_rank ==0) std::filesystem::copy(glyphParamPath.c_str(), glyphParamPath_old.c_str(), std::filesystem::copy_options::overwrite_existing);
//    }


#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif

    number_of_sample_points /= mpi_size;  
    number_of_sample_points /= max_threads;  

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

    for (int i =0 ; i< size_variables.size(); i++)
    {
        m_size_variables.push_back( std::atoi(size_variables[i].substr(1).c_str()) -1); 
    }

    if     (distribution_modes == "AllPoints"           ) m_distribution_modes  = jpv::GlyphMode::AllPoints;
    else if(distribution_modes == "EveryNthPoints"         ) m_distribution_modes  = jpv::GlyphMode::EveryNthPoints;
    else if(distribution_modes == "UniformDistribution" ) m_distribution_modes  = jpv::GlyphMode::UniformDistribution;
    else 
    {
       std::cout << "Not selecting Distribution mode !!! Skip glyph generate process !!!" << std::endl;
       return false;  
    }

    m_stride                  = stride;
    m_seed                    = seed;
    m_number_of_sample_points = number_of_sample_points;
    if     (color_sampling_method == "Constant"       ) m_color_sampling_method    = jpv::DataDefines::Constant;
    else if(color_sampling_method == "SingleVariable" ) m_color_sampling_method    = jpv::DataDefines::SingleVariable;
    else if(color_sampling_method == "VariableArray" ) m_color_sampling_method    = jpv::DataDefines::VariableArray;
    else 
    {
       std::cout << "No selecting Sampling method !!! Skip glyph generate process !!!" << std::endl;
       return false;  
    }
    
    for (int i =0 ; i< color_data_variables.size(); i++)
    {
        m_color_data_variables.push_back( std::atoi(color_data_variables[i].substr(1).c_str()) - 1); 
    }

#if 0
    std::cout << "m_direction_variables        = " << m_direction_variables[0] << ", " << m_direction_variables[1]   << std::endl; 
    std::cout << "m_size_sampling_method       = " << static_cast<int>(m_size_sampling_method)      << std::endl; 
    if(m_size_variables.size() > 0) std::cout << "m_size_variables             = " << m_size_variables[0]    << std::endl; 
    std::cout << "m_distribution_modes         = " << static_cast<int>(m_distribution_modes )       << std::endl; 
    std::cout << "m_stride                     = " << m_stride                    << std::endl; 
    std::cout << "m_seed                       = " << m_seed                      << std::endl; 
    std::cout << "m_number_of_sample_points    = " << m_number_of_sample_points   << std::endl; 
    std::cout << "m_color_sampling_method      = " << static_cast<int>(m_color_sampling_method )    << std::endl; 
    if(m_color_data_variables.size() > 0) std::cout << "m_color_data_variables       = " << m_color_data_variables[0] <<  std::endl; 
#endif 
      return glyph_flag; 

}
#endif

//void GlyphGenerator::PointSampling( glyph_parameters &glyphParameter)
void GlyphGenerator::PointSampling( )
{
    const int stride = m_stride;
    int nPoints = m_ncoords/stride ;
    m_glyph_coords.resize(nPoints * 3);
    m_glyph_vectors.resize(nPoints * 3);
    m_glyph_sizes.resize(nPoints);
    m_glyph_colors_data.resize( nPoints );
    m_glyph_colors.resize( nPoints * 3); 

    m_glyph_coords.resize(nPoints * 3);
    m_glyph_vectors.resize(nPoints * 3);
    m_glyph_sizes.resize(nPoints);
    m_glyph_colors_data.resize( nPoints );
    m_glyph_colors.resize( nPoints * 3); 

    // size 
   std::vector<int> vector_var = m_direction_variables;
   int glyph_count =0;
   for (int i=0;i < m_ncoords; i+= stride)
   {
      m_glyph_coords[3*glyph_count    ] = m_coords[3*i    ];
      m_glyph_coords[3*glyph_count +1 ] = m_coords[3*i +1 ];
      m_glyph_coords[3*glyph_count +2 ] = m_coords[3*i +2 ];

      m_glyph_vectors[3*glyph_count    ] = m_values[vector_var[0]][i ];
      m_glyph_vectors[3*glyph_count +1 ] = m_values[vector_var[1]][i ];
      m_glyph_vectors[3*glyph_count +2 ] = m_values[vector_var[2]][i ];
      glyph_count++;
   }

   //size
   if (m_size_sampling_method == jpv::DataDefines::Constant)
   {
       std::fill(m_glyph_sizes.begin(), m_glyph_sizes.begin() ,1);
   }
//   else if (m_size_sampling_method == jpv::DataDefines::SingleVariable) 
//   {
//       int glyph_count =0;
//       //float size_min = m_size_min[m_size_variables[0]] < 0 ? 0 : m_size_min[m_size_variables[0]];
//       //float size_max = m_size_max[m_size_variables[0]] < 0 ? 0 : m_size_max[m_size_variables[0]];
//
//       for (int i = 0 ; i < m_ncoords; i+= stride)
//       {
//           float ss = ( kvs::Math::Abs(m_values[m_size_variables[0]][ i ]) - m_size_min[m_size_variables[0]])/( m_size_max[m_size_variables[0]] - m_size_min[m_size_variables[0]] );
//           //float ss = kvs::Math::Abs(m_values[m_size_variables[0]][ i ]) /( m_size_max[m_size_variables[0]] );
//           m_glyph_sizes[ glyph_count]       = ss;
//           //.m_glyph_sizes[ glyph_count]       = kvs::Math::Abs(m_values[m_size_variables[0]][ i ]);
//           glyph_count++;
//       }
//   }
   else if (  m_size_sampling_method == jpv::DataDefines::VariableArray || m_size_sampling_method == jpv::DataDefines::SingleVariable ) 
   {
       std::vector<float> tmp_size(nPoints);
       std::vector<int> size_var = m_size_variables;
       int n_size_variables=m_size_variables.size();
       int glyph_count =0;
       for (int i=0;i < m_ncoords; i+= stride)
       {
           for(int k = 0 ; k< n_size_variables ; k++)
           {
               tmp_size[ glyph_count ] += kvs::Math::Square( m_values[ size_var[k] ][i] ); 
           }
           m_glyph_sizes[ glyph_count ] = std::sqrt(tmp_size[ glyph_count ]) ;
           glyph_count++;
       }

        int n_size_data=m_glyph_sizes.size();
        float max=FLT_MIN;
        float min=FLT_MAX;
        float tmp_max=FLT_MIN;
        float tmp_min=FLT_MAX;
        for(int k = 0; k< n_size_data; k++)
        {
            max = kvs::Math::Max(m_glyph_sizes[k], max ); 
            min = kvs::Math::Min(m_glyph_sizes[k], min ); 
        }

        tmp_max = max;
        tmp_min = min;

#if 0 //ISPBVR
#ifndef CPU_VER
        MPI_Allreduce( MPI_IN_PLACE, &min, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, &max, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
#endif
#else
        max = m_size_max;
        min = m_size_min;
#endif
        float factor =0;
        if (max - min > 1e-6 ) 
        {
            factor = 1/ (max - min) ;
        }
        else  
        {
            factor = 1;
        }

        m_size_min = tmp_min;
        m_size_max = tmp_max;
       for (int i = 0; i < nPoints; i++)
       {
           m_glyph_sizes[ i] = (m_glyph_sizes[ i ] - min )*factor;
           m_glyph_sizes[ i] = kvs::Math::Clamp<float>( m_glyph_sizes[ i], 0.0, 1.0 );
       }

   }

   //color 
   float glyph_color_data_max = FLT_MIN;
   float glyph_color_data_min = FLT_MAX;
   if (m_color_sampling_method == jpv::DataDefines::Constant)
   {
       std::fill(m_glyph_colors.begin(), m_glyph_colors.begin(), 0);
       m_color_map.setRange(0, 1);
       m_color_min = 0;
       m_color_max = 1;
   }
   else if (m_color_sampling_method == jpv::DataDefines::VariableArray || m_color_sampling_method == jpv::DataDefines::SingleVariable ) 
   {
       std::vector<int> color_var = m_color_data_variables;
       int n_color_variables=color_var.size();

       int glyph_count =0;
       std::vector<float> tmp_size(nPoints);
       for (int i = 0; i < m_ncoords; i+= stride)
       {
           for(int k = 0 ; k< n_color_variables ; k++)
           {
            tmp_size[ glyph_count ] += kvs::Math::Square(m_values[color_var[k]][ i ]) ;
           }
           m_glyph_colors_data[ glyph_count ] = std::sqrt(tmp_size[ glyph_count ]);
           glyph_count++;
       }

       int n_color_data=m_glyph_colors_data.size();
       float max=FLT_MIN;
       float min=FLT_MAX;
       for(int k = 0; k< n_color_data; k++)
       {
           max = kvs::Math::Max(m_glyph_colors_data[k], max ); 
           min = kvs::Math::Min(m_glyph_colors_data[k], min ); 
       }

#if 0 // IS
#ifndef CPU_VER
       MPI_Allreduce( MPI_IN_PLACE, &min, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
       MPI_Allreduce( MPI_IN_PLACE, &max, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
#endif
       m_color_map.setRange(min, max);
#else 
       m_color_map.setRange(m_color_min, m_color_max);
#endif

        m_color_min = min;
        m_color_max = max;
   }

    for (int ii=0;ii < nPoints; ii++)
    {
        kvs::RGBColor colors; 
        colors = m_color_map.at(m_glyph_colors_data[ ii ]);
        m_glyph_colors[3*ii    ] = colors.r() ;
        m_glyph_colors[3*ii +1 ] = colors.g() ;
        m_glyph_colors[3*ii +2 ] = colors.b() ;
    }
//    this -> show();
}

void GlyphGenerator::DistributionSampling( const pbvr::VolumeObjectBase::CellType& celltype)
{
    const int seed          = m_seed;

#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif

    int mpi_rank = 0;
    int mpi_size = 1;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#endif

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
                if (mpi_rank == 0) std::cout << "Cell type : Quadratic tetrahedra " << std::endl; 
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
                if (mpi_rank == 0) std::cout << "Cell type : Hexahedra " << std::endl; 
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
                if (mpi_rank == 0) std::cout << "Cell type : Quadratic hexahedra " << std::endl; 
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
        //nglyphs /= nthreads;
        kvs::MersenneTwister MT( seed + (mpi_rank+1)*thid );

    float TotalVolume = 0;
    float density = 0;
        // 動的な粒子データ配列
        std::vector<float> th_glyph_coords;
        std::vector<float> th_glyph_colors_data;
        std::vector<float> th_glyph_vectors;
        std::vector<float> th_glyph_sizes;

       
//#pragma omp for schedule( dynamic ) nowait
#pragma omp parallel for
        for( int cell_base = 0; cell_base < m_ncells; cell_base ++ )
        {
                interp[thid][0]->bindCell( cell_base );
                TotalVolume += interp[thid][0]->volume();
        }

        //density = m_number_of_sample_points/TotalVolume;
        density = m_number_of_sample_points/TotalVolume;

        //粒子生成ループ開始
//#pragma omp for schedule( dynamic ) nowait
#pragma omp parallel for
        for( int cell_base = 0; cell_base < m_ncells; cell_base ++ )
        {

            kvs::Vector3f coord = interp[thid][0]->localGravityPoint();
            interp[thid][0]->bindCell( cell_base);
            interp[thid][0]->setLocalPoint( coord );

            //生成glyph数を計算
            int  nglyphs = calculate_number_of_particles( density, interp[thid][0]->volume(), &MT ) ;

            for( int i = 0; i < nglyphs; i++ )
            {
                kvs::Vector3f local_coord = interp[thid][0] -> randomSampling_MT( &MT );
                //if (i ==0) local_coord = kvs::Vector3f(0.5f,0.5f,0.5f);

                //補間器にセルを一括でバインド
                for( int k = 0; k < m_nvariable; k++ )
                {
                    interp[thid][k]->bindCell( cell_base );
                    interp[thid][k]->setLocalPoint( local_coord );
                }

                kvs::Vector3f global_coord = interp[thid][0]->transformLocalToGlobal( local_coord );

                //サイズ計算
                float size = 0;
                if (m_size_sampling_method == jpv::DataDefines::SingleVariable || m_size_sampling_method == jpv::DataDefines::VariableArray )
                { 
                    float scalar_array[interp[thid].size()];
                    float eval_result =0;

                    for( size_t j= 0; j < m_nvariable; j++ )
                    {
                        scalar_array[j] = interp[thid][j]->scalar();
                    }

                    std::vector<int> size_var = m_size_variables;
                    int n_size_data=size_var.size();
                    for( size_t j= 0; j < n_size_data; j++ ) 
                    {
                        eval_result += scalar_array[size_var[j]] * scalar_array[size_var[j]] ; 
                    }
                    size = std::sqrt(eval_result) ; 
                }
                else size = 1.f;

                //色の計算
                float color_data =0; 

                if (m_color_sampling_method ==  jpv::DataDefines::SingleVariable || m_color_sampling_method == jpv::DataDefines::VariableArray )
                { 
                    float scalar_array[interp[thid].size()];
                    float eval_result =0;

                    for( size_t j= 0; j < m_nvariable; j++ )
                    {
                        scalar_array[j] = interp[thid][j]->scalar();
                    }

                    std::vector<int> color_var = m_color_data_variables;
                    int n_color_data=color_var.size();
                    for( size_t j= 0; j < n_color_data; j++ ) // スタブデータとして　0,1　を選択
                    {
                        eval_result += scalar_array[color_var[j]] * scalar_array[color_var[j]] ; 
                    }
                    color_data = std::sqrt(eval_result) ; 
                }
                else color_data = 0; 

                // glyph_vectorの計算
                float scalar_array[interp[thid].size()];
                float eval_result =0;

                for( size_t j= 0; j < m_nvariable; j++ )
                {
                    scalar_array[j] = interp[thid][j]->scalar();
                }

                // 座標情報の格納
                th_glyph_coords.push_back( global_coord.x() );
                th_glyph_coords.push_back( global_coord.y() );
                th_glyph_coords.push_back( global_coord.z() );
                // サイズ情報の格納
                th_glyph_sizes.push_back( size );
                // 色用データ情報の格納
                th_glyph_colors_data.push_back( color_data );
                // direction情報の格納
                th_glyph_vectors.push_back( scalar_array[m_direction_variables[0]] );
                th_glyph_vectors.push_back( scalar_array[m_direction_variables[1]] );
                th_glyph_vectors.push_back( scalar_array[m_direction_variables[2]] );

                // ------------------------------------------------

            }//end of for i
        }// end of for cell
        #pragma omp barrier
        #pragma omp critical
        {
            m_glyph_coords.insert  ( m_glyph_coords.end(), th_glyph_coords.begin(), th_glyph_coords.end() );
            m_glyph_colors_data.insert  ( m_glyph_colors_data.end(), th_glyph_colors_data.begin(), th_glyph_colors_data.end() );
            m_glyph_sizes.insert   ( m_glyph_sizes.end(),  th_glyph_sizes.begin(),  th_glyph_sizes.end() );
            m_glyph_vectors.insert ( m_glyph_vectors.end(), th_glyph_vectors.begin(), th_glyph_vectors.end() );
        }

     } //#pragma omp parallel

        std::cout << __FUNCTION__ << __LINE__ << std::endl;
    if (m_size_sampling_method == jpv::DataDefines::SingleVariable || m_size_sampling_method == jpv::DataDefines::VariableArray )
    { 
        int n_size_data=m_glyph_sizes.size();
        float max=FLT_MIN;
        float min=FLT_MAX;
        float tmp_max=FLT_MIN;
        float tmp_min=FLT_MAX;
        for(int k = 0; k< n_size_data; k++)
        {
            max = kvs::Math::Max(m_glyph_sizes[k], max ); 
            min = kvs::Math::Min(m_glyph_sizes[k], min ); 
        }

        tmp_max = max;
        tmp_min = min;
#if 0 //IS
#ifndef CPU_VER
        MPI_Allreduce( MPI_IN_PLACE, &min, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, &max, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
#endif
#else
        max = m_size_max;
        min = m_size_min;
        std::cout << "tmp_min = " << tmp_min << std::endl;
        std::cout << "tmp_max = " << tmp_max << std::endl;
        std::cout << "m_size_min = " << m_size_min << std::endl;
        std::cout << "m_size_max = " << m_size_max << std::endl;
#endif
        float factor = 0;
        if (max - min > 1e-6)
        {
            factor = 1.f/ (max - min)  ;
            for( int j = 0; j < n_size_data; j++ )
            {
                m_glyph_sizes[j] = (m_glyph_sizes[j] - min)*factor;
                m_glyph_sizes[j] = kvs::Math::Clamp<float>( m_glyph_sizes[j], 0.0, 1.0 );
            }
        }
        else
        {
            for( int j = 0; j < n_size_data; j++ )
            {
               m_glyph_sizes[j] = 1.f;
            }
        }
        m_size_min = tmp_min;
        m_size_max = tmp_max;

    }
    else
    {
        int n_size_data=m_glyph_sizes.size();
        for( int j = 0; j < n_size_data; j++ )
        {
            m_glyph_sizes[j] = 1.f;
        }
        m_size_min = 1.000000;
        m_size_max = 1.000001;
    }

    if (m_color_sampling_method == jpv::DataDefines::SingleVariable || m_color_sampling_method == jpv::DataDefines::VariableArray )
    { 
        int n_color_data=m_glyph_colors_data.size();
        float max=FLT_MIN;
        float min=FLT_MAX;
        for(int k = 0; k< n_color_data; k++)
        {
            max = kvs::Math::Max(m_glyph_colors_data[k], max ); 
            min = kvs::Math::Min(m_glyph_colors_data[k], min ); 
        }

        std::cout << __FUNCTION__ << __LINE__ << std::endl;
#if 0 // IS
#ifndef CPU_VER  // CS用処理
        float max_recv=FLT_MIN;
        float min_recv=FLT_MAX;
        //        MPI_Allreduce( MPI_IN_PLACE, &min, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
        //        MPI_Allreduce( MPI_IN_PLACE, &max, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
        std::cout << __FUNCTION__ << __LINE__ << std::endl;
        if (mpi_size > 2 )
        {
            if (mpi_rank != 0 )  // masterプロセスは除外
            {
                MPI_Reduce( &min , &min_recv, 1, MPI_FLOAT, MPI_MIN, 1, MPI_COMM_WORLD);
                MPI_Reduce( &max , &max_recv, 1, MPI_FLOAT, MPI_MAX, 1, MPI_COMM_WORLD);
            }

            std::cout << __FUNCTION__ << __LINE__ << std::endl;
            if (mpi_rank != 0) 
            {
                MPI_Bcast(&min_recv, 1, MPI_FLOAT, 1, MPI_COMM_WORLD);
                MPI_Bcast(&max_recv, 1, MPI_FLOAT, 1, MPI_COMM_WORLD);
            } 
        }
            else
            {
                min_recv = min;
                max_recv = max;
            }
            std::cout << __FUNCTION__ << __LINE__ << std::endl;
            m_color_map.setRange(min_recv,max_recv);
#else
        m_color_map.setRange(m_color_min,m_color_max);
#endif
#else 
        m_color_map.setRange(m_color_min,m_color_max);
        std::cout << "m_color_min = " << m_color_min << std::endl;
        std::cout << "m_color_max = " << m_color_max << std::endl;
#endif
        for( int jx=0; jx<n_color_data; jx++)
        {
            kvs::RGBColor color;
            color = m_color_map.at( m_glyph_colors_data[jx] );
            m_glyph_colors.push_back( color.r());
            m_glyph_colors.push_back( color.g());
            m_glyph_colors.push_back( color.b());
        }
        m_color_min = min;
        m_color_max = max;
    }
    else
    {
        int n_color_data=m_glyph_colors_data.size();
        for( int jx=0; jx<n_color_data; jx++)
        {
            m_glyph_colors.push_back( 255 );
            m_glyph_colors.push_back( 255 );
            m_glyph_colors.push_back( 255 );
        }
        m_color_min = 1.00000;
        m_color_max = 1.00001;
    }

//       this->show(); 
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

void GlyphGenerator::getGlyphData(kvs::KVSMLObjectGlyph* other)
{
    kvs::ValueArray<float> coords( m_glyph_coords  );
    kvs::ValueArray<float> directions(m_glyph_vectors );
    kvs::ValueArray<Byte>  colors( m_glyph_colors   );
    kvs::ValueArray<float> sizes(  m_glyph_sizes  );
    other -> setCoords( coords );
    other -> setColors( colors );
    other -> setDirections( directions );
    other -> setSizes( sizes );
    other -> setColorMin(m_color_min);
    other -> setColorMax(m_color_max);
    other -> setSizeMin(m_size_min);
    other -> setSizeMax(m_size_max);
}


//void GlyphGenerator::OutputGlyph( const  pbvr_parameters& particleBase, const int time_step)
void GlyphGenerator::OutputGlyph( const int time_step)
{
    if (!m_g_flag) return; 

    int mpi_rank = 0;
    int mpi_size = 1;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#endif
    int nbins = 256;

    ///-------------------------------------//
    ///--------粒子配列をファイル出力----------//
    //--------------------------------------//
    //kvs::ValueArray<float> coords( particleBase.m_sample_coords );
    //kvs::ValueArray<Byte>  colors( particleBase.m_sample_colors );
    //kvs::ValueArray<float> normals(particleBase.m_sample_normals );
    kvs::ValueArray<float> coords( m_glyph_coords  );
    kvs::ValueArray<float> vectors(m_glyph_vectors );
    kvs::ValueArray<Byte>  colors( m_glyph_colors   );
    kvs::ValueArray<float> sizes(  m_glyph_sizes  );

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
//    ss << ".kvsml";
    ss << ".dat";
    m_glyphFilePath += ss.str();
    // 20181226 end

    /*  分割後コミュニケータのランク0で出力する  */
#if 0
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
            kvsml_object->write( m_glyphFilePath.c_str() );
            delete kvsml_object;

//        }
        delete point_object;
//    }
    #else
            kvs::KVSMLObjectGlyph kvsmlobject( coords, colors, vectors, sizes);
            kvsmlobject.write(m_glyphFilePath.c_str());
#endif

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

//        if(mpi_rank==0)std::
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


void GlyphGenerator::show()
{

    std::cout << "glyphParameter.m_glyph_sizes.size() = " << m_glyph_sizes.size() << std::endl;
    std::cout << "glyphParameter.m_glyph_sizes.size() = " << m_glyph_coords.size() << std::endl;
    std::cout << "glyphParameter.m_glyph_sizes.size() = " << m_glyph_colors.size() << std::endl;
    std::cout << "glyphParameter.m_glyph_sizes.size() = " << m_glyph_vectors.size() << std::endl;
    
    int nsample = m_glyph_sizes.size() <30 ? m_glyph_sizes.size() : 30;
      for(int i=0; i<nsample; i++)
      {
          std::cout << "glyphParameter.m_glyph_coords = " <<  m_glyph_coords[i] << std::endl; 
          std::cout << "glyphParameter.m_glyph_colors = " <<  (int)m_glyph_colors[i]  << std::endl; 
          std::cout << "glyphParameter.m_glyph_sizes = " <<    m_glyph_sizes[i] << std::endl; 
          std::cout << "glyphParameter.m_glyph_vectors = " <<  m_glyph_vectors[i] << std::endl; 
      }
    
}

