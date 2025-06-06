void  search_file(*mvpl, param);
#if 1
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
                            std::cout << "単一ファイル" << std::endl;
                            mvpl.loadVtm( vtmfile );
                        }
                        // 連番ファイルの場合
                        else
                        {
                            std::cout << "連番ファイル" << std::endl;
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

