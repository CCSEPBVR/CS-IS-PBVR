void generate_glytph() 
{
                    std::vector<GlyphObjectCreator> glyph_creator_lst;

                if ( clntMes.m_time_parameter == 0 )
                {
                }
                else if ( clntMes.m_time_parameter == 1 )
                {
                }
                else
                {
                    timer_count++;
//                  param.m_transfer_function = vismodule::TransferFunction(); // *( clntMes.m_transfer_function );
                    param.m_sampling_method = clntMes.m_sampling_method;
                    param.m_component_Id = clntMes.m_rendering_id;
                    param.m_crop.setEnable( clntMes.m_enable_crop_region );
                    param.m_crop.set( clntMes.m_crop_region );
                    param.m_input_data_base = clntMes.m_input_directory;
                    param.m_particle_limit = clntMes.m_particle_limit;
                    param.m_particle_density = clntMes.m_particle_density;

#if 0
                    std::string pfifile, pflfile;
                    pfifile = param.m_input_data_base + ".pfi";
                    vismodule::File pfi( pfifile );
                    pflfile = param.m_input_data_base + ".pfl";
                    vismodule::File pfl( pflfile );
                    if ( pfl.isExisted() )
                    {
                        mvpl.loadPFL( pflfile );
                    }
                    else if ( pfi.isExisted() )
                    {
                        mvpl.loadPFL( pfifile );
                    }
#else
                    size_t found_pfl  = param.m_input_data_base.find(".pfl");
                    size_t found_pfi  = param.m_input_data_base.find(".pfi");
                    size_t found_vtm  = param.m_input_data_base.find(".vtm");
                    size_t found_vtu  = param.m_input_data_base.find(".vtu");
                    size_t found_vti  = param.m_input_data_base.find(".vti");
                    size_t found_inp  = param.m_input_data_base.find(".inp");
                    size_t found_pvtu = param.m_input_data_base.find(".pvtu");
                    size_t found_case = param.m_input_data_base.find(".case");
		            if ( found_pfl != std::string::npos )
                    {
                        std::string pflfile = param.m_input_data_base;
                        std::cout << "pflファイルが選択されました" << std::endl;
                        vismodule::File pfl( pflfile );
                        if ( pfl.isExisted() )
                        {
                            mvpl.loadPFL( pflfile );
                        }
                    }
                    else if ( found_pfi != std::string::npos )
                    {
                        std::string pfifile = param.m_input_data_base;
                        std::cout << "pfiファイルが選択されました" << std::endl;
                        vismodule::File pfi( pfifile );
                        if ( pfi.isExisted() )
                        {
                            mvpl.loadPFL( pfifile );
                        }
                    }
#ifdef EXTEND_FILE_FORMAT 
                    else if ( found_vtm != std::string::npos )
                    {
                        std::string vtmfile = param.m_input_data_base;
                        std::cout << ".vtmファイルが選択されました" << std::endl;
                        size_t found_asterisk = vtmfile.find( '*' );

                        // 単一ファイルの場合
                        if ( found_asterisk == std::string::npos )
                        {
                            mvpl.loadVtm( vtmfile );
                        }
                        // 連番ファイルの場合
                        else
                        {
                            mvpl.loadSeriesVtm( vtmfile );
                        }
                    }
                    else if ( found_vtu != std::string::npos )
                    {
                        std::string vtufile = param.m_input_data_base;
                        std::cout << ".vtuファイルが選択されました" << std::endl;
                        size_t found_asterisk = vtufile.find( '*' );

                        // 単一ファイルの場合
                        if ( found_asterisk == std::string::npos )
                        {
                            mvpl.loadVtu( vtufile );
                        }
                        // 連番ファイルの場合
                        else
                        {
                            mvpl.loadSeriesVtu( vtufile );
                        }
                    } 
                    else if ( found_vti != std::string::npos )
                    {
                        std::string vtifile = param.m_input_data_base;
                        std::cout << ".vtiファイルが選択されました" << std::endl;
                        size_t found_asterisk = vtifile.find( '*' );

                        // 単一ファイルの場合
                        if ( found_asterisk == std::string::npos )
                        {
                            mvpl.loadVti( vtifile );
                        }
                        // 連番ファイルの場合
                        else
                        {
                            mvpl.loadSeriesVti( vtifile );
                        }
                    }
                    else if ( found_inp != std::string::npos )
                    {
                        std::string inpfile = param.m_input_data_base;
                        std::cout << ".inpファイルが選択されました" << std::endl;
                        size_t found_asterisk = inpfile.find( '*' );

                        // 単一ファイルの場合
                        if ( found_asterisk == std::string::npos )
                        {
                            mvpl.loadInp( inpfile );
                        }
                        // 連番ファイルの場合
                        else
                        {
                            std::cout << ".inpファイルは連番ファイルに対応していません" << std::endl;
                        }
                    }        
                    else if ( found_pvtu != std::string::npos )
                    {
                        std::string pvtufile = param.m_input_data_base;
                        std::cout << ".pvtuファイルが選択されました" << std::endl;
                        size_t found_asterisk = pvtufile.find( '*' );

                        // 単一ファイルの場合
                        if ( found_asterisk == std::string::npos )
                        {
                            mvpl.loadPvtu( pvtufile );
                        }
                        // 連番ファイルの場合
                        else
                        {
                            mvpl.loadSeriesPvtu( pvtufile );
                        }
                    }
                    else if ( found_case != std::string::npos )
                    {
                        std::string casefile = param.m_input_data_base;
                        std::cout << ".caseファイルが選択されました" << std::endl;
                        size_t found_asterisk = casefile.find( '*' );

                        // 単一ファイルの場合
                        if ( found_asterisk == std::string::npos )
                        {
                            mvpl.loadEnsightGold( casefile );
                        }
                        // 連番ファイルの場合
                        else
                        {
                            std::cout << ".caseファイルは連番ファイルに対応していません" << std::endl;
                        }
                    }                    
#endif
                    else
                    {
                        std::cout << "このファイルは現在対応していません" << std::endl;
                    }
#endif

                    glyph_creator_lst.clear();
                    for ( int idx = 0; idx < mvpl.m_list.size(); idx++ )
                    {
                        GlyphObjectCreator glyph_creator;
                        glyph_creator.setFilterInfo( mvpl.m_list[idx] );

//                        point_creator.setFilterInfo( mvpl.m_list[idx] );
//                        glyph_creator.setCoordSynthStr( clntMes.m_x_synthesis,
//                                                        clntMes.m_y_synthesis, clntMes.m_z_synthesis );
//                        point_creator.setCoordSynthTkn( clntMes.x_synthesis_token,
//                                                        clntMes.y_synthesis_token, clntMes.z_synthesis_token );
                        glyph_creator_lst.push_back( glyph_creator );
                    }

                    transfunc_creator.setFilterInfo( mvpl.m_list[0] );
                    Calculate_minmax_glyph( param, mvpl, clntMes);
                    transfunc_creator.setProtocol( clntMes );
                    transfunc_creator.setAsisTransferFunction( param.m_transfer_function );
                    param.m_transfunc_synthesizer = transfunc_creator.create();
//
                    param.m_transfunc_array.resize(transfunc_creator.transfunc().size());
                    for(int i = 0; i<transfunc_creator.transfunc().size(); i++ )
                    {
                        param.m_transfunc_array[i]       = static_cast<vismodule::TransferFunction>(transfunc_creator.transfunc()[i]);
                    }

//                    if ( !param.hasOption( "L" ) ) param.m_latency_threshold = -1.0;
                    if ( param.m_crop.isEnabled() )
                    {
                        jd.initialize( clntMes.m_step, clntMes.m_step, mvpl.m_total_number_subvolumes,
                                       mvpl.m_total_min_subvolume_coord,
                                       mvpl.m_total_max_subvolume_coord,
                                       param.m_latency_threshold, param.m_job_id_pack_size,
                                       param.m_crop.getMinCoord(),
                                       param.m_crop.getMaxCoord() );
                    }
                    else
                    {
                        jd.initialize( clntMes.m_step, clntMes.m_step, mvpl.m_total_number_subvolumes,
                                       mvpl.m_total_min_subvolume_coord,
                                       mvpl.m_total_max_subvolume_coord,
                                       param.m_latency_threshold, param.m_job_id_pack_size );
                    }

                    param.m_sampling_step = CalculateSamplingStep( mvpl );
                    //param.m_sampling_step = 1;
                    param.m_subpixel_level = CalculateSubpixelLevel( param, mvpl, *clntMes.m_camera );
                    param.m_particle_limit_pre = param.m_particle_limit;
                    
                    int cnt = 2 ;
                    tmp_max = new float[cnt]; 
                    tmp_min = new float[cnt]; 

                    for ( int tf = 0; tf < cnt; tf++ )
                    {
                        tmp_max[tf] = FLT_MIN;
                        tmp_min[tf] = FLT_MAX;
                    }
 
                    while ( jd.dispatchNext( wid, &st, &vl ) )
                    {
                        int xvl, fidx;
                        fidx = mvpl.getFileIndex( vl, &xvl );
                        MultiVolumeProperty&mvp = mvpl.m_list[fidx];

                        size_t found_pfi  = mvp.m_file_path.find(".pfi");
                        size_t found_vtm  = mvp.m_file_path.find(".vtm");
                        size_t found_vtu  = mvp.m_file_path.find(".vtu");
                        size_t found_vti  = mvp.m_file_path.find(".vti");
                        size_t found_inp  = mvp.m_file_path.find(".inp");
                        size_t found_pvtu = mvp.m_file_path.find(".pvtu");
                        size_t found_case = mvp.m_file_path.find(".case");

                        if ( found_pfi != std::string::npos )
                        {
                            std::stringstream suffix;
                            suffix << '_' << std::setw( 5 ) << std::setfill( '0' ) << ( st )
                            << '_' << std::setw( 7 ) << std::setfill( '0' ) << ( xvl + 1 )
                            << '_' << std::setw( 7 ) << std::setfill( '0' ) << mvp.m_number_subvolumes;
                            vismodule::File ifpx( mvp.m_file_path );
                            param.m_input_data = ifpx.pathName() + ifpx.Separator()
                            + ifpx.baseName() + suffix.str() + ".kvsml";
                        }
#ifdef EXTEND_FILE_FORMAT 
                        else if ( found_vtm  != std::string::npos ||
                                  found_vtu  != std::string::npos ||
                                  found_vti  != std::string::npos ||
                                  found_inp  != std::string::npos ||
                                  found_pvtu != std::string::npos ||
                                  found_case != std::string::npos 
                                )
                        {
                            param.m_input_data = mvp.m_file_path;
                        }
#endif
                        else
                        {
                            std::cout << "このファイルは現在対応していません" << std::endl;
                        }

                        vismodule::KVSMLObjectGlyph* tmp_obj = new vismodule::KVSMLObjectGlyph;
                        param.m_subvolume_id = xvl ;
                        int timeStep = 1;
                        try
                        {
                            if ( mvp.m_file_type == 1 || mvp.m_file_type == 2 ) // filetype: gathered subvolume or gathered timestep
                            {
                                *tmp_obj = *glyph_creator_lst[fidx].run( param, *clntMes.m_camera, clntMes, mvpl.m_total_number_subvolumes, timeStep, st, xvl); 

                            }
#ifdef EXTEND_FILE_FORMAT
                            else if ( mvp.m_file_type == 3 || mvp.m_file_type == 4 )
                            {
                                glyph_creator_lst[fidx].run( param, *clntMes.m_camera, clntMes, mvpl.m_total_number_subvolumes, timeStep , tmp_obj, st, xvl );
                            }                                
#endif
                            else     // filetype: kvsml
                            {
                                glyph_creator_lst[fidx].run( param, *clntMes.m_camera, clntMes, mvpl.m_total_number_subvolumes, timeStep, tmp_obj, st );
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
#ifndef CPU_VER
//                        VariableRange* p_vr = &param.m_transfunc_synthesizer->variableRange();
//                        jc.jobCollect( tmp_obj, p_vr, &nan_error, &wid );
                        jc.jobCollect_glyph( tmp_obj, &nan_error, &wid );
#endif
                        if ( nan_error )
                        {
                            nan_error = false;
                            continue;
                        }
                        for ( int tf = 0; tf < cnt/2; tf++ )
                        {
                            //add by shimomura 2023/06/14
                            tmp_max[2*tf+1] = vismodule::Math::Max( tmp_max[2*tf+1] ,tmp_obj->colorMax());
                            tmp_min[2*tf+1] = vismodule::Math::Min( tmp_min[2*tf+1] ,tmp_obj->colorMin());
                            tmp_max[2*tf]   = vismodule::Math::Max( tmp_max[2*tf]   ,tmp_obj->sizeMax());
                            tmp_min[2*tf]   = vismodule::Math::Min( tmp_min[2*tf]   ,tmp_obj->sizeMin());
                        }



                    } // end of while(DispatchNext)
#ifndef CPU_VER

                    MPI_Allreduce( MPI_IN_PLACE, tmp_max, cnt, MPI_FLOAT, MPI_MAX , MPI_COMM_WORLD );
                    MPI_Allreduce( MPI_IN_PLACE, tmp_min, cnt, MPI_FLOAT, MPI_MIN , MPI_COMM_WORLD );
#endif
                    //add by shimomura 20250213
                    delete[] tmp_max;
                    delete[] tmp_min;

                    if ( timer_count == VIS_MODULE_TIMER_COUNT_NUM )
                    {
                        VIS_MODULE_TIMER_END( 1 );
                        VIS_MODULE_TIMER_FIN();
                    }
                    delete param.m_transfunc_synthesizer;
                }
               

}
