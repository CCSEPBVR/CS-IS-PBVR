#include <string>
#include <iostream>
#include <cstdlib>
#include "Hydrogen.h"
#include "kvs_wrapper.h"
#include <mpi.h>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/ValueArray>
#include <kvs/PointObject>
#include <kvs/TransferFunction>
#include <kvs/CellByCellRejectionSampling>

#include <kvs/KVSMLObjectUnstructuredVolume>
#include <kvs/UnstructuredVolumeExporter>

#define RANK 1

/*===========================================================================*/
/**
 *  @brief  Main function.
 *  @param  argc [i] argument counter
 *  @param  argv [i] argument values
 */
/*===========================================================================*/
int main( int argc, char** argv )
{
     MPI_Init( &argc, &argv );
    int mpi_rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &(mpi_rank) );

    // Generate hydrogen volume.
    Hydrogen hydro;

    //全体の座標
    domain_parameters dom = {
        hydro.global_min_coord.x(),
        hydro.global_min_coord.y(),
        hydro.global_min_coord.z(),
        hydro.global_max_coord.x(),
        hydro.global_max_coord.y(),
        hydro.global_max_coord.z()
    };

    int time_step = 0;
#if 0
    begin_wrapper_async_io();
    for(int i =0; i< 1; i++ )
    //for(;;)
    {
        if(mpi_rank==0) hydro.show();

        ensemble_generate_particles( time_step, dom,
                            hydro.values, hydro.nvariables,
                            hydro.coords, hydro.nnodes,
                            hydro.connections, hydro.ncells, pbvr::VolumeObjectBase::CellType::Tetrahedra  );

        time_step++;
    }
    end_wrapper_async_io();
#else
#if 1
    // 平均値計算(逐次)
    Hydrogen hydro_average; 
    int mpi_size = 8;
    //int mpi_size = 7;
    //MPI_Comm_rank( MPI_COMM_WORLD, &(mpi_size) );
    // 平均値ボリューム作成
    //hydro.calc_average(mpi_size);
    //各アンサンブルボリューム作成
    hydro.calc_each_ens(mpi_rank);
//    for (int i=0; i< hydro.nnodes; i++)
//    {
//        if(mpi_rank == 0 && hydro.values[0][i] > 150.f )std::cout << "recv_values["<< i << "] = " << hydro.values[0][i] << std::endl;
//    } 
//    //無作為に座標を抽出
//    std::cout << mpi_rank  << " : hydro.values[0][119] =  " << hydro.values[119] << std::endl;
////    std::cout << mpi_rank  << " : hydro.coords[0][11119] =  " << hydro.coords[11119] << std::endl;
////    std::cout << mpi_rank  << " : hydro.coords[0][11120] =  " << hydro.coords[11120] << std::endl;
////    std::cout << mpi_rank  << " : hydro.coords[0][11121] =  " << hydro.coords[11121] << std::endl;
//    std::cout << mpi_rank  << " : hydro.connections[0][11119] =  " << hydro.connections[11119] << std::endl;
//    std::cout << mpi_rank  << " : hydro.connections[0][11120] =  " << hydro.connections[11120] << std::endl;
//    std::cout << mpi_rank  << " : hydro.connections[0][11121] =  " << hydro.connections[11121] << std::endl;

    // 出力用ボリュームデータを作成
    std::vector<float> recv_values(hydro.nnodes);
    std::vector<unsigned int> recv_connections(hydro.ncells* 4);
    std::vector<float> recv_coords(hydro.nnodes*3);
//    std::vector<unsigned int> recv_connections(hydro.connections, hydro.connections+hydro.ncells*4);
//    std::vector<float> recv_coords(hydro.coords, hydro.coords+hydro.nnodes*3);
//    std::vector<float> recv_coords(hydro.coords, hydro.coords+hydro.nnodes*3);
    std::vector<float> send_value(hydro.nnodes);
    for (int i=0; i< hydro.nnodes; i++)
    {
        send_value[i]  = hydro.values[0][i];
        recv_coords[3*i+0] = hydro.coords[3*i+0];
        recv_coords[3*i+1] = hydro.coords[3*i+1];
        recv_coords[3*i+2] = hydro.coords[3*i+2];
    }

    for (int i=0; i< recv_connections.size(); i++)
    {
        recv_connections[i] = hydro.connections[i];
    }

//    mpi_size = 8;
//    for (int i=0; i< hydro.nnodes; i++)
//    {
//        hydro.values[0][i] = 0;
//    }
//    hydro.calc_average(mpi_size);
    for (int i=0; i< hydro.nnodes; i++)
    {
        recv_values[i] = hydro.values[0][i];
    }

    kvs::ValueArray<float> tmp_coords(recv_coords); 
    kvs::ValueArray<unsigned int> tmp_connections(recv_connections); 
    kvs::ValueArray<float> tmp_values(recv_values); 
    kvs::AnyValueArray array_values(tmp_values); 

    kvs::UnstructuredVolumeObject* ave_volume = new kvs::UnstructuredVolumeObject(kvs::UnstructuredVolumeObject::CellType::Tetrahedra
    //kvs::UnstructuredVolumeObject* ave_volume = new kvs::UnstructuredVolumeObject(kvs::UnstructuredVolumeObject::CellType::Hexahedra
                                              ,hydro.nnodes,hydro.ncells,hydro.nvariables
                                              ,tmp_coords, tmp_connections, array_values);
 
    // volume object　を出力する
    if(mpi_rank ==0)
    {
        kvs::KVSMLObjectUnstructuredVolume* kvsml_object = new kvs::UnstructuredVolumeExporter<kvs::KVSMLObjectUnstructuredVolume>( ave_volume );
        kvsml_object->setWritingDataType( kvs::KVSMLObjectUnstructuredVolume::ExternalBinary );
        kvsml_object->write( "test_volume_00000_0000001_0000001.kvsml" );
        delete kvsml_object;
    }


    for(int i =0; i< 1; i++ )
    //for(;;)
    {
        if(mpi_rank==0) hydro.show();

//        ensemble_generate_particles( time_step, dom,
//                            hydro_average.values, hydro_average.nvariables,
//                            hydro_average.coords, hydro_average.nnodes,
//                            hydro_average.connections, hydro_average.ncells, pbvr::VolumeObjectBase::CellType::Tetrahedra  );
        ensemble_generate_particles( time_step, dom,
                            hydro.values, hydro.nvariables,
                            hydro.coords, hydro.nnodes,
                            hydro.connections, hydro.ncells, pbvr::VolumeObjectBase::CellType::Tetrahedra  );


        time_step++;
    }
#else
    // 平均値計算(アンサンブル並列)
    Hydrogen hydro_average; 
    int mpi_size = 8;
    //MPI_Comm_rank( MPI_COMM_WORLD, &(mpi_size) );
    //hydro.calc_average(mpi_size);
    //hydro.calc_each_ens(mpi_rank);
    hydro.calc_each_ens(mpi_size);
    std::vector<float> recv_values(hydro.nnodes);
    //std::vector<unsigned int> recv_connections(hydro.ncells*4);
    std::vector<unsigned int> recv_connections(hydro.connections, hydro.connections+hydro.ncells*4);
    std::vector<float> recv_coords(hydro.coords, hydro.coords+hydro.nnodes*3);
//    std::vector<float> vec(std::begin(arr), std::end(arr));

    MPI_Allreduce(hydro.values[0],recv_values.data(),hydro.nnodes,MPI_FLOAT,MPI_SUM,MPI_COMM_WORLD);     
    for (int i=0; i< hydro.nnodes; i++)
    {
        recv_values[i] /= mpi_size;
    } 

    
//    for(int i=0; i< hydro.nnodes*3; i++ )
//    {
//        recv_coords[i] = ;
//    }


    kvs::ValueArray<float> tmp_coords(recv_coords); 
    kvs::ValueArray<unsigned int> tmp_connections(recv_connections); 
    kvs::AnyValueArray tmp_values(recv_values); 

    kvs::UnstructuredVolumeObject* ave_volume = new kvs::UnstructuredVolumeObject(kvs::VolumeObjectBase::CellType::Tetrahedra
                                              ,hydro.nnodes,hydro.ncells,hydro.nvariables
                                              ,tmp_coords, tmp_connections, tmp_values);

    int subpixel_level=1; 
    int tf_resolution = 256;
    auto tf = kvs::TransferFunction( tf_resolution );
    float min_value = 0;
    float max_value = 255;
    tf.setRange(min_value, max_value);


    for(int i =0; i< 1; i++ )
    //for(;;)
    {
        if(mpi_rank==0) hydro.show();

//        ensemble_generate_particles( time_step, dom,
//                            hydro_average.values, hydro_average.nvariables,
//                            hydro_average.coords, hydro_average.nnodes,
//                            hydro_average.connections, hydro_average.ncells, pbvr::VolumeObjectBase::CellType::Tetrahedra  );
//        generate_particles( time_step, dom,
//                            hydro.values, hydro.nvariables,
//                            hydro.coords, hydro.nnodes,
//                            hydro.connections, hydro.ncells, pbvr::VolumeObjectBase::CellType::Tetrahedra  );

        kvs::PointObject* object = new kvs::CellByCellRejectionSampling( ave_volume, subpixel_level, 12.8, tf );

        time_step++;
    }


#endif
#endif


    MPI_Finalize();

    return 0;
}
