#include "GlyphSeed.h"
#include <vismodule/GlyphProperty>

//// IS用 constructor
//GlyphSeed::GlyphSeed(Type** values,
//        int nvariables, float* coordinates, int ncoords,
//        unsigned int* connections, int ncells, const  vismodule::VolumeObjectBase::CellType& celltype) :
//    m_values( values ), m_nvariable(nvariables),  
//    m_coords( coordinates  ), m_ncoords( ncoords ), 
//    m_connections( connections ), m_ncells( ncells ) 
//{
//    m_g_flag = false; 
//    m_g_flag = this -> SetGlyphParameter();
//    if (m_g_flag)
//    { 
//        if( m_distribution_modes == jpv::GlyphMode:: AllPoints || m_distribution_modes == jpv::GlyphMode:: EveryNthPoints )
//        {
//            this->PointSampling_unstruct();
//        }
//        else if(m_distribution_modes == jpv::GlyphMode:: UniformDistribution)
//        {
//            this->DistributionSampling_unstruct( celltype );
//        }
//    }
//}

// unstruct 
GlyphSeed::GlyphSeed(
    const GlyphProperty& glyph_property,
    const int number_of_divide,
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    const bool is_flag
):
m_values( values ),
m_nvariable( nvariables ),
m_coords( coordinates ),
m_ncoords( ncoords ), 
m_connections( connections ),
m_ncells( ncells ),
m_is_flag( is_flag ) 
{
    std::cout << "m_is_flag = " << m_is_flag << std::endl; 
    m_g_flag = false; 
    m_g_flag = this->InputParameter( glyph_property, number_of_divide );
    if ( m_g_flag )
    { 
        if( 
            m_distribution_modes == GlyphMode:: AllPoints ||
            m_distribution_modes == GlyphMode:: EveryNthPoints
        )
        {
            this->PointSampling_unstruct();
        }
        else if( m_distribution_modes == GlyphMode::UniformDistribution )
        {
            this->DistributionSampling_unstruct( celltype );
        }
    }
}

// struct 
GlyphSeed::GlyphSeed(
    const GlyphProperty& glyph_property,
    const int number_of_divide,
    domain_parameters_struct dom,
    Type** values,
    int nvariables,
    const bool is_flag
): 
m_values( values ),
m_nvariable(nvariables),
m_is_flag( is_flag )
{   
    m_g_flag = false; 
    m_g_flag = this->InputParameter( glyph_property, number_of_divide );

    if ( m_g_flag )
    { 
        if( m_distribution_modes == GlyphMode:: AllPoints ||
            m_distribution_modes == GlyphMode:: EveryNthPoints
        )
        {
            m_ncoords = dom.resolution[0]*dom.resolution[1]*dom.resolution[2];
            // m_nvariable = nvariables;
            // m_values = (float**)values;
            
            this->PointSampling_struct( dom );
        }
        else if( m_distribution_modes == GlyphMode::UniformDistribution )
        {
            this->DistributionSampling_struct( dom, values, nvariables );
        }
    }
}

bool GlyphSeed::InputParameter( const GlyphProperty& glyph_property, const int number_of_divide )
{
    bool glyph_flag;
    int stride                    = glyph_property.m_stride;
    int seed                      = glyph_property.m_seed; 
    float number_of_sample_points = glyph_property.m_number_of_sampling_point;

    int mpi_size;
    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_size = 1;
    mpi_rank = 0;
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
    
    float glyph_min = 0;
    float glyph_max = 0;
    // minmaxの値は前ステップで算出したものを参照
    m_color_min = glyph_property.m_glyph_color_min;
    m_color_max = glyph_property.m_glyph_color_max;
    m_size_min  = glyph_property.m_glyph_size_min;
    m_size_max  = glyph_property.m_glyph_size_max;

    int table_size = glyph_property.m_glyph_color_map_table.size();    
    vismodule::ValueArray<vismodule::UInt8> u_table( table_size );
    for( std::size_t j = 0; j < table_size; j++ ) u_table[j] = (vismodule::UInt8)glyph_property.m_glyph_color_map_table[j];
    vismodule::ColorMap color_map( u_table, glyph_min, glyph_max );
    m_color_map = color_map;

    if( m_is_flag ) m_color_map = glyph_property.m_color_map;

    glyph_flag = glyph_property.m_glyph_flag;
   
    for ( int i = 0; i < 3 ; i++ )
    {
        m_direction_variables.push_back ( std::atoi( glyph_property.m_direction_variable[i].substr(1).c_str() ) - 1 );
    }

    m_size_sampling_method = glyph_property.m_size_sampling_method;

    for ( int i = 0; i < glyph_property.m_size_variable.size(); i++ )
    {
        m_size_variables.push_back( std::atoi( glyph_property.m_size_variable[i].substr(1).c_str() ) -1 ); 
    }

    m_distribution_modes = glyph_property.m_distribution_mode; 

    m_stride = stride;
    if ( m_distribution_modes == GlyphMode::AllPoints ) m_stride = 1;
    m_seed = seed; 
    m_color_sampling_method = glyph_property.m_color_data_sampling_method;

    for ( int i = 0; i < glyph_property.m_color_data_variable.size(); i++ )
    {
        m_color_data_variables.push_back( std::atoi( glyph_property.m_color_data_variable[i].substr(1).c_str() ) - 1 );
    }

#if 1
    std::string size_sampling_method_string;
    if ( m_size_sampling_method == DataDefines::Constant ) size_sampling_method_string = "Constant";
    else if ( m_size_sampling_method == DataDefines::VariableArray ) size_sampling_method_string = "VariableArray";
    else size_sampling_method_string = "Unknown";

    std::string distribution_modes_string;
    if ( m_distribution_modes == GlyphMode::AllPoints ) distribution_modes_string = "AllPoints";
    else if ( m_distribution_modes == GlyphMode::EveryNthPoints ) distribution_modes_string = "EveryNthPoints";
    else if ( m_distribution_modes == GlyphMode::UniformDistribution ) distribution_modes_string = "UniformDistribution";
    else distribution_modes_string = "Unknown";

    std::string color_sampling_method_string;
    if ( m_color_sampling_method == DataDefines::Constant ) color_sampling_method_string = "Constant";
    else if ( m_color_sampling_method == DataDefines::VariableArray ) color_sampling_method_string = "VariableArray";
    else color_sampling_method_string = "Unknown";

    std::cout << "m_size_sampling_method    = " << size_sampling_method_string  << std::endl;
    std::cout << "m_distribution_modes      = " << distribution_modes_string    << std::endl; 
    std::cout << "m_stride                  = " << m_stride                     << std::endl; 
    std::cout << "m_seed                    = " << m_seed                       << std::endl; 
    std::cout << "m_number_of_sample_points = " << m_number_of_sample_points    << std::endl;
    std::cout << "m_color_sampling_method   = " << color_sampling_method_string << std::endl; 

    for ( std::size_t i = 0; i < m_direction_variables.size(); i++ )
    {
        std::cout << "m_direction_variables[" << i << "]  = " << m_direction_variables[i] << std::endl;
    }

    for ( std::size_t i = 0; i < m_size_variables.size(); i++ )
    {
        std::cout << "m_size_variables[" << i << "]       = " << m_size_variables[i] << std::endl;
    }

    for ( std::size_t i = 0; i < m_color_data_variables.size(); i++ )
    {
        std::cout << "m_color_data_variables[" << i << "]       = " << m_color_data_variables[i] << std::endl;
    }
#endif 
    return glyph_flag; 
}

//#if 1
//bool GlyphSeed::SetGlyphParameter()
//{
//    std::string visParamDir;
//    std::string glyphParamPath;
//    std::string glyphFilePath;
//
//    const char *envBuf = NULL;
//    envBuf = std::getenv( "VIS_PARAM_DIR" );
//    if (envBuf == NULL) {
//        visParamDir = "./";
//    }
//    else {
//        visParamDir = envBuf;
//        if (visParamDir[visParamDir.size() - 1] != '/') {
//            visParamDir += "/";
//        }
//    }
//    envBuf = std::getenv( "PARTICLE_DIR" );
//    if (envBuf == NULL) {
//        glyphFilePath = "./g_";
//    }
//    else {
//        glyphFilePath = envBuf;
//        if (glyphFilePath[glyphFilePath.size() - 1] != '/') {
//             glyphFilePath += "/g_";
//        }
//        else {
//            glyphFilePath += "g_";
//        }
//    }
//
//    glyphParamPath = visParamDir + "parameter.gly";
//   
//    m_glyphParamPath = glyphParamPath;
//    m_glyphFilePath = glyphFilePath;
//
//    GlyphProperty glyph_property;
//
//    bool read_flag = glyph_property.LoadIN(glyphParamPath) ;
//
//    bool glyph_flag;
//    std::string              g_flag                = glyph_property.getString( "GLYPH_FLAG" );
//    std::vector<std::string> direction_variables   = glyph_property.getTableString( "DIRECTION_VARIABLES" );
//    std::string              size_sampling_method  = glyph_property.getString("SIZE_SAMPLING_METHOD");
//    std::vector<std::string> size_variables        = glyph_property.getTableString( "SIZE_VARIABLES" );
//    std::string distribution_modes                 = glyph_property.getString("DISTRIBUTION_MODE");
//    int stride                                     = glyph_property.getInt("STRIDE");
//    int seed                                       = glyph_property.getInt("SEED");
//    float number_of_sample_points                    = glyph_property.getInt("NUMBER_OF_SMAPLING_POINT");
//    std::string color_sampling_method              = glyph_property.getString("COLOR_DATA_SAMPLING_METHOD");
//    std::vector<std::string> color_data_variables  = glyph_property.getTableString( "COLOR_VARIABLES" );
//    
//
//    int mpi_size = 1;
//    int mpi_rank = 0;
//#ifndef CPU_VER
//    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
//    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
//#endif
//
//
//#if _OPENMP
//    int max_threads = omp_get_max_threads();
//#else
//    int max_threads = 1;
//#endif
//
//    number_of_sample_points /= mpi_size;  
//    number_of_sample_points /= max_threads;  
//
//    float glyph_min=0; 
//    float glyph_max=0;
//    glyph_min = glyph_property.getFloat("GLYPH_COLOR_MIN");
//    glyph_max = glyph_property.getFloat("GLYPH_COLOR_MAX");
//    std::vector<int> i_table;
//    i_table = glyph_property.getTableInt( "GLYPH_COLOR_MAP_TABLE" );
//    vismodule::ValueArray<vismodule::UInt8> u_table( i_table.size() );
//    for( std::size_t j = 0; j<i_table.size(); j++ ) u_table[j] = (vismodule::UInt8)i_table[j];
//    vismodule::ColorMap color_map( u_table, glyph_min, glyph_max);
//
//    m_color_map = color_map;
//    if(strcmp(g_flag.c_str(), "TRUE") ==0 ) glyph_flag = true;
//    else glyph_flag = false;
//   
//    if(direction_variables.size() < 3)
//    { 
//        std::cout << "variables number is less 3 numbers !!! Skip glyph generate process !!!" << std::endl;
//        return false;  
//    }
//    for (int i = 0; i< 3 ; i++)
//    {
//        m_direction_variables.push_back ( std::atoi(direction_variables[i].substr(1).c_str()) - 1);
//    }
//
//    if     (size_sampling_method == "Constant"       ) m_size_sampling_method    = jpv::DataDefines::Constant;
//    else if(size_sampling_method == "VariableArray" )  m_size_sampling_method    = jpv::DataDefines::VariableArray;
//    else 
//    {
//       std::cout << "No selecting Sampling method !!! Skip glyph generate process !!!" << std::endl;
//       return false;  
//    }
//
//    for (int i =0 ; i< size_variables.size(); i++)
//    {
//        m_size_variables.push_back( std::atoi(size_variables[i].substr(1).c_str()) -1); 
//    }
//
//    if     (distribution_modes == "AllPoints"           ) m_distribution_modes  = jpv::GlyphMode::AllPoints;
//    else if(distribution_modes == "EveryNthPoints"         ) m_distribution_modes  = jpv::GlyphMode::EveryNthPoints;
//    else if(distribution_modes == "UniformDistribution" ) m_distribution_modes  = jpv::GlyphMode::UniformDistribution;
//    else 
//    {
//       std::cout << "Not selecting Distribution mode !!! Skip glyph generate process !!!" << std::endl;
//       return false;  
//    }
//
//    m_stride                  = stride;
//    m_seed                    = seed;
//    if (m_distribution_modes  == jpv::GlyphMode::AllPoints )m_stride = 1;
//    m_number_of_sample_points = number_of_sample_points;
//    if     (color_sampling_method == "Constant"       ) m_color_sampling_method    = jpv::DataDefines::Constant;
//    else if(color_sampling_method == "VariableArray" ) m_color_sampling_method    = jpv::DataDefines::VariableArray;
//    else 
//    {
//       std::cout << "No selecting Sampling method !!! Skip glyph generate process !!!" << std::endl;
//       return false;  
//    }
//    
//    for (int i =0 ; i< color_data_variables.size(); i++)
//    {
//        m_color_data_variables.push_back( std::atoi(color_data_variables[i].substr(1).c_str()) - 1); 
//    }
//
//#if 0
//    std::cout << "m_direction_variables        = " << m_direction_variables[0] << ", " << m_direction_variables[1]   << std::endl; 
//    std::cout << "m_size_sampling_method       = " << static_cast<int>(m_size_sampling_method)      << std::endl; 
//    if(m_size_variables.size() > 0) std::cout << "m_size_variables             = " << m_size_variables[0]    << std::endl; 
//    std::cout << "m_distribution_modes         = " << static_cast<int>(m_distribution_modes )       << std::endl; 
//    std::cout << "m_stride                     = " << m_stride                    << std::endl; 
//    std::cout << "m_seed                       = " << m_seed                      << std::endl; 
//    std::cout << "m_number_of_sample_points    = " << m_number_of_sample_points   << std::endl; 
//    std::cout << "m_color_sampling_method      = " << static_cast<int>(m_color_sampling_method )    << std::endl; 
//    if(m_color_data_variables.size() > 0) std::cout << "m_color_data_variables       = " << m_color_data_variables[0] <<  std::endl; 
//#endif 
//      return glyph_flag; 
//
//}
//#endif
#if 1
void GlyphSeed::PointSampling_unstruct()
{
    const int stride = m_stride;
    int nPoints = m_ncoords/stride ;
    if( m_ncoords%stride >0 ) nPoints += 1;
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
   if (m_size_sampling_method == DataDefines::Constant)
   {
       std::fill(m_glyph_sizes.begin(), m_glyph_sizes.end() ,1);
   }
   else if ( m_size_sampling_method == DataDefines::VariableArray ) 
   {
       std::vector<float> tmp_size(nPoints);
       std::vector<int> size_var = m_size_variables;
       int n_size_variables=m_size_variables.size();
       int glyph_count =0;
       for (int i=0;i < m_ncoords; i+= stride)
       {
           for(int k = 0 ; k< n_size_variables ; k++)
           {
               tmp_size[ glyph_count ] += vismodule::Math::Square( m_values[ size_var[k] ][i] ); 
           }
           m_glyph_sizes[ glyph_count ] = std::sqrt(tmp_size[ glyph_count ]) ;
           glyph_count++;
       }

        // size計算用
        float max=FLT_MIN;
        float min=FLT_MAX;
        // 一時保存用
        float tmp_max=FLT_MIN;
        float tmp_min=FLT_MAX;
        
        int n_size_data=m_glyph_sizes.size();
        for(int k = 0; k< n_size_data; k++)
        {
            max = vismodule::Math::Max(m_glyph_sizes[k], max ); 
            min = vismodule::Math::Min(m_glyph_sizes[k], min ); 
        }

        tmp_max = max;
        tmp_min = min;

       if(m_is_flag)//IS の場合、全領域のminmaxをここで集約する
       {
#ifndef CPU_VER
        MPI_Allreduce( MPI_IN_PLACE, &min, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, &max, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
#endif
       }
       else // CSの場合クライアントから受け取った1ステップ前の集約済minmaxでサイズ計算
       {
           max = m_size_max;
           min = m_size_min;
       }

       float factor =0;
        if (max - min > 1e-6 ) 
        {
            factor = 1/ (max - min) ;
        }
        else  
        {
            factor = 1;
        }

       for (int i = 0; i < nPoints; i++)
       {
           m_glyph_sizes[ i] = (m_glyph_sizes[ i ] - min )*factor;
           m_glyph_sizes[ i] = vismodule::Math::Clamp<float>( m_glyph_sizes[ i], 0.0, 1.0 );
       }

       //minmax の登録. IS は集約したもの CSは保存したもの
       if(m_is_flag)
       {
           m_size_min = min;
           m_size_max = max;
       }
       else
       {
           m_size_min = tmp_min;
           m_size_max = tmp_max;
       }
   }

   //color 
   float glyph_color_data_max = FLT_MIN;
   float glyph_color_data_min = FLT_MAX;
   if (m_color_sampling_method == DataDefines::Constant)
   {
       // 色が白になるよう設定
       for (int ii=0;ii < nPoints; ii++)
       {
           m_glyph_colors[3*ii    ] = 255 ;
           m_glyph_colors[3*ii +1 ] = 255 ;
           m_glyph_colors[3*ii +2 ] = 255 ;
       }

   }
   else if (m_color_sampling_method == DataDefines::VariableArray ) 
   {
       std::vector<int> color_var = m_color_data_variables;
       int n_color_variables=color_var.size();

       int glyph_count =0;
       std::vector<float> tmp_size(nPoints);
       for (int i = 0; i < m_ncoords; i+= stride)
       {
           for(int k = 0 ; k< n_color_variables ; k++)
           {
            tmp_size[ glyph_count ] += vismodule::Math::Square(m_values[color_var[k]][ i ]) ;
           }
           m_glyph_colors_data[ glyph_count ] = std::sqrt(tmp_size[ glyph_count ]);
           glyph_count++;
       }

       int n_color_data=m_glyph_colors_data.size();
       float max=FLT_MIN;
       float min=FLT_MAX;
       for(int k = 0; k< n_color_data; k++)
       {
           max = vismodule::Math::Max(m_glyph_colors_data[k], max ); 
           min = vismodule::Math::Min(m_glyph_colors_data[k], min ); 
       }

       if(m_is_flag)//IS の場合、全領域のminmaxをここで集約する
       {
#ifndef CPU_VER
           MPI_Allreduce( MPI_IN_PLACE, &min, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
           MPI_Allreduce( MPI_IN_PLACE, &max, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
#endif
           m_color_map.setRange(min, max);
       }
       else // CSの場合クライアントから受け取った1ステップ前の集約済minmaxで色設定
       {
           m_color_map.setRange(m_color_min, m_color_max);
       }

       for (int ii=0;ii < nPoints; ii++)
       {
           vismodule::RGBColor colors; 
           colors = m_color_map.at(m_glyph_colors_data[ ii ]);
           m_glyph_colors[3*ii    ] = colors.r() ;
           m_glyph_colors[3*ii +1 ] = colors.g() ;
           m_glyph_colors[3*ii +2 ] = colors.b() ;
       }
        // minmax値の更新
        m_color_min = min;
        m_color_max = max;
    } // DataDefines::VariableArray

//    this -> show();
}
#endif

void GlyphSeed::PointSampling_struct(domain_parameters_struct dom)
{
    const int stride = m_stride;
    int nPoints = m_ncoords/stride ;
    if( m_ncoords%stride >0 ) nPoints += 1;
    
    m_glyph_coords.resize(nPoints * 3);
    m_glyph_vectors.resize(nPoints * 3);
    m_glyph_sizes.resize(nPoints);
    m_glyph_colors_data.resize( nPoints );
    m_glyph_colors.resize( nPoints * 3); 

    const vismodule::Vector3ui resolution(dom.resolution[0],dom.resolution[1],dom.resolution[2] );
    //const vismodule::Vector3ui resolution( object->resolution() );
    const int nx = resolution.x();
    const int ny = resolution.y();
    const int nz = resolution.z();
    const int nxy = nx * ny;
    const int nx_1 = nx-1;
    const int ny_1 = ny-1;
    const int nz_1 = nz-1;
    const int nxy_1 = nx_1 * ny_1;

    m_ncells = nxy_1*nz_1;

    //const vismodule::Vector3f min_vec = object->minObjectCoord(); 
    //const vismodule::Vector3f max_vec = object->maxObjectCoord(); 
       const vismodule::Vector3f min_vec( 
                dom.x_min, 
                dom.y_min, 
                dom.z_min); 
       const  vismodule::Vector3f max_vec( 
                dom.x_max, 
                dom.y_max, 
                dom.z_max ); 

    const vismodule::Vector3f cell_length( (max_vec.x() - min_vec.x() )/ nx_1,
            (max_vec.y() - min_vec.y() )/ ny_1,
            (max_vec.z() - min_vec.z() )/ nz_1) ;

    // coord & vector 
   std::vector<int> vector_var = m_direction_variables;
   int glyph_count =0;
    //#pragma omp for
    for ( vismodule::UInt32 z = 0; z < nz; ++z )
    {
        for ( vismodule::UInt32 y = 0; y < ny; ++y )
        {
            for ( vismodule::UInt32 x = 0; x < nx; ++x )
            {
                const int index = x + y*nx + z*nx*ny;
                if (index % stride == 0)
                {
                    const float x_g = ((float)x * cell_length.x())+min_vec.x();
                    const float y_g = ((float)y * cell_length.y())+min_vec.y();
                    const float z_g = ((float)z * cell_length.z())+min_vec.z();

                    m_glyph_coords[3*glyph_count    ] = x_g;
                    m_glyph_coords[3*glyph_count +1 ] = y_g;
                    m_glyph_coords[3*glyph_count +2 ] = z_g;
                    m_glyph_vectors[3*glyph_count    ] = m_values[vector_var[0]][index ];
                    m_glyph_vectors[3*glyph_count +1 ] = m_values[vector_var[1]][index ];
                    m_glyph_vectors[3*glyph_count +2 ] = m_values[vector_var[2]][index ];
                    glyph_count++;
                }
            }
        }
    }

   //size
   if (m_size_sampling_method == DataDefines::Constant)
   {
       std::fill(m_glyph_sizes.begin(), m_glyph_sizes.end() ,1);
   }
   else if ( m_size_sampling_method == DataDefines::VariableArray ) 
   {
       std::vector<float> tmp_size(nPoints);
       std::vector<int> size_var = m_size_variables;
       int n_size_variables=m_size_variables.size();
       int glyph_count =0;
       for ( vismodule::UInt32 z = 0; z < nz; ++z )
       {
           for ( vismodule::UInt32 y = 0; y < ny; ++y )
           {
               for ( vismodule::UInt32 x = 0; x < nx; ++x )
               {
                   const int index = x + y*nx + z*nx*ny;
                   if (index % stride == 0)
                   {
                       for(int k = 0 ; k< n_size_variables ; k++)
                       {
                           tmp_size[ glyph_count ] += vismodule::Math::Square( m_values[ size_var[k] ][index] ); 
                       }
                       m_glyph_sizes[ glyph_count ] = std::sqrt(tmp_size[ glyph_count ]) ;
                       glyph_count++;
                   }
               }
           }
       }

        int n_size_data=m_glyph_sizes.size();
        // size計算用
        float max=FLT_MIN;
        float min=FLT_MAX;
        // 一時保存用
        float tmp_max=FLT_MIN;
        float tmp_min=FLT_MAX;
        for(int k = 0; k< n_size_data; k++)
        {
            max = vismodule::Math::Max(m_glyph_sizes[k], max ); 
            min = vismodule::Math::Min(m_glyph_sizes[k], min ); 
        }
        tmp_max = max;
        tmp_min = min;

        if(m_is_flag)//IS の場合、全領域のminmaxをここで集約する
        {
#ifndef CPU_VER
            MPI_Allreduce( MPI_IN_PLACE, &min, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
            MPI_Allreduce( MPI_IN_PLACE, &max, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
#endif
        }
        else // CSの場合クライアントから受け取った1ステップ前の集約済minmaxでサイズ計算
        {
            max = m_size_max;
            min = m_size_min;
        }

        float factor =0;
        if (max - min > 1e-6 ) 
        {
            factor = 1/ (max - min) ;
        }
        else  
        {
            factor = 1;
        }

       for (int i = 0; i < nPoints; i++)
       {
           m_glyph_sizes[ i] = (m_glyph_sizes[ i ] - min )*factor;
           m_glyph_sizes[ i] = vismodule::Math::Clamp<float>( m_glyph_sizes[ i], 0.0, 1.0 );
       }
       //minmax の登録. IS は集約したもの CSは保存したもの
       if(m_is_flag)
       {
           m_size_min = min;
           m_size_max = max;
       }
       else
       {
           m_size_min = tmp_min;
           m_size_max = tmp_max;
       }

   }

   //color 
   float glyph_color_data_max = FLT_MIN;
   float glyph_color_data_min = FLT_MAX;
   if (m_color_sampling_method == DataDefines::Constant)
   {
       for (int ii=0;ii < nPoints; ii++)
       {
           m_glyph_colors[3*ii    ] = 255 ;
           m_glyph_colors[3*ii +1 ] = 255 ;
           m_glyph_colors[3*ii +2 ] = 255 ;
       }
   }
   else if ( m_color_sampling_method == DataDefines::VariableArray ) 
   {
       std::vector<int> color_var = m_color_data_variables;
       int n_color_variables=color_var.size();

       int glyph_count =0;
       std::vector<float> tmp_size(nPoints);
       for ( vismodule::UInt32 z = 0; z < nz; ++z )
       {
           for ( vismodule::UInt32 y = 0; y < ny; ++y )
           {
               for ( vismodule::UInt32 x = 0; x < nx; ++x )
               {
                   const int index = x + y*nx + z*nx*ny;
                   if (index % stride == 0)
                   {
                       for(int k = 0 ; k< n_color_variables ; k++)
                       {
                           tmp_size[ glyph_count ] += vismodule::Math::Square(m_values[color_var[k]][ index ]) ;
                       }
                       m_glyph_colors_data[ glyph_count ] = std::sqrt(tmp_size[ glyph_count ]);
                       glyph_count++;
                   }
               }
           }
       }

//   各分割領域内での最大最小値の算出
       int n_color_data=m_glyph_colors_data.size();
       float max=FLT_MIN;
       float min=FLT_MAX;

       for(int k = 0; k< n_color_data; k++)
       {
           max = vismodule::Math::Max(m_glyph_colors_data[k], max ); 
           min = vismodule::Math::Min(m_glyph_colors_data[k], min ); 
       }

       if(m_is_flag)//IS の場合、全領域のminmaxをここで集約する
       {
#ifndef CPU_VER
           MPI_Allreduce( MPI_IN_PLACE, &min, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
           MPI_Allreduce( MPI_IN_PLACE, &max, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
#endif
           m_color_map.setRange(min, max);
       }
       else // CSの場合クライアントから受け取った1ステップ前の集約済minmaxで色設定
       {
           m_color_map.setRange(m_color_min, m_color_max);
       }

        for (int ii=0;ii < nPoints; ii++)
        {
            vismodule::RGBColor colors; 
            colors = m_color_map.at(m_glyph_colors_data[ ii ]);
            m_glyph_colors[3*ii    ] = colors.r() ;
            m_glyph_colors[3*ii +1 ] = colors.g() ;
            m_glyph_colors[3*ii +2 ] = colors.b() ;
        }

        //minmaxの更新
        m_color_min = min;
        m_color_max = max;
   }

//    this -> show();
}

void GlyphSeed::DistributionSampling_unstruct( const vismodule::VolumeObjectBase::CellType& celltype)
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
    vismodule::Timer timer( vismodule::Timer::Start );

    std::vector< std::vector< vismodule::CellBase<Type>* > >  interp;
    interp.resize( max_threads );

    switch ( celltype )
    {
        case vismodule::VolumeObjectBase::Tetrahedra:
            {
                if (mpi_rank == 0) std::cout << "celltype: tetrahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( m_nvariable );
                    for ( int j = 0; j < m_nvariable; j++ )
                    {
                        interp[i][j]  = new vismodule::TetrahedralCell<Type>( m_values[j], m_coords, m_ncoords, m_connections, m_ncells );
                    }
                }
                break;
            }
        case vismodule::VolumeObjectBase::QuadraticTetrahedra:
            {
                if (mpi_rank == 0) std::cout << "Cell type : Quadratic tetrahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( m_nvariable );
                    for ( int j = 0; j < m_nvariable; j++ )
                    {
                        interp[i][j]  = new vismodule::QuadraticTetrahedralCell<Type>( m_values[j], m_coords, m_ncoords, m_connections, m_ncells );
                    }
                }
                break;
            }
        case vismodule::VolumeObjectBase::Hexahedra:
            {
                if (mpi_rank == 0) std::cout << "Cell type : Hexahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( m_nvariable  );
                    for ( int j = 0; j < m_nvariable; j++ )
                    {
                        interp[i][j]  = new vismodule::HexahedralCell<Type>( m_values[j], m_coords, m_ncoords, m_connections, m_ncells );
                    }
                }
                break;
            }
        case vismodule::VolumeObjectBase::QuadraticHexahedra:
            {
                if (mpi_rank == 0) std::cout << "Cell type : Quadratic hexahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( m_nvariable );
                    for ( int j = 0; j < m_nvariable; j++ )
                    {
                        interp[i][j]  = new vismodule::QuadraticHexahedralCell<Type>( m_values[j], m_coords, m_ncoords, m_connections, m_ncells );
                    }
                }
                break;
            }
        case vismodule::VolumeObjectBase::Prism:
            {
                if (mpi_rank == 0) std::cout << "celltype: Prism " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( m_nvariable );
                    for ( int j = 0; j < m_nvariable; j++ )
                    {
                        interp[i][j]  = new vismodule::PrismaticCell<Type>( m_values[j], m_coords, m_ncoords, m_connections, m_ncells );
                    }
                }
                break;
            }
        case vismodule::VolumeObjectBase::Pyramid:
            {
                if (mpi_rank == 0) std::cout << "celltype: Pyramid" << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( m_nvariable );
                    for ( int j = 0; j < m_nvariable; j++ )
                    {
                        interp[i][j]  = new vismodule::PyramidalCell<Type>( m_values[j], m_coords, m_ncoords, m_connections, m_ncells );
                    }
                }
                break;
            }
        default:
            {
                //BaseClass::m_is_success = false;
                //visModuleMessageError( "Unsupported cell type." );
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
        vismodule::MersenneTwister MT( seed*mpi_size*nthreads + (mpi_rank+1)*thid );

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

        density = m_number_of_sample_points/TotalVolume;

        //粒子生成ループ開始
//#pragma omp for schedule( dynamic ) nowait
#pragma omp parallel for
        for( int cell_base = 0; cell_base < m_ncells; cell_base ++ )
        {

            vismodule::Vector3f coord = interp[thid][0]->localGravityPoint();
            interp[thid][0]->bindCell( cell_base);
            interp[thid][0]->setLocalPoint( coord );

            //生成glyph数を計算
            int  nglyphs = calculate_number_of_particles( density, interp[thid][0]->volume(), &MT ) ;
                
            for( int i = 0; i < nglyphs; i++ )
            {
                vismodule::Vector3f local_coord = interp[thid][0] -> randomSampling_MT( &MT );

                //補間器にセルを一括でバインド
                for( int k = 0; k < m_nvariable; k++ )
                {
                    interp[thid][k]->bindCell( cell_base );
                    interp[thid][k]->setLocalPoint( local_coord );
                }

                vismodule::Vector3f global_coord = interp[thid][0]->transformLocalToGlobal( local_coord );

                // glyph_vectorの計算
                // float scalar_array[interp[thid].size()];
                std::vector<float> scalar_array(interp[thid].size());
                float eval_result =0;

                for( std::size_t j= 0; j < m_nvariable; j++ )
                {
                    scalar_array[j] = interp[thid][j]->scalar();
                }


                //サイズ計算
                float size = 0;
                if ( m_size_sampling_method == DataDefines::VariableArray )
                { 
                    // float scalar_array[interp[thid].size()];
                    float eval_result =0;

                    std::vector<int> size_var = m_size_variables;
                    int n_size_data=size_var.size();
                    for( std::size_t j= 0; j < n_size_data; j++ ) 
                    {
                        eval_result += scalar_array[size_var[j]] * scalar_array[size_var[j]] ; 
                    }
                    size = std::sqrt(eval_result) ; 
                }
                else size = 1.f;

                //色の計算
                float color_data =0; 

                if ( m_color_sampling_method == DataDefines::VariableArray )
                { 
                    float eval_result =0;

                    std::vector<int> color_var = m_color_data_variables;
                    int n_color_data=color_var.size();
                    for( std::size_t j= 0; j < n_color_data; j++ )
                    {
                        eval_result += scalar_array[color_var[j]] * scalar_array[color_var[j]] ; 
                    }
                    color_data = std::sqrt(eval_result) ; 
                }
                else color_data = 0; 

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

    //sizeのminmaxを計算する
    if ( m_size_sampling_method == DataDefines::VariableArray )
    { 
        int n_size_data=m_glyph_sizes.size();
        float max=FLT_MIN;
        float min=FLT_MAX;
        float tmp_max=FLT_MIN;
        float tmp_min=FLT_MAX;
        for(int k = 0; k< n_size_data; k++)
        {
            max = vismodule::Math::Max(m_glyph_sizes[k], max ); 
            min = vismodule::Math::Min(m_glyph_sizes[k], min ); 
        }

        tmp_max = max;
        tmp_min = min;

        if(m_is_flag)//IS の場合、全領域のminmaxをここで集約する
        {
#ifndef CPU_VER
            MPI_Allreduce( MPI_IN_PLACE, &min, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
            MPI_Allreduce( MPI_IN_PLACE, &max, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
#endif
        }
        else // CSの場合クライアントから受け取った1ステップ前の集約済minmaxでサイズ計算
        {
            max = m_size_max;
            min = m_size_min;
            //デバッグ用
            std::cout << "m_size_max = " << m_size_max << std::endl;
            std::cout << "m_size_min = " << m_size_min << std::endl;
        }

        float factor = 0;
        if (max - min > 1e-6)
        {
            factor = 1.f/ (max - min)  ;
            for( int j = 0; j < n_size_data; j++ )
            {
                m_glyph_sizes[j] = (m_glyph_sizes[j] - min)*factor;
                m_glyph_sizes[j] = vismodule::Math::Clamp<float>( m_glyph_sizes[j], 0.0, 1.0 );
            }
        }
        else
        {
            for( int j = 0; j < n_size_data; j++ )
            {
               m_glyph_sizes[j] = 1.f;
            }
        }
       //minmax の登録. IS は集約したもの CSは保存したもの
       if(m_is_flag)
       {
           m_size_min = min;
           m_size_max = max;
       }
       else
       {
           m_size_min = tmp_min;
           m_size_max = tmp_max;
       }

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

    if ( m_color_sampling_method == DataDefines::VariableArray )
    { 
        int n_color_data=m_glyph_colors_data.size();
        float max=FLT_MIN;
        float min=FLT_MAX;
        //グリフ間のminmaxを算出 (読み取ったファイル内のみ。全体のminmaxではない)
        for(int k = 0; k< n_color_data; k++)
        {
            max = vismodule::Math::Max(m_glyph_colors_data[k], max ); 
            min = vismodule::Math::Min(m_glyph_colors_data[k], min ); 
        }

        if(m_is_flag)//IS の場合、全領域のminmaxをここで集約する
        {
            std::cout << "min = " << min << std::endl;
            std::cout << "max = " << max << std::endl;
#ifndef CPU_VER
            MPI_Allreduce( MPI_IN_PLACE, &min, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
            MPI_Allreduce( MPI_IN_PLACE, &max, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
#endif
            m_color_map.setRange(min, max);
        }
        else // CSの場合クライアントから受け取った1ステップ前の集約済minmaxで色設定
        {
            std::cout << "m_color_min = " << m_color_min << std::endl;
            std::cout << "m_color_max = " << m_color_max << std::endl;
            m_color_map.setRange(m_color_min, m_color_max);
        }

        float diff = m_color_map.maxValue() - m_color_map.minValue();
//        std::cout << "m_color_map.maxValue() = " << m_color_map.maxValue() << std::endl;
//        std::cout << "m_color_map.minValue() = " << m_color_map.minValue() << std::endl;
//        std::cout << "m_glyph_colors_data.size = " << m_glyph_colors_data.size() << std::endl;
//        std::cout << "m_color_map.res() = " << m_color_map.resolution() << std::endl;

        if (diff > 1e-6)
        {
            for( int jx=0; jx<n_color_data; jx++)
            {
//                std::cout << "index = " << jx
//                    << " value = " << m_glyph_colors_data[jx]
//                    << std::endl;
                vismodule::RGBColor color;
                color = m_color_map.at( m_glyph_colors_data[jx] );
//                m_color_map.setRange(0, 1);
//                color = m_color_map.at( 0.5 );
                m_glyph_colors.push_back( color.r());
                m_glyph_colors.push_back( color.g());
                m_glyph_colors.push_back( color.b());
            }
        }
        else
        {
            // minmaxが等しい場合、グリフの色は全て最低値の色とする
            m_color_map.setRange(0, 1);
            vismodule::RGBColor color;
            color = m_color_map.at( 0 );
            for( int jx = 0; jx < n_color_data; jx++ )
            {
                m_glyph_colors.push_back( color.r() );
                m_glyph_colors.push_back( color.g() );
                m_glyph_colors.push_back( color.b() );
            }
        }
        // 各ファイルのminmaxを登録。集約処理はgenerate_glyph.cppで（関数外）
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

void GlyphSeed::DistributionSampling_struct(domain_parameters_struct dom, Type** values, int nvariables)
{
#if 1
    m_nvariable = nvariables;
    m_ncoords = dom.resolution[0]*dom.resolution[1]*dom.resolution[2];

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

    //const vismodule::Vector3ui resolution( object->resolution() );
    const vismodule::Vector3ui resolution(dom.resolution[0],dom.resolution[1],dom.resolution[2] );
    std::vector< std::vector< vismodule::TrilinearInterpolator* > >  interp;
    interp.resize( max_threads );
    for ( int i = 0; i < max_threads; i++ )
    {
        interp[ i ].resize( m_nvariable );
        for ( int j = 0; j < m_nvariable; j++ )
        {
             interp[i][j]  = new vismodule::TrilinearInterpolator( values[j], resolution);
        }
    }


    const int seed          = m_seed;

#pragma omp parallel
    {

#if _OPENMP
        int nthreads = omp_get_num_threads();
        int thid     = omp_get_thread_num();
#else
        int nthreads = 1;
        int thid     = 0;
#endif

//    timer.start();
    //nglyphs /= nthreads;
    vismodule::MersenneTwister MT( seed*mpi_size*nthreads + (mpi_rank+1)*thid );
    //vismodule::MersenneTwister MT( seed + (mpi_rank+1) );

    float TotalVolume = 0;
    float density = 0;
    // 動的な粒子データ配列
    std::vector<float> th_glyph_coords;
    std::vector<float> th_glyph_colors_data;
    std::vector<float> th_glyph_vectors;
    std::vector<float> th_glyph_sizes;
    const vismodule::Vector3ui ncells( resolution - vismodule::Vector3ui(1) );
//    const vismodule::Vector3f min_vec = object->minObjectCoord(); 
//    const vismodule::Vector3f max_vec = object->maxObjectCoord(); 
       const vismodule::Vector3f min_vec( 
                dom.x_min, 
                dom.y_min, 
                dom.z_min); 
       const  vismodule::Vector3f max_vec( 
                dom.x_max, 
                dom.y_max, 
                dom.z_max ); 

    const vismodule::Vector3f cell_length( (max_vec.x() - min_vec.x() )/ ncells.x(),
            (max_vec.y() - min_vec.y() )/ ncells.y(),
            (max_vec.z() - min_vec.z() )/ ncells.z()) ;

    // Generate particles for each cell.
    const float volume_of_cell = cell_length.x()*cell_length.y()*cell_length.z();
    TotalVolume = volume_of_cell*ncells.x()*ncells.y()*ncells.z();
    density = m_number_of_sample_points/TotalVolume;

#pragma omp parallel for
    for ( vismodule::Int32 z = 0; z < ncells.z(); ++z )
    {
        for ( vismodule::UInt32 y = 0; y < ncells.y(); ++y )
        {
            for ( vismodule::UInt32 x = 0; x < ncells.x(); ++x )
            {

                const float x_l = (float)x;
                const float y_l = (float)y;
                const float z_l = (float)z;

                const float x_g = (x_l * cell_length.x())+min_vec.x();
                const float y_g = (y_l * cell_length.y())+min_vec.y();
                const float z_g = (z_l * cell_length.z())+min_vec.z();
                // Calculate a density.
                int  nglyphs = calculate_number_of_particles( density, volume_of_cell, &MT ) ;
                const vismodule::Vector3f v( static_cast<float>(x_g), static_cast<float>(y_g), static_cast<float>(z_g) );

                for( int i = 0; i < nglyphs; i++ )
                {
                    const float x = (float)MT.rand();
                    const float y = (float)MT.rand();
                    const float z = (float)MT.rand();
                    const vismodule::Vector3f d( x, y, z );

                    const vismodule::Vector3f coord = v + d;

                    std::vector<float> scalar_array(interp[thid].size());
                    for( std::size_t j= 0; j < m_nvariable; j++ )
                    {
                        interp[thid][j]->attachPoint_woSIMD( coord );
                        scalar_array[j] = interp[thid][j]->scalar_woSIMD<float>();
                    }
                    //サイズ計算
                    float size = 0;
                    if ( m_size_sampling_method == DataDefines::VariableArray )
                    { 
                        float eval_result =0;

                        std::vector<int> size_var = m_size_variables;
                        int n_size_data=size_var.size();
                        for( std::size_t j= 0; j < n_size_data; j++ ) 
                        {
                            eval_result += scalar_array[size_var[j]] * scalar_array[size_var[j]] ; 
                        }
                        size = std::sqrt(eval_result) ; 
                    }
                    else size = 1.f;

                    //色の計算
                    float color_data =0; 

                    if ( m_color_sampling_method == DataDefines::VariableArray )
                    { 
                        // float scalar_array[interp[thid].size()];
                        float eval_result =0;
                        std::vector<int> color_var = m_color_data_variables;
                        int n_color_data=color_var.size();
                        for( std::size_t j= 0; j < n_color_data; j++ )
                        {
                            eval_result += scalar_array[color_var[j]] * scalar_array[color_var[j]] ; 
                        }
                        color_data = std::sqrt(eval_result) ; 
                    }
                    else color_data = 0; 

                    // 座標情報の格納
                    th_glyph_coords.push_back( coord.x() );
                    th_glyph_coords.push_back( coord.y() );
                    th_glyph_coords.push_back( coord.z() );
                    // サイズ情報の格納
                    th_glyph_sizes.push_back( size );
                    // 色用データ情報の格納
                    th_glyph_colors_data.push_back( color_data );
                    // direction情報の格納
                    th_glyph_vectors.push_back( scalar_array[m_direction_variables[0]] );
                    th_glyph_vectors.push_back( scalar_array[m_direction_variables[1]] );
                    th_glyph_vectors.push_back( scalar_array[m_direction_variables[2]] );
                } // end of glyphe loop

            } // end of 'x' loop
        } // end of 'y' loop
    } // end of 'z' loop

#pragma omp barrier
#pragma omp critical
        {
            m_glyph_coords.insert  ( m_glyph_coords.end(), th_glyph_coords.begin(), th_glyph_coords.end() );
            m_glyph_colors_data.insert  ( m_glyph_colors_data.end(), th_glyph_colors_data.begin(), th_glyph_colors_data.end() );
            m_glyph_sizes.insert   ( m_glyph_sizes.end(),  th_glyph_sizes.begin(),  th_glyph_sizes.end() );
            m_glyph_vectors.insert ( m_glyph_vectors.end(), th_glyph_vectors.begin(), th_glyph_vectors.end() );
        }
    
    }  // end of pragma omp

    if ( m_size_sampling_method == DataDefines::VariableArray )
    { 
        int n_size_data=m_glyph_sizes.size();
        //size計算用
        float max=FLT_MIN;
        float min=FLT_MAX;
        // 一時保存用
        float tmp_max=FLT_MIN;
        float tmp_min=FLT_MAX;
        for(int k = 0; k< n_size_data; k++)
        {
            max = vismodule::Math::Max(m_glyph_sizes[k], max ); 
            min = vismodule::Math::Min(m_glyph_sizes[k], min ); 
        }

        tmp_max = max;
        tmp_min = min;

        if(m_is_flag)//IS の場合、全領域のminmaxをここで集約する
        {
#ifndef CPU_VER
            MPI_Allreduce( MPI_IN_PLACE, &min, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
            MPI_Allreduce( MPI_IN_PLACE, &max, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
#endif
        }
        else // CSの場合クライアントから受け取った1ステップ前の集約済minmaxでサイズ計算
        {
            max = m_size_max;
            min = m_size_min;
            //デバッグ用
            std::cout << "m_size_max = " << m_size_max << std::endl;
            std::cout << "m_size_min = " << m_size_min << std::endl;
        }

        float factor = 0;
        if (max - min > 1e-6)
        {
            factor = 1.f/ (max - min)  ;
            for( int j = 0; j < n_size_data; j++ )
            {
                m_glyph_sizes[j] = (m_glyph_sizes[j] - min)*factor;
                m_glyph_sizes[j] = vismodule::Math::Clamp<float>( m_glyph_sizes[j], 0.0, 1.0 );
            }
        }
        else
        {
            for( int j = 0; j < n_size_data; j++ )
            {
               m_glyph_sizes[j] = 1.f;
            }
        }
       //minmax の登録. IS は集約したもの CSは保存したもの
        if(m_is_flag)
        {
            m_size_min = min;
            m_size_max = max;
        }
        else
        {
            m_size_min = tmp_min;
            m_size_max = tmp_max;
        }


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

    if ( m_color_sampling_method == DataDefines::VariableArray )
    { 
        int n_color_data=m_glyph_colors_data.size();
        float max=FLT_MIN;
        float min=FLT_MAX;
        for(int k = 0; k< n_color_data; k++)
        {
            max = vismodule::Math::Max(m_glyph_colors_data[k], max ); 
            min = vismodule::Math::Min(m_glyph_colors_data[k], min ); 
        }

        if(m_is_flag) //IS の場合、全領域のminmaxをここで集約する
        {
#ifndef CPU_VER
            MPI_Allreduce( MPI_IN_PLACE, &min, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
            MPI_Allreduce( MPI_IN_PLACE, &max, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
#endif
            m_color_map.setRange(min, max);
            std::cout << "min = " << min <<std::endl;
            std::cout << "max = " << max <<std::endl;
        }
        else // CSの場合クライアントから受け取った1ステップ前の集約済minmaxで色設定
        {
            m_color_map.setRange(m_color_min, m_color_max);
        std::cout << "m_color_min = " << m_color_min << std::endl;
        std::cout << "m_color_max = " << m_color_max << std::endl;
        }

        float diff = m_color_map.maxValue() - m_color_map.minValue();
//        std::cout << "m_color_map.maxValue() = " << m_color_map.maxValue() << std::endl;
//        std::cout << "m_color_map.minValue() = " << m_color_map.minValue() << std::endl;
//        std::cout << "m_glyph_colors_data.size = " << m_glyph_colors_data.size() << std::endl;
//        std::cout << "m_color_map.res() = " << m_color_map.resolution() << std::endl;

        if (diff > 1e-6)
        {
            for( int jx=0; jx<n_color_data; jx++)
            {
//                std::cout << "index = " << jx
//                    << " value = " << m_glyph_colors_data[jx]
//                    << std::endl;
                vismodule::RGBColor color;
                color = m_color_map.at( m_glyph_colors_data[jx] );
                m_glyph_colors.push_back( color.r());
                m_glyph_colors.push_back( color.g());
                m_glyph_colors.push_back( color.b());
            }
        }
        else
        {
            // minmaxが等しい場合、グリフの色は全て最低値の色とする
            m_color_map.setRange(0, 1);
            vismodule::RGBColor color;
            color = m_color_map.at( 0 );
            m_glyph_colors.push_back( color.r());
            m_glyph_colors.push_back( color.g());
            m_glyph_colors.push_back( color.b());
        }

//        for( int jx=0; jx<n_color_data; jx++)
//        {
//            vismodule::RGBColor color;
//            color = m_color_map.at( m_glyph_colors_data[jx] );
//            m_glyph_colors.push_back( color.r());
//            m_glyph_colors.push_back( color.g());
//            m_glyph_colors.push_back( color.b());
//        }
        // 各ファイルのminmaxを登録。集約処理はgenerate_glyph.cppで（関数外）
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

#endif
}

const std::size_t GlyphSeed::calculate_number_of_particles(
    const float density,
    const float volume_of_cell,
    vismodule::MersenneTwister* MT )
{
    const float N = density * volume_of_cell;
    const float R = MT->rand();

    std::size_t n = static_cast<size_t>( N );
    if ( N - n > R )
    {
        ++n;
    }

    return ( n );
}

void GlyphSeed::getGlyphData(vismodule::KVSMLObjectGlyph* other)
{
    vismodule::ValueArray<float> coords( m_glyph_coords  );
    vismodule::ValueArray<float> directions(m_glyph_vectors );
    vismodule::ValueArray<Byte>  colors( m_glyph_colors   );
    vismodule::ValueArray<float> sizes(  m_glyph_sizes  );
    other -> setCoords( coords );
    other -> setColors( colors );
    other -> setDirections( directions );
    other -> setSizes( sizes );
    other -> setColorMin(m_color_min);
    other -> setColorMax(m_color_max);
    other -> setSizeMin(m_size_min);
    other -> setSizeMax(m_size_max);
}


//void GlyphSeed::OutputGlyph( const  pbvr_parameters& particleBase, const int time_step)
void GlyphSeed::OutputGlyph( const int time_step)
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
    //vismodule::ValueArray<float> coords( particleBase.m_sample_coords );
    //vismodule::ValueArray<Byte>  colors( particleBase.m_sample_colors );
    //vismodule::ValueArray<float> normals(particleBase.m_sample_normals );
    vismodule::ValueArray<float> coords( m_glyph_coords  );
    vismodule::ValueArray<float> vectors(m_glyph_vectors );
    vismodule::ValueArray<Byte>  colors( m_glyph_colors   );
    vismodule::ValueArray<float> sizes(  m_glyph_sizes  );

    static bool first_step = true;
    // static MPI_Comm new_comm;
    static int count;
    static int num_nodes;

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
        //vismodule::PointObject* point_object = new vismodule::PointObject( new_coords, new_colors, new_normals, particleBase.m_subpixel_level );
        vismodule::PointObject* point_object = new vismodule::PointObject( coords, colors, vectors, sizes );
        point_object->setMinMaxObjectCoords( particleBase.m_min_vec, particleBase.m_max_vec );
        // If async_io is enabled, use worker thread to write kvsml data and state.txt
//        if (async_io_enabled){
//            vismodule::ParticleWriteThread* particle_write_thread =  &pwt;
//            particle_write_thread->join(true);
//            particle_write_thread->setPointObject( point_object );
//            particle_write_thread->setFilename(particleBase.m_ptcFilePath.c_str());
//            particle_write_thread->setTimestep(time_step ,particleBase.m_stateFilePath.c_str());
//            particle_write_thread->setStartTimestep(st_time_step); //add by shimomura 20240808
//            particle_write_thread->work(true);
//        }// If async_io is disabled, use vismodule::PointExporter here in main thread.
//        else{
            vismodule::KVSMLObjectPoint* kvsml_object = new vismodule::PointExporter<vismodule::KVSMLObjectPoint>( point_object );
            //kvsml_object->setWritingDataType( vismodule::KVSMLObjectPoint::ExternalBinary );
            kvsml_object->setWritingDataType( vismodule::KVSMLObjectPoint::ExternalAscii );
            kvsml_object->write( m_glyphFilePath.c_str() );
            delete kvsml_object;

//        }
        delete point_object;
//    }
    #else
            vismodule::KVSMLObjectGlyph kvsmlobject( coords, colors, vectors, sizes);
            kvsmlobject.write(m_glyphFilePath.c_str());
#endif


}


void GlyphSeed::show()
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

