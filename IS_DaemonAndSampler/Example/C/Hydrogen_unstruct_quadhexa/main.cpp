#include <string>
#include <iostream>
#include <cstdlib>
#include "Hydrogen.h"
#include "kvs_wrapper.h"
#include <mpi.h>
#include <fstream>
#include <string>
#include <kvs/KVSMLObjectUnstructuredVolume>
#include <kvs/UnstructuredVolumeExporter>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/VolumeObjectBase>
#include <kvs/ValueArray>

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

#ifdef VTK 
    int celltype = 20;
    std::ofstream writing_file;
    std::string filename = "hydro" +std::to_string(mpi_rank) + ".vtu";
    writing_file.open(filename, std::ios::out);
    writing_file << "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\">" << std::endl;
    writing_file << "<UnstructuredGrid>" << std::endl;
    writing_file << "<Piece NumberOfPoints=\"" << hydro.nnodes <<"\" NumberOfCells=\"" << hydro.ncells <<" \">" << std::endl;
    writing_file << "<Points>" << std::endl;
    writing_file << "<DataArray type=\"Float32\" NumberOfComponents=\"3\" format=\"ascii\">" << std::endl;
    for(int i=0; i<hydro.nnodes * 3; i +=3 )
    //for(int i=0; i< 8 * 3; i +=3 )
    {
        std::string writing_coord = std::to_string(hydro.coords[i]) + " " + std::to_string(hydro.coords[i+1]) + " " + std::to_string(hydro.coords[i+2]);
        //std::string writing_coord = std::to_string(CoordArray[i]) + " " + std::to_string(CoordArray[i+1]) + " " + std::to_string(CoordArray[i+2]);
        writing_file << writing_coord << std::endl;
    }
    writing_file << "</DataArray>" << std::endl;
    writing_file << "</Points>" << std::endl;
    writing_file << "<Cells>" << std::endl;
    writing_file << "<DataArray NumberOfComponents=\"1\" type=\"Int32\" Name=\"offsets\" format=\"ascii\">" << std::endl;
    //for(int i=0; i < 1 ; i++ )
    for(int i=0; i < hydro.ncells ; i++ )
    {
    writing_file << std::to_string(celltype*(i+1)) << " " ;
    }
    writing_file << "</DataArray>" << std::endl;
    writing_file << "<DataArray NumberOfComponents=\"1\" type=\"Int32\" Name=\"connectivity\" format=\"ascii\">" << std::endl;
    //for(int i=0; i < 1 * 8; i+=8 )
    for(int i=0; i < hydro.ncells * celltype; i+=celltype )
    {
        std::string writing_cells =  std::to_string(hydro.connections[i]) +   " " + std::to_string(hydro.connections[i+1]) + " " + std::to_string(hydro.connections[i+2])
                                    + " " + std::to_string(hydro.connections[i+3]) + " " + std::to_string(hydro.connections[i+4]) + " " + std::to_string(hydro.connections[i+5]) 
                                    + " " + std::to_string(hydro.connections[i+6]) + " " + std::to_string(hydro.connections[i+7]) + " " + std::to_string(hydro.connections[i+8])  
                                    + " " + std::to_string(hydro.connections[i+9]) + " " + std::to_string(hydro.connections[i+10]) + " " + std::to_string(hydro.connections[i+11])  
                                    + " " + std::to_string(hydro.connections[i+12]) + " " + std::to_string(hydro.connections[i+13]) + " " + std::to_string(hydro.connections[i+14])  
                                    + " " + std::to_string(hydro.connections[i+15]) + " " + std::to_string(hydro.connections[i+16]) + " " + std::to_string(hydro.connections[i+17])  
                                    + " " + std::to_string(hydro.connections[i+18]) + " " + std::to_string(hydro.connections[i+19]);  
        writing_file << writing_cells << std::endl;
    }
    writing_file << "</DataArray>" << std::endl;
    writing_file << "<DataArray NumberOfComponents=\"1\" type=\"Int32\" Name=\"types\" format=\"ascii\">" << std::endl;
    for(int i=0; i < hydro.ncells ; i++ )
    //for(int i=0; i < 1 ; i++ )
    {
    //writing_file << "12" << std::endl;
    writing_file << "25" << std::endl;
    }
    writing_file << "</DataArray>" << std::endl;
    writing_file << "</Cells>" << std::endl;
    writing_file << "<PointData>" << std::endl;
    writing_file << "<DataArray NumberOfComponents=\"1\" type=\"Float32\" Name=\"point_scalars\" format=\"ascii\">" << std::endl;
    //for(int i=0; i<8 ; i++ )
    for(int i=0; i<hydro.nnodes; i++ )
    {
        writing_file << std::to_string(hydro.values[0][i]) << std::endl;
        //writing_file << std::to_string(ValueArray[i]) << std::endl;
    }
    writing_file << "</DataArray>" << std::endl;
    writing_file << "</PointData>" << std::endl;
    writing_file << "<CellData>" << std::endl;
    writing_file << "</CellData>" << std::endl;
    writing_file << "</Piece>" << std::endl;
    writing_file << "</UnstructuredGrid>" << std::endl;
    writing_file << "</VTKFile>" << std::endl;
  
    if (mpi_rank == 0)
    { 
        std::ofstream pvtu_file;
        std::string pfilename = "hydro.pvtu";
        pvtu_file.open(pfilename, std::ios::out);
        pvtu_file << "<VTKFile type=\"PUnstructuredGrid\" version=\"1.0\">" << std::endl; 
        pvtu_file << "<PUnstructuredGrid>" << std::endl; 
        pvtu_file << "<PPoints>" << std::endl; 
        pvtu_file << "<PDataArray type=\"Float32\" Name=\"Position\" NumberOfComponents=\"3\"/>" << std::endl; 
        pvtu_file << "</PPoints>" << std::endl; 
        pvtu_file << "<PCells>" << std::endl; 
        pvtu_file << "<PDataArray NumberOfComponents=\"1\" type=\"Int32\" Name=\"connectivity\"/>" << std::endl;
        pvtu_file << "<PDataArray NumberOfComponents=\"1\" type=\"Int32\" Name=\"offsets\"/>" << std::endl;
        pvtu_file << "<PDataArray NumberOfComponents=\"1\" type=\"Int32\" Name=\"types\"/>" << std::endl;
        pvtu_file << "</PCells>" << std::endl;
        pvtu_file << "<PPointData Scalars=\"point_scalars\">" << std::endl; 
        pvtu_file << "<PDataArray NumberOfComponents=\"1\" type=\"Float32\" Name=\"point_scalars\"/>" << std::endl;
        pvtu_file << "</PPointData>" << std::endl; 
        pvtu_file << "<Piece Source=\"hydro0.vtu\"/>" << std::endl;
        pvtu_file << "<Piece Source=\"hydro1.vtu\"/>" << std::endl;
        pvtu_file << "<Piece Source=\"hydro2.vtu\"/>" << std::endl;
        pvtu_file << "<Piece Source=\"hydro3.vtu\"/>" << std::endl;
        pvtu_file << "</PUnstructuredGrid>" <<std::endl; 
        pvtu_file << "</VTKFile>" << std::endl;
    }
#else  

    begin_wrapper_async_io();
    for(;;)
    {
        if(mpi_rank==RANK) hydro.show();

        generate_particles( time_step, dom,
                            hydro.values, hydro.nvariables,
                            hydro.coords, hydro.nnodes,
                            hydro.connections, hydro.ncells, pbvr::VolumeObjectBase::CellType::QuadraticHexahedra );

        time_step++;
    }
    end_wrapper_async_io();

#endif

    MPI_Finalize();

    return 0;
}
