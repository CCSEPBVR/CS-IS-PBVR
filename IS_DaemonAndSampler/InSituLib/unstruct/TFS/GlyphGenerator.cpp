#include "GlyphGenerator.h"

GlyphGenerator::GlyphGenerator( GlyphMode mode, DataDefines size_DataDefines, DataDefines color_DataDefines,
           int  stride, int  seed, int number_of_sampling_point,
           Type** values, int nvariables,
           float* coordinates, int ncoords,
           unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltype,
           pbvr::TransferFunction& tf, TransferFunctionSynthesizer* tfs)  :
    m_values( values ), m_nvariable(nvariables), m_size_DataDefines(size_DataDefines),m_color_DataDefines(color_DataDefines), 
    m_coords( coordinates  ), m_ncoords( ncoords ),
    m_connections( connections ), m_ncells( ncells ), m_tfs(tfs),
    m_tf(tf)
{

    if( mode == GlyphMode:: All_points || mode == GlyphMode:: Every_points )
    {
        this->PointSampling( stride);
    }
    else if(mode == GlyphMode:: Uniform_distribution)
    {
        std::vector< std::vector< pbvr::CellBase<Type>* > >  interp;
        DistributionSampling( number_of_sampling_point, seed, celltype );
    }
}


//void GlyphGenerator::GlyphSampling(
//           domain_parameters dom,
//           Type** values, int nvariables,
//           float* coordinates, int ncoords,
//           unsigned int* connections, int ncells, const  pbvr::VolumeObjectBase::CellType& celltype )
//void GlyphGenerator::GlyphSampling( int stride, const  pbvr::VolumeObjectBase::CellType& celltype )
//{
//
//    int mode=0;
//    if( mode == GlyphMode:: All_points || mode == GlyphMode:: Every_points )
//    {
//        PointSampling(
//           //Type** values, int nvariables,
//           //float* coordinates, int ncoords,
//           int stride ); 
//    }
//    else if(mode = GlyphMode:: All_points)
//    {
//        std::vector< std::vector< pbvr::CellBase<Type>* > >  interp;
//        DistributionSampling();
//    } 
//
//}

void GlyphGenerator::PointSampling( int stride)
{

    int nPoints = m_ncoords/stride; 
    m_glyph_coords.resize(nPoints * 3);
    m_glyph_vectors.resize(nPoints * 3);
    m_glyph_sizes.resize(nPoints);
    m_glyph_colors_data.resize( nPoints );
    m_glyph_colors.resize( nPoints * 3); 

   //vertex by vertexでglyph_coord、glyph_vector、glyph_size、glyph_color_data、glyph_color
   //stabデータとして vector = 1,2,3, size = 4 , color_data =5 とする。

   for (int i=0;i < m_ncoords; i+= stride)
   {
      m_glyph_coords[3*i    ] = m_coords[3*i    ];
      m_glyph_coords[3*i +1 ] = m_coords[3*i +1 ];
      m_glyph_coords[3*i +2 ] = m_coords[3*i +2 ];

      m_glyph_vectors[3*i    ] = m_values[1][i ];
      m_glyph_vectors[3*i +1 ] = m_values[2][i ];
      m_glyph_vectors[3*i +2 ] = m_values[3][i ];
   
   }

   if (m_size_DataDefines == DataDefines::Constant)
   {
       fill(m_glyph_sizes.begin()      , m_glyph_sizes.begin()      , 1);
   }
   else if (m_size_DataDefines == DataDefines::SingleVariable) 
   {
       for (int i=0;i < m_ncoords; i+= stride)
       {
           m_glyph_sizes[ i ]       = m_values[4][ i ];
       }
   }
   else if (m_size_DataDefines == DataDefines::VariablesArray) 
   {
       std::vector<float> tmp_size(nPoints);
       for (int i=0;i < m_ncoords; i+= stride)
       {
           //for(k =0; k< nvar ; k++)
           //{
           // //tmp_size[ i ] += values[v[k]][ i ]*values[v[k]][ i ] ;
           // // 本来は　成分数はユーザー指定で変化するので、最初に二乗の和を取るようにする。平方根を一緒にとると、成分数の変化に対応できなさそう
           //}
           tmp_size[ i ] = m_values[4][ i ]*m_values[4][ i ] + m_values[5][ i ]*m_values[5][ i ]; 
           m_glyph_sizes[ i ] = std::sqrt(tmp_size[ i ]);
       }
   }

   float glyph_color_data_max = FLT_MIN;
   float glyph_color_data_min = FLT_MAX;
   if (m_color_DataDefines == DataDefines::Constant)
   {
       fill(m_glyph_colors_data.begin(), m_glyph_colors_data.begin(), 1);
   }
   else if (m_color_DataDefines == DataDefines::SingleVariable) 
   {
       for (int i=0;i < m_ncoords; i+= stride)
       {
           m_glyph_colors_data[ i ] = m_values[5][ i ];
           glyph_color_data_max =  glyph_color_data_max > m_glyph_colors_data[ i ] ? glyph_color_data_max : m_glyph_colors_data[ i ];
           glyph_color_data_min =  glyph_color_data_min < m_glyph_colors_data[ i ] ? glyph_color_data_min : m_glyph_colors_data[ i ];
       }
   }
   else if (m_color_DataDefines == DataDefines::VariablesArray) 
   {
       std::vector<float> tmp_size(nPoints);
       for (int i=0;i < m_ncoords; i+= stride)
       {
           //for(k =0; k< nvar ; k++)
           //{
           // //tmp_size[ i ] += values[v[k]][ i ]*values[v[k]][ i ] ;
           // // 本来は　成分数はユーザー指定で変化するので、最初に二乗の和を取るようにする。平方根を一緒にとると、成分数の変化に対応できなさそう
           //}
           tmp_size[ i ] = m_values[4][ i ]*m_values[4][ i ] + m_values[5][ i ]*m_values[5][ i ]; 
           m_glyph_colors_data[ i ] = std::sqrt(tmp_size[ i ]);
           glyph_color_data_max =  glyph_color_data_max > m_glyph_colors_data[ i ] ? glyph_color_data_max : m_glyph_colors_data[ i ];
           glyph_color_data_min =  glyph_color_data_min < m_glyph_colors_data[ i ] ? glyph_color_data_min : m_glyph_colors_data[ i ];
       }
   }

//    TransferFunctionSynthesizer* tfs;
//    std::vector<pbvr::TransferFunction>& tf;

    //pbvr::TransferFunction tf;
    m_tf.setRange( glyph_color_data_min, glyph_color_data_max );
    kvs::RGBColor colors; 
    for (int i=0;i < m_ncoords; i+= stride)
    {
//        m_glyph_colors[i] = tfs -> CalculateColor_glyph( m_glyph_colors_data[i], m_nvariable, m_tf ); 
        colors = m_tf.colorMap().at(m_glyph_colors_data[ i ]);
        m_glyph_colors[3*i    ] = colors.r() ;
        m_glyph_colors[3*i +1 ] = colors.g() ;
        m_glyph_colors[3*i +2 ] = colors.b() ;
    }

}

void GlyphGenerator::DistributionSampling(int TotalNumbers, int seed, const pbvr::VolumeObjectBase::CellType& celltype)
{
    float TotalVolume = 0;
    float density = 0;
    int nPoints = TotalNumbers; 
    m_glyph_coords.resize(nPoints * 3);
    m_glyph_vectors.resize(nPoints * 3);
    m_glyph_sizes.resize(nPoints);
    m_glyph_colors_data.resize( nPoints );
    m_glyph_colors.resize( nPoints * 3); 


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

        // 動的な粒子データ配列
        std::vector<float> th_vertex_coords;
        std::vector<Byte>  th_vertex_colors;
        std::vector<float> th_vertex_normals;
        std::vector<float> th_glyph_vector;
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
//        std::vector<float> o_scalars_array[ SIMD_BLK_SIZE ];
//        std::vector<float> c_scalars_array[ SIMD_BLK_SIZE ];
//
//        for (int i = 0; i < SIMD_BLK_SIZE; i++ )
//        {
//            o_scalars_array[i].resize( tf_number );
//            c_scalars_array[i].resize( tf_number );
//        }

        int nparticles_array[ SIMD_BLK_SIZE ];

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
        // -----------------------------------

        for( int cell_base = 0; cell_base < m_ncells; cell_base += SIMD_BLK_SIZE )
        {

            int remain = ( m_ncells - cell_base > SIMD_BLK_SIZE )? SIMD_BLK_SIZE: m_ncells - cell_base;
            //一括でセルをバインドするための配列と、座標の取得
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell_index[cell_BLK] = (kvs::UInt32)(cell_base + cell_BLK);
                local_center_array[cell_BLK] = kvs::Vector3f ( 0, 0, 0 );
                //local_center_array[cell_BLK] = interp[thid][0]->localGravityPoint();
            }

            //補間器にセルを一括でバインド
            for(int i = 0; i < m_nvariable; i++)
            {
                interp[thid][i]->bindCellArray(remain, cell_index);
            }
            TotalVolume += interp[thid][0]->volume();

        }

    density =TotalVolume/TotalNumbers;


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
                //local_center_array[cell_BLK] = kvs::Vector3f ( 0.5, 0.5, 0.5 );
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

            th_tfs[thid]->CalculateOpacityArrayAverage( interp[thid],
                                                 remain,
                                                 local_center_array,
                                                 global_center_array,
                                                 th_tf[thid],
                                                 cell_opacity_array );
             //生成粒子数を計算
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
//                    float density = Generator::CalculateDensity( cell_opacity_array[cell_BLK],
//                                                                       sampling_volume_inverse,
//                                                                       max_opacity, max_density );
//#ifdef REJECTION
//                    density             = cell_opacity_array[cell_BLK] < 0.0039 ? 0.0 : density; //  less than 1/256
//#endif
////                    const float density = cell_opacity_array[cell_BLK] < 0.003 ? 0.0 : max_density; //  less than 1/256
//                    interp[thid][0]->bindCell( cell_index[cell_BLK] );
                    nparticles_array[cell_BLK] 
                        = calculate_number_of_particles( density, interp[thid][0]->volume(), &MT ) ;

            }
        /////////////////////////////// Synthesized~ (), CalculateOpacity() ///////////////////////////////////
        /////////////////////////////// CalculateOpacity(), CalculateColor() ///////////////////////////////////
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                // ------------------------------------------------

                for( int i = 0; i < nparticles_array[cell_BLK]; i+=SIMD_BLK_SIZE )
                {
                    //ブロック内でのループ回数を取得
                    int remain_BLK = ( nparticles_array[cell_BLK] - i > SIMD_BLK_SIZE )
                                                        ? SIMD_BLK_SIZE: nparticles_array[cell_BLK] - i;
                    int nparticles_count = 0;
                    //一括でセルをバインドするための配列と、座標の取得
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        cell_index[j] = cell_base + cell_BLK;
                        local_coord_array[j] = interp[thid][0] -> randomSampling_MT( &MT );

                        //補間器にセルを一括でバインド
                        for( int k = 0; k < m_nvariable; k++ )
                        {
                            interp[thid][k]->bindCell( cell_index[j] );
                        }

                        interp[thid][0]->setLocalPoint( local_coord_array[j] );
                        global_coord_array[j] = interp[thid][0]->transformLocalToGlobal( local_coord_array[j] );
//                        cell_opacity_array[j] = th_tfs[thid]->CalculateOpacity( interp[thid],
//                                local_coord_array[j],
//                                global_coord_array[j],
//                                th_tf[thid]);

//                        density_array[j] = Generator::CalculateDensity( cell_opacity_array[j],
//                                sampling_volume_inverse,
//                                max_opacity, max_density );

                    //}
                    //densityの条件に適合するnparticlesの個数の取得
                    //そのときのcell_index, local_coordを再配置
                    //int nparticles_count = 0;
                    //for( int j = 0; j < remain_BLK; j++ )
                    //{
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

//                    // dsdx ----------------------------------------
//                    for( int j = 0; j < nparticles_count; j++ )
//                    {
//                        l_plus_coord[j] = local_coord_array[j] + kvs::Vector3f(0.1,0,0);
//                        l_minus_coord[j] = local_coord_array[j] + kvs::Vector3f(-0.1,0,0);
//
//                    }
//
//                    interp[thid][0]->setLocalPointArray( nparticles_count, l_plus_coord );
//                    interp[thid][0]->transformLocalToGlobalArray( nparticles_count,
//                                                                  l_plus_coord,
//                                                                  g_plus_coord );
//
//                    interp[thid][0]->setLocalPointArray( nparticles_count, l_minus_coord );
//                    interp[thid][0]->transformLocalToGlobalArray( nparticles_count,
//                                                                  l_minus_coord,
//                                                                  g_minus_coord );
//
//
//                    th_tfs[thid]->CalculateOpacityArray( interp[thid],
//                                                         nparticles_count,
//                                                         l_plus_coord,
//                                                         g_plus_coord,
//                                                         th_tf[thid],
//                                                         S_plus_opacity );
//                    th_tfs[thid]->CalculateOpacityArray( interp[thid],
//                                                         nparticles_count,
//                                                         l_minus_coord,
//                                                         g_minus_coord,
//                                                         th_tf[thid],
//                                                         S_minus_opacity );
//
//                    for( int j = 0; j < nparticles_count; j++ )
//                    {
//                        dsdx_array[j] = ( S_plus_opacity[j] - S_minus_opacity[j] )*5.0;
//                    }
//                // ------------------------------------------------
//
//                    // dsdy ----------------------------------------
//                    for( int j = 0; j < nparticles_count; j++ )
//                    {
//                        l_plus_coord[j] = local_coord_array[j] + kvs::Vector3f(0,0.1,0);
//                        l_minus_coord[j] = local_coord_array[j] + kvs::Vector3f(0,-0.1,0);
//                    }
//
//                    interp[thid][0]->setLocalPointArray( nparticles_count, l_plus_coord );
//                    interp[thid][0]->transformLocalToGlobalArray( nparticles_count,
//                                                                  l_plus_coord,
//                                                                  g_plus_coord );
//
//                    interp[thid][0]->setLocalPointArray( nparticles_count, l_minus_coord );
//                    interp[thid][0]->transformLocalToGlobalArray( nparticles_count,
//                                                                  l_minus_coord,
//                                                                  g_minus_coord );
//
//                    th_tfs[thid]->CalculateOpacityArray( interp[thid],
//                                                         nparticles_count,
//                                                         l_plus_coord,
//                                                         g_plus_coord,
//                                                         th_tf[thid],
//                                                         S_plus_opacity );
//                    th_tfs[thid]->CalculateOpacityArray( interp[thid],
//                                                         nparticles_count,
//                                                         l_minus_coord,
//                                                         g_minus_coord,
//                                                         th_tf[thid],
//                                                         S_minus_opacity );
//
//                    for( int j = 0; j < nparticles_count; j++ )
//                    {
//                        dsdy_array[j] = ( S_plus_opacity[j] - S_minus_opacity[j] )*5.0;
//                    }
//                // ------------------------------------------------
//                    // dsdz ----------------------------------------
//                    for( int j = 0; j < nparticles_count; j++ )
//                    {
//                        l_plus_coord[j] = local_coord_array[j] + kvs::Vector3f(0,0,0.1);
//                        l_minus_coord[j] = local_coord_array[j] + kvs::Vector3f(0,0,-0.1);
//                    }
//
//                    interp[thid][0]->setLocalPointArray( nparticles_count, l_plus_coord );
//                    interp[thid][0]->transformLocalToGlobalArray( nparticles_count,
//                                                                  l_plus_coord,
//                                                                  g_plus_coord );
//
//                    interp[thid][0]->setLocalPointArray( nparticles_count, l_minus_coord );
//                    interp[thid][0]->transformLocalToGlobalArray( nparticles_count,
//                                                                  l_minus_coord,
//                                                                  g_minus_coord );
//
//                    th_tfs[thid]->CalculateOpacityArray( interp[thid],
//                                                         nparticles_count,
//                                                         l_plus_coord,
//                                                         g_plus_coord,
//                                                         th_tf[thid],
//                                                         S_plus_opacity );
//                    th_tfs[thid]->CalculateOpacityArray( interp[thid],
//                                                         nparticles_count,
//                                                         l_minus_coord,
//                                                         g_minus_coord,
//                                                         th_tf[thid],
//                                                         S_minus_opacity );
//
//                    for( int j = 0; j < nparticles_count; j++ )
//                    {
//                        dsdz_array[j] = ( S_plus_opacity[j] - S_minus_opacity[j] )*5.0;
//                    }
//                // ------------------------------------------------
//                    //grad_arrayの算出
//                    for( int j = 0; j < nparticles_count; j++ )
//                    {
//                        //JacobiMatrixでメンバ変数を使用しているので再度バインド
//                        interp[thid][0]->bindCell( cell_index[j] );
//
//                        const kvs::Vector3f g( -dsdx_array[j], -dsdy_array[j], -dsdz_array[j] );
//                        const kvs::Matrix33f J = interp[thid][0]->JacobiMatrix();
//                        float determinant = 0.0f;
//                        const kvs::Vector3f G = J.inverse( &determinant ) * g;
//                        grad_array[j] = kvs::Math::IsZero( determinant ) ? kvs::Vector3f( 0.0f, 0.0f, 0.0f ) : G;
//                    }
                    //サイズ計算
                    if( m_size_DataDefines ==  DataDefines::Constant )
                    {
                        for( int j = 0; j < nparticles_count; j++ )
                        {
                            sizes_array[j] = 1;
                        }
                    }
                    else if (m_size_DataDefines ==  DataDefines::SingleVariable || m_size_DataDefines == DataDefines::VariablesArray )
                    { 
                        th_tfs[thid]->CalculateGlyphSizeArray( interp[thid],
                                nparticles_count,
                                local_coord_array,
                                global_coord_array,
                                th_tf[thid],
                                sizes_array );
                    }

                    //色の計算
                    if( m_color_DataDefines ==  DataDefines::Constant )
                    {
                        for( int j = 0; j < nparticles_count; j++ )
                        {
                              colors_array[j].set(0,0,0);
//                            colors_array[j].r() = 0;
//                            colors_array[j].g() = 0;
//                            colors_array[j].b() = 0;
                        }
                    }
                    else if (m_color_DataDefines ==  DataDefines::SingleVariable || m_size_DataDefines == DataDefines::VariablesArray )
                    { 
                        th_tfs[thid]->CalculateGlyphColorArray( interp[thid],
                                nparticles_count,
                                local_coord_array,
                                global_coord_array,
                                th_tf[thid],
                                colors_array );
                    }
                    // glyph_vectorの計算
                    th_tfs[thid]->CalculateGlyphVector( interp[thid],
                                                       nparticles_count,
                                                       local_coord_array,
                                                       global_coord_array,
                                                       glyph_vector_array );
 

                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        th_vertex_coords.push_back( global_coord_array[j].x() );
                        th_vertex_coords.push_back( global_coord_array[j].y() );
                        th_vertex_coords.push_back( global_coord_array[j].z() );

                        th_vertex_colors.push_back( colors_array[j].r() );
                        th_vertex_colors.push_back( colors_array[j].g() );
                        th_vertex_colors.push_back( colors_array[j].b() );

                        th_glyph_vector.push_back( glyph_vector_array[j].x() );
                        th_glyph_vector.push_back( glyph_vector_array[j].y() );
                        th_glyph_vector.push_back( glyph_vector_array[j].z() );
                    }
                // ------------------------------------------------
                
                }//end of for i
            }
        /////////////////////////////// CalculateOpacity(), CalculateColor() ///////////////////////////////////
        }// end of for cell
//            std::cout << __FUNCTION__  << ": " << __LINE__ << std::endl;
//        #pragma omp barrier
        #pragma omp critical
        {
            //vertex_coords.insert ( vertex_coords.end(), th_vertex_coords.begin(), th_vertex_coords.end() );
            //vertex_colors.insert ( vertex_colors.end(), th_vertex_colors.begin(), th_vertex_colors.end() );
            m_glyph_coords.insert ( m_glyph_coords.end(), th_vertex_coords.begin(), th_vertex_coords.end() );
            m_glyph_colors.insert ( m_glyph_colors.end(), th_vertex_colors.begin(), th_vertex_colors.end() );
        }

//                timer.stop();
    } //#pragma omp parallel


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



