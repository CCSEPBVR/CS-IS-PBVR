#include "PlotOverLineGenerator.h"
//#include <sys/time.h>
#include <vismodule/TransferFunction>
#include "UnstructuredVolumeObject.h"
#include <vismodule/UnstructuredVolumeImporter>
//#include "CellByCellLayeredSampling.h"
#include <vismodule/Camera>
#if 0 //TEST_DELETE
#include <vismodule/TestVolume>
#include <vismodule/FrontSTRFileReader>
#endif
#include <vismodule/AVSUcd>
#include <vismodule/ValueArray>
#include <vismodule/File>

#include <vismodule/FileChecker>
#include "StructuredVolumeObject.h"
#include <vismodule/StructuredVolumeImporter>

#include <vismodule/Argument>

#include <vismodule/timer_simple>

using namespace vismodule;

//void PlotOverLineGenerator::createFromFile( const Argument& param, const vismodule::Camera& camera )
void PlotOverLineGenerator::createFromFile( const Argument& param, const vismodule::Camera& camera, const jpv::ParticleTransferClientMessage &clntMes, const int number_of_divide )
{
//FJ_TIMER_KAWAMURA
    VIS_MODULE_TIMER_STA( 260 );
//FJ_TIMER_KAWAMURA

    // add by shimomura 2023/0407
    vismodule::VolumeObjectBase* volume = nullptr;
    if ( vismoduleview::FileChecker::ImportableStructuredVolume( param.m_input_data ))
    {
        std::cout << "Structured !" <<std::endl;
        volume = new vismodule::StructuredVolumeImporter( param.m_input_data ); 
//        visModuleMessageError("structured data does not apply." );
        int id = param.m_subvolume_id;
        volume->updateMinMaxValues();
        volume->setMinMaxObjectCoords( m_mvp->m_min_subvolume_coord[id], m_mvp->m_max_subvolume_coord[id] );
    } 
    else if ( vismoduleview::FileChecker::ImportableUnstructuredVolume( param.m_input_data))
    {
        std::cout << "Unstructured !" <<std::endl;
        volume = new vismodule::UnstructuredVolumeImporter( param.m_input_data );  
    }
    else 
    {
        visModuleMessageError("%s is not volume data.", param.m_input_data.c_str());
    }

//FJ_TIMER_KAWAMURA
    VIS_MODULE_TIMER_END( 260 );
//FJ_TIMER_KAWAMURA

    volume->updateMinMaxValues();
    std::cout << *volume << std::endl;
    std::cout << "min:" << volume->minObjectCoord() << ", max:" << volume->maxObjectCoord() << std::endl;
    std::cout << "min:" << volume->minExternalCoord() << ", max:" << volume->maxExternalCoord() << std::endl;

    vismodule::VolumeObjectBase::VolumeType voltype = volume->volumeType();

    //詰め替え処理
    vismodule::AnyValueArray valueArray; 
    valueArray = volume->values(); 
    int nnodes = volume->nnodes();
    int nvariables = volume->veclen();

    // ここで変数の値をfloatでまとめることで粒子生成のテンプレート化を回避
    std::unique_ptr<std::unique_ptr<Type[]>[]> values(new std::unique_ptr<Type[]>[nvariables]);

    // 実行時型分岐で呼び出す
    const std::type_info& type = volume->values().typeInfo()->type();
    if (type == typeid(vismodule::Int8))
    {
        copy_values<vismodule::Int8>(valueArray, values, nvariables, nnodes);
    }  
    else if ( type == typeid( vismodule::Int16  ) )
    {
        copy_values<vismodule::Int16>(valueArray, values, nvariables, nnodes);
    } 
    else if ( type == typeid( vismodule::Int32  ) )
    {
        copy_values<vismodule::Int32>(valueArray, values, nvariables, nnodes);
    }
    else if ( type == typeid( vismodule::Int64  ) )
    {
        copy_values<vismodule::Int64>(valueArray, values, nvariables, nnodes);
    }
    else if ( type == typeid( vismodule::UInt8  ) )
    {
        copy_values<vismodule::UInt8>(valueArray, values, nvariables, nnodes);
    }
    else if ( type == typeid( vismodule::UInt16 ) )
    {
        copy_values<vismodule::UInt16>(valueArray, values, nvariables, nnodes);
    }
    else if ( type == typeid( vismodule::UInt32 ) )
    {
        copy_values<vismodule::UInt32>(valueArray, values, nvariables, nnodes);
    }
    else if ( type == typeid( vismodule::UInt64 ) )
    {
        copy_values<vismodule::UInt64>(valueArray, values, nvariables, nnodes);
    }
    else if ( type == typeid( vismodule::Real32 ) )
    {
        copy_values<vismodule::Real32>(valueArray, values, nvariables, nnodes);
    }
    else if ( type == typeid( vismodule::Real64 ) )
    {
        copy_values<vismodule::Real64>(valueArray, values, nvariables, nnodes);
    }
    else 
    {
        throw std::runtime_error("Unsupported type");
    }

        // 型の生合成を取るために一時的に raw pointer の配列を作る
        std::vector<float*> raw_values(nvariables);
        for (int j = 0; j < nvariables; ++j) 
        {
            raw_values[j] = values[j].get();
        }
 
    if(voltype ==  vismodule::VolumeObjectBase::VolumeType::Unstructured)
    {

//詰め替え処理
        std::vector<float> coordinates; 
        int ncoords;
        std::vector<unsigned int> connections ;
        int ncells; 
        vismodule::VolumeObjectBase::CellType celltype;

        const vismodule::UnstructuredVolumeObject* uvo_p = static_cast<const vismodule::UnstructuredVolumeObject*>( volume );
        vismodule::Vec3 start_point( clntMes.m_start_point[0], clntMes.m_start_point[1], clntMes.m_start_point[2] );
        vismodule::Vec3 end_point( clntMes.m_end_point[0], clntMes.m_end_point[1], clntMes.m_end_point[2] );
       
        coordinates.assign( (float*)volume->coords().begin(),(float * )volume->coords().end()); 
        ncoords =  volume->nnodes();
        connections.assign((unsigned int*)uvo_p->connections().begin(), (unsigned int*)uvo_p->connections().end());
        ncells = uvo_p->ncells();
        celltype = uvo_p->cellType();


        int plot_variable =  std::atoi(clntMes.m_plot_variable.substr(1).c_str()) -1;

        try
        {
            //PlotOverLine plot_over_line(vo_p, clntMes.m_sampling_size, start_point, end_point, plot_variable);
            PlotOverLine plot_over_line( raw_values.data(), nvariables, coordinates.data(), ncoords,
            connections.data(), ncells,  celltype, clntMes.m_sampling_size, start_point, end_point, plot_variable);

            m_object->setValuesOnLine(plot_over_line.values()); 
            m_object->setXAxis(plot_over_line.xAxis()); 
            m_object->setMask(plot_over_line.mask()); 
        }
        catch ( const std::runtime_error& e )
        {
#ifdef _DEBUG		// debug by @hira
            printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
            //        m_object = NULL;
            delete volume;
            throw e;
        }
//        delete vo_p;
    }
    else if(voltype ==  vismodule::VolumeObjectBase::VolumeType::Structured)
    {
                        std::cout << __LINE__ << __FUNCTION__ <<std::endl;
#if 1
        const vismodule::StructuredVolumeObject* object = static_cast<const vismodule::StructuredVolumeObject*>( volume );
        vismodule::Vec3 start_point( clntMes.m_start_point[0], clntMes.m_start_point[1], clntMes.m_start_point[2] );
        vismodule::Vec3 end_point( clntMes.m_end_point[0], clntMes.m_end_point[1], clntMes.m_end_point[2] );

        int plot_variable =  std::atoi(clntMes.m_plot_variable.substr(1).c_str()) -1;

        const vismodule::StructuredVolumeObject* svo_p = static_cast<const vismodule::StructuredVolumeObject*>( volume );
        int resol[3] = { svo_p->resolution().x(), svo_p->resolution().y(), svo_p->resolution().z()};
        domain_parameters_struct dom={
         volume->minObjectCoord().x()
        ,volume->minObjectCoord().y()
        ,volume->minObjectCoord().z()
        ,volume->maxObjectCoord().x()
        ,volume->maxObjectCoord().y()
        ,volume->maxObjectCoord().z()
        ,resol
        ,1.f
        };


        try
        {
            PlotOverLine plot_over_line(dom, raw_values.data(), nvariables, clntMes.m_sampling_size, start_point, end_point, plot_variable);

            m_object->setValuesOnLine(plot_over_line.values()); 
            m_object->setXAxis(plot_over_line.xAxis()); 
            m_object->setMask(plot_over_line.mask()); 
        }
        catch ( const std::runtime_error& e )
        {
#ifdef _DEBUG		// debug by @hira
            printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
            //        m_object = NULL;
            delete volume;
            throw e;
        }
#endif
    }

    delete volume;
//    delete uvo_p;
}

void PlotOverLineGenerator::createFromFile( const Argument& param, const vismodule::Camera& camera, const jpv::ParticleTransferClientMessage& clntMes, const int number_of_divide, const int st, const int vl )
{
    vismodule::Vec3 start_point( clntMes.m_start_point[0], clntMes.m_start_point[1], clntMes.m_start_point[2] );
    vismodule::Vec3 end_point( clntMes.m_end_point[0], clntMes.m_end_point[1], clntMes.m_end_point[2] );

//    bool is_structured = false;
//    bool is_unstructured = false;
//    size_t found_vtm  = param.m_input_data_base.find(".vtm");
//    size_t found_vtu  = param.m_input_data_base.find(".vtu");
//    size_t found_vti  = param.m_input_data_base.find(".vti");
//    size_t found_inp  = param.m_input_data_base.find(".inp");
//    size_t found_pvtu = param.m_input_data_base.find(".pvtu");
//    size_t found_case = param.m_input_data_base.find(".case");
//    vismodule::VolumeObjectBase* volume = nullptr;
//    int plot_variable =  std::atoi(clntMes.m_plot_variable.substr(1).c_str()) -1;
//
//    if ( found_vtm != std::string::npos )
//    {
//        // Stuctured
//        if( m_mvp->m_file_type == 3 )
//        {
//            is_structured = true;
//            volume = new vismodule::StructuredVolumeImporter( m_mvp->m_file_path, st, vl );
//            volume->updateMinMaxValues();
//            volume->setMinMaxObjectCoords( m_mvp->m_min_subvolume_coord[vl], m_mvp->m_max_subvolume_coord[vl] );
//            volume->setMinMaxExternalCoords( m_mvp->m_min_subvolume_coord[vl], m_mvp->m_max_subvolume_coord[vl] );            
//        }
//        // Unstructured
//        if( m_mvp->m_file_type == 4 )
//        {
//            is_unstructured = true;
//            volume = new vismodule::UnstructuredVolumeImporter( m_mvp->m_file_path, m_mvp->m_file_type, m_mvp->m_elem_type, st, vl );
//            volume->updateMinMaxValues();
//            volume->setMinMaxObjectCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );
//            volume->setMinMaxExternalCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );            
//        }
//    }
//    else if ( found_vtu  != std::string::npos ||
//              found_inp  != std::string::npos ||
//              found_pvtu != std::string::npos ||
//              found_case != std::string::npos 
//            )
//    {
//        is_unstructured = true;
//        volume = new vismodule::UnstructuredVolumeImporter( m_mvp->m_file_path, m_mvp->m_file_type, m_mvp->m_elem_type, st, vl );
//        volume->updateMinMaxValues();
//        volume->setMinMaxObjectCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );
//        volume->setMinMaxExternalCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );        
//    }
//    else if ( found_vti != std::string::npos )
//    {
//        is_structured = true;
//        volume = new vismodule::StructuredVolumeImporter( m_mvp->m_file_path, st, vl );
//        volume->updateMinMaxValues();
//        volume->setMinMaxObjectCoords( m_mvp->m_min_subvolume_coord[vl], m_mvp->m_max_subvolume_coord[vl] );
//        volume->setMinMaxExternalCoords( m_mvp->m_min_subvolume_coord[vl], m_mvp->m_max_subvolume_coord[vl] );        
//    }
//
//    std::cout << *volume << std::endl;
//    std::cout << "min:" << volume->minObjectCoord() << ", max:" << volume->maxObjectCoord() << std::endl;
//    std::cout << "min:" << volume->minExternalCoord() << ", max:" << volume->maxExternalCoord() << std::endl;

    // ボリュームデータ読み取り
    size_t found_kvsml = param.m_input_data_base.find(".kvsml");
    size_t found_vtm   = param.m_input_data_base.find(".vtm");
    size_t found_vtu   = param.m_input_data_base.find(".vtu");
    size_t found_vti   = param.m_input_data_base.find(".vti");
    size_t found_inp   = param.m_input_data_base.find(".inp");
    size_t found_pvtu  = param.m_input_data_base.find(".pvtu");
    size_t found_case  = param.m_input_data_base.find(".case");

    vismodule::VolumeObjectBase* volume = nullptr;

    if ( found_kvsml != std::string::npos )
    {
        volume = new vismodule::UnstructuredVolumeImporter( param.m_input_data );
    
        vismodule::File ifpx( m_mvp->m_file_path );
        std::string path_base = ifpx.pathName() + ifpx.Separator() + ifpx.baseName();

        std::cout << "path_base = " << path_base << std::endl;    
        volume = new vismodule::UnstructuredVolumeImporter( path_base, m_mvp->m_file_type, st, vl );
    }
#ifdef EXTEND_FILE_FORMAT
    else if ( found_vtm != std::string::npos )
    {
        // structured
        if( m_mvp->m_file_type == 3 )
        {
            volume = new vismodule::StructuredVolumeImporter( m_mvp->m_file_path, st, vl );
        }
        // unstructured
        if( m_mvp->m_file_type == 4 )
        {
            volume = new vismodule::UnstructuredVolumeImporter( m_mvp->m_file_path, m_mvp->m_file_type, m_mvp->m_elem_type, st, vl );
        }
    }
    else if ( found_vtu  != std::string::npos ||
              found_inp  != std::string::npos ||
              found_pvtu != std::string::npos ||
              found_case != std::string::npos
            )
    {
        volume = new vismodule::UnstructuredVolumeImporter( m_mvp->m_file_path, m_mvp->m_file_type, m_mvp->m_elem_type, st, vl );
    }
    else if ( found_vti != std::string::npos )
    {
        volume = new vismodule::StructuredVolumeImporter( m_mvp->m_file_path, st, vl );
    }
#endif

    VIS_MODULE_TIMER_END( 260 );

    // .vtm .pvtu .case file format
    if ( ( found_vtm != std::string::npos ) || ( found_pvtu != std::string::npos ) || ( found_case != std::string::npos ) )
    {
        // Structured Volume Data
        if ( m_mvp->m_file_type == 3 )
        {
            volume->updateMinMaxValues();
            volume->setMinMaxObjectCoords( m_mvp->m_min_subvolume_coord[vl], m_mvp->m_max_subvolume_coord[vl] );
            volume->setMinMaxExternalCoords( m_mvp->m_min_subvolume_coord[vl], m_mvp->m_max_subvolume_coord[vl] );
        }

        // Unstructured Volume Data
        else if ( m_mvp->m_file_type == 4 )
        {
            volume->updateMinMaxValues();
            volume->setMinMaxObjectCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );
            volume->setMinMaxExternalCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );
        }
    }
    else
    {
        volume->setMinMaxValues( m_mvp->m_min_value, m_mvp->m_max_value );
        volume->setMinMaxObjectCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );
        volume->setMinMaxExternalCoords( m_mvp->m_min_object_coord, m_mvp->m_max_object_coord );
    }


    //詰め替え処理
    vismodule::AnyValueArray valueArray; 
    valueArray = volume->values(); 
    int nnodes = volume->nnodes();
    int nvariables = volume->veclen();

    // ここで変数の値をfloatでまとめることで粒子生成のテンプレート化を回避
    std::unique_ptr<std::unique_ptr<Type[]>[]> values(new std::unique_ptr<Type[]>[nvariables]);

    // 実行時型分岐で呼び出す
    const std::type_info& type = volume->values().typeInfo()->type();
    if (type == typeid(vismodule::Int8))
    {
        copy_values<vismodule::Int8>(valueArray, values, nvariables, nnodes);
    }  
    else if ( type == typeid( vismodule::Int16  ) )
    {
        copy_values<vismodule::Int16>(valueArray, values, nvariables, nnodes);
    } 
    else if ( type == typeid( vismodule::Int32  ) )
    {
        copy_values<vismodule::Int32>(valueArray, values, nvariables, nnodes);
    }
    else if ( type == typeid( vismodule::Int64  ) )
    {
        copy_values<vismodule::Int64>(valueArray, values, nvariables, nnodes);
    }
    else if ( type == typeid( vismodule::UInt8  ) )
    {
        copy_values<vismodule::UInt8>(valueArray, values, nvariables, nnodes);
    }
    else if ( type == typeid( vismodule::UInt16 ) )
    {
        copy_values<vismodule::UInt16>(valueArray, values, nvariables, nnodes);
    }
    else if ( type == typeid( vismodule::UInt32 ) )
    {
        copy_values<vismodule::UInt32>(valueArray, values, nvariables, nnodes);
    }
    else if ( type == typeid( vismodule::UInt64 ) )
    {
        copy_values<vismodule::UInt64>(valueArray, values, nvariables, nnodes);
    }
    else if ( type == typeid( vismodule::Real32 ) )
    {
        copy_values<vismodule::Real32>(valueArray, values, nvariables, nnodes);
    }
    else if ( type == typeid( vismodule::Real64 ) )
    {
        copy_values<vismodule::Real64>(valueArray, values, nvariables, nnodes);
    }
    else 
    {
        throw std::runtime_error("Unsupported type");
    }

    // 型の生合成を取るために一時的に raw pointer の配列を作る
    std::vector<float*> raw_values(nvariables);
    for (int j = 0; j < nvariables; ++j) 
    {
        raw_values[j] = values[j].get();
    }


    // ボリュームタイプ取得
    vismodule::VolumeObjectBase::VolumeType voltype = volume->volumeType();
    // plot 対象変数の取得    
    int plot_variable =  std::atoi(clntMes.m_plot_variable.substr(1).c_str()) -1;

    if(voltype ==  vismodule::VolumeObjectBase::VolumeType::Unstructured)
    {
        const vismodule::UnstructuredVolumeObject* uvo_p = static_cast<const vismodule::UnstructuredVolumeObject*>( volume );

        vismodule::VolumeObjectBase::CellType celltype = uvo_p->cellType();
//        switch(volume->cellType())
//        {
//            case vismodule::VolumeObjectBase::Tetrahedra:
//                {
//                    vo_p->setCellType(vismodule::VolumeObjectBase::Tetrahedra); 
//                    break;
//                }
//            case vismodule::VolumeObjectBase::QuadraticTetrahedra:
//                {
//                    vo_p->setCellType(vismodule::VolumeObjectBase::QuadraticTetrahedra); 
//                    break;
//                }
//            case vismodule::VolumeObjectBase::Hexahedra:
//                {
//                    vo_p->setCellType(vismodule::VolumeObjectBase::Hexahedra); 
//                    break;
//                }
//            case vismodule::VolumeObjectBase::QuadraticHexahedra:
//                {
//                    vo_p->setCellType(vismodule::VolumeObjectBase::QuadraticHexahedra); 
//                    break;
//                }
//            case vismodule::VolumeObjectBase::Prism:
//                {
//                    vo_p->setCellType(vismodule::VolumeObjectBase::Prism); 
//                    break;
//                }
//            case vismodule::VolumeObjectBase::Pyramid:
//                {
//                    vo_p->setCellType(vismodule::VolumeObjectBase::Pyramid); 
//                    break;
//                }
//            default:
//                {
//                    visModuleMessageError( "Unsupported cell type." );
//                    return;
//                }
//        }

//        vo_p->setNNodes( uvo_p->nnodes()); 
//        vo_p->setNCells( uvo_p->ncells()); 
//        vo_p->setCoords( uvo_p->coords()); 
//        vo_p->setVeclen( uvo_p->veclen()); 
//        vo_p->setValues( uvo_p->values()); 
//        vo_p->setConnections( uvo_p->connections());
 
//詰め替え処理
        std::vector<float> coordinates; 
        int ncoords;
        std::vector<unsigned int> connections ;
        int ncells; 
        
        coordinates.assign( (float * )volume->coords().begin(),(float * )volume->coords().end()); 
        ncoords =  volume->nnodes();
        connections.assign((unsigned int*)uvo_p->connections().begin(), (unsigned int*)uvo_p->connections().end());
        ncells = uvo_p->ncells();
       
        try
        {
            PlotOverLine plot_over_line( raw_values.data(), nvariables, coordinates.data(), ncoords,
            connections.data(), ncells,  celltype, clntMes.m_sampling_size, start_point, end_point, plot_variable);
            m_object->setValuesOnLine(plot_over_line.values()); 
            m_object->setXAxis(plot_over_line.xAxis()); 
            m_object->setMask(plot_over_line.mask()); 
        }
        catch ( const std::runtime_error& e )
        {
#ifdef _DEBUG		// debug by @hira
            printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
            //        m_object = NULL;
            delete volume;
            throw e;
        }

    }
    else if(voltype ==  vismodule::VolumeObjectBase::VolumeType::Structured)
    {
        const vismodule::StructuredVolumeObject* svo_p = static_cast<const vismodule::StructuredVolumeObject*>( volume );

        int resol[3] = { svo_p->resolution().x(), svo_p->resolution().y(), svo_p->resolution().z()};
        domain_parameters_struct dom={
         volume->minObjectCoord().x()
        ,volume->minObjectCoord().y()
        ,volume->minObjectCoord().z()
        ,volume->maxObjectCoord().x()
        ,volume->maxObjectCoord().y()
        ,volume->maxObjectCoord().z()
        ,resol
        ,1.f
        };

        try
        {
            PlotOverLine plot_over_line(dom, raw_values.data(), nvariables, clntMes.m_sampling_size, start_point, end_point, plot_variable);
            //PlotOverLine plot_over_line(object, clntMes.m_sampling_size, start_point, end_point, plot_variable);
            m_object->setValuesOnLine(plot_over_line.values()); 
            m_object->setXAxis(plot_over_line.xAxis()); 
            m_object->setMask(plot_over_line.mask()); 

        }
        catch ( const std::runtime_error& e )
        {
#ifdef _DEBUG		// debug by @hira
            printf("[Exception] %s[%d] :: %s \n", __FILE__, __LINE__, e.what());
#endif
            //        m_object = NULL;
            delete volume;
            throw e;
        }
    }
    else
    {
        visModuleMessageError( "Unsupported Volume type." );
        return;        
    }

    delete volume;
}

std::string PlotOverLineGenerator::getErrorMessage( const size_t maxMemory ) const
{
    std::string errorMessage( "" );
    //const size_t totalMemory = sizeof( float ) * m_object.values_on_line().size() + sizeof( float ) * m_object.values_on_line().size() + sizeof( bool ) * m_object.values_on_line().size();
    const size_t totalMemory = sizeof( float ) * m_object->values_on_line().size() + sizeof( float ) * m_object->values_on_line().size() + sizeof( bool ) * m_object->values_on_line().size();
    if ( totalMemory > maxMemory )
    {
        char ms[512];
        sprintf( ms, "Memory Error: TotalMemory = %zd, MaxMemory = %zd\n", totalMemory, maxMemory );
        errorMessage += ms;
    }
    return errorMessage;
}

const vismodule::ValueArray<vismodule::Real32>& PlotOverLineGenerator::coords( void ) const
{
    return( m_coords );
}

const vismodule::ValueArray<vismodule::UInt8>& PlotOverLineGenerator::colors( void ) const
{
    return( m_colors );
}

const vismodule::ValueArray<vismodule::Real32>& PlotOverLineGenerator::directions( void ) const
{
    return( m_directions );
}

const vismodule::ValueArray<vismodule::Real32>& PlotOverLineGenerator::sizes( void ) const
{
    return( m_sizes );
}

void PlotOverLineGenerator::setCoords( const vismodule::ValueArray<vismodule::Real32>& coords )
{
    m_coords = coords;
}

void PlotOverLineGenerator::setColors( const vismodule::ValueArray<vismodule::UInt8>& colors )
{
    m_colors = colors;
}

void PlotOverLineGenerator::setDirections( const vismodule::ValueArray<vismodule::Real32>& directions )
{
    m_directions = directions;
}

void PlotOverLineGenerator::setSizes( const vismodule::ValueArray<vismodule::Real32>& sizes )
{
    m_sizes = sizes;
}

void PlotOverLineGenerator::clear()
{
    m_sizes.deallocate();
    m_directions.deallocate();
    m_coords.deallocate();
    m_colors.deallocate();
}

template <typename T>
void PlotOverLineGenerator::copy_values(vismodule::AnyValueArray& valueArray, std::unique_ptr<std::unique_ptr<Type[]>[]>& values, int nvariables, int nnodes) 
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
